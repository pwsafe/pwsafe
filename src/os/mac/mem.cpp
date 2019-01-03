/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of mem.h
 */

#include <sys/mman.h>
#include <cassert>
#include "../mem.h"

bool pws_os::mlock(void *p, size_t size)
{
  assert(p != NULL);
  return ::mlock(p, size) == 0;
}

bool pws_os::munlock(void *p, size_t size)
{
  assert(p != NULL);
  return ::munlock(p, size) == 0;
}

// Following has OS support only in Windows
bool pws_os::mcryptProtect(void *, size_t)
{
  return true;
}

bool pws_os::mcryptUnprotect(void *, size_t)
{
  return true;
}
