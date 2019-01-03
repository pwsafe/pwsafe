/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

// Following disables Microsoft's telemetry code
// added in VS2015
// See https://www.reddit.com/r/cpp/comments/4ibauu/visual_studio_adding_telemetry_function_calls_to/
extern "C"
{
        void _cdecl __vcrt_initialize_telemetry_provider() {}
        void _cdecl __telemetry_main_invoke_trigger() {}
        void _cdecl __telemetry_main_return_trigger() {}
        void _cdecl __vcrt_uninitialize_telemetry_provider() {}
};

void pws_os::Logit(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);

  TCHAR szBuffer[1024];
  int nBuf = _vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), _TRUNCATE,
                           lpszFormat, args);
#ifdef DEBUG
  ASSERT(nBuf > 0);
#else
  UNREFERENCED_PARAMETER(nBuf); // In Release build only otherwise MS Compiler warning
#endif
  PWSLog::GetLog()->Add(stringT(szBuffer));
  va_end(args);
}
