/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __LIB_H
#define __LIB_H

#include "typedefs.h"

namespace pws_os {
  /**
   * Windows:
   * LOAD_LIBRARY_SYS -- load from system dir
   * LOAD_LIBRARY_APP -- load from application dir
   * LOAD_LIBRARY_CUSTOM -- use specified path (ask system to find it)
   *
   * Linux: 'type' maps to 'flags' for dlopen()
   */
   enum loadLibraryTypes { LOAD_LIBRARY_SYS, LOAD_LIBRARY_APP, LOAD_LIBRARY_CUSTOM };
   extern void *LoadLibrary(const TCHAR *lib, int type);
   extern void *GetFunction(void *handle, const char *name);
   extern bool FreeLibrary(void *handle);
}
#endif /* __LIB_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
