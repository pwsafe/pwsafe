/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "afx.h"

/////////////////////////////////////////////////////////////////////////////
// CSMemFile - Secure-ish Memory File

class CSMemFile : public CMemFile
{
  // Construction
public:
  CSMemFile(UINT nGrowBytes = 1024) : CMemFile(nGrowBytes), m_size(0) {}

  CSMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0)
    : CMemFile(lpBuffer, nBufferSize, nGrowBytes), m_size(0) {}

  // Implementation
public:
  virtual BYTE* Alloc(SIZE_T nBytes);
  virtual BYTE* Realloc(BYTE* lpMem, SIZE_T nBytes);
  virtual void Free(BYTE * lpMem);

private:
  size_t m_size;
};
