/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __RUN_H
#define __RUN_H

#include "typedefs.h"
#include "../corelib/StringX.h"

namespace pws_os {
  /**
   * runcmd executes the command
   * getruncmd return path to the command to be run based on Windows
   * Run command search rules or Linux equivalent
   */
  extern StringX getruncmd(const StringX first_part, bool &bfound);
  
  extern bool runcmd(const StringX execute_string);
};

#endif /* __GETRUNCMD_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
