/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __ENV_H
#define __ENV_H
#include "typedefs.h"

namespace pws_os {
  /**
   * getenv return environment value associated with env, empty string if
   * not defined. if is_path is true, the returned value will end with a path
   * separator ('/' or '\') if found
   */
  extern stringT getenv(const char *env, bool is_path);
  extern stringT getusername(); // returns name of current user
  extern stringT gethostname(); // returns name of current machine
  extern stringT getprocessid();
};
#endif /* __ENV_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
