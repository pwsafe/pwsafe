/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
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
#else
  system("read");
#endif

  /* 
     Tidy up some memory leaks - at least 85 left!
     These are all probably caused in core/hmac.h
     line 57 & 64 "Hash = new H" leaving any
     exising H that was in Hash in limbo.

     Leave to Rony to fix :-)
  */

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
