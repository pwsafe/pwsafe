/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "afxole.h"
#include "DropSource.h"

CDataSource::CDataSource()
{
}

CDataSource::~CDataSource()
{
}

DROPEFFECT CDataSource::StartDragging(BYTE *szData, DWORD dwLength, CLIPFORMAT cpfmt,
                                      RECT *rClient, CPoint *ptMousePos)
{
  HGLOBAL hgData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLength);
  ASSERT(hgData != NULL);

  LPCSTR lpData = (LPCSTR)GlobalLock(hgData);
  ASSERT(lpData != NULL);

  memcpy((void *)lpData, szData, dwLength);
  CacheGlobalData(cpfmt, hgData);

  DROPEFFECT dropEffect = DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE,
    (LPCRECT)rClient);

  if ((dropEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
    CompleteMove();

  LPARAM lparam = (LPARAM(ptMousePos->y) << 16) | LPARAM(ptMousePos->x);
  SendMessage(GetActiveWindow(), WM_LBUTTONUP, 0, lparam);

  Empty();

  return dropEffect;
}
