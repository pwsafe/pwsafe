/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MEDIA_H
#define __MEDIA_H

#include "typedefs.h"

namespace pws_os {
  /**
  * Windows: Get Media Type from OS using FindMimeFromData
  *
  * Linux: Uses gio library? UNTESTED
  */
  stringT GetMediaType(const stringT &sfilename);
};

#endif /* __MEDIA_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
