/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "PWSfileV3.h"
#include "UUIDGen.h"
#include "PWSrand.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

static unsigned char TERMINAL_BLOCK[TwoFish::BLOCKSIZE] = {
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F',
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F'};

PWSfileV3::PWSfileV3(const CMyString &filename, RWmode mode, VERSION version)
  : PWSfile(filename,mode), m_utf8(NULL), m_utf8Len(0), m_utf8MaxLen(0),
    m_wc(NULL), m_wcMaxLen(0), m_tmp(NULL), m_tmpMaxLen(0)
{
  m_curversion = version;
  m_IV = m_ipthing;
  m_terminal = TERMINAL_BLOCK;
#ifdef NOTDEF // defined for troubleshooting only
  // determine UTF8 support here for now
  // we assume that 95, 98 and NT 3.51 do not support it
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  BOOL osvstat = GetVersionEx (&osvi);
  ASSERT(osvstat == TRUE);

  bool oldOS = (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) // 95
    || (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) // 98
    || (osvi.dwMajorVersion == 3 && osvi.dwMinorVersion == 51); // NT 3.51
  m_useUTF8 = !oldOS;
#endif
}

PWSfileV3::~PWSfileV3()
{
  if (m_utf8 != NULL) {
    trashMemory(m_utf8, m_utf8MaxLen* sizeof(m_utf8[0]));
    delete[] m_utf8;
  }
  if (m_wc != NULL) {
    trashMemory(m_wc, m_wcMaxLen*sizeof(m_wc[0]));
    delete[] m_wc;
  }
  if (m_tmp != NULL) {
    trashMemory(m_tmp, m_tmpMaxLen*sizeof(m_tmp[0]));
    delete[] m_tmp;
  }
}

int PWSfileV3::Open(const CMyString &passkey)
{
  int status = SUCCESS;

  ASSERT(m_curversion == V30);

  m_passkey = passkey;

  FOpen();
  if (m_fd == NULL)
    return CANT_OPEN_FILE;

  if (m_rw == Write) {
    status = WriteHeader();
  } else { // open for read
    status = ReadHeader();
    if (status != SUCCESS) {
      Close();
      return status;
    }
  }
  return status;
}

int PWSfileV3::Close()
{
  if (m_fd == NULL)
    return SUCCESS; // idempotent
  if (m_utf8 != NULL) {
    trashMemory(m_utf8, m_utf8MaxLen* sizeof(m_utf8[0]));
    delete[] m_utf8; m_utf8 = NULL;
    m_utf8Len = m_utf8MaxLen = 0;
  }
  if (m_wc != NULL) {
    trashMemory(m_wc, m_wcMaxLen*sizeof(m_wc[0]));
    delete[] m_wc; m_wc = NULL; m_wcMaxLen = 0;
  }
  if (m_tmp != NULL) {
    trashMemory(m_tmp, m_tmpMaxLen*sizeof(m_tmp[0]));
    delete[] m_tmp; m_tmp = NULL; m_tmpMaxLen = 0;
  }

  unsigned char digest[HMAC_SHA256::HASHLEN];
  m_hmac.Final(digest);

  // Write or verify HMAC, depending on RWmode.
  if (m_rw == Write) {
    fwrite(TERMINAL_BLOCK, sizeof(TERMINAL_BLOCK), 1, m_fd);
    fwrite(digest, sizeof(digest), 1, m_fd);
    return PWSfile::Close();
  } else { // Read
    // We're here *after* TERMINAL_BLOCK has been read
    // and detected (by _readcbc) - just read hmac & verify
    unsigned char d[HMAC_SHA256::HASHLEN];
    fread(d, sizeof(d), 1, m_fd);
    if (memcmp(d, digest, HMAC_SHA256::HASHLEN) == 0)
      return PWSfile::Close();
    else {
      PWSfile::Close();
      return BAD_DIGEST;
    }
  }
}

const char V3TAG[4] = {'P','W','S','3'}; // ASCII chars, not wchar

int PWSfileV3::CheckPassword(const CMyString &filename,
                             const CMyString &passkey,
                             FILE *a_fd, unsigned char *aPtag)
{
  FILE *fd = a_fd;
  int retval = SUCCESS;
  SHA256 H;

  if (fd == NULL) {
#if _MSC_VER >= 1400
    _tfopen_s(&fd, (LPCTSTR) filename, _T("rb"));
#else
    fd = _tfopen((LPCTSTR) filename, _T("rb"));
#endif
  }
  if (fd == NULL)
    return CANT_OPEN_FILE;

  char tag[sizeof(V3TAG)];
  fread(tag, 1, sizeof(tag), fd);
  if (memcmp(tag, V3TAG, sizeof(tag)) != 0) {
    retval = NOT_PWS3_FILE;
    goto err;
  }

  unsigned char salt[SaltLengthV3];
  fread(salt, 1, sizeof(salt), fd);

  unsigned char Nb[sizeof(unsigned int)];;
  fread(Nb, 1, sizeof(Nb), fd);
  const unsigned int N = getInt32(Nb);

  ASSERT(N >= 2048);
  if (N < 2048) {
    retval = FAILURE;
    goto err;
  }
  unsigned char Ptag[SHA256::HASHLEN];
  if (aPtag == NULL)
    aPtag = Ptag;
  LPCTSTR passstr = LPCTSTR(passkey); 
  StretchKey(salt, sizeof(salt),
             (const unsigned char *)passstr, passkey.GetLength(),
             N, aPtag);


  unsigned char HPtag[SHA256::HASHLEN];
  H.Update(aPtag, SHA256::HASHLEN);
  H.Final(HPtag);
  unsigned char readHPtag[SHA256::HASHLEN];
  fread(readHPtag, 1, sizeof(readHPtag), fd);
  if (memcmp(readHPtag, HPtag, sizeof(readHPtag)) != 0) {
    retval = WRONG_PASSWORD;
    goto err;
  }
 err:
  if (a_fd == NULL) // if we opened the file, we close it...
    fclose(fd);
  return retval;
}

int PWSfileV3::WriteCBC(unsigned char type, const CString &data)
{
  if (m_useUTF8) {
    bool status;
    status = ToUTF8(data);
    if (!status)
      TRACE(_T("ToUTF8(%s) failed\n"), data);
    return WriteCBC(type, m_utf8, m_utf8Len);
  } else {
    const LPCSTR str = (const LPCSTR)data;
    return WriteCBC(type,(const unsigned char *)str, data.GetLength());
  }
}

int PWSfileV3::WriteCBC(unsigned char type, const unsigned char *data,
                        unsigned int length)
{
  m_hmac.Update(data, length);
  return PWSfile::WriteCBC(type, data, length);
}


int PWSfileV3::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);
  int status = SUCCESS;
  CMyString tmp;
  uuid_array_t uuid_array;
  item.GetUUID(uuid_array);
  WriteCBC(CItemData::UUID, uuid_array, sizeof(uuid_array));
  tmp = item.GetGroup();
  if (!tmp.IsEmpty())
    WriteCBC(CItemData::GROUP, tmp);
  WriteCBC(CItemData::TITLE, item.GetTitle());
  WriteCBC(CItemData::USER, item.GetUser());
  WriteCBC(CItemData::PASSWORD, item.GetPassword());
  tmp = item.GetNotes();
  if (!tmp.IsEmpty())
    WriteCBC(CItemData::NOTES, tmp);
  tmp = item.GetURL();
  if (!tmp.IsEmpty())
    WriteCBC(CItemData::URL, tmp);
  tmp = item.GetAutoType();
  if (!tmp.IsEmpty())
    WriteCBC(CItemData::AUTOTYPE, tmp);
  time_t t = 0;
  item.GetCTime(t);
  if (t != 0)
    WriteCBC(CItemData::CTIME, (unsigned char *)&t, sizeof(t));
  item.GetPMTime(t);
  if (t != 0)
    WriteCBC(CItemData::PMTIME, (unsigned char *)&t, sizeof(t));
  item.GetATime(t);
  if (t != 0)
    WriteCBC(CItemData::ATIME, (unsigned char *)&t, sizeof(t));
  item.GetLTime(t);
  if (t != 0)
    WriteCBC(CItemData::LTIME, (unsigned char *)&t, sizeof(t));
  item.GetRMTime(t);
  if (t != 0)
    WriteCBC(CItemData::RMTIME, (unsigned char *)&t, sizeof(t));
  tmp = item.GetPWHistory();
  if (!tmp.IsEmpty())
    WriteCBC(CItemData::PWHIST, tmp);
  WriteCBC(CItemData::END, _T(""));

  return status;
}

int
PWSfileV3::ReadCBC(unsigned char &type, CMyString &data)
{
  CMyString text;
  int numRead = PWSfile::ReadCBC(type, text);

  if (numRead > 0) {
    LPCTSTR d = LPCTSTR(text);
    m_hmac.Update((const unsigned char *)d, text.GetLength());
    // HACK - following types non-utf-8
    if (type == CItemData::UUID ||
        type == CItemData::CTIME || type == CItemData::PMTIME ||
        type == CItemData::ATIME || type == CItemData::LTIME ||
        type == CItemData::RMTIME ||
        !m_useUTF8 // E.g., win98 doesn't grok utf8
        ) {
      data = text;
      return numRead;
    }
    bool status;
    m_utf8 = (unsigned char *)d;
    m_utf8Len = text.GetLength();
    status = FromUTF8(data);
    m_utf8 = NULL; m_utf8Len = 0; // so we don't double delete
    if (!status)
      TRACE(_T("FromUTF8(%s) failed\n"), text);
  }

  return numRead;
}

int PWSfileV3::ReadCBC(unsigned char &type, unsigned char *data,
                       unsigned int &length)
{
  int numRead = PWSfile::ReadCBC(type, data, length);

  if (numRead > 0) {
    m_hmac.Update(data, length);
  }

  return numRead;
}

int PWSfileV3::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);

  CMyString tempdata;  
  int numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  int fieldLen; // <= 0 means end of file reached
  bool endFound = false; // set to true when record end detected - happy end
  time_t t;

  do {
    fieldLen = ReadCBC(type, tempdata);
    if (fieldLen > 0) {
      numread += fieldLen;
      switch (type) {
      case CItemData::TITLE:
        item.SetTitle(tempdata); break;
      case CItemData::USER:
        item.SetUser(tempdata); break;
      case CItemData::PASSWORD:
        item.SetPassword(tempdata); break;
      case CItemData::NOTES:
        item.SetNotes(tempdata); break;
      case CItemData::END:
        endFound = true; break;
      case CItemData::UUID: {
        LPCTSTR ptrU = LPCTSTR(tempdata);
        uuid_array_t uuid_array;
        for (int i = 0; i < sizeof(uuid_array); i++)
          uuid_array[i] = (unsigned char)ptrU[i];
        item.SetUUID(uuid_array); break;
      }
      case CItemData::GROUP:
        item.SetGroup(tempdata); break;
      case CItemData::URL:
        item.SetURL(tempdata); break;
      case CItemData::AUTOTYPE:
        item.SetAutoType(tempdata); break;
      case CItemData::CTIME: {
		LPCTSTR ptrC = LPCTSTR(tempdata);
        memcpy(&t, ptrC, sizeof(t));
		item.SetCTime(t); break;
		}
	  case CItemData::PMTIME: {
        LPCTSTR ptrPM = LPCTSTR(tempdata);
		memcpy(&t, ptrPM, sizeof(t));
		item.SetPMTime(t); break;
      }
	  case CItemData::ATIME: {
        LPCTSTR ptrA = LPCTSTR(tempdata);
		memcpy(&t, ptrA, sizeof(t));
        item.SetATime(t); break;
		}
	  case CItemData::LTIME: {
        LPCTSTR ptrL = LPCTSTR(tempdata);
		memcpy(&t, ptrL, sizeof(t));
        item.SetLTime(t); break;
		}
	  case CItemData::RMTIME: {
        LPCTSTR ptrRM = LPCTSTR(tempdata);
		memcpy(&t, ptrRM, sizeof(t));
        item.SetRMTime(t); break;
		}
	  case CItemData::PWHIST: {
        item.SetPWHistory(tempdata); break;
		}
      // just silently ignore fields we don't support.
      // this is forward compatability...
      case CItemData::POLICY:
      default:
        // XXX Set a flag here so user can be warned that
        // XXX we read a file format we don't fully support
        break;
      } // switch
    } // if (fieldLen > 0)
  } while (!endFound && fieldLen > 0 && --emergencyExit > 0);
  if (numread > 0)
    return SUCCESS;
  else
    return END_OF_FILE;
}

void PWSfileV3::StretchKey(const unsigned char *salt, unsigned long saltLen,
                         const unsigned char *passkey, unsigned long passLen,
                         unsigned int N, unsigned char *Ptag)
{
  /*
   * P' is the "stretched key" of the user's passphrase and the SALT, as defined
   * by the hash-function-based key stretching algorithm in
   * http://www.schneier.com/paper-low-entropy.pdf (Section 4.1), with SHA-256
   * as the hash function, and N iterations.
   */
  unsigned char *X = Ptag;
  SHA256 H0;
  H0.Update(passkey, passLen);
  H0.Update(salt, saltLen);
  H0.Final(X);

  ASSERT(N >= 2048); // minimal value we're willing to use
  for (unsigned int i = 0; i < N; i++) {
    SHA256 H;
    // The 2nd param in next line was sizeof(X) in Beta-1
    // (bug #1451422). This change broke the ability to read beta-1
    // generated databases. If this is really needed, we should
    // hack the read functionality to try both variants (ugh).
    H.Update(X, SHA256::HASHLEN);
    H.Final(X);
  }
}

const short VersionNum = 0x0301;

int PWSfileV3::WriteHeader()
{
  // See formatV3.txt for explanation of what's written here and why
  const unsigned int NumHashIters = 2048; // At least 2048

  fwrite(V3TAG, 1, sizeof(V3TAG), m_fd);

  // According to the spec, salt is just random data. I don't think though,
  // that it's good practice to directly expose the generated randomness
  // to the attacker. Therefore, we'll hash the salt.
  // The following takes shameless advantage of the fact that
  // SaltLengthV3 == SHA256::HASHLEN
  ASSERT(SaltLengthV3 == SHA256::HASHLEN); // if false, have to recode
  unsigned char salt[SaltLengthV3];
  PWSrand::GetInstance()->GetRandomData(salt, sizeof(salt));
  SHA256 salter;
  salter.Update(salt, sizeof(salt));
  salter.Final(salt);
  fwrite(salt, 1, sizeof(salt), m_fd);

  unsigned char Nb[sizeof(NumHashIters)];;
  putInt32(Nb, NumHashIters);
  fwrite(Nb, 1, sizeof(Nb), m_fd);

  unsigned char Ptag[SHA256::HASHLEN];
  LPCTSTR passstr = LPCTSTR(m_passkey); 
  StretchKey(salt, sizeof(salt),
             (const unsigned char *)passstr, m_passkey.GetLength(),
             NumHashIters, Ptag);

  unsigned char HPtag[SHA256::HASHLEN];
  SHA256 H;
  H.Update(Ptag, sizeof(Ptag));
  H.Final(HPtag);
  fwrite(HPtag, 1, sizeof(HPtag), m_fd);

  PWSrand::GetInstance()->GetRandomData(m_key, sizeof(m_key));
  unsigned char B1B2[sizeof(m_key)];
  ASSERT(sizeof(B1B2) == 32); // Generalize later
  TwoFish TF(Ptag, sizeof(Ptag));
  TF.Encrypt(m_key, B1B2);
  TF.Encrypt(m_key + 16, B1B2 + 16);
  fwrite(B1B2, 1, sizeof(B1B2), m_fd);

  unsigned char L[32]; // for HMAC
  PWSrand::GetInstance()->GetRandomData(L, sizeof(L));
  unsigned char B3B4[sizeof(L)];
  ASSERT(sizeof(B3B4) == 32); // Generalize later
  TF.Encrypt(L, B3B4);
  TF.Encrypt(L + 16, B3B4 + 16);
  fwrite(B3B4, 1, sizeof(B3B4), m_fd);

  m_hmac.Init(L, sizeof(L));
  
  // See discussion on Salt to understand why we hash
  // random data instead of writing it directly
  unsigned char ip_rand[SHA256::HASHLEN];
  PWSrand::GetInstance()->GetRandomData(ip_rand, sizeof(ip_rand));
  SHA256 ipHash;
  ipHash.Update(ip_rand, sizeof(ip_rand));
  ipHash.Final(ip_rand);
  ASSERT(sizeof(ip_rand) >= sizeof(m_ipthing)); // compilation assumption
  memcpy(m_ipthing, ip_rand, sizeof(m_ipthing));
  
  fwrite(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  m_fish = new TwoFish(m_key, sizeof(m_key));

  // write some actual data (at last!)
  int numWritten = 0;
  // Write version number
  unsigned char vnb[sizeof(VersionNum)];;
  vnb[0] = (unsigned char) (VersionNum & 0xff);
  vnb[1] = (unsigned char) ((VersionNum & 0xff00) >> 8);
  m_nCurrentMajorVersion = (unsigned short) ((VersionNum & 0xff00) >> 8);
  m_nCurrentMinorVersion = (unsigned short) (VersionNum & 0xff);

  numWritten = WriteCBC(HDR_VERSION, vnb, sizeof(VersionNum));
  
  // Write UUID
  // We should probably reuse the UUID when saving an existing
  // database, and generate a new one only from new dbs.
  // For now, this is Good Enough. XXX
  uuid_array_t uuid_array;
  CUUIDGen uuid;
  
  uuid.GetUUID(uuid_array);
  
  numWritten += WriteCBC(HDR_UUID, uuid_array, sizeof(uuid_array));

  if (numWritten <= 0) { // WriteCBC writes at least 2 blocks per datum.
    Close();
    return FAILURE;
  }

  // Write (non default) user preferences
  numWritten = WriteCBC(HDR_NDPREFS, m_prefString);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  }

  // Write out display status
  if (!m_file_displaystatus.IsEmpty()) {
  	numWritten = WriteCBC(HDR_DISPSTAT, m_file_displaystatus);
    if (numWritten <= 0) {
      Close();
      return FAILURE;
    }
  }

  // Write out time of this update
  time_t time_now;
  time(&time_now);
  CString cs_update_time;
  cs_update_time.Format("%08x", time_now);
  numWritten = WriteCBC(HDR_LASTUPDATETIME, cs_update_time);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  } else {
    m_whenlastsaved = cs_update_time;
  }

  // Write out who saved it!
  CString cs_who;
  cs_who.Format("%04x%s%s", m_user.GetLength(), m_user, m_sysname);
  numWritten = WriteCBC(HDR_LASTUPDATEUSERHOST, cs_who);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  } else {
    m_wholastsaved = cs_who;
  }

  // Write out what saved it!
  // First get the application Version number (NOT the file format version)
  int nMajor = HIWORD(m_dwMajorMinor);
  int nMinor = LOWORD(m_dwMajorMinor);
  CString cs_what;
  cs_what.Format("%s V%d.%02d", AfxGetAppName(), nMajor, nMinor);
  numWritten = WriteCBC(HDR_LASTUPDATEAPPLICATION, cs_what);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  } else {
    m_whatlastsaved = cs_what;
  }

  // Write zero-length end-of-record type item
  // for future-proof (skip possible additional fields in read)
  WriteCBC(HDR_END, NULL, 0);

  return SUCCESS;
}

int PWSfileV3::ReadHeader()
{
  unsigned char Ptag[SHA256::HASHLEN];
  int status = CheckPassword(m_filename, m_passkey, m_fd, Ptag);

  if (status != SUCCESS) {
    Close();
    return status;
  }

  unsigned char B1B2[sizeof(m_key)];
  ASSERT(sizeof(B1B2) == 32); // Generalize later
  fread(B1B2, 1, sizeof(B1B2), m_fd);
  TwoFish TF(Ptag, sizeof(Ptag));
  TF.Decrypt(B1B2, m_key);
  TF.Decrypt(B1B2 + 16, m_key + 16);

  unsigned char L[32]; // for HMAC
  unsigned char B3B4[sizeof(L)];
  ASSERT(sizeof(B3B4) == 32); // Generalize later
  fread(B3B4, 1, sizeof(B3B4), m_fd);
  TF.Decrypt(B3B4, L);
  TF.Decrypt(B3B4 + 16, L + 16);

  m_hmac.Init(L, sizeof(L));

  fread(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  m_fish = new TwoFish(m_key, sizeof(m_key));

  LPCTSTR headerData;
  CMyString headerField;
  unsigned char type;
  int numRead;

  do {
    numRead = PWSfile::ReadCBC (type, headerField);
    headerData = LPCTSTR (headerField);
    m_hmac.Update ((const unsigned char *) headerData, headerField.GetLength());

    if (numRead < 0) {
      Close ();
      return FAILURE;
    }

    switch (type) {
    case HDR_VERSION: /* version */
      {
        // in Beta, VersionNum was an int (4 bytes) instead of short (2)
        // This hack keeps bwd compatability.

        int vlen = headerField.GetLength ();
        if (vlen != sizeof(VersionNum) && vlen != sizeof(int)) {
          Close();
          return FAILURE;
        }
        if ((headerData[1]) != (unsigned char)((VersionNum & 0xff00) >> 8)) {
          //major version mismatch
          Close();
          return UNSUPPORTED_VERSION;
        }
        // for now we assume that minor version changes will
        // be backward-compatible
        m_nCurrentMajorVersion = (unsigned short)headerData[1];
        m_nCurrentMinorVersion = (unsigned short)headerData[0];
      }
      break;

    case HDR_UUID: /* UUID */
      {
        // Read UUID XXX should save into data member
        int ulen = headerField.GetLength ();
        if (ulen != sizeof(uuid_array_t)) {
          Close();
          return FAILURE;
        }
      }
      break;

    case HDR_NDPREFS: /* Non-default user preferences */
      {
        m_utf8 = (unsigned char *) headerData;
        m_utf8Len = headerField.GetLength();
        if (!FromUTF8 (headerField)) {
          Close ();
          return FAILURE;
        }
        m_prefString = headerField;
        m_utf8 = NULL;
        m_utf8Len = 0;
      }
      break;

	case HDR_DISPSTAT: /* Tree Display Status */
	  {
        m_utf8 = (unsigned char *) headerData;
        m_utf8Len = headerField.GetLength();
        if (!FromUTF8 (headerField)) {
          Close ();
          return FAILURE;
        }
        m_file_displaystatus = (CString)headerField;
        m_utf8 = NULL;
        m_utf8Len = 0;
	  }
	  break;

	  	case HDR_LASTUPDATETIME: /* When last saved */
	  {
        m_utf8 = (unsigned char *) headerData;
        m_utf8Len = headerField.GetLength();
        if (!FromUTF8 (headerField)) {
          Close ();
          return FAILURE;
        }
        m_whenlastsaved = (CString)headerField;
        m_utf8 = NULL;
        m_utf8Len = 0;
	  }
	  break;

	  	case HDR_LASTUPDATEUSERHOST: /* and by whom */
	  {
        m_utf8 = (unsigned char *) headerData;
        m_utf8Len = headerField.GetLength();
        if (!FromUTF8 (headerField)) {
          Close ();
          return FAILURE;
        }
        m_wholastsaved = (CString)headerField;
        m_utf8 = NULL;
        m_utf8Len = 0;
	  }
	  break;

	  	case HDR_LASTUPDATEAPPLICATION: /* and by what */
	  {
        m_utf8 = (unsigned char *) headerData;
        m_utf8Len = headerField.GetLength();
        if (!FromUTF8 (headerField)) {
          Close ();
          return FAILURE;
        }
        m_whatlastsaved = (CString)headerField;
        m_utf8 = NULL;
        m_utf8Len = 0;
	  }
	  break;

    default:
      // ignore fields that may be addded by future versions
      break;
    }
  }
  while (type != HDR_END);

  return SUCCESS;
}

bool PWSfileV3::ToUTF8(const CString &data)
{
  // If we're not in Unicode, call MultiByteToWideChar to get from
  // current codepage to Unicode, and then WideCharToMultiByte to
  // get to UTF-8 encoding.
  // Output is in m_utf8 & m_utf8Len

  if (data.IsEmpty()) {
    m_utf8Len = 0;
    return true;
  }
  wchar_t *wcPtr; // to hide UNICODE differences
  int wcLen; // number of wide chars needed
#ifndef UNICODE
  // first get needed wide char buffer size
  wcLen = MultiByteToWideChar(CP_THREAD_ACP,      // code page
                              0,                  // character-type options
                              LPCSTR(data),       // string to map
                              data.GetLength(),   // number of bytes in string
                              NULL, 0);           // get needed buffer size
  if (wcLen == 0) { // uh-oh
    m_utf8Len = 0;
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (wcLen > m_wcMaxLen) {
    if (m_wc != NULL)
      trashMemory(m_wc, m_wcMaxLen * sizeof(m_wc[0]));
    delete[] m_wc;
    m_wc = new wchar_t[wcLen];
    m_wcMaxLen = wcLen;
  }
  // next translate to buffer
  wcLen = MultiByteToWideChar(CP_THREAD_ACP,      // code page
                              0,                  // character-type options
                              LPCSTR(data),       // string to map
                              data.GetLength(),   // number of bytes in string
                              m_wc, wcLen);       // output buffer
  ASSERT(wcLen != 0);
  wcPtr = m_wc;
#else
  wcPtr = LPCTSTR(data);
  wcLen = data.GetLength();
#endif
  // first get needed utf8 buffer size
  int mbLen = WideCharToMultiByte(CP_UTF8,       // code page
                                  0,             // performance and mapping flags
                                  wcPtr,        // wide-character string
                                  wcLen,         // number of chars in string
                                  NULL, 0,       // get needed buffer size
                                  NULL,NULL);    // use system default for unmappables

  if (mbLen == 0) { // uh-oh
    m_utf8Len = 0;
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (mbLen > m_utf8MaxLen) {
    if (m_utf8 != NULL)
      trashMemory(m_utf8, m_utf8MaxLen);
    delete[] m_utf8;
    m_utf8 = new unsigned char[mbLen];
    m_utf8MaxLen = mbLen;
  }
  // Finally get result
  m_utf8Len = WideCharToMultiByte(CP_UTF8,      // code page
                                  0,            // performance and mapping flags
                                  wcPtr, wcLen, // wide-character string
                                  LPSTR(m_utf8), mbLen,// buffer and length
                                  NULL,NULL);   // use system default for unmappables
  ASSERT(m_utf8Len != 0);
  return true;
}

bool PWSfileV3::FromUTF8(CMyString &data)
{
  // Call MultiByteToWideChar to get from UTF-8 to Unicode.
  // If we're not in Unicode, call WideCharToMultiByte to
  // get to current codepage.
  // Input is in m_utf8 & m_utf8Len

  if (m_utf8Len == 0) {
    data = _T("");
    return true;
  }
  // first get needed wide char buffer size
  int wcLen = MultiByteToWideChar(CP_UTF8,      // code page
                                  0,            // character-type options
                                  LPSTR(m_utf8), // string to map
                                  m_utf8Len,    // number of bytes in string
                                  NULL, 0);     // get needed buffer size
  if (wcLen == 0) { // uh-oh
    data = _T("");
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (wcLen > m_wcMaxLen) {
    if (m_wc != NULL)
      trashMemory(m_wc, m_wcMaxLen * sizeof(m_wc[0]));
    delete[] m_wc;
    m_wc = new wchar_t[wcLen+1];
    m_wcMaxLen = wcLen;
  }
  // next translate to buffer
  wcLen = MultiByteToWideChar(CP_UTF8,      // code page
                              0,                  // character-type options
                              LPSTR(m_utf8),       // string to map
                              m_utf8Len,   // number of bytes in string
                              m_wc, wcLen);       // output buffer
  ASSERT(wcLen != 0);
#ifdef UNICODE
  m_wc[wcLen] = TCHAR('\0');
  data = m_wc;
#else /* Go from Unicode to Locale encoding */
      // first get needed utf8 buffer size
  int mbLen = WideCharToMultiByte(CP_THREAD_ACP, // code page
                                  0,             // performance and mapping flags
                                  m_wc,        // wide-character string
                                  wcLen,         // number of chars in string
                                  NULL, 0,       // get needed buffer size
                                  NULL,NULL);    // use system default for unmappables

  if (mbLen == 0) { // uh-oh
    data = _T("");
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (mbLen > m_tmpMaxLen) {
    if (m_tmp != NULL)
      trashMemory(m_tmp, m_utf8MaxLen);
    delete[] m_tmp;
    m_tmp = new unsigned char[mbLen+1];
    m_tmpMaxLen = mbLen;
  }
  // Finally get result
  int tmpLen = WideCharToMultiByte(CP_THREAD_ACP,      // code page
                                   0,            // performance and mapping flags
                                   m_wc, wcLen, // wide-character string
                                   LPSTR(m_tmp), mbLen,// buffer and length
                                   NULL,NULL);   // use system default for unmappables
  ASSERT(tmpLen == mbLen);
  m_tmp[mbLen] = TCHAR('\0');
  data = m_tmp;
#endif /* !UNICODE */
  ASSERT(data.GetLength() != 0);
  return true;
}


bool PWSfileV3::IsV3x(const CMyString &filename, VERSION &v)
{
  // This is written so as to support V30, V31, V3x...

  ASSERT(FileExists(filename));
  FILE *fd;
#if _MSC_VER >= 1400
  _tfopen_s(&fd, (LPCTSTR) filename, _T("rb"));
#else
  fd = _tfopen((LPCTSTR) filename, _T("rb") );
#endif

  ASSERT(fd != NULL);
  char tag[sizeof(V3TAG)];
  fread(tag, 1, sizeof(tag), fd);
  if (memcmp(tag, V3TAG, sizeof(tag)) == 0) {
    v = V30;
    return true;
  } else {
    v = UNKNOWN_VERSION;
    return false;
  }
}
