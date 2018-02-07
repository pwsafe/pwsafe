/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

int localtime64_r(const __time64_t *timep, struct tm *result)
{
  const time_t *tp = reinterpret_cast<const time_t *>(timep);

#ifdef PWS_BIG_ENDIAN
  //handle downsizing cast on 32-bit big-endian systems
  if (sizeof(time_t) < sizeof(__time64_t)) {
    //assume alignment
    assert(sizeof(__time64_t) % sizeof(time_t) == 0);
    size_t offset = (sizeof(__time64_t)/sizeof(time_t)) - 1;
    tp = reinterpret_cast<const time_t *>(((time_t*)timep) + offset);
  }
#endif

  return localtime_r(tp, result) != 0;
}

int pws_os::asctime(TCHAR *s, size_t, tm const *t)
{
  char cbuf[26]; // length specified in man (3) asctime
  asctime_r(t, cbuf);
  std::wstring wstr = pws_os::towc(cbuf);
  std::copy(wstr.begin(), wstr.end(), s);
  return 0;
}
