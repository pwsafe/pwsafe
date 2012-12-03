/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __LIB_H
#define __LIB_H

#include "typedefs.h"
#ifdef _WIN32
#include <wtypes.h>
#endif

namespace pws_os {
   //LOAD_LIBRARY_SYS -- load form system dir
   //LOAD_LIBRARY_APP -- load from application dir
   //LOAD_LIBRARY_CUSTOM -- use specified path (ask system to find it)
   enum loadLibraryTypes { LOAD_LIBRARY_SYS, LOAD_LIBRARY_APP, LOAD_LIBRARY_CUSTOM };
#ifdef _WIN32
   extern HMODULE LoadLibraryPWS(const TCHAR *lib, loadLibraryTypes type);
#endif
}
#endif /* __LIB_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
