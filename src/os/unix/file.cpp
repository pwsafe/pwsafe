/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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

#include <dirent.h>
#include <fnmatch.h>
#ifndef __FreeBSD__
#include <malloc.h> // for free
#endif

#include "../file.h"
#include "../env.h"
#include "core.h"
#include "StringXStream.h"

using namespace std;

const TCHAR pws_os::PathSeparator = _T('/');

bool pws_os::FileExists(const stringT &filename)
{
  struct stat statbuf;
  int status;
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  status = ::stat(fn, &statbuf);
  delete[] fn;
  return (status == 0);
}

bool pws_os::FileExists(const stringT &filename, bool &bReadOnly)
{
  bool retval;
  bReadOnly = false;
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  retval = (::access(fn, R_OK) == 0);
  if (retval) {
    bReadOnly = (::access(fn, W_OK) != 0);
  }
  delete[] fn;
  return retval;
}

bool pws_os::RenameFile(const stringT &oldname, const stringT &newname)
{
  int status;
  size_t oldN = wcstombs(NULL, oldname.c_str(), 0) + 1;
  char *oldfn = new char[oldN];
  wcstombs(oldfn, oldname.c_str(), oldN);
  size_t newN = wcstombs(NULL, newname.c_str(), 0) + 1;
  char *newfn = new char[newN];
  wcstombs(newfn, newname.c_str(), newN);
  status = ::rename(oldfn, newfn);
  delete[] oldfn;
  delete[] newfn;
  return (status == 0);
}

bool pws_os::CopyAFile(const stringT &from, const stringT &to)
{
  const char *szfrom = NULL;
  const char *szto = NULL;
  bool retval = false;
  size_t fromsize = wcstombs(NULL, from.c_str(), 0) + 1;
  szfrom = new char[fromsize];
  wcstombs(const_cast<char *>(szfrom), from.c_str(), fromsize);
  size_t tosize = wcstombs(NULL, to.c_str(), 0) + 1;
  assert(tosize > 0);
  szto = new char[tosize];
  wcstombs(const_cast<char *>(szto), to.c_str(), tosize);
  // can we read the source?
  bool readable = ::access(szfrom, R_OK) == 0;
  if (!readable) {
    retval = false;
  } else { // creates dirs as needed

    string cto(szto);
    string::size_type start = (cto[0] == '/') ? 1 : 0;
    string::size_type stop;
    do {
      stop = cto.find_first_of("/", start);
      if (stop != stringT::npos)
        ::mkdir(cto.substr(start, stop).c_str(), 0700); // fail if already there - who cares?
      start = stop + 1;
    } while (stop != stringT::npos);

    ifstream src(szfrom, ios_base::in|ios_base::binary);
    ofstream dst(szto, ios_base::out|ios_base::binary);
    const size_t BUFSIZE = 2048;
    char buf[BUFSIZE];
    size_t readBytes;

    do {
      src.read(buf, BUFSIZE);
      readBytes = src.gcount();
      dst.write(buf, readBytes);
    } while(readBytes != 0);
    retval = true;
  }
  delete[] szfrom;
  delete[] szto;
  return retval;
}

bool pws_os::DeleteAFile(const stringT &filename)
{
  size_t fnsize = wcstombs(NULL, filename.c_str(), 0) + 1;
  assert(fnsize > 0);
  const char *szfn = new char[fnsize];
  wcstombs(const_cast<char *>(szfn), filename.c_str(), fnsize);

  bool retval = (::unlink(szfn) == 0);
  delete[] szfn;
  return retval;
}

static string filterString;

static int filterFunc(const struct dirent *de)
{
  return fnmatch(filterString.c_str(), de->d_name, 0) == 0;
}

void pws_os::FindFiles(const stringT &filter, vector<stringT> &res)
{
  if (filter.empty())
    return;
  // filter is a full path with a filter file name.
  const char *szfilter;
  size_t fltsize = wcstombs(NULL, filter.c_str(), 0) + 1;
  assert(fltsize > 0);
  szfilter = new char[fltsize];
  wcstombs(const_cast<char *>(szfilter), filter.c_str(), fltsize);
  string cfilter(szfilter);
  delete[] szfilter;
  // start by splitting it up
  string dir;
  string::size_type last_slash = cfilter.find_last_of("/");
  if (last_slash != string::npos) {
    dir = cfilter.substr(0, last_slash);
    filterString = cfilter.substr(last_slash + 1);
  } else {
    dir = ".";
    filterString = cfilter;
  }
  res.clear();
  struct dirent **namelist;
  int nMatches = scandir(dir.c_str(), &namelist,
                         filterFunc, alphasort);
  if (nMatches <= 0)
    return;
  while (nMatches-- != 0) {
    size_t wname_len = ::mbstowcs(NULL,
                                  namelist[nMatches]->d_name,
                                  0) + 1;
    wchar_t *wname = new wchar_t[wname_len];
    mbstowcs(wname, namelist[nMatches]->d_name, wname_len);
    res.push_back(wname);
    delete[] wname;
    free(namelist[nMatches]);
  }
  free(namelist);
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
  UNREFERENCED_PARAMETER(lockFileHandle);
  UNREFERENCED_PARAMETER(LockCount);
  const stringT lock_filename = GetLockFileName(filename);
  stringT s_locker;
  bool retval = false;
  size_t lfs = wcstombs(NULL, lock_filename.c_str(), lock_filename.length()) + 1;
  char *lfn = new char[lfs];
  wcstombs(lfn, lock_filename.c_str(), lfs);
  int fh = open(lfn, (O_CREAT | O_EXCL | O_WRONLY),
                 (S_IREAD | S_IWRITE));

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
          wifstream is(lfn);
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
      LoadAString(locker, IDSC_UNKNOWN_ERROR);
      break;
    } // switch (errno)
    retval = false;
  } else { // valid filehandle, write our info
    int numWrit;
    const stringT user = pws_os::getusername();
    const stringT host = pws_os::gethostname();
    const stringT pid = pws_os::getprocessid();

    const stringT lockStr = user + _T("@") + host + _T(":") + pid;

    numWrit = write(fh, lockStr.c_str(),
                    lockStr.length() * sizeof(TCHAR));
    ASSERT(numWrit > 0);
    close(fh);
    retval = (numWrit > 0);
  }
  delete[] lfn;
  return retval;
}

void pws_os::UnlockFile(const stringT &filename,
                        HANDLE &lockFileHandle, int &LockCount)
{
  UNREFERENCED_PARAMETER(lockFileHandle);
  UNREFERENCED_PARAMETER(LockCount);
  stringT lock_filename = GetLockFileName(filename);
  size_t lfs = wcstombs(NULL, lock_filename.c_str(), lock_filename.length()) + 1;
  char *lfn = new char[lfs];
  wcstombs(lfn, lock_filename.c_str(), lfs);
  unlink(lfn);
  delete[] lfn;
}

bool pws_os::IsLockedFile(const stringT &filename)
{
  const stringT lock_filename = GetLockFileName(filename);
  return pws_os::FileExists(lock_filename);
}

std::FILE *pws_os::FOpen(const stringT &filename, const TCHAR *mode)
{
  const char *cfname = NULL;
  const char *cmode = NULL;
  size_t fnsize = wcstombs(NULL, filename.c_str(), 0) + 1;
  assert(fnsize > 1);
  cfname = new char[fnsize];
  wcstombs(const_cast<char *>(cfname), filename.c_str(), fnsize);

  size_t modesize = wcstombs(NULL, mode, 0) + 1;
  assert(modesize > 0);
  cmode = new char[modesize];
  wcstombs(const_cast<char *>(cmode), mode, modesize);
  FILE *retval = ::fopen(cfname, cmode);
  delete[] cfname;
  delete[] cmode;
  return retval;
}

ulong64 pws_os::fileLength(std::FILE *fp)
{
  if (fp == NULL)
    return -1;
  int fd = fileno(fp);
  if (fd == -1)
    return -1;
  struct stat st;
  if (fstat(fd, &st) == -1)
    return -1;
  return ulong64(st.st_size);
}

bool pws_os::GetFileTimes(const stringT &filename,
			time_t &ctime, time_t &mtime, time_t &atime)
{
  struct stat info;
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  int status = ::stat(fn, &info);
  delete[] fn;
  if (status == 0) {
    ctime = info.st_ctime;
    mtime = info.st_mtime;
    atime = info.st_atime;
    return true;
  } else {
    return false;
  }
}

bool pws_os::SetFileTimes(const stringT &filename,
      time_t ctime, time_t mtime, time_t atime)
{
  UNREFERENCED_PARAMETER(filename);
  UNREFERENCED_PARAMETER(ctime);
  UNREFERENCED_PARAMETER(mtime);
  UNREFERENCED_PARAMETER(atime);
  
  return true;
}
