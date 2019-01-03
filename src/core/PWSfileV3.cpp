/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "PWSfileV3.h"
#include "PWSrand.h"
#include "Util.h"
#include "SysInfo.h"
#include "PWScore.h"
#include "PWSFilters.h"
#include "PWSdirs.h"
#include "PWSprefs.h"
#include "core.h"

#include "os/debug.h"
#include "os/file.h"
#include "os/logit.h"

#include "XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#ifdef _WIN32
#include <io.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <iomanip>
#include <type_traits> // for static_assert
#include <algorithm> // for sort

using namespace std;
using pws_os::CUUID;

/**
 * Format version history:
 *
 * PasswordSafe Version   Format Version
 * =====================================
 *         V3.01           0x0300
 *         V3.03           0x0301
 *         V3.09           0x0302
 *         V3.12           0x0303
 *         V3.13           0x0304
 *         V3.14           0x0305
 *         V3.19           0x0306
 *         V3.22           0x0307
 *         V3.25           0x0308
 *         V3.26           0x0309
 *         V3.28           0x030A
 *         V3.29           0x030B
 *         V3.29Y          0x030C
 *         V3.30           0x030D
 *         V3.47           0x030E
*/

const short VersionNum = 0x030E;

static unsigned char TERMINAL_BLOCK[TwoFish::BLOCKSIZE] = {
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F',
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F'};

PWSfileV3::PWSfileV3(const StringX &filename, RWmode mode, VERSION version)
: PWSfile(filename, mode, version), m_nHashIters(0)
{
  m_IV = m_ipthing;
  m_terminal = TERMINAL_BLOCK;
}

PWSfileV3::~PWSfileV3()
{
}

int PWSfileV3::Open(const StringX &passkey)
{
  PWS_LOGIT;

  m_status = SUCCESS;

  ASSERT(m_curversion == V30);
  if (passkey.empty()) { // Can happen if db 'locked'
    pws_os::Trace(_T("PWSfileV3::Open(empty_passkey)\n"));
    return WRONG_PASSWORD;
  }
  m_passkey = passkey;

  FOpen();
  if (m_fd == nullptr)
    return CANT_OPEN_FILE;

  if (m_rw == Write) {
    m_status = WriteHeader();
  } else { // open for read
    m_status = ReadHeader();
    if (m_status != SUCCESS) {
      Close();
      return m_status;
    }
  }
  return m_status;
}

int PWSfileV3::Close()
{
  PWS_LOGIT;

  if (m_fd == nullptr)
    return SUCCESS; // idempotent

  // If we're here as part of failure handling,
  // no sense to work on digests, we might even
  // assert there...
  if (m_status != SUCCESS)
    return PWSfile::Close();

  unsigned char digest[SHA256::HASHLEN];
  m_hmac.Final(digest);

  // Write or verify HMAC, depending on RWmode.
  if (m_rw == Write) {
    size_t fret;
    fret = fwrite(TERMINAL_BLOCK, sizeof(TERMINAL_BLOCK), 1, m_fd);
    if (fret != 1) {
      PWSfile::Close();
      return FAILURE;
    }
    fret = fwrite(digest, sizeof(digest), 1, m_fd);
    if (fret != 1) {
      PWSfile::Close();
      return FAILURE;
    }
    return PWSfile::Close();
  } else { // Read
    // We're here *after* TERMINAL_BLOCK has been read
    // and detected (by _readcbc) - just read hmac & verify
    unsigned char d[SHA256::HASHLEN];
    fread(d, sizeof(d), 1, m_fd);
    if (memcmp(d, digest, SHA256::HASHLEN) == 0)
      return PWSfile::Close();
    else {
      PWSfile::Close();
      return BAD_DIGEST;
    }
  }
}

const char V3TAG[4] = {'P','W','S','3'}; // ASCII chars, not wchar

int PWSfileV3::SanityCheck(FILE *stream)
{
  int retval = SUCCESS;
  size_t nread = 0;

  ASSERT(stream != nullptr);
  const long pos = ftell(stream); // restore when we're done

  // Is file too small?
  const auto file_length = pws_os::fileLength(stream);
  const unsigned long min_v3file_length = 232; // pre + post, no hdr or records

  // Does file have a valid header?
  if (file_length < sizeof(V3TAG)) {
    retval = NOT_PWS3_FILE;
    goto err;
  }
  char tag[sizeof(V3TAG)];
  nread = fread(tag, sizeof(tag), 1, stream);
  if (nread != 1) {
    retval = READ_FAIL;
    goto err;
  }
  if (memcmp(tag, V3TAG, sizeof(tag)) != 0) {
    retval = NOT_PWS3_FILE;
    goto err;
  }

  if (file_length < min_v3file_length)
    return TRUNCATED_FILE;
  // Does file have a valid EOF block?
  unsigned char eof_block[sizeof(TERMINAL_BLOCK)];
  if (fseek(stream, -int(sizeof(TERMINAL_BLOCK) + SHA256::HASHLEN), SEEK_END) != 0) {
    retval = READ_FAIL; // actually, seek error, but that's too nuanced
    goto err;
  }
  nread = fread(eof_block, sizeof(eof_block), 1, stream);
  if (nread != 1) {
    retval = READ_FAIL;
    goto err;
  }
  if (memcmp(eof_block, TERMINAL_BLOCK, sizeof(TERMINAL_BLOCK)) != 0)
    retval = TRUNCATED_FILE;

err:
  fseek(stream, pos, SEEK_SET);
  return retval;
}

int PWSfileV3::CheckPasskey(const StringX &filename,
                            const StringX &passkey, FILE *a_fd,
                            unsigned char *aPtag, uint32 *nITER)
{
  PWS_LOGIT;

  FILE *fd = a_fd;
  SHA256 H;
  unsigned char Ptag[SHA256::HASHLEN];
  unsigned char *usedPtag = (aPtag == nullptr) ? Ptag : aPtag;


  if (fd == nullptr) {
    fd = pws_os::FOpen(filename.c_str(), _T("rb"));
  }
  if (fd == nullptr)
    return CANT_OPEN_FILE;

  int retval = SanityCheck(fd);
  if (retval != SUCCESS)
    goto err;

  fseek(fd, sizeof(V3TAG), SEEK_SET); // skip over tag
  unsigned char salt[PWSaltLength];
  fread(salt, 1, sizeof(salt), fd);

  unsigned char Nb[sizeof(uint32)];
  fread(Nb, 1, sizeof(Nb), fd);
  { // block to shut up compiler warning w.r.t. goto
    const uint32 N = getInt32(Nb);

    ASSERT(N >= MIN_HASH_ITERATIONS);
    if (N < MIN_HASH_ITERATIONS) {
      retval = FAILURE;
      goto err;
    }

    if (nITER != nullptr)
      *nITER = N;

    StretchKey(salt, sizeof(salt), passkey, N, usedPtag);
  }
  unsigned char HPtag[SHA256::HASHLEN];
  H.Update(usedPtag, SHA256::HASHLEN);
  H.Final(HPtag);
  unsigned char readHPtag[SHA256::HASHLEN];
  fread(readHPtag, 1, sizeof(readHPtag), fd);
  if (memcmp(readHPtag, HPtag, sizeof(readHPtag)) != 0) {
    retval = WRONG_PASSWORD;
    goto err;
  }
err:
  if (a_fd == nullptr) // if we opened the file, we close it...
    fclose(fd);
  return retval;
}

size_t PWSfileV3::WriteCBC(unsigned char type, const StringX &data)
{
  const unsigned char *utf8(nullptr);
  size_t utf8Len(0);

  bool status = m_utf8conv.ToUTF8(data, utf8, utf8Len);
  if (!status)
    pws_os::Trace(_T("ToUTF8(%ls) failed\n"), data.c_str());
  return WriteCBC(type, utf8, utf8Len);
}

size_t PWSfileV3::WriteCBC(unsigned char type, const unsigned char *data,
                           size_t length)
{
  m_hmac.Update(data, static_cast<unsigned long>(length));
  return PWSfile::WriteCBC(type, data, length);
}

int PWSfileV3::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != nullptr);
  ASSERT(m_curversion == V30);
  return item.Write(this);
}

size_t PWSfileV3::ReadCBC(unsigned char &type, unsigned char* &data,
                          size_t &length)
{
  size_t numRead = PWSfile::ReadCBC(type, data, length);

  if (numRead > 0) {
    m_hmac.Update(data, reinterpret_cast<unsigned long &>(length));
  }

  return numRead;
}

int PWSfileV3::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != nullptr);
  ASSERT(m_curversion == V30);
  return item.Read(this);
}

void PWSfileV3::StretchKey(const unsigned char *salt, unsigned long saltLen,
                           const StringX &passkey,
                           unsigned int N, unsigned char *Ptag)
{
  /*
  * P' is the "stretched key" of the user's passphrase and the SALT, as defined
  * by the hash-function-based key stretching algorithm in
  * http://www.schneier.com/paper-low-entropy.pdf (Section 4.1), with SHA-256
  * as the hash function, and N iterations.
  */
  size_t passLen = 0;
  unsigned char *pstr = nullptr;

  ConvertPasskey(passkey, pstr, passLen);
  unsigned char *X = Ptag;
  SHA256 H0;
  H0.Update(pstr, passLen);
  H0.Update(salt, saltLen);
  H0.Final(X);

  trashMemory(pstr, passLen);
  delete[] pstr;

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

// Following specific for PWSfileV3::WriteHeader
#define SAFE_FWRITE(p, sz, cnt, stream) \
  { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { m_status = FAILURE; goto end;} \
  }

int PWSfileV3::WriteHeader()
{
  PWS_LOGIT;

  // Following code is divided into {} blocks to
  // prevent "uninitialized" compile errors, as we use
  // goto for error handling
  size_t numWritten;
  unsigned char salt[PWSaltLength];

  m_status = SUCCESS;

  // See formatV3.txt for explanation of what's written here and why
  uint32 NumHashIters;
  if (m_nHashIters < MIN_HASH_ITERATIONS)
    NumHashIters = MIN_HASH_ITERATIONS;
  else
    NumHashIters = m_nHashIters;

  SAFE_FWRITE(V3TAG, 1, sizeof(V3TAG), m_fd);

  static_assert(int(PWSaltLength) == int(SHA256::HASHLEN),
                "can't call HashRandom256");

  HashRandom256(salt);
  SAFE_FWRITE(salt, 1, sizeof(salt), m_fd);

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
    // See discussion in HashRandom256 to understand why we hash
    // random data instead of writing it directly
    unsigned char ip_rand[SHA256::HASHLEN];
    HashRandom256(ip_rand);
    static_assert(sizeof(ip_rand) >= sizeof(m_ipthing),
                  "m_ipthing can't be more that 32 bytes to use HashRandom256");
    memcpy(m_ipthing, ip_rand, sizeof(m_ipthing));
  }
  SAFE_FWRITE(m_ipthing, 1, sizeof(m_ipthing), m_fd);

  m_fish = new TwoFish(m_key, sizeof(m_key));

  // write some actual data (at last!)
  numWritten = 0;
  // Write version number
  unsigned char vnb[sizeof(VersionNum)];
  vnb[0] = static_cast<unsigned char>(VersionNum & 0xff);
  vnb[1] = static_cast<unsigned char>((VersionNum & 0xff00) >> 8);
  m_hdr.m_nCurrentMajorVersion = static_cast<unsigned short>((VersionNum & 0xff00) >> 8);
  m_hdr.m_nCurrentMinorVersion = static_cast<unsigned short>(VersionNum & 0xff);

  numWritten = WriteCBC(HDR_VERSION, vnb, sizeof(VersionNum));

  if (numWritten <= 0) { m_status = FAILURE; goto end; }

  // Write UUID
  if (m_hdr.m_file_uuid == pws_os::CUUID::NullUUID()) {
    // If not there or zeroed, create new
    CUUID uuid;
    m_hdr.m_file_uuid = uuid;
  }

  numWritten = WriteCBC(HDR_UUID, *m_hdr.m_file_uuid.GetARep(),
                        sizeof(uuid_array_t));
  if (numWritten <= 0) { m_status = FAILURE; goto end; }

  // Write (non default) user preferences
  numWritten = WriteCBC(HDR_NDPREFS, m_hdr.m_prefString.c_str());
  if (numWritten <= 0) { m_status = FAILURE; goto end; }

  // Write out display status
  if (!m_hdr.m_displaystatus.empty()) {
    StringX ds(_T(""));
    vector<bool>::const_iterator iter;
    for (iter = m_hdr.m_displaystatus.begin();
         iter != m_hdr.m_displaystatus.end(); iter++)
      ds += (*iter) ? _T("1") : _T("0");
    numWritten = WriteCBC(HDR_DISPSTAT, ds);
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  // Write out time of this update
  time_t time_now;
  time(&time_now);
  // V3 still uses 32 bit time, so we truncate ruthlessly...
  unsigned char buf[4];
  putInt32(buf, static_cast<int32>(time_now));
  numWritten = WriteCBC(HDR_LASTUPDATETIME, buf, sizeof(buf));
  if (numWritten <= 0) { m_status = FAILURE; goto end; }
  m_hdr.m_whenlastsaved = time_now;

  // Write out last master password change time, if set
  if (m_hdr.m_whenpwdlastchanged != 0) {
    putInt32(buf, static_cast<int32>(m_hdr.m_whenpwdlastchanged));
    numWritten = WriteCBC(HDR_LASTPWDUPDATETIME, buf, sizeof(buf));
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  // Write out who saved it!
  {
    const SysInfo *si = SysInfo::GetInstance();
    const stringT &user = si->GetRealUser();
    const stringT &sysname = si->GetRealHost();
    numWritten = WriteCBC(HDR_LASTUPDATEUSER, user.c_str());
    if (numWritten > 0)
      numWritten = WriteCBC(HDR_LASTUPDATEHOST, sysname.c_str());
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
    m_hdr.m_lastsavedby = user.c_str();
    m_hdr.m_lastsavedon = sysname.c_str();
  }

  // Write out what saved it!
  numWritten = WriteCBC(HDR_LASTUPDATEAPPLICATION,
                        m_hdr.m_whatlastsaved);
  if (numWritten <= 0) { m_status = FAILURE; goto end; }

  if (!m_hdr.m_DB_Name.empty()) {
    numWritten = WriteCBC(HDR_DBNAME, m_hdr.m_DB_Name);
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }
  if (!m_hdr.m_DB_Description.empty()) {
    numWritten = WriteCBC(HDR_DBDESC, m_hdr.m_DB_Description);
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }
  if (!m_MapDBFilters.empty()) {
    coStringXStream oss;  // XML is always char not wchar_t
    m_MapDBFilters.WriteFilterXMLFile(oss, m_hdr, _T(""));
    numWritten = WriteCBC(HDR_FILTERS,
                          reinterpret_cast<const unsigned char *>(oss.str().c_str()),
                          oss.str().length());
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  if (!m_hdr.m_RUEList.empty()) {
    coStringXStream oss;
    size_t num = m_hdr.m_RUEList.size();
    if (num > 255)
      num = 255;  // Do not exceed 2 hex character length field
    oss << setw(2) << setfill('0') << hex << num;
    auto iter = m_hdr.m_RUEList.begin();
    // Only save up to max as defined by FormatV3.
    for (size_t n = 0; n < num; n++, iter++) {
      const uuid_array_t *rep = iter->GetARep();
      for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
        oss << setw(2) << setfill('0') << hex <<  static_cast<unsigned int>((*rep)[i]);
      }
    }

    numWritten = WriteCBC(HDR_RUE,
                          reinterpret_cast<const unsigned char *>(oss.str().c_str()),
                          oss.str().length());
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  // Named Policies
  if (!m_MapPSWDPLC.empty()) {
    oStringXStream oss;
    oss.fill(charT('0'));

    size_t num = m_MapPSWDPLC.size();
    if (num > 255)
      num = 255;  // Do not exceed 2 hex character length field

    oss << setw(2) << hex << num;
    auto iter = m_MapPSWDPLC.begin();
    for (size_t n = 0; n < num; n++, iter++) {
      // The Policy name is limited to 255 characters.
      // This should have been prevented by the GUI.
      // If not, don't write it out as it may cause issues
      if (iter->first.length() > 255)
        continue;

      oss << setw(2) << hex << iter->first.length();
      oss << iter->first.c_str();
      StringX strpwp(iter->second);
      oss << strpwp.c_str();
      if (iter->second.symbols.empty()) {
        oss << _T("00");
      } else {
        oss << setw(2) << hex << iter->second.symbols.length();
        oss << iter->second.symbols.c_str();
      }
    }

    numWritten = WriteCBC(HDR_PSWDPOLICIES, StringX(oss.str().c_str()));
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  // Empty Groups
  for (size_t n = 0; n < m_vEmptyGroups.size(); n++) {
    numWritten = WriteCBC(HDR_EMPTYGROUP, m_vEmptyGroups[n]);
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  for (UnknownFieldList::iterator vi_IterUHFE = m_UHFL.begin();
       vi_IterUHFE != m_UHFL.end(); vi_IterUHFE++) {
    UnknownFieldEntry &unkhfe = *vi_IterUHFE;
    numWritten = WriteCBC(unkhfe.uc_Type,
                          unkhfe.uc_pUField, static_cast<unsigned int>(unkhfe.st_length));
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  if (m_hdr.m_yubi_sk != nullptr) {
    numWritten = WriteCBC(HDR_YUBI_SK, m_hdr.m_yubi_sk, PWSfileHeader::YUBI_SK_LEN);
    if (numWritten <= 0) { m_status = FAILURE; goto end; }
  }

  // Write zero-length end-of-record type item
  numWritten = WriteCBC(HDR_END, nullptr, 0);
  if (numWritten <= 0) { m_status = FAILURE; goto end; }

 end:
  if (m_status != SUCCESS)
    Close();
  return m_status;
}

int PWSfileV3::ReadHeader()
{
  PWS_LOGIT;

  unsigned char Ptag[SHA256::HASHLEN];
  m_status = CheckPasskey(m_filename, m_passkey, m_fd,
                          Ptag, &m_nHashIters);

  if (m_status != SUCCESS) {
    Close();
    return m_status;
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
  bool utf8status;
  unsigned char *utf8 = nullptr;
  size_t utf8Len = 0;
  bool found0302UserHost = false; // to resolve potential conflicts

  do {
    if (ReadCBC(fieldType, utf8, utf8Len) == 0)
      continue;

    switch (fieldType) {
    case HDR_VERSION: /* version */
      // in Beta, VersionNum was an int (4 bytes) instead of short (2)
      // This hack keeps bwd compatibility.
      if (utf8Len != sizeof(VersionNum) &&
          utf8Len != sizeof(int32)) {
        delete[] utf8;
        Close();
        return FAILURE;
      }
      if (utf8[1] !=
          static_cast<unsigned char>((VersionNum & 0xff00) >> 8)) {
        //major version mismatch
        delete[] utf8;
        Close();
        return UNSUPPORTED_VERSION;
      }
      // for now we assume that minor version changes will
      // be backward-compatible
      m_hdr.m_nCurrentMajorVersion = static_cast<unsigned short>(utf8[1]);
      m_hdr.m_nCurrentMinorVersion = static_cast<unsigned short>(utf8[0]);
      break;

    case HDR_UUID: /* UUID */
      if (utf8Len != sizeof(uuid_array_t)) {
        delete[] utf8;
        Close();
        return FAILURE;
      }
      uuid_array_t ua;
      memcpy(ua, utf8, sizeof(ua));
      m_hdr.m_file_uuid = pws_os::CUUID(ua);
      break;

    case HDR_NDPREFS: /* Non-default user preferences */
      if (utf8Len != 0) {
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        StringX pref;
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, pref);
        m_hdr.m_prefString = pref;
        if (!utf8status)
          pws_os::Trace0(_T("FromUTF8(m_prefString) failed\n"));
      } else
        m_hdr.m_prefString = _T("");
      break;

    case HDR_DISPSTAT: /* Tree Display Status */
      if (utf8 != nullptr) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      for (StringX::iterator iter = text.begin(); iter != text.end(); iter++) {
        const TCHAR v = *iter;
        m_hdr.m_displaystatus.push_back(v == TCHAR('1'));
      }
      if (!utf8status)
        pws_os::Trace0(_T("FromUTF8(m_displaystatus) failed\n"));
      break;

    case HDR_LASTUPDATETIME: /* When last saved */
      m_hdr.m_whenlastsaved = 0;
      if (utf8Len == 8) {
        // Handle pre-3.09 implementations that mistakenly
        // stored this as a hex value
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        if (utf8status) {
          iStringXStream is(text);
          is >> hex >> m_hdr.m_whenlastsaved;
          if (is.fail()) {
            // Perhaps 8 byte time_t representation?
            m_hdr.m_whenlastsaved = 0;
          } else {
            pws_os::Trace0(_T("Pre-3.09 HDR_LASTUPDATETIME - hex string found\n"));
          }
        }
      }
      if (m_hdr.m_whenlastsaved == 0) {
        // Here if 8 byte non-hex or != 8 byte
        if (!PWSUtil::pull_time(m_hdr.m_whenlastsaved, utf8, utf8Len))
          pws_os::Trace0(_T("Failed to pull_time(m_whenlastsaved)\n"));
      }
      break;

    case HDR_LASTPWDUPDATETIME: /* when was master password last changed */
      m_hdr.m_whenpwdlastchanged = 0;
      if (!PWSUtil::pull_time(m_hdr.m_whenpwdlastchanged, utf8, utf8Len)) {
        pws_os::Trace0(_T("Failed to pull_time(m_whenpwdlastchanged)\n"));
      }
      break;

    case HDR_LASTUPDATEUSERHOST: /* who last saved */
        // DEPRECATED, but we still know how to read this
        if (!found0302UserHost) { // if new fields also found, don't overwrite
          if (utf8 != nullptr) utf8[utf8Len] = '\0';
          utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
          if (utf8status) {
            StringX tlen = text.substr(0, 4);
            iStringXStream is(tlen);
            unsigned int ulen = 0;
            is >> hex >> ulen;
            StringX uh = text.substr(4);
            m_hdr.m_lastsavedby = uh.substr(0, ulen);
            m_hdr.m_lastsavedon = uh.substr(ulen);
          } else
            pws_os::Trace0(_T("FromUTF8(m_wholastsaved) failed\n"));
        }
        break;

      case HDR_LASTUPDATEAPPLICATION: /* and by what */
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        m_hdr.m_whatlastsaved = text;
        if (!utf8status)
          pws_os::Trace0(_T("FromUTF8(m_whatlastsaved) failed\n"));
        break;

      case HDR_LASTUPDATEUSER:
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        found0302UserHost = true; // so HDR_LASTUPDATEUSERHOST won't override
        m_hdr.m_lastsavedby = text;
        break;

      case HDR_LASTUPDATEHOST:
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        found0302UserHost = true; // so HDR_LASTUPDATEUSERHOST won't override
        m_hdr.m_lastsavedon = text;
        break;

      case HDR_DBNAME:
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        m_hdr.m_DB_Name = text;
        break;

      case HDR_DBDESC:
        if (utf8 != nullptr) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        m_hdr.m_DB_Description = text;
        break;

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
        // Don't support importing XML from non-Windows platforms
        // using Microsoft XML libraries
        // Will be treated as an 'unknown header field' by the 'default' clause below
#else
    case HDR_FILTERS:
      if (utf8 != nullptr) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      if (utf8Len > 0) {
        stringT strErrors;
        stringT XSDFilename = PWSdirs::GetXMLDir() + _T("pwsafe_filter.xsd");
        if (!pws_os::FileExists(XSDFilename)) {
          // No filter schema => user won't be able to access stored filters
          // Inform her of the fact (probably an installation problem).
          stringT message, message2;
          Format(message, IDSC_MISSINGXSD, L"pwsafe_filter.xsd");
          LoadAString(message2, IDSC_FILTERSKEPT);
          message += stringT(_T("\n\n")) + message2;
          if (m_pReporter != nullptr)
            (*m_pReporter)(message);

          // Treat it as an Unknown field!
          // Maybe user used a later version of PWS
          // and we don't want to lose anything
          UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);
          m_UHFL.push_back(unkhfe);
          break;
        }
        int rc = m_MapDBFilters.ImportFilterXMLFile(FPOOL_DATABASE, text.c_str(), _T(""),
                                                    XSDFilename.c_str(),
                                                    strErrors, m_pAsker);
        if (rc != PWScore::SUCCESS) {
          // Can't parse it - treat as an unknown field,
          // Notify user that filter won't be available
          stringT message;
          LoadAString(message, IDSC_CANTPROCESSDBFILTERS);
          if (m_pReporter != nullptr)
            (*m_pReporter)(message);
          pws_os::Trace(L"Error while parsing header filters.\n\tData: %ls\n\tErrors: %ls\n",
                        text.c_str(), strErrors.c_str());
          UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);
          m_UHFL.push_back(unkhfe);
        }
      }
      break;
#endif
      case HDR_RUE:
        {
          if (utf8 != nullptr) utf8[utf8Len] = '\0';
          // All data is character representation of hex - i.e. 0-9a-f
          // No need to convert from char.
          std::string temp = reinterpret_cast<char *>(utf8);

          // Get number of entries
          int num(0);
          std::istringstream is(temp.substr(0, 2));
          is >> hex >> num;

          // verify we have enough data
          if (utf8Len != num * sizeof(uuid_array_t) * 2 + 2)
            break;

          // Get the entries and save them
          size_t j = 2;
          for (int n = 0; n < num; n++) {
            unsigned int x(0);
            uuid_array_t tmp_ua;
            for (size_t i = 0; i < sizeof(uuid_array_t); i++, j += 2) {
              stringstream ss;
              ss.str(temp.substr(j, 2));
              ss >> hex >> x;
              tmp_ua[i] = static_cast<unsigned char>(x);
            }
            const CUUID uuid(tmp_ua);
            if (uuid != CUUID::NullUUID())
              m_hdr.m_RUEList.push_back(uuid);
          }
          break;
        }

      case HDR_YUBI_SK:
        if (utf8Len != PWSfileHeader::YUBI_SK_LEN) {
          delete[] utf8;
          Close();
          return FAILURE;
        }
        m_hdr.m_yubi_sk = new unsigned char[PWSfileHeader::YUBI_SK_LEN];
        memcpy(m_hdr.m_yubi_sk, utf8, PWSfileHeader::YUBI_SK_LEN);
        break;

      case HDR_PSWDPOLICIES:
        /**
         * Very sad situation here: this field code was also assigned to
         * YUBI_SK in 3.27Y. Here we try to infer the actual type based
         * on the actual value stored in the field.
         * Specifically, YUBI_SK is YUBI_SK_LEN bytes of binary data, whereas
         * HDR_PSWDPOLICIES is of varying length, starting with at least 4 hex
         * digits.
         */
        if (utf8Len != PWSfileHeader::YUBI_SK_LEN ||
            (utf8Len >= 4 &&
             isxdigit(utf8[0]) && isxdigit(utf8[1]) &&
             isxdigit(utf8[1]) && isxdigit(utf8[2]))) {
          if (utf8 != nullptr) utf8[utf8Len] = '\0';
          utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
          if (utf8status) {
            const size_t recordlength = text.length();
            StringX sxBlank(_T(" "));  // Needed in case hex value is all zeroes!
            StringX sxTemp;

            // Get number of polices
            sxTemp = text.substr(0, 2) + sxBlank;
            size_t j = 2;  // Skip over # name entries
            iStringXStream is(sxTemp);
            int num(0);
            is >> hex >> num;

            // Get the policies and save them
            for (int n = 0; n < num; n++) {
              if (j > recordlength) break;  // Error

              unsigned int namelength, symbollength;

              sxTemp = text.substr(j, 2) + sxBlank;
              iStringXStream tmp_is(sxTemp);
              j += 2;  // Skip over name length

              tmp_is >> hex >> namelength;
              if (j + namelength > recordlength) break;  // Error

              StringX sxPolicyName = text.substr(j, namelength);
              j += namelength;  // Skip over name
              if (j + 19 > recordlength) break;  // Error

              StringX cs_pwp(text.substr(j, 19));
              PWPolicy pwp(cs_pwp);
              j += 19;  // Skip over pwp

              if (j + 2 > recordlength) break;  // Error
              sxTemp = text.substr(j, 2) + sxBlank;
              tmp_is.str(sxTemp);
              j += 2;  // Skip over symbols length
              tmp_is >> hex >> symbollength;

              StringX sxSymbols;
              if (symbollength != 0) {
                if (j + symbollength > recordlength) break;  // Error
                sxSymbols = text.substr(j, symbollength);
                j += symbollength;  // Skip over symbols
              }
              pwp.symbols = sxSymbols;

              pair< map<StringX, PWPolicy>::iterator, bool > pr;
              pr = m_MapPSWDPLC.insert(PSWDPolicyMapPair(sxPolicyName, pwp));
              if (!pr.second) break; // Error
            }
          }
        } else { // Looks like YUBI_OLD_SK: field length is exactly YUBI_SK_LEN
          //        and at least one non-hex character in first 4 of field.
          m_hdr.m_yubi_sk = new unsigned char[PWSfileHeader::YUBI_SK_LEN];
          memcpy(m_hdr.m_yubi_sk, utf8, PWSfileHeader::YUBI_SK_LEN);
        }
        break;

      case HDR_EMPTYGROUP:
        {
          if (utf8 != nullptr) utf8[utf8Len] = '\0';
          utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
          if (utf8status) {
            m_vEmptyGroups.push_back(text);
          }
          break;
        }

      case HDR_END: /* process END so not to treat it as 'unknown' */
        break;

      default:
        // Save unknown fields that may be addded by future versions
        UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);

        m_UHFL.push_back(unkhfe);
#if 0
#ifdef _DEBUG
        stringT stimestamp;
        PWSUtil::GetTimeStamp(stimestamp);
        pws_os::Trace(_T("Header has unknown field: %02x, length %d/0x%04x, value:\n"),
                      fieldType, utf8Len, utf8Len);
        pws_os::HexDump(utf8, utf8Len, stimestamp);
#endif
#endif
        break;
      }
      delete[] utf8; utf8 = nullptr; utf8Len = 0;
    } while (fieldType != HDR_END);

  // Now sort it for when we compare.
  std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());

    return SUCCESS;
  }

  bool PWSfileV3::IsV3x(const StringX &filename, VERSION &v)
  {
    // This is written so as to support V30, V31, V3x...

    ASSERT(pws_os::FileExists(filename.c_str()));
    FILE *fd = pws_os::FOpen(filename.c_str(), _T("rb"));

    ASSERT(fd != nullptr);
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
