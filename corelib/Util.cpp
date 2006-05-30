/// \file Util.cpp
//-----------------------------------------------------------------------------

#include "sha1.h"
#include "BlowFish.h"
#include "PWSrand.h"
#include "PwsPlatform.h"

#include <stdio.h>

#include "Util.h"


// used by CBC routines...
static void
xormem(unsigned char* mem1, const unsigned char* mem2, int length)
{
  for (int x=0;x<length;x++)
    mem1[x] ^= mem2[x];
}



//-----------------------------------------------------------------------------
/*
  Note: A bunch of the encryption-related routines may not be Unicode
  compliant.  Which really isn't a huge problem, since they are actually
  using MFC routines anyway
*/

//-----------------------------------------------------------------------------
//Overwrite the memory

#pragma optimize("",off)
void
trashMemory(void* buffer, long length)
{
  ASSERT(buffer != NULL);
  // {kjp} no point in looping around doing nothing is there?
  if ( length != 0 ) {
    const int numiter = 30;
    for (int x=0; x<numiter; x++) {
      memset(buffer, 0x00, length);
      memset(buffer, 0xFF, length);
      memset(buffer, 0x00, length);
    }
  }
}
#pragma optimize("",on)

void
trashMemory( LPTSTR buffer, long length )
{
  trashMemory( (unsigned char *) buffer, length * sizeof(buffer[0])  );
}

/**
   Burn some stack memory
   @param len amount of stack to burn in bytes
*/
void burnStack(unsigned long len)
{
  unsigned char buf[32];
  trashMemory(buf, sizeof(buf));
  if (len > (unsigned long)sizeof(buf))
    burnStack(len - sizeof(buf));
}

//Generates a passkey-based hash from stuff - used to validate the passkey
void
GenRandhash(const CMyString &a_passkey,
            const unsigned char* a_randstuff,
            unsigned char* a_randhash)
{
  const LPCTSTR pkey = (const LPCTSTR)a_passkey;
  /*
    I'm not quite sure what this is doing, so as I figure out each piece,
    I'll add more comments {jpr}
  */

  /*
    tempSalt <- H(a_randstuff + a_passkey)
  */
  SHA1 keyHash;
  keyHash.Update(a_randstuff, StuffSize);
  keyHash.Update((const unsigned char*)pkey, a_passkey.GetLength());

  unsigned char tempSalt[20]; // HashSize
  keyHash.Final(tempSalt);

  /*
    tempbuf <- a_randstuff encrypted 1000 times using tempSalt as key?
  */
	
  BlowFish Cipher(tempSalt, sizeof(tempSalt));
	
  unsigned char tempbuf[StuffSize];
  memcpy((char*)tempbuf, (char*)a_randstuff, StuffSize);

  for (int x=0; x<1000; x++)
    Cipher.Encrypt(tempbuf, tempbuf);

  /*
    hmm - seems we're not done with this context
    we throw the tempbuf into the hasher, and extract a_randhash
  */
  keyHash.Update(tempbuf, StuffSize);
  keyHash.Final(a_randhash);
}

int
_writecbc(FILE *fp, const unsigned char* buffer, int length, unsigned char type,
          Fish *Algorithm, unsigned char* cbcbuffer)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  int numWritten = 0;

  // some trickery to avoid new/delete
  unsigned char block1[16];

  unsigned char *curblock = NULL;
  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...

  // First encrypt and write the length of the buffer
  curblock = block1;
  // Fill unused bytes of length with random data, to make
  // a dictionary attack harder
  PWSrand::GetInstance()->GetRandomData(curblock, BS);
  // block length overwrites 4 bytes of the above randomness.
  putInt32(curblock, length);

  // following new for format 2.0 - lengthblock bytes 4-7 were unused before.
  curblock[sizeof(length)] = type;

  if (BS == 16) {
    // In this case, we've too many (11) wasted bytes in the length block
    // So we store actual data there:
    // (11 = BlockSize - 4 (length) - 1 (type)
    const int len1 = (length > 11) ? 11 : length;
    memcpy(curblock+5, buffer, len1);
    length -= len1;
    buffer += len1;
  }

  xormem(curblock, cbcbuffer, BS); // do the CBC thing
  Algorithm->Encrypt(curblock, curblock);
  memcpy(cbcbuffer, curblock, BS); // update CBC for next round

  numWritten = fwrite(curblock, 1, BS, fp);

  if (length > 0 ||
      (BS == 8 && length == 0)) { // This part for bwd compat w/pre-3 format
    unsigned int BlockLength = ((length+(BS-1))/BS)*BS;
    if (BlockLength == 0 && BS == 8)
      BlockLength = BS;

    // Now, encrypt and write the (rest of the) buffer
    for (unsigned int x=0; x<BlockLength; x+=BS) {
      if ((length == 0) || ((length%BS != 0) && (length-x<BS))) {
        //This is for an uneven last block
        PWSrand::GetInstance()->GetRandomData(curblock, BS);
        memcpy(curblock, buffer+x, length % BS);
      } else
        memcpy(curblock, buffer+x, BS);
      xormem(curblock, cbcbuffer, BS);
      Algorithm->Encrypt(curblock, curblock);
      memcpy(cbcbuffer, curblock, BS);
      numWritten += fwrite(curblock, 1, BS, fp);
    }
  }
  trashMemory(curblock, BS);
  return numWritten;
}

/*
 * Reads an encrypted record into buffer.
 * The first block of the record contains the encrypted record length
 * We have the usual ugly problem of fixed buffer lengths in C/C++.
 * Don't want to use CStrings, for future portability.
 * Best solution would be STL strings, but not enough experience with them (yet)
 * So for the meantime, we're going to allocate the buffer here, to ensure that it's long
 * enough.
 * *** THE CALLER MUST delete[] IT AFTER USE *** UGH++
 *
 * (unless buffer_len is zero)
 *
 * An alternative to STL strings would be to accept a buffer, and allocate a replacement
 * iff the buffer is too small. This is serious ugliness, but should be considered if
 * the new/delete performance hit is too big.
 *
 * If TERMINAL_BLOCK is non-NULL, the first block read is tested against it,
 * and -1 is returned if it matches. (used in V3)
 */
int
_readcbc(FILE *fp,
         unsigned char* &buffer, unsigned int &buffer_len, unsigned char &type,
         Fish *Algorithm, unsigned char* cbcbuffer,
         const unsigned char *TERMINAL_BLOCK)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  unsigned int numRead = 0;
  
  // some trickery to avoid new/delete
  unsigned char block1[16];
  unsigned char block2[16];
  unsigned char block3[16];
  unsigned char *lengthblock = NULL;

  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...
  lengthblock = block1;

  buffer_len = 0;
  numRead = fread(lengthblock, 1, BS, fp);
  if (numRead != BS) {
    return 0;
  }

  if (TERMINAL_BLOCK != NULL &&
      memcmp(lengthblock, TERMINAL_BLOCK, BS) == 0)
    return -1;

  unsigned char *lcpy = block2;
  memcpy(lcpy, lengthblock, BS);

  Algorithm->Decrypt(lengthblock, lengthblock);
  xormem(lengthblock, cbcbuffer, BS);
  memcpy(cbcbuffer, lcpy, BS);

  int length = getInt32(lengthblock);

  // new for 2.0 -- lengthblock[4..7] previously set to zero
  type = lengthblock[sizeof(int)]; // type is first byte after the length

  if (length < 0) { // sanity check
    TRACE("_readcbc: Read negative length - aborting\n");
    buffer = NULL;
    buffer_len = 0;
    trashMemory(lengthblock, BS);
    return 0;
  }

  buffer_len = length;
  buffer = new unsigned char[(length/BS)*BS +2*BS]; // round upwards
  unsigned char *b = buffer;

  if (BS == 16) {
    // length block contains up to 11 (= 16 - 4 - 1) bytes
    // of data
    const int len1 = (length > 11) ? 11 : length;
    memcpy(b, lengthblock+5, len1);
    length -= len1;
    b += len1;
  }

  unsigned int BlockLength = ((length+(BS-1))/BS)*BS;
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
    for (unsigned int x=0; x<BlockLength; x+=BS) {
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

// PWSUtil implementations

void PWSUtil::strCopy(LPTSTR target, size_t tcount, const LPCTSTR source, size_t scount)
{
#if (_MSC_VER >= 1400)
  (void) _tcsncpy_s(target, tcount, source, scount);
#else
  tcount; // shut up warning;
  (void)_tcsncpy(target, source, scount);
#endif
}

size_t PWSUtil::strLength(const LPCTSTR str)
{
  return _tcslen(str);
}

/**
 * Returns the current length of a file.
 */
long PWSUtil::fileLength(FILE *fp)
{
  long	pos;
  long	len;

  pos = ftell( fp );
  fseek( fp, 0, SEEK_END );
  len	= ftell( fp );
  fseek( fp, pos, SEEK_SET );

  return len;
}
bool
PWSUtil::VerifyImportDateTimeString(const CString time_str, time_t &t)
{
  //  String format must be "yyyy/mm/dd hh:mm:ss"
  //                        "0123456789012345678"

  CString xtime_str;
  const int month_lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const int idigits[14] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18};
  const int ndigits = 14;
  int yyyy, mon, dd, hh, min, ss, nscanned;

  t = (time_t)-1;

  if (time_str.GetLength() != 19)
    return false;

  // Validate time_str
  if (time_str.Mid(4,1) != '/' ||
      time_str.Mid(7,1) != '/' ||
      time_str.Mid(10,1) != ' ' ||
      time_str.Mid(13,1) != ':' ||
      time_str.Mid(16,1) != ':')
    return false;

  for (int i = 0;  i < ndigits; i++)
    if (!isdigit(time_str.GetAt(idigits[i])))
      return false;

  // Since white space is ignored with sscanf, first verify that there are no invalid '#' characters
  // Then take copy of the string and replace all blanks by '#' (should only be 1)
  if (time_str.Find('#') != (-1))
    return false;

  xtime_str = time_str;
  if (xtime_str.Replace(' ', '#') != 1)
    return false;

#if _MSC_VER >= 1400
  nscanned = sscanf_s(xtime_str, "%4d/%2d/%2d#%2d:%2d:%2d",
                      &yyyy, &mon, &dd, &hh, &min, &ss);
#else
  nscanned = sscanf(xtime_str, "%4d/%2d/%2d#%2d:%2d:%2d",
                    &yyyy, &mon, &dd, &hh, &min, &ss);
#endif

  if (nscanned != 6)
    return false;

  // Built-in obsolesence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon != 2 && (yyyy % 4) == 0) {
    if(dd > month_lengths[mon - 1])
      return false;
  } else { // Feb of a leap-year
    if (dd > 29)
      return false;
  }

  if ((hh < 0 || hh > 23) ||
      (min < 0 || min > 59) ||
      (ss < 0 || ss > 59))
    return false;

  const CTime ct(yyyy, mon, dd, hh, min, ss, -1);

  t = (time_t)ct.GetTime();

  return true;
}

bool
PWSUtil::VerifyASCDateTimeString(const CString time_str, time_t &t)
{
  //  String format must be "ddd MMM dd hh:mm:ss yyyy"
  //                        "012345678901234567890123"

  const int month_lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const CString str_months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const CString str_days = "SunMonTueWedThuFriSat";
  CString xtime_str;
  char cmonth[4], cdayofweek[4];
  const int idigits[12] = {8, 9, 11, 12, 14, 15, 17, 18, 20, 21, 22, 23};
  const int ndigits = 12;
  int iMON, iDOW, nscanned;
  int yyyy, mon, dd, hh, min, ss;

  cmonth[3] = cdayofweek[3] = '\0';

  t = (time_t)-1;

  if (time_str.GetLength() != 24)
    return false;

  // Validate time_str
  if (time_str.Mid(13,1) != ':' ||
      time_str.Mid(16,1) != ':')
    return false;

  for (int i = 0; i < ndigits; i++)
    if (!isdigit(time_str.GetAt(idigits[i])))
      return false;

  // Since white space is ignored with sscanf, first verify that there are no invalid '#' characters
  // Then take copy of the string and replace all blanks by '#' (should be 4)
  if (time_str.Find('#') != (-1))
    return false;

  xtime_str = time_str;
  if (xtime_str.Replace(' ', '#') != 4)
    return false;

#if _MSC_VER >= 1400
  nscanned = sscanf_s(xtime_str, "%3c#%3c#%2d#%2d:%2d:%2d#%4d",
                      cdayofweek, sizeof(cdayofweek), cmonth, sizeof(cmonth), &dd, &hh, &min, &ss, &yyyy);
#else
  nscanned = sscanf(xtime_str, "%3c#%3c#%2d#%2d:%2d:%2d#%4d",
                    cdayofweek, cmonth, &dd, &hh, &min, &ss, &yyyy);
#endif

  if (nscanned != 7)
    return false;

  iMON = str_months.Find(cmonth);
  if (iMON < 0)
    return false;

  mon = (iMON / 3) + 1;

  // Built-in obsolesence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon != 2 && (yyyy % 4) == 0) {
    if(dd > month_lengths[mon - 1])
      return false;
  } else { // Feb of a leap-year
    if (dd > 29)
      return false;
  }

  if ((hh < 0 || hh > 23) ||
      (min < 0 || min > 59) ||
      (ss < 0 || ss > 59))
    return false;

  const CTime ct(yyyy, mon, dd, hh, min, ss, -1);

  iDOW = str_days.Find(cdayofweek);
  if (iDOW < 0)
    return false;

  iDOW = (iDOW / 3) + 1;
  if (iDOW != ct.GetDayOfWeek())
    return false;

  t = (time_t)ct.GetTime();

  return true;
}

bool
PWSUtil::ToClipboard(const CMyString &data,
                     unsigned char clipboard_digest[SHA256::HASHLEN],
                     HWND hWindow)
{
  unsigned int uGlobalMemSize;
  HGLOBAL hGlobalMemory;
  bool b_retval;

  uGlobalMemSize = (data.GetLength() + 1) * sizeof(TCHAR);
  hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, uGlobalMemSize);
  LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);

  strCopy(pGlobalLock, uGlobalMemSize, data ,data.GetLength());

  GlobalUnlock(hGlobalMemory);
  b_retval = false;

  if (OpenClipboard(hWindow) == TRUE) {
    if (EmptyClipboard() != TRUE) {
      TRACE("The clipboard was not emptied correctly");
    }
    if (SetClipboardData(CLIPBOARD_TEXT_FORMAT, hGlobalMemory) == NULL) {
      TRACE("The data was not pasted into the clipboard correctly");
      GlobalFree(hGlobalMemory); // wasn't passed to Clipboard
    } else {
      // identify data in clipboard as ours, so as not to clear the wrong data later
      // of course, we don't want an extra copy of a password floating around
      // in memory, so we'll use the hash
      const char *str = (const char *)data;
      SHA256 ctx;
      ctx.Update((const unsigned char *)str, data.GetLength());
      ctx.Final(clipboard_digest);
      b_retval = true;
    }
    if (CloseClipboard() != TRUE) {
      TRACE("The clipboard could not be closed");
    }
  } else {
    TRACE("The clipboard could not be opened correctly");
    GlobalFree(hGlobalMemory); // wasn't passed to Clipboard
  }
  return b_retval;
}

bool
PWSUtil::ClearClipboard(unsigned char clipboard_digest[SHA256::HASHLEN],
                        HWND hWindow)
{
  bool b_retval;
  b_retval = true;

  if (OpenClipboard(hWindow) != TRUE) {
    TRACE("The clipboard could not be opened correctly");
    return b_retval;
  }

  if (IsClipboardFormatAvailable(CLIPBOARD_TEXT_FORMAT) != 0) {
    HGLOBAL hglb = GetClipboardData(CLIPBOARD_TEXT_FORMAT);
    if (hglb != NULL) {
      LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
      if (lptstr != NULL) {
        // check identity of data in clipboard
        unsigned char digest[SHA256::HASHLEN];
        SHA256 ctx;
        ctx.Update((const unsigned char *)lptstr, PWSUtil::strLength(lptstr));
        ctx.Final(digest);
        if (memcmp(digest, clipboard_digest, SHA256::HASHLEN) == 0) {
          trashMemory( lptstr, strLength(lptstr));
          GlobalUnlock(hglb);
          if (EmptyClipboard() == TRUE) {
            memset(clipboard_digest, '\0', SHA256::HASHLEN);
            b_retval = false;
          } else {
            TRACE("The clipboard was not emptied correctly");
          }
        } else { // hashes match
          GlobalUnlock(hglb);
        }
      } // lptstr != NULL
    } // hglb != NULL
  } // IsClipboardFormatAvailable
  if (CloseClipboard() != TRUE) {
    TRACE("The clipboard could not be closed");
  }
  return b_retval;
}

CMyString
PWSUtil::ConvertToDateTimeString(const time_t &t, const int result_format)
{
  CMyString ret;
  if (t != 0) {
		char time_str[32];
#if _MSC_VER >= 1400
		struct tm st;
    	localtime_s(&st, &t);  // secure version
    	if ((result_format & EXPORT_IMPORT) == EXPORT_IMPORT)
      		sprintf_s(time_str, 20, "%04d/%02d/%02d %02d:%02d:%02d",
            		    st.tm_year+1900, st.tm_mon+1, st.tm_mday, st.tm_hour,
                		st.tm_min, st.tm_sec);
    	else
      		_tasctime_s(time_str, 32, &st);  // secure version
    	ret = time_str;
#else
    	char *t_str_ptr;
		struct tm *st;
    	st = localtime(&t);
    	if ((result_format & EXPORT_IMPORT) == EXPORT_IMPORT) {
      		sprintf(time_str, "%04d/%02d/%02d %02d:%02d:%02d",
            	  st->tm_year+1900, st->tm_mon+1, st->tm_mday,
            	  st->tm_hour, st->tm_min, st->tm_sec);
      	t_str_ptr = time_str;
    	} else
      		t_str_ptr = _tasctime(st);
    	ret = t_str_ptr;
#endif
  } else {
  	if ((result_format & ASC_UNKNOWN) == ASC_UNKNOWN)
    	ret = _T("Unknown");
    else
    	ret = _T("");
  }
  // remove the trailing EOL char.
  ret.TrimRight();
  return ret;
}
