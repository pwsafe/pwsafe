/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "PWSAttfile.h"
#include "PWSAttfileV3.h"
#include "corelib.h"
#include "os/file.h"
#include "return_codes.h"

#include <fcntl.h>

PWSAttfile *PWSAttfile::MakePWSfile(const StringX &a_filename, VERSION &version,
                                    RWmode mode, int &status,
                                    Asker *pAsker, Reporter *pReporter)
{
  if (mode == Read && !pws_os::FileExists(a_filename.c_str())) {
    status = PWSRC::CANT_OPEN_FILE;
    return NULL;
  }

  PWSAttfile *retval = new PWSAttfileV3(a_filename, mode, version);

  retval->m_pAsker = pAsker;
  retval->m_pReporter = pReporter;
  status = PWSRC::SUCCESS;
  return retval;
}

PWSAttfile::VERSION PWSAttfile::ReadVersion(const StringX &filename)
{
  if (pws_os::FileExists(filename.c_str())) {
    VERSION v;
    if (PWSAttfileV3::IsV3x(filename, v))
      return v;
    else
      return UNKNOWN_VERSION;
  } else
    return UNKNOWN_VERSION;
}

PWSAttfile::PWSAttfile(const StringX &filename, RWmode mode)
  : m_filename(filename), m_passkey(_T("")), m_fd(NULL),
  m_curversion(UNKNOWN_VERSION), m_rw(mode),
  m_fish(NULL), m_terminal(NULL)
{
}

PWSAttfile::~PWSAttfile()
{
  Close(); // idempotent
}

PWSAttfile::AttHeaderRecord::AttHeaderRecord()
  : nCurrentMajorVersion(0), nCurrentMinorVersion(0),
  nITER(0),
  whenlastsaved(0), whatlastsaved(_T("")),
  lastsavedby(_T("")), lastsavedon(_T(""))
{
  memset(attfile_uuid, 0, sizeof(uuid_array_t));
  memset(DBfile_uuid, 0, sizeof(uuid_array_t));
}

PWSAttfile::AttHeaderRecord::AttHeaderRecord(const PWSAttfile::AttHeaderRecord &ahr)
  : nCurrentMajorVersion(ahr.nCurrentMajorVersion),
  nCurrentMinorVersion(ahr.nCurrentMinorVersion),
  nITER(ahr.nITER),
  whenlastsaved(ahr.whenlastsaved), whatlastsaved(ahr.whatlastsaved),
  lastsavedby(ahr.lastsavedby), lastsavedon(ahr.lastsavedon)
{
  memcpy(attfile_uuid, ahr.attfile_uuid, sizeof(uuid_array_t));
  memcpy(DBfile_uuid, ahr.DBfile_uuid, sizeof(uuid_array_t));
}

PWSAttfile::AttHeaderRecord &PWSAttfile::AttHeaderRecord::operator=(const PWSAttfile::AttHeaderRecord &ahr)
{
  if (this != &ahr) {
    nITER = ahr.nITER;
    nCurrentMajorVersion = ahr.nCurrentMajorVersion;
    nCurrentMinorVersion = ahr.nCurrentMinorVersion;
    whenlastsaved = ahr.whenlastsaved;
    whatlastsaved = ahr.whatlastsaved;
    lastsavedby = ahr.lastsavedby;
    lastsavedon = ahr.lastsavedon;
    memcpy(attfile_uuid, ahr.attfile_uuid, sizeof(uuid_array_t));
    memcpy(DBfile_uuid, ahr.DBfile_uuid, sizeof(uuid_array_t));
  }
  return *this;
}

void PWSAttfile::FOpen()
{
  const TCHAR* m = (m_rw == Read) ? _T("rb") : _T("wb");
  if (m_fd != NULL) {
    fclose(m_fd);
    m_fd = NULL;
  }
  m_fd = pws_os::FOpen(m_filename.c_str(), m);
  m_fileLength = pws_os::fileLength(m_fd);
}

int PWSAttfile::Close()
{
  delete m_fish;
  m_fish = NULL;
  if (m_fd != NULL) {
    fflush(m_fd);
    fclose(m_fd);
    m_fd = NULL;
  }
  return PWSRC::SUCCESS;
}

size_t PWSAttfile::WriteCBC(unsigned char type, const unsigned char *data,
                            unsigned int length)
{
  ASSERT(m_fish != NULL && m_IV != NULL);
  return _writecbc(m_fd, data, length, type, m_fish, m_IV);
}

size_t PWSAttfile::ReadCBC(unsigned char &type, unsigned char* &data,
                           unsigned int &length,
                           bool bSkip, unsigned char *pSkipTypes)
{
  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  size_t retval;

  ASSERT(m_fish != NULL && m_IV != NULL);
  retval = _readcbc(m_fd, buffer, buffer_len, type,
                    m_fish, m_IV, m_terminal, m_fileLength,
                    bSkip, pSkipTypes);

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

int PWSAttfile::CheckPasskey(const StringX &filename,
                            const StringX &passkey, VERSION &version)
{
  if (passkey.empty())
    return PWSRC::WRONG_PASSWORD;

  int status;
  version = UNKNOWN_VERSION;
  status = PWSAttfileV3::CheckPasskey(filename, passkey);
  if (status == PWSRC::SUCCESS)
    version = V30;
  return status;
}
