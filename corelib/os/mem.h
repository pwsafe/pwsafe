/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __MEM_H
#define __MEM_H
#include "typedefs.h"

namespace pws_os {
  /**
   * platform-indepenedent functions to lock and unlock memory
   * in RAM. Useful to prevent sensitive stuff from being swapped
   * to a snoopable swap file/device
   */
  extern bool mlock(void *p, size_t size);
  extern bool  munlock(void *p, size_t size);
};
#endif /* __MEM_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
