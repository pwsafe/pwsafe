/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PWSAttfileV3.h"
#include "UUIDGen.h"
#include "PWSrand.h"
#include "Util.h"
#include "SysInfo.h"
#include "PWScore.h"
#include "PWSdirs.h"
#include "PWSprefs.h"
#include "corelib.h"
#include "return_codes.h"

#include "os/debug.h"
#include "os/file.h"

#ifdef _WIN32
#include <io.h>
#endif

#include <fcntl.h>
#include <iomanip>

extern bool pull_string(StringX &str, unsigned char *data, int len);
extern bool pull_time(time_t &t, unsigned char *data, size_t len);
extern bool pull_uint(unsigned int &uint, unsigned char *data, size_t len);
extern bool pull_int(int &i, unsigned char *data, size_t len);

using namespace std;

static unsigned char ATT_TERMINAL_BLOCK[TwoFish::BLOCKSIZE] = {
  'P', 'A', 'T', '3', '-', 'E', 'O', 'F',
  'P', 'A', 'T', '3', '-', 'E', 'O', 'F'};

PWSAttfileV3::PWSAttfileV3(const StringX &filename, RWmode mode, VERSION version)
  : PWSAttfile(filename, mode)
{
  m_curversion = version;
  m_IV = m_ipthing;
  m_terminal = ATT_TERMINAL_BLOCK;
}

PWSAttfileV3::~PWSAttfileV3()
{
}

int PWSAttfileV3::Open(const StringX &passkey)
{
  int status = PWSRC::SUCCESS;

  ASSERT(m_curversion == V30);
  if (passkey.empty()) { // Can happen if db 'locked'
    pws_os::Trace(_T("PWSAttfileV3::Open(empty_passkey)\n"));
    return PWSRC::WRONG_PASSWORD;
  }
  m_passkey = passkey;

  FOpen();
  if (m_fd == NULL)
    return PWSRC::CANT_OPEN_FILE;

  if (m_rw == Write) {
    status = WriteHeader();
  } else { // open for read
    status = ReadHeader();
    if (status != PWSRC::SUCCESS) {
      Close();
      return status;
    }
  }
  return status;
}

int PWSAttfileV3::Close()
{
  if (m_fd == NULL)
    return PWSRC::SUCCESS; // idempotent

  unsigned char digest[HMAC_SHA256::HASHLEN];
  m_hmac.Final(digest);

  // Write or verify HMAC, depending on RWmode.
  if (m_rw == Write) {
    size_t fret;
    fret = fwrite(ATT_TERMINAL_BLOCK, sizeof(ATT_TERMINAL_BLOCK), 1, m_fd);
    if (fret != 1) {
      PWSAttfile::Close();
      return PWSRC::FAILURE;
    }
    fret = fwrite(digest, sizeof(digest), 1, m_fd);
    if (fret != 1) {
      PWSAttfile::Close();
      return PWSRC::FAILURE;
    }
    return  PWSAttfile::Close();
  } else { // Read
    // We're here *after* TERMINAL_BLOCK has been read
    // and detected (by _readcbc) - just read hmac & verify
    unsigned char d[HMAC_SHA256::HASHLEN];
    fread(d, sizeof(d), 1, m_fd);
    if (memcmp(d, digest, HMAC_SHA256::HASHLEN) == 0)
      return PWSAttfile::Close();
    else {
      PWSAttfile::Close();
      return PWSRC::BAD_DIGEST;
    }
  }
}

const char V3ATT_TAG[4] = {'P','A','T','3'}; // ASCII chars, not wchar

int PWSAttfileV3::CheckPasskey(const StringX &filename, const StringX &passkey,
                               FILE *a_fd, unsigned char *aPtag, int *nITER)
{
  FILE *fd = a_fd;
  int retval = PWSRC::SUCCESS;
  SHA256 H;

  if (fd == NULL) {
    fd = pws_os::FOpen(filename.c_str(), _T("rb"));
  }
  if (fd == NULL)
    return PWSRC::CANT_OPEN_FILE;

  char tag[sizeof(V3ATT_TAG)];
  fread(tag, 1, sizeof(tag), fd);
  if (memcmp(tag, V3ATT_TAG, sizeof(tag)) != 0) {
    retval = PWSRC::NOT_PWS3_FILE;
    goto err;
  }

  unsigned char salt[SaltLengthV3];
  fread(salt, 1, sizeof(salt), fd);

  unsigned char Nb[sizeof(unsigned int)];
  fread(Nb, 1, sizeof(Nb), fd);
  { // block to shut up compiler warning w.r.t. goto
    const unsigned int N = getInt32(Nb);

    ASSERT(N >= MIN_HASH_ITERATIONS);
    if (N < MIN_HASH_ITERATIONS) {
      retval = PWSRC::FAILURE;
      goto err;
    }

    if (nITER != NULL)
      *nITER = N;
    unsigned char Ptag[SHA256::HASHLEN];
    if (aPtag == NULL)
      aPtag = Ptag;

    StretchKey(salt, sizeof(salt), passkey, N, aPtag);
  }
  unsigned char HPtag[SHA256::HASHLEN];
  H.Update(aPtag, SHA256::HASHLEN);
  H.Final(HPtag);
  unsigned char readHPtag[SHA256::HASHLEN];
  fread(readHPtag, 1, sizeof(readHPtag), fd);
  if (memcmp(readHPtag, HPtag, sizeof(readHPtag)) != 0) {
    retval = PWSRC::WRONG_PASSWORD;
    goto err;
  }
err:
  if (a_fd == NULL) // if we opened the file, we close it...
    fclose(fd);
  return retval;
}

size_t PWSAttfileV3::WriteCBC(unsigned char type, const StringX &data)
{
  bool status;
  const unsigned char *utf8;
  int utf8Len;
  status = m_utf8conv.ToUTF8(data, utf8, utf8Len);
  if (!status)
    pws_os::Trace(_T("ToUTF8(%s) failed\n"), data.c_str());
  return WriteCBC(type, utf8, utf8Len);
}

size_t PWSAttfileV3::WriteCBC(unsigned char type, const unsigned char *data,
                              unsigned int length)
{
  m_hmac.Update(data, length);
  return PWSAttfile::WriteCBC(type, data, length);
}

int PWSAttfileV3::WriteAttmntRecordPreData(const ATRecord &atr)
{
  ASSERT(m_curversion == V30);

  // ******************************************************************
  // NOTE: The attachment record UUID MUST be the first field written *
  // so that it is read in before the actual attachment data is read  *
  // just in case we wish to skip reading it depending on its value   *
  // ******************************************************************
  WriteCBC(ATTMT_UUID, atr.attmt_uuid, sizeof(uuid_array_t));
  WriteCBC(ATTMT_ENTRY_UUID, atr.entry_uuid, sizeof(uuid_array_t));

  WriteCBC(ATTMT_FLAGS, &atr.flags, sizeof(atr.flags));

  WriteCBC(ATTMT_FNAME, atr.filename);
  WriteCBC(ATTMT_PATH, atr.path);
  WriteCBC(ATTMT_DESC, atr.description);

  WriteCBC(ATTMT_UNCSIZE, (unsigned char *)&atr.uncsize, sizeof(atr.uncsize));
  WriteCBC(ATTMT_BLKSIZE, (unsigned char *)&atr.blksize, sizeof(atr.blksize));

  WriteCBC(ATTMT_CTIME, (unsigned char *)&atr.ctime, sizeof(time_t));
  WriteCBC(ATTMT_ATIME, (unsigned char *)&atr.atime, sizeof(time_t));
  WriteCBC(ATTMT_MTIME, (unsigned char *)&atr.mtime, sizeof(time_t));
  WriteCBC(ATTMT_DTIME, (unsigned char *)&atr.dtime, sizeof(time_t));

  return PWSRC::SUCCESS;
}

int PWSAttfileV3::WriteAttmntRecordData(unsigned char *pData, const unsigned int len,
                                        const unsigned char type)
{
  // type should be ATTMT_DATA or ATTMT_LASTDATA
  WriteCBC(type, pData, len);

  return PWSRC::SUCCESS;
}

int PWSAttfileV3::WriteAttmntRecordPostData(const ATRecord &atr)
{
  WriteCBC(ATTMT_CMPSIZE, (unsigned char *)&atr.cmpsize, sizeof(atr.cmpsize));
  WriteCBC(ATTMT_CRC, (unsigned char *)&atr.CRC, sizeof(atr.CRC));
  WriteCBC(ATTMT_ODIGEST, (unsigned char *)&atr.odigest, sizeof(atr.odigest));
  WriteCBC(ATTMT_CDIGEST, (unsigned char *)&atr.cdigest, sizeof(atr.cdigest));
  WriteCBC(ATTMT_END, _T(""));

  return PWSRC::SUCCESS;
}

size_t PWSAttfileV3::ReadCBC(unsigned char &type, unsigned char* &data,
                             unsigned int &length,
                             bool bSkip, unsigned char *pSkipTypes)
{
  size_t num_bytes_read = PWSAttfile::ReadCBC(type, data, length,
                                 bSkip, pSkipTypes);

  if (num_bytes_read > 0 && !bSkip) {
    m_hmac.Update(data, length);
  }

  return num_bytes_read;
}

int PWSAttfileV3::ReadAttmntRecordPreData(ATRecord &atr)
{
  /*
    *** NOTE *** - The attachment database REQUIRES that fields are written in
    numerical order as defined by the field type.

    This routine will NOT read the data portions or the END field.

    *** IT WILL STOP WHEN THE FIELD TYPE JUST READ IS ATTMT_LASTDATA. ***
  */

  ASSERT(m_curversion == V30);

  int status = PWSRC::SUCCESS;
  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen;    // <= 0 means end of file reached

  StringX str;
  unsigned int uint;
  time_t t;
  bool go(true);

  do {
    unsigned char *utf8 = NULL;
    int utf8Len = 0;
    fieldLen = static_cast<signed long>(ReadCBC(type, utf8,
                                          (unsigned int &)utf8Len));

    if (fieldLen > 0) {
      numread += fieldLen;
      switch (type) {
        case ATTMT_UUID:
          if (utf8Len != sizeof(uuid_array_t)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          memcpy(atr.attmt_uuid, utf8, sizeof(uuid_array_t));
          break;
        case ATTMT_ENTRY_UUID:
          if (utf8Len != sizeof(uuid_array_t)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          memcpy(atr.entry_uuid, utf8, sizeof(uuid_array_t));
          break;
        case ATTMT_FLAGS:
          if (utf8Len != 1) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.flags = utf8[0];
          break;
        case ATTMT_FNAME:
          if (!pull_string(str, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.filename = str;
          break;
        case ATTMT_PATH:
          if (!pull_string(str, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.path = str;
          break;
        case ATTMT_DESC:
          if (!pull_string(str, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.description = str;
          break;
        case ATTMT_UNCSIZE:
          if (!pull_uint(uint, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.uncsize = uint;
          break;
        case ATTMT_BLKSIZE:
          if (!pull_uint(uint, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.blksize = uint;
          break;
        case ATTMT_CTIME:
          if (!pull_time(t, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.ctime = t;
          break;
        case ATTMT_ATIME:
          if (!pull_time(t, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.atime = t;
          break;
        case ATTMT_MTIME:
          if (!pull_time(t, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.mtime = t;
          break;
        case ATTMT_DTIME:
          if (!pull_time(t, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.dtime = t;
          break;
        // Data fields
        case ATTMT_DATA:
        case ATTMT_LASTDATA:
        // Post-data fields
        case ATTMT_CMPSIZE:
        case ATTMT_CRC:
        case ATTMT_ODIGEST:
        case ATTMT_CDIGEST:
        case ATTMT_END:
          ASSERT(0);
          break;
        default:
          break;
      }
    } // if (fieldLen > 0)

    if (utf8 != NULL) {
      trashMemory(utf8, utf8Len * sizeof(utf8[0]));
      delete[] utf8; utf8 = NULL; utf8Len = 0;
    }
  } while (go && (type != (ATTMT_LAST_PREDATA - 1)) &&
                 fieldLen > 0 && --emergencyExit > 0);

  if (numread > 0)
    return status;
  else
    return PWSRC::END_OF_FILE;
}

int PWSAttfileV3::ReadAttmntRecordData(unsigned char * &pCmpData, unsigned int &uiCmpLen,
                                       unsigned char &readtype, const bool bSkip)
{
  ASSERT(m_curversion == V30);

  int status = PWSRC::SUCCESS;

  unsigned int fieldLen(0);    // <= 0 means end of file reached

  unsigned char pSkipTypes[256] = {0};
  // If we skip a field - these are the ones
  pSkipTypes[ATTMT_DATA] = 0x01;
  pSkipTypes[ATTMT_LASTDATA] = 0x01;

  unsigned char type;
  unsigned char *utf8 = NULL;
  int utf8Len = 0;
  uiCmpLen = 0;

  fieldLen = static_cast<signed long>(ReadCBC(type, utf8,
                      (unsigned int &)utf8Len, bSkip, pSkipTypes));

  if (fieldLen > 0) {
    readtype = type;
    switch (type) {
      case ATTMT_DATA:
      case ATTMT_LASTDATA:
        if (utf8Len == 0) {
          pCmpData = NULL;
          status = PWSRC::BAD_ATTACHMENT; break;
        }
        uiCmpLen = utf8Len;
        if (bSkip) {
          pCmpData = NULL;
        } else {
          pCmpData = new unsigned char[utf8Len];
          memcpy(pCmpData, utf8, utf8Len);
        }
        break;
      // Pre-data fields
      case ATTMT_UUID:
      case ATTMT_ENTRY_UUID:
      case ATTMT_FLAGS:
      case ATTMT_FNAME:
      case ATTMT_PATH:
      case ATTMT_DESC:
      case ATTMT_UNCSIZE:
      case ATTMT_BLKSIZE:
      case ATTMT_CTIME:
      case ATTMT_ATIME:
      case ATTMT_MTIME:
      case ATTMT_DTIME:
      // Post-data fields
      case ATTMT_CMPSIZE:
      case ATTMT_CRC:
      case ATTMT_ODIGEST:
      case ATTMT_CDIGEST:
      case ATTMT_END:
        ASSERT(0);
        break;
      default:
        ASSERT(0);
        break;
    }
  } // if (fieldLen > 0)

  if (utf8 != NULL) {
    trashMemory(utf8, utf8Len * sizeof(utf8[0]));
    delete[] utf8; utf8 = NULL; utf8Len = 0;
  }

  if (fieldLen > 0)
    return status;
  else
    return PWSRC::END_OF_FILE;
}

int PWSAttfileV3::ReadAttmntRecordPostData(ATRecord &atr)
{
  /*
    *** NOTE *** - The attachment database REQUIRES that fields are written in
    numerical order as defined by the field type.

    This routine will NOT read the data portions or the END field.

    *** IT WILL STOP WHEN THE FIELD TYPE JUST READ IS ATTMT_LASTDATA. ***
  */

  ASSERT(m_curversion == V30);

  int status = PWSRC::SUCCESS;
  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen;    // <= 0 means end of file reached

  StringX str;
  unsigned int uint;
  int i32;
  bool go(true);

  do {
    unsigned char *utf8 = NULL;
    int utf8Len = 0;
    fieldLen = static_cast<signed long>(ReadCBC(type, utf8,
                                          (unsigned int &)utf8Len));

    if (fieldLen > 0) {
      numread += fieldLen;
      switch (type) {
        case ATTMT_CMPSIZE:
          if (!pull_uint(uint, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.cmpsize = uint;
          break;
        case ATTMT_CRC:
          if (!pull_int(i32, utf8, utf8Len)) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          atr.CRC = i32;
          break;
        case ATTMT_ODIGEST:
          if (utf8Len != SHA1::HASHLEN) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          memcpy(atr.odigest, utf8, SHA1::HASHLEN);
          break;
        case ATTMT_CDIGEST:
          if (utf8Len != SHA1::HASHLEN) {
            go = false; status = PWSRC::BAD_ATTACHMENT; break;
          }
          memcpy(atr.cdigest, utf8, SHA1::HASHLEN);
          break;
        // Pre-data fields
        case ATTMT_UUID:
        case ATTMT_ENTRY_UUID:
        case ATTMT_FLAGS:
        case ATTMT_FNAME:
        case ATTMT_PATH:
        case ATTMT_DESC:
        case ATTMT_UNCSIZE:
        case ATTMT_BLKSIZE:
        case ATTMT_CTIME:
        case ATTMT_ATIME:
        case ATTMT_MTIME:
        case ATTMT_DTIME:
        // Data fields
        case ATTMT_DATA:
        case ATTMT_LASTDATA:
          ASSERT(0);
          break;
        case ATTMT_END:
        default:
          break;
      }
    } // if (fieldLen > 0)

    if (utf8 != NULL) {
      trashMemory(utf8, utf8Len * sizeof(utf8[0]));
      delete[] utf8; utf8 = NULL; utf8Len = 0;
    }
  } while (go && (type != ATTMT_END) && fieldLen > 0 && --emergencyExit > 0);

  if (numread > 0)
    return status;
  else
    return PWSRC::END_OF_FILE;
}

void PWSAttfileV3::StretchKey(const unsigned char *salt, unsigned long saltLen,
                              const StringX &passkey,
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

const unsigned short AttVersionNum = 0x8301;

// Following specific for PWSAttfileV3::WriteHeader
#define SAFE_FWRITE(p, sz, cnt, stream) \
  { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { status = PWSRC::FAILURE; goto end;} \
  }

int PWSAttfileV3::WriteHeader()
{
  // Following code is divided into {} blocks to
  // prevent "uninitialized" compile errors, as we use
  // goto for error handling
  int status = PWSRC::SUCCESS;
  size_t numWritten;
  unsigned char salt[SaltLengthV3];

  // See formatV3.txt for explanation of what's written here and why
  unsigned int NumHashIters;
  if (m_atthdr.nITER < MIN_HASH_ITERATIONS)
    NumHashIters = MIN_HASH_ITERATIONS;
  else
    NumHashIters = m_atthdr.nITER;

  SAFE_FWRITE(V3ATT_TAG, 1, sizeof(V3ATT_TAG), m_fd);

  // According to the spec, salt is just random data. I don't think though,
  // that it's good practice to directly expose the generated randomness
  // to the attacker. Therefore, we'll hash the salt.
  // The following takes shameless advantage of the fact that
  // SaltLengthV3 == SHA256::HASHLEN
  ASSERT(SaltLengthV3 == SHA256::HASHLEN); // if false, have to recode
  { // in a block to protect against goto
    PWSrand::GetInstance()->GetRandomData(salt, sizeof(salt));
    SHA256 salter;
    salter.Update(salt, sizeof(salt));
    salter.Final(salt);
    SAFE_FWRITE(salt, 1, sizeof(salt), m_fd);
  }

  unsigned char Nb[sizeof(NumHashIters)];
  putInt32(Nb, NumHashIters);
  SAFE_FWRITE(Nb, 1, sizeof(Nb), m_fd);

  unsigned char Ptag[SHA256::HASHLEN];

  StretchKey(salt, sizeof(salt), m_passkey, NumHashIters, Ptag);

  {
    unsigned char HPtag[SHA256::HASHLEN];
    SHA256 H;
    H.Update(Ptag, sizeof(Ptag));
    H.Final(HPtag);
    SAFE_FWRITE(HPtag, 1, sizeof(HPtag), m_fd);
  }

  {
    PWSrand::GetInstance()->GetRandomData(m_key, sizeof(m_key));
    unsigned char B1B2[sizeof(m_key)];
    unsigned char L[32]; // for HMAC
    ASSERT(sizeof(B1B2) == 32); // Generalize later
    TwoFish TF(Ptag, sizeof(Ptag));
    TF.Encrypt(m_key, B1B2);
    TF.Encrypt(m_key + 16, B1B2 + 16);
    SAFE_FWRITE(B1B2, 1, sizeof(B1B2), m_fd);
    PWSrand::GetInstance()->GetRandomData(L, sizeof(L));
    unsigned char B3B4[sizeof(L)];
    ASSERT(sizeof(B3B4) == 32); // Generalize later
    TF.Encrypt(L, B3B4);
    TF.Encrypt(L + 16, B3B4 + 16);
    SAFE_FWRITE(B3B4, 1, sizeof(B3B4), m_fd);
    m_hmac.Init(L, sizeof(L));
  }

  {
    // See discussion on Salt to understand why we hash
    // random data instead of writing it directly
    unsigned char ip_rand[SHA256::HASHLEN];
    PWSrand::GetInstance()->GetRandomData(ip_rand, sizeof(ip_rand));
    SHA256 ipHash;
    ipHash.Update(ip_rand, sizeof(ip_rand));
    ipHash.Final(ip_rand);
    ASSERT(sizeof(ip_rand) >= sizeof(m_ipthing)); // compilation assumption
    memcpy(m_ipthing, ip_rand, sizeof(m_ipthing));
  }
  SAFE_FWRITE(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  m_fish = new TwoFish(m_key, sizeof(m_key));

  // write some actual data (at last!)
  numWritten = 0;
  // Write version number
  unsigned char vnb[sizeof(AttVersionNum)];
  vnb[0] = (unsigned char) (AttVersionNum & 0xff);
  vnb[1] = (unsigned char) ((AttVersionNum & 0xff00) >> 8);
  m_atthdr.nCurrentMajorVersion = (unsigned short) ((AttVersionNum & 0xff00) >> 8);
  m_atthdr.nCurrentMinorVersion = (unsigned short) (AttVersionNum & 0xff);

  numWritten = WriteCBC(ATTHDR_VERSION, vnb, sizeof(AttVersionNum));

  if (numWritten <= 0) { status = PWSRC::FAILURE; goto end; }

  // Write UUIDs
  numWritten = WriteCBC(ATTHDR_FILEUUID, m_atthdr.attfile_uuid,
                        sizeof(uuid_array_t));
  if (numWritten <= 0) { status = PWSRC::FAILURE; goto end; }

  numWritten = WriteCBC(ATTHDR_DBUUID, m_atthdr.DBfile_uuid,
                        sizeof(uuid_array_t));
  if (numWritten <= 0) { status = PWSRC::FAILURE; goto end; }

  // Write out time of this update
  numWritten = WriteCBC(ATTHDR_LASTUPDATETIME,
                        (unsigned char *)&(m_atthdr.whenlastsaved), sizeof(time_t));
  if (numWritten <= 0) { status = PWSRC::FAILURE; goto end; }

  // Write out who saved it!
  {
    const SysInfo *si = SysInfo::GetInstance();
    stringT user = si->GetRealUser();
    stringT sysname = si->GetRealHost();
    numWritten = WriteCBC(ATTHDR_LASTUPDATEUSER, user.c_str());
    if (numWritten > 0)
      numWritten = WriteCBC(ATTHDR_LASTUPDATEHOST, sysname.c_str());
    if (numWritten <= 0) { status = PWSRC::FAILURE; goto end; }
    m_atthdr.lastsavedby = user.c_str();
    m_atthdr.lastsavedon = sysname.c_str();
  }

  // Write out what saved it!
  numWritten = WriteCBC(ATTHDR_LASTUPDATEAPPLICATION,
                        m_atthdr.whatlastsaved);
  if (numWritten <= 0) { status = PWSRC::FAILURE; goto end; }

  // Write zero-length end-of-record type item
  WriteCBC(PWSAttfileV3::END, NULL, 0);

end:
  if (status != PWSRC::SUCCESS)
    Close();
  return status;
}

int PWSAttfileV3::ReadHeader()
{
  unsigned char Ptag[SHA256::HASHLEN];
  int status = CheckPasskey(m_filename, m_passkey, m_fd,
                            Ptag, &m_atthdr.nITER);

  if (status != PWSRC::SUCCESS) {
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
  StringX text;
  size_t numRead;
  bool utf8status;
  unsigned char *utf8 = NULL;
  int utf8Len = 0;

  do {
    numRead = ReadCBC(fieldType, utf8, (unsigned int &)utf8Len);

    if (numRead < 0) {
      Close();
      return PWSRC::FAILURE;
    }

    switch (fieldType) {
      case ATTHDR_VERSION: /* version */
        if (utf8Len != sizeof(AttVersionNum)) {
          delete[] utf8;
          Close();
          return PWSRC::FAILURE;
        }
        if (utf8[1] !=
           (unsigned char)((AttVersionNum & 0xff00) >> 8)) {
          //major version mismatch
          delete[] utf8;
          Close();
          return PWSRC::UNSUPPORTED_VERSION;
        }
        // for now we assume that minor version changes will
        // be backward-compatible
        m_atthdr.nCurrentMajorVersion = (unsigned short)utf8[1];
        m_atthdr.nCurrentMinorVersion = (unsigned short)utf8[0];
        break;

      case ATTHDR_FILEUUID: /* UUID */
        if (utf8Len != sizeof(uuid_array_t)) {
          delete[] utf8;
          Close();
          return PWSRC::FAILURE;
        }
        memcpy(m_atthdr.attfile_uuid, utf8, sizeof(uuid_array_t));
        break;

      case ATTHDR_DBUUID: /* DBase UUID */
        if (utf8Len != sizeof(uuid_array_t)) {
          delete[] utf8;
          Close();
          return PWSRC::FAILURE;
        }
        memcpy(m_atthdr.DBfile_uuid, utf8, sizeof(uuid_array_t));
        break;

      case ATTHDR_LASTUPDATETIME: /* When last saved */
        if (utf8Len == 4) {
          // retrieve time_t
          m_atthdr.whenlastsaved = *reinterpret_cast<time_t*>(utf8);
        } else {
          m_atthdr.whenlastsaved = 0;
        }
        break;

      case ATTHDR_LASTUPDATEAPPLICATION: /* and by what */
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        m_atthdr.whatlastsaved = text;
        if (!utf8status)
          pws_os::Trace0(_T("FromUTF8(m_whatlastsaved) failed\n"));
        break;

      case ATTHDR_LASTUPDATEUSER:
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        m_atthdr.lastsavedby = text;
        break;

      case ATTHDR_LASTUPDATEHOST:
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        m_atthdr.lastsavedon = text;
        break;

      case END: /* process END so not to treat it as 'unknown' */
        break;

      default:
        break;
    }
    delete[] utf8; utf8 = NULL; utf8Len = 0;
  } while (fieldType != END);

  return PWSRC::SUCCESS;
}

bool PWSAttfileV3::IsV3x(const StringX &filename, VERSION &v)
{
  // This is written so as to support V30, V31, V3x...

  ASSERT(pws_os::FileExists(filename.c_str()));
  FILE *fd = pws_os::FOpen(filename.c_str(), _T("rb"));

  ASSERT(fd != NULL);
  char tag[sizeof(V3ATT_TAG)];
  fread(tag, 1, sizeof(tag), fd);
  fclose(fd);

  if (memcmp(tag, V3ATT_TAG, sizeof(tag)) == 0) {
    v = V30;
    return true;
  } else {
    v = UNKNOWN_VERSION;
    return false;
  }
}
