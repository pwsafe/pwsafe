/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#pragma once

#include "afx.h"

/////////////////////////////////////////////////////////////////////////////
// CSMemFile - Secure-ish Memory File

class CSMemFile : public CMemFile
{
// Construction
public:

// Implementation
public:

  virtual BYTE* Realloc(BYTE* lpMem, SIZE_T nBytes);
  virtual void Free(BYTE * lpMem);
};
