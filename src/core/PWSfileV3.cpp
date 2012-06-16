/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

using namespace std;
using pws_os::CUUID;

static unsigned char TERMINAL_BLOCK[TwoFish::BLOCKSIZE] = {
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F',
  'P', 'W', 'S', '3', '-', 'E', 'O', 'F'};

PWSfileV3::PWSfileV3(const StringX &filename, RWmode mode, VERSION version)
  : PWSfile(filename, mode)
{
  m_curversion = version;
  m_IV = m_ipthing;
  m_terminal = TERMINAL_BLOCK;
}

PWSfileV3::~PWSfileV3()
{
}

int PWSfileV3::Open(const StringX &passkey)
{
  PWS_LOGIT;

  int status = SUCCESS;

  ASSERT(m_curversion == V30);
  if (passkey.empty()) { // Can happen if db 'locked'
    pws_os::Trace(_T("PWSfileV3::Open(empty_passkey)\n"));
    return WRONG_PASSWORD;
  }
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
  PWS_LOGIT;

  if (m_fd == NULL)
    return SUCCESS; // idempotent

  unsigned char digest[HMAC_SHA256::HASHLEN];
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

int PWSfileV3::SanityCheck(FILE *stream)
{
  int retval = SUCCESS;
  ASSERT(stream != NULL);

  // Is file too small?
  const long min_file_length = 232; // pre + post, no hdr or records
  if (pws_os::fileLength(stream) < min_file_length)
    return TRUNCATED_FILE;

  long pos = ftell(stream); // restore when we're done
  // Does file have a valid header?
  char tag[sizeof(V3TAG)];
  size_t nread = fread(tag, sizeof(tag), 1, stream);
  if (nread != 1) {
    retval = READ_FAIL;
    goto err;
  }
  if (memcmp(tag, V3TAG, sizeof(tag)) != 0) {
    retval = NOT_PWS3_FILE;
    goto err;
  }

  // Does file have a valid EOF block?
  unsigned char eof_block[sizeof(TERMINAL_BLOCK)];
  if (fseek(stream, -int(sizeof(TERMINAL_BLOCK) + HMAC_SHA256::HASHLEN), SEEK_END) != 0) {
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
                            unsigned char *aPtag, int *nITER)
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
  if (retval != SUCCESS)
    goto err;

  fseek(fd, sizeof(V3TAG), SEEK_SET); // skip over tag
  unsigned char salt[SaltLengthV3];
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
    retval = WRONG_PASSWORD;
    goto err;
  }
err:
  if (a_fd == NULL) // if we opened the file, we close it...
    fclose(fd);
  return retval;
}

size_t PWSfileV3::WriteCBC(unsigned char type, const StringX &data)
{
  bool status;
  const unsigned char *utf8;
  size_t utf8Len;
  status = m_utf8conv.ToUTF8(data, utf8, utf8Len);
  if (!status)
    pws_os::Trace(_T("ToUTF8(%s) failed\n"), data.c_str());
  return WriteCBC(type, utf8, utf8Len);
}

size_t PWSfileV3::WriteCBC(unsigned char type, const unsigned char *data,
                           size_t length)
{
  m_hmac.Update(data, reinterpret_cast<int &>(length));
  return PWSfile::WriteCBC(type, data, length);
}

int PWSfileV3::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);
  int status = SUCCESS;
  StringX tmp;
  uuid_array_t item_uuid;
  time_t t = 0;
  int32 i32;
  short i16;

  item.GetUUID(item_uuid);
  WriteCBC(CItemData::UUID, item_uuid, sizeof(uuid_array_t));
  tmp = item.GetGroup();
  if (!tmp.empty())
    WriteCBC(CItemData::GROUP, tmp);
  WriteCBC(CItemData::TITLE, item.GetTitle());
  WriteCBC(CItemData::USER, item.GetUser());
  WriteCBC(CItemData::PASSWORD, item.GetPassword());

  tmp = item.GetNotes();
  if (!tmp.empty())
    WriteCBC(CItemData::NOTES, tmp);
  tmp = item.GetURL();
  if (!tmp.empty())
    WriteCBC(CItemData::URL, tmp);
  tmp = item.GetAutoType();
  if (!tmp.empty())
    WriteCBC(CItemData::AUTOTYPE, tmp);
  item.GetCTime(t);
  if (t != 0) {
    i32 = static_cast<int>(t);
    WriteCBC(CItemData::CTIME, reinterpret_cast<unsigned char *>(&i32), sizeof(int32));
  }
  item.GetPMTime(t);
  if (t != 0) {
    i32 = static_cast<int>(t);
    WriteCBC(CItemData::PMTIME, reinterpret_cast<unsigned char *>(&i32), sizeof(int32));
  }
  item.GetATime(t);
  if (t != 0) {
    i32 = static_cast<int>(t);
    WriteCBC(CItemData::ATIME, reinterpret_cast<unsigned char *>(&i32), sizeof(int32));
  }
  item.GetXTime(t);
  if (t != 0) {
    i32 = static_cast<int>(t);
    WriteCBC(CItemData::XTIME, reinterpret_cast<unsigned char *>(&i32), sizeof(int32));
  }
  item.GetXTimeInt(i32);
  if (i32 > 0 && i32 <= 3650) {
    WriteCBC(CItemData::XTIME_INT, reinterpret_cast<unsigned char *>(&i32), sizeof(int32));
  }
  item.GetRMTime(t);
  if (t != 0) {
    i32 = static_cast<int>(t);
    WriteCBC(CItemData::RMTIME, reinterpret_cast<unsigned char *>(&i32), sizeof(int32));
  }
  tmp = item.GetPWPolicy();
  if (!tmp.empty())
    WriteCBC(CItemData::POLICY, tmp);
  tmp = item.GetPWHistory();
  if (!tmp.empty())
    WriteCBC(CItemData::PWHIST, tmp);
  tmp = item.GetRunCommand();
  if (!tmp.empty())
    WriteCBC(CItemData::RUNCMD, tmp);
  item.GetDCA(i16);
  if (i16 >= PWSprefs::minDCA && i16 <= PWSprefs::maxDCA)
    WriteCBC(CItemData::DCA, reinterpret_cast<unsigned char *>(&i16), sizeof(short));
  item.GetShiftDCA(i16);
  if (i16 >= PWSprefs::minDCA && i16 <= PWSprefs::maxDCA)
    WriteCBC(CItemData::SHIFTDCA, reinterpret_cast<unsigned char *>(&i16), sizeof(short));
  tmp = item.GetEmail();
  if (!tmp.empty())
    WriteCBC(CItemData::EMAIL, tmp);
  tmp = item.GetProtected();
  if (!tmp.empty())
    WriteCBC(CItemData::PROTECTED, tmp);
  tmp = item.GetSymbols();
  if (!tmp.empty())
    WriteCBC(CItemData::SYMBOLS, tmp);
  tmp = item.GetPolicyName();
  if (!tmp.empty())
    WriteCBC(CItemData::POLICYNAME, tmp);

  UnknownFieldsConstIter vi_IterURFE;
  for (vi_IterURFE = item.GetURFIterBegin();
       vi_IterURFE != item.GetURFIterEnd();
       vi_IterURFE++) {
    unsigned char type;
    size_t length = 0;
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
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion == V30);

  int status = SUCCESS;

  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen; // <= 0 means end of file reached

  do {
    unsigned char *utf8 = NULL;
    size_t utf8Len = 0;
    fieldLen = static_cast<signed long>(ReadCBC(type, utf8,
                                                utf8Len));

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
  
  if (item.IsPasswordPolicySet() && item.IsPolicyNameSet()) {
    // Error can't have both - clear Password Policy Name
    item.SetPolicyName(StringX(_T("")));
  }
  
  if (item.IsPolicyNameSet()) {
    StringX sxPolicyName = item.GetPolicyName();
    PSWDPolicyMapIter iter = m_MapPSWDPLC.find(sxPolicyName);
    if (iter == m_MapPSWDPLC.end()) {
      // Map name not present in database - clear it!
      item.SetPolicyName(StringX(_T("")));
    } else {
      // Increase use count
      iter->second.usecount++;
    }
  }
    
  if (numread > 0)
    return status;
  else
    return END_OF_FILE;
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

const short VersionNum = 0x030C;

// Following specific for PWSfileV3::WriteHeader
#define SAFE_FWRITE(p, sz, cnt, stream) \
  { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { status = FAILURE; goto end;} \
  }

int PWSfileV3::WriteHeader()
{
  PWS_LOGIT;

  // Following code is divided into {} blocks to
  // prevent "uninitialized" compile errors, as we use
  // goto for error handling
  int status = SUCCESS;
  size_t numWritten;
  unsigned char salt[SaltLengthV3];

  // See formatV3.txt for explanation of what's written here and why
  unsigned int NumHashIters;
  if (m_hdr.m_nITER < MIN_HASH_ITERATIONS)
    NumHashIters = MIN_HASH_ITERATIONS;
  else
    NumHashIters = m_hdr.m_nITER;

  SAFE_FWRITE(V3TAG, 1, sizeof(V3TAG), m_fd);

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
  if (!m_vEmptyGroups.empty()) {
    for (size_t n = 0; n < m_vEmptyGroups.size(); n++) {
      numWritten = WriteCBC(HDR_EMPTYGROUP, m_vEmptyGroups[n]);
      if (numWritten <= 0) { status = FAILURE; goto end; }
    }
  }

  if (!m_UHFL.empty()) {
    UnknownFieldList::iterator vi_IterUHFE;
    for (vi_IterUHFE = m_UHFL.begin();
         vi_IterUHFE != m_UHFL.end();
         vi_IterUHFE++) {
      UnknownFieldEntry &unkhfe = *vi_IterUHFE;
      numWritten = WriteCBC(unkhfe.uc_Type,
                            unkhfe.uc_pUField, static_cast<unsigned int>(unkhfe.st_length));
      if (numWritten <= 0) { status = FAILURE; goto end; }
    }
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

int PWSfileV3::ReadHeader()
{
  PWS_LOGIT;

  unsigned char Ptag[SHA256::HASHLEN];
  int status = CheckPasskey(m_filename, m_passkey, m_fd,
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
      // This hack keeps bwd compatability.
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
        std::string temp = (char *)utf8;

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
          uuid_array_t ua;
          for (size_t i = 0; i < sizeof(uuid_array_t); i++, j += 2) {
            stringstream ss;
            ss.str(temp.substr(j, 2));
            ss >> hex >> x;
            ua[i] = static_cast<unsigned char>(x);
          }
          const CUUID uuid(ua);
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
      {
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
            iStringXStream is(sxTemp);
            j += 2;  // Skip over name length

            is >> hex >> namelength;
            if (j + namelength > recordlength) break;  // Error
            
            StringX sxPolicyName = text.substr(j, namelength);
            j += namelength;  // Skip over name
            if (j + 19 > recordlength) break;  // Error

            StringX cs_pwp(text.substr(j, 19));
            PWPolicy pwp(cs_pwp);
            j += 19;  // Skip over pwp

            if (j + 2 > recordlength) break;  // Error
            sxTemp = text.substr(j, 2) + sxBlank;
            is.str(sxTemp);
            j += 2;  // Skip over symbols length
            is >> hex >> symbollength;
            
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
        break;
      }

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

bool PWSfileV3::IsV3x(const StringX &filename, VERSION &v)
{
  // This is written so as to support V30, V31, V3x...

  ASSERT(pws_os::FileExists(filename.c_str()));
  FILE *fd = pws_os::FOpen(filename.c_str(), _T("rb"));

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
