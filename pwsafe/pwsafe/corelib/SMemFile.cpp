/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#include "SMemFile.h"
#include "util.h"

BYTE* CSMemFile::Realloc(BYTE* lpOldMem, SIZE_T nBytes)
{
  if (nBytes == 0) {
    Free(lpOldMem);
    return NULL;
  }

  size_t old_size = _msize((void *)lpOldMem);
  BYTE* lpNewMem = (BYTE *)malloc(nBytes);

  if (lpNewMem == NULL) {
    TRACE("SMemfile:Realloc Old size=%d, New Size=%d FAILED\n", old_size, nBytes);
    return NULL;
  }

  memcpy((void *)lpNewMem, (void *)lpOldMem, old_size);
  trashMemory(lpOldMem, old_size);
  free(lpOldMem);

  TRACE("SMemfile:Realloc Old size=%d, New Size=%d\n", old_size, nBytes);
  return lpNewMem;
}

void CSMemFile::Free(BYTE* lpMem)
{
  size_t mem_size = _msize((void *)lpMem);
  TRACE("SMemfile:Free Memory at %p, Size=%d\n", lpMem, mem_size);
  if (lpMem == NULL)
    return;

  if (mem_size != 0)
    trashMemory(lpMem, mem_size);

  free(lpMem);
}
