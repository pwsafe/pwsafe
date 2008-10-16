/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of file.h
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <cassert>
#include <fstream>
#include "../file.h"
#include "../env.h"
#include "../../corelib.h"
#include "../../StringXStream.h"

using namespace std;

const TCHAR *pws_os::PathSeparator = _T("/");

bool pws_os::FileExists(const stringT &filename)
{
  struct stat statbuf;
  int status;
#ifndef UNICODE
  status = ::stat(filename.c_str(), &statbuf);
#else
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  status = ::stat(fn, &statbuf);
  delete[] fn;
#endif /* UNICODE */
  return (status == 0);
}

bool pws_os::FileExists(const stringT &filename, bool &bReadOnly)
{
  int status;
  bool retval = false;
  bReadOnly = false;
#ifndef UNICODE
  status = ::access(filename.c_str(), R_OK);
  if (status == 0) {
    bReadOnly = (::access(filename.c_str(), W_OK) != 0);
    retval = true;
  }
#else
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  status = ::access(fn, R_OK);
  if (status == 0) {
    bReadOnly = (::access(fn, W_OK) != 0);
    retval = true;
  }
  delete[] fn;
#endif /* UNICODE */
  return retval;
}

bool pws_os::RenameFile(const stringT &oldname, const stringT &newname)
{
  int status;
#ifndef UNICODE
  status = ::rename(oldname.c_str(), newname.c_str());
#else
  size_t oldN = wcstombs(NULL, oldname.c_str(), 0) + 1;
  char *oldfn = new char[oldN];
  wcstombs(oldfn, oldname.c_str(), oldN);
  size_t newN = wcstombs(NULL, newname.c_str(), 0) + 1;
  char *newfn = new char[newN];
  wcstombs(newfn, newname.c_str(), newN);
  status = ::rename(oldfn, newfn);
  delete[] oldfn;
  delete[] newfn;
#endif /* UNICODE */
  return (status == 0);
}

bool pws_os::CopyAFile(const stringT &from, const stringT &to)
{
  // can we read the source?
  if (::access(from.c_str(), R_OK) != 0)
    return false;
  // creates dirs as needed
  stringT::size_type start = (to[0] == '/') ? 1 : 0;
  stringT::size_type stop;
  do {
    stop = to.find_first_of("/", start);
    if (stop != stringT::npos)
      ::mkdir(to.substr(start, stop).c_str(), 0700); // fail if already there - who cares?
    start = stop + 1;
  } while (stop != stringT::npos);
  ifstream src(from.c_str(), ios_base::in|ios_base::binary);
  ofstream dst(to.c_str(), ios_base::out|ios_base::binary);
  const size_t BUFSIZE = 2048;
  char buf[BUFSIZE];
  size_t readBytes;

  do {
    src.read(buf, BUFSIZE);
    readBytes = src.gcount();
    dst.write(buf, readBytes);
  } while(readBytes != 0);
  return true;
}

bool pws_os::DeleteAFile(const stringT &filename)
{
  return ::unlink(filename.c_str());
}


static stringT GetLockFileName(const stringT &filename)
{
  assert(!filename.empty());
  // derive lock filename from filename
  stringT retval(filename, 0, filename.find_last_of(TCHAR('.')));
  retval += _T(".plk");
  return retval;
}

bool pws_os::LockFile(const stringT &filename, stringT &locker, 
                      HANDLE &lockFileHandle, int &LockCount)
{
  const stringT lock_filename = GetLockFileName(filename);
  stringT s_locker;
#ifndef UNICODE
  const char *lfn = lock_filename.c_str();
#else
  size_t lfs = wcstombs(NULL, lock_filename.c_str(), lock_filename.length()) + 1;
  char *lfn = new char[lfs];
  wcstombs(lfn, lock_filename.c_str(), lock_filename.length());
#endif
  int fh = open(lfn, (O_CREAT | O_EXCL | O_WRONLY),
                 (S_IREAD | S_IWRITE));
#ifdef UNICODE
  delete[] lfn;
#endif

  if (fh == -1) { // failed to open exclusively. Already locked, or ???
    switch (errno) {
    case EACCES:
      // Tried to open read-only file for writing, or file's
      // sharing mode does not allow specified operations, or given path is directory
        LoadAString(locker, IDSC_NOLOCKACCESS);
      break;
    case EEXIST: // filename already exists
      {
        // read locker data ("user@machine:nnnnnnnn") from file
          istringstreamT is(lock_filename);
          stringT lockerStr;
          if (is >> lockerStr) {
            locker = lockerStr;
          }
      } // EEXIST block
        break;
    case EINVAL: // Invalid oflag or pmode argument
      LoadAString(locker, IDSC_INTERNALLOCKERROR);
      break;
    case EMFILE: // No more file handles available (too many open files)
      LoadAString(locker, IDSC_SYSTEMLOCKERROR);
      break;
    case ENOENT: //File or path not found
      LoadAString(locker, IDSC_LOCKFILEPATHNF);
      break;
    default:
      LoadAString(locker, IDSC_LOCKUNKNOWNERROR);
      break;
    } // switch (errno)
    return false;
  } else { // valid filehandle, write our info
    int numWrit;
    const stringT user = pws_os::getusername();
    const stringT host = pws_os::gethostname();
    const stringT pid = pws_os::getprocessid();

    numWrit = write(fh, user.c_str(), user.length() * sizeof(TCHAR));
    numWrit += write(fh, _T("@"), sizeof(TCHAR));
    numWrit += write(fh, host.c_str(), host.length() * sizeof(TCHAR));
    numWrit += write(fh, _T(":"), sizeof(TCHAR));
    numWrit += write(fh, pid.c_str(), pid.length() * sizeof(TCHAR));
    ASSERT(numWrit > 0);
    close(fh);
    return true;
  }
}

void pws_os::UnlockFile(const stringT &filename,
                        HANDLE &lockFileHandle, int &LockCount)
{
  stringT lock_filename = GetLockFileName(filename);
#ifndef UNICODE
  const char *lfn = lock_filename.c_str();
#else
  size_t lfs = wcstombs(NULL, lock_filename.c_str(), lock_filename.length()) + 1;
  char *lfn = new char[lfs];
  wcstombs(lfn, lock_filename.c_str(), lock_filename.length());
#endif
  unlink(lfn);
#ifdef UNICODE
  delete[] lfn;
#endif
}

bool pws_os::IsLockedFile(const stringT &filename)
{
  const stringT lock_filename = GetLockFileName(filename);
  return pws_os::FileExists(lock_filename);
}

std::FILE *pws_os::FOpen(const stringT &filename, const TCHAR *mode)
{
  return ::fopen(filename.c_str(), mode);
}

long pws_os::fileLength(std::FILE *fp)
{
  int fd = fileno(fp);
  if (fd == -1)
    return -1;
  struct stat st;
  if (fstat(fd, &st) == -1)
    return -1;
  return st.st_size;
}

