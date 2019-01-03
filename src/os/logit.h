/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef _OSLOGIT_H
#define _OSLOGIT_H

#include "typedefs.h"

#ifdef _WIN32
#include "wtypes.h"
#endif

/**
  In MFC under Windows, the circular log is dumped in a UserStream for
  separate processing (could probably can be implemented using wxWidgets under Windows)

  In other environments, this is not possible and so each log entry is preceded
  by "PWSLOG" to make it easier to find in the memory dump.
**/

/*
  Maybe an issue if a user compiles with a different compiler than VC or gcc
*/

#if defined(_MSC_VER)
// As __FILE__ and __FUNCTION__ are character strings, have to use %S rather than %s
// in UNICODE.This is a Microsoft specific extension to "printf Type Field Characters"
// and is not ANSI compatible.
// Implies Windows!
#define PWS_LOGIT_HEADER L"%S;\t%S; "
#endif

#if defined(__GNUC__)
// As __FILE__ and __FUNCTION__ are character strings, GCC treats %s as single-character
// strings when in UNICODE.
// Could be Windows or non-Windows!
#ifdef _WIN32
#define PWS_LOGIT_HEADER L"%s;\t%s; "
#else
#define PWS_LOGIT_HEADER L"PWSLOG %s;\t%s; "
#endif
#endif

#define PWS_LOGIT_CONCAT(str) PWS_LOGIT_HEADER L ## str

// Now the actual logging macros
#define PWS_LOGIT pws_os::Logit(PWS_LOGIT_HEADER, __FILE__, __FUNCTION__)
#define PWS_LOGIT_ARGS0(str) pws_os::Logit(PWS_LOGIT_CONCAT(str), \
            __FILE__, __FUNCTION__)
#define PWS_LOGIT_ARGS(format_str, ...) pws_os::Logit(PWS_LOGIT_CONCAT(format_str), \
            __FILE__, __FUNCTION__, __VA_ARGS__)

namespace pws_os {
  extern void Logit(LPCTSTR lpszFormat, ...);
}

#endif /* _OSLOGIT_H */
