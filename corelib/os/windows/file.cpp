/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of file.h
 */
#include <afx.h>
#include <Windows.h>
#include <LMCONS.H> // for UNLEN definition
#include <shellapi.h>
#include <shlwapi.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../typedefs.h"
#include "../file.h"
#include "../dir.h"
#include "../env.h"

const TCHAR *pws_os::PathSeparator = _T("\\");

bool pws_os::FileExists(const stringT &filename)
{
  struct _stat statbuf;
  int status;

  status = _tstat(filename.c_str(), &statbuf);
  return (status == 0);
}

bool pws_os::FileExists(const stringT &filename, bool &bReadOnly)
{
  struct _stat statbuf;
  int status;

  status = _tstat(filename.c_str(), &statbuf);

  // As "stat" gives "user permissions" not "file attributes"....
  if (status == 0) {
    DWORD dwAttr = GetFileAttributes(filename.c_str());
    bReadOnly = (FILE_ATTRIBUTE_READONLY & dwAttr) == FILE_ATTRIBUTE_READONLY;
    return true;
  } else {
    bReadOnly = false;
    return false;
  }
}

bool pws_os::RenameFile(const stringT &oldname, const stringT &newname)
{
  _tremove(newname.c_str()); // otherwise rename will fail if newname exists
  int status = _trename(oldname.c_str(), newname.c_str());

  return (status == 0);
}

extern bool pws_os::CopyAFile(const stringT &from, const stringT &to)
{
  // Copy file and create any intervening directories as necessary & automatically
  TCHAR szSource[_MAX_PATH];
  TCHAR szDestination[_MAX_PATH];

  const TCHAR *lpsz_current = from.c_str();
  const TCHAR *lpsz_new = to.c_str();
#if _MSC_VER >= 1400
  _tcscpy_s(szSource, _MAX_PATH, lpsz_current);
  _tcscpy_s(szDestination, _MAX_PATH, lpsz_new);
#else
  _tcscpy(szSource, lpsz_current);
  _tcscpy(szDestination, lpsz_new);
#endif

  // Must end with double NULL
  szSource[from.length() + 1] = TCHAR('\0');
  szDestination[to.length() + 1] = TCHAR('\0');

  SHFILEOPSTRUCT sfop;
  memset(&sfop, 0, sizeof(sfop));
  sfop.hwnd = GetActiveWindow();
  sfop.wFunc = FO_COPY;
  sfop.pFrom = szSource;
  sfop.pTo = szDestination;
  sfop.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT;

  return (SHFileOperation(&sfop) == 0);
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

static stringT GetLockFileName(const stringT &filename)
{
  ASSERT(!filename.empty());
  // derive lock filename from filename
  stringT retval(filename, 0, filename.find_last_of(TCHAR('.')));
  retval += _T(".plk");
  return retval;
}

static void GetLocker(const stringT &lock_filename, stringT &locker)
{
  locker = _T("Unable to determine locker");
  // read locker data ("user@machine:nnnnnnnn") from file
  TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + 11];
  // flags here counter (my) intuition, but see
  // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/creating_and_opening_files.asp
  HANDLE h2 = ::CreateFile(lock_filename.c_str(),
                           GENERIC_READ,
                           FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           (FILE_ATTRIBUTE_NORMAL |
                            // (Lockheed Martin) Secure Coding  11-14-2007
                            SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION),
                           NULL);
  // Make sure it's a file and not a pipe.  (Lockheed Martin) Secure Coding  11-14-2007
  if (h2 != INVALID_HANDLE_VALUE) {
    if (::GetFileType( h2 ) != FILE_TYPE_DISK) {
      ::CloseHandle( h2 );
      h2 = INVALID_HANDLE_VALUE;
    }
  }
  // End of Change.  (Lockheed Martin) Secure Coding  11-14-2007
 
  if (h2 != INVALID_HANDLE_VALUE) {
    DWORD bytesRead;
    (void)::ReadFile(h2, lockerStr, sizeof(lockerStr)-1,
                     &bytesRead, NULL);
    CloseHandle(h2);
    if (bytesRead > 0) {
      lockerStr[bytesRead/sizeof(TCHAR)] = TCHAR('\0');
      locker = lockerStr;
    } // read info from lock file
  }
}

bool pws_os::LockFile(const stringT &filename, stringT &locker, 
                      HANDLE &lockFileHandle, int &LockCount)
{
  const stringT lock_filename = GetLockFileName(filename);
  stringT s_locker;
  const stringT user = pws_os::getusername();
  const stringT host = pws_os::gethostname();
  const stringT pid = pws_os::getprocessid();

  const stringT path(lock_filename);
  stringT drv, dir, fname, ext;

  pws_os::splitpath(path, drv, dir, fname, ext);

  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (lockFileHandle != INVALID_HANDLE_VALUE) {
    // here if we've open another (or same) dbase previously,
    // need to unlock it. A bit inelegant...
    // If app was minimized and ClearData() called, we've a small
    // potential for a TOCTTOU issue here. Worse case, lock
    // will fail.

    const stringT cs_me = user + _T("@") + host + _T(":") + pid;
    GetLocker(lock_filename, s_locker);

    if (cs_me == s_locker) {
      LockCount++;
      locker.clear();
      return true;
    } else {
      pws_os::UnlockFile(filename, lockFileHandle, LockCount);
    }
  }
  lockFileHandle = ::CreateFile(lock_filename.c_str(),
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS, // rely on share to fail if exists!
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH | 
                                // (Lockheed Martin) Secure Coding  11-14-2007
                                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                                NULL);

  // Make sure it's a file and not a pipe.  (Lockheed Martin) Secure Coding  11-14-2007
  if (lockFileHandle != INVALID_HANDLE_VALUE) {
    if (::GetFileType( lockFileHandle ) != FILE_TYPE_DISK) {
      ::CloseHandle( lockFileHandle );
      lockFileHandle = INVALID_HANDLE_VALUE;
    }
  }
  // End of Change.  (Lockheed Martin) Secure Coding  11-14-2007

  if (lockFileHandle == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    switch (error) {
    case ERROR_SHARING_VIOLATION: // already open by a live process
      GetLocker(lock_filename, s_locker);
      locker = s_locker.c_str();
      break;
    default:
      locker = _T("Cannot create lock file - no permission in directory?");
      break;
    } // switch (error)
    return false;
  } else { // valid filehandle, write our info
    DWORD numWrit, sumWrit;
    BOOL write_status;
    write_status = ::WriteFile(lockFileHandle,
                               user.c_str(), user.length() * sizeof(TCHAR),
                               &sumWrit, NULL);
    write_status &= ::WriteFile(lockFileHandle,
                                _T("@"), sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(lockFileHandle,
                                host.c_str(), host.length() * sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(lockFileHandle,
                                _T(":"), sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    write_status &= ::WriteFile(lockFileHandle,
                                pid.c_str(), pid.length() * sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    ASSERT(sumWrit > 0);
    LockCount++;
    return (write_status == TRUE);
  }
}

void pws_os::UnlockFile(const stringT &filename,
                        HANDLE &lockFileHandle, int &LockCount)
{
  const stringT user = pws_os::getusername();
  const stringT host = pws_os::gethostname();
  const stringT pid = pws_os::getprocessid();

  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (lockFileHandle != INVALID_HANDLE_VALUE) {
    stringT locker;
    const stringT lock_filename = GetLockFileName(filename);
    const stringT cs_me = user + _T("@") + host + _T(":") + pid;
    GetLocker(lock_filename, locker);

    const stringT path(lock_filename);
    stringT drv, dir, fname, ext;
    
    pws_os::splitpath(path, drv, dir, fname, ext);

    if (cs_me == locker && LockCount > 1) {
      LockCount--;
    } else {
      LockCount = 0;
      CloseHandle(lockFileHandle);
      lockFileHandle = INVALID_HANDLE_VALUE;
      DeleteFile(lock_filename.c_str());
    }
  }
}

bool pws_os::IsLockedFile(const stringT &filename)
{
  const stringT lock_filename = GetLockFileName(filename);
  // under this scheme, we need to actually try to open the file to determine
  // if it's locked.
  HANDLE h = CreateFile(lock_filename.c_str(),
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING, // don't create one!
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH |
                        // (Lockheed Martin) Secure Coding  11-14-2007
                        SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                        NULL);
 
  // Make sure it's a file and not a pipe.  (Lockheed Martin) Secure Coding  11-14-2007
  if (h != INVALID_HANDLE_VALUE) {
    if (::GetFileType( h ) != FILE_TYPE_DISK) {
      ::CloseHandle( h );
      h = INVALID_HANDLE_VALUE;
    }
  }
  // End of Change.  (Lockheed Martin) Secure Coding  11-14-2007
 
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
}

std::FILE *pws_os::FOpen(const stringT &filename, const TCHAR *mode)
{
  std::FILE *fd = NULL;
#if _MSC_VER >= 1400
  _tfopen_s(&fd, filename.c_str(), mode);
#else
  fd = _tfopen(m_filename.c_str(), mode);
#endif
  return fd;
}

long pws_os::fileLength(std::FILE *fp) {
  if (fp != NULL) {
    long pos = std::ftell(fp);
    std::fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    std::fseek(fp, pos, SEEK_SET);
    return len;
  } else
    return 0;
}
