/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ControlExtns.cpp
//

#include "stdafx.h"
#include "ControlExtns.h"

#include "core/ItemField.h" // for CSecEditExtn
#include "core/BlowFish.h"  // ditto
#include "core/PWSrand.h"   // ditto

#include "resource2.h"     // for CEditExtn context menu

#include "vsstyle.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EDIT_CLIPBOARD_TEXT_FORMAT  CF_UNICODETEXT

const COLORREF crefInFocus = (RGB(222, 255, 222));  // Light green
const COLORREF crefNoFocus = (RGB(255, 255, 255));  // White
const COLORREF crefBlack   = (RGB(  0,   0,   0));  // Black

// timer event numbers used to by ControlExtns for ListBox tooltips. See DboxMain.h
#define TIMER_LB_HOVER     0x0A
#define TIMER_LB_SHOWING   0x0B 

/*
HOVER_TIME_LB       The length of time the pointer must remain stationary
                    within a tool's bounding rectangle before the tool tip
                    window appears.

TIMEINT_LB_SHOWING The length of time the tool tip window remains visible
                   if the pointer is stationary within a tool's bounding
                   rectangle.
*/
#define HOVER_TIME_LB      1000
#define TIMEINT_LB_SHOWING 5000

/////////////////////////////////////////////////////////////////////////////
// CStaticExtn

CStaticExtn::CStaticExtn()
  : m_bUserColour(false), m_bUserBkColour(false), m_bMouseInWindow(false), 
  m_iFlashing(0), m_bHighlight(false)
{
}

CStaticExtn::~CStaticExtn()
{
  if (m_bUserBkColour)
    m_brBkUser.DeleteObject();
}

BEGIN_MESSAGE_MAP(CStaticExtn, CStatic)
  //{{AFX_MSG_MAP(CStaticExtn)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_MOUSEMOVE()
  ON_WM_CTLCOLOR_REFLECT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CStaticExtn::SetBkColour(COLORREF cfBkUser)
{
  if (m_bUserBkColour)
    m_brBkUser.DeleteObject();

  m_cfBkUser = cfBkUser;
  m_brBkUser.CreateSolidBrush(cfBkUser);
  m_bUserBkColour = true;
}

void CStaticExtn::ResetBkColour()
{
  if (m_bUserBkColour)
    m_brBkUser.DeleteObject();
  m_bUserBkColour = false;
}

void CStaticExtn::FlashBkgnd(COLORREF cfFlashColour)
{
  // Set flash colour
  m_cfFlashColour = cfFlashColour;
  m_iFlashing = 1;
  // Cause repaint
  Invalidate();
  UpdateWindow();
  // Sleep to give the impression of a flash
  ::Sleep(200);
  // Reset colour
  m_iFlashing = -1;
  // Cause repaint
  Invalidate();
  UpdateWindow();
  // Turn off flashing
  m_iFlashing = 0;
}

void CStaticExtn::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!m_bMouseInWindow) {
    m_bMouseInWindow = true;
    if (m_bHighlight) {
      // Set background
      Invalidate();
      UpdateWindow();
    }
    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0};
    VERIFY(TrackMouseEvent(&tme));
  }

  CStatic::OnMouseMove(nFlags, point);
}

LRESULT CStaticExtn::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_bHighlight) {
    m_bMouseInWindow = false;
    // Reset background
    Invalidate();
    UpdateWindow();
  }
  return 0L;
}

HBRUSH CStaticExtn::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
  if (!this->IsWindowEnabled())
    return NULL;

  if (m_bUserColour) {
    pDC->SetTextColor(m_cfUser);
  }
  if (m_bUserBkColour) {
    pDC->SetBkColor(m_cfBkUser);
    pDC->SetBkMode(TRANSPARENT);
    return (HBRUSH)(m_brBkUser.GetSafeHandle());
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CEditExtn

CEditExtn::CEditExtn(COLORREF focusColor)
  : m_bIsFocused(FALSE), m_lastposition(-1),
    m_crefInFocus(focusColor)
{
  m_brInFocus.CreateSolidBrush(focusColor);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);

  m_vmenu_items.clear();
}

CEditExtn::CEditExtn(std::vector<st_context_menu> vmenu_items,
                     COLORREF focusColor)
  : m_bIsFocused(FALSE), m_lastposition(-1),
    m_crefInFocus(focusColor)
{
  m_brInFocus.CreateSolidBrush(focusColor);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);
  // Don't allow if menu string is empty.
  if (vmenu_items.empty()) {
    m_vmenu_items.clear();
  } else {
    m_vmenu_items = vmenu_items;
  }
}

CEditExtn::~CEditExtn()
{
  m_brInFocus.DeleteObject();
  m_brNoFocus.DeleteObject();
}

BEGIN_MESSAGE_MAP(CEditExtn, CEdit)
  //{{AFX_MSG_MAP(CEditExtn)
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_CTLCOLOR_REFLECT()
  ON_WM_CONTEXTMENU()
  ON_WM_LBUTTONDOWN()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditExtn message handlers

DWORD CEditExtn::GetSel()
{
  DWORD dwSelection = CEdit::GetSel();
  m_nStartChar = LOWORD(dwSelection);
  m_nEndChar = HIWORD(dwSelection);
  return dwSelection;
}

void CEditExtn::GetSel(int &nStartChar, int &nEndChar)
{
  CEdit::GetSel(nStartChar, nEndChar);
  m_nStartChar = nStartChar;
  m_nEndChar = nEndChar;
}

void CEditExtn::SetSel(DWORD dwSelection, BOOL bNoScroll)
{
  m_nStartChar = LOWORD(dwSelection);
  m_nEndChar = HIWORD(dwSelection);
  CEdit::SetSel(dwSelection, bNoScroll);
}

void CEditExtn::SetSel(int nStartChar, int nEndChar, BOOL bNoScroll)
{
  m_nStartChar = nStartChar;
  m_nEndChar = nEndChar;
  CEdit::SetSel(nStartChar, nEndChar, bNoScroll);
}

void CEditExtn::OnSetFocus(CWnd* pOldWnd)
{
  m_bIsFocused = TRUE;
  CEdit::OnSetFocus(pOldWnd);
  if (m_lastposition >= 0) {
    int iLine = LineFromChar(m_lastposition);
    LineScroll(iLine);
    SetSel(m_nStartChar, m_nEndChar); 
  }
  Invalidate(TRUE);
}

void CEditExtn::OnKillFocus(CWnd* pNewWnd)
{
  m_bIsFocused = FALSE;
  m_lastposition = LineIndex();
  GetSel(m_nStartChar, m_nEndChar);
  CEdit::OnKillFocus(pNewWnd);
  Invalidate(TRUE);
}

void CEditExtn::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Get the scroll bar position.
  int nScrollHPos = GetScrollPos(SB_HORZ);
  int nScrollVPos = GetScrollPos(SB_VERT);

  int n = CharFromPos(point);
  m_lastposition = HIWORD(n);
  m_nStartChar = m_nEndChar = LOWORD(n);

  CEdit::OnLButtonDown(nFlags, point);

  // Reset the scroll bar position.
  SetScrollPos(SB_HORZ, nScrollHPos);
  SetScrollPos(SB_VERT, nScrollVPos);

  // Reset the display scroll position.
  SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, nScrollHPos), 0);
  SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, nScrollVPos), 0);

  SetSel(m_nStartChar, m_nEndChar);
}

HBRUSH CEditExtn::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
  if (!this->IsWindowEnabled())
    return NULL;

  pDC->SetTextColor(crefBlack);
  if (m_bIsFocused == TRUE) {
    pDC->SetBkColor(m_crefInFocus);
    return m_brInFocus;
  } else {
    pDC->SetBkColor(crefNoFocus);
    return m_brNoFocus;
  }
}

// Following structure used by botlh CEditExtn and CRichEditExtn for
// finding menu items in the supplied extra vector based on command message number
struct equal_cmd {
  equal_cmd(UINT_PTR const& nCmd) : m_nCmd(nCmd) {}
  bool operator()(st_context_menu const& context_menu) const
  {
    return (context_menu.message_number == m_nCmd);
  }
private:
  UINT_PTR m_nCmd;
};

void CEditExtn::OnContextMenu(CWnd* pWnd, CPoint point)
{
  if (m_vmenu_items.empty()) {
    CEdit::OnContextMenu(pWnd, point);
    return;
  }

  SetFocus();
  CMenu menu;
  menu.CreatePopupMenu();

  BOOL bReadOnly = GetStyle() & ES_READONLY;
  DWORD flags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED;

  int iPos(0);
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, EM_UNDO,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_UNDO)));

  menu.InsertMenu(iPos++, MF_BYPOSITION | MF_SEPARATOR);

  DWORD sel = GetSel();
  flags = LOWORD(sel) == HIWORD(sel) ? MF_GRAYED : 0;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_COPY,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_COPY)));

  flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_CUT,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_CUT)));

  flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_CLEAR,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_DELETE)));

  flags = IsClipboardFormatAvailable(EDIT_CLIPBOARD_TEXT_FORMAT) &&
                                     !bReadOnly ? 0 : MF_GRAYED;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_PASTE,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_PASTE)));

  menu.InsertMenu(iPos++, MF_BYPOSITION | MF_SEPARATOR);

  int len = GetWindowTextLength();
  flags = (!len || (LOWORD(sel) == 0 && HIWORD(sel) == len)) ? MF_GRAYED : 0;

  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, EM_SELECTALL,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_SELECTALL)));

  menu.InsertMenu(iPos++, MF_BYPOSITION | MF_SEPARATOR);

  for (size_t i = 0; i < m_vmenu_items.size(); i++) {
      menu.InsertMenu((int)i + iPos, MF_BYPOSITION | m_vmenu_items[i].flags,
                      m_vmenu_items[i].message_number,
                      m_vmenu_items[i].menu_string.c_str());
      if (!m_vmenu_items[i].bEnable)
        menu.EnableMenuItem((int)i + iPos, MF_BYPOSITION | MF_GRAYED);
  }

  if (point.x == -1 || point.y == -1) {
    CRect rc;
    GetClientRect(&rc);
    point = rc.CenterPoint();
    ClientToScreen(&point);
  }

  UINT_PTR nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
                                      TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                      point.x, point.y, this);

  if (nCmd == 0)
    return;

  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(nCmd));
  if (iter != m_vmenu_items.end()) {
    this->GetParent()->SendMessage((UINT)nCmd, iter->wParam, iter->lParam);
    return;
  }

  switch (nCmd) {
    case EM_UNDO:
    case WM_CUT:
    case WM_COPY:
    case WM_CLEAR:
    case WM_PASTE:
      SendMessage((UINT)nCmd);
      break;
    case EM_SELECTALL:
      SendMessage(EM_SETSEL, 0, -1);
      break;
    default:
      break;
  }
}

void CEditExtn::UpdateState(const int message_number, const BOOL new_state)
{
  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(message_number));
  if (iter != m_vmenu_items.end()) {
    int flags = new_state == TRUE ? MF_CHECKED : MF_UNCHECKED;
    iter->flags = flags;
    return;
  }
}

void CEditExtn::EnableMenuItem(const int message_number, const bool bEnable)
{
  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(message_number));
  if (iter != m_vmenu_items.end()) {
    iter->bEnable = bEnable;
    return;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditExtn

CRichEditExtn::CRichEditExtn(COLORREF focusColor)
  : m_bIsFocused(FALSE), m_lastposition(-1),
    m_crefInFocus(focusColor)
{
  m_brInFocus.CreateSolidBrush(focusColor);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);

  m_vmenu_items.clear();
}

CRichEditExtn::CRichEditExtn(std::vector<st_context_menu> vmenu_items,
                     COLORREF focusColor)
  : m_bIsFocused(FALSE), m_lastposition(-1),
    m_crefInFocus(focusColor)
{
  m_brInFocus.CreateSolidBrush(focusColor);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);
  // Don't allow if menu string is empty.
  if (vmenu_items.empty()) {
    m_vmenu_items.clear();
  } else {
    m_vmenu_items = vmenu_items;
  }
}

CRichEditExtn::~CRichEditExtn()
{
  m_brInFocus.DeleteObject();
  m_brNoFocus.DeleteObject();
}

BEGIN_MESSAGE_MAP(CRichEditExtn, CRichEditCtrl)
  //{{AFX_MSG_MAP(CRichEditExtn)
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_CTLCOLOR_REFLECT()
  ON_WM_CONTEXTMENU()
  ON_WM_LBUTTONDOWN()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRichEditExtn message handlers

void CRichEditExtn::GetSel(long &nStartChar, long &nEndChar)
{
  CRichEditCtrl::GetSel(nStartChar, nEndChar);
  m_nStartChar = nStartChar;
  m_nEndChar = nEndChar;
}

void CRichEditExtn::SetSel(long nStartChar, long nEndChar)
{
  m_nStartChar = nStartChar;
  m_nEndChar = nEndChar;
  CRichEditCtrl::SetSel(nStartChar, nEndChar);
}

void CRichEditExtn::OnSetFocus(CWnd* pOldWnd)
{
  m_bIsFocused = TRUE;
  CRichEditCtrl::OnSetFocus(pOldWnd);
  if (m_lastposition >= 0) {
    int iLine = LineFromChar(m_lastposition);
    LineScroll(iLine);
    SetSel(m_nStartChar, m_nEndChar); 
  }
  Invalidate(TRUE);
}

void CRichEditExtn::OnKillFocus(CWnd* pNewWnd)
{
  m_bIsFocused = FALSE;
  m_lastposition = LineIndex();
  GetSel(m_nStartChar, m_nEndChar);
  CRichEditCtrl::OnKillFocus(pNewWnd);
  Invalidate(TRUE);
}

void CRichEditExtn::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Get the scroll bar position.
  int nScrollHPos = GetScrollPos(SB_HORZ);
  int nScrollVPos = GetScrollPos(SB_VERT);

  int n = CharFromPos(point);
  m_lastposition = HIWORD(n);
  m_nStartChar = m_nEndChar = LOWORD(n);

  CRichEditCtrl::OnLButtonDown(nFlags, point);

  // Reset the scroll bar position.
  SetScrollPos(SB_HORZ, nScrollHPos);
  SetScrollPos(SB_VERT, nScrollVPos);

  // Reset the display scroll position.
  SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, nScrollHPos), 0);
  SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, nScrollVPos), 0);

  SetSel(m_nStartChar, m_nEndChar);
}

HBRUSH CRichEditExtn::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
  if (!this->IsWindowEnabled())
    return NULL;

  pDC->SetTextColor(crefBlack);
  if (m_bIsFocused == TRUE) {
    pDC->SetBkColor(m_crefInFocus);
    return m_brInFocus;
  } else {
    pDC->SetBkColor(crefNoFocus);
    return m_brNoFocus;
  }
}

void CRichEditExtn::OnContextMenu(CWnd* pWnd, CPoint point)
{
  if (m_vmenu_items.empty()) {
    CRichEditExtn::OnContextMenu(pWnd, point);
    return;
  }

  SetFocus();
  CMenu menu;
  menu.CreatePopupMenu();

  BOOL bReadOnly = GetStyle() & ES_READONLY;
  DWORD flags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED;

  int iPos(0);
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, EM_UNDO,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_UNDO)));

  menu.InsertMenu(iPos++, MF_BYPOSITION | MF_SEPARATOR);

  long lStart, lEnd;
  GetSel(lStart, lEnd);
  flags = lStart == lEnd ? MF_GRAYED : 0;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_COPY,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_COPY)));

  flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_CUT,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_CUT)));

  flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_CLEAR,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_DELETE)));

  flags = IsClipboardFormatAvailable(EDIT_CLIPBOARD_TEXT_FORMAT) &&
                                     !bReadOnly ? 0 : MF_GRAYED;
  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, WM_PASTE,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_PASTE)));

  menu.InsertMenu(iPos++, MF_BYPOSITION | MF_SEPARATOR);

  int len = GetWindowTextLength();
  flags = (!len || (lStart == 0 && lEnd == len)) ? MF_GRAYED : 0;

  menu.InsertMenu(iPos++, MF_BYPOSITION | flags, EM_SELECTALL,
                  CString(MAKEINTRESOURCE(IDS_MENUSTRING_SELECTALL)));

  menu.InsertMenu(iPos++, MF_BYPOSITION | MF_SEPARATOR);

  for (size_t i = 0; i < m_vmenu_items.size(); i++) {
      menu.InsertMenu((int)i + iPos, MF_BYPOSITION | m_vmenu_items[i].flags,
                      m_vmenu_items[i].message_number,
                      m_vmenu_items[i].menu_string.c_str());
      if (!m_vmenu_items[i].bEnable)
        menu.EnableMenuItem((int)i + iPos, MF_BYPOSITION | MF_GRAYED);
  }

  if (point.x == -1 || point.y == -1) {
    CRect rc;
    GetClientRect(&rc);
    point = rc.CenterPoint();
    ClientToScreen(&point);
  }

  UINT_PTR nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
                                      TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                      point.x, point.y, this);

  if (nCmd == 0)
    return;

  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(nCmd));
  if (iter != m_vmenu_items.end()) {
    this->GetParent()->SendMessage((UINT)nCmd, iter->wParam, iter->lParam);
    return;
  }

  switch (nCmd) {
    case EM_UNDO:
    case WM_CUT:
    case WM_COPY:
    case WM_CLEAR:
    case WM_PASTE:
      SendMessage((UINT)nCmd);
      break;
    case EM_SELECTALL:
      SendMessage(EM_SETSEL, 0, -1);
      break;
    default:
      break;
  }
}

void CRichEditExtn::UpdateState(const int message_number, const BOOL new_state)
{
  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(message_number));
  if (iter != m_vmenu_items.end()) {
    int flags = new_state == TRUE ? MF_CHECKED : MF_UNCHECKED;
    iter->flags = flags;
    return;
  }
}

void CRichEditExtn::EnableMenuItem(const int message_number, const bool bEnable)
{
  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(message_number));
  if (iter != m_vmenu_items.end()) {
    iter->bEnable = bEnable;
    return;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CListBoxExtn

CListBoxExtn::CListBoxExtn()
  : m_bIsFocused(FALSE), m_pLBToolTips(NULL), m_bUseToolTips(false),
  m_bMouseInWindow(false), m_nHoverLBTimerID(0), m_nShowLBTimerID(0),
  m_HoverLBnItem(-1), m_pCombo(NULL)
{
  m_brInFocus.CreateSolidBrush(crefInFocus);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);
}

CListBoxExtn::~CListBoxExtn()
{
  m_brInFocus.DeleteObject();
  m_brNoFocus.DeleteObject();

  delete m_pLBToolTips;
}

BEGIN_MESSAGE_MAP(CListBoxExtn, CListBox)
  //{{AFX_MSG_MAP(CListBoxExtn)
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_CTLCOLOR_REFLECT()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_MOUSEMOVE()
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListBoxExtn message handlers

void CListBoxExtn::OnSetFocus(CWnd* pOldWnd)
{
  m_bIsFocused = TRUE;
  CListBox::OnSetFocus(pOldWnd);
  Invalidate(TRUE);
}

void CListBoxExtn::OnKillFocus(CWnd* pNewWnd)
{
  m_bIsFocused = FALSE;
  CListBox::OnKillFocus(pNewWnd);
  Invalidate(TRUE);
}

HBRUSH CListBoxExtn::CtlColor(CDC* pDC, UINT /* nCtlColor */)
{
  if (!this->IsWindowEnabled())
    return NULL;

  if (m_bIsFocused == TRUE) {
    pDC->SetBkColor(crefInFocus);
    return m_brInFocus;
  } else {
    pDC->SetBkColor(crefNoFocus);
    return m_brNoFocus;
  }
}

void CListBoxExtn::ActivateToolTips()
{
  m_bUseToolTips = true;
  m_pLBToolTips = new CInfoDisplay;

  if (!m_pLBToolTips->Create(0, 0, L"", this)) {
    // failed
    delete m_pLBToolTips;
    m_pLBToolTips = NULL;
  } else
    m_pLBToolTips->ShowWindow(SW_HIDE);
}

bool CListBoxExtn::ShowToolTip(int nItem, const bool bVisible)
{
  if (!m_bUseToolTips)
    return false;

  if (nItem < 0) {
    m_pLBToolTips->ShowWindow(SW_HIDE);
    return false;
  }

  m_pLBToolTips->SetWindowText(m_pCombo->GetToolTip(nItem));
  if (!bVisible) {
    m_pLBToolTips->ShowWindow(SW_HIDE);
    return false;
  }

  CPoint pt;
  ::GetCursorPos(&pt);

  pt.y += ::GetSystemMetrics(SM_CYCURSOR) / 2; // half-height of cursor

  m_pLBToolTips->SetWindowPos(&wndTopMost, pt.x, pt.y, 0, 0,
                              SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

  return true;
}

void CListBoxExtn::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_LB_HOVER:
      KillTimer(m_nHoverLBTimerID);
      m_nHoverLBTimerID = 0;
      if (ShowToolTip(m_HoverLBnItem, true)) {
        if (m_nShowLBTimerID) {
          KillTimer(m_nShowLBTimerID);
          m_nShowLBTimerID = 0;
        }
        m_nShowLBTimerID = SetTimer(TIMER_LB_SHOWING, TIMEINT_LB_SHOWING, NULL);
      }
      break;
    case TIMER_LB_SHOWING:
      KillTimer(m_nShowLBTimerID);
      m_nShowLBTimerID = 0;
      m_HoverLBPoint = CPoint(0, 0);
      m_HoverLBnItem = -1;
      ShowToolTip(m_HoverLBnItem, false);
      break;
    default:
      CListBox::OnTimer(nIDEvent);
      break;
  }
}

void CListBoxExtn::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!m_bUseToolTips) {
    goto exit;
  } else {
    CRect rectClient;
    GetClientRect(&rectClient);
    BOOL bOutside(FALSE);
    int nItem = -2;

    if (rectClient.PtInRect(point)) {
      CPoint pointScreen;
      ::GetCursorPos(&pointScreen);
      nItem = ItemFromPoint(point, bOutside);  // calculate listbox item number (if any)
    }

    if (m_nHoverLBTimerID) {
      if (!bOutside && m_HoverLBnItem == nItem)
        return;
      KillTimer(m_nHoverLBTimerID);
      m_nHoverLBTimerID = 0;
    }

    if (m_nShowLBTimerID) {
      if (!bOutside && m_HoverLBnItem == nItem)
        return;
      KillTimer(m_nShowLBTimerID);
      m_nShowLBTimerID = 0;
      ShowToolTip(m_HoverLBnItem, false);
    }

    if (!m_bMouseInWindow) {
      m_bMouseInWindow = true;
      TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0};
      VERIFY(TrackMouseEvent(&tme));
    }

    m_nHoverLBTimerID = SetTimer(TIMER_LB_HOVER, HOVER_TIME_LB, NULL);
    m_HoverLBPoint = point;
    m_HoverLBnItem = nItem;
  }

exit:
  CListBox::OnMouseMove(nFlags, point);
}

LRESULT CListBoxExtn::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_bUseToolTips) {
    KillTimer(m_nHoverLBTimerID);
    KillTimer(m_nShowLBTimerID);
    m_nHoverLBTimerID = m_nShowLBTimerID = 0;
    m_HoverLBPoint = CPoint(0, 0);
    m_HoverLBnItem = -1;
    ShowToolTip(m_HoverLBnItem, false);
    m_bMouseInWindow = false;
  }
  return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// CComboBoxExtn

CComboBoxExtn::CComboBoxExtn()
 : m_bUseToolTips(false)
{
}

CComboBoxExtn::~CComboBoxExtn()
{
}

BEGIN_MESSAGE_MAP(CComboBoxExtn, CComboBox)
  //{{AFX_MSG_MAP(CComboBoxExtn)
  ON_WM_CTLCOLOR()
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboBoxExt message handlers

HBRUSH CComboBoxExtn::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);

  if (nCtlColor == CTLCOLOR_EDIT) {
    // Extended Edit control
    if (m_edit.GetSafeHwnd() == NULL)
      m_edit.SubclassWindow(pWnd->GetSafeHwnd());
  }
  else if (nCtlColor == CTLCOLOR_LISTBOX) {
    // Extended ListBox control
    if (m_listbox.GetSafeHwnd() == NULL) {
      m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
      m_listbox.SetCombo(this);
      if (m_bUseToolTips) {
        m_listbox.ActivateToolTips();
      }
    }
  }

  return hbr;
}

void CComboBoxExtn::OnDestroy()
{
  if (m_edit.GetSafeHwnd() != NULL)
    m_edit.UnsubclassWindow();

  if (m_listbox.GetSafeHwnd() != NULL)
    m_listbox.UnsubclassWindow();

  CComboBox::OnDestroy();
}

void CComboBoxExtn::ChangeColour()
{
  m_edit.ChangeColour();
  m_listbox.ChangeColour();
}

void CComboBoxExtn::SetToolTipStrings(std::vector<CSecString> vtooltips)
{
  m_bUseToolTips = true;
  m_vtooltips = vtooltips;
}

//-----------------------------------------------------------------
// CSecEditExtn is meant for sensitive information that you really don't
// want to be in memory more than necessary, such as master passwords
//
// Note that to debug it, you need to #define DEBUG_CSECEDITEXTN
// so that sensitive information doesn't leak in a debug version...
//-----------------------------------------------------------------

const wchar_t FILLER = 0x08; // ASCII backspace doesn't occur in Edit

struct CSecEditExtn::Impl {
  Impl() : m_field(0) {
    unsigned char key[20];
    PWSrand::GetInstance()->GetRandomData(key, sizeof(key));
    m_bf = new BlowFish(key, sizeof(key));
    SecureZeroMemory(key, sizeof(key));
  }

  ~Impl() {delete m_bf;}
  CItemField m_field;
  BlowFish *m_bf;
};

CSecEditExtn::CSecEditExtn()
  : CEditExtn((RGB(255, 222, 222))), // light red
    m_pImpl(new Impl), m_secure(true), m_in_recursion(false)
{
}

CSecEditExtn::CSecEditExtn(std::vector<st_context_menu> vmenu_items)
  : CEditExtn(vmenu_items, (RGB(255, 222, 222))),
    m_pImpl(new Impl), m_secure(true), m_in_recursion(false)
{
}

BEGIN_MESSAGE_MAP(CSecEditExtn, CEditExtn)
  //{{AFX_MSG_MAP(CSecEditExtn)
  ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSecEditExtn::~CSecEditExtn()
{
  delete m_pImpl;
}

void CSecEditExtn::SetSecure(bool on_off)
{
  m_secure = on_off;
}

CSecString CSecEditExtn::GetSecureText() const
{
  CSecString retval;
  StringX sval;
  m_pImpl->m_field.Get(sval, m_pImpl->m_bf);
  retval = sval.c_str();
  return retval;
}

void CSecEditExtn::SetSecureText(const CSecString &str)
{
  m_pImpl->m_field.Set(str, m_pImpl->m_bf);
  if (::IsWindow(m_hWnd)) {
    if (!m_secure) {
      SetWindowText(str);
    } else {
      CString blanks;
      for (int i = 0; i < str.GetLength(); i++)
        blanks += FILLER;
      SetWindowText(blanks);
    }
  }
}

void CSecEditExtn::DoDDX(CDataExchange *pDX, CSecString &str)
{
  if (pDX->m_bSaveAndValidate) {
    str = GetSecureText();
  } else {
    SetSecureText(str);
  }
}

afx_msg void CSecEditExtn::OnUpdate()
{
#ifdef DEBUG_CSECEDITEXTN
  CString dstr;
  GetWindowText(dstr);
  pws_os::Trace(L"CSecEditExtn::OnUpdate(%s)\n",dstr);
#endif
  if (m_secure) {
    if (!m_in_recursion)
      OnSecureUpdate();
  } else {
    CSecString str;
    GetWindowText(str);
    m_pImpl->m_field.Set(str, m_pImpl->m_bf);
  }
  m_in_recursion = false;
}

void CSecEditExtn::OnSecureUpdate()
{
  // after text's changed
  // update local store, replace with same number of FILLERS
  // Note that text may have been added or deleted anywhere, so we
  // rely on the fact that the user cannot enter FILLER chars,
  // and treat any non-FILLER character as added or deleted text.

  int startSel, endSel;
  GetSel(startSel, endSel);

  CSecString new_str, old_str, str;
  int new_len, old_len;
  old_str = GetSecureText();
  GetWindowText(new_str);
  new_len = new_str.GetLength(); old_len = old_str.GetLength();

  // Find start/end of new text:
  int newEnd, newStart;
  for (newStart = 0; new_str.GetAt(newStart) == FILLER; newStart++)
    ;
  for (newEnd = new_len - 1; newEnd >= 0 && new_str.GetAt(newEnd) == FILLER;
       newEnd--)
    ;

  // Simple case: new text fully replaces old
  if (newEnd - newStart == new_len - 1) {
    str = new_str;
  } else {
    int delta = new_len - old_len;
    if (delta == 0) { // no-op or text replaced via Paste with same length
      if (new_len == 0)
        return;
      else // note that new_len == old_len
        for (int i = 0; i < new_len; i++) {
          str += new_str[i] == FILLER ? old_str[i] : new_str[i];
        }
    } else if (delta >= 0) { // text added, but where?
      // Added text most likely by typing at end, but can also be
      // via typing or pasting in another position.
      if (newEnd == new_str.GetLength() - 1) { // - at the end
        str = old_str;
        str += new_str.Mid(new_len - delta, delta);
      } else { // - in the beginning or middle
        if (newStart == 0) { // beginning
          str = new_str.Left(newEnd + 1);
          str += old_str;
        } else { // middle
          str = old_str.Left(newStart);
          str += new_str.Mid(newStart, newEnd - newStart + 1);
          str += old_str.Right(new_len - newEnd - 1);
        }
      }
    } else { // text was deleted
      str = old_str.Left(startSel);
      str += old_str.Right(new_len - endSel);
    }
  }
  m_in_recursion = true; // the following change will trigger another update
#ifdef DEBUG_CSECEDITEXTN
  pws_os::Trace(L"CSecEditExtn::OnSecureUpdate: GetSel(%d, %d), str = %s\n",
        startSel, endSel, str);
#endif
  SetSecureText(str);
  SetSel(startSel, endSel); // need to restore after Set.
}

/////////////////////////////////////////////////////////////////////////////
// CSymbolEdit

BEGIN_MESSAGE_MAP(CSymbolEdit, CEdit)
  //{{AFX_MSG_MAP(CSymbolEdit)
  ON_WM_CHAR()
  ON_MESSAGE(WM_PASTE, OnPaste)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSymbolEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  //Allowed inputs are passed on to the base class
  if (nChar == VK_BACK) {
    CEdit::OnChar(nChar, nRepCnt, nFlags);
    return;
  }

  wint_t wChar = reinterpret_cast<wint_t &>(nChar);
  // Must not be alphanumeric nor have and Alt/Ctrl key
  if ((_istalpha(wChar) == 0 && _istdigit(wChar) == 0 && (nFlags & 0xE0800000) == 0)) {
    CString cs_text;
    GetWindowText(cs_text);
    // Must not have duplicates
    if (cs_text.Find(wChar) == -1)
      CEdit::OnChar(nChar, 0, nFlags);
  }
}

LRESULT CSymbolEdit::OnPaste(WPARAM , LPARAM )
{
  // Only allow symbols to be pasted and stop duplicates
  if (!OpenClipboard() || !IsClipboardFormatAvailable(CF_UNICODETEXT))
    return 0L; 
 
  std::wstring cs_data;
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (hData != NULL) {
    wchar_t *buffer = (wchar_t *)GlobalLock(hData);
    if (buffer != NULL) {
      cs_data = buffer;
      GlobalUnlock(hData);
    }
  }
	CloseClipboard();

  CString cs_text, cs_oldtext;;
  GetWindowText(cs_text);
  cs_oldtext = cs_text;

  // Must not be alphanumeric
  for (size_t i = 0; i < cs_data.length(); i++) {
    wchar_t wChar = cs_data.at(i);
    if ((_istalpha(wChar) == 0 && _istdigit(wChar) == 0)) {
      // Must not have duplicates
      if (cs_text.Find(wChar) == -1)
        cs_text += wChar;
    }
  }

  if (cs_text.Compare(cs_oldtext) != 0)
    SetWindowText(cs_text);

  return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// CButtonExtn

CButtonExtn::CButtonExtn()
  : m_bUseTextColour(false), m_bUseBkgColour(false),
  m_caption(L""), m_type(BS_AUTOCHECKBOX)
{
}

CButtonExtn::~CButtonExtn()
{
}

BEGIN_MESSAGE_MAP(CButtonExtn, CButton)
  //{{AFX_MSG_MAP(CButtonExtn)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CButtonExtn::SetType(int type)
{
  // Only the following are supported
  if (type != BS_AUTOCHECKBOX && type != BS_AUTORADIOBUTTON) {
    ASSERT(0);
    return;
  }

  m_type = type;
}

void CButtonExtn::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // Code originally by Nikita Leontiev in answer to "Change checkBox text color Win32"
  // in MS's Forum: "Visual Studio Developer Center > Visual Studio vNext Forums > Visual C++ General"
  // Modified for MFC, Checkbox and Radio buttons by DK

  LPNMCUSTOMDRAW lpNMCustomDraw = (LPNMCUSTOMDRAW)pNotifyStruct;
  *pLResult = CDRF_DODEFAULT;

  switch (lpNMCustomDraw->dwDrawStage) {
    case CDDS_PREERASE:
      BOOL fChecked = GetCheck() & BST_CHECKED;
      BOOL fHot = lpNMCustomDraw->uItemState & CDIS_HOT;
      DrawButton(lpNMCustomDraw->hdr.hwndFrom, lpNMCustomDraw->hdc,
				     		&lpNMCustomDraw->rc, fChecked, fHot);
  }
}

void CButtonExtn::DrawButton(HWND hWnd, HDC hDC, RECT *pRect, BOOL fChecked, BOOL fHot)
{
  // Code originally by Nikita Leontiev in answer to "Change checkBox text color Win32"
  // in MS's Forum: "Visual Studio Developer Center > Visual Studio vNext Forums > Visual C++ General"
  // Modified for MFC, Checkbox and Radio buttons by DK

  int nWidth = pRect -> right - pRect -> left;
  int nHeight = pRect -> bottom - pRect -> top;

  HDC hMemDC = CreateCompatibleDC(hDC);
  HBITMAP hBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
  SelectObject(hMemDC, hBitmap);

  RECT rFillRect = {0, 0, nWidth, nHeight};
  FillRect(hMemDC, &rFillRect,
           CreateSolidBrush(GetSysColor(m_bUseBkgColour ? m_icolour : COLOR_WINDOW)));

  HTHEME hTheme = OpenThemeData(hWnd, L"BUTTON");
  int nStateID(0);

  if (m_type == BS_AUTOCHECKBOX) {
    nStateID = (fChecked) ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
    if (fHot)
      nStateID = (fChecked) ? CBS_CHECKEDHOT : CBS_UNCHECKEDHOT;
  } else {
    nStateID = (fChecked) ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;
    if (fHot)
      nStateID = (fChecked) ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
  }

  RECT rIconRect = {0, 0, 13, nHeight};
  DrawThemeBackground(hTheme, hMemDC, m_type == BS_AUTOCHECKBOX ? BP_CHECKBOX : BP_RADIOBUTTON,
                      nStateID, &rIconRect, NULL);
  CloseThemeData(hTheme);

  RECT rTextRect = {18, 0, nWidth - 18, nHeight};
  SetBkMode(hMemDC, TRANSPARENT);
  if (m_bUseTextColour)
    SetTextColor(hMemDC, m_crfText);

  SelectObject(hMemDC, (HFONT)GetStockObject(DEFAULT_GUI_FONT));

  if (m_caption.IsEmpty()) {
    GetWindowText(m_caption);
    SetWindowText(L"");
  }

  DrawText(hMemDC, m_caption, m_caption.GetLength(), &rTextRect, DT_SINGLELINE | DT_VCENTER);

  BitBlt(hDC, 0, 0, nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

  DeleteObject(hBitmap);
  DeleteDC(hMemDC);
}
