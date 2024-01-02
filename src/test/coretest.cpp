/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
#ifdef WIN32
  // initialize MFC -- needed for string lookup in error handling
  if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), 0)) {
    std::cerr << _T("Fatal Error: MFC initialization failed") << std::endl;
  }
  AfxGetInstanceHandle();
#endif

  int rc = RUN_ALL_TESTS();

  // Delete singletons
  PWSLog::GetLog()->DeleteLog();
  PWSprefs::GetInstance()->DeleteInstance();
  PWSrand::GetInstance()->DeleteInstance();

  return rc;
}
