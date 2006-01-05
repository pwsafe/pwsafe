#include "PWSfile.h"
#include "PWSfileV1V2.h"
#include "PWSfileV3.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

PWSfile *PWSfile::MakePWSfile(const CMyString &a_filename, VERSION &version,
                              RWmode mode, int &status)
{
  if (mode == Read && !FileExists(a_filename)) {
    status = CANT_OPEN_FILE;
    return NULL;
  }

  switch (version) {
  case V17:
  case V20:
    status = SUCCESS;
    return new PWSfileV1V2(a_filename, mode, version);
  case V30: // XXX ...
    status = UNSUPPORTED_VERSION;
    return NULL;
  case UNKNOWN_VERSION:
    ASSERT(mode == Read);
    // XXX need to quickly determine file version
    version = V20;
    status = SUCCESS;
    return new PWSfileV1V2(a_filename, mode, version);
  default:
    ASSERT(0);
    status = FAILURE; return NULL;
  }
}


bool PWSfile::FileExists(const CMyString &filename)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);
  return (status == 0);
}

int PWSfile::RenameFile(const CMyString &oldname, const CMyString &newname)
{
  _tremove(newname); // otherwise rename will fail if newname exists
  int status = _trename(oldname, newname);

  return (status == 0) ? SUCCESS : FAILURE;
}


PWSfile::PWSfile(const CMyString &filename, RWmode mode)
  : m_filename(filename), m_passkey(_T("")),  m_defusername(_T("")),
    m_curversion(UNKNOWN_VERSION), m_rw(mode),
    m_fd(NULL), m_prefString(_T("")), m_fish(NULL)
{
}

PWSfile::~PWSfile()
{
  Close(); // idempotent
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

int PWSfile::WriteCBC(unsigned char type, const CString &data)
{
  // We do a double cast because the LPCSTR cast operator is overridden
  // by the CString class to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR datastr = LPCSTR(data);

  return WriteCBC(type, (const unsigned char *)datastr,
                  data.GetLength());
}

int PWSfile::WriteCBC(unsigned char type, const unsigned char *data,
                          unsigned int length)
{
  ASSERT(m_fish != NULL && m_IV != NULL);
  return _writecbc(m_fd, data, length, type, m_fish, m_IV);
}

int PWSfile::ReadCBC(unsigned char &type, CMyString &data)
{

  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  int retval;

  ASSERT(m_fish != NULL && m_IV != NULL);
  retval = _readcbc(m_fd, buffer, buffer_len, type, m_fish, m_IV);

  if (buffer_len > 0) {
    CMyString str(LPCSTR(buffer), buffer_len);
    data = str;
    trashMemory(buffer, buffer_len);
    delete[] buffer;
  } else {
    data = _T("");
    // no need to delete[] buffer, since _readcbc will not allocate if
    // buffer_len is zero
  }
  return retval;
}

int PWSfile::ReadCBC(unsigned char &type, unsigned char *data,
                     unsigned int &length)
{

  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  int retval;

  ASSERT(m_fish != NULL && m_IV != NULL);
  retval = _readcbc(m_fd, buffer, buffer_len, type, m_fish, m_IV);

  if (buffer_len > 0) {
    if (buffer_len < length)
      length = buffer_len; // set to length read
    // if buffer_len > length, data is truncated to length
    // probably an error.
    memcpy(data, buffer, length);
    trashMemory(buffer, buffer_len);
    delete[] buffer;
  } else {
    // no need to delete[] buffer, since _readcbc will not allocate if
    // buffer_len is zero
  }
  return retval;
}


int PWSfile::CheckPassword(const CMyString &filename, const CMyString &passkey)
{
  int status = PWSfileV3::CheckPassword(filename, passkey);
  if (status == NOT_PWS3_FILE)
    status = PWSfileV1V2::CheckPassword(filename, passkey);
  return status;
}


