/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

#include <type_traits> // for static_assert

/**
* Silly wrapper to abstract away the difference between a Unicode
* (wchar_t) and non-Unicode (char) std::string, as well as
* Linux/Windows portability.
*
*/

#include <string>
/*
 * _S is defined same as m'soft's _T, just to avoid collisions or
 * lousy include order dependencies.
 */

// Sometimes we need specific ones irrespective of in Unicode mode or not.
// In particular, the underlying format of most XML is Unicode.
typedef std::wstring wstringT;
typedef std::string  cstringT;

typedef std::wstring stringT;
typedef wchar_t charT;
#define _S(x) L ## x

#include "../core/PwsPlatform.h" // for afxwin.h, and endian macros

// Hotkey values. Internal PWS values - need to convert to either MFC or wxWidgets
// values in the GUI when retrieving these from the preferences and aso back to PWS
// internal values before giving them to PWSPrefs for saving in the XML config file.
#define PWS_HOTKEYF_ALT     0x01
#define PWS_HOTKEYF_CONTROL 0x02
#define PWS_HOTKEYF_SHIFT   0x04
#define PWS_HOTKEYF_EXT     0x08

// wxWidgets Only - Not used in Windows MFC
#define PWS_HOTKEYF_ALTGR   PWS_HOTKEYF_ALT | PWS_HOTKEYF_CONTROL
#define PWS_HOTKEYF_META    0x10
#define PWS_HOTKEYF_WIN     0x20
#define PWS_HOTKEYF_CMD     0x40

// Define the MFC HotKey values if not built under Windows
#ifndef HOTKEYF_SHIFT
// Windows MFC HotKey values
#define HOTKEYF_SHIFT           0x01
#define HOTKEYF_CONTROL         0x02
#define HOTKEYF_ALT             0x04
#define HOTKEYF_EXT             0x08
#endif

#ifdef _WIN32
#include "TCHAR.h"
typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef __int64 int64;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;

typedef unsigned __int64 ulong64;
typedef unsigned long    ulong32;

typedef unsigned int uint;

typedef void *HANDLE;

// Following not defined by Windows - needed by _access mode
#define F_OK 00
#define W_OK 02
#define R_OK 04

#ifdef __WX__
#include "debug.h"

typedef unsigned long DWORD;
// assorted conveniences:
#define ASSERT(p) assert(p)
#define VERIFY(p) if (!(p)) pws_os::Trace(_T("VERIFY Failed"))
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif  /* __WX__ */

#else /* !defined(_WIN32) */
#include <stdint.h>
#include <sys/types.h>
#include <errno.h>
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#ifndef _ERRNO_T
typedef int errno_t;
#endif

#ifndef _T
#define _T(x) L ## x
#endif
typedef wchar_t TCHAR;

// mimic Microsoft conventional typdefs:
typedef TCHAR *LPTSTR;
typedef const TCHAR *LPCTSTR;
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
#if defined(PWS_LITTLE_ENDIAN)
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define LOWORD(ul) (WORD(DWORD(ul) & 0xffff))
#define HIWORD(ul) (WORD(DWORD(ul) >> 16))
#define MAKELONG(low, high) ((LONG) (((WORD) (low)) | ((DWORD) ((WORD) (high))) << 16))
#define MAKEWORD(low, high) ((WORD)((((WORD)(high)) << 8) | ((BYTE)(low))))
#elif defined(PWS_BIG_ENDIAN)
#define HIBYTE(w) ((BYTE)(w))
#define LOBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define HIWORD(ul) (WORD(DWORD(ul) & 0xffff))
#define LOWORD(ul) (WORD(DWORD(ul) >> 16))
#define MAKELONG(low, high) ((LONG) (((WORD) (high)) | ((DWORD) ((WORD) (low))) << 16))
#define MAKEWORD(low, high) ((WORD)((((WORD)(low)) << 8) | ((BYTE)(high))))
#else
#error "One of PWS_LITTLE_ENDIAN or PWS_BIG_ENDIAN must be defined before including typedefs.h"
#endif
typedef int32_t LPARAM;
typedef unsigned int UINT;
typedef int HANDLE;
#define INVALID_HANDLE_VALUE HANDLE(-1)

// assorted conveniences:
#define ASSERT(p) assert(p)
#define VERIFY(p) if (!(p)) TRACE(_T("VERIFY Failed"))
#define TRACE pws_os::Trace
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

/* These two files require the above definitions */
#include "debug.h"
#include "unix/pws_time.h"
#endif /* _WIN32 */

// Compile-time check that our sized types are correct:
static_assert(sizeof(int8) == 1, "int8 misdefined");
static_assert(sizeof(uint8) == 1, "uint8 misdefined");
static_assert(sizeof(int16) == 2, "int16 misdefined");
static_assert(sizeof(uint16) == 2, "uint16 misdefined");
static_assert(sizeof(int32) == 4, "int32 misdefined");
static_assert(sizeof(uint32) == 4, "uint32 misdefined");
static_assert(sizeof(int64) == 8, "int64 misdefined");
static_assert(sizeof(uint64) == 8, "uint64 misdefined");

#endif /* _TYPEDEFS_H */
