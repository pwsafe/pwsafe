/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.0.0 released on September 29, 2008
*
* See http://xerces.apache.org/xerces-c/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
* To use the static version, the following pre-processor statement
* must be defined: XERCES_STATIC_LIBRARY
*
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

#include "XSecMemMgr.h"

#include <string.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <iostream>
#include <fstream>

using namespace std;

XERCES_CPP_NAMESPACE_BEGIN

static const int header = max(sizeof(XMLSize_t), sizeof(XMLSize_t *));
static const int offset = max((int)(header / sizeof(XMLSize_t *)), 1);

void* XSecMemMgr::allocate(XMLSize_t size)
{
  // Get actual size to allocate
  XMLSize_t actual_size = size + header;
  void* preal_mem;
  try {
  // Allocate it
    preal_mem = ::operator new(actual_size);
  }
  catch(...) {
    throw OutOfMemoryException();
  }
  if (preal_mem != NULL) {
    // Put user size in header
    XMLSize_t *puser_mem = (XMLSize_t*)preal_mem;
    *puser_mem = size;
    // Get pointer to start of user memory and return it to user
    puser_mem += offset;
    return (void *)puser_mem;
  }
  throw OutOfMemoryException();
}

void XSecMemMgr::deallocate(void * puser_mem)
{
  if (puser_mem) {
    // Back off to the start of the header
    XMLSize_t* preal_mem = (XMLSize_t*)puser_mem - offset;
    // Get the size of 'user' memory
    XMLSize_t size = (XMLSize_t)*preal_mem;
    // Trash it!
    if (size > 0) {
      memset(puser_mem, 0x55, size);
      memset(puser_mem, 0xAA, size);
      memset(puser_mem, 0x00, size);
    }
    ::operator delete((void *)preal_mem);
  }
}

XERCES_CPP_NAMESPACE_END

#endif /* USE_XML_LIBRARY == XERCES */
