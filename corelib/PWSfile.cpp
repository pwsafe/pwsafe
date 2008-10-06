/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
                              RWmode mode, int &status)
{
  if (mode == Read && !pws_os::FileExists(a_filename.c_str())) {
    status = CANT_OPEN_FILE;
    return NULL;
  }

  switch (version) {
    case V17:
    case V20:
      status = SUCCESS;
      return new PWSfileV1V2(a_filename, mode, version);
    case V30:
      status = SUCCESS;
      return new PWSfileV3(a_filename, mode, version);
    case UNKNOWN_VERSION:
      ASSERT(mode == Read);
      if (PWSfile::ReadVersion(a_filename) == V30) {
        version = V30;
        status = SUCCESS;
        return new PWSfileV3(a_filename, mode, version);
      } else {
        version = V20; // may be inaccurate (V17)
        status = SUCCESS;
        return new PWSfileV1V2(a_filename, mode, version);
      }
    default:
      ASSERT(0);
      status = FAILURE; return NULL;
  }
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
  // calls right variant of m_fd = fopen(m_filename);
#if _MSC_VER >= 1400
  _tfopen_s(&m_fd, m_filename.c_str(), m);
#else
  m_fd = _tfopen(m_filename.c_str(), m);
#endif
  m_fileLength = PWSUtil::fileLength(m_fd);
}

int PWSfile::Close()
{
  delete m_fish;
  m_fish = NULL;
  if (m_fd != NULL) {
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
  default:
    break;
  }
  return cs_text;
}

bool PWSfile::Encrypt(const stringT &fn, const StringX &passwd, stringT &errmess)
{
  unsigned int len;
  unsigned char* buf;

  FILE *in;
#if _MSC_VER >= 1400
  _tfopen_s(&in, fn.c_str(), _T("rb"));
#else
  in = _tfopen(fn.c_str(), _T("rb"));
#endif
  if (in != NULL) {
    len = PWSUtil::fileLength(in);
    buf = new unsigned char[len];

    fread(buf, 1, len, in);
    fclose(in);
  } else {
    errmess = ErrorMessages();
    return false;
  }

  stringT out_fn = fn;
  out_fn += CIPHERTEXT_SUFFIX;

  FILE *out;
#if _MSC_VER >= 1400
  _tfopen_s(&out, out_fn.c_str(), _T("wb"));
#else
  out = _tfopen(out_fn.c_str(), _T("wb"));
#endif
  if (out != NULL) {
#ifdef KEEP_FILE_MODE_BWD_COMPAT
    fwrite( &len, 1, sizeof(len), out);
#else
    unsigned char randstuff[StuffSize];
    unsigned char randhash[SHA1::HASHLEN];   // HashSize
    PWSrand::GetInstance()->GetRandomData( randstuff, 8 );
    // miserable bug - have to fix this way to avoid breaking existing files
    randstuff[8] = randstuff[9] = TCHAR('\0');
    GenRandhash(passwd, randstuff, randhash);
    fwrite(randstuff, 1,  8, out);
    fwrite(randhash,  1, sizeof(randhash), out);
#endif // KEEP_FILE_MODE_BWD_COMPAT

    unsigned char thesalt[SaltLength];
    PWSrand::GetInstance()->GetRandomData( thesalt, SaltLength );
    fwrite(thesalt, 1, SaltLength, out);

    unsigned char ipthing[8];
    PWSrand::GetInstance()->GetRandomData( ipthing, 8 );
    fwrite(ipthing, 1, 8, out);

    unsigned char *pwd = NULL;
    int passlen = 0;
    ConvertString(passwd, pwd, passlen);
    Fish *fish = BlowFish::MakeBlowFish(pwd, passlen, thesalt, SaltLength);
    trashMemory(pwd, passlen);
#ifdef UNICODE
    delete[] pwd; // gross - ConvertString allocates only if UNICODE.
#endif
    _writecbc(out, buf, len, (unsigned char)0, fish, ipthing);
    delete fish;
    fclose(out);

  } else {
    errmess = ErrorMessages();
    delete [] buf;
    return false;
  }
  delete[] buf;
  return true;
}

bool PWSfile::Decrypt(const stringT &fn, const StringX &passwd, stringT &errmess)
{
  unsigned int len;
  unsigned char* buf;

  FILE *in;
#if _MSC_VER >= 1400
  _tfopen_s(&in, fn.c_str(), _T("rb"));
#else
  in = _tfopen(fn.c_str(), _T("rb"));
#endif
  if (in != NULL) {
    unsigned char salt[SaltLength];
    unsigned char ipthing[8];
    unsigned char randstuff[StuffSize];
    unsigned char randhash[SHA1::HASHLEN];

#ifdef KEEP_FILE_MODE_BWD_COMPAT
    fread(&len, 1, sizeof(len), in); // XXX portability issue
#else
    fread(randstuff, 1, 8, in);
    randstuff[8] = randstuff[9] = TCHAR('\0'); // ugly bug workaround
    fread(randhash, 1, sizeof(randhash), in);

    unsigned char temphash[SHA1::HASHLEN];
    GenRandhash(passwd, randstuff, temphash);
    if (memcmp((char*)randhash, (char*)temphash, SHA1::HASHLEN != 0)) {
      fclose(in);
      LoadAString(errmess, IDSC_BADPASSWORD);
      return false;
    }
#endif // KEEP_FILE_MODE_BWD_COMPAT
    buf = NULL; // allocated by _readcbc - see there for apologia

    fread(salt,    1, SaltLength, in);
    fread(ipthing, 1, 8,          in);

    unsigned char dummyType;
    unsigned char *pwd = NULL;
    int passlen = 0;
    long file_len = PWSUtil::fileLength(in);
	if (file_len == -1L)
		file_len = 0;
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
  } else {
    errmess = ErrorMessages();
    return false;
  }

  size_t suffix_len = CIPHERTEXT_SUFFIX.length();
  size_t filepath_len = fn.length();

  stringT out_fn = fn;
  out_fn = out_fn.substr(0,filepath_len - suffix_len);

#if _MSC_VER >= 1400
  FILE *out;
  _tfopen_s(&out, out_fn.c_str(), _T("wb"));
#else
  FILE *out = _tfopen(out_fn.c_str(), _T("wb"));
#endif
  if (out != NULL) {
    fwrite(buf, 1, len, out);
    delete[] buf; // allocated by _readcbc
    fclose(out);
  } else {
    delete[] buf; // allocated by _readcbc
    errmess = ErrorMessages();
    return false;
  }
  return true;
}
