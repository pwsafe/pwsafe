/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of utf8conv.h
 */

#include "../typedefs.h"
#include "../utf8conv.h"
#include <cstdlib>
#include <clocale>
#include <cassert>
#include <cstring>
#include <string>

#include <CoreFoundation/CoreFoundation.h>

#if defined(PWS_LITTLE_ENDIAN)
#define wcharEncoding kCFStringEncodingUTF32LE
#else
#define wcharEncoding kCFStringEncodingUTF32BE
#endif

using namespace std;

class Startup {
public:
  Startup() {
    char *sl = setlocale(LC_ALL, "");
    if (sl == NULL)
      throw "Couldn't initialize locale - bailing out";
  }
};

static Startup startup;

size_t pws_os::wcstombs(char *dst, size_t maxdstlen,
                        const wchar_t *src, size_t srclen, bool isUTF8)
{
  if (!isUTF8)
    return ::wcstombs(dst, src, maxdstlen) + 1;

  if (srclen == size_t(-1))
    srclen = wcslen(src);
  else
    srclen = wcsnlen(src, srclen);  // CoreFoundation encodes NUL characters (no special treatment), make sure the input does not contain any NUL characters

  // Convert to UTF-16
  CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const unsigned char *>(src), srclen*sizeof(wchar_t), wcharEncoding, false);
  if (str == NULL)
    return 0;  // return wcstombs + 1, so 0 to signal error

  CFRange range = CFRangeMake(0, CFStringGetLength(str));
  CFIndex usedBufLen;

  // Convert to UTF-8
  // Note: in case dst == NULL this only calculates usedBufLen
  CFIndex idx = CFStringGetBytes(str, range, kCFStringEncodingUTF8, 0, false, reinterpret_cast<unsigned char *>(dst), maxdstlen, &usedBufLen);
  CFRelease(str);
  if (idx != range.length)
    return 0;  // return wcstombs + 1, so 0 to signal error

  if (dst != NULL && static_cast<size_t>(usedBufLen) < maxdstlen)
    dst[usedBufLen] = 0;

  return usedBufLen + 1;
}

size_t pws_os::mbstowcs(wchar_t *dst, size_t maxdstlen,
                        const char *src, size_t srclen, bool isUTF8)
{
  if (!isUTF8)
    return ::mbstowcs(dst, src, maxdstlen) + 1;

  if (srclen == size_t(-1))
    srclen = strlen(src);
  else
    srclen = strnlen(src, srclen);  // CoreFoundation encodes NUL characters (no special treatment), make sure the input does not contain any NUL characters

  // Convert to UTF-16
  CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const unsigned char *>(src), srclen, kCFStringEncodingUTF8, false);
  if (str == NULL)
    return 0;  // return mbstowcs + 1, so 0 to signal error

  CFRange range = CFRangeMake(0, CFStringGetLength(str));
  CFIndex usedBufLen;

  // Skip UTF-32 encoding if no output buffer provided, we have the number of wchars
  if (dst == NULL) {
      CFRelease(str);
      return range.length + 1;
  }

  // Convert to UTF-32
  CFIndex idx = CFStringGetBytes(str, range, wcharEncoding, 0, false, reinterpret_cast<unsigned char *>(dst), maxdstlen*sizeof(wchar_t), &usedBufLen);
  CFRelease(str);
  if (idx != range.length)
    return 0;  // return mbstowcs + 1, so 0 to signal error

  if (static_cast<size_t>(idx) < maxdstlen)
    dst[idx] = 0;

  return idx + 1;
}

wstring pws_os::towc(const char *val)
{
  wstring retval(L"");
  assert(val != NULL);
  long len = static_cast<long>(strlen(val));
  int wsize;
  const char *p = val;
  wchar_t wvalue;
  while (len > 0) {
    wsize = mbtowc(&wvalue, p, MB_CUR_MAX);
    if (wsize <= 0)
      break;
    retval += wvalue;
    p += wsize;
    len -= wsize;
  };
  return retval;
}

#ifdef UNICODE
std::string pws_os::tomb(const stringT& val)
{
  if (!val.empty()) {
    const size_t N = std::wcstombs(NULL, val.c_str(), 0);
    assert(N > 0);
    char* szstr = new char[N+1];
    szstr[N] = 0;
    std::wcstombs(szstr, val.c_str(), N);
    std::string retval(szstr);
    delete[] szstr;
    return retval;
  } else
    return string();
}
#else
std::string pws_os::tomb(const stringT& val)
{
  return val;
}
#endif
