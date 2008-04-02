/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __RAND_H
#define __RAND_H
#include "typedefs.h"

namespace pws_os {
  extern bool InitRandomDataFunction();
  extern bool GetRandomData(void *p, unsigned long len);
};
#endif /* __RAND_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
