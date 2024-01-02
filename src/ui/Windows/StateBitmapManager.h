/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include <vector>
#include <memory>

class CStateBitmapManager
{
public:
  static const UINT RGB_COLOR_NOT_TRANSPARENT = ((UINT)-1);
public:
  CStateBitmapManager(
    UINT nIdBitmapFirst,
    UINT nIdBitmapLast,
    UINT nIdBitmapError,
    UINT rgbTransparentColor = CStateBitmapManager::RGB_COLOR_NOT_TRANSPARENT
  );
  CBitmap& GetStateBitmap(UINT nIdBitmap);
  void GetBitmapInfo(UINT nIdBitmap, BITMAP* pBmpInfo = nullptr, LONG* pBmWidthDpi = nullptr, LONG* pBmHeightDpi = nullptr);
  CBitmap& GetBitmapAndInfo(UINT nIdBitmap, BITMAP* pBmpInfo = nullptr, LONG* pBmWidthDpi = nullptr, LONG* pBmHeightDpi = nullptr);
  void BitBltStateBitmap(UINT nIdBitmap, int xDest, int yDest, HDC hDC);
  UINT GetFirstId() const { return m_idFirst; }
  UINT GetLastId() const { return m_idLast; }
private:
  UINT m_rgbTransparentColor;
  UINT m_idFirst;
  UINT m_idLast;
  UINT m_idError;
  std::vector<std::shared_ptr<CBitmap>> m_stateBitmaps;
};
