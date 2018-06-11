/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include <sstream>

#include <dirent.h>
#include <fnmatch.h>
#ifndef __FreeBSD__
#include <malloc.h> // for free
#endif

#include "../file.h"
#include "../env.h"

#include "core/core.h"
#include "core/StringXStream.h"
#include "core/Util.h"

using namespace std;

const TCHAR pws_os::PathSeparator = _T('/');

bool pws_os::FileExists(const stringT &filename)
{
  struct stat statbuf;
  int status;
  size_t N = wcstombs(nullptr, filename.c_str(), 0) + 1;
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
  size_t N = wcstombs(nullptr, filename.c_str(), 0) + 1;
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
  size_t oldN = wcstombs(nullptr, oldname.c_str(), 0) + 1;
  char *oldfn = new char[oldN];
  wcstombs(oldfn, oldname.c_str(), oldN);
  size_t newN = wcstombs(nullptr, newname.c_str(), 0) + 1;
  char *newfn = new char[newN];
  wcstombs(newfn, newname.c_str(), newN);
  status = ::rename(oldfn, newfn);
  delete[] oldfn;
  delete[] newfn;
  return (status == 0);
}

bool pws_os::CopyAFile(const stringT &from, const stringT &to)
{
  const char *szfrom = nullptr;
  const char *szto = nullptr;
  bool retval = false;
  size_t fromsize = wcstombs(nullptr, from.c_str(), 0) + 1;
  szfrom = new char[fromsize];
  wcstombs(const_cast<char *>(szfrom), from.c_str(), fromsize);
  size_t tosize = wcstombs(nullptr, to.c_str(), 0) + 1;
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
      stop = cto.find_first_of('/', start);
      if (stop != stringT::npos)
        ::mkdir(cto.substr(start, stop).c_str(), 0700); // fail if already there - who cares?
      start = stop + 1;
    } while (stop != stringT::npos);

    ifstream src(szfrom, ios_base::in|ios_base::binary);
    ofstream dst(szto, ios_base::out|ios_base::binary);
    const size_t BUFSIZE = 2048;
    char buf[BUFSIZE];
    streamsize readBytes;

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
  size_t fnsize = wcstombs(nullptr, filename.c_str(), 0) + 1;
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
  size_t fltsize = wcstombs(nullptr, filter.c_str(), 0) + 1;
  assert(fltsize > 0);
  szfilter = new char[fltsize];
  wcstombs(const_cast<char *>(szfilter), filter.c_str(), fltsize);
  string cfilter(szfilter);
  delete[] szfilter;
  // start by splitting it up
  string dir;
  string::size_type last_slash = cfilter.find_last_of('/');
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
    size_t wname_len = ::mbstowcs(nullptr,
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
  /*
   * If the filename ends with .cfg, then we add .plk to it, e.g., foo.cfg.plk
   * otherwise we replace the suffix with .plk, e.g., foo.psafe3 -> foo.plk
   * This fixes a bug while maintaining bwd compat.
   */
  stringT retval;
  if (filename.length() > 4 && filename.substr(filename.length() - 4) == _T(".cfg"))
    retval = filename;
  else
    retval = filename.substr(0, filename.find_last_of(TCHAR('.')));
  retval += _T(".plk");
  return retval;
}

bool pws_os::LockFile(const stringT &filename, stringT &locker,
                      HANDLE &lockFileHandle)
{
  UNREFERENCED_PARAMETER(lockFileHandle);
  const stringT lock_filename = GetLockFileName(filename);
  bool retval = false;
  size_t lfs = wcstombs(nullptr, lock_filename.c_str(), lock_filename.length()) + 1;
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
        StringXStream lockerStream;
        if (PWSUtil::loadFile(lock_filename.c_str(), lockerStream)) {
          locker = stringx2std(lockerStream.str());
        }
        else {
          LoadAString(locker, IDSC_CANTREADLOCKER);
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
    ssize_t numWrit;
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

void pws_os::UnlockFile(const stringT &filename, HANDLE &lockFileHandle)
{
  UNREFERENCED_PARAMETER(lockFileHandle);
  stringT lock_filename = GetLockFileName(filename);
  size_t lfs = wcstombs(nullptr, lock_filename.c_str(), lock_filename.length()) + 1;
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
  const char *cfname = nullptr;
  const char *cmode = nullptr;
  size_t fnsize = wcstombs(nullptr, filename.c_str(), 0) + 1;
  assert(fnsize > 1);
  cfname = new char[fnsize];
  wcstombs(const_cast<char *>(cfname), filename.c_str(), fnsize);

  size_t modesize = wcstombs(nullptr, mode, 0) + 1;
  assert(modesize > 0);
  cmode = new char[modesize];
  wcstombs(const_cast<char *>(cmode), mode, modesize);
  FILE *retval = ::fopen(cfname, cmode);
  delete[] cfname;
  delete[] cmode;
  return retval;
}

int pws_os::FClose(std::FILE *fd, const bool &bIsWrite)
{
  if (fd != nullptr) {
    if (bIsWrite) {
      // Flush the data buffers
      fflush(fd);
    }
    // Now close file
    return fclose(fd);
  }
  return 0;
}

ulong64 pws_os::fileLength(std::FILE *fp)
{
  if (fp == nullptr)
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
  size_t N = wcstombs(nullptr, filename.c_str(), 0) + 1;
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

bool pws_os::ProgramExists(const stringT &filename)
{
  stringT pathEnvVar = pws_os::getenv("PATH", false);
  
  if (pathEnvVar.empty()) {
    return false;
  }

  std::wistringstream wstringstream(pathEnvVar);
  stringT path;

  while(std::getline(wstringstream, path, _T(':')))
  {
    if (path.empty()) {
      continue;
    }
    
    if (path.back() != pws_os::PathSeparator) {
      path.append(1, pws_os::PathSeparator);
      path.append(filename);
    }
    else {
      path.append(filename);
    }
    
    if (pws_os::FileExists(path)) {
      return true;
    }
  }
  
  return false;
}
