/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of dir.h
 */
#include <afx.h>
#include <Windows.h>
#include "../dir.h"
#include <direct.h>
stringT pws_os::getexecdir()
{
  // returns the directory part of ::GetModuleFileName()
  TCHAR acPath[MAX_PATH + 1];

  if (GetModuleFileName( NULL, acPath, MAX_PATH + 1) != 0) {
    // guaranteed file name of at least one character after path '\'
    *(_tcsrchr(acPath, _T('\\')) + 1) = _T('\0');
  } else {
    acPath[0] = TCHAR('\\'); acPath[1] = TCHAR('\0');
  }
  return stringT(acPath);
}

stringT pws_os::getcwd()
{
  charT *curdir = _tgetcwd(NULL, 512); // NULL means 512 doesn't matter
  stringT CurDir(curdir);
  free(curdir);
  return CurDir;
}

bool pws_os::chdir(const stringT &dir)
{
  ASSERT(!dir.empty());
  return (_tchdir(dir.c_str()) == 0);
}

  // In following, drive will be empty on non-Windows platforms
bool pws_os::splitpath(const stringT &path,
                       stringT &drive, stringT &dir,
                       stringT &file, stringT &ext)
{
  TCHAR tdrv[_MAX_DRIVE];
  TCHAR tdir[_MAX_DIR];
  TCHAR tname[_MAX_FNAME];
  TCHAR text[_MAX_EXT];

  memset(tdrv, 0, sizeof(tdrv));
  memset(tdir, 0, sizeof(tdir));
  memset(tname, 0, sizeof(tname));
  memset(text, 0, sizeof(text));

  if (_tsplitpath_s(path.c_str(), tdrv, tdir, tname, text) == 0) {
    drive = tdrv;
    dir = tdir;
    file = tname;
    ext = text;
    return true;
  } else
    return false;
}

stringT pws_os::makepath(const stringT &drive, const stringT &dir,
                         const stringT &file, const stringT &ext)
{
  stringT retval;
  TCHAR path[_MAX_PATH];

  memset(path, 0, sizeof(path));
  if (_tmakepath_s(path, drive.c_str(), dir.c_str(),
                   file.c_str(), ext.c_str()) == 0)
    retval = path;
  return retval;
}

stringT pws_os::getuserprefsdir(void)
{
  // Return an empty string to
  // have Windows punt to exec dir, which is the historical behaviour
  // May want to change this in future to
  // SHGetFolderPath(CSIDL_APPDATA) + "/pwsafe".
  return stringT();
}



