/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Expat library V2.0.1 released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
*
*/

#ifndef __ESECMEMMGR_H
#define __ESECMEMMGR_H

#include "../../stdafx.h"

class ESecMemMgr
{
public:
  void* malloc(size_t size);
  void* realloc(void *p, size_t size);
  void free(void *p);
};

#endif /* __ESECMEMMGR_H */

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
