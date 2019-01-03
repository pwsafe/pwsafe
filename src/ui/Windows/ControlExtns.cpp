/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ControlExtns.cpp
//

#include "stdafx.h"
#include "ControlExtns.h"
#include "InfoDisplay.h"      // for Listbox Tooltips

#include "core/ItemField.h"   // for CSecEditExtn
#include "core/BlowFish.h"    // ditto
#include "core/PWSrand.h"     // ditto
#include "core/PWCharPool.h"  // for CSymbolEdit
#include "resource2.h"        // for CRichEditExtnX context menu

#include "vsstyle.h"

#include <algorithm>
#include <locale>

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
// CEditExtnX

CEditExtnX::CEditExtnX(COLORREF focusColor)
  : m_bIsFocused(FALSE), m_lastposition(-1),
    m_crefInFocus(focusColor)
{
  m_brInFocus.CreateSolidBrush(focusColor);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);

  m_vmenu_items.clear();
}

CEditExtnX::CEditExtnX(std::vector<st_context_menu> vmenu_items,
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

CEditExtnX::~CEditExtnX()
{
  m_brInFocus.DeleteObject();
  m_brNoFocus.DeleteObject();
}

BEGIN_MESSAGE_MAP(CEditExtnX, CEdit)
  //{{AFX_MSG_MAP(CEditExtnX)
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_CTLCOLOR_REFLECT()
  ON_WM_CONTEXTMENU()
  ON_WM_LBUTTONDOWN()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditExtnX message handlers

DWORD CEditExtnX::GetSel()
{
  DWORD dwSelection = CEdit::GetSel();
  m_nStartChar = LOWORD(dwSelection);
  m_nEndChar = HIWORD(dwSelection);
  return dwSelection;
}

void CEditExtnX::GetSel(int &nStartChar, int &nEndChar)
{
  CEdit::GetSel(nStartChar, nEndChar);
  m_nStartChar = nStartChar;
  m_nEndChar = nEndChar;
}

void CEditExtnX::SetSel(DWORD dwSelection, BOOL bNoScroll)
{
  m_nStartChar = LOWORD(dwSelection);
  m_nEndChar = HIWORD(dwSelection);
  CEdit::SetSel(dwSelection, bNoScroll);
}

void CEditExtnX::SetSel(int nStartChar, int nEndChar, BOOL bNoScroll)
{
  m_nStartChar = nStartChar;
  m_nEndChar = nEndChar;
  CEdit::SetSel(nStartChar, nEndChar, bNoScroll);
}

void CEditExtnX::OnSetFocus(CWnd* pOldWnd)
{
  m_bIsFocused = TRUE;
  CEdit::OnSetFocus(pOldWnd);

  if (m_lastposition >= 0) {
    int iLine = LineFromChar(m_lastposition);
    LineScroll(iLine);
    SetSel(m_nStartChar, m_nEndChar); 
  }
  Invalidate(TRUE);

  ShowCaret();
}

void CEditExtnX::OnKillFocus(CWnd* pNewWnd)
{
  m_bIsFocused = FALSE;
  m_lastposition = LineIndex();
  GetSel(m_nStartChar, m_nEndChar);

  // Force update colour via CtlColor
  Invalidate(TRUE);

  CEdit::OnKillFocus(pNewWnd);
}

void CEditExtnX::OnLButtonDown(UINT nFlags, CPoint point)
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

HBRUSH CEditExtnX::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
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

// Following structure used by both CEditExtnX and CRichEditExtnX for
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

void CEditExtnX::OnContextMenu(CWnd* pWnd, CPoint point)
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

void CEditExtnX::UpdateState(const int message_number, const bool new_state)
{
  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(message_number));
  if (iter != m_vmenu_items.end()) {
    int flags = new_state ? MF_CHECKED : MF_UNCHECKED;
    iter->flags = flags;
    return;
  }
}

void CEditExtnX::EnableMenuItem(const int message_number, const bool bEnable)
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
// CRichEditExtnX

CRichEditExtnX::CRichEditExtnX(COLORREF focusColor)
  : m_bIsFocused(FALSE), m_crefInFocus(focusColor), m_bContextMenu(false)
{
  m_vmenu_items.clear();
}

void CRichEditExtnX::SetContextMenu(const std::vector<st_context_menu> &vmenu_items)
{
  // Don't allow if menu string is empty.
  if (vmenu_items.empty()) {
    m_vmenu_items.clear();
  } else {
    m_vmenu_items = vmenu_items;
  }

  m_hCursor = LoadCursor(NULL, IDC_ARROW);
}

CRichEditExtnX::~CRichEditExtnX()
{
}

BEGIN_MESSAGE_MAP(CRichEditExtnX, CRichEditCtrl)
  //{{AFX_MSG_MAP(CRichEditExtnX)
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_CONTEXTMENU()
  ON_WM_SETCURSOR()
  ON_WM_LBUTTONDBLCLK()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRichEditExtnX message handlers

void CRichEditExtnX::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  long nStartChar, nEndChar;

  // Do what ever would normally happen
  CRichEditCtrl::OnLButtonDblClk(nFlags, point);

  // Get selection
  CRichEditCtrl::GetSel(nStartChar, nEndChar);

  // Check if this included a trailing whitespace and, if so, trim it
  CString csTemp = GetSelText();
  csTemp.TrimRight();

  // Reselect without trailing whitespace
  SetSel(nStartChar, nStartChar + csTemp.GetLength());
}

void CRichEditExtnX::OnSetFocus(CWnd *pOldWnd)
{
  m_bIsFocused = TRUE;
  CRichEditCtrl::OnSetFocus(pOldWnd);

  SetBackgroundColor(FALSE, m_crefInFocus);
  Invalidate(TRUE);
}

void CRichEditExtnX::OnKillFocus(CWnd *pNewWnd)
{
  m_bIsFocused = FALSE;

  CRichEditCtrl::OnKillFocus(pNewWnd);

  SetBackgroundColor(FALSE, crefNoFocus);
  Invalidate(TRUE);
}

BOOL CRichEditExtnX::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
  if (m_bContextMenu) {
    ::SetCursor(m_hCursor);
    return TRUE;
  }
  return CRichEditCtrl::OnSetCursor(pWnd, nHitTest, message);
}

void CRichEditExtnX::OnContextMenu(CWnd* pWnd, CPoint point)
{
  if (m_vmenu_items.empty()) {
    CRichEditExtnX::OnContextMenu(pWnd, point);
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

  // For some reason - weird cursor
  m_bContextMenu = true;
  
  // Now show menu
  UINT_PTR nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
                                      TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                      point.x, point.y, this);
  // Restore cursor
  m_bContextMenu = false;

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
      SendMessage((UINT)nCmd);
      break;
    case WM_PASTE:
      SendMessage(EM_PASTESPECIAL, CF_UNICODETEXT, NULL);
      break;
    case EM_SELECTALL:
      SendMessage(EM_SETSEL, 0, -1);
      break;
    default:
      break;
  }
}

void CRichEditExtnX::UpdateState(const int message_number, const bool new_state)
{
  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(message_number));
  if (iter != m_vmenu_items.end()) {
    int flags = new_state ? MF_CHECKED : MF_UNCHECKED;
    iter->flags = flags;
    return;
  }
}

void CRichEditExtnX::EnableMenuItem(const int message_number, const bool bEnable)
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
  : m_bUseToolTips(false), m_nPenStyle(PS_SOLID), m_crColor(RGB(64, 64, 64)),
  m_nBottomMargin(2), m_nSepWidth(1), m_nHorizontalMargin(2)
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

    CRect rc;
    int nIndex, n = m_listbox.GetCount();

    CPen pen(m_nPenStyle, m_nSepWidth, m_crColor), *pOldPen;
    pOldPen = pDC->SelectObject(&pen);

    for (size_t i = 0; i < m_vSeparators.size(); i++) {
      nIndex = m_vSeparators[i];
      if (nIndex < 0)
        nIndex += n - 1;

      if (nIndex < n - 1) {
        m_listbox.GetItemRect(nIndex, &rc);
        pDC->MoveTo(rc.left + m_nHorizontalMargin, rc.bottom - m_nBottomMargin);
        pDC->LineTo(rc.right - m_nHorizontalMargin, rc.bottom - m_nBottomMargin);
      }
    }

    pDC->SelectObject(pOldPen);
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

  // Disable associated ListBox background colour, since although the
  // background text colour is set correctly, the unused portion of
  // any text line is not set until the mouse moves over it.
  // Re-enable if/when this is solved (probably by making ownerdraw!)

  // m_listbox.ChangeColour();
}

void CComboBoxExtn::SetToolTipStrings(std::vector<CSecString> vtooltips)
{
  m_bUseToolTips = true;
  m_vtooltips = vtooltips;
}

void CComboBoxExtn::SetSeparator(int iSep)
{
  if (!m_vSeparators.size())
    AdjustItemHeight();

  m_vSeparators.push_back(iSep);
}

void CComboBoxExtn::SetSeparator()
{
  SetSeparator(GetCount() - 1);
}

void CComboBoxExtn::AdjustItemHeight(int nInc)
{
  SetItemHeight(0, GetItemHeight(0) + nInc);
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

  ~Impl() {
    delete m_bf;
    // Following to clear the keyboard buffer
    BYTE bytKeyBoardState[256] = {0};
    
    VERIFY(::SetKeyboardState(bytKeyBoardState));
  }
  CItemField m_field;
  BlowFish *m_bf;
};

CSecEditExtn::CSecEditExtn()
  : CEditExtnX((RGB(255, 222, 222))), // light red
    m_pImpl(new Impl), m_secure(true), m_in_recursion(false)
{
}

CSecEditExtn::CSecEditExtn(std::vector<st_context_menu> vmenu_items)
  : CEditExtnX(vmenu_items, (RGB(255, 222, 222))),
    m_pImpl(new Impl), m_secure(true), m_in_recursion(false)
{
}

BEGIN_MESSAGE_MAP(CSecEditExtn, CEditExtnX)
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

CSymbolEdit::CSymbolEdit() : CEdit(), m_validSym(CPasswordCharPool::GetDefaultSymbols())
{
}

void CSymbolEdit::SetValidSym(const std::wstring &s)
{
  // Set the member variable.
  // it's the caller's responsibility to decide
  // whether or not to filter current value of
  // the control to new set.
  m_validSym = s;

# if 0
  // Here's one way to filter text to respect
  // new symbol set
  CString curSyms, newSyms;
  GetWindowText(curSyms);
  size_t slen = m_validSym.length();
  for (size_t i = 0; i < slen; i++)
    if (curSyms.Find(m_validSym[i]))
      newSyms += m_validSym[i];
  SetWindowText(newSyms);
#endif
}

void CSymbolEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  //Allowed inputs are passed on to the base class
  if (nChar == VK_BACK) {
    CEdit::OnChar(nChar, nRepCnt, nFlags);
    return;
  }

  wint_t wChar = reinterpret_cast<wint_t &>(nChar);
  // Do not limit user to 'our' definition of symbols
  // i.e. allow such things as currency symbols - £, € & ¥ etc.
  // Note: EasyVision and MakePronounceable symbols will still be
  // restricted to our lists defined in CPasswordCharPool as will the
  // default symbol set.

  // Can't know user's valid symbols in their locale so just
  // prevent user's locale letters & numbers
  std::locale locl("");
  if (!std::isalnum(wChar, locl)) {
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

  CString cs_text, cs_oldtext;
  GetWindowText(cs_text);
  cs_oldtext = cs_text;

  std::locale locl("");

  for (size_t i = 0; i < cs_data.length(); i++) {
    wchar_t wChar = cs_data.at(i);
    // Can't know user's valid symbols in their locale so just
    // prevent user's locale letters & numbers
    if (!std::isalnum(wChar, locl)) {
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

  NMCUSTOMDRAW *pLVCD = (NMCUSTOMDRAW *)pNotifyStruct;
  *pLResult = CDRF_DODEFAULT;

  switch (pLVCD->dwDrawStage) {
    case CDDS_PREPAINT:
      BOOL fChecked = GetCheck() & BST_CHECKED;
      BOOL fHot = pLVCD->uItemState & CDIS_HOT;
      BOOL fFocus = pLVCD->uItemState & CDIS_FOCUS;
      DrawButton(pLVCD->hdr.hwndFrom, pLVCD->hdc,
                 &pLVCD->rc, fChecked, fHot, fFocus);
  }
}

void CButtonExtn::DrawButton(HWND hWnd, HDC hDC, RECT *pRect, BOOL fChecked, BOOL fHot, BOOL fFocus)
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

  //If bg color isn't set, try get backgroung color from current theme
  if (m_bUseBkgColour) {
    FillRect(hMemDC, &rFillRect, CreateSolidBrush(GetSysColor(m_icolour)));
  }
  else { 
    // Don't check IsThemeBackgroundPartiallyTransparent because it return false for BP_CHECKBOX
    DrawThemeParentBackground(hWnd, hMemDC, &rFillRect);
  }

  RECT rIconRect = {0, 0, 13, nHeight};
  DrawThemeBackground(hTheme, hMemDC, m_type == BS_AUTOCHECKBOX ? BP_CHECKBOX : BP_RADIOBUTTON,
                      nStateID, &rIconRect, NULL);
  CloseThemeData(hTheme);

  RECT rTextRect = {16, 0, nWidth - 16, nHeight};
  SetBkMode(hMemDC, TRANSPARENT);
  if (m_bUseTextColour)
    SetTextColor(hMemDC, m_crfText);

  SelectObject(hMemDC, (HFONT)GetStockObject(DEFAULT_GUI_FONT));

  if (m_caption.IsEmpty()) {
    GetWindowText(m_caption);
    SetWindowText(L"");
  }

  DrawText(hMemDC, m_caption, m_caption.GetLength(), &rTextRect, DT_SINGLELINE | DT_VCENTER);

  if (fFocus){
    DrawText(hMemDC, m_caption, m_caption.GetLength(), &rTextRect, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
    rTextRect.left--;
    rTextRect.right++;
    DrawFocusRect(hMemDC, &rTextRect);
  }

  BitBlt(hDC, 0, 0, nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

  DeleteObject(hBitmap);
  DeleteDC(hMemDC);
}

/////////////////////////////////////////////////////////////////////////////
// CButtonExtn

CButtonBitmapExtn::CButtonBitmapExtn()
{
}

CButtonBitmapExtn::~CButtonBitmapExtn()
{
}

BEGIN_MESSAGE_MAP(CButtonBitmapExtn, CButton)
  //{{AFX_MSG_MAP(CButtonBitmapExtn)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CButtonBitmapExtn::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  CDC dc;
  dc.Attach(lpDrawItemStruct->hDC);

  CBitmap bmp;
  bmp.LoadBitmap(m_IDB);

  BITMAP bitMapInfo;
  bmp.GetBitmap(&bitMapInfo);

  CDC memDC;
  memDC.CreateCompatibleDC(&dc);

  memDC.SelectObject(&bmp);
  int bmw = bitMapInfo.bmWidth;
  int bmh = bitMapInfo.bmHeight;

  // Draw button image transparently
  ::TransparentBlt(dc.GetSafeHdc(), 0, 0, bmw, bmh, memDC.GetSafeHdc(), 0, 0, bmw, bmh, m_cfMAsk);
}

void FixBitmapBackground(CBitmap &bm)
{
  // Change bitmap's {192,192,192} pixels
  // to current flavor of the month default background

  // Get how many pixels in the bitmap
  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  BITMAP bmInfo;
  int rc = bm.GetBitmap(&bmInfo);

  if (rc == 0) {
    ASSERT(0);
    return;
  }
  const UINT numPixels(bmInfo.bmHeight * bmInfo.bmWidth);

  // get a pointer to the pixels
  DIBSECTION ds;
  VERIFY(bm.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));

  RGBTRIPLE *pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
  if (pixels == NULL) {
    ASSERT(0);
    return;
  }

  const RGBTRIPLE newbkgrndColourRGB = {GetBValue(crCOLOR_3DFACE),
                                        GetGValue(crCOLOR_3DFACE),
                                        GetRValue(crCOLOR_3DFACE)};

  for (UINT i = 0; i < numPixels; ++i) {
    if (pixels[i].rgbtBlue  == 192 &&
        pixels[i].rgbtGreen == 192 &&
        pixels[i].rgbtRed   == 192) {
      pixels[i] = newbkgrndColourRGB;
    }
  }
}
