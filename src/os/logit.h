/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef _OSLOGIT_H
#define _OSLOGIT_H

/*

In MFC under Windows, the circular log is dumped in a UserStream for separate processing.

In other environments, this is not possible and so each log entry is preceded by "PWSLOG"
to make it easier to find in the memory dump.

*/

#include "typedefs.h"

#ifdef _WIN32
#include "wtypes.h"
#endif

#if defined(_MSC_VER)
#ifdef UNICODE
#define PWS_LOGIT_HEADER L"%s;\t%s; "
#else
#define PWS_LOGIT_HEADER  "%s;\t%s; "
#endif
#else
#ifdef UNICODE
// Not MSC - prepend "PWSLOG" to each record
#define PWS_LOGIT_HEADER L"PWSLOG %s;\t%s; "
#else
#define PWS_LOGIT_HEADER  "PWSLOG %s;\t%s; "
#endif
#endif

#define PWS_GET_FUNC_NAME(n) __PWS__FN__ ## n

#ifdef UNICODE
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#define PWS_LOGIT_CONCAT(x) PWS_LOGIT_HEADER L ## x
#define __PWS_FILE__  WIDEN(__FILE__)

#if defined(_MSC_VER)
#define __PWS_FUNCTION__ __LPREFIX( __FUNCTION__ )
#else
#define __PWS_FUNCTION__ WIDEN( __FUNCTION__ )
#endif

#else
#define PWS_LOGIT_CONCAT(x) PWS_LOGIT_HEADER ## x
#define __PWS_FILE__ __FILE__
#define __PWS_FUNCTION__ __FUNCTION__
#endif

#ifdef UNICODE
#define PWS_LOGIT0(n) \
  const std::wstring PWS_GET_FUNC_NAME(n)(__PWS_FUNCTION__); \
  pws_os::Logit(PWS_LOGIT_HEADER, \
            __PWS_FILE__, PWS_GET_FUNC_NAME(n).c_str());
#define PWS_LOGIT0_ARGS(n, x, ...) \
  const std::wstring PWS_GET_FUNC_NAME(n)(__PWS_FUNCTION__); \
  pws_os::Logit(PWS_LOGIT_CONCAT(x), \
            __PWS_FILE__, PWS_GET_FUNC_NAME(n).c_str(), __VA_ARGS__);
#else
#define PWS_LOGIT0(n) \
  const std::string PWS_GET_FUNC_NAME(n)(__FUNCTION__); \
  pws_os::Logit(PWS_LOGIT_HEADER, \
            __PWS_FILE__, PWS_GET_FUNC_NAME(n).c_str());
#define PWS_LOGIT0_ARGS(n, x, ...) \
  const std::string PWS_GET_FUNC_NAME(n)(__FUNCTION__); \
  pws_os::Logit(PWS_LOGIT_CONCAT(x), \
            __PWS_FILE__, PWS_GET_FUNC_NAME(n).c_str(), __VA_ARGS__);
#endif

#define PWS_LOGIT              PWS_LOGIT0(__COUNTER__)
#define PWS_LOGIT_ARGS(x, ...) PWS_LOGIT0_ARGS(__COUNTER__, x, __VA_ARGS__)

namespace pws_os {
  extern void Logit(LPCTSTR lpszFormat, ...);
}

#endif /* _OSLOGIT_H */
