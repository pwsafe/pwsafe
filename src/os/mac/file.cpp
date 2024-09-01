/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of file.h
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

#ifdef UNICODE
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "../file.h"
#include "../env.h"

#include "../../core/core.h"
#include "../../core/StringXStream.h"
#include "../../core/PwsPlatform.h"


#include "../../core/pugixml/pugixml.hpp"

#ifdef UNICODE
#if defined(PWS_LITTLE_ENDIAN)
#define wcharEncoding kCFStringEncodingUTF32LE
#else
#define wcharEncoding kCFStringEncodingUTF32BE
#endif
#endif

using namespace std;

const TCHAR pws_os::PathSeparator = _T('/');

#ifdef UNICODE
// This function returns a pointer to an allocated char[],
// the caller must delete the array.
static char *createFileSystemRepresentation(const stringT &filename)
{
  // Convert to UTF-16
  CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const unsigned char *>(filename.c_str()), filename.length()*sizeof(wchar_t), wcharEncoding, false);
  assert(str != NULL);

  CFIndex maxBufLen = CFStringGetMaximumSizeOfFileSystemRepresentation(str);
  char *buffer = new char[maxBufLen];

  bool res = CFStringGetFileSystemRepresentation(str, buffer, maxBufLen);
  assert(res);

  CFRelease(str);
  return buffer;
}
#endif

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
  bool retval;
  bReadOnly = false;
#ifndef UNICODE
  retval = (::access(filename.c_str(), R_OK) == 0);
  if (retval) {
    bReadOnly = (::access(filename.c_str(), W_OK) != 0);
  }
#else
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  retval = (::access(fn, R_OK) == 0);
  if (retval) {
    bReadOnly = (::access(fn, W_OK) != 0);
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
  const char *szfrom = NULL;
  const char *szto = NULL;
  bool retval = false;
#ifndef UNICODE
  szfrom = from.c_str();
  szto = to.c_str();
#else
  size_t fromsize = wcstombs(NULL, from.c_str(), 0) + 1;
  szfrom = new char[fromsize];
  wcstombs(const_cast<char *>(szfrom), from.c_str(), fromsize);
  size_t tosize = wcstombs(NULL, to.c_str(), 0) + 1;
  assert(tosize > 0);
  szto = new char[tosize];
  wcstombs(const_cast<char *>(szto), to.c_str(), tosize);
#endif /* UNICODE */
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
#ifdef UNICODE
  delete[] szfrom;
  delete[] szto;
#endif
  return retval;
}

bool pws_os::DeleteAFile(const stringT &filename)
{
#ifndef UNICODE
  const char *szfn =  filename.c_str();
#else
  size_t fnsize = wcstombs(NULL, filename.c_str(), 0) + 1;
  assert(fnsize > 0);
  const char *szfn = new char[fnsize];
  wcstombs(const_cast<char *>(szfn), filename.c_str(), fnsize);
#endif /* UNICODE */

  bool retval = (::unlink(szfn) == 0);
#ifdef UNICODE
  delete[] szfn;
#endif
  return retval;
}

static string filterString;

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(__amd64__)
#define build_is_64_bit
#endif

// I don't even know the platform/sdk where scandir() requires this new signature of filterfunc, so I'm trying to
// deduce if we are building 64-bit with Xcode 5.  May be this is more of a SDK-dependent thing,
// but I'm not sure right now.  Also note that this has nothing to do with wxWidgets
#if defined(__PWS_MACINTOSH__) && defined(__clang__) && (__clang_major__ >= 5)
static int filterFunc(const struct dirent *de)
#else
static int filterFunc(struct dirent *de)
#endif
{
  return fnmatch(filterString.c_str(), de->d_name, 0) == 0;
}

void pws_os::FindFiles(const stringT &filter, vector<stringT> &res)
{
  if (filter.empty())
    return;
  // filter is a full path with a filter file name.
  const char *szfilter;
#ifdef UNICODE
  size_t fltsize = wcstombs(NULL, filter.c_str(), 0) + 1;
  assert(fltsize > 0);
  szfilter = new char[fltsize];
  wcstombs(const_cast<char *>(szfilter), filter.c_str(), fltsize);
#else
  szfilter = filter.c_str();
#endif /* UNICODE */
  string cfilter(szfilter);
#ifdef UNICODE
  delete[] szfilter;
#endif
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
#ifndef UNICODE
    res.push_back(namelist[nMatches]->d_name);
#else
    size_t wname_len = ::mbstowcs(NULL,
                                  namelist[nMatches]->d_name,
                                  0) + 1;
    wchar_t *wname = new wchar_t[wname_len];
    mbstowcs(wname, namelist[nMatches]->d_name, wname_len);
    res.push_back(wname);
    delete[] wname;
#endif
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

bool pws_os::LockFile(const stringT &filename, stringT &locker, HANDLE &)
{
  const stringT lock_filename = GetLockFileName(filename);
  stringT s_locker;
#ifndef UNICODE
  const char *lfn = lock_filename.c_str();
#else
  size_t lfs = wcstombs(NULL, lock_filename.c_str(), lock_filename.length()) + 1;
  char *lfn = new char[lfs];
  wcstombs(lfn, lock_filename.c_str(), lfs);
#endif
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
        locker = _T("Unable to determine locker");
        // read locker data ("user@machine:nnnnnnnn") from file
        fh = open(lfn, (O_RDONLY));
        if(fh != -1) {
          struct stat sbuf;
          if((fstat(fh, &sbuf) != -1) && sbuf.st_size) {
            char *lb = new char [sbuf.st_size + sizeof(TCHAR)];
            ssize_t num;
            ASSERT(lb);
            num = read(fh, lb, sbuf.st_size);
            if(num == sbuf.st_size) {
              char *lp;
              for(lp = &lb[num-1]; lp >= lb && *lp != ':'; --lp) ; // Search for ':'
              if(lp >= lb) {
                unsigned long offset = &lb[num] - lp;
                pugi::xml_encoding encoding = pugi::encoding_auto;
                if(offset == 9) { // ':' '1' '2' '3' '4' '5' '6' '7' '8' ('\0')
                  // UTF-8 coding
                  encoding = pugi::encoding_utf8;
                }
                else if(offset == 18) { // ':' '\0' '1' '\0' '2' '\0' '3' '\0' '4' '\0' '5' '\0' '6' '\0' '7' '\0' '8' '\0' ('\0' '\0')
                  // UTF-16 coding little endian
                  encoding = pugi::encoding_utf16_le;
                }
                else if(offset == 17) { // '\0' ':' '\0' '1' '\0' '2' '\0' '3' '\0' '4' '\0' '5' '\0' '6' '\0' '7' '\0' '8' ('\0' '\0')
                  // UTF-16 coding big endian);
                  encoding = pugi::encoding_utf16_be;
                }
                else if(offset == 36) { // ':' '\0' '\0' '\0' '1' '\0' '\0' '\0' '2' '\0' '\0' '\0' '3' '\0' '\0' '\0' '4' '\0' '\0' '\0' '5' '\0' '\0' '\0' '6' '\0' '\0' '\0' '7' '\0' '\0' '\0' '8' '\0' '\0' '\0' ('\0' '\0' '\0' '\0')
                  // UTF-32 coding little endian
                  encoding = pugi::encoding_utf32_le;
                }
                else if(offset == 34) { // '\0' '\0' '\0' ':' '\0' '\0' '\0' '1' '\0' '\0' '\0' '2' '\0' '\0' '\0' '3' '\0' '\0' '\0' '4' '\0' '\0' '\0' '5' '\0' '\0' '\0' '6' '\0' '\0' '\0' '7' '\0' '\0' '\0' '8' ('\0' '\0' '\0' '\0')
                  // UTF-32 coding big endian
                  encoding = pugi::encoding_utf32_be;
                }
                if(encoding != pugi::encoding_auto) {
                  // get private buffer
                  wchar_t* buffer = 0;
                  size_t length = 0;
                  // Convert from UTF-8, UTF-16 or UTF-32 to machine wchar_t
                  if(pugi::convertBuffer(buffer, length, encoding, lb, num, true)) {
                    ASSERT(buffer);
                    locker = _T("");
                    locker.append(buffer, length);
                    if(static_cast<void *>(buffer) != static_cast<void *>(lb))
                      (*pugi::get_memory_deallocation_function())(buffer);
                  }
                }
              }
            }
            delete [] lb;
          }
          close(fh);
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
#ifdef UNICODE
    delete[] lfn;
#endif
    return false;
  } else { // valid filehandle, write our info

// Since ASSERT is a no-op in a release build, numWrit
// becomes an unused variable
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    ssize_t numWrit;
    const stringT user = pws_os::getusername();
    const stringT host = pws_os::gethostname();
    const stringT pid = pws_os::getprocessid();

    numWrit = write(fh, user.c_str(), user.length() * sizeof(TCHAR));
    numWrit += write(fh, _T("@"), sizeof(TCHAR));
    numWrit += write(fh, host.c_str(), host.length() * sizeof(TCHAR));
    numWrit += write(fh, _T(":"), sizeof(TCHAR));
    numWrit += write(fh, pid.c_str(), pid.length() * sizeof(TCHAR));
    ASSERT(numWrit > 0);
#pragma GCC diagnostic pop

    close(fh);
#ifdef UNICODE
    delete[] lfn;
#endif
    return true;
  }
}

void pws_os::UnlockFile(const stringT &filename, HANDLE &)
{
  stringT lock_filename = GetLockFileName(filename);
#ifndef UNICODE
  const char *lfn = lock_filename.c_str();
#else
  size_t lfs = wcstombs(NULL, lock_filename.c_str(), lock_filename.length()) + 1;
  char *lfn = new char[lfs];
  wcstombs(lfn, lock_filename.c_str(), lfs);
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
  if (filename.empty()) { // set to stdin/stdout, depending on mode[0] (r/w/a)
	  return mode[0] == L'r' ? stdin : stdout;
  }
  
  const char *cfname = NULL;
  const char *cmode = NULL;
#ifdef UNICODE
  cfname = createFileSystemRepresentation(filename);

  size_t modesize = wcstombs(NULL, mode, 0) + 1;
  assert(modesize > 0);
  cmode = new char[modesize];
  wcstombs(const_cast<char *>(cmode), mode, modesize);
#else
  cfname = filename.c_str();
  cmode = mode;
#endif /* UNICODE */
  FILE *retval = ::fopen(cfname, cmode);
#ifdef UNICODE
  delete[] cfname;
  delete[] cmode;
#endif
  return retval;
}

int pws_os::FClose(std::FILE *fd, const bool &bIsWrite)
{
  if (fd != NULL) {
    if (bIsWrite) {
      // Flush the data buffers
      fflush(fd);
    }
    // Now close file
    return fclose(fd);
  }
  return 0;
}

size_t pws_os::fileLength(std::FILE *fp)
{
  int fd = fileno(fp);
  if (fd == -1)
    return -1;
  struct stat st;
  if (fstat(fd, &st) == -1)
    return -1;
  return size_t(st.st_size);
}

bool pws_os::GetFileTimes(const stringT &filename,
			time_t &ctime, time_t &mtime, time_t &atime)
{
  struct stat statbuf;
  int status;
  size_t N = wcstombs(NULL, filename.c_str(), 0) + 1;
  char *fn = new char[N];
  wcstombs(fn, filename.c_str(), N);
  status = ::stat(fn, &statbuf);
  delete[] fn;
  if (status == 0) {
    ctime = statbuf.st_ctime;
    mtime = statbuf.st_mtime;
    atime = statbuf.st_atime;
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
