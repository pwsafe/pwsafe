/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef _PWS_TCHAR_H
#define _PWS_TCHAR_H
/**
 * Use Windows' tchar.h for Windows build,
 * roll our own for others.
 *
 */
#ifdef _WIN32
#include <tchar.h>
#else
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
#ifdef __linux__
#include "unix/pws_time.h"
#define _tcsdup(s) wcsdup(s)
#endif
#ifdef __FreeBSD__
#include "unix/pws_time.h"
#define _tcsdup(s) wcsdup(s)
#endif // __FreeBSD__
#include "funcwrap.h"
#define _tcsncpy(t, s, sc) wcsncpy(t, s, sc)
#define _tcsncpy_s wcsncpy_s
#define _tcslen(s) wcslen(s)
#define _tcsclen(s) wcslen(s)
#define _tcscmp(s1, s2) wcscmp(s1, s2)
#define _tcsncmp(s1, s2, n) wcsncmp(s1, s2, n)
#define _tcschr(s, c) wcschr(s, c)
#define _tcsftime wcsftime
#define _tasctime_s(s, N, st) pws_os::asctime(s, N, st)
#define _vsctprintf(fmt, args) vswprintf(NULL, 0, fmt, args)
#define _vstprintf_s vswprintf_s
#define _ftprintf fwprintf
#define _stprintf_s swprintf_s
#define _stscanf swscanf
#define _stscanf_s swscanf_s
#ifdef __PWS_MACINTOSH__
# include "./mac/pws_str.h"
#define _tcsicmp pws_os::wcscasecmp
#define _tcsdup pws_os::wcsdup
#else
#define _tcsicmp(s1, s2) wcscasecmp(s1, s2)
# include "unix/pws_str.h"
#endif
#define _ttoi(s) pws_os::wctoi(s)
#define _tstoi(s) pws_os::wctoi(s)
#define _tstof(s) pws_os::wctof(s)
#ifdef __linux__
#include "./file.h"
#define _tfopen(f,m) pws_os::FOpen(f,m)
#endif
#define _itot(i, buf, base) pws_os::pws_itot(i, buf, base)
#endif /* _WIN32 */
#endif /* _PWS_TCHAR_H */
