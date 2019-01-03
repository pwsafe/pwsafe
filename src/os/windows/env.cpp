/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of env.h
 */

#ifndef __WX__
#include <afx.h>
#else
#include <Windows.h> // for GetCurrentProcessId()
#endif

#include "../env.h"
#include "../lib.h"

#include <sstream>
#include <LMCONS.H> // for UNLEN definition

stringT pws_os::getenv(const char *env, bool is_path)
{
  ASSERT(env != NULL);
  stringT retval;
  char* value;
  size_t requiredSize;
  getenv_s(&requiredSize, NULL, 0, env);
  if (requiredSize > 0) {
    value = new char[requiredSize];
    ASSERT(value);
    if (value != NULL) {
      getenv_s(&requiredSize, value, requiredSize, env);
      int wsize;
      wchar_t wvalue;
      char *p = value;
      do {
        wsize = mbtowc(&wvalue, p, MB_CUR_MAX);
        if (wsize <= 0)
          break;
        retval += wvalue;
        p += wsize;
        requiredSize -= wsize;
      } while (requiredSize != 1);
      delete[] value;
      if (is_path) {
        // make sure path has trailing '\'
        if (retval[retval.length()-1] != charT('\\'))
          retval += _T("\\");
      }
    }
  }
  return retval;
}

void pws_os::setenv(const char *name, const char *value)
{
  ASSERT(name != NULL && value != NULL);
  _putenv_s(name, value);
}

stringT pws_os::getusername()
{
  TCHAR user[UNLEN + sizeof(TCHAR)];
  //  ulen INCLUDES the trailing blank
  DWORD ulen = UNLEN + sizeof(TCHAR);
  if (::GetUserName(user, &ulen) == FALSE) {
    user[0] = TCHAR('?');
    user[1] = TCHAR('\0');
    ulen = 2;
  }
  ulen--;
  stringT retval(user);
  return retval;
}

stringT pws_os::gethostname()
{
  //  slen EXCLUDES the trailing blank
  TCHAR sysname[MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR)];
  DWORD slen = MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR);
  if (::GetComputerName(sysname, &slen) == FALSE) {
    sysname[0] = TCHAR('?');
    sysname[1] = TCHAR('\0');
    slen = 1;
  }
  stringT retval(sysname);
  return retval;
}

stringT pws_os::getprocessid()
{
  std::wostringstream os;
  os.width(8);
  os.fill(charT('0'));
  os << GetCurrentProcessId();

  return os.str();
}

/*
  Microsoft have depreciated GetVersion & GetVersionEx and so it is no longer possible to determine
  the version of Windows we are running under.

  With the release of Windows 8.1, the behavior of the GetVersionEx API has changed in the value it
  will return for the operating system version. The value returned by the GetVersionEx function now
  depends on how the application is manifested.

  Applications not manifested for Windows 8.1 or Windows 10 will return the Windows 8 OS version
  value (6.2). Once an application is manifested for a given operating system version, GetVersionEx
  will always return the version that the application is manifested for in future releases.

  The newer Version Helper functions are simply wrappers for VerifyVersionInfo(). Starting in
  Windows 10, it is now subject to manifestation as well:

  Windows 10: VerifyVersionInfo returns false when called by applications that do not have a
  compatibility manifest for Windows 8.1 or Windows 10 if the lpVersionInfo parameter is set
  so that it specifies Windows 8.1 or Windows 10, even when the current operating system
  version is Windows 8.1 or Windows 10. Specifically, VerifyVersionInfo has the following behavior:

  If the application has no manifest, VerifyVersionInfo behaves as if the operation system
  version is Windows 8 (6.2) even if it is not!

  If the application has a manifest that contains the GUID that corresponds to Windows 8.1,
  VerifyVersionInfo behaves as if the operation system version is Windows 8.1 (6.3) even if it is not!

  If the application has a manifest that contains the GUID that corresponds to Windows 10,
  VerifyVersionInfo behaves as if the operation system version is Windows 10 (10.0).

  The Version Helper functions use the VerifyVersionInfo function, so the behavior
  IsWindows8Point1OrGreater and IsWindows10OrGreater are similarly affected by the presence
  and content of the manifest.

  To get the true OS version regardless of manifestation, Microsoft suggests querying
  the file version of a system DLL:

    Call the GetFileVersionInfo   function on one of the system DLLs, such as Kernel32.dll,
    then call VerQueryValue to obtain the \\StringFileInfo\\<lang><codepage>\\ProductVersion
    sub-block of the file version information.

  Alternatively, use RtlGetVersion(), NetServerGetInfo(), or NetWkstaGetInfo() instead.
  They all report an accurate OS version and are not subject to manifestation (yet?).
*/

typedef void (WINAPI * RtlGetVersion_FUNC) (OSVERSIONINFOEXW *);

bool pws_os::RtlGetVersion(DWORD &dwMajor, DWORD &dwMinor, DWORD &dwBuild)
{
  RtlGetVersion_FUNC rtl_func;
  dwMajor = dwMinor = dwBuild = 0;

  HMODULE hMod = HMODULE(pws_os::LoadLibrary(L"ntdll.dll", pws_os::loadLibraryTypes::SYS));

  if (hMod != NULL) {
    rtl_func = (RtlGetVersion_FUNC)GetProcAddress(hMod, "RtlGetVersion");
    if (rtl_func == 0) {
      pws_os::FreeLibrary(hMod);
      return false;
    }

    OSVERSIONINFOEXW osw;
    ZeroMemory(&osw, sizeof(osw));
    osw.dwOSVersionInfoSize = sizeof(osw);

    rtl_func(&osw);

    pws_os::FreeLibrary(hMod);

    dwMajor = osw.dwMajorVersion;
    dwMinor = osw.dwMinorVersion;
    dwBuild = osw.dwBuildNumber;

    return true;
  } else {
    return false;
  }
}

/*
* Versions supported by current PasswordSafe
*   Operating system       Version  Other
*    Windows 10            10.0     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
*    Windows 8.1            6.3     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
*    Windows 2012 R2        6.2     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
*    Windows 8              6.2     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
*    Windows 2012           6.1     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
*    Windows 7              6.1     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
*    Windows Server 2008 R2 6.1     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION

*
* Versions no longer supported by current PasswordSafe due to missing APIs or not supported by
* latest version of Visual Studio.
*   Operating system       Version  Other
*    Windows Vista          6.0     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
*    Windows Server 2008    6.0     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
*    Windows Server 2003 R2 5.2     GetSystemMetrics(SM_SERVERR2) != 0
*    Windows Home Server    5.2     OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
*    Windows Server 2003    5.2     GetSystemMetrics(SM_SERVERR2) == 0
*    Windows XP Pro x64     5.2     (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) &&
*                                   (SYSTEM_INFO.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
*    Windows XP             5.1     Not applicable
*    Windows 2000           5.0     PlatformID 2
*    Windows NT 4.0         4.0     PlatformID 2
*    Windows ME             4.90    PlatformID 1
*    Windows 98             4.10    PlatformID 1
*    Windows 95             4.0     PlatformID 1
*    Windows NT 3.51        3.51    PlatformID 2
*    Windows NT 3.5         3.5     PlatformID 2
*    Windows for Workgroups 3.11    PlatformID 0
*    Windows NT 3.1         3.10    PlatformID 2
*    Windows 3.0            3.0     n/a
*    Windows 2.0            2.--    n/a
*    Windows 1.0            1.--    n/a
*
*/

bool pws_os::IsWindowsVistaOrGreater()
{
  DWORD dwMajor, dwMinor, dwBuild;

  bool rc = RtlGetVersion(dwMajor, dwMinor, dwBuild);

  if (rc) {
    return dwMajor >= 6;
  } else
    return false;
}

bool pws_os::IsWindows7OrGreater()
{
  DWORD dwMajor, dwMinor, dwBuild;

  bool rc = RtlGetVersion(dwMajor, dwMinor, dwBuild);

  if (rc) {
    return dwMajor >= 10  || (dwMajor == 6 && dwMinor >= 1);
  } else
    return false;
}

bool pws_os::IsWindows8OrGreater()
{
  DWORD dwMajor, dwMinor, dwBuild;

  bool rc = RtlGetVersion(dwMajor, dwMinor, dwBuild);

  if (rc) {
    return dwMajor >= 10 || (dwMajor == 6 && dwMinor >= 2);
  } else
    return false;
}

bool pws_os::IsWindows81OrGreater()
{
  DWORD dwMajor, dwMinor, dwBuild;

  bool rc = RtlGetVersion(dwMajor, dwMinor, dwBuild);

  if (rc) {
    return dwMajor >= 10 || (dwMajor == 6 && dwMinor == 3);
  } else
    return false;
}

bool pws_os::IsWindows10OrGreater()
{
  DWORD dwMajor, dwMinor, dwBuild;

  bool rc = RtlGetVersion(dwMajor, dwMinor, dwBuild);

  if (rc) {
    return dwMajor >= 10;
  } else
    return false;
}
