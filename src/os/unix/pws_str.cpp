/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of some wide-string related functionality
 */

#include <wchar.h>
#include "../pws_str.h"
#include "../utf8conv.h"
#include "../pws_tchar.h"
#include <algorithm>
#include <cstdarg>

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

/**
 * Get TCHAR buffer size by format string with parameters
 * @param[in] fmt - format string
 * @param[in] args - arguments for format string
 * @return buffer size including nullptr-terminating character
*/
unsigned int pws_os::GetStringBufSize(const TCHAR *fmt, va_list args)
{
  TCHAR *buffer=nullptr;
  unsigned int len = 0;
  va_list ar;
  va_copy(ar, args);
  // Linux doesn't do this correctly :-(
  unsigned int guess = 16;
  int nBytes = -1;
  while (true) {
    len = guess;
    buffer = new TCHAR[len];
    nBytes = _vstprintf_s(buffer, len, fmt, ar);
    va_end(ar);//after using args we should reset list
    va_copy(ar, args);
    /*
     * If 'nBytes' is zero due to an empty format string,
     * it would result in an endless memory-consuming loop.
     */
    ASSERT(nBytes != 0);
    if (nBytes++ > 0) {
      len = nBytes;
      break;
    } else { // too small, resize & try again
      delete[] buffer;
      buffer = nullptr;
      guess *= 2;
    }
  }

  va_end(ar);
  delete[] buffer;
  return len;
}

