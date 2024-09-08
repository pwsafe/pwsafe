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

#include <CoreFoundation/CoreFoundation.h>

#include "../file.h"
#include "../env.h"

#include "../../core/core.h"
#include "../../core/StringXStream.h"
#include "../../core/PwsPlatform.h"


#include "../../core/pugixml/pugixml.hpp"

#if defined(PWS_LITTLE_ENDIAN)
#define wcharEncoding kCFStringEncodingUTF32LE
#else
#define wcharEncoding kCFStringEncodingUTF32BE
#endif

using namespace std;

const TCHAR pws_os::PathSeparator = _T('/');

// To add non-Unicode support, createFileSystemRepresentation() needs to be extended,
// and the conversion of the mode parameter of FOpen().
#ifndef UNICODE
#error UNICODE must be defined
#endif

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

bool pws_os::FileExists(const stringT &filename)
{
  struct stat statbuf;
  int status;
  char *fn = createFileSystemRepresentation(filename);
  status = ::stat(fn, &statbuf);
  delete[] fn;
  return (status == 0);
}

bool pws_os::FileExists(const stringT &filename, bool &bReadOnly)
{
  bool retval;
  bReadOnly = false;
  char *fn = createFileSystemRepresentation(filename);
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
  char *oldfn = createFileSystemRepresentation(oldname);
  char *newfn = createFileSystemRepresentation(newname);
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
  szfrom = createFileSystemRepresentation(from);
  szto = createFileSystemRepresentation(to);
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
  const char *szfn = createFileSystemRepresentation(filename);

  bool retval = (::unlink(szfn) == 0);
  delete[] szfn;
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
  szfilter = createFileSystemRepresentation(filter);
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
  char *lfn = createFileSystemRepresentation(lock_filename);
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
    delete[] lfn;
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
    delete[] lfn;
    return true;
  }
}

void pws_os::UnlockFile(const stringT &filename, HANDLE &)
{
  stringT lock_filename = GetLockFileName(filename);
  char *lfn = createFileSystemRepresentation(lock_filename);
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
  if (filename.empty()) { // set to stdin/stdout, depending on mode[0] (r/w/a)
	  return mode[0] == L'r' ? stdin : stdout;
  }
  
  const char *cfname = NULL;
  const char *cmode = NULL;
  cfname = createFileSystemRepresentation(filename);

  size_t modesize = wcstombs(NULL, mode, 0) + 1;
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
  char *fn = createFileSystemRepresentation(filename);
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
