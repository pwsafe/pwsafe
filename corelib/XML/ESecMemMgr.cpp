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

#include "XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

#include "ESecMemMgr.h"

#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

static const int header = max(sizeof(size_t), sizeof(size_t *));
static const int offset = max((int)(header / sizeof(size_t *)), 1);

void* ESecMemMgr::malloc(size_t size)
{
  // Get actual size to allocate
  size_t actual_size = size + header;
  // Allocate it
  void* preal_mem = ::malloc(actual_size);

  if (preal_mem != NULL) {
    // Put user size in header
    size_t *puser_mem = (size_t*)preal_mem;
    *puser_mem = size;
    // Get pointer to start of user memory and return it to user
    puser_mem += offset;
    //TRACE(_T("malloc:  preal = %p, puser = %p, size = %08d, actual size = %08d\n"),
    //  preal_mem, puser_mem, size, actual_size);
    return (void *)puser_mem;
  } else {
    //TRACE(_T("malloc:  preal = %p, puser =   N/A   , size = %08d, actual size = %08d\n"),
    //  preal_mem, size, actual_size);
    return NULL;
  }
}

void* ESecMemMgr::realloc(void *pold_mem, size_t new_size)
{
  if (pold_mem == NULL) {
    // Equivalent to allocate it
    //TRACE(_T("realloc: pold =  %p, puser =   N/A   , size =   N/A   , new size    = %08d\n"),
    //  pold_mem, new_size);
    return this->malloc(new_size);
  }

  if (new_size == 0) {
    // Equivalent to free it
    //TRACE(_T("realloc: pold =  %p, puser =   N/A   , size =   N/A   , new size    = %08d\n"),
    //  pold_mem, new_size);
    this->free(pold_mem);
    return NULL;
  }

  void* pnew_mem = this->malloc(new_size);

  if (pnew_mem != NULL) {
    // Back off to the start of the header
    size_t* preal_mem = (size_t*)pold_mem - offset;
    // Get the size of 'user' memory
    size_t old_size = (size_t)*preal_mem;
    // Get the smaller of old vs. new size
    size_t copy_size = min(old_size, new_size);
    // Copy only the smallest amount (no overruns)
    memcpy(pnew_mem, pold_mem, copy_size);
    // Now free old memory
    this->free(pold_mem);
    //TRACE(_T("realloc: pold =  %p, preal =   N/A   , size = %08d, new size    = %08d\n"),
    //  pold_mem, old_size, new_size);
    // Give the user the new memory
    return pnew_mem;
  } else {
    //TRACE(_T("realloc: pold =  %p, preal =   N/A   , size =   N/A   , new size    = %08d\n"),
    //  pold_mem, new_size);
    return NULL;
  }
}

void ESecMemMgr::free(void * puser_mem)
{
  if (puser_mem) {
    // Back off to the start of the header
    size_t* preal_mem = (size_t*)puser_mem - offset;
    // Get the size of 'user' memory
    size_t size = (size_t)*preal_mem;
    // Trash it!
    if (size > 0) {
      memset(puser_mem, 0x55, size);
      memset(puser_mem, 0xAA, size);
      memset(puser_mem, 0x00, size);
    }
    // Free it
    //TRACE(_T("free:    preal = %p, puser = %p, size = %08d\n"),
    //  preal_mem, puser_mem, size);
    ::free((void *)preal_mem);
  }
}

#endif /* USE_XML_LIBRARY == EXPAT */
