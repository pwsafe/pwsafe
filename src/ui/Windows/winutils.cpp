/*
 * Copyright (c) 2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file winutils.cpp
*
* Contains generic utility functions that should be global and don't fit anywhere else
*/

#include "winutils.h"
#include "core/StringX.h"
#include "core/SysInfo.h"
#include "os/dir.h"

void WinUtil::RelativizePath(std::wstring &curfile)
{
  // If  IsUnderPw2go() && exec's drive == curfile's drive, remove
  // from latter's path. This supports DoK usage
  if (SysInfo::IsUnderPw2go()) {
    const std::wstring execDir = pws_os::getexecdir();
    std::wstring execDrive, dontCare;
    pws_os::splitpath(execDir, execDrive, dontCare, dontCare, dontCare);
    std::wstring fileDrive, fileDir, fileFile, fileExt;
    pws_os::splitpath(curfile, fileDrive, fileDir, fileFile, fileExt);
    ToUpper(fileDrive); ToUpper(execDrive);
    if (fileDrive == execDrive) {
      curfile = pws_os::makepath(L"", fileDir, fileFile, fileExt);
    }
  }
}

static BOOL CALLBACK EnumScreens(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam)
{
  MONITORINFO mi;
  HRGN hrgn2;

  HRGN *phrgn = (HRGN *)lParam;

  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  hrgn2 = CreateRectRgnIndirect(&mi.rcWork);
  CombineRgn(*phrgn, *phrgn, hrgn2, RGN_OR);
  ::DeleteObject(hrgn2);

  return TRUE;
}

HRGN WinUtil::GetWorkAreaRegion()
{
  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);

  HDC hdc = ::GetDC(NULL);
  EnumDisplayMonitors(hdc, NULL, EnumScreens, (LPARAM)&hrgn);
  ::ReleaseDC(NULL, hdc);

  return hrgn;
}
