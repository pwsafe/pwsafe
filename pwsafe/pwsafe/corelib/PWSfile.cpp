#include "PWSfile.h"
#include "PWSfileV1V2.h"

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
    m_fd(NULL), m_prefString(_T(""))
{
}

PWSfile::~PWSfile()
{
  Close(); // idempotent
}


int PWSfile::Close()
{
  if (m_fd != NULL) {
    fclose(m_fd);
    m_fd = NULL;
  }
  return SUCCESS;
}

int PWSfile::CheckPassword(const CMyString &filename, const CMyString &passkey)
{
  // XXX start with V3 check
  int status = PWSfileV1V2::CheckPassword(filename, passkey);
  return status;
}


