/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// Used for Windows minidumps
enum UserStream {usAll = -2, usPrefs = -1, us0 = 0, us1 = 1, us2 = 2, us3 = 3};

#ifndef _DEBUG

#include "stdafx.h"
#include "core/PWSprefs.h"

#if _MSC_VER < 1900
  #include <dbghelp.h>
#else
  // VS2015 produces warnings from SDK dbghelp.h re: lines 1544 & 3190
  //   'typedef ': ignored on left of '' when no variable is declared
  // This is true with SDK 7.1A (won't be fixed by MS) & 8.1 (unknown if will be fixed).
  // However, it is fixed with SDK 10 for Windows 10 applications.
  #pragma warning(push)
  #pragma warning(disable: 4091)
  #include <dbghelp.h>
  #pragma warning(pop)
#endif

// User stream 0 version
#define IVERSION 1

// Function call definitions
void InstallFaultHandler(const int major, const int minor, const int build,
                         const wchar_t *revision, const DWORD timestamp);
void LocalizeFaultHandler(HINSTANCE inst);

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *ExInfo);
void RemoveFaultHandler(bool bFreeLibrary = true);
void PopulateMinidumpUserStreams(PWSprefs *prefs, bool bOpen, bool bRW, UserStream iStream);

#endif
