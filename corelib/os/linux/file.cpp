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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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
  char *oldfn = new char[N];
  wcstombs(oldfn, oldname.c_str(), oldN);
  size_t newN = wcstombs(NULL, newname.c_str(), 0) + 1;
  char *newfn = new char[N];
  wcstombs(newfn, newname.c_str(), newN);
  status = ::rename(oldfn, newfn);
  delete[] oldfn;
  delete[] newfn;
#endif /* UNICODE */
  return (status == 0);
}
