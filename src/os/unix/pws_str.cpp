/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of some wide-string related functionality
 */

#include <wchar.h>
#include "pws_str.h"
#include "../utf8conv.h"
#include "../../core/PwsPlatform.h"
#include <algorithm>

int pws_os::wctoi(const wchar_t *s)
{
  return int(wcstol(s, nullptr, 10));
}

double pws_os::wctof(const wchar_t *s)
{
  return double(wcstold(s, nullptr));
}

TCHAR* pws_os::pws_itot(int val, TCHAR* out, unsigned base)
{
  const TCHAR digits[] = _T("0123456789abcdef");
  
  assert(base > 0 && base <= NumberOf(digits));
  
  TCHAR* p = out;
  
  do {
    *p++ = digits[val % base];
  } 
  while( (val /= base) != 0);

  *p++ = 0;
  
  std::reverse(out, p);
  return out;
}
