/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

extern int localtime64_r(const __time64_t *timep, struct tm *result);

namespace pws_os {
  /**
   * Workaround the lack of a wchar_t version of asctime()
   */
  extern int asctime(TCHAR *buf, size_t N, const struct tm *tm);
};

#endif /* __PWS_TIME_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
