/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#pragma once

#include "afxole.h"

class CDropSource : protected COleDataSource
{
public:
  CDropSource();
  virtual ~CDropSource();
  virtual DROPEFFECT StartDragging(LPCSTR szData, DWORD dwLength,
    CLIPFORMAT cpfmt, RECT* rClient, CPoint* ptMousePos);

protected:
  virtual void CompleteMove() {};

private:

};

