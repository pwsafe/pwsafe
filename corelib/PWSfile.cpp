/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "PWSfile.h"
#include "PWSfileV1V2.h"
#include "PWSfileV3.h"
#include "SysInfo.h"
#include "corelib.h"

#include <LMCONS.H> // for UNLEN definition
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int PWSfile::m_nITER;

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


bool PWSfile::FileExists(const CMyString &filename)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);
  return (status == 0);
}

bool PWSfile::FileExists(const CMyString &filename, bool &bReadOnly)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);
  
  // As "stat" gives "user permissions" not "file attributes"....
  if (status == 0) {
	  DWORD dwAttr = GetFileAttributes(filename);
	  bReadOnly = (FILE_ATTRIBUTE_READONLY & dwAttr) == FILE_ATTRIBUTE_READONLY;
	  return true;
  } else {
	  bReadOnly = false;
	  return false;
  }
}
void PWSfile::FileError(int formatRes, int cause)
{
    // present error from FileException to user
	CString cs_error, cs_msg;

	ASSERT(cause >= 0 && cause <= 14);
	cs_error.LoadString(IDSC_FILEEXCEPTION00 + cause);
	cs_msg.Format(formatRes, cs_error);
	AfxMessageBox(cs_msg, MB_OK);
}

PWSfile::VERSION PWSfile::ReadVersion(const CMyString &filename)
{
  if (FileExists(filename)) {
    VERSION v;
    if (PWSfileV3::IsV3x(filename, v))
      return v;
    else
      return V20;
  } else
    return UNKNOWN_VERSION;
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
    m_fd(NULL), m_prefString(_T("")), m_fish(NULL), m_terminal(NULL),
    m_file_displaystatus(_T("")), m_whenlastsaved(_T("")),
    m_wholastsaved(_T("")), m_whatlastsaved(_T(""))
{
}

PWSfile::~PWSfile()
{
  Close(); // idempotent
}

void PWSfile::FOpen()
{
  TCHAR* m = (m_rw == Read) ? _T("rb") : _T("wb");
  // calls right variant of m_fd = fopen(m_filename);
#if _MSC_VER >= 1400
  _tfopen_s(&m_fd, (LPCTSTR) m_filename, m);
#else
  m_fd = _tfopen((LPCTSTR) m_filename, m);
#endif
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

size_t PWSfile::WriteCBC(unsigned char type, const CString &data)
{
  // We do a double cast because the LPCTSTR cast operator is overridden
  // by the CString class to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCTSTR datastr = LPCTSTR(data);

  return WriteCBC(type, (const unsigned char *)datastr,
                  data.GetLength());
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
                    m_fish, m_IV, m_terminal);

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

int PWSfile::CheckPassword(const CMyString &filename,
                           const CMyString &passkey, VERSION &version)
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

/*
 * The file lock/unlock functions were first implemented (in 2.08)
 * with Posix semantics (using open(_O_CREATE|_O_EXCL) to detect
 * an existing lock.
 * This fails to check liveness of the locker process, specifically,
 * if a user just turns of her PC, the lock file will remain.
 * So, I'm keeping the Posix code under idef POSIX_FILE_LOCK,
 * and re-implementing using the Win32 API, whose semantics
 * supposedly protect against this scenario.
 * Thanks to Frank (xformer) for discussion on the subject.
 */


/*
 * Originally, the lock/unlock functions were designed for working with the
 * passsword database file alone.
 * As of 3.05, there's a need to lock the application's configuration
 * file as well, potentially concurrently with the database file.
 * Problem is, we need to keep state information (HANDLE, LockCount) per
 * locked file. since the filenames are unique, indeally we'd maintain a map
 * between the filename and the resources. For now, we require the application
 * to differentiate between them for us by the bool param bDB. Less elegant,
 * but *much* easier than setting up a map...
 */

static HANDLE s_lockFileHandle[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
static int s_LockCount[2] = {0, 0};


static void GetLockFileName(const CMyString &filename,
                            CMyString &lock_filename)
{
  ASSERT(!filename.IsEmpty());
  // derive lock filename from filename
  lock_filename = CMyString(filename.Left(MAX_PATH - 4) + _T(".plk"));
}

bool PWSfile::LockFile(const CMyString &filename, CMyString &locker, const bool bDB)
{
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
#ifdef POSIX_FILE_LOCK
  int fh = _open(lock_filename, (_O_CREAT | _O_EXCL | _O_WRONLY),
                 (_S_IREAD | _S_IWRITE));

  if (fh == -1) { // failed to open exclusively. Already locked, or ???
    switch (errno) {
    case EACCES:
      // Tried to open read-only file for writing, or file’s
      // sharing mode does not allow specified operations, or given path is directory
	  locker.LoadString(IDSC_NOLOCKACCESS);
      break;
    case EEXIST: // filename already exists
      {
        // read locker data ("user@machine:nnnnnnnn") from file
        TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR) * 11];
        int fh2 = _open(lock_filename, _O_RDONLY);
        if (fh2 == -1) {
          locker.LoadString(IDSC_CANTGETLOCKER);
        } else {
          int bytesRead = _read(fh2, lockerStr, sizeof(lockerStr)-1);
          _close(fh2);
          if (bytesRead > 0) {
            lockerStr[bytesRead] = TCHAR('\0');
            locker = lockerStr;
          } else { // read failed for some reason
            locker = _T("Unable to read locker?");
          } // read info from lock file
        } // open lock file for read
      } // EEXIST block
      break;
    case EINVAL: // Invalid oflag or pmode argument
      locker.LoadString(IDSC_INTERNALLOCKERROR);
      break;
    case EMFILE: // No more file handles available (too many open files)
      locker.LoadString(IDSC_SYSTEMLOCKERROR);
      break;
    case ENOENT: //File or path not found
      locker.LoadString(IDSC_LOCKFILEPATHNF);
      break;
    default:
      locker.LoadString(IDSC_LOCKUNKNOWNERROR);
      break;
    } // switch (errno)
    return false;
  } else { // valid filehandle, write our info
    int numWrit;
    numWrit = _write(fh, m_user, m_user.GetLength() * sizeof(TCHAR));
    numWrit += _write(fh, _T("@"), sizeof(TCHAR));
    numWrit += _write(fh, m_sysname, m_sysname.GetLength() * sizeof(TCHAR));
    numWrit += _write(fh, _T(":"), sizeof(TCHAR));
    numWrit += _write(fh, m_ProcessID, m_ProcessID.GetLength() * sizeof(TCHAR));
    ASSERT(numWrit > 0);
    _close(fh);
    return true;
  }
#else
  const SysInfo *si = SysInfo::GetInstance();
  const CString user = si->GetCurrentUser();
  const CString host = si->GetCurrentHost();
  const CString pid = si->GetCurrentPID();

  const int iLFHandle = bDB ? DATABASE_LOCK : CONFIG_LOCK;
  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (s_lockFileHandle[iLFHandle] != INVALID_HANDLE_VALUE) {
    // here if we've open another (or same) dbase previously,
    // need to unlock it. A bit inelegant...
    // If app was minimized and ClearData() called, we've a small
    // potential for a TOCTTOU issue here. Worse case, lock
    // will fail.

    const CString cs_me = user + _T("@") + host + _T(":") + pid;
    GetLockFileName(filename, lock_filename);
	GetLocker(lock_filename, locker);

	if (cs_me == CString(locker)) {
      s_LockCount[iLFHandle]++;
      TRACE(_T("%s Lock1   %s, Count now %d\n"), 
			PWSUtil::GetTimeStamp(), iLFHandle == 0 ? _T("DB") : _T("CF"),
			s_LockCount[iLFHandle]);
      locker.Empty();
      return true;
	} else {
      // XXX UnlockFile(bDB ? GetCurFile() : filename, bDB);
      UnlockFile(filename, bDB);
	}
  }
  s_lockFileHandle[iLFHandle] = ::CreateFile(LPCTSTR(lock_filename),
                                             GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             CREATE_ALWAYS, // rely on share to fail if exists!
                                             FILE_ATTRIBUTE_NORMAL| FILE_FLAG_WRITE_THROUGH,
                                             NULL);
  if (s_lockFileHandle[iLFHandle] == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    switch (error) {
    case ERROR_SHARING_VIOLATION: // already open by a live process
	  GetLocker(lock_filename, locker);
      break;
    default:
      locker = _T("Cannot create lock file - no permission in directory?");
      break;
    } // switch (error)
    return false;
  } else { // valid filehandle, write our info
    DWORD numWrit, sumWrit;
    BOOL write_status;
    write_status = ::WriteFile(s_lockFileHandle[iLFHandle],
                               user, user.GetLength() * sizeof(TCHAR),
                               &sumWrit, NULL);
    write_status &= ::WriteFile(s_lockFileHandle[iLFHandle],
                               _T("@"), sizeof(TCHAR),
                               &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(s_lockFileHandle[iLFHandle],
                                host, host.GetLength() * sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(s_lockFileHandle[iLFHandle],
                               _T(":"), sizeof(TCHAR),
                               &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(s_lockFileHandle[iLFHandle],
                                pid, pid.GetLength() * sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    ASSERT(sumWrit > 0);
	s_LockCount[iLFHandle]++;
	TRACE(_T("%s Lock1   %s, Count now %d; File Created\n"), 
          PWSUtil::GetTimeStamp(), iLFHandle == 0 ? _T("DB") : _T("CF"),
          s_LockCount[iLFHandle]);
    return (write_status == TRUE);
  }
#endif // POSIX_FILE_LOCK
}

void PWSfile::UnlockFile(const CMyString &filename, const bool bDB)
{
#ifdef POSIX_FILE_LOCK
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
  _unlink(lock_filename);
#else
  const SysInfo *si = SysInfo::GetInstance();
  const CString user = si->GetCurrentUser();
  const CString host = si->GetCurrentHost();
  const CString pid = si->GetCurrentPID();

  const int iLFHandle = bDB ? DATABASE_LOCK : CONFIG_LOCK;
  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (s_lockFileHandle[iLFHandle] != INVALID_HANDLE_VALUE) {
    CMyString lock_filename, locker;
	const CString cs_me = user + _T("@") + host + _T(":") + pid;
    GetLockFileName(filename, lock_filename);
	GetLocker(lock_filename, locker);

	if (cs_me == CString(locker) && s_LockCount[iLFHandle] > 1) {
		s_LockCount[iLFHandle]--;
		TRACE(_T("%s Unlock2 %s, Count now %d\n"), 
			PWSUtil::GetTimeStamp(), iLFHandle == 0 ? _T("DB") : _T("CF"),
			s_LockCount[iLFHandle]);
	} else {
		s_LockCount[iLFHandle] = 0;
		TRACE(_T("%s Unlock1 %s, Count now %d; File Deleted\n"), 
			PWSUtil::GetTimeStamp(), iLFHandle == 0 ? _T("DB") : _T("CF"),
			s_LockCount[iLFHandle]);
		CloseHandle(s_lockFileHandle[iLFHandle]);
		s_lockFileHandle[iLFHandle] = INVALID_HANDLE_VALUE;
		DeleteFile(LPCTSTR(lock_filename));
	}
  }
#endif // POSIX_FILE_LOCK
}

bool PWSfile::IsLockedFile(const CMyString &filename, const bool)
{
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
#ifdef POSIX_FILE_LOCK
  return PWSfile::FileExists(lock_filename);
#else
  // under this scheme, we need to actually try to open the file to determine
  // if it's locked.
  HANDLE h = CreateFile(LPCTSTR(lock_filename),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING, // don't create one!
			FILE_ATTRIBUTE_NORMAL| FILE_FLAG_WRITE_THROUGH,
			NULL);
  if (h == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    if (error == ERROR_SHARING_VIOLATION)
      return true;
    else
      return false; // couldn't open it, probably doesn't exist.
  } else {
    CloseHandle(h); // here if exists but lockable.
    return false;
  }
#endif // POSIX_FILE_LOCK
}

bool PWSfile::GetLocker(const CMyString &lock_filename, CMyString &locker)
{
	bool bResult = false;
	// read locker data ("user@machine:nnnnnnnn") from file
	TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + 11];
	// flags here counter (my) intuition, but see
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/creating_and_opening_files.asp
	HANDLE h2 = ::CreateFile(LPCTSTR(lock_filename),
							GENERIC_READ, FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL, NULL);
	if (h2 == INVALID_HANDLE_VALUE) {
		locker.LoadString(IDSC_CANTGETLOCKER);
	} else {
		DWORD bytesRead;
		(void)::ReadFile(h2, lockerStr, sizeof(lockerStr)-1,
					&bytesRead, NULL);
		CloseHandle(h2);
		if (bytesRead > 0) {
			lockerStr[bytesRead/sizeof(TCHAR)] = TCHAR('\0');
			locker = lockerStr;
			bResult = true;
		} else { // read failed for some reason
			locker.LoadString(IDSC_CANTREADLOCKER);
		} // read info from lock file
	}
	return bResult;
}

void PWSfile::SetFileUUID(const uuid_array_t &file_uuid_array)
{
  memcpy(m_file_uuid_array, file_uuid_array, sizeof(file_uuid_array));
}

void PWSfile::GetFileUUID(uuid_array_t &file_uuid_array)
{
  memcpy(file_uuid_array, m_file_uuid_array, sizeof(file_uuid_array));
}

void PWSfile::GetUnknownHeaderFields(UnknownHeaderFieldList &UHFL)
{
  if (!m_UHFL.empty()) {
    UnknownHeaderFieldList::const_iterator vi_IterUHFE;
    for (vi_IterUHFE = m_UHFL.begin();
         vi_IterUHFE != m_UHFL.end();
         vi_IterUHFE++) {
       UnknownFieldEntry unkhfe = *vi_IterUHFE;
       UHFL.push_back(unkhfe);
    }
  } else
    UHFL.clear();
}

void PWSfile::SetUnknownHeaderFields(UnknownHeaderFieldList &UHFL)
{
  if (!UHFL.empty()) {
    UnknownHeaderFieldList::const_iterator vi_IterUHFE;
    for (vi_IterUHFE = UHFL.begin();
         vi_IterUHFE != UHFL.end();
         vi_IterUHFE++) {
       UnknownFieldEntry unkhfe = *vi_IterUHFE;
       m_UHFL.push_back(unkhfe);
    }
  } else
    m_UHFL.clear();
}
