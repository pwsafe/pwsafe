/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "../logit.h"
#include "../pws_str.h"

#include <stdio.h>
#include <stdarg.h>

const stringT pws_os::Logit(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);

  int num_written;

  unsigned int num_required = pws_os::GetStringBufSize(lpszFormat, args);
  va_end(args);  // after using args we should reset list
  va_start(args, lpszFormat);

  wchar_t *szBuffer = new wchar_t[num_required];
  num_written = vswprintf(szBuffer, num_required, lpszFormat, args);
  assert(static_cast<int>(num_required) == num_written + 1);
  szBuffer[num_required - 1] = L'\0';
  UNREFERENCED_PARAMETER(num_written); // used only in assert
  const stringT retval(szBuffer);
  delete[] szBuffer;
  va_end(args);
  return retval;
}
