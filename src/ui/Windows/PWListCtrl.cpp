/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "PWListCtrl.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPWListCtrlX::CPWListCtrlX()
  : m_FindTimerID(0), m_csFind(L""), m_bMouseInWindow(false), 
  m_nHoverNDTimerID(0), m_nShowNDTimerID(0), m_bListFilterActive(false),
  m_bUseHighLighting(false)
{
}

CPWListCtrlX::~CPWListCtrlX()
{
}

BEGIN_MESSAGE_MAP(CPWListCtrlX, CListCtrl)
  //{{AFX_MSG_MAP(CPWListCtrlX)
  ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, OnItemChanging)
  ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnSelectionChanged)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_MESSAGE(WM_CHAR, OnCharItemlist)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_MOUSEMOVE()
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_VSCROLL()
  ON_MESSAGE(WM_SETFONT, OnSetFont)
  ON_WM_MEASUREITEM_REFLECT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWListCtrlX::Initialize()
{
  UpdateRowHeight(false);
}

void CPWListCtrlX::ActivateND(const bool bActivate)
{
  m_bShowNotes = bActivate;
  if (!m_bShowNotes) {
    m_bMouseInWindow = false;
  }
}

bool CPWListCtrlX::IsNotesColumnPresent()
{
  CHeaderCtrl *pHeader = GetHeaderCtrl();
  if (pHeader == NULL)
    return false;

  HDITEM hdi;
  hdi.mask = HDI_LPARAM;
  
  for (int icol = 0; icol < pHeader->GetItemCount(); icol++) {
    pHeader->GetItem(icol, &hdi);
    if (hdi.lParam == CItemData::NOTES)
      return true;    
  }
  return false;
}

void CPWListCtrlX::SetUpFont()
{
  Fonts::GetInstance()->SetUpFont(this, Fonts::GetInstance()->GetTreeListFont());
}

LRESULT CPWListCtrlX::OnCharItemlist(WPARAM wParam, LPARAM /* lParam */)
{
  const int iSubItem = app.GetMainDlg()->IsImageVisible() ? 1 : 0;
  bool bFirst;

  if (m_FindTimerID != 0) {
    KillTimer(TIMER_FIND);
    m_csFind += (wchar_t)wParam;
    bFirst = false;
  } else {
    m_csFind = (wchar_t)wParam;
    bFirst = true;
  }

  if (!FindNext(m_csFind, iSubItem) && !bFirst) {
    // Didn't find a match when more than one character
    // Emulate CListCtrl and try again (once) with this matching the first character
    m_csFind = (wchar_t)wParam;
    FindNext(m_csFind, iSubItem);
  }

  // Set timer going again
  m_FindTimerID = SetTimer(TIMER_FIND, 1000, NULL);
  return 0L;
}

void CPWListCtrlX::OnDestroy()
{
  // Remove dummy ImageList. PWTreeCtrl removes the real one!
  app.GetMainDlg()->m_pImageList0->DeleteImageList();
  delete app.GetMainDlg()->m_pImageList0;
}

void CPWListCtrlX::OnPaint()
{
  CListCtrl::OnPaint();

  app.GetMainDlg()->SaveGUIStatusEx(DboxMain::LISTONLY);
}

void CPWListCtrlX::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

  app.GetMainDlg()->SaveGUIStatusEx(DboxMain::LISTONLY);
}

BOOL CPWListCtrlX::PreTranslateMessage(MSG *pMsg)
{
  // Process User's AutoType shortcut
  if (app.GetMainDlg()->CheckPreTranslateAutoType(pMsg))
    return TRUE;

  // Process User's Delete shortcut
  if (app.GetMainDlg()->CheckPreTranslateDelete(pMsg))
    return TRUE;

  // Let the parent class do its thing
  return CListCtrl::PreTranslateMessage(pMsg);
}

void CPWListCtrlX::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_FIND:
      KillTimer(TIMER_FIND);
      m_FindTimerID = 0;
      break;
    case TIMER_ND_HOVER:
      KillTimer(m_nHoverNDTimerID);
      m_nHoverNDTimerID = 0;
      if (app.GetMainDlg()->SetNotesWindow(m_HoverNDPoint)) {
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
      app.GetMainDlg()->SetNotesWindow(m_HoverNDPoint, false);
      break;
    default:
      CListCtrl::OnTimer(nIDEvent);
      break;
  }
}

void CPWListCtrlX::OnMouseMove(UINT nFlags, CPoint point)
{
  app.GetMainDlg()->ResetIdleLockCounter();
  if (!m_bShowNotes)
    return;

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
    app.GetMainDlg()->SetNotesWindow(CPoint(0, 0), false);
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

LRESULT CPWListCtrlX::OnMouseLeave(WPARAM, LPARAM)
{
  KillTimer(m_nHoverNDTimerID);
  KillTimer(m_nShowNDTimerID);
  m_nHoverNDTimerID = m_nShowNDTimerID = 0;
  m_HoverNDPoint = CPoint(0, 0);
  app.GetMainDlg()->SetNotesWindow(m_HoverNDPoint, false);
  m_bMouseInWindow = false;
  return 0L;
}

bool CPWListCtrlX::FindNext(const CString &cs_find, const int iSubItem)
{
  int iItem;
  bool bFound(false);
  CString cs_text;
  const int iNum = GetItemCount();
  const int iFindLen = cs_find.GetLength();

  // Get selected item, if any
  POSITION pos = GetFirstSelectedItemPosition();

  // First search down.
  if (pos == NULL)
    iItem = 0;
  else
    iItem = (int)(INT_PTR)pos;

  do {
    cs_text = GetItemText(iItem, iSubItem);
    cs_text = cs_text.Mid(0, iFindLen);
    if (cs_text.GetLength() > 0 && cs_find.CompareNoCase(cs_text) == 0) {
      bFound = true;
      break;
    }
    iItem++;
  } while (iItem <= iNum);

  // Not found searching down and we didn't start from the top, now start from the top until
  // we get to where we started!
  if (!bFound && pos != NULL) {
    iItem = 0;
    do {
      cs_text = GetItemText(iItem, iSubItem);
      cs_text = cs_text.Mid(0, iFindLen);
      if (cs_text.GetLength() > 0 && cs_find.CompareNoCase(cs_text) == 0) {
        bFound = true;
        break;
      }
      iItem++;
    } while (iItem != (INT_PTR)pos);
  }

  if (bFound) {
    SetItemState(iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    EnsureVisible(iItem, FALSE);
    Invalidate();
    CItemData *pci = (CItemData *)GetItemData(iItem);
    app.GetMainDlg()->UpdateToolBarForSelectedItem(pci);
  }

  return bFound;
}

void CPWListCtrlX::SetFilterState(bool bState)
{
  m_bListFilterActive = bState;

  // Red if filter active, black if not
  SetTextColor(m_bListFilterActive ? RGB(168, 0, 0) : RGB(0, 0, 0));
}

BOOL CPWListCtrlX::OnEraseBkgnd(CDC* pDC)
{
  if (m_bListFilterActive && app.GetMainDlg()->GetNumPassedFiltering() == 0) {
    int nSavedDC = pDC->SaveDC(); //save the current DC state

    // Set up variables
    COLORREF clrText = RGB(168, 0, 0);
    COLORREF clrBack = ::GetSysColor(COLOR_WINDOW);    //system background color
    CBrush cbBack(clrBack);

    CRect rc;
    GetClientRect(&rc);  //get client area of the ListCtrl

    // If there is a header, we need to account for the space it occupies
    CHeaderCtrl* pHC = GetHeaderCtrl();
    if (pHC != NULL) {
      CRect rcH;
      pHC->GetClientRect(&rcH);
      rc.top += rcH.bottom;
    }

    // Here is the string we want to display (or you can use a StringTable entry)
    const CString cs_emptytext(MAKEINTRESOURCE(IDS_NOITEMSPASSEDFILTERING));

    // Now we actually display the text
    // set the text color
    pDC->SetTextColor(clrText);
    // set the background color
    pDC->SetBkColor(clrBack);
    // fill the client area rect
    pDC->FillRect(&rc, &cbBack);
    // select a font
    pDC->SelectStockObject(ANSI_VAR_FONT);
    // and draw the text
    pDC->DrawText(cs_emptytext, -1, rc,
                  DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);

    // Restore dc
    pDC->RestoreDC(nSavedDC);
    ReleaseDC(pDC);
  } else {
    //  If there are items in the ListCtrl, we need to call the base class function
    CListCtrl::OnEraseBkgnd(pDC);
  }

  return TRUE;
}

void CPWListCtrlX::OnItemChanging(NMHDR *pNMHDR, LRESULT *pLResult)
{
  *pLResult = FALSE;  // Allow change

  NMLISTVIEW *pNMLV = reinterpret_cast<NMLISTVIEW *>(pNMHDR);
  
  // Check if state unchanged - unchanged == not interested
  if ((pNMLV->uChanged & LVIF_STATE) != LVIF_STATE)
    return;

  // Has the selected state changed?  Only care if not selected and now is
  if (!((pNMLV->uOldState & LVIS_SELECTED) == 0 &&
        (pNMLV->uNewState & LVIS_SELECTED) != 0)) {
    return;
  }

  if ((GetKeyState(VK_CONTROL) & 0x8000) && GetSelectedCount() == 2) {
    // Control key pressed - multi-select but limited to 2
    *pLResult = TRUE; // Deny change - no more than 2 allowed to be selected
    return;
  }
}

void CPWListCtrlX::OnSelectionChanged(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  if (GetItemCount() == 0)
    return;

  LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNotifyStruct);

  switch(pLVKeyDown->wVKey) {
    case VK_UP:
    case VK_DOWN:
      app.GetMainDlg()->OnItemSelected(pNotifyStruct, pLResult, false);
      break;
    default:
      break;
  }
}

CFont *CPWListCtrlX::GetFontBasedOnStatus(CItemData *pci, COLORREF &cf)
{
  if (pci == NULL)
    return NULL;

  Fonts *pFonts = Fonts::GetInstance();
  switch (pci->GetStatus()) {
    case CItemData::ES_ADDED:
    case CItemData::ES_MODIFIED:
      cf = pFonts->GetModified_Color();
      return pFonts->GetItalicTreeListFont();
    default:
      break;
  }
  return NULL;
}

void CPWListCtrlX::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;

  static bool bitem_selected(false);
  static bool bchanged_item_font(false), bchanged_subitem_font(false);
  static CFont *pAddEditFont = NULL;
  static CFont *pPasswordFont = NULL;
  static CFont *pNotesFont = NULL;
  static CDC *pDC = NULL;

  HDITEM hdi = {0};
  hdi.mask = HDI_LPARAM;

  CItemData *pci = (CItemData *)pLVCD->nmcd.lItemlParam;

  switch (pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
      bchanged_item_font = bchanged_subitem_font = false;
      pAddEditFont = Fonts::GetInstance()->GetAddEditFont();
      pPasswordFont = Fonts::GetInstance()->GetPasswordFont();
      pNotesFont = Fonts::GetInstance()->GetNotesFont();
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      bitem_selected = (pLVCD->nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED;
      *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPOSTPAINT:
      // Item PostPaint - restore old font if any
      if (bchanged_item_font) {
        bchanged_item_font = false;
        pDC->SelectObject(pAddEditFont);
        *pLResult |= CDRF_NEWFONT;
      }
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      // Sub-item PrePaint
      GetHeaderCtrl()->GetItem(pLVCD->iSubItem, &hdi);
      if (hdi.lParam == CItemData::PASSWORD) {
        // Use Password font
        bchanged_subitem_font = true;
        pDC->SelectObject(pPasswordFont);
        *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
      } else
        if (hdi.lParam == CItemData::NOTES) {
          // Use Notes font
          bchanged_subitem_font = true;
          pDC->SelectObject(pNotesFont);
          *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
        } else
      if (m_bUseHighLighting) {
        COLORREF cf;
        CFont *uFont = GetFontBasedOnStatus(pci, cf);
        if (uFont != NULL) {
          bchanged_subitem_font = true;
          pDC->SelectObject(uFont);
          if (!bitem_selected)
            pLVCD->clrText = cf;
          *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
        }
      }
      break;

    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
      // Sub-item PostPaint
      // Restore old font if any
      if (bchanged_subitem_font) {
        bchanged_subitem_font = bitem_selected = false;
        pDC->SelectObject(pAddEditFont);
        *pLResult |= CDRF_NEWFONT;
      }
      break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    case CDDS_POSTPAINT:
    */
    default:
      break;
  }
}

void CPWListCtrlX::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
  if (!Fonts::GetInstance())
     return;
  
  int padding = 4;
  if (GetExtendedStyle() & LVS_EX_GRIDLINES)
     padding += 2;
  
  const bool bNotesDisplayed = IsNotesColumnPresent();
  lpMeasureItemStruct->itemHeight = Fonts::GetInstance()->CalcHeight(bNotesDisplayed) + 
    padding;

  // Remove LVS_OWNERDRAWFIXED style to apply default DrawItem
  ModifyStyle(LVS_OWNERDRAWFIXED, 0);
}

void CPWListCtrlX::UpdateRowHeight(bool bInvalidate)
{
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

LRESULT CPWListCtrlX::OnSetFont(WPARAM, LPARAM)
{
  LRESULT res = Default();
  UpdateRowHeight(false);
  return res;
}

void CPWListCtrlX::DrawItem(LPDRAWITEMSTRUCT){
  //DrawItem must be overridden for LVS_OWNERDRAWFIXED style lists
}
