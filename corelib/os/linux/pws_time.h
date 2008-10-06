/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWS_TIME_H
#define __PWS_TIME_H
#include "../typedefs.h"

typedef time_t __time32_t;
typedef unsigned long long __time64_t;

extern struct tm *gmtime64_r(const __time64_t *timep, struct tm *result);

namespace pws_os {
  /**
   * Workaround the lack of a wchar_t version of asctime()
   */
  extern int asctime(TCHAR *buf, size_t N, const struct tm *tm);
};

// Provide a functional clone of MFC's CTime class
class CTime {
public:
  CTime();
  CTime(time_t t);
  CTime(int Y, int M, int D, int h, int m, int s, int dst=-1);
  ~CTime() {}
  int GetYear() const {return m_tm.tm_year;}
  int GetMonth() const {return m_tm.tm_mon;}
  int GetDay() const {return m_tm.tm_mday;}
  int GetHour() const {return m_tm.tm_hour;}
  int GetMinute() const {return m_tm.tm_min;}
  int GetSecond() const {return m_tm.tm_sec;}
  time_t GetTime() const {return m_t;}
private:
  time_t m_t;
  struct tm m_tm;
};
#endif /* __PWS_TIME_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
