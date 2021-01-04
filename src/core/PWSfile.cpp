/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "PWSfile.h"
#include "PWSfileV1V2.h"
#include "PWSfileV3.h"
#include "PWSfileV4.h"
#include "SysInfo.h"
#include "core.h"
#include "os/file.h"

#include "crypto/sha1.h" // for simple encrypt/decrypt
#include "PWSrand.h"

#include <cerrno>

PWSfile *PWSfile::MakePWSfile(const StringX &a_filename, const StringX &passkey,
                              VERSION &version, RWmode mode, int &status,
                              Asker *pAsker, Reporter *pReporter)
{
  PWSfile *retval = nullptr;

  if (mode == Read && !pws_os::FileExists(a_filename.c_str())) {
    status = CANT_OPEN_FILE;
    return nullptr;
  }

  switch (version) {
    case V17:
    case V20:
      status = SUCCESS;
      retval = new PWSfileV1V2(a_filename, mode, version);
      break;
    case V30:
      status = SUCCESS;
      retval = new PWSfileV3(a_filename, mode, version);
      break;
    case V40:
      status = SUCCESS;
      retval = new PWSfileV4(a_filename, mode, version);
      break;
    case UNKNOWN_VERSION:
      ASSERT(mode == Read);
      version = PWSfile::ReadVersion(a_filename, passkey);
      switch (version) {
      case V40:
        status = SUCCESS;
        retval = new PWSfileV4(a_filename, mode, version);
        break;
      case V30:
        status = SUCCESS;
        retval = new PWSfileV3(a_filename, mode, version);
        break;
      case V17:  // never actually returned
      case V20:  // may be inaccurate (V17)
        status = SUCCESS;
        retval = new PWSfileV1V2(a_filename, mode, version);
        break;
      case NEWFILE:
        ASSERT(0);
        //[[fallthrough]];
      case UNKNOWN_VERSION:
      default:
        status = FAILURE;
      } // inner switch
      break;
  case NEWFILE: // should never happen
  default:
    status = FAILURE;
    ASSERT(0);
  }
  if (retval != nullptr) {
    retval->m_pAsker = pAsker;
    retval->m_pReporter = pReporter;
  }
  return retval;
}

PWSfile::VERSION PWSfile::ReadVersion(const StringX &filename, const StringX &passkey)
{
  if (pws_os::FileExists(filename.c_str())) {
    VERSION v;
    if (PWSfileV3::IsV3x(filename, v))
      return v;
    else if (PWSfileV4::IsV4x(filename, passkey, v))
      return v;
    else if (PWSfileV1V2::CheckPasskey(filename, passkey) == SUCCESS)
      return V20;
    else
      return UNKNOWN_VERSION;
  } else
    return UNKNOWN_VERSION;
}

PWSfile::PWSfile(const StringX &filename, RWmode mode, VERSION v)
  : m_filename(filename), m_passkey(_T("")), m_fd(nullptr),
  m_curversion(v), m_rw(mode), m_defusername(_T("")),
  m_fish(nullptr), m_terminal(nullptr), m_status(SUCCESS),
  m_nRecordsWithUnknownFields(0)
{
}

PWSfile::~PWSfile()
{
  Close(); // idempotent
}

void PWSfile::HashRandom256(unsigned char *p256)
{
  /**
   * This is for random data that will be written to the file directly.
   * The idea is to avoid directly exposing our generated randomness
   * to the attacker, since this might expose state of the RNG.
   * Therefore, we'll hash the randomness.
   *
   * As the names imply, this works on 256 bit (32 byte) arrays.
   */
  PWSrand::GetInstance()->GetRandomData(p256, 32);
  SHA256 salter;
  salter.Update(p256, 32);
  salter.Final(p256);
}

void PWSfile::FOpen()
{
  ASSERT(!m_filename.empty());
  const TCHAR* m = (m_rw == Read) ? _T("rb") : _T("wb");
  if (m_fd != nullptr) {
    fclose(m_fd);
    m_fd = nullptr;
  }
  m_fd = pws_os::FOpen(m_filename.c_str(), m);
  m_fileLength = pws_os::fileLength(m_fd);
}

int PWSfile::Close()
{
  delete m_fish;
  m_fish = nullptr;
  int rc(SUCCESS);

  if (m_fd != nullptr) {
    rc = pws_os::FClose(m_fd, m_rw == Write);
    m_fd = nullptr;
  }

  return rc;
}

size_t PWSfile::WriteCBC(unsigned char type, const unsigned char *data,
                         size_t length)
{
  ASSERT(m_fish != nullptr && m_IV != nullptr);
  return _writecbc(m_fd, data, length, type, m_fish, m_IV);
}

size_t PWSfile::ReadCBC(unsigned char &type, unsigned char* &data,
                        size_t &length)
{
  unsigned char *buffer = nullptr;
  size_t buffer_len = 0;
  size_t retval;

  ASSERT(m_fish != nullptr && m_IV != nullptr);
  retval = _readcbc(m_fd, buffer, buffer_len, type,
    m_fish, m_IV, m_terminal, m_fileLength);

  if (buffer_len > 0) {
    if (buffer_len < length || data == nullptr)
      length = buffer_len; // set to length read
    // if buffer_len > length, data is truncated to length
    // probably an error.
    if (data != nullptr) {
      memcpy(data, buffer, length);
      trashMemory(buffer, buffer_len);
      delete[] buffer;
    } else { // nullptr data means pass buffer directly to caller
      data = buffer; // caller must trash & delete[]!
    }
  } else {
    // no need to delete[] buffer, since _readcbc will not allocate if
    // buffer_len is zero
  }
  return retval;
}

int PWSfile::CheckPasskey(const StringX &filename, const StringX &passkey, VERSION &version)
{
  /**
   * We start with V3 because it's the quickest to rule out
   * due to header/footer.
   * V4 can take a looong time if the iter value's too big.
   * XXX Need to address this later with a popup prompting the user.
   */
  if (passkey.empty())
    return WRONG_PASSWORD;

  int status;
  version = UNKNOWN_VERSION;
  status = PWSfileV3::CheckPasskey(filename, passkey);
  if (status == SUCCESS) {
    version = V30;
  } else {
    status = PWSfileV4::CheckPasskey(filename, passkey);
    if (status == SUCCESS)
      version = V40;
    else {
      status = PWSfileV1V2::CheckPasskey(filename, passkey);
      if (status == SUCCESS)
        version = V20; // or V17?
    }
  }
  return status;
}

void PWSfile::GetUnknownHeaderFields(UnknownFieldList &UHFL)
{
  if (!m_UHFL.empty())
    UHFL = m_UHFL;
  else
    UHFL.clear();
}

void PWSfile::SetUnknownHeaderFields(UnknownFieldList &UHFL)
{
  if (!UHFL.empty())
    m_UHFL = UHFL;
  else
    m_UHFL.clear();
}

long PWSfile::GetOffset() const
{
  long retval = ftell(m_fd);
  ASSERT(ulong64(retval) <= pws_os::fileLength(m_fd));
  return retval;
}

// Following for 'legacy' use of pwsafe as file encryptor/decryptor
// this is for the undocumented 'command line file encryption'
static const stringT CIPHERTEXT_SUFFIX(_S(".PSF"));

size_t PWSfile::fileThresholdSize = std::numeric_limits<uint32>::max(); // files this size and above encrypted differently - configurable for testing


static stringT ErrorMessages()
{
  stringT cs_text;

  switch (errno) {
  case EACCES:
    LoadAString(cs_text, IDSC_FILEREADONLY);
    break;
  case EEXIST:
    LoadAString(cs_text, IDSC_FILEEXISTS);
    break;
  case EINVAL:
    LoadAString(cs_text, IDSC_INVALIDFLAG);
    break;
  case EMFILE:
    LoadAString(cs_text, IDSC_NOMOREHANDLES);
    break;
  case ENOENT:
    LoadAString(cs_text, IDSC_FILEPATHNOTFOUND);
    break;
  case EIO: // synthesized upon fwrite failure
    LoadAString(cs_text, IDSC_FILEWRITEERROR);
    break;
  case EFBIG:
    LoadAString(cs_text, IDSC_FILE_TOO_BIG);
    break;
  default:
    break;
  }
  return cs_text;
}

// Following specific for PWSfile::Encrypt
#define SAFE_FWRITE(p, sz, cnt, stream) \
  { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { status = false; goto exit;} \
  }

// std::numeric_limits<>::max() && m'soft's silly macros don't work together
#ifdef max
#undef max
#endif

#ifndef BUFSIZ
#define BUFSIZ 2048
#endif

bool PWSfile::Encrypt(const stringT &fn, const StringX &passwd, stringT &errmess)
{
  FILE* out = nullptr;
  Fish *fish = nullptr;
  bool status = true;
  const stringT out_fn = fn + CIPHERTEXT_SUFFIX;
  unsigned char *pass = nullptr;
  unsigned char* ivthing = nullptr;
  size_t passlen = 0;
  size_t file_len = 0, orig_filelen = 0;
  size_t nread = 0;
  unsigned char thesalt[SaltLength];
  unsigned int BS = 0;
  const unsigned char* bufp = nullptr;
  bool isBigFile = false;

  
  FILE *in = pws_os::FOpen(fn, _T("rb"));
  if (in == nullptr) {
    status = false; goto exit;
  }

  file_len = pws_os::fileLength(in);

  isBigFile = (file_len > fileThresholdSize);

  out = pws_os::FOpen(out_fn, _T("wb"));
  if (out == nullptr) {
    status = false; goto exit;
  }

  ConvertPasskey(passwd, pass, passlen);
  PWSrand::GetInstance()->GetRandomData(thesalt, SaltLength);

  if (isBigFile)
    fish = makeFish<TwoFish, SHA256>(pass, static_cast<unsigned int>(passlen), thesalt, SaltLength);
  else
    fish = makeFish<BlowFish, SHA1>(pass, static_cast<unsigned int>(passlen), thesalt, SaltLength);

  trashMemory(pass, passlen);
  delete[] pass; // gross - ConvertPasskey allocates.
  BS = fish->GetBlockSize();

  unsigned char randstuff[StuffSize];
  unsigned char randhash[SHA1::HASHLEN];   // HashSize
  PWSrand::GetInstance()->GetRandomData(randstuff, StuffSize);

  if (!isBigFile) {
    // miserable bug - have to fix this way to avoid breaking existing files
    randstuff[StuffSize - 2] = randstuff[StuffSize - 1] = TCHAR('\0');
  }
  
  GenRandhash(passwd, randstuff, randhash);
  if (isBigFile)
    SAFE_FWRITE(randstuff, 1, StuffSize, out)
  else
    SAFE_FWRITE(randstuff, 1, StuffSize - 2, out)

  SAFE_FWRITE(randhash, 1, sizeof(randhash), out)
  SAFE_FWRITE(thesalt, 1, SaltLength, out)

  ivthing = new unsigned char[BS];
  PWSrand::GetInstance()->GetRandomData(ivthing, BS);
  SAFE_FWRITE(ivthing, 1, BS, out)

  unsigned char buf[BUFSIZ];
  bufp = buf;
  nread = fread(buf, 1, BUFSIZ, in);
  orig_filelen = file_len;

  try {
    //write first block: length + dummy type +  bytes of data
    size_t nwritten = _writecbc1st(out, &bufp, &file_len, 0, fish, ivthing, isBigFile);
    if (nwritten != BS) {
      status = false;
      goto exit;
    }
    // write rest of first buffer
    nread -= orig_filelen - file_len;
    nwritten = _writecbcRest(out, bufp, nread, fish, ivthing);
    if (nwritten < nread) {
      status = false;
      goto exit;
    }


    do { // main read/encrypt/write loop
      nread = fread(buf, 1, BUFSIZ, in);
      if (ferror(in)) { // this is how to detect fread errors
       status = false;
        goto exit;
      }

      if (nread == 0) // save writing a block or two.
        break;

      _writecbcRest(out, buf, nread, fish, ivthing);
    
    } while (!feof(in));

  } catch (...) { // _writecbc* throws an exception if it fails to write
    errno = EIO;
    status = false;
    goto exit;
  } // catch

  status = (pws_os::FClose(out, true) == 0); out = nullptr;

 exit:
  if (!status)
    errmess = ErrorMessages();
  delete fish;
  trashMemory(buf, BUFSIZ);
  delete[] ivthing;
  pws_os::FClose(in, false);
  pws_os::FClose(out, true);
  return status;
}

bool PWSfile::Decrypt(const stringT &fn, const StringX &passwd, stringT &errmess)
{
  ulong64 file_len;
  bool status = true;
  unsigned char salt[SaltLength];
  unsigned char *ivthing = nullptr;
  unsigned char randstuff[StuffSize];
  unsigned char randhash[SHA1::HASHLEN];
  unsigned char temphash[SHA1::HASHLEN];
  FILE* out = nullptr;
  bool isBigFile(false);
  size_t stuffLen(0);

  FILE *in = pws_os::FOpen(fn, _T("rb"));
  if (in == nullptr) {
    status = false;
    goto exit;
  }

  file_len = pws_os::fileLength(in);

  if (file_len < (8 + sizeof(randhash) + 8 + SaltLength)) {
    pws_os::FClose(in, false);
    LoadAString(errmess, IDSC_FILE_TOO_SHORT);
    return false;
  }

  isBigFile = (file_len > fileThresholdSize);

  stuffLen = isBigFile ? StuffSize : (StuffSize - 2);

  if (fread(randstuff, 1, stuffLen, in) != stuffLen) {
    status = false;
    goto exit;
  }
  if (!isBigFile) {
    randstuff[StuffSize - 2] = randstuff[StuffSize - 1] = TCHAR('\0'); // ugly bug workaround
  }
  if (fread(randhash, 1, sizeof(randhash), in) != sizeof(randhash)) {
    status = false;
    goto exit;
  }

  GenRandhash(passwd, randstuff, temphash);
  if (memcmp(reinterpret_cast<char *>(randhash), reinterpret_cast<char *>(temphash), SHA1::HASHLEN) != 0) {
    pws_os::FClose(in, false);
    LoadAString(errmess, IDSC_BADPASSWORD);
    return false;
  }

  { // decryption in a block, since we use goto


    unsigned char *pwd = nullptr;
    size_t passlen = 0;
    Fish* fish;

    ConvertPasskey(passwd, pwd, passlen);
    if (fread(salt, 1, SaltLength, in) != SaltLength) {
      status = false;
      goto exit;
    }
    if (isBigFile)
      fish = makeFish<TwoFish, SHA256>(pwd, static_cast<unsigned int>(passlen), salt, SaltLength);
    else
      fish = makeFish<BlowFish, SHA1>(pwd, static_cast<unsigned int>(passlen), salt, SaltLength);

    trashMemory(pwd, passlen);
    delete[] pwd; // gross - ConvertPasskey allocates.
    const unsigned int BS = fish->GetBlockSize();

    ivthing = new unsigned char[BS];

    if (fread(ivthing, 1, BS, in) != BS) {
      status = false;
      goto exit;
    }

    // read first block, containing plaintext length
    size_t plaintext_length;
    if (readcbc1st(in, plaintext_length, fish, ivthing, isBigFile) != BS) {
      delete fish;
      status = false;
      goto exit;
    }

    // Open output file
    size_t suffix_len = CIPHERTEXT_SUFFIX.length();
    size_t filepath_len = fn.length();

    stringT out_fn = fn;
    out_fn = out_fn.substr(0, filepath_len - suffix_len);

    out = pws_os::FOpen(out_fn, _T("wb"));
    if (out == nullptr) {
      delete fish;
      status = false;
      goto exit;
    }

    // now iterate over rest of file
    unsigned char buf[BUFSIZ];
    size_t nleft = plaintext_length;

    do {
      size_t nread = _readcbc(in, buf, BUFSIZ, fish, ivthing);
      if (ferror(in)) {
        delete fish;
        status = false;
        goto exit;
      }
      if (nread == 0) { // no plaintext or we hit the exact end. In any case, break loop peacefully
        break;
      }
      // write plaintext
      auto nwrite = nleft > BUFSIZ ? BUFSIZ : nleft;
      if (fwrite(buf,1, nwrite, out) != nwrite) {
        delete fish;
        status = false;
        goto exit;
      }
      nleft -= nwrite;
    } while (!feof(in));

    if (nleft != 0) {
      // truncated ciphertext?
      status = false;
    }
    delete fish;
  } // write decrypted
 exit:
  delete[] ivthing;
  if (!status)
    errmess = ErrorMessages();
  pws_os::FClose(in, false);
  pws_os::FClose(out, true);

  return status;
}

//-----------------------------------------------------------------
// A quick way to determine if two files are equal, or if a given
// file has been modified.

// For large files (>2K), we only hash the head & tail of the file.
// This is due to a performance trade-off.

// For unencrypted file, only checking the head & tail of large files may
// cause changes made to the middle of the file to be missed.
// However, since we write encrypted in CBC mode, this means that if the file
// was modified at offset X, then everything from X to the end of the file will
// be modified and the digests would be different.

PWSFileSig::PWSFileSig(const stringT &fname)
{
  const long THRESHOLD = 2048; // if file's longer than this, hash only head & tail

  m_length = 0;
  m_iErrorCode = PWSfile::SUCCESS;
  memset(m_digest, 0, sizeof(m_digest));
  FILE *fp = pws_os::FOpen(fname, _T("rb"));
  if (fp != nullptr) {
    SHA256 hash;
    m_length = pws_os::fileLength(fp);
    // Not the right place to be worried about min size, as this is format
    // version specific (and we're in PWSFile).
    // An empty file, though, should be failed.
    if (m_length > 0) {
      unsigned char buf[THRESHOLD];
      if (m_length <= THRESHOLD) {
        if (fread(buf, size_t(m_length), 1, fp) == 1) {
          hash.Update(buf, size_t(m_length));
          hash.Final(m_digest);
        }
      } else { // m_length > THRESHOLD
        if (fread(buf, THRESHOLD / 2, 1, fp) == 1 &&
            fseek(fp, -THRESHOLD / 2, SEEK_END) == 0 &&
            fread(buf + THRESHOLD / 2, THRESHOLD / 2, 1, fp) == 1) {
          hash.Update(buf, THRESHOLD);
          hash.Final(m_digest);
        }
      }
    } else { // Empty file
      m_iErrorCode = PWSfile::TRUNCATED_FILE;
    }

    fclose(fp);
  } else {
    m_iErrorCode = PWSfile::CANT_OPEN_FILE;
  }
}

PWSFileSig::PWSFileSig(const PWSFileSig &pfs)
{
  m_length = pfs.m_length;
  m_iErrorCode = pfs.m_iErrorCode;
  memcpy(m_digest, pfs.m_digest, sizeof(m_digest));
}

PWSFileSig &PWSFileSig::operator=(const PWSFileSig &that)
{
  if (this != &that) {
    m_length = that.m_length;
    m_iErrorCode = that.m_iErrorCode;
    memcpy(m_digest, that.m_digest, sizeof(m_digest));
  }
  return *this;
}

bool PWSFileSig::operator==(const PWSFileSig &that)
{
  // Check this first as digest may otherwise be invalid
  if (m_iErrorCode != 0 || that.m_iErrorCode != 0)
    return false;

  return (m_length == that.m_length &&
          memcmp(m_digest, that.m_digest, sizeof(m_digest)) == 0);
}
