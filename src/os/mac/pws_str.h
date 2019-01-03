/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWS_STR_H
#define __PWS_STR_H
#include "../typedefs.h"

namespace pws_os {
  /**
   * Workaround the lack of a wchar_t version of silly conversion functions
   */
    extern int wctoi(const wchar_t *s);
    extern double wctof(const wchar_t *s);
  extern int wcscasecmp(const wchar_t* s1, const wchar_t* s2);
    extern wchar_t* wcsdup(const wchar_t* src);
};

#endif /* __PWS_STR_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
