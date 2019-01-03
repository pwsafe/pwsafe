/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "PWHistListCtrl.h"

#include "Fonts.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CPWHistListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CPWHistListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_MESSAGE(WM_SETFONT, OnSetFont)
  ON_WM_MEASUREITEM_REFLECT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWHistListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;

  static bool bchanged_subitem_font(false);
  static CFont *pAddEditFont = NULL;
  static CFont *pPasswordFont = NULL;
  static CDC *pDC = NULL;

  switch (pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      bchanged_subitem_font = false;
      pAddEditFont = Fonts::GetInstance()->GetAddEditFont();
      pPasswordFont = Fonts::GetInstance()->GetPasswordFont();
      pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      // Item PrePaint
      *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      // Sub-item PrePaint
      if (pLVCD->iSubItem == 1) {
        bchanged_subitem_font = true;
        pDC->SelectObject(pPasswordFont);
        *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
      }
      break;

    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
      // Sub-item PostPaint - restore old font if any
      if (bchanged_subitem_font) {
        bchanged_subitem_font = false;
        pDC->SelectObject(pAddEditFont);
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

void CPWHistListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
  if (!Fonts::GetInstance())
     return;
  
  int padding=4;
  if (GetExtendedStyle() & LVS_EX_GRIDLINES)
     padding+=2;
  
  lpMeasureItemStruct->itemHeight = Fonts::GetInstance()->CalcHeight()+padding;
  //Remove LVS_OWNERDRAWFIXED style to apply default DrawItem
  ModifyStyle(LVS_OWNERDRAWFIXED, 0);
}

void CPWHistListCtrl::UpdateRowHeight(bool bInvalidate){
  // We need to change WINDOWPOS to trigger MeasureItem 
  // http://www.codeproject.com/Articles/1401/Changing-Row-Height-in-an-owner-drawn-Control
  CRect rc;
  GetWindowRect(&rc);
  WINDOWPOS wp;
  wp.hwnd = m_hWnd;
  wp.cx = rc.Width();
  wp.cy = rc.Height();
  wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
  
  //Add LVS_OWNERDRAWFIXED style for generating MeasureItem event
  ModifyStyle(0, LVS_OWNERDRAWFIXED);

  SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);
  if (bInvalidate)
  {
    Invalidate();
    int idx = GetTopIndex();
    if (idx >=0)
      EnsureVisible(idx, FALSE);
  }
}

LRESULT CPWHistListCtrl::OnSetFont(WPARAM, LPARAM)
{
  LRESULT res = Default();
  UpdateRowHeight(false);
  return res;
}

void CPWHistListCtrl::DrawItem(LPDRAWITEMSTRUCT){
  //DrawItem must be overridden for LVS_OWNERDRAWFIXED style lists
}
