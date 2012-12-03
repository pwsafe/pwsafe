/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of lib.h
 */
#include "../lib.h"
#include "../debug.h"
#include <windows.h>

void *pws_os::LoadLibrary(const TCHAR *lib, int type){
// Qualify full path name.  (Lockheed Martin) Secure Coding  11-14-2007
  TCHAR szFilePath[MAX_PATH+1];
  memset(szFilePath, 0, MAX_PATH+1);
  if (type == LOAD_LIBRARY_SYS) {
    if (!GetSystemDirectory(szFilePath, MAX_PATH)) {
       TRACE(_T("GetSystemDirectory failed when loading dynamic library\n"));
       return NULL;
    }
  }
  else if (type == LOAD_LIBRARY_APP) {
    if (!GetModuleFileName(NULL, szFilePath, MAX_PATH)) {
      TRACE(_T("GetModuleFileName failed when loading dynamic library\n"));
      return NULL;
    }
    else
       //set last slash to \0 for truncating app name
       *_tcsrchr(szFilePath, _T('\\')) = _T('\0');
  }
  //Add slash after directory path
  if (type != LOAD_LIBRARY_CUSTOM) {
    size_t nLen = _tcslen(szFilePath);
    if (nLen > 0) {
      if (szFilePath[nLen - 1] != '\\')
        _tcscat_s(szFilePath, MAX_PATH, _T("\\"));
    }
  }

  _tcscat_s(szFilePath, MAX_PATH, lib);
  TRACE(_T("Loading Library: %s\n"), szFilePath);
  return ::LoadLibrary(szFilePath);
}
