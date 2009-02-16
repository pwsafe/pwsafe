/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of some time related functionality
 */

#include "pws_time.h"
#include "../utf8conv.h"

struct tm *gmtime64_r(const __time64_t *timep, struct tm *result)
{
  return gmtime_r((const time_t *)timep, result);
}

CTime::CTime() :m_t(0)
{
  localtime_r(&m_t, &m_tm);
}

CTime::CTime(time_t t) :m_t(t)
{
  localtime_r(&m_t, &m_tm);
}

CTime::CTime(int Y, int M, int D, int h, int m, int s, int dst)
{
  m_tm.tm_year = Y; m_tm.tm_mon = M; m_tm.tm_mday = D;
  m_tm.tm_hour = h; m_tm.tm_min = m; m_tm.tm_sec = s;
  m_tm.tm_isdst = dst;
  m_t = mktime(&m_tm);
}

int pws_os::asctime(TCHAR *s, unsigned int, tm const *t)
{
#ifdef UNICODE
  char cbuf[26]; // length specified in man (3) asctime
  asctime_r(t, cbuf);
  std::wstring wstr = pws_os::towc(cbuf);
  std::copy(wstr.begin(), wstr.end(), s);
#else
  asctime_r(t, s);
#endif
  return 0;
}
