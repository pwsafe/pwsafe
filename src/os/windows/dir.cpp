/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of dir.h
 */

#ifndef __WX__
#include <afx.h>
#else
#include <Windows.h>
#endif

#include "../dir.h"
#include <direct.h>

#include <shlwapi.h>
#include <Shobjidl.h>
#include <shlobj.h>

stringT pws_os::getexecdir()
{
  // returns the directory part of ::GetModuleFileName()
  TCHAR acPath[MAX_PATH + 1];

  if (GetModuleFileName(NULL, acPath, MAX_PATH + 1) != 0) {
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

  wmemset(tdrv, 0, sizeof(tdrv) / sizeof(TCHAR));
  wmemset(tdir, 0, sizeof(tdir) / sizeof(TCHAR));
  wmemset(tname, 0, sizeof(tname) / sizeof(TCHAR));
  wmemset(text, 0, sizeof(text) / sizeof(TCHAR));

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

  wmemset(path, 0, sizeof(path)/sizeof(TCHAR));
  if (_tmakepath_s(path, drive.c_str(), dir.c_str(),
                   file.c_str(), ext.c_str()) == 0)
    retval = path;
  return retval;
}

stringT pws_os::fullpath(const stringT &relpath)
{
  stringT retval;
  TCHAR full[_MAX_PATH];

  wmemset(full, 0, sizeof(full)/sizeof(TCHAR));
  if (_tfullpath(full, relpath.c_str(), _MAX_PATH))
    retval = full;
  return retval;
}

static bool GetLocalDir(int nFolder, stringT &sLocalPath)
{
  // String buffer for holding the path.
  TCHAR strPath[MAX_PATH];

  // Get the special folder path - do not create it if it does not exist
  BOOL brc = SHGetSpecialFolderPath(NULL, strPath, nFolder, FALSE);

  if (brc == TRUE) {
    // Call to 'SHGetSpecialFolderPath' worked
    sLocalPath = strPath;
  } else {
    // Unsupported release or 'SHGetSpecialFolderPath' failed
    sLocalPath = _T("");
  }
  return (brc == TRUE);
}

stringT pws_os::getuserprefsdir()
{
  /**
   * Returns LOCAL_APPDATA\PasswordSafe (or ...\PasswordSafeD)
   * (Creating if necessary)
   * If can't figure out LOCAL_APPDATA, then return an empty string
   * to have Windows punt to exec dir, which is the historical behaviour
   */
#ifndef _DEBUG
  const stringT sPWSDir(_T("\\PasswordSafe\\"));
#else
  const stringT sPWSDir(_T("\\PasswordSafeD\\"));
#endif

  stringT sDrive, sDir, sName, sExt, retval;

  pws_os::splitpath(getexecdir(), sDrive, sDir, sName, sExt);
  sDrive += _T("\\"); // Trailing slash required.

  const UINT uiDT = ::GetDriveType(sDrive.c_str());
  if (uiDT == DRIVE_FIXED || uiDT == DRIVE_REMOTE) {
    stringT sLocalAppDataPath;
    if (GetLocalDir(CSIDL_LOCAL_APPDATA, sLocalAppDataPath))
      retval = sLocalAppDataPath + sPWSDir;
    if (PathFileExists(retval.c_str()) == FALSE)
      if (_tmkdir(retval.c_str()) != 0)
        retval = _T(""); // couldn't create dir!?
  } else if (uiDT == DRIVE_REMOVABLE) {
    stringT::size_type index = sDir.rfind(_T("Program\\"));
    if (index != stringT::npos)
      retval = getexecdir().substr(0, getexecdir().length() - 8);
  }
  return retval;
}

stringT pws_os::getsafedir(void)
{
  stringT sDrive, sDir, sName, sExt, retval;

  pws_os::splitpath(getexecdir(), sDrive, sDir, sName, sExt);
  const stringT sDriveT = sDrive + _T("\\"); // Trailing slash required.

  const UINT uiDT = ::GetDriveType(sDriveT.c_str());
  if (uiDT == DRIVE_REMOVABLE) { 
    stringT::size_type index = sDir.rfind(_T("Program\\"));
    if (index != stringT::npos) {
      sDir.replace(index, 8, stringT(_T("Safes\\")));
      retval = sDrive + sDir;
      if (PathFileExists(retval.c_str()) == TRUE)
        return retval;
    }
  }
  stringT sLocalSafePath;
  if (GetLocalDir(CSIDL_PERSONAL, sLocalSafePath)) {
    retval = sLocalSafePath + _T("\\My Safes");
    if (PathFileExists(retval.c_str()) == FALSE)
      if (_tmkdir(retval.c_str()) != 0)
        retval = _T(""); // couldn't create dir!?
  }
  return retval;
}

stringT pws_os::getxmldir(void)
{
  stringT sDrive, sDir, sName, sExt;

  pws_os::splitpath(getexecdir(), sDrive, sDir, sName, sExt);
  const stringT sDriveT = sDrive + _T("\\"); // Trailing slash required.

  const UINT uiDT = ::GetDriveType(sDriveT.c_str());
  if (uiDT == DRIVE_REMOVABLE) { 
    stringT::size_type index = sDir.rfind(_T("Program\\"));
    if (index != stringT::npos) {
      sDir.replace(index, 8, stringT(_T("xml\\")));
      stringT retval = sDrive + sDir;
      if (PathFileExists(retval.c_str()) == TRUE)
        return retval;
    }
  }
  return getexecdir();
}

stringT pws_os::gethelpdir(void)
{
  return getexecdir();
}
  
