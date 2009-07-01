/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "PWListCtrl.h"
#include "DboxMain.h"
#include "InfoDisplay.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPWListCtrl::CPWListCtrl()
  : m_FindTimerID(0), m_csFind(L""), m_bMouseInWindow(false), 
  m_nHoverNDTimerID(0), m_nShowNDTimerID(0), m_bFilterActive(false),
  m_wpDeleteMsg(WM_KEYDOWN), m_wpDeleteKey(VK_DELETE),
  m_bDeleteCtrl(false), m_bDeleteShift(false)
{
}

CPWListCtrl::~CPWListCtrl()
{
}

BEGIN_MESSAGE_MAP(CPWListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CPWListCtrl)
  ON_MESSAGE(WM_CHAR, OnCharItemlist)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_MOUSEMOVE()
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_ERASEBKGND()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWListCtrl::Initialize()
{
  m_pDbx = static_cast<DboxMain *>(GetParent());
}

void CPWListCtrl::ActivateND(const bool bActivate)
{
  m_bShowNotes = bActivate;
  if (!m_bShowNotes) {
    m_bMouseInWindow = false;
  }
}

LRESULT CPWListCtrl::OnCharItemlist(WPARAM wParam, LPARAM /* lParam */)
{
  const int iSubItem = m_pDbx->IsImageVisible() ? 1 : 0;
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

void CPWListCtrl::OnDestroy()
{
  // Remove dummy ImageList. PWTreeCtrl removes the real one!
  m_pDbx->m_pImageList0->DeleteImageList();
  delete m_pDbx->m_pImageList0;
}

void CPWListCtrl::SetDeleteKey(const unsigned char cVirtKey, const unsigned char cModifier)
{
  m_wpDeleteMsg = ((cModifier & HOTKEYF_ALT) == HOTKEYF_ALT) ? WM_SYSKEYDOWN : WM_KEYDOWN;
  m_wpDeleteKey = cVirtKey;
  m_bDeleteCtrl = (cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL;
  m_bDeleteShift = (cModifier & HOTKEYF_SHIFT) == HOTKEYF_SHIFT;
}

BOOL CPWListCtrl::PreTranslateMessage(MSG* pMsg)
{
  // Process User's Delete shortcut
  if (pMsg->message == m_wpDeleteMsg && pMsg->wParam == m_wpDeleteKey) {
    if (m_bDeleteCtrl  == (GetKeyState(VK_CONTROL) < 0) && 
        m_bDeleteShift == (GetKeyState(VK_SHIFT)   < 0)) {
      if (m_pDbx != NULL)
        m_pDbx->SendMessage(WM_COMMAND, MAKEWPARAM(ID_MENUITEM_DELETEENTRY, 1), 0);
      return TRUE;
    }
  }
  
  // Let the parent class do its thing
  return CListCtrl::PreTranslateMessage(pMsg);
}

void CPWListCtrl::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_FIND:
      KillTimer(TIMER_FIND);
      m_FindTimerID = 0;
      break;
    case TIMER_ND_HOVER:
      KillTimer(m_nHoverNDTimerID);
      m_nHoverNDTimerID = 0;
      if (m_pDbx->SetNotesWindow(m_HoverNDPoint)) {
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
      m_pDbx->SetNotesWindow(m_HoverNDPoint, false);
      break;
    default:
      CListCtrl::OnTimer(nIDEvent);
      break;
  }
}

void CPWListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
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
    m_pDbx->SetNotesWindow(CPoint(0, 0), false);
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

LRESULT CPWListCtrl::OnMouseLeave(WPARAM, LPARAM)
{
  KillTimer(m_nHoverNDTimerID);
  KillTimer(m_nShowNDTimerID);
  m_nHoverNDTimerID = m_nShowNDTimerID = 0;
  m_HoverNDPoint = CPoint(0, 0);
  m_pDbx->SetNotesWindow(m_HoverNDPoint, false);
  m_bMouseInWindow = false;
  return 0L;
}

bool CPWListCtrl::FindNext(const CString &cs_find, const int iSubItem)
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
    iItem = (int)pos;

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
    } while (iItem != (int)pos);
  }

  if (bFound) {
    SetItemState(iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    EnsureVisible(iItem, FALSE);
    Invalidate();
  }

  return bFound;
}

void CPWListCtrl::SetFilterState(bool bState)
{
  m_bFilterActive = bState;

  // Red if filter active, black if not
  SetTextColor(m_bFilterActive ? RGB(168, 0, 0) : RGB(0, 0, 0));
}

BOOL CPWListCtrl::OnEraseBkgnd(CDC* pDC)
{
  if (m_bFilterActive && m_pDbx->GetNumPassedFiltering() == 0) {
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
