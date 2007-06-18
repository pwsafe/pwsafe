/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "PWSfileV3.h"
#include "UUIDGen.h"
#include "PWSrand.h"
#include "util.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

static unsigned char TERMINAL_BLOCK[TwoFish::BLOCKSIZE] = {
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F',
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F'};

PWSfileV3::PWSfileV3(const CMyString &filename, RWmode mode, VERSION version)
  : PWSfile(filename, mode), m_utf8(NULL), m_utf8Len(0), m_utf8MaxLen(0),
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
  m_useUTF8 = true;
}

PWSfileV3::~PWSfileV3()
{
  if (m_utf8 != NULL) {
    trashMemory(m_utf8, m_utf8MaxLen * sizeof(m_utf8[0]));
    delete[] m_utf8;
  }
  if (m_wc != NULL) {
    trashMemory(m_wc, m_wcMaxLen);
    delete[] m_wc;
  }
  if (m_tmp != NULL) {
    trashMemory(m_tmp, m_tmpMaxLen * sizeof(m_tmp[0]));
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
        trashMemory(m_utf8, m_utf8MaxLen * sizeof(m_utf8[0]));
        delete[] m_utf8; m_utf8 = NULL;
        m_utf8Len = m_utf8MaxLen = 0;
    }
    if (m_wc != NULL) {
        trashMemory(m_wc, m_wcMaxLen);
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
  if (m_useUTF8) {
    bool status;
    status = ToUTF8(data);
    if (!status)
      TRACE(_T("ToUTF8(%s) failed\n"), data);
    return WriteCBC(type, m_utf8, m_utf8Len);
  } else {
    const LPCTSTR str = (const LPCTSTR)data;
    return WriteCBC(type,(const unsigned char *)str, data.GetLength());
  }
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

    CMyString tempdata;  
    signed long numread = 0;
    unsigned char type;

    int emergencyExit = 255; // to avoid endless loop.
    signed long fieldLen; // <= 0 means end of file reached
    bool endFound = false; // set to true when record end detected - happy end
    time_t t;

    do {
        fieldLen = static_cast<signed long>(ReadCBC(type, m_utf8,
                                                    (unsigned int &)m_utf8Len));
        if (m_utf8 != NULL) m_utf8[m_utf8Len] = '\0';

        if (CItemData::IsTextField(type)) {
            bool utf8status = FromUTF8(tempdata);
            if (!utf8status) {
                TRACE(_T("PWSfileV3::ReadRecord(): FromUTF failed!\n"));
                status = FAILURE;
            }
        }

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
                    uuid_array_t uuid_array;
                    ASSERT(m_utf8Len == sizeof(uuid_array));
                    for (unsigned i = 0; i < sizeof(uuid_array); i++)
                        uuid_array[i] = m_utf8[i];
                    item.SetUUID(uuid_array); break;
                }
                case CItemData::GROUP:
                    item.SetGroup(tempdata); break;
                case CItemData::URL:
                    item.SetURL(tempdata); break;
                case CItemData::AUTOTYPE:
                    item.SetAutoType(tempdata); break;
                case CItemData::CTIME:
                    ASSERT(m_utf8Len == sizeof(t));
                    memcpy(&t, m_utf8, sizeof(t));
                    item.SetCTime(t); break;
                case CItemData::PMTIME:
                    ASSERT(m_utf8Len == sizeof(t));
                    memcpy(&t, m_utf8, sizeof(t));
                    item.SetPMTime(t); break;
                case CItemData::ATIME:
                    ASSERT(m_utf8Len == sizeof(t));
                    memcpy(&t, m_utf8, sizeof(t));
                    item.SetATime(t); break;
                case CItemData::LTIME:
                    ASSERT(m_utf8Len == sizeof(t));
                    memcpy(&t, m_utf8, sizeof(t));
                    item.SetLTime(t); break;
                case CItemData::RMTIME:
                    ASSERT(m_utf8Len == sizeof(t));
                    memcpy(&t, m_utf8, sizeof(t));
                    item.SetRMTime(t); break;
                case CItemData::PWHIST:
                    item.SetPWHistory(tempdata); break;

                case CItemData::POLICY:
                default:
                    // just silently save fields we don't support.
                    item.SetUnknownField(type, m_utf8Len, m_utf8);
#ifdef DEBUG
                    CString cs_timestamp;
                    cs_timestamp = PWSUtil::GetTimeStamp();
                    TRACE(_T("%s: Record %s, %s, %s has unknown field: %02x, length %d/0x%04x, value:\n"),
                               cs_timestamp, item.GetGroup(), item.GetTitle(), item.GetUser(), 
                               type, m_utf8Len, m_utf8Len);
                    PWSUtil::HexDump(m_utf8, (int)m_utf8Len, cs_timestamp);
#endif /* DEBUG */
                    break;
            } // switch
        } // if (fieldLen > 0)
        if (m_utf8 != NULL) {
            trashMemory(m_utf8, m_utf8Len * sizeof(m_utf8[0]));
            delete[] m_utf8; m_utf8 = NULL; m_utf8Len = 0;
        }
    } while (!endFound && fieldLen > 0 && --emergencyExit > 0);
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

const short VersionNum = 0x0301;

int PWSfileV3::WriteHeader()
{
  // See formatV3.txt for explanation of what's written here and why
  unsigned int NumHashIters;
  if (m_nITER < MIN_HASH_ITERATIONS)
    NumHashIters = MIN_HASH_ITERATIONS;
  else
    NumHashIters = m_nITER;

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
  m_nCurrentMajorVersion = (unsigned short) ((VersionNum & 0xff00) >> 8);
  m_nCurrentMinorVersion = (unsigned short) (VersionNum & 0xff);

  numWritten = WriteCBC(HDR_VERSION, vnb, sizeof(VersionNum));
 
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  }
 
  // Write UUID
  uuid_array_t file_uuid_array;
  memset(file_uuid_array, 0x00, sizeof(file_uuid_array));
  // If not there or zeroed, create new
  if (memcmp(m_file_uuid_array, file_uuid_array, sizeof(file_uuid_array)) == 0) {
    CUUIDGen uuid;
    uuid.GetUUID(m_file_uuid_array);
  }
  
  numWritten = WriteCBC(HDR_UUID, m_file_uuid_array, sizeof(m_file_uuid_array));

  if (numWritten <= 0) {
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
  cs_update_time.Format(_T("%08x"), time_now);
  numWritten = WriteCBC(HDR_LASTUPDATETIME, cs_update_time);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  } else {
    m_whenlastsaved = cs_update_time;
  }

  // Write out who saved it!
  CString cs_who;
  cs_who.Format(_T("%04x%s%s"), m_user.GetLength(), m_user, m_sysname);
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
  cs_what.Format(_T("%s V%d.%02d"), AfxGetAppName(), nMajor, nMinor);
  numWritten = WriteCBC(HDR_LASTUPDATEAPPLICATION, cs_what);
  if (numWritten <= 0) {
    Close();
    return FAILURE;
  } else {
    m_whatlastsaved = cs_what;
  }

  if (!m_UHFL.empty()) {
    UnknownFieldList::iterator vi_IterUHFE;
    for (vi_IterUHFE = m_UHFL.begin();
         vi_IterUHFE != m_UHFL.end();
         vi_IterUHFE++) {
       UnknownFieldEntry &unkhfe = *vi_IterUHFE;
       numWritten = WriteCBC(unkhfe.uc_Type, unkhfe.uc_pUField, (unsigned int)unkhfe.st_length);
       if (numWritten <= 0) {
         Close();
         return FAILURE;
       }
    }
  }

  // Write zero-length end-of-record type item
  WriteCBC(HDR_END, NULL, 0);

  return SUCCESS;
}

int PWSfileV3::ReadHeader()
{
    unsigned char Ptag[SHA256::HASHLEN];
    int status = CheckPassword(m_filename, m_passkey, m_fd,
                               Ptag, &m_nITER);

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

    m_utf8 = NULL; m_utf8Len = 0;

    do {
        numRead = ReadCBC(fieldType, m_utf8, (unsigned int &)m_utf8Len);

        if (numRead < 0) {
            Close();
            return FAILURE;
        }

        switch (fieldType) {
            case HDR_VERSION: /* version */
                // in Beta, VersionNum was an int (4 bytes) instead of short (2)
                // This hack keeps bwd compatability.
                if (m_utf8Len != sizeof(VersionNum) &&
                    m_utf8Len != sizeof(int)) {
                    delete[] m_utf8; m_utf8 = NULL;
                    Close();
                    return FAILURE;
                }
                if (m_utf8[1] !=
                    (unsigned char)((VersionNum & 0xff00) >> 8)) {
                    //major version mismatch
                    delete[] m_utf8; m_utf8 = NULL;
                    Close();
                    return UNSUPPORTED_VERSION;
                }
                // for now we assume that minor version changes will
                // be backward-compatible
                m_nCurrentMajorVersion = (unsigned short)m_utf8[1];
                m_nCurrentMinorVersion = (unsigned short)m_utf8[0];
                break;

            case HDR_UUID: /* UUID */
                if (m_utf8Len != sizeof(uuid_array_t)) {
                    delete[] m_utf8; m_utf8 = NULL;
                    Close();
                    return FAILURE;
                }
                memcpy(m_file_uuid_array, m_utf8, sizeof(m_file_uuid_array));
                break;

            case HDR_NDPREFS: /* Non-default user preferences */
                if (m_utf8Len != 0) {
                    if (m_utf8 != NULL)
                        m_utf8[m_utf8Len] = '\0';
                    utf8status = FromUTF8(m_prefString);
                    if (!utf8status)
                        TRACE(_T("FromUTF8(m_prefString) failed\n"));
                } else
                    m_prefString = _T("");
            break;

            case HDR_DISPSTAT: /* Tree Display Status */
                if (m_utf8 != NULL)
                    m_utf8[m_utf8Len] = '\0';
                utf8status = FromUTF8(text);
                m_file_displaystatus = CString(text);
                if (!utf8status)
                    TRACE(_T("FromUTF8(m_file_displaystatus) failed\n"));
                break;

            case HDR_LASTUPDATETIME: /* When last saved */
                // THIS SHOULDN'T BE A STRING !!!
                // BROKE SPEC !!!
                if (m_utf8 != NULL)
                    m_utf8[m_utf8Len] = '\0';
                utf8status = FromUTF8(text);
                m_whenlastsaved = CString(text);
                if (!utf8status)
                    TRACE(_T("FromUTF8(m_whenlastsaved) failed\n"));
                break;

            case HDR_LASTUPDATEUSERHOST: /* and by whom */
                if (m_utf8 != NULL)
                    m_utf8[m_utf8Len] = '\0';
                utf8status = FromUTF8(text);
                m_wholastsaved = CString(text);
                if (!utf8status)
                    TRACE(_T("FromUTF8(m_wholastsaved) failed\n"));
                break;

            case HDR_LASTUPDATEAPPLICATION: /* and by what */
                if (m_utf8 != NULL)
                    m_utf8[m_utf8Len] = '\0';
                utf8status = FromUTF8(text);
                m_whatlastsaved = CString(text);
                if (!utf8status)
                    TRACE(_T("FromUTF8(m_whatlastsaved) failed\n"));
                break;

            case HDR_END: /* process END so not to treat it as 'unknown' */
                break;

            default:
                // Save unknown fields that may be addded by future versions
                UnknownFieldEntry unkhfe(fieldType, m_utf8Len, m_utf8);
                m_UHFL.push_back(unkhfe);
#ifdef _DEBUG
                CString cs_timestamp;
                cs_timestamp = PWSUtil::GetTimeStamp();
                TRACE(_T("%s: Header has unknown field: %02x, length %d/0x%04x, value:\n"), 
                          cs_timestamp, fieldType, m_utf8Len, m_utf8Len);
                PWSUtil::HexDump(m_utf8, m_utf8Len, cs_timestamp);
#endif /* DEBUG */
                break;
        }
        delete[] m_utf8; m_utf8 = NULL; m_utf8Len = 0;
    } while (fieldType != HDR_END);

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
    wcLen = MultiByteToWideChar(CP_ACP,             // code page
                                MB_PRECOMPOSED,     // character-type options
                                LPCSTR(data),       // string to map
                                -1,                 // -1 means null-terminated
                                NULL, 0);           // get needed buffer size
    if (wcLen == 0) { // uh-oh
        ASSERT(0);
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
    wcLen = MultiByteToWideChar(CP_ACP,             // code page
                                MB_PRECOMPOSED,     // character-type options
                                LPCSTR(data),       // string to map
                                -1,                 // -1 means null-terminated
                                m_wc, wcLen);       // output buffer
    ASSERT(wcLen != 0);
    wcPtr = m_wc;
#else
    wcPtr = const_cast<CString &>(data).GetBuffer();
    wcLen = data.GetLength()+1;
#endif
    // first get needed utf8 buffer size
    int mbLen = WideCharToMultiByte(CP_UTF8,       // code page
                                    0,             // performance and mapping flags
                                    wcPtr,         // wide-character string
                                    -1,            // -1 means null-terminated
                                    NULL, 0,       // get needed buffer size
                                    NULL,NULL);    // use system default for unmappables

    if (mbLen == 0) { // uh-oh
        ASSERT(0);
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
                                    LPSTR(m_utf8), mbLen, // buffer and length
                                    NULL,NULL);   // use system default for unmappables
    ASSERT(m_utf8Len != 0);
    m_utf8Len--; // remove unneeded null termination
    return true;
}

bool PWSfileV3::FromUTF8(CMyString &data)
{
    // Call MultiByteToWideChar to get from UTF-8 to Unicode.
    // If we're not in Unicode, call WideCharToMultiByte to
    // get to current codepage.
    // Input is in m_utf8 & m_utf8Len
    //
    // Due to a bug in pre-3.08 versions, data may be in ACP
    // instead of utf-8. We try to detect and workaround this.

    if (m_utf8Len == 0) {
        data = _T("");
        return true;
    }
    // first get needed wide char buffer size
    int wcLen = MultiByteToWideChar(CP_UTF8,      // code page
                                    0,            // character-type options
                                    LPSTR(m_utf8), // string to map
                                    -1,           // -1 means null-terminated
                                    NULL, 0);     // get needed buffer size
    if (wcLen == 0) { // uh-oh
      // it seems that this always returns non-zero, even if encoding
      // broken. Therefore, we'll give a consrevative value here,
      // and try to recover later
      TRACE(_T("FromUTF8: Couldn't get buffer size - guessing!"));
      wcLen = 2 * m_utf8Len;
    }
    // Allocate buffer (if previous allocation was smaller)
    if (wcLen > m_wcMaxLen) {
        if (m_wc != NULL)
            trashMemory(m_wc, m_wcMaxLen);
        delete[] m_wc;
        m_wc = new wchar_t[wcLen];
        m_wcMaxLen = wcLen;
    }
    // next translate to buffer
    wcLen = MultiByteToWideChar(CP_UTF8,      // code page
                                MB_ERR_INVALID_CHARS,  // character-type options
                                LPSTR(m_utf8),       // string to map
                                -1,           // -1 means null-terminated
                                m_wc, wcLen);       // output buffer
    if (wcLen == 0) {
        DWORD errCode = GetLastError();
        switch (errCode) {
            case ERROR_INSUFFICIENT_BUFFER:
                TRACE("INSUFFICIENT BUFFER"); break;
            case ERROR_INVALID_FLAGS:
                TRACE("INVALID FLAGS"); break;
            case ERROR_INVALID_PARAMETER:
                TRACE("INVALID PARAMETER"); break;
            case ERROR_NO_UNICODE_TRANSLATION:
              // try to recover
                TRACE("NO UNICODE TRANSLATION");
                wcLen = MultiByteToWideChar(CP_ACP,      // code page
                                MB_ERR_INVALID_CHARS,  // character-type options
                                LPSTR(m_utf8),       // string to map
                                -1,           // -1 means null-terminated
                                m_wc, wcLen);       // output buffer
                if (wcLen > 0) {
                  TRACE(_T("FromUTF8: recovery succeeded!"));
                }
                break;
            default:
                ASSERT(0);
        }
    }
    ASSERT(wcLen != 0);
#ifdef UNICODE
    if (wcLen != 0) {
      m_wc[wcLen-1] = TCHAR('\0');
      data = m_wc;
      return true;
    } else
      return false;
#else /* Go from Unicode to Locale encoding */
      // first get needed utf8 buffer size
    int mbLen = WideCharToMultiByte(CP_ACP,       // code page
                                    0, // performance and mapping flags
                                    m_wc,         // wide-character string
                                    -1,           // -1 means null-terminated
                                    NULL, 0,      // get needed buffer size
                                    NULL,NULL);   // use system default for unmappables

    if (mbLen == 0) { // uh-oh
        ASSERT(0);
        data = _T("");
        return false;
    }
    // Allocate buffer (if previous allocation was smaller)
    if (mbLen > m_tmpMaxLen) {
        if (m_tmp != NULL)
            trashMemory(m_tmp, m_tmpMaxLen);
        delete[] m_tmp;
        m_tmp = new unsigned char[mbLen];
        m_tmpMaxLen = mbLen;
    }
    // Finally get result
    int tmpLen = WideCharToMultiByte(CP_ACP,      // code page
                                     0, // performance and mapping flags
                                     m_wc, -1, // wide-character string
                                     LPSTR(m_tmp), mbLen,// buffer and length
                                     NULL,NULL);   // use system default for unmappables
    ASSERT(tmpLen == mbLen);
    m_tmp[mbLen-1] = '\0'; // char, no need to _T()...
    data = m_tmp;
    ASSERT(data.GetLength() != 0);
    return true;
#endif /* !UNICODE */
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
