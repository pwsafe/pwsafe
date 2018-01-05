/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __DIR_H
#define __DIR_H

#include "typedefs.h"

namespace pws_os {
  extern stringT getexecdir(); // path of executable
  extern stringT getcwd();
  extern bool chdir(const stringT &dir);
  // In following, drive will be empty on non-Windows platforms
  extern bool splitpath(const stringT &path,
                        stringT &drive, stringT &dir,
                        stringT &file, stringT &ext);
  extern stringT makepath(const stringT &drive, const stringT &dir,
                          const stringT &file, const stringT &ext);
  extern stringT fullpath(const stringT &relpath);

  extern stringT getuserprefsdir(void);
  extern stringT getsafedir(void);
  extern stringT getxmldir(void);
  extern stringT gethelpdir(void);
}
#endif /* __DIR_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
