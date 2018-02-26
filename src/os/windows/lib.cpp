/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

void *pws_os::LoadLibrary(const TCHAR *lib, loadLibraryTypes type)
{
  ASSERT(lib != NULL);
  
  // Qualify full path name.  (Lockheed Martin) Secure Coding  11-14-2007
  TCHAR szFilePath[MAX_PATH+1];
  memset(szFilePath, 0, MAX_PATH+1);
  if (type == loadLibraryTypes::SYS) {
    if (!GetSystemDirectory(szFilePath, MAX_PATH)) {
       pws_os::Trace(_T("GetSystemDirectory failed when loading dynamic library\n"));
       return NULL;
    }
  }
  else if (type == loadLibraryTypes::APP || type == loadLibraryTypes::RESOURCE) {
    if (!GetModuleFileName(NULL, szFilePath, MAX_PATH)) {
      pws_os::Trace(_T("GetModuleFileName failed when loading dynamic library\n"));
      return NULL;
    }
    else
       //set last slash to \0 for truncating app name
       *_tcsrchr(szFilePath, _T('\\')) = _T('\0');
  }
  //Add slash after directory path
  if (type != loadLibraryTypes::CUSTOM) {
    size_t nLen = _tcslen(szFilePath);
    if (nLen > 0) {
      if (szFilePath[nLen - 1] != '\\')
        _tcscat_s(szFilePath, MAX_PATH, _T("\\"));
    }
  }

  _tcscat_s(szFilePath, MAX_PATH, lib);
  pws_os::Trace(_T("Loading Library: %s\n"), szFilePath);
  HMODULE hMod = NULL;
  switch ((loadLibraryTypes)type) {
    // We load resource files (e.g language translation resource files) as
    // data files to avoid any problem with 32/64 bit DLLs. This allows
    // the use of 32-bit language DLLs in 64-bit builds and vice versa.
    case loadLibraryTypes::RESOURCE:
      hMod = ::LoadLibraryEx(szFilePath, NULL, LOAD_LIBRARY_AS_DATAFILE);
	    break;
    // All other DLLs are loaded for execution
    default:
	    hMod = ::LoadLibrary(szFilePath);
	    break;
  }
  return hMod;
  // End of change.  (Lockheed Martin) Secure Coding  11-14-2007
}

bool pws_os::FreeLibrary(void *handle)
{
  if (handle != NULL)
    return ::FreeLibrary(HMODULE(handle)) == TRUE;
  else
    return false;
}

void *pws_os::GetFunction(void *handle, const char *name)
{
  ASSERT(handle != NULL && name != NULL);
  return ::GetProcAddress(HMODULE(handle), name);
}
