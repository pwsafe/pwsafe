/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef _TYPEDEFS_H
/**
* Silly wrapper to abstract away the difference between a Unicode
* (wchar_t) and non-Unicode (char) std::string
*
*/

#include <string>

/*
 * _S is defined same as m'soft's _T, just to avoid collisions or
 * lousy include order dependencies.
 */

#ifdef UNICODE
typedef std::wstring stringT;
typedef wchar_t charT;
#define _S(x) L ## x
#else
typedef std::string stringT;
typedef char charT;
#define _S(x) x
#endif

#ifdef _WIN32
typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef __int64 int64;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;

typedef unsigned __int64   ulong64;
typedef unsigned long      ulong32;

typedef unsigned int uint;
#else /* !defined(_WIN32) */
#include <sys/types.h>
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef u_int8_t  uint8;
typedef u_int16_t uint16;
typedef u_int32_t uint32;
typedef u_int64_t uint64;

#ifdef UNICODE
#define _T(x) L ## x
typedef wchar_t TCHAR;
#else
#define _T(x) x
typedef char TCHAR;
#endif /* UNICODE */
typdef const TCHAR *LPCTSTR;
#endif /* _WIN32 */
          
#endif /* _TYPEDEFS_H */
