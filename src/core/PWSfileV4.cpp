/*
* Copyright (c) 2013-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PWSfileV4.h"
#include "PWSrand.h"
#include "Util.h"
#include "SysInfo.h"
#include "PWScore.h"
#include "PWSFilters.h"
#include "PWSdirs.h"
#include "PWSprefs.h"
#include "core.h"
#include "pbkdf2.h"
#include "KeyWrap.h"
#include "PWStime.h"
#include "TwoFish.h"

#include "ItemAtt.h" // for WriteContentFields()

#include "os/debug.h"
#include "os/file.h"
#include "os/logit.h"
#include "os/utf8conv.h"

#include "XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#ifdef _WIN32
#include <io.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <iomanip>
#include <algorithm>
#include <type_traits> // for static_assert

using namespace std;
using pws_os::CUUID;

PWSfileV4::CKeyBlocks::KeyBlock::KeyBlock(const KeyBlock &kb)
{
  memcpy(m_salt, kb.m_salt, PWSaltLength);
  m_nHashIters = kb.m_nHashIters;
  memcpy(m_kw_k, kb.m_kw_k, KWLEN);
  memcpy(m_kw_l, kb.m_kw_l, KWLEN);
}

PWSfileV4::CKeyBlocks::KeyBlock &
PWSfileV4::CKeyBlocks::KeyBlock::operator=(const KeyBlock &kb)
{
  if (this != &kb) {
    memcpy(m_salt, kb.m_salt, PWSaltLength);
    m_nHashIters = kb.m_nHashIters;
    memcpy(m_kw_k, kb.m_kw_k, KWLEN);
    memcpy(m_kw_l, kb.m_kw_l, KWLEN);
  }
  return *this;
}

PWSfileV4::CKeyBlocks::CKeyBlocks()
{
}

PWSfileV4::CKeyBlocks::CKeyBlocks(const PWSfileV4::CKeyBlocks &ckb)
  : m_kbs(ckb.m_kbs)
{
}

PWSfileV4::CKeyBlocks::~CKeyBlocks()
{
}

PWSfileV4::CKeyBlocks& PWSfileV4::CKeyBlocks::operator=(const PWSfileV4::CKeyBlocks &that)
{
  if (this != &that) {
    m_kbs = that.m_kbs;
  }
  return *this;
}

PWSfileV4::PWSfileV4(const StringX &filename, RWmode mode, VERSION version)
  : PWSfile(filename, mode, version),
    m_effectiveFileLength(0), m_nHashIters(MIN_HASH_ITERATIONS)
{
  m_IV = m_ipthing;
  m_terminal = nullptr;
  memset(m_key, 0, KLEN);
  memset(m_ell, 0, KLEN);
  memset(m_nonce, 0, NONCELEN);
}

PWSfileV4::~PWSfileV4()
{
  trashMemory(m_key, sizeof(m_key));
  trashMemory(m_ell, sizeof(m_ell));
}

int PWSfileV4::Open(const StringX &passkey)
{
  PWS_LOGIT;

  int status = SUCCESS;

  ASSERT(m_curversion == V40);
  if (passkey.empty()) { // Can happen if db 'locked'
    pws_os::Trace(_T("PWSfileV4::Open(empty_passkey)\n"));
    return WRONG_PASSWORD;
  }

  FOpen();
  if (m_fd == nullptr)
    return CANT_OPEN_FILE;

  m_passkey = passkey;
  if (m_rw == Write) {
    // Nonce is used to detect end of keyblocks
    static_assert(int(NONCELEN) == int(SHA256::HASHLEN), "can't call HashRandom256");
    HashRandom256(m_nonce); // Generate nonce
    if (!m_keyblocks.GetKeys(passkey, m_nHashIters, m_key, m_ell)) {
      PWSfile::Close();
      return WRONG_PASSWORD;
    }
    if (WriteKeyBlocks()) {
      status = WriteHeader();
    } else {
      status = WRITE_FAIL;
    }
  } else { // open for read
    status = ParseKeyBlocks(passkey);
    if (status == SUCCESS)
      status = ReadHeader();
  }
  if (status != SUCCESS) {
    Close();
  } else {
    if (m_rw == Read)
      m_effectiveFileLength = pws_os::fileLength(m_fd) - SHA256::HASHLEN;
  }
  return status;
}

int PWSfileV4::Close()
{
  PWS_LOGIT;

  if (m_fd == nullptr)
    return SUCCESS; // idempotent

  if (!m_hmac.IsInited()) {
    // Here if we're closing before starting to work on hmac
    // e.g., wrong password
    // Clear keyblocks, in case we re-open for read
    m_keyblocks.m_kbs.clear();
    return PWSfile::Close();
  }

  unsigned char digest[SHA256::HASHLEN];
  m_hmac.Final(digest);

  // Write or verify HMAC, depending on RWmode.
  size_t fret;
  if (m_rw == Write) {
    fret = fwrite(digest, sizeof(digest), 1, m_fd);
    if (fret != 1) {
      PWSfile::Close();
      return FAILURE;
    }
    return PWSfile::Close();
  } else { // Read
    // Clear keyblocks, in case we re-open for read
    m_keyblocks.m_kbs.clear();
    // read hmac & verify
    unsigned char d[SHA256::HASHLEN];
    fret = fread(d, sizeof(d), 1, m_fd);
    if (fret != 1) {
      PWSfile::Close();
      return TRUNCATED_FILE;
    }
    if (memcmp(d, digest, SHA256::HASHLEN) == 0)
      return PWSfile::Close();
    else {
      PWSfile::Close();
      return BAD_DIGEST;
    }
  }
}

int PWSfileV4::SanityCheck(FILE *stream)
{
  ASSERT(stream != nullptr);

  // Is file too small?
  const long min_file_length = 232; // pre + post, no hdr or records
  if (pws_os::fileLength(stream) < min_file_length)
    return TRUNCATED_FILE;

  return SUCCESS;
}

int PWSfileV4::CheckPasskey(const StringX &filename,
                            const StringX &passkey, FILE *a_fd,
                            unsigned char *, uint32 *)
{
  PWS_LOGIT;

  FILE *fd = a_fd;
  SHA256 H;

  if (fd == nullptr) {
    fd = pws_os::FOpen(filename.c_str(), _T("rb"));
  }
  if (fd == nullptr)
    return CANT_OPEN_FILE;

  int retval = SanityCheck(fd);
  if (retval == SUCCESS) {
    PWSfileV4 pv4(filename, Read, V40);
    pv4.m_fd = fd;
    retval = pv4.ParseKeyBlocks(passkey);
    pv4.m_fd = nullptr; // s.t. d'tor doesn't fclose()
  }
  if (a_fd == nullptr) // if we opened the file, we close it...
    fclose(fd);
  return retval;
}

size_t PWSfileV4::WriteCBC(unsigned char type, const StringX &data)
{
  const unsigned char *utf8(nullptr);
  size_t utf8Len(0);

  bool status = m_utf8conv.ToUTF8(data, utf8, utf8Len);
  if (!status)
    pws_os::Trace(_T("ToUTF8(%s) failed\n"), data.c_str());
  return WriteCBC(type, utf8, utf8Len);
}

size_t PWSfileV4::WriteCBC(unsigned char type, const unsigned char *data,
                           size_t length)
{
  int32 len32 = static_cast<int>(length);
  unsigned char buf[4];
  putInt32(buf, len32);

  m_hmac.Update(&type, 1);
  m_hmac.Update(buf, sizeof(buf));
  m_hmac.Update(data, (unsigned long)length);

  return PWSfile::WriteCBC(type, data, length);
}

int PWSfileV4::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != nullptr);
  ASSERT(m_curversion == V40);
  return item.Write(this);
}

int PWSfileV4::WriteRecord(const CItemAtt &att)
{
  ASSERT(m_fd != nullptr);
  ASSERT(m_curversion == V40);
  return att.Write(this);
}

  // Following writes AttIV, AttEK, AttAK, AttContent
  // and AttContentHMAC per format spec.
  // All except the content are generated internally.
size_t PWSfileV4::WriteContentFields(unsigned char *content, size_t len)
{
  if (len == 0)
    return SUCCESS;
  ASSERT(content != nullptr);

  unsigned char IV[TwoFish::BLOCKSIZE];
  unsigned char EK[KLEN];
  unsigned char AK[KLEN];

  PWSrand::GetInstance()->GetRandomData(IV, sizeof(IV));
  PWSrand::GetInstance()->GetRandomData(EK, sizeof(EK));
  PWSrand::GetInstance()->GetRandomData(AK, sizeof(AK));

  WriteField(CItemAtt::ATTIV, IV, sizeof(IV));
  WriteField(CItemAtt::ATTEK, EK, sizeof(EK));
  WriteField(CItemAtt::ATTAK, AK, sizeof(AK));

  // Write content length as the "value" of the content field
  int32 len32 = static_cast<int>(len);
  unsigned char buf[4];
  putInt32(buf, len32);
  WriteField(CItemAtt::CONTENT, buf, sizeof(buf));

  // Create fish with EK
  TwoFish fish(EK, sizeof(EK));
  trashMemory(EK, sizeof(EK));

  // Create hmac with AK
  HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> hmac;
  hmac.Init(AK, sizeof(AK));
  trashMemory(AK, sizeof(AK));

  // write actual content using EK
  _writecbc(m_fd, content, len, &fish, IV);

  // update content's HMAC
  hmac.Update(content, (unsigned long)len);

  // write content's HMAC
  unsigned char digest[SHA256::HASHLEN];
  hmac.Final(digest);
  WriteField(CItemAtt::CONTENTHMAC, digest, sizeof(digest));

  return len;
}

size_t PWSfileV4::ReadContent(Fish *fish,  unsigned char *cbcbuffer,
                              unsigned char *&content, size_t clen)
{
  ASSERT(clen > 0 && fish != nullptr && cbcbuffer != nullptr);
  // round up clen to nearest BS:
  const unsigned int BS = fish->GetBlockSize();
  size_t blen = (clen/BS + 1)*BS;

  content = new unsigned char[blen]; // caller's responsible for delete[]
  return _readcbc(m_fd, content, blen, fish, cbcbuffer);
}

size_t PWSfileV4::ReadCBC(unsigned char &type, unsigned char* &data,
                          size_t &length)
{
  size_t numRead = PWSfile::ReadCBC(type, data, length);

  if (numRead > 0) {
    int32 len32 = static_cast<int>(length);
    unsigned char buf[4];
    putInt32(buf, len32);

    m_hmac.Update(&type, 1);
    m_hmac.Update(buf, sizeof(buf));
    m_hmac.Update(data, (unsigned long)length);
  }

  return numRead;
}

void PWSfileV4::SaveState()
{
  m_savepos = ftell(m_fd);
  memcpy(m_saveIV, m_IV, m_fish->GetBlockSize());
  m_savehmac = m_hmac;
}

void PWSfileV4::RestoreState()
{
  int seekstat = fseek(m_fd, m_savepos, SEEK_SET);
  if (seekstat != 0)
    ASSERT(0);
  memcpy(m_IV, m_saveIV, m_fish->GetBlockSize());
  m_hmac = m_savehmac;
}

int PWSfileV4::ReadRecord(CItemData &item)
{
  int status;
  ASSERT(m_fd != nullptr);
  ASSERT(m_curversion == V40);
  SaveState();
  unsigned fpos = unsigned(ftell(m_fd));
  if (fpos < m_effectiveFileLength) {
    status = item.Read(this);
    if (status < 0) { // detected an inappropriate field
      RestoreState();
      status = WRONG_RECORD;
    }
  } else if (fpos == m_effectiveFileLength)
    status = END_OF_FILE;
  else // fpos >= effectiveFileLength !?
    status = READ_FAIL;
  return status;
}

int PWSfileV4::ReadRecord(CItemAtt &att)
{
  ASSERT(m_fd != nullptr);
  ASSERT(m_curversion == V40);
  if (unsigned(GetOffset()) < m_effectiveFileLength)
    return att.Read(this);
  else
    return END_OF_FILE;
}

void PWSfileV4::StretchKey(const unsigned char *salt, unsigned long saltLen,
                           const StringX &passkey,
                           unsigned int N, unsigned char *Ptag, unsigned long PtagLen)
{
  /*
  * P' is the "stretched key" of the user's passphrase and the SALT, as defined
  * by the hash-function-based key stretching algorithm PBKDF2, with SHA-256
  * as the hash function, and N iterations.
  */
  ASSERT(N >= MIN_HASH_ITERATIONS); // minimal value we're willing to use
  size_t passLen = 0;
  unsigned char *pstr = nullptr;

  HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> hmac;
  ConvertPasskey(passkey, pstr, passLen);
  pbkdf2(pstr, (unsigned long)passLen, salt, saltLen, N, &hmac, Ptag, &PtagLen);

#ifdef UNICODE
  trashMemory(pstr, passLen);
  delete[] pstr;
#endif
}

const short VersionNum = 0x0400;

struct PWSfileV4::CKeyBlocks::KeyBlockFinder {
  KeyBlockFinder(const StringX &passkey) : passkey(passkey) {}

  bool operator()(const KeyBlock &kb) {
    unsigned char Ptag[SHA256::HASHLEN];
    unsigned char K[PWSfileV4::KLEN];
    PWSfileV4::StretchKey(kb.m_salt, sizeof(kb.m_salt), passkey, kb.m_nHashIters,
                          Ptag, sizeof(Ptag));
    // Try to unwrap K
    TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well
    KeyWrap kwK(&Fish);
    
    bool retval = kwK.Unwrap(kb.m_kw_k, K, sizeof(kb.m_kw_k));
    if (retval) {
      trashMemory(Ptag, sizeof(Ptag));
      trashMemory(K, sizeof(K));
    }
    return retval;
  }
private:
  KeyBlockFinder& operator=(const KeyBlockFinder&); // Do not implement
  const StringX &passkey;
};

bool PWSfileV4::CKeyBlocks::GetKeys(const StringX &passkey, uint32 nHashIters,
                                     unsigned char K[KLEN], unsigned char L[KLEN])
{
  // Note that nHashIters is only used if m_kbs is empty
  // Which will happen if this file is used 'single user'
  // and not with an externally managed CKeyBlocks.
  
  if (m_kbs.empty())
    AddKeyBlock(passkey, passkey, nHashIters);

  KeyBlockFinder find_kb(passkey);
  auto kb_iter = find_if(m_kbs.begin(), m_kbs.end(), find_kb);
  if (kb_iter == m_kbs.end())
    return false;

  unsigned char Ptag[SHA256::HASHLEN];
  PWSfileV4::StretchKey(kb_iter->m_salt, sizeof(kb_iter->m_salt),
                        passkey, kb_iter->m_nHashIters,
                        Ptag, sizeof(Ptag));
  TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well
  KeyWrap kwK(&Fish);
  if (!kwK.Unwrap(kb_iter->m_kw_k, K, sizeof(kb_iter->m_kw_k)))
    ASSERT(0);
  KeyWrap kwL(&Fish);
  if (!kwL.Unwrap(kb_iter->m_kw_l, L, sizeof(kb_iter->m_kw_l)))
    ASSERT(0);
  return true;
}

void PWSfileV4::ComputeEndKB(const unsigned char hnonce[SHA256::HASHLEN],
                             unsigned char digest[SHA256::HASHLEN])
{
  m_hmac.Init(m_ell, sizeof(m_ell));
  // Iterate over all KeyBlocks, calculate endKB
  for (auto iter = m_keyblocks.m_kbs.begin(); iter != m_keyblocks.m_kbs.end(); iter++) {
    m_hmac.Update(iter->m_salt, CKeyBlocks::PWSaltLength);
    unsigned char Nb[sizeof(iter->m_nHashIters)];
    putInt32(Nb, iter->m_nHashIters);
    m_hmac.Update(Nb, sizeof(Nb));
    m_hmac.Update(iter->m_kw_k, CKeyBlocks::KWLEN);
    m_hmac.Update(iter->m_kw_l, CKeyBlocks::KWLEN);
  }
  // Add hash of nonce
  m_hmac.Update(hnonce, SHA256::HASHLEN);
  m_hmac.Final(digest);
}

#define SAFE_FWRITE(p, sz, cnt, stream) \
  { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { status = FAILURE; goto end;} \
  }

struct PWSfileV4::KeyBlockWriter
{
  KeyBlockWriter(FILE *fd) : m_ok(true), m_fd(fd)
  {}
  void operator()(const PWSfileV4::CKeyBlocks::KeyBlock &kb)
  {
    write(kb.m_salt, sizeof(kb.m_salt));
    unsigned char Nb[sizeof(kb.m_nHashIters)];
    putInt32(Nb, kb.m_nHashIters);
    write(Nb, sizeof(Nb));
    write(kb.m_kw_k, sizeof(kb.m_kw_k));
    write(kb.m_kw_l, sizeof(kb.m_kw_l));
  }
  bool m_ok;
private:
  FILE *m_fd;
  void write(const void *p, size_t size)
  {
    // First time we fail, m_ok goes to false and we stop trying.
    if (m_ok) {
      size_t nw = fwrite(p, size, 1, m_fd);
      if (nw != 1)
        m_ok = false; // first time this is false, we stop writing!
    }
  }
};

bool PWSfileV4::WriteKeyBlocks()
{
  size_t nw;

  nw = fwrite(m_nonce, NONCELEN, 1, m_fd);
  if (nw != 1)
    return false;

  KeyBlockWriter kbw(m_fd);
  for_each(m_keyblocks.m_kbs.begin(), m_keyblocks.m_kbs.end(), kbw);
  if (!kbw.m_ok)
    return false;

  // Mark end of keyblocks
  unsigned char hnonce[SHA256::HASHLEN];
  SHA256 noncehasher;
  noncehasher.Update(m_nonce, NONCELEN);
  noncehasher.Final(hnonce);
  nw = fwrite(hnonce, sizeof(hnonce), 1, m_fd);
  if (nw != 1)
    return false;

  unsigned char digest[SHA256::HASHLEN];
  ComputeEndKB(hnonce, digest);
  nw = fwrite(digest, sizeof(digest), 1, m_fd);
  return (nw == 1);
}

int PWSfileV4::WriteHeader()
{
  PWS_LOGIT;

  // Following code is divided into {} blocks to
  // prevent "uninitialized" compile errors, as we use
  // goto for error handling
  int status = SUCCESS;
  size_t numWritten = 0;
  m_hmac.Init(m_ell, sizeof(m_ell)); // re-init for header & data integrity
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

  if (numWritten <= 0) { status = FAILURE; goto end; }

  // Write UUID
  if (m_hdr.m_file_uuid == pws_os::CUUID::NullUUID()) {
    // If not there or zeroed, create new
    CUUID uuid;
    m_hdr.m_file_uuid = uuid;
  }

  numWritten = WriteCBC(HDR_UUID, *m_hdr.m_file_uuid.GetARep(),
                        sizeof(uuid_array_t));
  if (numWritten <= 0) { status = FAILURE; goto end; }

  // Write (non default) user preferences
  numWritten = WriteCBC(HDR_NDPREFS, m_hdr.m_prefString.c_str());
  if (numWritten <= 0) { status = FAILURE; goto end; }

  // Write out display status
  if (!m_hdr.m_displaystatus.empty()) {
    StringX ds(_T(""));
    vector<bool>::const_iterator iter;
    for (iter = m_hdr.m_displaystatus.begin();
         iter != m_hdr.m_displaystatus.end(); iter++)
      ds += (*iter) ? _T("1") : _T("0");
    numWritten = WriteCBC(HDR_DISPSTAT, ds);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  // Write out time of this update
  {
    PWStime pwt; // c'tor set current time
    numWritten = WriteCBC(HDR_LASTUPDATETIME, pwt, pwt.GetLength());
    if (numWritten <= 0) { status = FAILURE; goto end; }
    m_hdr.m_whenlastsaved = pwt;
  }

  // Write out last master password change time, if set
  if (m_hdr.m_whenpwdlastchanged != 0) {
    PWStime pwt(m_hdr.m_whenpwdlastchanged);
    numWritten = WriteCBC(HDR_LASTPWDUPDATETIME, pwt, pwt.GetLength());
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
    if (numWritten <= 0) { status = FAILURE; goto end; }
    m_hdr.m_lastsavedby = user.c_str();
    m_hdr.m_lastsavedon = sysname.c_str();
  }

  // Write out what saved it!
  numWritten = WriteCBC(HDR_LASTUPDATEAPPLICATION,
                        m_hdr.m_whatlastsaved);
  if (numWritten <= 0) { status = FAILURE; goto end; }

  if (!m_hdr.m_DB_Name.empty()) {
    numWritten = WriteCBC(HDR_DBNAME, m_hdr.m_DB_Name);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }
  if (!m_hdr.m_DB_Description.empty()) {
    numWritten = WriteCBC(HDR_DBDESC, m_hdr.m_DB_Description);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }
  if (!m_MapDBFilters.empty()) {
    coStringXStream oss;  // XML is always char not wchar_t
    m_MapDBFilters.WriteFilterXMLFile(oss, m_hdr, _T(""));
    numWritten = WriteCBC(HDR_FILTERS,
                          reinterpret_cast<const unsigned char *>(oss.str().c_str()),
                          oss.str().length());
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  if (!m_hdr.m_RUEList.empty()) {
    size_t num = m_hdr.m_RUEList.size();
    if (num > 255)
      num = 255;  // Only save up to max as defined by FormatV3.

    size_t buflen = (num * sizeof(uuid_array_t)) + 1;
    auto *buf = new unsigned char[buflen];
    buf[0] = (unsigned char)num;
    unsigned char *buf_ptr = buf + 1;

    auto iter = m_hdr.m_RUEList.begin();
    
    for (size_t n = 0; n < num; n++, iter++) {
      const uuid_array_t *rep = iter->GetARep();
      memcpy(buf_ptr, rep, sizeof(uuid_array_t));
      buf_ptr += sizeof(uuid_array_t);
    }

    numWritten = WriteCBC(HDR_RUE, buf, buflen);
    trashMemory(buf, buflen);
    delete[] buf;
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  // Named Policies
  if (!m_MapPSWDPLC.empty()) {
    size_t numPols = m_MapPSWDPLC.size();

    if (numPols > 255)
      numPols = 255;  // Do not exceed a single byte

    // Quick iteration to figure out total size
    PSWDPolicyMapIter iter;
    size_t totlen = 1; // 1 byte for num of policies
    for (iter = m_MapPSWDPLC.begin(); iter != m_MapPSWDPLC.end(); iter++) {
      size_t polNameLen = pws_os::wcstombs(nullptr, 0, iter->first.c_str(), iter->first.length());
      size_t symSetLen = ((iter->second.symbols.empty()) ?
                          0 : pws_os::wcstombs(nullptr, 0, iter->second.symbols.c_str(), iter->second.symbols.length()));
      totlen +=
        1 +                            // length of policy name
        polNameLen +
        2 +                            // flags
        2*5 +                          // various lengths
        1 +                            // length of special symbol set
        symSetLen;
    }

    // Allocate buffer in calculated size
    auto *buf = new unsigned char[totlen];
    memset(buf, 0, totlen); // in case we truncate some names, don't leak info.

    // fill buffer
    buf[0] = (unsigned char)numPols;
    unsigned char *buf_ptr = buf + 1;
    for (iter = m_MapPSWDPLC.begin(); iter != m_MapPSWDPLC.end(); iter++) {
      size_t polNameLen = pws_os::wcstombs(nullptr, 0, iter->first.c_str(), iter->first.length());
      if (polNameLen > 255) // too bad if too long...
        polNameLen = 255;
      *buf_ptr++ = (unsigned char)polNameLen;
      pws_os::wcstombs((char *)buf_ptr, polNameLen, iter->first.c_str(), iter->first.length());
      buf_ptr += polNameLen;

      const PWPolicy &pwpol = iter->second;
      putInt16(buf_ptr, pwpol.flags);            buf_ptr += 2;
      putInt16(buf_ptr, uint16(pwpol.length));           buf_ptr += 2;
      putInt16(buf_ptr, uint16(pwpol.lowerminlength));   buf_ptr += 2;
      putInt16(buf_ptr, uint16(pwpol.upperminlength));   buf_ptr += 2;
      putInt16(buf_ptr, uint16(pwpol.digitminlength));   buf_ptr += 2;
      putInt16(buf_ptr, uint16(pwpol.symbolminlength));  buf_ptr += 2;

      if (pwpol.symbols.empty()) {
        *buf_ptr++ = 0;
      } else {
        size_t symSetLen = pws_os::wcstombs(nullptr, 0,
                                          pwpol.symbols.c_str(), pwpol.symbols.length());
      if (symSetLen > 255) // too bad if too long...
        symSetLen = 255;
      *buf_ptr++ = (unsigned char)symSetLen;
      pws_os::wcstombs((char *)buf_ptr, symSetLen,
                       pwpol.symbols.c_str(), pwpol.symbols.length());
      buf_ptr += symSetLen;
      }
    } // for loop over policies

    ASSERT(buf_ptr == buf + totlen); // check our math...
    numWritten = WriteCBC(HDR_PSWDPOLICIES, buf, totlen);
    trashMemory(buf, totlen);
    delete[] buf;
    if (numWritten <= 0) { status = FAILURE; goto end; }
  } // named policies

  // Empty Groups
  for (size_t n = 0; n < m_vEmptyGroups.size(); n++) {
    numWritten = WriteCBC(HDR_EMPTYGROUP, m_vEmptyGroups[n]);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  for (UnknownFieldList::iterator vi_IterUHFE = m_UHFL.begin();
       vi_IterUHFE != m_UHFL.end(); vi_IterUHFE++) {
    UnknownFieldEntry &unkhfe = *vi_IterUHFE;
    numWritten = WriteCBC(unkhfe.uc_Type,
                          unkhfe.uc_pUField, static_cast<unsigned int>(unkhfe.st_length));
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  if (m_hdr.m_yubi_sk != nullptr) {
    numWritten = WriteCBC(HDR_YUBI_SK, m_hdr.m_yubi_sk, PWSfileHeader::YUBI_SK_LEN);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  // Write zero-length end-of-record type item
  numWritten = WriteCBC(HDR_END, nullptr, 0);
  if (numWritten <= 0) { status = FAILURE; goto end; }
 end:
  if (status != SUCCESS)
    Close();
  return status;
}

int PWSfileV4::ReadKeyBlock()
{
  /*
   * Reads a keyblock, pushes it on m_keyblocks.
   * If couldn't read anything, returns END_OF_FILE
   * without pushing.
   */
  CKeyBlocks::KeyBlock kb;
  size_t nRead;
  nRead = fread(kb.m_salt, sizeof(kb.m_salt), 1, m_fd);
  if (nRead == 0) return END_OF_FILE;
  unsigned char Nb[sizeof(uint32)];

  nRead = fread(Nb, sizeof(Nb), 1, m_fd);
  if (nRead == 0) return END_OF_FILE;
  kb.m_nHashIters = getInt32(Nb);

  nRead = fread(kb.m_kw_k, CKeyBlocks::KWLEN, 1, m_fd);
  if (nRead == 0) return END_OF_FILE;

  nRead = fread(kb.m_kw_l, CKeyBlocks::KWLEN, 1, m_fd);
  if (nRead == 0) return END_OF_FILE;

  m_keyblocks.m_kbs.push_back(kb);

  return SUCCESS;
}

int PWSfileV4::TryKeyBlock(unsigned index, const StringX &passkey,
                           unsigned char K[KLEN], unsigned char L[KLEN],
                           uint32 &nHashIters)
{
  CKeyBlocks::KeyBlock &kb = m_keyblocks.at(index);
  unsigned char Ptag[SHA256::HASHLEN];

  StretchKey(kb.m_salt, sizeof(kb.m_salt), passkey, kb.m_nHashIters,
             Ptag, sizeof(Ptag));
  // Try to unwrap K
  TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well
  KeyWrap kwK(&Fish);

  if (!kwK.Unwrap(kb.m_kw_k, K, sizeof(kb.m_kw_k)))
    return WRONG_PASSWORD;
      
  KeyWrap kwL(&Fish);
  if (!kwL.Unwrap(kb.m_kw_l, L, sizeof(kb.m_kw_l))) {
    ASSERT(0); // Shouln't happen if K unwrapped OK
    return WRONG_PASSWORD;
  }
  nHashIters = kb.m_nHashIters;
  return SUCCESS;
}

bool PWSfileV4::VerifyKeyBlocks()
{
  unsigned char hnonce[SHA256::HASHLEN];
  unsigned char ReadEndKB[SHA256::HASHLEN];
  unsigned char CalcEndKB[SHA256::HASHLEN];

  int nRead = (int)fread(hnonce, sizeof(hnonce), 1, m_fd);
  if (nRead != 1)
    return false;
  nRead = (int)fread(ReadEndKB, sizeof(ReadEndKB), 1, m_fd);
  if (nRead != 1)
    return false;

  ComputeEndKB(hnonce, CalcEndKB);
  return (memcmp(CalcEndKB, ReadEndKB, SHA256::HASHLEN) == 0);
}

bool PWSfileV4::EndKeyBlocks(const unsigned char calc_hnonce[SHA256::HASHLEN])
{
  unsigned char read_hnonce[SHA256::HASHLEN];
  size_t nr = fread(read_hnonce, sizeof(read_hnonce), 1, m_fd);
  if (nr != 1)
    return false; // EOF will be hit again and reported later
  // go back regardless of success/failure
  fseek(m_fd, -long(sizeof(read_hnonce)), SEEK_CUR);
  return (memcmp(calc_hnonce, read_hnonce, SHA256::HASHLEN) == 0);
}

int PWSfileV4::ParseKeyBlocks(const StringX &passkey)
{
  PWS_LOGIT;
  /**
   * We need to read in all keyblocks,
   * and find one that works.
   * "All" means running until Hash(m_nonce) detected
   * or EOF.
   * "works" means TryKeyBlock returns true.
   * Once we have a working keyblock, we can verify the integrity
   * of all keyblocks.
   * Consider that we'll hit EOF if file's wrong type/corrupt
   */
  int status;
  unsigned char calc_hnonce[SHA256::HASHLEN];
  SHA256 noncehasher;

  // Start by reading in nonce
  size_t nr = fread(m_nonce, NONCELEN, 1, m_fd);
  if (nr != 1)
    return READ_FAIL;

  noncehasher.Update(m_nonce, NONCELEN);
  noncehasher.Final(calc_hnonce);

  do {
    status = ReadKeyBlock();
    // status is either SUCCESS or END_OF_FILE
    if (status == END_OF_FILE) {
      return status;
    }
  } while (!EndKeyBlocks(calc_hnonce));

  for (unsigned i = 0; i < m_keyblocks.size(); i++) {
    status = TryKeyBlock(i, passkey, m_key, m_ell, m_nHashIters);
    if (status == SUCCESS) {
      if (!VerifyKeyBlocks())
        status = BAD_DIGEST;
      break;
    }
  }
  return status;
}

bool PWSfileV4::CKeyBlocks::AddKeyBlock(const StringX &current_passkey,
                                        const StringX &new_passkey,
                                        uint nHashIters)
{
  unsigned char Ptag[SHA256::HASHLEN];
  unsigned char K[KLEN];
  unsigned char L[KLEN];
  KeyBlock kb;
  kb.m_nHashIters = nHashIters;
  HashRandom256(kb.m_salt);

  if (m_kbs.empty()) { // we get to generate new K and L
    PWSrand::GetInstance()->GetRandomData(K, KLEN);
    PWSrand::GetInstance()->GetRandomData(L, KLEN);

    StretchKey(kb.m_salt, sizeof(kb.m_salt), current_passkey, kb.m_nHashIters,
               Ptag, sizeof(Ptag));
  } else { // we need to get K & L from current
    KeyBlockFinder find_kb(current_passkey);
    auto kb_iter = find_if(m_kbs.begin(), m_kbs.end(), find_kb);
    if (kb_iter == m_kbs.end())
      return false;
    PWSfileV4::StretchKey(kb_iter->m_salt, sizeof(kb_iter->m_salt),
                          current_passkey, kb_iter->m_nHashIters,
                          Ptag, sizeof(Ptag));
    TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well
    KeyWrap kwK(&Fish);
    kwK.Unwrap(kb_iter->m_kw_k, K, sizeof(kb_iter->m_kw_k));
    KeyWrap kwL(&Fish);
    kwL.Unwrap(kb_iter->m_kw_l, L, sizeof(kb_iter->m_kw_l));

    StretchKey(kb.m_salt, sizeof(kb.m_salt), new_passkey, kb.m_nHashIters,
               Ptag, sizeof(Ptag));
  }
    
  TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well

  KeyWrap kwK(&Fish);
  kwK.Wrap(K, kb.m_kw_k, KLEN);

  KeyWrap kwL(&Fish);
  kwL.Wrap(L, kb.m_kw_l, KLEN);

  trashMemory(Ptag, sizeof(Ptag));
  trashMemory(K, KLEN);
  trashMemory(L, KLEN);
  m_kbs.push_back(kb);
  return true;
}

bool PWSfileV4::CKeyBlocks::RemoveKeyBlock(const StringX &passkey)
{
  // fails if m_keyblocks.size() <= 1...
  // ... or if passkey doesn't match any keyblock
  if (m_kbs.size() <= 1)
    return false;

  KeyBlockFinder find_kb(passkey);
  const auto old_size = m_kbs.size();
  m_kbs.erase(remove_if(m_kbs.begin(), m_kbs.end(), find_kb),
               m_kbs.end());

  return (m_kbs.size() != old_size);
}

int PWSfileV4::ReadHeader()
{
  m_hmac.Init(m_ell, sizeof(m_ell));
  size_t nIPread = fread(m_ipthing, sizeof(m_ipthing), 1, m_fd);
  if (nIPread != 1) {
    Close();
    return TRUNCATED_FILE;
  }

  m_fish = new TwoFish(m_key, sizeof(m_key));

  unsigned char fieldType;
  StringX text;
  bool utf8status;
  unsigned char *utf8 = nullptr;
  size_t utf8Len = 0;

  do {
    size_t numRead = ReadCBC(fieldType, utf8, utf8Len);
    if (numRead == 0)
      return TRUNCATED_FILE;

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
      ASSERT(utf8Len == PWStime::TIME_LEN); // V4 header only needs to deal with PWStime 40 bit representation
      if (utf8Len == PWStime::TIME_LEN) { // fail silently in Release build if not 
        PWStime pwt(utf8);
        m_hdr.m_whenlastsaved = pwt;
      } else {
        m_hdr.m_whenlastsaved = 0;
      }
      break;

    case HDR_LASTPWDUPDATETIME: /* when was master password last changed */
      ASSERT(utf8Len == PWStime::TIME_LEN); // V4 header only needs to deal with PWStime 40 bit representation
      if (utf8Len == PWStime::TIME_LEN) { // fail silently in Release build if not 
        PWStime pwt(utf8);
        m_hdr.m_whenpwdlastchanged = pwt;
      } else {
        m_hdr.m_whenlastsaved = 0;
      }
      
      break;
    case HDR_LASTUPDATEUSERHOST: /* and by whom */
      // DEPRECATED, should never appear in a V4 format file header
      ASSERT(0);
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
      m_hdr.m_lastsavedby = text;
      break;

    case HDR_LASTUPDATEHOST:
      if (utf8 != nullptr) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
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
          Format(message, IDSC_MISSINGXSD, _T("pwsafe_filter.xsd"));
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

          UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);
          m_UHFL.push_back(unkhfe);
        }
      }
      break;
#endif

    case HDR_RUE:
      {
        // All data is binary
        // Get number of entries
        int num = utf8[0];

        // verify we have enough data
        if (utf8Len != num * sizeof(uuid_array_t) + 1)
          break;

        // Get the entries and save them
        unsigned char *buf = utf8 + 1;
        for (int n = 0; n < num; n++, buf += sizeof(uuid_array_t)) {
          uuid_array_t uax = {0};
          memcpy(uax, buf, sizeof(uuid_array_t));
          const CUUID uuid(uax);
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
      {
        const size_t minPolLen = 1 + 1 + 1 + 2 + 2*5 + 1; // see formatV4.txt
        if (utf8Len < minPolLen)
          break; // Error

        unsigned char *buf_ptr = utf8;
        const unsigned char *max_ptr = utf8 + utf8Len; // for sanity checks
        int num = *buf_ptr++;

        // Get the policies and save them
        for (int n = 0; n < num; n++) {
          StringX sxPolicyName;
          PWPolicy pwp;

          int nameLen = *buf_ptr++;
          // need to tack on null byte to name before conversion
          auto *nmbuf = new unsigned char[nameLen + 1];
          memcpy(nmbuf, buf_ptr, nameLen); nmbuf[nameLen] = 0;
          utf8status = m_utf8conv.FromUTF8(nmbuf, nameLen, sxPolicyName);
          trashMemory(nmbuf, nameLen); delete[] nmbuf;
          if (!utf8status)
            continue;
          buf_ptr += nameLen;
          if (buf_ptr > max_ptr)
            break; // Error
          pwp.flags = getInt16(buf_ptr);            buf_ptr += 2;
          pwp.length = getInt16(buf_ptr);           buf_ptr += 2;
          pwp.lowerminlength = getInt16(buf_ptr);   buf_ptr += 2;
          pwp.upperminlength = getInt16(buf_ptr);   buf_ptr += 2;
          pwp.digitminlength = getInt16(buf_ptr);   buf_ptr += 2;
          pwp.symbolminlength = getInt16(buf_ptr);  buf_ptr += 2;
          if (buf_ptr > max_ptr)
            break; // Error
          int symLen = *buf_ptr++;
          if (symLen > 0) {
            // need to tack on null byte to symbols before conversion
            auto *symbuf = new unsigned char[symLen + 1];
            memcpy(symbuf, buf_ptr, symLen); symbuf[symLen] = 0;
            utf8status = m_utf8conv.FromUTF8(symbuf, symLen, pwp.symbols);
            trashMemory(symbuf, symLen); delete[] symbuf;
            if (!utf8status)
              continue;
            buf_ptr += symLen;
          }
          if (buf_ptr > max_ptr)
            break; // Error
          pair< map<StringX, PWPolicy>::iterator, bool > pr;
          pr = m_MapPSWDPLC.insert(PSWDPolicyMapPair(sxPolicyName, pwp));
          if (!pr.second) break; // Error
        } // iterate over named policies
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

bool PWSfileV4::IsV4x(const StringX &filename, const StringX &passkey, VERSION &v)
{
  if (CheckPasskey(filename, passkey) == SUCCESS) {
    v = V40;
    return true;
  } else
    return false;
}
