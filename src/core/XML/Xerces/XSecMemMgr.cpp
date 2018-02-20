/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == XERCES

#include "XSecMemMgr.h"

#include <string.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <iostream>
#include <fstream>
#include <cstring>  //for std::memset, on Linux
#include <algorithm>

using namespace std;

XERCES_CPP_NAMESPACE_BEGIN

static const XMLSize_t header = std::max(sizeof(XMLSize_t), sizeof(XMLSize_t *));
static const XMLSize_t offset = std::max(header / sizeof(XMLSize_t *), XMLSize_t(1));

#if XERCES_VERSION_MAJOR > 2
void* XSecMemMgr::allocate(XMLSize_t size)
#else
void* XSecMemMgr::allocate(size_t size)
#endif
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
  if (preal_mem != nullptr) {
    // Put user size in header
    XMLSize_t *puser_mem = reinterpret_cast<XMLSize_t*>(preal_mem);
    *puser_mem = size;
    // Get pointer to start of user memory and return it to user
    puser_mem += offset;
    return reinterpret_cast<void *>(puser_mem);
  }
  throw OutOfMemoryException();
}

void XSecMemMgr::deallocate(void * puser_mem)
{
  if (puser_mem) {
    // Back off to the start of the header
    XMLSize_t* preal_mem = reinterpret_cast<XMLSize_t*>(puser_mem) - offset;
    // Get the size of 'user' memory
    XMLSize_t size = *preal_mem;
    // Trash it!
    if (size > 0) {
      std::memset(puser_mem, 0x55, size);
      std::memset(puser_mem, 0xAA, size);
      std::memset(puser_mem,    0, size);
    }
    ::operator delete(reinterpret_cast<void *>(preal_mem));
  }
}

XERCES_CPP_NAMESPACE_END

#endif /* USE_XML_LIBRARY == XERCES */
