/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// GEListCtrl.cpp : implementation file
//

#include "stdafx.h"

#include "FRListCtrl.h"
#include "FindReplaceDlg.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFRListCtrl

CFRListCtrl::CFRListCtrl()
  : bInitdone(false), m_pGEDlg(NULL), m_pCheckImageList(NULL)
{
  m_clrDisabled = GetSysColor(COLOR_GRAYTEXT);
}

CFRListCtrl::~CFRListCtrl()
{
  if (m_pCheckImageList) {
    m_pCheckImageList->DeleteImageList();
  }
}

BEGIN_MESSAGE_MAP(CFRListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CFRListCtrl)
  ON_WM_DESTROY()
  ON_WM_MEASUREITEM_REFLECT()

  ON_MESSAGE(WM_SETFONT, OnSetFont)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFRListCtrl message handlers

void CFRListCtrl::OnDestroy()
{
  // Stop subclassing the ListCtrl HeaderCtrl
  if (m_CheckHeadCtrl.GetSafeHwnd() != NULL)
    m_CheckHeadCtrl.UnsubclassWindow();

  if (m_pCheckImageList) {
    m_pCheckImageList->DeleteImageList();
    delete m_pCheckImageList;
    m_pCheckImageList = NULL;
  }

  CListCtrl::OnDestroy();
}

void CFRListCtrl::Init(CFindReplaceDlg *pGERdlg)
{
  if (bInitdone)
    return;

  // Tell CListctrl and its CHeaderCtrl of the parent dialog
  m_pGEDlg = pGERdlg;
  m_CheckHeadCtrl.SetParent(m_pGEDlg);

  // Subclass CListctrl CHeaderCtrl
  CHeaderCtrl *pHeadCtrl = this->GetHeaderCtrl();
  VERIFY(m_CheckHeadCtrl.SubclassWindow(pHeadCtrl->GetSafeHwnd()));

  // Set users Current font
  m_pModifiedFont = Fonts::GetInstance()->GetModifiedFont();
  m_pCurrentFont = Fonts::GetInstance()->GetCurrentFont();
  SetFont(m_pCurrentFont);
  m_CheckHeadCtrl.SetFont(m_pCurrentFont);

  // Load all images into list GERSTate FR_UNCHECKED, FR_CHECKED, FR_CHANGED & FR_PROTECTED
  m_pCheckImageList = new CImageList;
  VERIFY(m_pCheckImageList->Create(IDB_FINDREPLACEIMAGES, 16, 4, RGB(255, 0, 255)));

  // Set our images
  SetImageList(m_pCheckImageList, LVSIL_SMALL);

  // Set CHeaderCtrl images
  m_CheckHeadCtrl.SetImageList(m_pCheckImageList, HDSIL_NORMAL);

  // Add image to first column
  HDITEM hdi = { 0 };
  hdi.mask = HDI_IMAGE | HDI_FORMAT | HDI_WIDTH;
  m_CheckHeadCtrl.GetItem(0, &hdi);

  hdi.fmt |= HDF_IMAGE | HDF_FIXEDWIDTH | HDF_CENTER;
  hdi.iImage = 0;
  hdi.cxy = 20;
  m_CheckHeadCtrl.SetItem(0, &hdi);

  SetHeaderImage(FR_UNCHECKED);

  bInitdone = true;

  return;
}

void CFRListCtrl::SetHeaderImage(FRState state)
{
  HDITEM hdi = { 0 };
  hdi.mask = HDI_FORMAT | HDI_IMAGE;
  VERIFY(m_CheckHeadCtrl.GetItem(0, &hdi));

  hdi.fmt = HDF_CENTER | HDF_IMAGE;
  hdi.iImage = (int)state;

  VERIFY(m_CheckHeadCtrl.SetItem(0, &hdi));

  SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

  m_CheckHeadCtrl.SetState(state);
}

void CFRListCtrl::SetHeaderSortArrows(const int &iSortedColumn,  const bool &bSortAscending)
{
  // First column click is not sort but select/unselect all
  if (iSortedColumn == 0)
    return;

  // Turn off existing arrows (if any)
  HDITEM hdi1 = { 0 };
  hdi1.mask = HDI_FORMAT;
  for (int i = 0; i < m_CheckHeadCtrl.GetItemCount(); i++) {
    m_CheckHeadCtrl.GetItem(i, &hdi1);
    if ((hdi1.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
      hdi1.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
      m_CheckHeadCtrl.SetItem(i, &hdi1);
    }
  }
  
  HDITEM hdi2 = { 0 };
  hdi2.mask = HDI_FORMAT;
  m_CheckHeadCtrl.GetItem(iSortedColumn, &hdi2);

  // Turn off all arrows
  hdi2.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi2.fmt |= (HDI_FORMAT | (bSortAscending ? HDF_SORTUP : HDF_SORTDOWN));

  m_CheckHeadCtrl.SetItem(iSortedColumn, &hdi2);
}

void CFRListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;
  const int iItem = (int)pLVCD->nmcd.dwItemSpec;
  const int iSubItem = pLVCD->iSubItem;
  const DWORD_PTR dwData = pLVCD->nmcd.lItemlParam;

  static bool bchanged_subitem_font(false);
  static CDC *pDC = NULL;

  const FRState result_state = m_pGEDlg->GetResultState((int)pLVCD->nmcd.lItemlParam);

  if (result_state == FR_INVALID)
    return;

  switch (pLVCD->nmcd.dwDrawStage) {
  case CDDS_PREPAINT:
    bchanged_subitem_font = false;
    pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
    *pLResult = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    *pLResult = CDRF_NOTIFYSUBITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
  {
    CRect rect;
    GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rect);
    if (rect.top < 0) {
      *pLResult = CDRF_SKIPDEFAULT;
      break;
    }

    if (iSubItem != 0) {
      if (result_state == FR_CHANGED) {
        // Disable text
        pLVCD->clrText = m_clrDisabled;
      }

      if (result_state == FR_PROTECTED) {
        // Disable text and make italic
        pLVCD->clrText = m_clrDisabled;
        bchanged_subitem_font = true;
        pDC->SelectObject(m_pModifiedFont);
        *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
      }
      break;
    }

    CRect rect1;
    GetSubItemRect(iItem, 1, LVIR_BOUNDS, rect1);
    rect.right = rect1.left;

    // Overpaint any ghost images
    pDC->FillSolidRect(&rect, GetBkColor());

    CRect inner_rect(rect), first_rect(rect);
    inner_rect.DeflateRect(2, 2);

    // Draw checked/unchecked/disabled image
    int ix = inner_rect.CenterPoint().x;
    int iy = inner_rect.CenterPoint().y;

    // The '7' below is ~ half the bitmap size of 13.
    inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
    DrawImage(pDC, inner_rect, result_state);
    *pLResult = CDRF_SKIPDEFAULT;
    break;
  }

  case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
    // Sub-item PostPaint - restore old font if any
    if (bchanged_subitem_font) {
      bchanged_subitem_font = false;
      pDC->SelectObject(m_pCurrentFont);
      *pLResult |= CDRF_NEWFONT;
    }
    break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    case CDDS_ITEMPOSTPAINT:
    case CDDS_POSTPAINT:
    */
  default:
    break;
  }
}

void CFRListCtrl::DrawImage(CDC *pDC, CRect &rect, FRState nImage)
{
  // Draw check image in given rectangle
  if (rect.IsRectEmpty() || nImage < 0) {
    return;
  }

  if (m_pCheckImageList) {
    SIZE sizeImage = { 0, 0 };
    IMAGEINFO info;

    if (m_pCheckImageList->GetImageInfo(nImage, &info)) {
      sizeImage.cx = info.rcImage.right - info.rcImage.left;
      sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
    }

    if (nImage >= 0) {
      if (rect.Width() > 0) {
        POINT point;

        point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
        point.x = rect.left;

        SIZE size;
        size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
        size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;

        m_pCheckImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
      }
    }
  }
}

void CFRListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
  int padding = 4;
  if (GetExtendedStyle() & LVS_EX_GRIDLINES)
    padding += 2;

  lpMeasureItemStruct->itemHeight = Fonts::GetInstance()->CalcHeight(false) + padding;

  if (lpMeasureItemStruct->itemHeight < 20)
    lpMeasureItemStruct->itemHeight = 20;

  // Remove LVS_OWNERDRAWFIXED style to apply default DrawItem
  ModifyStyle(LVS_OWNERDRAWFIXED, 0);
}

void CFRListCtrl::UpdateRowHeight(bool bInvalidate)
{
  // We need to change WINDOWPOS to trigger MeasureItem 
  // http://www.codeproject.com/Articles/1401/Changing-Row-Height-in-an-owner-drawn-Control
  CRect rc = { 0 };
  GetWindowRect(&rc);

  WINDOWPOS wp = { 0 };
  wp.hwnd = m_hWnd;
  wp.cx = rc.Width();
  wp.cy = rc.Height();
  wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;

  // Add LVS_OWNERDRAWFIXED style for generating MeasureItem event
  ModifyStyle(0, LVS_OWNERDRAWFIXED);

  SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);

  if (bInvalidate) {
    Invalidate();
    int idx = GetTopIndex();
    if (idx >= 0)
      EnsureVisible(idx, FALSE);
  }
}

LRESULT CFRListCtrl::OnSetFont(WPARAM, LPARAM)
{
  LRESULT res = Default();
  UpdateRowHeight(false);
  return res;
}

void CFRListCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
  // DrawItem must be overridden for LVS_OWNERDRAWFIXED style lists
}
