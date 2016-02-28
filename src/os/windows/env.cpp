/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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

#include <sstream>
#include <LMCONS.H> // for UNLEN definition
#include "../env.h"

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
  * Versions supported by current PasswordSafe
  *   Operating system       Version  Other
  *    Windows 8.1            6.2     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
  *    Windows 2012 R2        6.2     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
  *    Windows 8              6.1     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
  *    Windows 2012           6.1     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
  *    Windows 7              6.1     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
  *    Windows Server 2008 R2 6.1     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
  *    Windows Vista          6.0     OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
  *    Windows Server 2008    6.0     OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
  *    Windows Server 2003 R2 5.2     GetSystemMetrics(SM_SERVERR2) != 0
  *    Windows Home Server    5.2     OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
  *    Windows Server 2003    5.2     GetSystemMetrics(SM_SERVERR2) == 0
  *    Windows XP Pro x64     5.2     (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) &&
  *                                   (SYSTEM_INFO.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
  *    Windows XP             5.1     Not applicable
  *
  * Versions no longer supported by current PasswordSafe due to missing APIs
  *   Operating system       Version  Other
  *    Windows 2000           5.0     PlatformID 2
  *    Windows NT 4.0         4.0     PlatformID 2
  *    Windows ME             4.90    PlatformID 1
  *    Windows 98             4.10    PlatformID 1
  *    Windows 95             4.0     PlatformID 1
  *    Windows NT 3.51        3.51    PlatformID 2
  *    Windows NT 3.5          3.5     PlatformID 2
  *    Windows for Workgroups 3.11    PlatformID 0
  *    Windows NT 3.1          3.10    PlatformID 2
  *    Windows 3.0            3.0     n/a
  *    Windows 2.0            2.??    n/a
  *    Windows 1.0            1.??    n/a
  *
  */

bool pws_os::IsWindowsVistaOrGreater()
{
  OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
  DWORDLONG        dwlConditionMask = 0;

  VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
  VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

  osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
  osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);

  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != FALSE;
}
