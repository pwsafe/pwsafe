/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "stdafx.h"

#include "resource.h"
#include "winutils.h"

#include "StateBitmapManager.h"

CStateBitmapManager::CStateBitmapManager(UINT nIdBitmapFirst, UINT nIdBitmapLast, UINT nIdBitmapError)
  :
  m_idFirst(nIdBitmapFirst),
  m_idLast(nIdBitmapLast),
  m_idError(nIdBitmapError)
{
  ASSERT(nIdBitmapFirst <= nIdBitmapLast);
  ASSERT(nIdBitmapError >= nIdBitmapFirst);
  ASSERT(nIdBitmapError <= nIdBitmapLast);

  // from https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
  int dpi = WinUtil::GetDPI(); // can't use ForWindow(m_Hwnd) as we don't have a valid one when this is called.

  for (UINT nId = m_idFirst; nId <= m_idLast; nId++) {
    CBitmap origBmp;
    origBmp.LoadBitmap(nId);
    BITMAP bm;
    origBmp.GetBitmap(&bm);
    int bmWidth = MulDiv(bm.bmWidth, dpi, 96);
    int bmHeight = MulDiv(bm.bmHeight, dpi, 96);
    UINT bmpIndex = nId - m_idFirst;
    m_stateBitmaps.push_back(new CBitmap);
    ASSERT(bmpIndex == m_stateBitmaps.size() - 1);
    WinUtil::ResizeBitmap(origBmp, *m_stateBitmaps[bmpIndex], bmWidth, bmHeight);
    origBmp.DeleteObject();
  }
}

CStateBitmapManager::~CStateBitmapManager()
{
  std::for_each(
    m_stateBitmaps.begin(),
    m_stateBitmaps.end(),
    [](CBitmap* pbmp) { pbmp->DeleteObject(); }
  );
  m_stateBitmaps.clear();
}

CBitmap& CStateBitmapManager::GetStateBitmap(UINT nIdBitmap)
{
  int nBitmapIndex = nIdBitmap - IDB_SCRCAP_FIRST;
  ASSERT(nBitmapIndex >= 0 && nBitmapIndex < m_stateBitmaps.size());
  if (nBitmapIndex < 0 || nBitmapIndex >= m_stateBitmaps.size())
    nBitmapIndex = m_idError - IDB_SCRCAP_FIRST;
  return *m_stateBitmaps[nBitmapIndex];
}

void CStateBitmapManager::GetBitmapInfo(UINT nIdBitmap, BITMAP* pBmpInfo, LONG* pBmWidthDpi, LONG* pBmHeightDpi)
{
  BITMAP bmpInfo;
  LONG bmWidthDpi;
  LONG bmHeightDpi;

  if (!pBmpInfo)
    pBmpInfo = &bmpInfo;
  if (!pBmWidthDpi)
    pBmWidthDpi = &bmWidthDpi;
  if (!pBmHeightDpi)
    pBmHeightDpi = &bmHeightDpi;

  CBitmap& stateBitmap = GetStateBitmap(nIdBitmap);
  stateBitmap.GetBitmap(pBmpInfo);

  // Constructor initialization already DPI-adjusted.
  *pBmWidthDpi = pBmpInfo->bmWidth;
  *pBmHeightDpi = pBmpInfo->bmHeight;
}

CBitmap& CStateBitmapManager::GetBitmapAndInfo(
  UINT nIdBitmap,
  BITMAP* pBmpInfo,
  LONG* pBmWidthDpi,
  LONG* pBmHeightDpi
)
{
  CBitmap& stateBitmap = GetStateBitmap(nIdBitmap);
  GetBitmapInfo(nIdBitmap, pBmpInfo, pBmWidthDpi, pBmHeightDpi);
  return stateBitmap;
}

void CStateBitmapManager::BitBltStateBitmap(UINT nIdBitmap, int xDest, int yDest, HDC hDC)
{
  BITMAP bmpInfo;
  LONG bmWidthDpi;
  LONG bmHeightDpi;
  CBitmap& stateBitmap = GetBitmapAndInfo(nIdBitmap, &bmpInfo, &bmWidthDpi, &bmHeightDpi);

  CDC srcDC;
  srcDC.CreateCompatibleDC(NULL);
  CBitmap* pOldBitmap = srcDC.SelectObject(&stateBitmap);
  CDC dc;
  dc.Attach(hDC);
  dc.BitBlt(xDest, yDest, bmWidthDpi, bmHeightDpi, &srcDC, 0, 0, SRCCOPY);
  dc.Detach();
  srcDC.SelectObject(pOldBitmap);
}
