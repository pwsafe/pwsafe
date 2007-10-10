/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "PWSfileV3.h"
#include "UUIDGen.h"
#include "PWSrand.h"
#include "util.h"
#include "SysInfo.h"
#include "PWScore.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

static unsigned char TERMINAL_BLOCK[TwoFish::BLOCKSIZE] = {
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F',
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F'};

PWSfileV3::PWSfileV3(const CMyString &filename, RWmode mode, VERSION version)
  : PWSfile(filename, mode)
{
  m_curversion = version;
  m_IV = m_ipthing;
  m_terminal = TERMINAL_BLOCK;
}

PWSfileV3::~PWSfileV3()
{
}

int PWSfileV3::Open(const CMyString &passkey)
{
  int status = SUCCESS;

  ASSERT(m_curversion == V30);
  ASSERT(!passkey.IsEmpty());

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
                             const CMyString &passkey, FILE *a_fd,
                             unsigned char *aPtag, int *nITER)
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

  ASSERT(N >= MIN_HASH_ITERATIONS);
  if (N < MIN_HASH_ITERATIONS) {
    retval = FAILURE;
    goto err;
  }

  if (nITER != NULL)
    *nITER = N;

  unsigned char Ptag[SHA256::HASHLEN];
  if (aPtag == NULL)
    aPtag = Ptag;

  StretchKey(salt, sizeof(salt), passkey, N, aPtag);

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

size_t PWSfileV3::WriteCBC(unsigned char type, const CString &data)
{
  bool status;
  const unsigned char *utf8;
  int utf8Len;
  status = m_utf8conv.ToUTF8(data, utf8, utf8Len);
  if (!status)
    TRACE(_T("ToUTF8(%s) failed\n"), data);
  return WriteCBC(type, utf8, utf8Len);
}

size_t PWSfileV3::WriteCBC(unsigned char type, const unsigned char *data,
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
  uuid_array_t item_uuid;

  item.GetUUID(item_uuid);
  WriteCBC(CItemData::UUID, item_uuid, sizeof(uuid_array_t));
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
  int t32;
  item.GetCTime(t);
  if (t != 0) {
    t32 = (int)t;
    WriteCBC(CItemData::CTIME, (unsigned char *)&t32, sizeof(t32));
  }
  item.GetPMTime(t);
  if (t != 0) {
    t32 = (int)t;
    WriteCBC(CItemData::PMTIME, (unsigned char *)&t32, sizeof(t32));
  }
  item.GetATime(t);
  if (t != 0) {
    t32 = (int)t;
    WriteCBC(CItemData::ATIME, (unsigned char *)&t32, sizeof(t32));
  }
  item.GetLTime(t);
  if (t != 0) {
    t32 = (int)t;
    WriteCBC(CItemData::LTIME, (unsigned char *)&t32, sizeof(t32));
  }
  item.GetRMTime(t);
  if (t != 0) {
    t32 = (int)t;
    WriteCBC(CItemData::RMTIME, (unsigned char *)&t32, sizeof(t32));
  }
  tmp = item.GetPWHistory();
  if (!tmp.IsEmpty())
    WriteCBC(CItemData::PWHIST, tmp);

  UnknownFieldsConstIter vi_IterURFE;
  for (vi_IterURFE = item.GetURFIterBegin();
       vi_IterURFE != item.GetURFIterEnd();
       vi_IterURFE++) {
    unsigned char type;
    unsigned int length = 0;
    unsigned char *pdata = NULL;
    item.GetUnknownField(type, length, pdata, vi_IterURFE);
    WriteCBC(type, pdata, length);
    trashMemory(pdata, length);
    delete[] pdata;
  }

  WriteCBC(CItemData::END, _T(""));

  return status;
}

size_t PWSfileV3::ReadCBC(unsigned char &type, unsigned char* &data,
                       unsigned int &length)
{
  size_t numRead = PWSfile::ReadCBC(type, data, length);

  if (numRead > 0) {
    m_hmac.Update(data, length);
  }

  return numRead;
}

int PWSfileV3::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);

  int status = SUCCESS;

  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen; // <= 0 means end of file reached

  do {
    unsigned char *utf8 = NULL;
    int utf8Len = 0;
    fieldLen = static_cast<signed long>(ReadCBC(type, utf8,
                                                (unsigned int &)utf8Len));

    if (fieldLen > 0) {
      numread += fieldLen;
      if (!item.SetField(type, utf8, utf8Len)) {
        status = FAILURE;
        break;
      }
    } // if (fieldLen > 0)

    if (utf8 != NULL) {
      trashMemory(utf8, utf8Len * sizeof(utf8[0]));
      delete[] utf8; utf8 = NULL; utf8Len = 0;
    }
  } while (type != CItemData::END && fieldLen > 0 && --emergencyExit > 0);
  if (numread > 0)
    return status;
  else
    return END_OF_FILE;
}

void PWSfileV3::StretchKey(const unsigned char *salt, unsigned long saltLen,
                           const CMyString &passkey,
                           unsigned int N, unsigned char *Ptag)
{
  /*
   * P' is the "stretched key" of the user's passphrase and the SALT, as defined
   * by the hash-function-based key stretching algorithm in
   * http://www.schneier.com/paper-low-entropy.pdf (Section 4.1), with SHA-256
   * as the hash function, and N iterations.
   */
  int passLen = 0;
  unsigned char *pstr = NULL;

  ConvertString(passkey, pstr, passLen);
  unsigned char *X = Ptag;
  SHA256 H0;
  H0.Update(pstr, passLen);
  H0.Update(salt, saltLen);
  H0.Final(X);

#ifdef UNICODE
  trashMemory(pstr, passLen);
  delete[] pstr;
#endif

  ASSERT(N >= MIN_HASH_ITERATIONS); // minimal value we're willing to use
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

const short VersionNum = 0x0302;

int PWSfileV3::WriteHeader()
{
  int status = SUCCESS;

  // See formatV3.txt for explanation of what's written here and why
  unsigned int NumHashIters;
  if (m_hdr.m_nITER < MIN_HASH_ITERATIONS)
    NumHashIters = MIN_HASH_ITERATIONS;
  else
    NumHashIters = m_hdr.m_nITER;

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

  unsigned char Nb[sizeof(NumHashIters)];
  
  putInt32(Nb, NumHashIters);
  fwrite(Nb, 1, sizeof(Nb), m_fd);

  unsigned char Ptag[SHA256::HASHLEN];

  StretchKey(salt, sizeof(salt), m_passkey, NumHashIters, Ptag);

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
  size_t numWritten = 0;
  // Write version number
  unsigned char vnb[sizeof(VersionNum)];;
  vnb[0] = (unsigned char) (VersionNum & 0xff);
  vnb[1] = (unsigned char) ((VersionNum & 0xff00) >> 8);
  m_hdr.m_nCurrentMajorVersion = (unsigned short) ((VersionNum & 0xff00) >> 8);
  m_hdr.m_nCurrentMinorVersion = (unsigned short) (VersionNum & 0xff);

  numWritten = WriteCBC(HDR_VERSION, vnb, sizeof(VersionNum));
 
  if (numWritten <= 0) { status = FAILURE; goto end; }
 
  // Write UUID
  uuid_array_t file_uuid_array;
  memset(file_uuid_array, 0x00, sizeof(file_uuid_array));
  // If not there or zeroed, create new
  if (memcmp(m_hdr.m_file_uuid_array,
             file_uuid_array, sizeof(file_uuid_array)) == 0) {
    CUUIDGen uuid;
    uuid.GetUUID(m_hdr.m_file_uuid_array);
  }
  
  numWritten = WriteCBC(HDR_UUID, m_hdr.m_file_uuid_array,
                        sizeof(m_hdr.m_file_uuid_array));
  if (numWritten <= 0) { status = FAILURE; goto end; }

  // Write (non default) user preferences
  numWritten = WriteCBC(HDR_NDPREFS, m_hdr.m_prefString);
  if (numWritten <= 0) { status = FAILURE; goto end; }

  // Write out display status
  if (!m_hdr.m_displaystatus.empty()) {
    CString ds(_T(""));
    vector<bool>::const_iterator iter;
    for (iter = m_hdr.m_displaystatus.begin();
         iter != m_hdr.m_displaystatus.end(); iter++)
      ds += (*iter) ? _T("1") : _T("0");
  	numWritten = WriteCBC(HDR_DISPSTAT, ds);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  // Write out time of this update
  time_t time_now;
  time(&time_now);
#if 0  // BUGBUGBUG
  CString cs_update_time;
  cs_update_time.Format(_T("%08x"), time_now);
  numWritten = WriteCBC(HDR_LASTUPDATETIME, cs_update_time);
#endif
  numWritten = WriteCBC(HDR_LASTUPDATETIME,
                        (unsigned char *)&time_now, sizeof(time_now));
  if (numWritten <= 0) { status = FAILURE; goto end; }
  m_hdr.m_whenlastsaved = time_now;

  // Write out who saved it!
  {
    const SysInfo *si = SysInfo::GetInstance();
    CString user = si->GetRealUser();
    CString sysname = si->GetRealHost();
    // Following DEPRECATED in format 0x0302
#if 0
    CString cs_who;
    cs_who.Format(_T("%04x%s%s"), m_user.GetLength(), m_user, m_sysname);
    numWritten = WriteCBC(HDR_LASTUPDATEUSERHOST, cs_who);
#endif
    numWritten = WriteCBC(HDR_LASTUPDATEUSER, user);
    if (numWritten > 0)
      numWritten = WriteCBC(HDR_LASTUPDATEHOST, sysname);
    if (numWritten <= 0) { status = FAILURE; goto end; }
    m_hdr.m_lastsavedby = user;
    m_hdr.m_lastsavedon = sysname;
  }

  // Write out what saved it!
  numWritten = WriteCBC(HDR_LASTUPDATEAPPLICATION,
                        m_hdr.m_whatlastsaved);
  if (numWritten <= 0) { status = FAILURE; goto end; }

  if (!m_hdr.m_dbname.IsEmpty()) {
    numWritten = WriteCBC(HDR_DBNAME, m_hdr.m_dbname);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }
  if (!m_hdr.m_dbdesc.IsEmpty()) {
    numWritten = WriteCBC(HDR_DBDESC, m_hdr.m_dbdesc);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  if (!m_UHFL.empty()) {
    UnknownFieldList::iterator vi_IterUHFE;
    for (vi_IterUHFE = m_UHFL.begin();
         vi_IterUHFE != m_UHFL.end();
         vi_IterUHFE++) {
      UnknownFieldEntry &unkhfe = *vi_IterUHFE;
      numWritten = WriteCBC(unkhfe.uc_Type,
                            unkhfe.uc_pUField, (unsigned int)unkhfe.st_length);
      if (numWritten <= 0) { status = FAILURE; goto end; }
    }
  }

  // Write zero-length end-of-record type item
  WriteCBC(HDR_END, NULL, 0);
 end:
  if (status != SUCCESS)
    Close();
  return status;
}

int PWSfileV3::ReadHeader()
{
  unsigned char Ptag[SHA256::HASHLEN];
  int status = CheckPassword(m_filename, m_passkey, m_fd,
                             Ptag, &m_hdr.m_nITER);

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

  unsigned char fieldType;
  CMyString text;
  size_t numRead;
  bool utf8status;
  unsigned char *utf8 = NULL;
  int utf8Len = 0;
  bool found0302UserHost = false; // to resolve potential conflicts

  do {
    numRead = ReadCBC(fieldType, utf8, (unsigned int &)utf8Len);

    if (numRead < 0) {
      Close();
      return FAILURE;
    }

    switch (fieldType) {
    case HDR_VERSION: /* version */
      // in Beta, VersionNum was an int (4 bytes) instead of short (2)
      // This hack keeps bwd compatability.
      if (utf8Len != sizeof(VersionNum) &&
          utf8Len != sizeof(int)) {
        delete[] utf8;
        Close();
        return FAILURE;
      }
      if (utf8[1] !=
          (unsigned char)((VersionNum & 0xff00) >> 8)) {
        //major version mismatch
        delete[] utf8;
        Close();
        return UNSUPPORTED_VERSION;
      }
      // for now we assume that minor version changes will
      // be backward-compatible
      m_hdr.m_nCurrentMajorVersion = (unsigned short)utf8[1];
      m_hdr.m_nCurrentMinorVersion = (unsigned short)utf8[0];
      break;

    case HDR_UUID: /* UUID */
      if (utf8Len != sizeof(uuid_array_t)) {
        delete[] utf8;
        Close();
        return FAILURE;
      }
      memcpy(m_hdr.m_file_uuid_array, utf8,
             sizeof(m_hdr.m_file_uuid_array));
      break;

    case HDR_NDPREFS: /* Non-default user preferences */
      if (utf8Len != 0) {
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len,
                                         m_hdr.m_prefString);
        if (!utf8status)
          TRACE(_T("FromUTF8(m_prefString) failed\n"));
      } else
        m_hdr.m_prefString = _T("");
      break;

    case HDR_DISPSTAT: /* Tree Display Status */
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      for (int i = 0; i != text.GetLength(); i++) {
        const TCHAR v = text.GetAt(i);
        m_hdr.m_displaystatus.push_back(v == TCHAR('1'));
      }
      if (!utf8status)
        TRACE(_T("FromUTF8(m_displaystatus) failed\n"));
      break;

    case HDR_LASTUPDATETIME: /* When last saved */
      if (utf8Len == 8) {
        // Handle pre-3.09 implementations that mistakenly
        // stored this as a hex value
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        if (!utf8status)
          TRACE(_T("FromUTF8(m_whenlastsaved) failed\n"));
#if _MSC_VER >= 1400
        _stscanf_s(text, _T("%8x"), &m_hdr.m_whenlastsaved);
#else
        _stscanf(text, _T("%8x"), &m_hdr.m_whenlastsaved);
#endif
      } else if (utf8Len == 4) {
        // retrieve time_t
        m_hdr.m_whenlastsaved = *reinterpret_cast<time_t*>(utf8);
      } else {
        m_hdr.m_whenlastsaved = 0;
      }
      break;

    case HDR_LASTUPDATEUSERHOST: /* and by whom */
        // DEPRECATED, but we still know how to read this
      if (!found0302UserHost) { // if new fields also found, don't overwrite
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        if (utf8status) {
          int ulen = 0;
#if _MSC_VER >= 1400
          _stscanf_s(text, _T("%4x"), &ulen);
#else
          _stscanf(text, _T("%4x"), &ulen);
#endif
          m_hdr.m_lastsavedby = CString(text.Mid(4, ulen));
          m_hdr.m_lastsavedon = CString(text.Mid(ulen + 4));
        } else
          TRACE(_T("FromUTF8(m_wholastsaved) failed\n"));
      }
      break;

    case HDR_LASTUPDATEAPPLICATION: /* and by what */
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      m_hdr.m_whatlastsaved = CString(text);
      if (!utf8status)
        TRACE(_T("FromUTF8(m_whatlastsaved) failed\n"));
      break;

    case HDR_LASTUPDATEUSER:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      found0302UserHost = true; // so HDR_LASTUPDATEUSERHOST won't override
      m_hdr.m_lastsavedby = CString(text);
      break;
    case HDR_LASTUPDATEHOST:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      found0302UserHost = true; // so HDR_LASTUPDATEUSERHOST won't override
      m_hdr.m_lastsavedon = CString(text);
      break;
    case HDR_DBNAME:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      m_hdr.m_dbname = CString(text);
      break;
    case HDR_DBDESC:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      m_hdr.m_dbdesc = CString(text);
      break;
    case HDR_END: /* process END so not to treat it as 'unknown' */
      break;

    default:
      // Save unknown fields that may be addded by future versions
      UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);
      m_UHFL.push_back(unkhfe);
#ifdef _DEBUG
      CString cs_timestamp;
      cs_timestamp = PWSUtil::GetTimeStamp();
      TRACE(_T("%s: Header has unknown field: %02x, length %d/0x%04x, value:\n"), 
            cs_timestamp, fieldType, utf8Len, utf8Len);
      PWSUtil::HexDump(utf8, utf8Len, cs_timestamp);
#endif /* DEBUG */
      break;
    }
    delete[] utf8; utf8 = NULL; utf8Len = 0;
  } while (fieldType != HDR_END);

  return SUCCESS;
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
    fclose(fd);

    if (memcmp(tag, V3TAG, sizeof(tag)) == 0) {
      v = V30;
      return true;
    } else {
      v = UNKNOWN_VERSION;
      return false;
    }
  }
