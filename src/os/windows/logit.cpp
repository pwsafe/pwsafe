/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of logit.h
 */

#include "../logit.h"
#include "../../core/PWSLog.h"

#include <wtypes.h>

void pws_os::Logit(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);

  TCHAR szBuffer[1024];
  int nBuf = _vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), _TRUNCATE,
                           lpszFormat, args);
  ASSERT(nBuf > 0);

  PWSLog::GetLog()->Add(stringT(szBuffer));
  va_end(args);
}
