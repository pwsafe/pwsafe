/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
  int rc = RUN_ALL_TESTS();

#ifdef WIN32
  system("pause");
#endif

  // Need to find these in order to delete them
  PWSLog *pwslog = PWSLog::GetLog();
  PWSprefs *pwsprefs = PWSprefs::GetInstance();
  PWSrand *pwsrand = PWSrand::GetInstance();

  pwsprefs->DeleteInstance();
  pwslog->DeleteLog();
  pwsrand->DeleteInstance();

  // To stop Compiler warning C4189
  pwsprefs = NULL;
  pwslog = NULL;
  pwsrand = NULL;

  return rc;
}
