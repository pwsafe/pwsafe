/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#include "stdafx.h"
#include "afxole.h"
#include "DropSource.h"

CDropSource::CDropSource()
{
}

CDropSource::~CDropSource()
{
}

DROPEFFECT CDropSource::StartDragging(LPCSTR szData, DWORD dwLength, CLIPFORMAT cpfmt,
                  RECT *rClient, CPoint *ptMousePos)
{
  HGLOBAL hgData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLength);
  ASSERT(hgData != NULL);

  LPCSTR lpData = (LPCSTR)GlobalLock(hgData);
  ASSERT(lpData != NULL);

  memcpy((void *)lpData, szData, dwLength);

  CacheGlobalData(cpfmt, hgData);

  DROPEFFECT dropEffect = DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE, (LPCRECT)rClient);

  if ((dropEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
     CompleteMove();

  LPARAM lparam;

  lparam = ptMousePos->y;
  lparam = lparam << 16;
  lparam &= ptMousePos->x;

  SendMessage(GetActiveWindow(), WM_LBUTTONUP, 0, lparam);

  Empty();

  //InternalRelease();

  return dropEffect;
}
