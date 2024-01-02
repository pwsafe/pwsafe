/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "os/env.h"
#include "os/dir.h"
#include "PWSdirs.h"

/**
* Provide directories used by application
* The functions here return values that cause the application
* to default to current behaviour.
*/

stringT PWSdirs::execdir;
//-----------------------------------------------------------------------------

stringT PWSdirs::GetOurExecDir()
{
  if (execdir.empty())
    execdir = pws_os::getexecdir();
  return execdir;
}

stringT PWSdirs::GetSafeDir()
{
  // returns pws_os::getsafedir
  stringT retval = pws_os::getsafedir();
  return retval;
}

stringT PWSdirs::GetConfigDir()
{
  // PWS_PREFSDIR overrides all:
  stringT retval(pws_os::getenv("PWS_PREFSDIR", true));
  if (retval.empty()) {
    retval = pws_os::getuserprefsdir(); // "LOCAL_APPDATA\PasswordSafe" for Windows,
                                          // ~/.pwsafe for Linux
    if (retval.empty())
      retval = GetOurExecDir(); // ancient fallback - remove?
  }
  return retval;
}

stringT PWSdirs::GetXMLDir()
{
  stringT retval = pws_os::getxmldir();
  return retval;
}

stringT PWSdirs::GetHelpDir()
{
  stringT retval = pws_os::gethelpdir();
  return retval;
}

stringT PWSdirs::GetExeDir()
{
  stringT retval = GetOurExecDir();
  return retval;
}

void PWSdirs::Push(const stringT &dir)
{
  const stringT CurDir(pws_os::getcwd());
  if (CurDir != dir) { // minor optimization
    dirs.push(CurDir);
    pws_os::chdir(dir);
  }
}

void PWSdirs::Pop()
{
  if (!dirs.empty()) {
    pws_os::chdir(dirs.top());
    dirs.pop();
  }
}

PWSdirs::~PWSdirs()
{
  while (!dirs.empty()) {
    pws_os::chdir(dirs.top());
    dirs.pop();
  }
}
