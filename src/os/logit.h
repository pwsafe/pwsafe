/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

#ifdef UNICODE
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#define PWS_LOGIT_CONCAT(x) L"%s;\t%s; " L ## x
#define __PWS_FILE__ WIDEN(__FILE__)
#define __PWS__FUNCTION__ WIDEN(__FUNCTION__)
#else
#define PWS_LOGIT_CONCAT(x) "%s;\t%s; " ## x
#define __PWS_FILE__ __FILE__
#define __PWS__FUNCTION__ __FUNCTION__
#endif

/*

In MFC under Windows, the circular log is dumped in a UserStream for separate processing.

In other environments, this is not possible and so each log entry is preceded by "PWSLOG"
to make it easier to find in the memory dump.

*/

// MS Compiler
#if defined(_MSC_VER)

#define PWS_LOGIT pws_os::Logit(_T("%s;\t%s;"), __PWS_FILE__, __PWS__FUNCTION__);
#define PWS_LOGIT_ARGS(x, ...) pws_os::Logit(PWS_LOGIT_CONCAT(x), \
            __PWS_FILE__, __PWS__FUNCTION__, __VA_ARGS__);

#else

#define PWS_LOGIT pws_os::Logit(_T("PWSLOG %s;\t%s;"), __PWS_FILE__, __PWS__FUNCTION__);
#define PWS_LOGIT_ARGS(x, ...) pws_os::Logit(PWS_LOGIT_CONCAT(x), \
            __PWS_FILE__, __PWS__FUNCTION__, __VA_ARGS__);
#endif


namespace pws_os {
  extern void Logit(LPCTSTR lpszFormat, ...);
}

#endif /* _OSLOGIT_H */
