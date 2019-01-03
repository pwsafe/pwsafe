/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.1.1 released on April 27, 2010
*
* See http://xerces.apache.org/xerces-c/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
* To use the static version, the following pre-processor statement
* must be defined: XERCES_STATIC_LIBRARY
*
*/

#ifndef __XSECMEMMGR_H
#define __XSECMEMMGR_H

// Xerces includes
#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XSecMemMgr : public MemoryManager
{
  XSecMemMgr* getExceptionMemoryManager()
  {return this;}

#if XERCES_VERSION_MAJOR > 2
  void* allocate(XMLSize_t size);
#else
  void* allocate(size_t size);
#endif
  void deallocate(void * p);
};

XERCES_CPP_NAMESPACE_END

#endif /* __XSECMEMMGR_H */

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
