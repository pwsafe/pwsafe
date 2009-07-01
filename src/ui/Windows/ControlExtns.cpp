/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ControlExtns.cpp
//

#include "stdafx.h"
#include "ControlExtns.h"
#include "corelib/ItemField.h" // for CSecEditExtn
#include "corelib/BlowFish.h"  // ditto
#include "corelib/PWSrand.h"   // ditto
#include "resource2.h" // for CEditExtn context menu
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

/////////////////////////////////////////////////////////////////////////////
// CStaticExtn

CStaticExtn::CStaticExtn()
  : m_bUserColour(false), m_bMouseInWindow(false), 
  m_iFlashing(0), m_bHighlight(false)
{
}

CStaticExtn::~CStaticExtn()
{
}

BEGIN_MESSAGE_MAP(CStaticExtn, CStatic)
  //{{AFX_MSG_MAP(CStaticExtn)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_MOUSEMOVE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CStaticExtn::FlashBkgnd(COLORREF cfFlashColour)
{
  // Set flash colour
  m_cfFlashColour = cfFlashColour;
  m_iFlashing = 1;
  // Cause repaint
  Invalidate();
  UpdateWindow();
  // Sleep to give the impression of a flash
  Sleep(200);
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
  if (vmenu_items.size() == 0) {
    m_vmenu_items.clear();
  } else {
    m_vmenu_items = vmenu_items;
  }
}

CEditExtn::~CEditExtn()
{
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

struct equal_cmd {
  equal_cmd(int const& nCmd) : m_nCmd(nCmd) {}
  bool operator()(st_context_menu const& context_menu) const
  {
    return (context_menu.message_number == m_nCmd);
  }
private:
  int m_nCmd;
};

void CEditExtn::OnContextMenu(CWnd* pWnd, CPoint point)
{
  if (m_vmenu_items.size() == 0) {
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
      menu.InsertMenu(i + iPos, MF_BYPOSITION | m_vmenu_items[i].flags,
                      m_vmenu_items[i].message_number,
                      m_vmenu_items[i].menu_string.c_str());
  }

  if (point.x == -1 || point.y == -1) {
    CRect rc;
    GetClientRect(&rc);
    point = rc.CenterPoint();
    ClientToScreen(&point);
  }

  UINT nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
                                 TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                 point.x, point.y, this);

  if (nCmd == 0)
    return;

  std::vector<st_context_menu>::iterator iter;
  iter = std::find_if(m_vmenu_items.begin(), m_vmenu_items.end(),
                      equal_cmd(nCmd));
  if (iter != m_vmenu_items.end()) {
    this->GetParent()->SendMessage(nCmd);
    return;
  }

  switch (nCmd) {
    case EM_UNDO:
    case WM_CUT:
    case WM_COPY:
    case WM_CLEAR:
    case WM_PASTE:
      SendMessage(nCmd);
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

  pt.y += ::GetSystemMetrics(SM_CYCURSOR); // height of cursor

  m_pLBToolTips->SetWindowPos(&wndTopMost, pt.x, pt.y, 0, 0,
                              SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );

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

HBRUSH CComboBoxExtn::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
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

  return CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
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
// CStaticExtn is meant for sensitive information that you really don't
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
    memset(key, 0, sizeof(key));
  }

  ~Impl() {delete m_bf;}
  CItemField m_field;
  BlowFish *m_bf;
};

CSecEditExtn::CSecEditExtn()
  : CEditExtn((RGB(255, 222, 222))), // light red
    m_impl(new Impl), m_secure(true), m_in_recursion(false)
{
}

CSecEditExtn::CSecEditExtn(std::vector<st_context_menu> vmenu_items)
  : CEditExtn(vmenu_items, (RGB(255, 222, 222))),
    m_impl(new Impl), m_secure(true), m_in_recursion(false)
{
}

BEGIN_MESSAGE_MAP(CSecEditExtn, CEditExtn)
  //{{AFX_MSG_MAP(CSecEditExtn)
  ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSecEditExtn::~CSecEditExtn()
{
  delete m_impl;
}

void CSecEditExtn::SetSecure(bool on_off)
{
  m_secure = on_off;
}

CSecString CSecEditExtn::GetSecureText() const
{
  CSecString retval;
  StringX sval;
  m_impl->m_field.Get(sval, m_impl->m_bf);
  retval = sval.c_str();
  return retval;
}

void CSecEditExtn::SetSecureText(const CSecString &str)
{
  m_impl->m_field.Set(str, m_impl->m_bf);
  if (::IsWindow(m_hWnd)) {
    if (!m_secure)
      SetWindowText(str);
    else {
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
  TRACE(L"CSecEditExtn::OnUpdate(%s)\n",dstr);
#endif
  if (m_secure) {
    if (!m_in_recursion)
      OnSecureUpdate();
  } else {
    CSecString str;
    GetWindowText(str);
    m_impl->m_field.Set(str, m_impl->m_bf);
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
        for (int i = 0; i < new_len; i++)
          str += new_str[i] == FILLER ? old_str[i] : new_str[i];
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
  TRACE(L"CSecEditExtn::OnSecureUpdate: GetSel(%d, %d), str = %s\n",
        startSel, endSel, str);
#endif
  SetSecureText(str);
  SetSel(startSel, endSel); // need to restore after Set.
}
