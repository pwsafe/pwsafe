/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#ifdef UNICODE
#include <wctype.h>
#define _istalpha(x) iswalpha(x)
#define _totupper(x) towupper(x)
#define _totlower(x) towlower(x)
#define _istlower(x) iswlower(x)
#define _istupper(x) iswupper(x)
#define _istdigit(x) iswdigit(x)
#include <wchar.h>
#define _tcsncpy(t, s, sc) wcsncpy(t, s, sc)
#define _tcslen(s) wcslen(s)
#define _tcsicmp(s1, s2) wcscasecmp(s1, s2)
#define _tcsftime wcsftime
#define _tasctime_s(s, N, st) asctime_r(st, s)
#else /* !UNICODE */
#include <ctype.h>
#define _istalpha(x) isalpha(x)
#define _totupper(x) toupper(x)
#define _totlower(x) tolower(x)
#define _istlower(x) islower(x)
#define _istupper(x) isupper(x)
#define _istdigit(x) isdigit(x)
#include <string.h>
#define _tcsncpy(t, s, sc) strncpy(t, s, sc)
#define _tcslen(s) strlen(s)
#define _tcsicmp(s1, s2) strcasecmp(s1, s2)
#include <time.h>
#define _tcsftime strftime
#define _tasctime_s(s, N, st) asctime_r(st, s)
#endif /* UNICODE */
#endif /* _WIN32 */
#endif /* _PWS_TCHAR_H */
