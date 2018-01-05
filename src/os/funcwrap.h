/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef _FUNCWRAP_H
#define _FUNCWRAP_H

#include "typedefs.h"
#include <errno.h>
#include <cstring>
#include <ctime>

/**
 * Some function wrappers for cross platform/complier compatibility
*/
#ifndef _MSC_VER
#ifndef _MAX__TIME64_T
//number of seconds from 00:00:00, 01/01/1970 UTC to 23:59:59. 12/31/3000 UTC
#define _MAX__TIME64_T 0x793406fffLL
#endif
#ifndef _MAX__TIME32_T
//number of seconds from 00:00:00, 01/01/1970 UTC to 23:59:59, 01/18/2038 UTC
#define _MAX__TIME32_T 0x7fffd27f
#endif

inline errno_t localtime_s(struct tm *_tm, const time_t *time)
{
  if (!_tm) {
    return EINVAL;
  } else if (!time || (*time > _MAX__TIME32_T) || (*time < 0)) {
    _tm->tm_sec=_tm->tm_min=_tm->tm_hour=-1;
    _tm->tm_mday=_tm->tm_mon=_tm->tm_year=-1;
    _tm->tm_wday=_tm->tm_yday=_tm->tm_isdst=-1;
    return EINVAL;
  }
  struct tm *res = std::localtime(time);
  if (res) {
    *_tm = *res;
    return 0;
  }
  return EINVAL;
}

inline errno_t memcpy_s(void *dst, size_t dst_size, const void *src, size_t cnt)
{
  if (!dst) {
    return EINVAL;
  } else if (!src) {
    std::memset(dst, 0, dst_size);
    return EINVAL;
  } else if (dst_size < cnt) {
    std::memset(dst, 0, dst_size);
    return ERANGE;
  } else {
    std::memcpy(dst, src, cnt);
    return 0;
  }
}

#define _mktime32(ts) mktime(ts)

inline errno_t _localtime64_s(struct tm* _tm, const __time64_t* time)
{
  if (!_tm) {
    return EINVAL;
  } else if (!time || (*time > _MAX__TIME64_T)) {
    _tm->tm_sec=_tm->tm_min=_tm->tm_hour=-1;
    _tm->tm_mday=_tm->tm_mon=_tm->tm_year=-1;
    _tm->tm_wday=_tm->tm_yday=_tm->tm_isdst=-1;
    return EINVAL;
  }
  return localtime64_r(time, _tm) ? 0 : EINVAL;
}

#include <cwchar>

#ifndef _TRUNCATE
#define _TRUNCATE static_cast<size_t>(-1)

inline errno_t wcsncpy_s(wchar_t *dst, size_t dst_size, const wchar_t *src, size_t cnt)
{
  if (cnt == _TRUNCATE)
    cnt = dst_size - 1;

  if (!dst || !dst_size) {
    return EINVAL;
  } else if (!src) {
    dst[0] = L'\0';
    return EINVAL;
  } else if (dst_size <= cnt) { // '<=' because we need space for null character
    dst[0] = L'\0';
    return ERANGE;
  }

  std::wcsncpy(dst, src, cnt);
  dst[cnt] = L'\0'; // don't forget null (dst[cnt] is available because dst_size > cnt)
  return 0; // truncation state not reported
}

// to create full wrapper for swprintf_s/swscanf_s/vswprintf_s we need to parse
// format specification and process varargs, so for now just substitute
// non-secure version
#define swprintf_s swprintf
#define swscanf_s swscanf
#define vswprintf_s vswprintf
#endif

#endif /* !_MSC_VER */

#endif /* _FUNCWRAP_H */
