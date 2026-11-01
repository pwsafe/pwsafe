/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWSLog.h"
#include "core/PWSprefs.h"
#include "core/PWSrand.h"

#include "gtest/gtest.h"
#include "TestCommon.h"

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#include <cstdio>
#include <cstdlib>

static int __cdecl AbortOnCrtAssertHook(int reportType, char *message, int *)
{
  if (reportType != _CRT_ASSERT)
    return FALSE;
  fputs(message, stderr);
  fflush(stderr);
  // Not abort(): the debug CRT's abort() may pop up a "Debug Error!" dialog,
  // e.g., when running under Visual Studio
  _exit(3);
}

AbortOnCrtAssert::AbortOnCrtAssert()
{
  _CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, AbortOnCrtAssertHook);
}

AbortOnCrtAssert::~AbortOnCrtAssert()
{
  _CrtSetReportHook2(_CRT_RPTHOOK_REMOVE, AbortOnCrtAssertHook);
}
#endif

#ifndef WIN32
#include <langinfo.h>
#include <cstring>
#endif

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);

#ifndef WIN32
  // utf8conv.cpp's Startup object called setlocale(LC_ALL,"") before main().
  // Without a UTF-8 locale some tests will fail because the C library's wide-char to multi-byte string
  // conversions silently corrupt multi-byte characters.
  if (strcmp(nl_langinfo(CODESET), "UTF-8") != 0) {
    std::cerr << "FATAL: locale does not support UTF-8"
                 " (CODESET=" << nl_langinfo(CODESET) << ").\n"
                 "       Set LANG=C.UTF-8 (or any UTF-8 locale)"
                 " before running coretest.\n";
    return 1;
  }
#endif
#ifdef WIN32
  HRESULT hrCOM = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(hrCOM)) {
    std::wcerr << _T("Fatal Error: COM initialization failed") << std::endl;
    return 1;
  }

  // initialize MFC -- needed for string lookup in error handling
  if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), 0)) {
    std::wcerr << _T("Fatal Error: MFC initialization failed") << std::endl;
    CoUninitialize();
    return 1;
  }
  AfxGetInstanceHandle();
#endif

  int rc = RUN_ALL_TESTS();

  // Delete singletons
  PWSLog::GetLog()->DeleteLog();
  PWSprefs::GetInstance()->DeleteInstance();
  PWSrand::GetInstance()->DeleteInstance();

#ifdef WIN32
  CoUninitialize();
#endif

  return rc;
}
