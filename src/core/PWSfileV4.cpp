/*
* Copyright (c) 2003-2013 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include <algorithm>

using namespace std;
using pws_os::CUUID;

PWSfileV4::PWSfileV4(const StringX &filename, RWmode mode, VERSION version)
: PWSfile(filename, mode, version), m_keyblocks(1), m_current_keyblock(0),
  m_effectiveFileLength(0)
{
  m_IV = m_ipthing;
  m_terminal = NULL;
}

PWSfileV4::~PWSfileV4()
{
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
  if (m_fd == NULL)
    return CANT_OPEN_FILE;

  m_passkey = passkey;
  if (m_rw == Write) {
    SetupKeyBlocksForWrite();
    size_t numWritten = WriteKeyBlocks();
    if (numWritten != (PWSaltLength + sizeof(uint32) + 2 * KWLEN) * m_keyblocks.size() + SHA256::HASHLEN) {
      status = WRITE_FAIL;
    } else {
      status = WriteHeader();
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

  if (m_fd == NULL)
    return SUCCESS; // idempotent

  if (!m_hmac.IsInited()) {
    // Here if we're closing before starting to work on hmac
    // e.g., wrong password
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
    // and detected (by _readcbc) - just read hmac & verify
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
  ASSERT(stream != NULL);

  // Is file too small?
  const long min_file_length = 232; // pre + post, no hdr or records
  if (pws_os::fileLength(stream) < min_file_length)
    return TRUNCATED_FILE;

  return SUCCESS;
}

int PWSfileV4::CheckPasskey(const StringX &filename,
                            const StringX &passkey, FILE *a_fd,
                            unsigned char *aPtag, uint32 *nITER)
{
  PWS_LOGIT;

  FILE *fd = a_fd;
  int retval = SUCCESS;
  SHA256 H;

  if (fd == NULL) {
    fd = pws_os::FOpen(filename.c_str(), _T("rb"));
  }
  if (fd == NULL)
    return CANT_OPEN_FILE;

  retval = SanityCheck(fd);
  if (retval == SUCCESS) {
    PWSfileV4 pv4(filename, Read, V40);
    pv4.m_fd = fd;
    retval = pv4.ParseKeyBlocks(passkey);
  }
  if (a_fd == NULL) // if we opened the file, we close it...
    fclose(fd);
  return retval;
}

size_t PWSfileV4::WriteCBC(unsigned char type, const StringX &data)
{
  bool status;
  const unsigned char *utf8;
  size_t utf8Len;
  status = m_utf8conv.ToUTF8(data, utf8, utf8Len);
  if (!status)
    pws_os::Trace(_T("ToUTF8(%s) failed\n"), data.c_str());
  return WriteCBC(type, utf8, utf8Len);
}

size_t PWSfileV4::WriteCBC(unsigned char type, const unsigned char *data,
                           size_t length)
{
  m_hmac.Update(data, reinterpret_cast<int &>(length));
  return PWSfile::WriteCBC(type, data, length);
}

int PWSfileV4::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V40);
  return item.Write(this);
}

size_t PWSfileV4::ReadCBC(unsigned char &type, unsigned char* &data,
                          size_t &length)
{
  size_t numRead = PWSfile::ReadCBC(type, data, length);

  if (numRead > 0) {
    m_hmac.Update(data, reinterpret_cast<unsigned long &>(length));
  }

  return numRead;
}

int PWSfileV4::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V40);
  if (ftell(m_fd) < m_effectiveFileLength)
    return item.Read(this);
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
  unsigned char *pstr = NULL;

  HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> hmac;
  ConvertString(passkey, pstr, passLen);
  pbkdf2(pstr, passLen, salt, saltLen, N, &hmac, Ptag, &PtagLen);

#ifdef UNICODE
  trashMemory(pstr, passLen);
  delete[] pstr;
#endif
}

const short VersionNum = 0x0400;

uint32 PWSfileV4::GetNHashIters() const
{
  // Using "at" to check bounds
  return m_keyblocks.at(m_current_keyblock).m_nHashIters;
}

void PWSfileV4::SetNHashIters(uint32 N)
{
  // Using "at" to check bounds
  m_keyblocks.at(m_current_keyblock).m_nHashIters = N;
}


void PWSfileV4::SetupKeyBlocksForWrite()
{
  /*
   * We only need to deal with the current KeyBlock, as
   * the rest are read in and written out verbatim.
   * 
   * If we have exactly one KeyBlock, we can regenerate Salt, K, L and wrapped keys.
   * If we have > 1, we get K and L from the current KeyBlock.
   *
   * Once we have these, we iterate over all 
   * KeyBlocks and calculate endKB.
   */

  ASSERT(!m_keyblocks.empty()); // PWSfileV4's c'tor assures us that we'll have at least one.
  ASSERT(m_current_keyblock < m_keyblocks.size());

  KeyBlock &kb = m_keyblocks[m_current_keyblock];
  unsigned char Ptag[SHA256::HASHLEN];

  if (m_keyblocks.size() == 1) {
    // According to the spec, salt is just random data. I don't think though,
    // that it's good practice to directly expose the generated randomness
    // to the attacker. Therefore, we'll hash the salt.
    // The following takes shameless advantage of the fact that
    // PWSaltLength == SHA256::HASHLEN
    ASSERT(PWSaltLength == SHA256::HASHLEN); // if false, have to recode
    PWSrand::GetInstance()->GetRandomData(kb.m_salt, sizeof(kb.m_salt));
    SHA256 salter;
    salter.Update(kb.m_salt, sizeof(kb.m_salt));
    salter.Final(kb.m_salt);

    StretchKey(kb.m_salt, sizeof(kb.m_salt), m_passkey, kb.m_nHashIters,
               Ptag, sizeof(Ptag));
    
    PWSrand::GetInstance()->GetRandomData(m_key, sizeof(m_key)); // K
    // Wrap K
    TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well
    KeyWrap kwK(&Fish);

    kwK.Wrap(m_key, kb.m_kw_k, sizeof(m_key));
      
    PWSrand::GetInstance()->GetRandomData(m_ell, sizeof(m_ell));
    KeyWrap kwL(&Fish);
    kwL.Wrap(m_ell, kb.m_kw_l, sizeof(m_ell));
  } else { // we've more than one KeyBlock, get current values
    StretchKey(kb.m_salt, sizeof(kb.m_salt), m_passkey, kb.m_nHashIters,
               Ptag, sizeof(Ptag));
    TwoFish Fish(Ptag, sizeof(Ptag)); // XXX generalize to support AES as well
    KeyWrap kwK(&Fish);
    if (!kwK.Unwrap(kb.m_kw_k, m_key, sizeof(kb.m_kw_k)))
      ASSERT(0);
    KeyWrap kwL(&Fish);
    if (!kwL.Unwrap(kb.m_kw_l, m_ell, sizeof(kb.m_kw_l)))
      ASSERT(0);
  }
}

void PWSfileV4::ComputeEndKB(unsigned char digest[SHA256::HASHLEN])
{
  m_hmac.Init(m_ell, sizeof(m_ell));
  // Iterate over all KeyBlocks, calculate endKB
  vector<KeyBlock>::iterator iter;
  for (iter = m_keyblocks.begin(); iter != m_keyblocks.end(); iter++) {
    m_hmac.Update(iter->m_salt, PWSaltLength);
    unsigned char Nb[sizeof(iter->m_nHashIters)];
    putInt32(Nb, iter->m_nHashIters);
    m_hmac.Update(Nb, sizeof(Nb));
    m_hmac.Update(iter->m_kw_k, KWLEN);
    m_hmac.Update(iter->m_kw_l, KWLEN);
  }
  m_hmac.Final(digest);
}


#define SAFE_FWRITE(p, sz, cnt, stream) \
  { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { status = FAILURE; goto end;} \
  }

struct KeyBlockWriter
{
  KeyBlockWriter(FILE *fd, size_t &numWritten)
    : m_fd(fd), m_numWritten(numWritten), m_ok(true)
  {}
  void operator()(const PWSfileV4::KeyBlock &kb)
  {
    write(kb.m_salt, sizeof(kb.m_salt));
    unsigned char Nb[sizeof(kb.m_nHashIters)];
    putInt32(Nb, kb.m_nHashIters);
    write(Nb, sizeof(Nb));
    write(kb.m_kw_k, sizeof(kb.m_kw_k));
    write(kb.m_kw_l, sizeof(kb.m_kw_l));
  }
  size_t &m_numWritten;
  bool m_ok;
private:
  FILE *m_fd;
  void write(const void *p, size_t size)
  {
    if (m_ok) {
      size_t nw = fwrite(p, size, 1, m_fd);
      if (nw == 1)
        m_numWritten += size;
      else
        m_ok = false; // first time this is false, we stop writing!
    }
  }
};

size_t PWSfileV4::WriteKeyBlocks()
{
  size_t numWritten = 0;
  KeyBlockWriter kbw(m_fd, numWritten);
  for_each(m_keyblocks.begin(), m_keyblocks.end(), kbw);

  unsigned char digest[SHA256::HASHLEN];
  ComputeEndKB(digest);
  size_t fret = fwrite(digest, sizeof(digest), 1, m_fd);
  if (fret == 1)
    numWritten += SHA256::HASHLEN;
  return numWritten;
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
  time_t time_now;
  time(&time_now);
  numWritten = WriteCBC(HDR_LASTUPDATETIME,
                        reinterpret_cast<unsigned char *>(&time_now), sizeof(time_t));
  if (numWritten <= 0) { status = FAILURE; goto end; }
  m_hdr.m_whenlastsaved = time_now;

  // Write out who saved it!
  {
    const SysInfo *si = SysInfo::GetInstance();
    stringT user = si->GetRealUser();
    stringT sysname = si->GetRealHost();
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

  if (!m_hdr.m_dbname.empty()) {
    numWritten = WriteCBC(HDR_DBNAME, m_hdr.m_dbname);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }
  if (!m_hdr.m_dbdesc.empty()) {
    numWritten = WriteCBC(HDR_DBDESC, m_hdr.m_dbdesc);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }
  if (!m_MapFilters.empty()) {
    coStringXStream oss;  // XML is always char not wchar_t
    m_MapFilters.WriteFilterXMLFile(oss, m_hdr, _T(""));
    numWritten = WriteCBC(HDR_FILTERS,
                          reinterpret_cast<const unsigned char *>(oss.str().c_str()),
                          oss.str().length());
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  if (!m_hdr.m_RUEList.empty()) {
    coStringXStream oss;
    size_t num = m_hdr.m_RUEList.size();
    if (num > 255)
      num = 255;  // Do not exceed 2 hex character length field
    oss << setw(2) << setfill('0') << hex << num;
    UUIDListIter iter = m_hdr.m_RUEList.begin();
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
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  // Named Policies
  if (!m_MapPSWDPLC.empty()) {
    oStringXStream oss;
    oss.fill(charT('0'));

    size_t num = m_MapPSWDPLC.size();
    if (num > 255)
      num = 255;  // Do not exceed 2 hex character length field

    oss << setw(2) << hex << num;
    PSWDPolicyMapIter iter = m_MapPSWDPLC.begin();
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
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

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

  if (m_hdr.m_yubi_sk != NULL) {
    numWritten = WriteCBC(HDR_YUBI_SK, m_hdr.m_yubi_sk, HeaderRecord::YUBI_SK_LEN);
    if (numWritten <= 0) { status = FAILURE; goto end; }
  }

  // Write zero-length end-of-record type item
  WriteCBC(HDR_END, NULL, 0);
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
  KeyBlock kb;
  size_t nRead;
  nRead = fread(kb.m_salt, sizeof(kb.m_salt), 1, m_fd);
  if (nRead == 0) return END_OF_FILE;
  unsigned char Nb[sizeof(uint32)];

  nRead = fread(Nb, sizeof(Nb), 1, m_fd);
  if (nRead == 0) return END_OF_FILE;
  kb.m_nHashIters = getInt32(Nb);

  nRead = fread(kb.m_kw_k, KWLEN, 1, m_fd);
  if (nRead == 0) return END_OF_FILE;

  nRead = fread(kb.m_kw_l, KWLEN, 1, m_fd);
  if (nRead == 0) return END_OF_FILE;
  
  // m_keyblocks created size(1), push back iff > 1 keyblocks
  if (m_keyblocks.size() == 1)
    m_keyblocks[0] = kb;
  else
    m_keyblocks.push_back(kb);

  return SUCCESS;
}

int PWSfileV4::TryKeyBlock(unsigned index, const StringX &passkey,
                           unsigned char K[KLEN], unsigned char L[KLEN])
{
  ASSERT(index < m_keyblocks.size());
  KeyBlock &kb = m_keyblocks[index];
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
  return SUCCESS;
}

bool PWSfileV4::VerifyKeyBlocks()
{
  unsigned char ReadEndKB[SHA256::HASHLEN];
  unsigned char CalcEndKB[SHA256::HASHLEN];

  int nRead = fread(ReadEndKB, sizeof(ReadEndKB), 1, m_fd);
  if (nRead != 1)
    return false; // will catch EOF later
  ComputeEndKB(CalcEndKB);

  if (memcmp(CalcEndKB, ReadEndKB, SHA256::HASHLEN) == 0)
    return true;
  else {
    fseek(m_fd, -long(sizeof(ReadEndKB)), SEEK_CUR);
    return false;
  }
}

int PWSfileV4::ParseKeyBlocks(const StringX &passkey)
{
  PWS_LOGIT;
  int status;
  do {
    status = ReadKeyBlock();
    // status is either SUCESS or END_OF_FILE
    if (status != SUCCESS) {
      Close();
      return status;
    }

    status = TryKeyBlock(m_keyblocks.size() - 1, passkey, m_key, m_ell);
    // status is either SUCCESS or WRONG_PASSWORD
  } while (status != SUCCESS);

  // here iff we found a good keyblock
  m_current_keyblock = m_keyblocks.size() - 1;

  // Question is, is the the last keyblock?
  while (!VerifyKeyBlocks()) {
    status = ReadKeyBlock();
    // status is either SUCESS or END_OF_FILE
    if (status != SUCCESS) {
      Close();
    }
  }
  return status;
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
  size_t numRead;
  bool utf8status;
  unsigned char *utf8 = NULL;
  size_t utf8Len = 0;
  bool found0302UserHost = false; // to resolve potential conflicts

  do {
    numRead = ReadCBC(fieldType, utf8, utf8Len);

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
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        StringX pref;
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, pref);
        m_hdr.m_prefString = pref;
        if (!utf8status)
          pws_os::Trace0(_T("FromUTF8(m_prefString) failed\n"));
      } else
        m_hdr.m_prefString = _T("");
      break;

    case HDR_DISPSTAT: /* Tree Display Status */
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      for (StringX::iterator iter = text.begin(); iter != text.end(); iter++) {
        const TCHAR v = *iter;
        m_hdr.m_displaystatus.push_back(v == TCHAR('1'));
      }
      if (!utf8status)
        pws_os::Trace0(_T("FromUTF8(m_displaystatus) failed\n"));
      break;

    case HDR_LASTUPDATETIME: /* When last saved */
      if (utf8Len == 8) {
        // Handle pre-3.09 implementations that mistakenly
        // stored this as a hex value
        if (utf8 != NULL) utf8[utf8Len] = '\0';
        utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
        if (!utf8status)
          pws_os::Trace0(_T("FromUTF8(m_whenlastsaved) failed\n"));
        iStringXStream is(text);
        is >> hex >> m_hdr.m_whenlastsaved;
      } else if (utf8Len == 4) {
        // retrieve time_t
        m_hdr.m_whenlastsaved = *reinterpret_cast< time_t*>(utf8);
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
          StringX tlen = text.substr(0, 4);
          iStringXStream is(tlen);
          int ulen = 0;
          is >> hex >> ulen;
          StringX uh = text.substr(4);
          m_hdr.m_lastsavedby = uh.substr(0, ulen);
          m_hdr.m_lastsavedon = uh.substr(ulen);
        } else
          pws_os::Trace0(_T("FromUTF8(m_wholastsaved) failed\n"));
      }
      break;

    case HDR_LASTUPDATEAPPLICATION: /* and by what */
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      m_hdr.m_whatlastsaved = text;
      if (!utf8status)
        pws_os::Trace0(_T("FromUTF8(m_whatlastsaved) failed\n"));
      break;

    case HDR_LASTUPDATEUSER:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      found0302UserHost = true; // so HDR_LASTUPDATEUSERHOST won't override
      m_hdr.m_lastsavedby = text;
      break;

    case HDR_LASTUPDATEHOST:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      found0302UserHost = true; // so HDR_LASTUPDATEUSERHOST won't override
      m_hdr.m_lastsavedon = text;
      break;

    case HDR_DBNAME:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      m_hdr.m_dbname = text;
      break;

    case HDR_DBDESC:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
      utf8status = m_utf8conv.FromUTF8(utf8, utf8Len, text);
      m_hdr.m_dbdesc = text;
      break;

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
      // Don't support importing XML from non-Windows platforms
      // using Microsoft XML libraries
      // Will be treated as an 'unknown header field' by the 'default' clause below
#else
    case HDR_FILTERS:
      if (utf8 != NULL) utf8[utf8Len] = '\0';
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
          if (m_pReporter != NULL)
            (*m_pReporter)(message);

          // Treat it as an Unknown field!
          // Maybe user used a later version of PWS
          // and we don't want to lose anything
          UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);
          m_UHFL.push_back(unkhfe);
          break;
        }
        int rc = m_MapFilters.ImportFilterXMLFile(FPOOL_DATABASE, text.c_str(), _T(""),
                                                  XSDFilename.c_str(),
                                                  strErrors, m_pAsker);
        if (rc != PWScore::SUCCESS) {
          // Can't parse it - treat as an unknown field,
          // Notify user that filter won't be available
          stringT message;
          LoadAString(message, IDSC_CANTPROCESSDBFILTERS);
          if (m_pReporter != NULL)
            (*m_pReporter)(message);

          UnknownFieldEntry unkhfe(fieldType, utf8Len, utf8);
          m_UHFL.push_back(unkhfe);
        }
      }
      break;
#endif

    case HDR_RUE:
      {
        if (utf8 != NULL) utf8[utf8Len] = '\0';
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
      if (utf8Len != HeaderRecord::YUBI_SK_LEN) {
        delete[] utf8;
        Close();
        return FAILURE;
      }
      m_hdr.m_yubi_sk = new unsigned char[HeaderRecord::YUBI_SK_LEN];
      memcpy(m_hdr.m_yubi_sk, utf8, HeaderRecord::YUBI_SK_LEN);
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
      if (utf8Len != HeaderRecord::YUBI_SK_LEN ||
          (utf8Len >= 4 &&
           isxdigit(utf8[0]) && isxdigit(utf8[1]) &&
           isxdigit(utf8[1]) && isxdigit(utf8[2]))) {
        if (utf8 != NULL) utf8[utf8Len] = '\0';
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

            int namelength, symbollength;

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
            if (pr.second == false) break; // Error
          }
        }
      } else { // Looks like YUBI_OLD_SK: field length is exactly YUBI_SK_LEN
        //        and at least one non-hex character in first 4 of field.
        m_hdr.m_yubi_sk = new unsigned char[HeaderRecord::YUBI_SK_LEN];
        memcpy(m_hdr.m_yubi_sk, utf8, HeaderRecord::YUBI_SK_LEN);
      }
      break;

    case HDR_EMPTYGROUP:
      {
        if (utf8 != NULL) utf8[utf8Len] = '\0';
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
    delete[] utf8; utf8 = NULL; utf8Len = 0;
  } while (fieldType != HDR_END);

  return SUCCESS;
}

bool PWSfileV4::IsV4x(const StringX &filename , VERSION &v)
{
  // XXX Certainly has to change, if can be at all supported!
  v = V40;
  ASSERT(pws_os::FileExists(filename.c_str()));
  FILE *fd = pws_os::FOpen(filename.c_str(), _T("rb"));

  ASSERT(fd != NULL);
  return true;
}
