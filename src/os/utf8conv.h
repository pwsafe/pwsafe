/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __OSUTF8CONV_H
#define __OSUTF8CONV_H
#include "typedefs.h"

namespace pws_os {
  /**
   * wrappers to convert to/from wchar_t and multibyte
   * (i.e., UTF8 and 8-bit encoding)
   */

  /*
   * See a Unix man page for details. Most important to note
   * that if dst is null, the required size is returned.
   * make sure maxdstlen is greater or equal to
   * wcstombs(nullptr, 0, src, srclen) + 1
   */
  extern size_t wcstombs(char *dst, size_t maxdstlen,
                         const wchar_t *src, size_t srclen, bool isUTF8 = true);

  extern size_t mbstowcs(wchar_t *dst, size_t maxdstlen,
                         const char *src, size_t srclen, bool isUTF8 = true);
#ifndef _WIN32
  // General conversion from/to char* to/from std::wstring routine
  // for use in os/unix
  extern std::wstring towc(const char *val);
  extern std::string tomb(const stringT& val);
#endif
}
#endif /* __OSUTF8CONV_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
