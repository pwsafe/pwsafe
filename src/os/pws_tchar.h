/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef _PWS_TCHAR_H
/**
 * Use Windows' tchar.h for Windows build,
 * roll our own for others.
 *
 */
#ifdef _WIN32
#include <tchar.h>
#else
#define _gmtime64_s(ts64, tm64) gmtime64_r(tm64, ts64)
#define _mkgmtime32(ts) mktime(ts)
#define _localtime32_s(st, t) (localtime_r(t, st) != NULL ? 0 : 1)
#ifdef UNICODE
#include <wctype.h>
#define _istalpha(x) iswalpha(x)
#define _istalnum(x) iswalnum(x)
#define _totupper(x) towupper(x)
#define _totlower(x) towlower(x)
#define _istlower(x) iswlower(x)
#define _istupper(x) iswupper(x)
#define _istdigit(x) iswdigit(x)
#define _istspace(x) iswspace(x)
#include <wchar.h>
#include "linux/pws_time.h"
#define _tcsncpy(t, s, sc) wcsncpy(t, s, sc)
#define _tcslen(s) wcslen(s)
#define _tcsclen(s) wcslen(s)
#define _tcsicmp(s1, s2) wcscasecmp(s1, s2)
#define _tcscmp(s1, s2) wcscmp(s1, s2)
#define _tcsncmp(s1, s2, n) wcsncmp(s1, s2, n)
#define _tcschr(s, c) wcschr(s, c)
#define _tcsftime wcsftime
#define _tasctime_s(s, N, st) pws_os::asctime(s, N, st)
#define _vsctprintf(fmt, args) vswprintf(NULL, 0, fmt, args)
#define _vstprintf_s(str, size, fmt, args) vswprintf(str, size, fmt, args)
#define _ftprintf fwprintf
#include "linux/pws_str.h"
#define _tstoi(s) pws_os::wctoi(s)
#define _tstof(s) pws_os::wctof(s)
#else /* !UNICODE */
#include <ctype.h>
#define _istalpha(x) isalpha(x)
#define _istalnum(x) isalnum(x)
#define _totupper(x) toupper(x)
#define _totlower(x) tolower(x)
#define _istlower(x) islower(x)
#define _istupper(x) isupper(x)
#define _istdigit(x) isdigit(x)
#define _istspace(x) isspace(x)
#include <string.h>
#define _tcsncpy(t, s, sc) strncpy(t, s, sc)
#define _tcslen(s) strlen(s)
#define _tcsclen(s) strlen(s)
#define _tcsicmp(s1, s2) strcasecmp(s1, s2)
#define _tcscmp(s1, s2) strcmp(s1, s2)
#define _tcsncmp(s1, s2, n) strncmp(s1, s2, n)
#define _tcschr(s, c) strchr(s, c)
#include <time.h>
#define _tcsftime strftime
#define _tasctime_s(s, N, st) (asctime_r(st, s) != NULL ? 0 : 1)
#define _vsctprintf(fmt, args) vsnprintf(NULL, 0, fmt, args)
#define _vstprintf_s(str, size, fmt, args) vsnprintf(str, size, fmt, args)
#define _ftprintf fprintf
#define _tstoi(s) atoi(s)
#define _tstof(s) atof(s)
#endif /* UNICODE */
#endif /* _WIN32 */
#endif /* _PWS_TCHAR_H */
