/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "../logit.h"
#include "../../core/PWSLog.h"

#include <stdio.h>
#include <stdarg.h>

enum {MAX_LOG_STATEMENT = 1024 * 64, STARTING_LOG_STATEMENT = 256};

#if defined(UNICODE) || defined(_UNICODE)
# define vstprintf vswprintf
# define _stprintf_s swprintf
#else
# define vstprintf vsprintf
# define _stprintf_s snprintf
#endif

void pws_os::Logit(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);

  TCHAR *szbuffer = 0;
  int nwritten, len = STARTING_LOG_STATEMENT;
  do {
    len *= 2;
    delete [] szbuffer;
    szbuffer = new TCHAR[len + 1];
    memset(szbuffer, 0, sizeof(TCHAR) * (len + 1));
    nwritten = vstprintf(szbuffer, len, lpszFormat, args);
    //apple's documentation doesn't say if nwritten is +ve, -ve, 0 or if errno is set in case of overflow
  }
  while(!(nwritten > 0 && nwritten < len) && len <= MAX_LOG_STATEMENT);

  PWSLog::GetLog()->Add(stringT(szbuffer));
  delete[] szbuffer;
  va_end(args);
}
