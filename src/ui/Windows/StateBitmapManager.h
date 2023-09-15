/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include <vector>

class CStateBitmapManager
{
public:
  CStateBitmapManager(UINT nIdBitmapFirst, UINT nIdBitmapLast, UINT nIdBitmapError);
  virtual ~CStateBitmapManager();
  CBitmap& GetStateBitmap(UINT nIdBitmap);
  void GetBitmapInfo(UINT nIdBitmap, BITMAP* pBmpInfo = nullptr, LONG* pBmWidthDpi = nullptr, LONG* pBmHeightDpi = nullptr);
  CBitmap& GetBitmapAndInfo(UINT nIdBitmap, BITMAP* pBmpInfo = nullptr, LONG* pBmWidthDpi = nullptr, LONG* pBmHeightDpi = nullptr);
  void BitBltStateBitmap(UINT nIdBitmap, int xDest, int yDest, HDC hDC);
private:
  UINT m_idFirst;
  UINT m_idLast;
  UINT m_idError;
  std::vector<CBitmap*> m_stateBitmaps;
};
