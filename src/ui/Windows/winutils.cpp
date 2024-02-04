/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file winutils.cpp
*
* Contains generic utility functions that should be global and don't fit anywhere else
*/

#include "stdafx.h"
#include "winutils.h"

#include "WtsApi32.h"
#pragma comment (lib, "Wtsapi32")

#include <sstream>

#include "core/StringX.h"
#include "core/SysInfo.h"
#include "os/dir.h"

#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "PWDialog.h"

#include "core/PWSprefs.h"
#include "core/XMLprefs.h"
#include "os/env.h"
#include "os/file.h"
#include "os/lib.h"

// typedefs for function pointers:
typedef int (WINAPI* FP_GETDPI4SYSTEM) ();
typedef int (WINAPI* FP_GETDPI4WINDOW) (HWND);
typedef int (WINAPI* FP_GETSYSMETRICS4DPI) (int, UINT);


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

  HRGN *phrgn = (HRGN *)lParam;

  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  HRGN hrgn2 = CreateRectRgnIndirect(&mi.rcWork);
  CombineRgn(*phrgn, *phrgn, hrgn2, RGN_OR);
  ::DeleteObject(hrgn2);

  return TRUE;
}

HRGN WinUtil::GetWorkAreaRegion()
{
  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);

  HDC hdc = ::GetDC(nullptr);
  EnumDisplayMonitors(hdc, nullptr, EnumScreens, (LPARAM)&hrgn);
  ::ReleaseDC(nullptr, hdc);

  return hrgn;
}

// Following 2 functions moved from MigratePrefs.cpp

/**
 * PerformConfigMigration() should be called if (a) we detected the preference file
 * in the old location (exec. dir), and (b) user chose to migrate.
 * To be more accurate: In addition to being in the exec dir, the current
 * username/hostname combination should be in the file, since when there are
 * several u/h prefs, we migrate only the current one.
 */

bool WinUtil::OfferConfigMigration()
{
  /**
   * Offer the user the option of migrating config files iff ALL
   * of the following are true:
   * 1. Config file is currently in executable directory
   * 2. (obsolete)
   * 3. The executable directory is on a fixed or network drive
   * 4. The user did NOT override the config file, user name or host name
   *    via command line (-g, -u, -h).
   * 5. There isn't a config file already in the APPDATA location
   * 6. The APPDATA location exists
   */
  const SysInfo *si = SysInfo::GetInstance();

  // start with quickest checks
  if (PWSprefs::UserSetCfgFile() ||
      (si->GetRealHost() != si->GetEffectiveHost()) ||
      (si->GetRealUser() != si->GetEffectiveUser()))
    return false;

  std::wstring wsExecDir = pws_os::getexecdir();
  std::wstring wsExDrive, wsExDir, wsExFileName, wsExExt;

  pws_os::splitpath(wsExecDir, wsExDrive, wsExDir, wsExFileName, wsExExt);
  wsExDrive += L"\\";

  UINT uiDT = GetDriveType(wsExDrive.c_str());
  // Do not touch if not on local or remote (network) disk
  if (uiDT != DRIVE_FIXED && uiDT != DRIVE_REMOTE)
    return false;
      
  const std::wstring wsUserCfgDir = pws_os::getuserprefsdir(); // empty if couldn't find/create
  if (wsUserCfgDir.empty())
    return false;
  const std::wstring wsExecDirCfgFile = wsExecDir + PWSprefs::cfgFileName;
  const std::wstring wsDefaultCfgFile = wsUserCfgDir + PWSprefs::cfgFileName;
  return (pws_os::FileExists(wsExecDirCfgFile) &&
          !pws_os::FileExists(wsDefaultCfgFile));
}

bool WinUtil::PerformConfigMigration()
{
  /**
   *
   * We're here after the application's started and the conditions
   * listed above (in OfferConfigMigration) hold.
   * This constrains what we can assume and what we have to check.
   */

  ASSERT(OfferConfigMigration()); // should not be here otherwise!
  if (!OfferConfigMigration()) return false; // I mean it!

  PWSprefs::ConfigOption configoption;  // Note value meaningless at this point!
  std::wstring wsCnfgFile = PWSprefs::GetConfigFile(configoption);
  const std::wstring wsExecDir = pws_os::getexecdir();
  const std::wstring wsUserCfgDir = pws_os::getuserprefsdir();

  if (wsUserCfgDir.empty()) // couldn't find or create !?
    return false;

  std::wstring wsDefaultCfgFile = wsUserCfgDir + PWSprefs::cfgFileName;
  std::wstring wsExecDirCfgFile = wsExecDir + PWSprefs::cfgFileName;
  bool bRetVal(false);
  bool bExecCFRO(false);
  pws_os::FileExists(wsExecDirCfgFile, bExecCFRO);

  /**
   *  MIGRATE
  **/

  bRetVal = false;
  bool bNoMoreNodes(false);
  CXMLprefs newXMLConfig(wsExecDirCfgFile.c_str()); // for migrating user/host to new
  CXMLprefs oldXMLConfig(wsExecDirCfgFile.c_str()); // for removing user/host from old

  // Create the new one from it just containing our host/user
  if (!newXMLConfig.XML_Load())
    return false; // WTF?!?

  const SysInfo *si = SysInfo::GetInstance();
  stringT hn = si->GetEffectiveHost();
  PWSprefs::XMLify(charT('H'), hn);
  stringT un = si->GetEffectiveUser();
  PWSprefs::XMLify(charT('u'), un);

  stringT csHKCU_PREF = _T("Pwsafe_Settings\\");
  csHKCU_PREF += hn.c_str();
  csHKCU_PREF += _T("\\");
  csHKCU_PREF += un.c_str();
  csHKCU_PREF += _T("\\Preferences");

  bool rc = newXMLConfig.MigrateSettings(wsDefaultCfgFile, hn, un);
  if (rc) {
    // That worked, now remove us from the old one config file
    // in the Installation directory
    newXMLConfig.Unlock();

    // Since we now have new config file, remove host/user from old.
    if (!oldXMLConfig.XML_Load()) {
      rc = false;
      if (!oldXMLConfig.getReason().empty()) {
        CGeneralMsgBox gmb;
        gmb.SetMsg(oldXMLConfig.getReason().c_str());
        gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
        gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
        if (gmb.DoModal() == IDS_EXIT) {
          goto exit;
        }

        // Problem loading XML file but user says continue rather than Exit PWS!
        // But we will not remove them from the old file and we will
        // delete the new file - better luck next time!
        pws_os::DeleteAFile(wsDefaultCfgFile);
      }
    } // Load failed

    // Now remove this hostname/username from old configuration file in the
    // installation directory (as long as everything OK and it is not R-O)
    if (rc && !bExecCFRO) {
      rc = oldXMLConfig.RemoveHostnameUsername(hn, un, bNoMoreNodes);
      if (rc) {
        oldXMLConfig.XML_Store(csHKCU_PREF);

        // However, if no more host/user nodes in this file - delete the
        // configuration file from the installation directory!
        if (bNoMoreNodes) {
          pws_os::DeleteAFile(wsExecDirCfgFile);
        }

        bRetVal = true;
      } // RemoveHostnameUsername
    } // rc && !bExecCFRO
  } // MigrateSettings

  // If this all worked, now copy autoload_filters.xml if it exists and not
  // already in the new location.
  // This is ONLY done when we migrate the user's settings.
  if (bRetVal == true) {
    bool bALFRO(false);
    std::wstring wsOldAutoLoadFilters = wsExecDir + L"autoload_filters.xml";
    std::wstring wsNewAutoLoadFilters = wsUserCfgDir + L"autoload_filters.xml";
    if (pws_os::FileExists(wsOldAutoLoadFilters, bALFRO) &&
        !pws_os::FileExists(wsNewAutoLoadFilters)) {
      bool bCopyAutoloadFilters = pws_os::CopyAFile(wsOldAutoLoadFilters,
                                               wsNewAutoLoadFilters);

     // If we have copied it, there are no more nodes in the old configuration file
     // and it isn't read only - delete it from the installation directory
     if (bCopyAutoloadFilters && bNoMoreNodes && !bALFRO)
       pws_os::DeleteAFile(wsOldAutoLoadFilters);
    }
  }

  // Migration all done!

exit:
  // Clean up
  newXMLConfig.Unlock();
  oldXMLConfig.Unlock();

  // Set config file
  if (bRetVal)
    PWSprefs::SetConfigFile(wsDefaultCfgFile);
  return bRetVal;
}

/**
 * Following started out as a way to test hi-resolution support without access to a hires monitor.
 * Now it's a wrapper to support pre-Windows 10 systems as well.
 *
 */
UINT WinUtil::GetDPI(HWND hwnd)
{
  static bool inited = false;
  static FP_GETDPI4SYSTEM fp_getdpi4_system = nullptr;
  static FP_GETDPI4WINDOW fp_getdpi4_window = nullptr;


  if (!inited) {
    auto hUser32 = static_cast<HMODULE>(pws_os::LoadLibrary(L"User32.dll", pws_os::loadLibraryTypes::SYS));
    ASSERT(hUser32 != nullptr);
    if (hUser32 != nullptr) {
      fp_getdpi4_system = static_cast<FP_GETDPI4SYSTEM>(pws_os::GetFunction(hUser32, "GetDpiForSystem"));
      fp_getdpi4_window = static_cast<FP_GETDPI4WINDOW>(pws_os::GetFunction(hUser32, "GetDpiForWindow"));
      inited = true;
    }
  }
  UINT retval = defDPI;
  const stringT dbg_dpi = pws_os::getenv("PWS_DPI", false);
  if (dbg_dpi.empty()) {
    if (fp_getdpi4_window != nullptr && fp_getdpi4_system != nullptr)
      retval = (hwnd == nullptr) ? fp_getdpi4_system() : fp_getdpi4_window(hwnd);
  } else { // !dbg_dpi.empty()
    std::wistringstream iss(dbg_dpi);
    iss >> retval;
  }
  return retval;
}

void WinUtil::ResizeBitmap(CBitmap& bmp_src, CBitmap& bmp_dst, int dstW, int dstH)
{
  // from https://stackoverflow.com/questions/2770855/how-do-you-scale-a-cbitmap-object
  BITMAP bm = { 0 };
  bmp_src.GetBitmap(&bm);
  auto size = CSize(bm.bmWidth, bm.bmHeight);
  CWindowDC wndDC(nullptr);
  CDC srcDC;
  srcDC.CreateCompatibleDC(&wndDC);
  srcDC.SelectObject(&bmp_src);

  CDC destDC;
  destDC.CreateCompatibleDC(&wndDC);
  bmp_dst.CreateCompatibleBitmap(&wndDC, dstW, dstH);
  destDC.SelectObject(&bmp_dst);

  destDC.StretchBlt(0, 0, dstW, dstH, &srcDC, 0, 0, size.cx, size.cy, SRCCOPY);
}

void WinUtil::FixBitmapBackground(CBitmap& bm)
{
  // Change bitmap's {192,192,192} pixels
  // to current flavor of the month default background

  // Get how many pixels in the bitmap
  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  BITMAP bmInfo;
  int rc = bm.GetBitmap(&bmInfo);

  if (rc == 0) {
    ASSERT(0);
    return;
  }
  const UINT numPixels(bmInfo.bmHeight * bmInfo.bmWidth);

  // get a pointer to the pixels
  DIBSECTION ds;
  VERIFY(bm.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));

  RGBTRIPLE* pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
  if (pixels == nullptr) {
    ASSERT(0);
    return;
  }

  const RGBTRIPLE newbkgrndColourRGB = { GetBValue(crCOLOR_3DFACE),
                                        GetGValue(crCOLOR_3DFACE),
                                        GetRValue(crCOLOR_3DFACE) };

  for (UINT i = 0; i < numPixels; ++i) {
    if (pixels[i].rgbtBlue == 192 &&
      pixels[i].rgbtGreen == 192 &&
      pixels[i].rgbtRed == 192) {
      pixels[i] = newbkgrndColourRGB;
    }
  }
}

BOOL WinUtil::LoadScaledBitmap(CBitmap &bitmap, UINT nID, bool fixBckgrnd, HWND hwnd)
{
  CBitmap tmpBitmap;
  BITMAP bm;

  BOOL retval = tmpBitmap.Attach(
      ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nID), RT_BITMAP),
      MAKEINTRESOURCE(nID), IMAGE_BITMAP, 0, 0,
      (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));
  if (retval == FALSE)
    return retval;

  if (fixBckgrnd) {
    FixBitmapBackground(tmpBitmap);
  }

  UINT dpi = GetDPI(hwnd);
  tmpBitmap.GetBitmap(&bm);
  int dpiScaledWidth = MulDiv(bm.bmWidth, dpi, defDPI);
  int dpiScaledHeight = MulDiv(bm.bmHeight, dpi, defDPI);

  WinUtil::ResizeBitmap(tmpBitmap, bitmap, dpiScaledWidth, dpiScaledHeight);
  tmpBitmap.DeleteObject();
  return TRUE;
}

int  WinUtil::GetSystemMetrics(int nIndex, HWND hwnd)
{
  static FP_GETSYSMETRICS4DPI fp_getsysmetrics_4dpi = nullptr;
  static bool inited = false;

  if (!inited) {
    auto hUser32 = static_cast<HMODULE>(pws_os::LoadLibrary(reinterpret_cast<const TCHAR *>(L"User32.dll"), pws_os::loadLibraryTypes::SYS));
    ASSERT(hUser32 != nullptr);
    if (hUser32 != nullptr) {
      fp_getsysmetrics_4dpi = static_cast<FP_GETSYSMETRICS4DPI>(pws_os::GetFunction(hUser32, "GetSystemMetricsForDpi"));
      inited = true;
    }
  }
    if (fp_getsysmetrics_4dpi != nullptr) { // Windows 10 or greater
      UINT dpi = GetDPI(hwnd);
      return fp_getsysmetrics_4dpi(nIndex, dpi);
    } else { // server or older than Win10, punt to older API
      return ::GetSystemMetrics(nIndex);
    }
}

bool WinUtil::HasTouchscreen() // for BR1539 workaround
{
  int value = ::GetSystemMetrics(SM_DIGITIZER);
  return (value != 0);
}

DWORD WinUtil::SetWindowExcludeFromScreenCapture(HWND hwnd, bool excludeFromScreenCapture)
{
  ASSERT(::IsWindow(hwnd));
  if (!::IsWindow(hwnd))
    return ERROR_INVALID_WINDOW_HANDLE;
  DWORD dwNewDisplayAffinity = excludeFromScreenCapture ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
  DWORD dwCurrentDisplayAffinity;
  DWORD dwResult = ERROR_SUCCESS;
  if (::GetWindowDisplayAffinity(hwnd, &dwCurrentDisplayAffinity) &&
      dwNewDisplayAffinity != dwCurrentDisplayAffinity &&
      !::SetWindowDisplayAffinity(hwnd, dwNewDisplayAffinity)) {
    dwResult = ::GetLastError();
  }
  return dwResult;
}

