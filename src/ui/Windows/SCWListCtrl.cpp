/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "SCWListCtrl.h"
#include "Fonts.h"
#include "ShowCompareDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSCWListCtrl::CSCWListCtrl()
  : m_nHoverNDTimerID(0), m_nShowNDTimerID(0), m_bMouseInWindow(false),
  m_pParent(NULL)
{
}

CSCWListCtrl::~CSCWListCtrl()
{
}

void CSCWListCtrl::Initialize()
{
  m_pParent = reinterpret_cast<CShowCompareDlg *>(GetParent());
  UpdateRowHeight(false);
}

BEGIN_MESSAGE_MAP(CSCWListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CSCWListCtrl)
    ON_WM_MOUSEMOVE()
  ON_WM_TIMER()
  ON_WM_MEASUREITEM_REFLECT()
  
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_MESSAGE(WM_SETFONT, OnSetFont)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSCWListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;

  static bool bchanged_subitem_font(false);
  static CDC *pDC = NULL;
  static COLORREF crWindowText;
  static CFont *pAddEditFont = NULL;
  static CFont *pPasswordFont = NULL;

  switch (pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      crWindowText = GetTextColor();
      pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
      pAddEditFont = Fonts::GetInstance()->GetAddEditFont();
      pPasswordFont = Fonts::GetInstance()->GetPasswordFont();
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      // Item PrePaint
      *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      // Sub-item PrePaint
      if (pLVCD->iSubItem == 0) {
        CRect rect;
        GetSubItemRect((int)pLVCD->nmcd.dwItemSpec, pLVCD->iSubItem, LVIR_BOUNDS, rect);
        if (rect.top < 0) {
          *pLResult = CDRF_SKIPDEFAULT;
          break;
        }
        CRect rect1;
        GetSubItemRect((int)pLVCD->nmcd.dwItemSpec, 1, LVIR_BOUNDS, rect1);
        rect.right = rect1.left;
        rect.DeflateRect(2, 2);

        CString str = GetItemText((int)pLVCD->nmcd.dwItemSpec, pLVCD->iSubItem);
        pLVCD->clrText = ((pLVCD->nmcd.lItemlParam & REDTEXT) == REDTEXT) ?
                          RGB(255, 0, 0) : crWindowText;

        int iFormat = (pLVCD->nmcd.lItemlParam & 0x0F);
        UINT nFormat = DT_VCENTER | DT_SINGLELINE;
        if (iFormat == LVCFMT_RIGHT)
          nFormat |= DT_RIGHT;
        else if (iFormat == LVCFMT_CENTER)
          nFormat |= DT_CENTER;
        pDC->DrawText(str, &rect, nFormat);
        pDC->SelectObject(pAddEditFont);
      } else {
        // For Password values
        if ((pLVCD->nmcd.lItemlParam & PASSWORDFONT) == PASSWORDFONT) {
          bchanged_subitem_font = true;
          pDC->SelectObject(pPasswordFont);
        }
        pLVCD->clrText = crWindowText;
      }
      
      bchanged_subitem_font = true;
      *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
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
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
    case CDDS_POSTPAINT:
    */
    default:
      break;
  }
}

void CSCWListCtrl::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_ND_HOVER:
      KillTimer(m_nHoverNDTimerID);
      m_nHoverNDTimerID = 0;
      if (m_pParent->SetNotesWindow(m_HoverNDPoint)) {
        if (m_nShowNDTimerID) {
          KillTimer(m_nShowNDTimerID);
          m_nShowNDTimerID = 0;
        }
        m_nShowNDTimerID = SetTimer(TIMER_ND_SHOWING, TIMEINT_ND_SHOWING, NULL);
      }
      break;
    case TIMER_ND_SHOWING:
      KillTimer(m_nShowNDTimerID);
      m_nShowNDTimerID = 0;
      m_HoverNDPoint = CPoint(0, 0);
      m_pParent->SetNotesWindow(m_HoverNDPoint, false);
      break;
    default:
      CListCtrl::OnTimer(nIDEvent);
      break;
  }
}

LRESULT CSCWListCtrl::OnMouseLeave(WPARAM, LPARAM)
{
  KillTimer(m_nHoverNDTimerID);
  KillTimer(m_nShowNDTimerID);
  m_nHoverNDTimerID = m_nShowNDTimerID = 0;
  m_HoverNDPoint = CPoint(0, 0);
  m_pParent->SetNotesWindow(m_HoverNDPoint, false);
  m_bMouseInWindow = false;
  return 0L;
}

void CSCWListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
  if (m_nHoverNDTimerID) {
    if (HitTest(m_HoverNDPoint) == HitTest(point))
      return;
    KillTimer(m_nHoverNDTimerID);
    m_nHoverNDTimerID = 0;
  }

  if (m_nShowNDTimerID) {
    if (HitTest(m_HoverNDPoint) == HitTest(point))
      return;
    KillTimer(m_nShowNDTimerID);
    m_nShowNDTimerID = 0;
    m_pParent->SetNotesWindow(CPoint(0, 0), false);
  }

  if (!m_bMouseInWindow) {
    m_bMouseInWindow = true;
    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0};
    VERIFY(TrackMouseEvent(&tme));
  }

  m_nHoverNDTimerID = SetTimer(TIMER_ND_HOVER, HOVER_TIME_ND, NULL);
  m_HoverNDPoint = point;

  CListCtrl::OnMouseMove(nFlags, point);
}

void CSCWListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
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

void CSCWListCtrl::UpdateRowHeight(bool bInvalidate){
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
  if (bInvalidate) {
    Invalidate();
    int idx = GetTopIndex();
    if (idx >= 0)
      EnsureVisible(idx, FALSE);
  }
}

LRESULT CSCWListCtrl::OnSetFont(WPARAM, LPARAM)
{
  LRESULT res = Default();
  UpdateRowHeight(false);
  return res;
}

void CSCWListCtrl::DrawItem(LPDRAWITEMSTRUCT){
  //DrawItem must be overridden for LVS_OWNERDRAWFIXED style lists
}
