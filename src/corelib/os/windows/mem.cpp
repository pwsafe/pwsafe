/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of mem.h
 */

#include <Windows.h>
#include "../typedefs.h"
#include "../mem.h"

bool pws_os::mlock(void *p, size_t size)
{
#ifndef POCKET_PC
  return VirtualLock(p, size) != 0;
#else
  return true;
#endif
}

bool pws_os::munlock(void *p, size_t size)
{
#ifndef POCKET_PC
  return VirtualUnlock(p, size) != 0;
#else
  return true;
#endif
}

