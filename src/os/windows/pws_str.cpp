/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of GetStringBufSize()
 */

#include <wchar.h>
#include "../pws_str.h"
#include <cstdarg>


/**
 * Get TCHAR buffer size by format string with parameters
 * @param[in] fmt - format string
 * @param[in] args - arguments for format string
 * @return buffer size including nullptr-terminating character
*/
unsigned int pws_os::GetStringBufSize(const TCHAR *fmt, va_list args)
{
  return _vsctprintf(fmt, args) + 1;
}

