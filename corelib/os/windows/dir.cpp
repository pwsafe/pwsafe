/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

