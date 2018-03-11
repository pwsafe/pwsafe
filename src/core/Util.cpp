/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file Util.cpp
//-----------------------------------------------------------------------------

#include "sha1.h"
#include "BlowFish.h"
#include "PWSrand.h"
#include "PwsPlatform.h"
#include "core.h"
#include "StringXStream.h"
#include "PWPolicy.h"
#include "./UTF8Conv.h"

#include "Util.h"

#include "os/debug.h"
#include "os/pws_tchar.h"
#include "os/dir.h"
#include "os/env.h"
#include "os/file.h"
#include "os/utf8conv.h"

#include <stdio.h>
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <sstream>
#include <iomanip>

#include <errno.h>

using namespace std;

// used by CBC routines...
static void xormem(unsigned char *mem1, const unsigned char *mem2, int length)
{
  for (int x = 0; x < length; x++)
    mem1[x] ^= mem2[x];
}

//-----------------------------------------------------------------------------
//Overwrite the memory
// used to be a loop here, but this was deemed (1) overly paranoid
// (2) The wrong way to scrub DRAM memory
// see http://www.cs.auckland.ac.nz/~pgut001/pubs/secure_del.html
// and http://www.cypherpunks.to/~peter/usenix01.pdf

#ifdef _WIN32
#pragma optimize("",off)
#endif
void trashMemory(void *buffer, size_t length)
{
  ASSERT(buffer != nullptr);
  // {kjp} no point in looping around doing nothing is there?
  if (length > 0) {
    std::memset(buffer, 0x55, length);
    std::memset(buffer, 0xAA, length);
    std::memset(buffer,    0, length);
#ifdef __GNUC__
    // break compiler optimization of this function for gcc
    // see trick used in google's boring ssl:
    // https://boringssl.googlesource.com/boringssl/+/ad1907fe73334d6c696c8539646c21b11178f20f%5E!/#F0
    __asm__ __volatile__("" : : "r"(buffer) : "memory");
#endif
  }
}
#ifdef _WIN32
#pragma optimize("",on)
#endif
void trashMemory(LPTSTR buffer, size_t length)
{
  trashMemory(reinterpret_cast<unsigned char *>(buffer), length * sizeof(buffer[0]));
}

/**
Burn some stack memory
@param len amount of stack to burn in bytes
*/
void burnStack(unsigned long len)
{
  unsigned char buf[32];
  trashMemory(buf, sizeof(buf));
  if (len > static_cast<unsigned long>(sizeof(buf)))
    burnStack(len - sizeof(buf));
}

void ConvertPasskey(const StringX &text,
                   unsigned char *&txt,
                   size_t &txtlen)
{
  bool isUTF8 = pws_os::getenv("PWS_PK_CP_ACP", false).empty();
  LPCTSTR txtstr = text.c_str();
  txtlen = text.length();

  size_t dstlen = pws_os::wcstombs(nullptr, 0, txtstr, txtlen, isUTF8);
#ifdef _MSC_VER
  dstlen++; // ugly, but no easy way around this now
#endif
  char *dst = new char[dstlen];

  size_t res = pws_os::wcstombs(dst, dstlen, txtstr, txtlen, isUTF8);
  ASSERT(res != 0);
  txt = reinterpret_cast<unsigned char *>(dst);
  txtlen = dstlen - 1;
  txt[txtlen] = '\0'; // not strictly needed, since txtlen returned, but helps debug
}

// Generates a passkey-based hash from stuff - used to validate the passkey
// Used by file encryption/decryption and by V1V2 
void GenRandhash(const StringX &a_passkey,
                 const unsigned char *a_randstuff,
                 unsigned char *a_randhash)
{
  size_t pkeyLen = 0;
  unsigned char *pstr = nullptr;

  ConvertPasskey(a_passkey, pstr, pkeyLen);

  /*
  tempSalt <- H(a_randstuff + a_passkey)
  */
  SHA1 keyHash;
  keyHash.Update(a_randstuff, StuffSize);
  keyHash.Update(pstr, reinterpret_cast<int &>(pkeyLen));

  trashMemory(pstr, pkeyLen);
  delete[] pstr;

  unsigned char tempSalt[SHA1::HASHLEN];
  keyHash.Final(tempSalt);

  /*
  tempbuf <- a_randstuff encrypted 1000 times using tempSalt as key?
  */

  BlowFish Cipher(tempSalt, sizeof(tempSalt));

  unsigned char tempbuf[StuffSize];
  memcpy(reinterpret_cast<char *>(tempbuf), reinterpret_cast<const char *>(a_randstuff), StuffSize);

  for (int x=0; x<1000; x++)
    Cipher.Encrypt(tempbuf, tempbuf);

  /*
  hmm - seems we're not done with this context
  we throw the tempbuf into the hasher, and extract a_randhash
  */
  keyHash.Update(tempbuf, StuffSize);
  keyHash.Final(a_randhash);
}

size_t _writecbc(FILE *fp, const unsigned char *buffer, size_t length, unsigned char type,
                 Fish *Algorithm, unsigned char *cbcbuffer)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  size_t numWritten = 0;

  // some trickery to avoid new/delete
  unsigned char block1[16];

  unsigned char *curblock = nullptr;
  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...

  // First encrypt and write the length of the buffer
  curblock = block1;
  // Fill unused bytes of length with random data, to make
  // a dictionary attack harder
  PWSrand::GetInstance()->GetRandomData(curblock, BS);
  // block length overwrites 4 bytes of the above randomness.
  putInt32(curblock, static_cast<int32>(length));

  // following new for format 2.0 - lengthblock bytes 4-7 were unused before.
  curblock[sizeof(int32)] = type;

  if (BS == 16) {
    // In this case, we've too many (11) wasted bytes in the length block
    // So we store actual data there:
    // (11 = BlockSize - 4 (length) - 1 (type)
    const size_t len1 = (length > 11) ? 11 : length;
    memcpy(curblock + 5, buffer, len1);
    length -= len1;
    buffer += len1;
  }

  xormem(curblock, cbcbuffer, BS); // do the CBC thing
  Algorithm->Encrypt(curblock, curblock);
  memcpy(cbcbuffer, curblock, BS); // update CBC for next round

  numWritten = fwrite(curblock, 1, BS, fp);
  if (numWritten != BS) {
    trashMemory(curblock, BS);
    throw(EIO);
  }

  numWritten += _writecbc(fp, buffer, length, Algorithm, cbcbuffer);

  trashMemory(curblock, BS);
  return numWritten;
}

size_t _writecbc(FILE *fp, const unsigned char *buffer, size_t length,
                 Fish *Algorithm, unsigned char *cbcbuffer)
{
  // Doesn't write out length, just CBC's the data, padding with randomness
  // as required.

  const unsigned int BS = Algorithm->GetBlockSize();
  size_t numWritten = 0;

  // some trickery to avoid new/delete
  unsigned char block1[16];

  unsigned char *curblock = nullptr;
  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...

  // First encrypt and write the length of the buffer
  curblock = block1;
  // Fill unused bytes of length with random data, to make
  // a dictionary attack harder
  PWSrand::GetInstance()->GetRandomData(curblock, BS);

  if (length > 0 ||
      (BS == 8 && length == 0)) { // This part for bwd compat w/pre-3 format
    size_t BlockLength = ((length + (BS - 1)) / BS) * BS;
    if (BlockLength == 0 && BS == 8)
      BlockLength = BS;

    // Now, encrypt and write the (rest of the) buffer
    for (unsigned int x = 0; x < BlockLength; x += BS) {
      if ((length == 0) || ((length % BS != 0) && (length - x < BS))) {
        //This is for an uneven last block
        PWSrand::GetInstance()->GetRandomData(curblock, BS);
        memcpy(curblock, buffer + x, length % BS);
      } else
        memcpy(curblock, buffer + x, BS);
      xormem(curblock, cbcbuffer, BS);
      Algorithm->Encrypt(curblock, curblock);
      memcpy(cbcbuffer, curblock, BS);
      size_t nw =  fwrite(curblock, 1, BS, fp);
      if (nw != BS) {
        trashMemory(curblock, BS);
        throw(EIO);
      }
      numWritten += nw;
    }
  }
  trashMemory(curblock, BS);
  return numWritten;
}

/*
* Reads an encrypted record into buffer.
* The first block of the record contains the encrypted record length
* We have the usual ugly problem of fixed buffer lengths in C/C++.
* allocate the buffer here, to ensure that it's long enough.
* *** THE CALLER MUST delete[] IT AFTER USE *** UGH++
*
* (unless buffer_len is zero)
*
* Note that the buffer is a byte array, and buffer_len is number of
* bytes. This means that any data can be passed, and we don't
* care at this level if strings are char or wchar_t.
*
* If TERMINAL_BLOCK is non-nullptr, the first block read is tested against it,
* and -1 is returned if it matches. (used in V3)
*/
size_t _readcbc(FILE *fp,
         unsigned char* &buffer, size_t &buffer_len, unsigned char &type,
         Fish *Algorithm, unsigned char *cbcbuffer,
         const unsigned char *TERMINAL_BLOCK, ulong64 file_len)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  size_t numRead = 0;

  // some trickery to avoid new/delete
 // Initialize memory.  (Lockheed Martin) Secure Coding  11-14-2007
  unsigned char block1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char block2[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char block3[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char *lengthblock = nullptr;

  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...

  // Safety check.  (Lockheed Martin) Secure Coding  11-14-2007
  if ((BS > sizeof( block1 )) || (BS == 0))
   return 0;

  lengthblock = block1;

  buffer_len = 0;
  numRead = fread(lengthblock, 1, BS, fp);
  if (numRead != BS) {
    return 0;
  }

  if (TERMINAL_BLOCK != nullptr &&
    memcmp(lengthblock, TERMINAL_BLOCK, BS) == 0)
    return static_cast<size_t>(-1);

  unsigned char *lcpy = block2;
  memcpy(lcpy, lengthblock, BS);

  Algorithm->Decrypt(lengthblock, lengthblock);
  xormem(lengthblock, cbcbuffer, BS);
  memcpy(cbcbuffer, lcpy, BS);

  size_t length = getInt32(lengthblock);

  // new for 2.0 -- lengthblock[4..7] previously set to zero
  type = lengthblock[sizeof(int32)]; // type is first byte after the length

  if ((file_len != 0 && length >= file_len)) {
    pws_os::Trace0(_T("_readcbc: Read size larger than file length - aborting\n"));
    buffer = nullptr;
    buffer_len = 0;
    trashMemory(lengthblock, BS);
    return 0;
  }

  buffer_len = length;
  buffer = new unsigned char[(length / BS) * BS + 2 * BS]; // round upwards
  unsigned char *b = buffer;

  // Initialize memory.  (Lockheed Martin) Secure Coding  11-14-2007
  memset(b, 0, (length / BS) * BS + 2 * BS);

  if (BS == 16) {
    // length block contains up to 11 (= 16 - 4 - 1) bytes
    // of data
    const size_t len1 = (length > 11) ? 11 : length;
    memcpy(b, lengthblock + 5, len1);
    length -= len1;
    b += len1;
  }

  size_t BlockLength = ((length + (BS - 1)) / BS) * BS;
  // Following is meant for lengths < BS,
  // but results in a block being read even
  // if length is zero. This is wasteful,
  // but fixing it would break all existing pre-3.0 databases.
  if (BlockLength == 0 && BS == 8)
    BlockLength = BS;

  trashMemory(lengthblock, BS);

  if (length > 0 ||
      (BS == 8 && length == 0)) { // pre-3 pain
    unsigned char *tempcbc = block3;
    numRead += fread(b, 1, BlockLength, fp);
    for (unsigned int x = 0; x < BlockLength; x += BS) {
      memcpy(tempcbc, b + x, BS);
      Algorithm->Decrypt(b + x, b + x);
      xormem(b + x, cbcbuffer, BS);
      memcpy(cbcbuffer, tempcbc, BS);
    }
  }

  if (buffer_len == 0) {
    // delete[] buffer here since caller will see zero length
    delete[] buffer;
  }
  return numRead;
}

// typeless version for V4 content (caller pre-allocates buffer)
size_t _readcbc(FILE *fp, unsigned char *buffer,
                const size_t buffer_len, Fish *Algorithm,
                unsigned char *cbcbuffer)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  ASSERT((buffer_len % BS) == 0);
  size_t nread = 0;
  unsigned char *p = buffer;
  auto *tmpcbc = new unsigned char[BS];

  do {
    size_t nr = fread(p, 1, BS, fp);
    nread += nr;
    if (nr != BS)
      break;

    memcpy(tmpcbc, p, BS);
    Algorithm->Decrypt(p, p);
    xormem(p, cbcbuffer, BS);
    memcpy(cbcbuffer, tmpcbc, BS);

    p += nr;
  } while (nread < buffer_len);

  delete[] tmpcbc;
  return nread;
}

// PWSUtil implementations

void PWSUtil::strCopy(LPTSTR target, size_t tcount, const LPCTSTR source, size_t scount)
{
  UNREFERENCED_PARAMETER(tcount); //not used in now in non-MSVS wrapped version of _tcsncpy_s
  _tcsncpy_s(target, tcount, source, scount);
}

size_t PWSUtil::strLength(const LPCTSTR str)
{
  return _tcslen(str);
}

const TCHAR *PWSUtil::UNKNOWN_XML_TIME_STR = _T("1970-01-01 00:00:00");
const TCHAR *PWSUtil::UNKNOWN_ASC_TIME_STR = _T("Unknown");

StringX PWSUtil::ConvertToDateTimeString(const time_t &t, TMC result_format)
{
  StringX ret;
  if (t != 0) {
    TCHAR datetime_str[80];
    struct tm *st;
    struct tm st_s;
    errno_t err;
    err = localtime_s(&st_s, &t);  // secure version
    if (err != 0) // invalid time
      return ConvertToDateTimeString(0, result_format);
    st = &st_s; // hide difference between versions
    switch (result_format) {
    case TMC_EXPORT_IMPORT:
      _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
                _T("%Y/%m/%d %H:%M:%S"), st);
      break;
    case TMC_XML:
      _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
                _T("%Y-%m-%dT%H:%M:%S"), st);
      break;
    case TMC_LOCALE:
      setlocale(LC_TIME, "");
      _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
                _T("%c"), st);
      break;
    case TMC_LOCALE_DATE_ONLY:
      setlocale(LC_TIME, "");
      _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
                _T("%x"), st);
      break;
    default:
      if (_tasctime_s(datetime_str, 32, st) != 0)
        return ConvertToDateTimeString(0, result_format);
    }
    ret = datetime_str;
  } else { // t == 0
    switch (result_format) {
    case TMC_ASC_UNKNOWN:
      ret = UNKNOWN_ASC_TIME_STR;
      break;
    case TMC_XML:
      ret = UNKNOWN_XML_TIME_STR;
      break;
    default:
      ret = _T("");
    }
  }
  // remove the trailing EOL char.
  TrimRight(ret);
  return ret;
}

stringT PWSUtil::GetNewFileName(const stringT &oldfilename,
                                const stringT &newExtn)
{
  stringT inpath(oldfilename);
  stringT drive, dir, fname, ext;
  stringT outpath;

  if (pws_os::splitpath(inpath, drive, dir, fname, ext)) {
    outpath = pws_os::makepath(drive, dir, fname, newExtn);
  } else
    ASSERT(0);
  return outpath;
}

stringT PWSUtil::GetTimeStamp(const bool bShort)
{
  stringT sTimeStamp;
  GetTimeStamp(sTimeStamp, bShort);
  return sTimeStamp;
}

void PWSUtil::GetTimeStamp(stringT &sTimeStamp, const bool bShort)
{
  // Now re-entrant
  // Gets datetime stamp in format YYYY/MM/DD HH:MM:SS.mmm
  // If bShort == true, don't add milli-seconds

#ifdef _WIN32
  struct _timeb *ptimebuffer;
  ptimebuffer = new _timeb;
  _ftime_s(ptimebuffer);
  time_t the_time = ptimebuffer->time;
#else
  struct timeval *ptimebuffer;
  ptimebuffer = new timeval;
  gettimeofday(ptimebuffer, nullptr);
  time_t the_time = ptimebuffer->tv_sec;
#endif
  StringX cmys_now = ConvertToDateTimeString(the_time, TMC_EXPORT_IMPORT);

  if (bShort) {
    sTimeStamp = cmys_now.c_str();
  } else {
    ostringstreamT *p_os;
    p_os = new ostringstreamT;
    *p_os << cmys_now << TCHAR('.') << setw(3) << setfill(TCHAR('0'))
          << static_cast<unsigned int>(the_time);

    sTimeStamp = p_os->str();
    delete p_os;
  }
  delete ptimebuffer;
}

stringT PWSUtil::Base64Encode(const BYTE *strIn, size_t len)
{
  stringT cs_Out;
  static const TCHAR base64ABC[] =
    _S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

  for (size_t i = 0; i < len; i += 3) {
    long l = ( static_cast<long>(strIn[i]) << 16 ) |
               (((i + 1) < len) ? (static_cast<long>(strIn[i + 1]) << 8) : 0) |
               (((i + 2) < len) ? static_cast<long>(strIn[i + 2]) : 0);

    cs_Out += base64ABC[(l >> 18) & 0x3F];
    cs_Out += base64ABC[(l >> 12) & 0x3F];
    if (i + 1 < len) cs_Out += base64ABC[(l >> 6) & 0x3F];
    if (i + 2 < len) cs_Out += base64ABC[(l ) & 0x3F];
  }

  switch (len % 3) {
    case 1:
      cs_Out += TCHAR('=');
    case 2:
      cs_Out += TCHAR('=');
    default:
      break;
  }
  return cs_Out;
}

void PWSUtil::Base64Decode(const StringX &inString, BYTE * &outData, size_t &out_len)
{
  static const char szCS[]=
    "=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  int iDigits[4] = {0,0,0,0};

  size_t st_length = 0;
  const size_t in_length = inString.length();

  size_t i1, i2, i3;
  for (i2 = 0; i2 < in_length; i2 += 4) {
    iDigits[0] = iDigits[1] = iDigits[2] = iDigits[3] = -1;

    for (i1 = 0; i1 < sizeof(szCS) - 1; i1++) {
      for (i3 = i2; i3 < i2 + 4; i3++) {
        if (i3 < in_length &&  inString[i3] == szCS[i1])
          iDigits[i3 - i2] = reinterpret_cast<int &>(i1) - 1;
      }
    }

    outData[st_length] = (static_cast<BYTE>(iDigits[0]) << 2);

    if (iDigits[1] >= 0) {
      outData[st_length] += (static_cast<BYTE>(iDigits[1]) >> 4) & 0x3;
    }

    st_length++;

    if (iDigits[2] >= 0) {
      outData[st_length++] = ((static_cast<BYTE>(iDigits[1]) & 0x0f) << 4)
        | ((static_cast<BYTE>(iDigits[2]) >> 2) & 0x0f);
    }

    if (iDigits[3] >= 0) {
      outData[st_length++] = ((static_cast<BYTE>(iDigits[2]) & 0x03) << 6)
        | (static_cast<BYTE>(iDigits[3]) & 0x3f);
    }
  }

  out_len = st_length;
}

StringX PWSUtil::NormalizeTTT(const StringX &in, size_t maxlen)
{
  StringX ttt;
  if (in.length() >= maxlen) {
    ttt = in.substr(0, maxlen/2-6) +
      _T(" ... ") + in.substr(in.length() - maxlen/2);
  } else {
    ttt = in;
  }
  return ttt;
}

bool ValidateXMLCharacters(const StringX &value, ostringstream &ostInvalidPositions)
{
  // From: http://www.w3.org/TR/REC-xml/#sec-cdata-sect
  // CDATA Sections
  // [18]    CDSect   ::=    CDStart CData CDEnd
  // [19]    CDStart  ::=    '<![CDATA['
  // [20]    CData    ::=    (Char* - (Char* ']]>' Char*))
  // [21]    CDEnd    ::=    ']]>'

  // From: http://www.w3.org/TR/REC-xml/#NT-Char
  //  Char    ::=    #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] |
  //                 [#x10000-#x10FFFF]
  // any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.

  // Easy to check low values (0x00-0x1f excluding 3 above) and the 2 special values
  // Not so easy for the range 0xD800 to 0xDFFF (Unicode only) - could use regex
  // but expected to be slower than this!

  ostInvalidPositions.str("");
  bool bInvalidFound(false);
  for (size_t i = 0; i < value.length(); i++) {
    TCHAR current = value[i];
    if (!((current == 0x09) ||
          (current == 0x0A) ||
          (current == 0x0D) ||
          ((current >=    0x20) && (current <=   0xD7FF)) ||
          ((current >=  0xE000) && (current <=   0xFFFD)) ||
          ((current >= 0x10000) && (current <= 0x10FFFF)))) {
      if (bInvalidFound) {
        // Already 1 position, add a comma
        ostInvalidPositions << ", ";
      }
      bInvalidFound = true;
      ostInvalidPositions << (i + 1);
    }
  }
  return !bInvalidFound;
}

bool PWSUtil::WriteXMLField(ostream &os, const char *fname,
                            const StringX &value, CUTF8Conv &utf8conv,
                            const char *tabs)
{
  const unsigned char *utf8 = nullptr;
  size_t utf8Len = 0;
  ostringstream ostInvalidPositions;
  if (!ValidateXMLCharacters(value, ostInvalidPositions)) {
    os << tabs << "<!-- Field '<" << fname << ">' contains invalid XML character(s)" << endl;
    os << tabs << "   at position(s): " << ostInvalidPositions.str().c_str() << endl;
    os << tabs << "   and has been skipped -->" << endl;
    return false;
  }

  StringX::size_type p = value.find(_T("]]>")); // special handling required
  if (p == StringX::npos) {
    // common case
    os << tabs << "<" << fname << "><![CDATA[";
    if (utf8conv.ToUTF8(value, utf8, utf8Len))
      os.write(reinterpret_cast<const char *>(utf8), utf8Len);
    else
      os << "Internal error - unable to convert field to utf-8";
    os << "]]></" << fname << ">" << endl;
  } else {
    // value has "]]>" sequence(s) that need(s) to be escaped
    // Each "]]>" splits the field into two CDATA sections, one ending with
    // ']]', the other starting with '>'
    os << tabs << "<" << fname << ">";
    size_t from = 0, to = p + 2;
    do {
      StringX slice = value.substr(from, (to - from));
      os << "<![CDATA[";
      if (utf8conv.ToUTF8(slice, utf8, utf8Len))
        os.write(reinterpret_cast<const char *>(utf8), utf8Len);
      else
        os << "Internal error - unable to convert field to utf-8";
      os << "]]><![CDATA[";
      from = to;
      p = value.find(_T("]]>"), from); // are there more?
      if (p == StringX::npos) {
        to = value.length();
        slice = value.substr(from, (to - from));
      } else {
        to = p + 2;
        slice = value.substr(from, (to - from));
        from = to;
        to = value.length();
      }
      if (utf8conv.ToUTF8(slice, utf8, utf8Len))
        os.write(reinterpret_cast<const char *>(utf8), utf8Len);
      else
        os << "Internal error - unable to convert field to utf-8";
      os << "]]>";
    } while (p != StringX::npos);
    os << "</" << fname << ">" << endl;
  } // special handling of "]]>" in value.
  return true;
}

string PWSUtil::GetXMLTime(int indent, const char *name,
                           time_t t, CUTF8Conv &utf8conv)
{
  int i;
  const StringX tmp = PWSUtil::ConvertToDateTimeString(t, TMC_XML);
  ostringstream oss;
  const unsigned char *utf8 = nullptr;
  size_t utf8Len = 0;

  for (i = 0; i < indent; i++) oss << "\t";
  oss << "<" << name << ">" ;
  utf8conv.ToUTF8(tmp.substr(0, 10), utf8, utf8Len);
  oss.write(reinterpret_cast<const char *>(utf8), utf8Len);
  oss << "T";
  utf8conv.ToUTF8(tmp.substr(tmp.length() - 8), utf8, utf8Len);
  oss.write(reinterpret_cast<const char *>(utf8), utf8Len);
  oss << "</" << name << ">" << endl;
  return oss.str();
}

/**
 * Get TCHAR buffer size by format string with parameters
 * @param[in] fmt - format string
 * @param[in] args - arguments for format string
 * @return buffer size including nullptr-terminating character
*/
unsigned int GetStringBufSize(const TCHAR *fmt, va_list args)
{
  TCHAR *buffer=nullptr;

  unsigned int len = 0;

#ifdef _WIN32
  len = _vsctprintf(fmt, args) + 1;
#else
  va_list ar;
  va_copy(ar, args);
  // Linux doesn't do this correctly :-(
  unsigned int guess = 16;
  int nBytes = -1;
  while (true) {
    len = guess;
    buffer = new TCHAR[len];
    nBytes = _vstprintf_s(buffer, len, fmt, ar);
    va_end(ar);//after using args we should reset list
    va_copy(ar, args);
    if (nBytes++ > 0) {
      len = nBytes;
      break;
    } else { // too small, resize & try again
      delete[] buffer;
      buffer = nullptr;
      guess *= 2;
    }
  }
  va_end(ar);
#endif
  if (buffer)
    delete[] buffer;

  return len;
}

StringX PWSUtil::DeDupString(StringX &in_string)
{
  // Size of input string
  const size_t len = in_string.length();

  // Create output string
  StringX out_string;

  // It will never be longer than the input string
  out_string.reserve(len);
  const charT *c = in_string.c_str();

  // Cycle through characters - only appending if not already there
  for (size_t i = 0; i < len; i++) {
    if (out_string.find_first_of(c) == StringX::npos) {
      out_string.append(c, 1);
    }
    c++;
  }
  return out_string;
}

stringT PWSUtil::GetSafeXMLString(const StringX &sxInString)
{
  stringT retval(_T(""));
  ostringstreamT os;

  StringX::size_type p = sxInString.find(_T("]]>")); // special handling required
  if (p == StringX::npos) {
    // common case
    os << "<![CDATA[" << sxInString << "]]>";
  } else {
    // value has "]]>" sequence(s) that need(s) to be escaped
    // Each "]]>" splits the field into two CDATA sections, one ending with
    // ']]', the other starting with '>'
    const StringX value = sxInString;
    size_t from = 0, to = p + 2;
    do {
      StringX slice = value.substr(from, (to - from));
      os << "<![CDATA[" << slice << "]]><![CDATA[";
      from = to;
      p = value.find(_T("]]>"), from); // are there more?
      if (p == StringX::npos) {
        to = value.length();
        slice = value.substr(from, (to - from));
      } else {
        to = p + 2;
        slice = value.substr(from, (to - from));
        from = to;
        to = value.length();
      }
      os <<  slice << "]]>";
    } while (p != StringX::npos);
  }
  retval = os.str().c_str();
  return retval;
}

bool operator==(const std::string& str1, const stringT& str2)
{
    CUTF8Conv conv;
    StringX xstr;
    VERIFY( conv.FromUTF8( reinterpret_cast<const unsigned char*>(str1.data()), str1.size(), xstr) );
    return stringx2std(xstr) == str2;
}

bool PWSUtil::pull_time(time_t &t, const unsigned char *data, size_t len)
{
  // len can be either 4, 5 or 8...
  // len == 5 is new for V4
  ASSERT(len == 4 || len == 5 || len == 8);
  if (!(len == 4 || len == 5 || len == 8))
    return false;
  // sizeof(time_t) is either 4 or 8
  if (len == sizeof(time_t)) { // 4 == 4 or 8 == 8
    t = getInt<time_t>(data);
  } else if (len < sizeof(time_t)) { // 4 < 8 or 5 < 8
    unsigned char buf[sizeof(time_t)] = {0};
    memcpy(buf, data, len);
    t = getInt<time_t>(buf);
  } else { // convert from 40 or 64 bit time to 32 bit
    unsigned char buf[sizeof(__time64_t)] = {0};
    memcpy(buf, data, len); // not needed if len == 8, but no harm
    struct tm ts;
    const auto t64 = getInt<__time64_t>(buf);
    if (_localtime64_s(&ts, &t64) != 0) {
      ASSERT(0); return false;
    }
    t = _mktime32(&ts);
    if (t == time_t(-1)) { // time is past 2038!
      t = 0; return false;
    }
  }
  return true;
}

bool FindNoCase( const StringX& src, const StringX& dest)
{
    StringX srcLower = src;
    ToLower(srcLower);

    StringX destLower = dest;
    ToLower(destLower);

    return destLower.find(srcLower) != StringX::npos;
}

std::string toutf8(const std::wstring &w)
{
  CUTF8Conv conv;
  const unsigned char *utf8str = nullptr;
  size_t length = 0;
  if (conv.ToUTF8(std2stringx(w), utf8str, length))
    return string{ reinterpret_cast<const char *>(utf8str), length};
  return string{};
}

bool PWSUtil::loadFile(const StringX &filename, StringXStream &stream) {
  // We need to use FOpen as the file name/file path may contain non-Latin
  // characters
  FILE *fs = pws_os::FOpen(filename.c_str(), _T("rb"));
  if (fs == nullptr)
    return false;

  // when using wifstream, each byte will be converted to wchar_t, but we need to load
  // each n-bytes as one wchar, so read in whole file and put it in a StringXStream
  const size_t BUFFER_SIZE = 1024;

  wchar_t buffer[BUFFER_SIZE];
  bool bError(false);
  while(!feof(fs)) {
    auto count = fread(buffer, sizeof(wchar_t), BUFFER_SIZE, fs);
    if (ferror(fs)) {
      bError = true;
      break;
    }
    stream.write(buffer, count);
  }

  // Close the file
  fclose(fs);

  return !bError;
}
