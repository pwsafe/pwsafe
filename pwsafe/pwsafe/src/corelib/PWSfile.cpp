/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "PWSfile.h"
#include "PWSfileV1V2.h"
#include "PWSfileV3.h"
#include "SysInfo.h"
#include "corelib.h"
#include "os/file.h"

#include "sha1.h" // for simple encrypt/decrypt
#include "PWSrand.h" // ditto

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

PWSfile *PWSfile::MakePWSfile(const StringX &a_filename, VERSION &version,
                              RWmode mode, int &status,
                              Asker *pAsker, Reporter *pReporter)
{
  if (mode == Read && !pws_os::FileExists(a_filename.c_str())) {
    status = CANT_OPEN_FILE;
    return NULL;
  }

  PWSfile *retval;
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
    case UNKNOWN_VERSION:
      ASSERT(mode == Read);
      if (PWSfile::ReadVersion(a_filename) == V30) {
        version = V30;
        status = SUCCESS;
        retval = new PWSfileV3(a_filename, mode, version);
      } else {
        version = V20; // may be inaccurate (V17)
        status = SUCCESS;
        retval = new PWSfileV1V2(a_filename, mode, version);
      }
      break;
    default:
      ASSERT(0);
      status = FAILURE; return NULL;
  }
  retval->m_pAsker = pAsker;
  retval->m_pReporter = pReporter;
  return retval;
}


PWSfile::VERSION PWSfile::ReadVersion(const StringX &filename)
{
  if (pws_os::FileExists(filename.c_str())) {
    VERSION v;
    if (PWSfileV3::IsV3x(filename, v))
      return v;
    else
      return V20;
  } else
    return UNKNOWN_VERSION;
}

PWSfile::PWSfile(const StringX &filename, RWmode mode)
  : m_filename(filename), m_passkey(_T("")), m_fd(NULL),
  m_curversion(UNKNOWN_VERSION), m_rw(mode), m_defusername(_T("")),
  m_fish(NULL), m_terminal(NULL),
  m_nRecordsWithUnknownFields(0)
{
}

PWSfile::~PWSfile()
{
  Close(); // idempotent
}

PWSfile::HeaderRecord::HeaderRecord()
  : m_nCurrentMajorVersion(0), m_nCurrentMinorVersion(0),
  m_nITER(0), m_prefString(_T("")), m_whenlastsaved(0),
  m_lastsavedby(_T("")), m_lastsavedon(_T("")),
  m_whatlastsaved(_T("")),
  m_dbname(_T("")), m_dbdesc(_T(""))
{
  memset(m_file_uuid_array, 0x00, sizeof(m_file_uuid_array));
}

PWSfile::HeaderRecord::HeaderRecord(const PWSfile::HeaderRecord &h) 
  : m_nCurrentMajorVersion(h.m_nCurrentMajorVersion),
  m_nCurrentMinorVersion(h.m_nCurrentMinorVersion),
  m_nITER(h.m_nITER), m_displaystatus(h.m_displaystatus),
  m_prefString(h.m_prefString), m_whenlastsaved(h.m_whenlastsaved),
  m_lastsavedby(h.m_lastsavedby), m_lastsavedon(h.m_lastsavedon),
  m_whatlastsaved(h.m_whatlastsaved),
  m_dbname(h.m_dbname), m_dbdesc(h.m_dbdesc)
{
  memcpy(m_file_uuid_array, h.m_file_uuid_array,
         sizeof(m_file_uuid_array));
}

PWSfile::HeaderRecord &PWSfile::HeaderRecord::operator=(const PWSfile::HeaderRecord &h)
{
  if (this != &h) {
    m_nCurrentMajorVersion = h.m_nCurrentMajorVersion;
    m_nCurrentMinorVersion = h.m_nCurrentMinorVersion;
    m_nITER = h.m_nITER;
    m_displaystatus = h.m_displaystatus;
    m_prefString = h.m_prefString;
    m_whenlastsaved = h.m_whenlastsaved;
    m_lastsavedby = h.m_lastsavedby;
    m_lastsavedon = h.m_lastsavedon;
    m_whatlastsaved = h.m_whatlastsaved;
    m_dbname = h.m_dbname;
    m_dbdesc = h.m_dbdesc;
    memcpy(m_file_uuid_array, h.m_file_uuid_array,
           sizeof(m_file_uuid_array));
  }
  return *this;
}

void PWSfile::FOpen()
{
  const TCHAR* m = (m_rw == Read) ? _T("rb") : _T("wb");
  if (m_fd != NULL) {
    fclose(m_fd);
    m_fd = NULL;
  }
  m_fd = pws_os::FOpen(m_filename.c_str(), m);
  m_fileLength = pws_os::fileLength(m_fd);
}

int PWSfile::Close()
{
  delete m_fish;
  m_fish = NULL;
  if (m_fd != NULL) {
    fflush(m_fd);
    fclose(m_fd);
    m_fd = NULL;
  }
  return SUCCESS;
}

size_t PWSfile::WriteCBC(unsigned char type, const unsigned char *data,
                         unsigned int length)
{
  ASSERT(m_fish != NULL && m_IV != NULL);
  return _writecbc(m_fd, data, length, type, m_fish, m_IV);
}

size_t PWSfile::ReadCBC(unsigned char &type, unsigned char* &data,
                        unsigned int &length)
{
  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  size_t retval;

  ASSERT(m_fish != NULL && m_IV != NULL);
  retval = _readcbc(m_fd, buffer, buffer_len, type,
    m_fish, m_IV, m_terminal, m_fileLength);

  if (buffer_len > 0) {
    if (buffer_len < length || data == NULL)
      length = buffer_len; // set to length read
    // if buffer_len > length, data is truncated to length
    // probably an error.
    if (data != NULL) {
      memcpy(data, buffer, length);
      trashMemory(buffer, buffer_len);
      delete[] buffer;
    } else { // NULL data means pass buffer directly to caller
      data = buffer; // caller must trash & delete[]!
    }
  } else {
    // no need to delete[] buffer, since _readcbc will not allocate if
    // buffer_len is zero
  }
  return retval;
}

int PWSfile::CheckPassword(const StringX &filename,
                           const StringX &passkey, VERSION &version)
{

  if (passkey.empty())
    return WRONG_PASSWORD;

  int status;
  version = UNKNOWN_VERSION;
  status = PWSfileV3::CheckPassword(filename, passkey);
  if (status == SUCCESS)
    version = V30;
  if (status == NOT_PWS3_FILE) {
    status = PWSfileV1V2::CheckPassword(filename, passkey);
    if (status == SUCCESS)
      version = V20; // or V17?
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

// Following for 'legacy' use of pwsafe as file encryptor/decryptor
// this is for the undocumented 'command line file encryption'
static const stringT CIPHERTEXT_SUFFIX(_S(".PSF"));


static stringT
ErrorMessages()
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
  default:
    break;
  }
  return cs_text;
}

// Following specific for PWSfile::Encrypt
#define SAFE_FWRITE(p, sz, cnt, stream) do { \
    size_t _ret = fwrite(p, sz, cnt, stream); \
    if (_ret != cnt) { status = false; goto exit;} \
  } while (0)

bool PWSfile::Encrypt(const stringT &fn, const StringX &passwd, stringT &errmess)
{
  unsigned int len = 0;
  unsigned char* buf = NULL;
  Fish *fish = NULL;
  bool status = true;
  stringT out_fn;
  unsigned char *pwd = NULL;
  int passlen = 0;
  FILE *out = NULL;

  FILE *in = pws_os::FOpen(fn, _T("rb"));;
  if (in != NULL) {
    len = pws_os::fileLength(in);
    buf = new unsigned char[len];

    fread(buf, 1, len, in);
    if (ferror(in)) { // this is how to detect fread errors
      status = false;
      int save_error = errno;
      fclose(in);
      errno = save_error;
      goto exit;
    }
    if (fclose(in) != 0) {
      status = false;
      goto exit;
    }
  } else {
    status = false; goto exit;
  }

  out_fn = fn;
  out_fn += CIPHERTEXT_SUFFIX;

  out = pws_os::FOpen(out_fn, _T("wb"));
  if (out == NULL) {
    status = false; goto exit;
  }
#ifdef KEEP_FILE_MODE_BWD_COMPAT
  SAFE_FWRITE( &len, 1, sizeof(len), out);
#else
  unsigned char randstuff[StuffSize];
  unsigned char randhash[SHA1::HASHLEN];   // HashSize
  PWSrand::GetInstance()->GetRandomData( randstuff, 8 );
  // miserable bug - have to fix this way to avoid breaking existing files
  randstuff[8] = randstuff[9] = TCHAR('\0');
  GenRandhash(passwd, randstuff, randhash);
  SAFE_FWRITE(randstuff, 1,  8, out);
  SAFE_FWRITE(randhash,  1, sizeof(randhash), out);
#endif // KEEP_FILE_MODE_BWD_COMPAT

  unsigned char thesalt[SaltLength];
  PWSrand::GetInstance()->GetRandomData( thesalt, SaltLength );
  SAFE_FWRITE(thesalt, 1, SaltLength, out);

  unsigned char ipthing[8];
  PWSrand::GetInstance()->GetRandomData( ipthing, 8 );
  SAFE_FWRITE(ipthing, 1, 8, out);

  ConvertString(passwd, pwd, passlen);
  fish = BlowFish::MakeBlowFish(pwd, passlen, thesalt, SaltLength);
  trashMemory(pwd, passlen);
#ifdef UNICODE
  delete[] pwd; // gross - ConvertString allocates only if UNICODE.
#endif
  try {
    _writecbc(out, buf, len, (unsigned char)0, fish, ipthing);
  } catch (...) { // _writecbc throws an exception if it fails to write
    fclose(out);
    errno = EIO;
    status = false;
    goto exit;
  }
  status = (fclose(out) == 0);
 exit:
  if (!status)
    errmess = ErrorMessages();
  delete fish;
  delete[] buf;
  return status;
}

bool PWSfile::Decrypt(const stringT &fn, const StringX &passwd, stringT &errmess)
{
  unsigned int len;
  unsigned char* buf = NULL;
  bool status = true;
  unsigned char salt[SaltLength];
  unsigned char ipthing[8];
  unsigned char randstuff[StuffSize];
  unsigned char randhash[SHA1::HASHLEN];
  unsigned char temphash[SHA1::HASHLEN];

  FILE *in = pws_os::FOpen(fn, _T("rb"));
  if (in == NULL) {
    status = false;
    goto exit;
  }

#ifdef KEEP_FILE_MODE_BWD_COMPAT
  fread(&len, 1, sizeof(len), in); // XXX portability issue
#else
  fread(randstuff, 1, 8, in);
  randstuff[8] = randstuff[9] = TCHAR('\0'); // ugly bug workaround
  fread(randhash, 1, sizeof(randhash), in);

  GenRandhash(passwd, randstuff, temphash);
  if (memcmp((char*)randhash, (char*)temphash, SHA1::HASHLEN != 0)) {
    fclose(in);
    LoadAString(errmess, IDSC_BADPASSWORD);
    return false;
  }
#endif // KEEP_FILE_MODE_BWD_COMPAT

  { // decryption in a block, since we use goto
    fread(salt,    1, SaltLength, in);
    fread(ipthing, 1, 8,          in);

    unsigned char dummyType;
    unsigned char *pwd = NULL;
    int passlen = 0;
    long file_len = pws_os::fileLength(in);
    ConvertString(passwd, pwd, passlen);
    Fish *fish = BlowFish::MakeBlowFish(pwd, passlen, salt, SaltLength);
    trashMemory(pwd, passlen);
#ifdef UNICODE
    delete[] pwd; // gross - ConvertString allocates only if UNICODE.
#endif
    if (_readcbc(in, buf, len,dummyType, fish, ipthing, 0, file_len) == 0) {
      delete fish;
      delete[] buf; // if not yet allocated, delete[] NULL, which is OK
      return false;
    }
    delete fish;
    fclose(in);
  } // decrypt

  { // write decrypted data
    size_t suffix_len = CIPHERTEXT_SUFFIX.length();
    size_t filepath_len = fn.length();

    stringT out_fn = fn;
    out_fn = out_fn.substr(0,filepath_len - suffix_len);

    FILE *out = pws_os::FOpen(out_fn, _T("wb"));
    if (out != NULL) {
      size_t fret = fwrite(buf, 1, len, out);
      if (fret != len) {
        int save_errno = errno;
        fclose(out);
        errno = save_errno;
        goto exit;
      }
      if (fclose(out) != 0) {
        status = false;
        goto exit;
      }
    } else { // open failed
      status = false;
      goto exit;
    }
  } // write decrypted
 exit:
  if (!status)
    errmess = ErrorMessages();
  delete[] buf; // allocated by _readcbc
  return status;
}
