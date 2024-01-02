/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWS_STR_H
#define __PWS_STR_H

#include "typedefs.h"
#include <stdarg.h>

namespace pws_os {
  /**
   * Workaround the lack of a wchar_t version of silly conversion functions
   */
    extern int wctoi(const wchar_t *s);
    extern double wctof(const wchar_t *s);
    extern int sswscanf(const wchar_t *str, const wchar_t *format, ...);
    TCHAR* pws_itot(int val, TCHAR* out, unsigned base);

    
/**
 * Get TCHAR buffer size by format string with parameters
 * @param[in] fmt - format string
 * @param[in] args - arguments for format string
 * @return buffer size including null-terminating character
*/
    extern unsigned int GetStringBufSize(const TCHAR *fmt, va_list args);
};

#endif /* __PWS_STR_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
