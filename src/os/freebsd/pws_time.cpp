/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of some time related functionality
 */

#include "pws_time.h"
#include "../utf8conv.h"

struct tm *gmtime64_r(const __time64_t *timep, struct tm *result)
{
  return gmtime_r(reinterpret_cast<const time_t *>(timep), result);
}

int pws_os::asctime(TCHAR *s, size_t, tm const *t)
{
  char cbuf[26]; // length specified in man (3) asctime
  asctime_r(t, cbuf);
  std::wstring wstr = pws_os::towc(cbuf);
  std::copy(wstr.begin(), wstr.end(), s);
  return 0;
}
