/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __RAND_H
#define __RAND_H
#include "typedefs.h"

namespace pws_os {
  // Calling the following sets up a hook to the OS's best
  // rng, which, if successful, may then be called via
  // GetRandomData
  extern bool InitRandomDataFunction();
  
  extern bool GetRandomData(void *p, unsigned long len);
  
  // Calling following with a null p will return the size of
  // the generated seed in byte in slen. A pointer to slen
  // bytes should then be passed, which will be filled with
  // (hopefully) enough entropy to get the ball rolling...
  extern void GetRandomSeed(void *p, unsigned &slen);
}
#endif /* __RAND_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
