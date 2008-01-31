/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ControlExtns.cpp
//

#include "stdafx.h"
#include "ControlExtns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Right-click Edit Context Menu
#define MENUSTRING_UNDO        _T("&Undo")
// Separator
#define MENUSTRING_CUT         _T("Cu&t")
#define MENUSTRING_COPY        _T("&Copy")
#define MENUSTRING_PASTE       _T("&Paste")
#define MENUSTRING_DELETE      _T("&Delete")
// Separator
#define MENUSTRING_SELECTALL   _T("Select &All")
// Separator
// Custom menu goes here!

#if defined(UNICODE)
#define EDIT_CLIPBOARD_TEXT_FORMAT  CF_UNICODETEXT
#else
#define EDIT_CLIPBOARD_TEXT_FORMAT  CF_TEXT
#endif

const COLORREF crefInFocus = (RGB(222, 255, 222));  // Light green
const COLORREF crefNoFocus = (RGB(255, 255, 255));  // White
const COLORREF crefBlack   = (RGB(  0,   0,   0));  // Black

/////////////////////////////////////////////////////////////////////////////
// CStaticExtn

CStaticExtn::CStaticExtn()
  : m_bUserColour(FALSE)
{
}

CStaticExtn::~CStaticExtn()
{
}

BEGIN_MESSAGE_MAP(CStaticExtn, CStatic)
  //{{AFX_MSG_MAP(CStaticExtn)
  ON_WM_CTLCOLOR_REFLECT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

HBRUSH CStaticExtn::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
  if (!this->IsWindowEnabled() || !m_bUserColour)
    return NULL;

  pDC->SetTextColor(m_cfUser);
  pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
  return GetSysColorBrush(COLOR_BTNFACE);
}

/////////////////////////////////////////////////////////////////////////////
// CEditExtn

CEditExtn::CEditExtn()
  : m_bIsFocused(FALSE), m_lastposition(-1),
  m_message_number(-1), m_menustring("")
{
  m_brInFocus.CreateSolidBrush(crefInFocus);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);
}

CEditExtn::CEditExtn(int message_number, LPCTSTR menustring)
  : m_bIsFocused(FALSE), m_lastposition(-1),
  m_message_number(message_number), m_menustring(menustring)
{
  m_brInFocus.CreateSolidBrush(crefInFocus);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);
  // Don't allow if menu string is empty.
  if (m_menustring.IsEmpty())
    m_message_number = -1;
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

HBRUSH CEditExtn::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
  if (!this->IsWindowEnabled())
    return NULL;

  pDC->SetTextColor(crefBlack);
  if (m_bIsFocused == TRUE) {
    pDC->SetBkColor(crefInFocus);
    return m_brInFocus;
  } else {
    pDC->SetBkColor(crefNoFocus);
    return m_brNoFocus;
  }
}

void CEditExtn::OnContextMenu(CWnd* pWnd, CPoint point)
{
  if (m_message_number < 0) {
    CEdit::OnContextMenu(pWnd, point);
    return;
  }

  SetFocus();
  CMenu menu;
  menu.CreatePopupMenu();

  BOOL bReadOnly = GetStyle() & ES_READONLY;
  DWORD flags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED;

  menu.InsertMenu(0, MF_BYPOSITION | flags, EM_UNDO, MENUSTRING_UNDO);

  menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);

  DWORD sel = GetSel();
  flags = LOWORD(sel) == HIWORD(sel) ? MF_GRAYED : 0;
  // Add it in position 2 but adding the next will make it 3
  menu.InsertMenu(2, MF_BYPOSITION | flags, WM_COPY, MENUSTRING_COPY);

  flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
  menu.InsertMenu(2, MF_BYPOSITION | flags, WM_CUT, MENUSTRING_CUT);

  flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
  // Add it in position 4 but adding the next will make it 5
  menu.InsertMenu(4, MF_BYPOSITION | flags, WM_CLEAR, MENUSTRING_DELETE);

  flags = IsClipboardFormatAvailable(EDIT_CLIPBOARD_TEXT_FORMAT) &&
                                     !bReadOnly ? 0 : MF_GRAYED;
  menu.InsertMenu(4, MF_BYPOSITION | flags, WM_PASTE, MENUSTRING_PASTE);

  menu.InsertMenu(6, MF_BYPOSITION | MF_SEPARATOR);

  int len = GetWindowTextLength();
  flags = (!len || (LOWORD(sel) == 0 && HIWORD(sel) == len)) ? MF_GRAYED : 0;

  menu.InsertMenu(7, MF_BYPOSITION | flags, EM_SELECTALL, MENUSTRING_SELECTALL);

  menu.InsertMenu(8, MF_BYPOSITION | MF_SEPARATOR);

  menu.InsertMenu(9, MF_BYPOSITION , m_message_number, m_menustring);

  if (point.x == -1 || point.y == -1) {
    CRect rc;
    GetClientRect(&rc);
    point = rc.CenterPoint();
    ClientToScreen(&point);
  }

  int nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
                                 TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                 point.x, point.y, this);

  if (nCmd < 0)
    return;

  if (nCmd == m_message_number) {
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

/////////////////////////////////////////////////////////////////////////////
// CListBoxExtn

CListBoxExtn::CListBoxExtn() : m_bIsFocused(FALSE)
{
  m_brInFocus.CreateSolidBrush(crefInFocus);
  m_brNoFocus.CreateSolidBrush(crefNoFocus);
}

CListBoxExtn::~CListBoxExtn()
{
}

BEGIN_MESSAGE_MAP(CListBoxExtn, CListBox)
  //{{AFX_MSG_MAP(CListBoxExtn)
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_CTLCOLOR_REFLECT()
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

/////////////////////////////////////////////////////////////////////////////
// CComboBoxExtn

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
    if (m_listbox.GetSafeHwnd() == NULL)
      m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
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
