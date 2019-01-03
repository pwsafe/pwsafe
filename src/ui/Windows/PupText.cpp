/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
////////////////////////////////////////////////////////////////
// MSDN Magazine -- November 2003
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio .NET on Windows XP. Tab size=3.
//
#include "stdafx.h"
#include "puptext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPopupText,CWnd)

BEGIN_MESSAGE_MAP(CPopupText,CWnd)
  ON_WM_NCHITTEST()
  ON_WM_PAINT()
  ON_MESSAGE(WM_SETTEXT, OnSetText)
  ON_WM_TIMER()
END_MESSAGE_MAP()

CPopupText::CPopupText()
{
  CNonClientMetrics ncm;
  m_font.CreateFontIndirect(&ncm.lfMenuFont);
  m_szMargins = CSize(4, 4);
}

CPopupText::~CPopupText()
{
  m_font.DeleteObject();
}

//////////////////
// Create window. pt is upper left corner
//
int CPopupText::Create(CPoint pt, CWnd* pParentWnd, UINT nID)
{
  return CreateEx(0,
                  NULL,
                  NULL,
                  WS_POPUP|WS_VISIBLE,
                  CRect(pt,CSize(0, 0)),
                  pParentWnd,
                  nID);
}

//////////////////
// Text changed: resize window to fit
//
LRESULT CPopupText::OnSetText(WPARAM , LPARAM lp)
{
  CRect rc;
  const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  GetWindowRect(&rc);
  int h = rc.Height();
  CClientDC dc(this);
  DrawText(dc, CString((LPCWSTR)lp), rc, DT_CALCRECT); //  | DT_PATH_ELLIPSIS
  int w = rc.Width();
  rc.InflateRect(m_szMargins);
  if (m_szMargins.cy)
    h = rc.Height();
  if ((rc.left + w + 10) > screenWidth)
    w = screenWidth - rc.left - 10;
  SetWindowPos(NULL, 0, 0, w, h,
    SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
  return Default();
}

void CPopupText::DrawText(CDC& dc, LPCWSTR lpText, CRect& rc, UINT flags)
{
  CBrush b(GetSysColor(COLOR_INFOBK)); // use tooltip bg color
  dc.FillRect(&rc, &b);
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(GetSysColor(COLOR_INFOTEXT)); // tooltip text color
  CFont* pOldFont = dc.SelectObject(&m_font);
  if (flags != DT_CALCRECT)
    dc.DrawText(lpText, &rc, flags | DT_PATH_ELLIPSIS);
  else
    dc.DrawText(lpText, &rc, flags);
  dc.SelectObject(pOldFont);
}

//////////////////
// Paint text using system colors
//
void CPopupText::OnPaint()
{
  CRect rc;
  GetClientRect(&rc);
  CString s;
  GetWindowText(s);
  CPaintDC dc(this);
  DrawText(dc, s, rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}

//////////////////
// Register class if needed
//
BOOL CPopupText::PreCreateWindow(CREATESTRUCT& cs)
{
  static CString sClassName;
  if (sClassName.IsEmpty())
    sClassName = AfxRegisterWndClass(0);
  cs.lpszClass = sClassName;
  cs.style = WS_POPUP | WS_BORDER;
  cs.dwExStyle |= WS_EX_TOOLWINDOW;
  return CWnd::PreCreateWindow(cs);
}

//////////////////
// CPopupText is intended to be used on the stack,
// not heap, so don't auto-delete.
//
void CPopupText::PostNcDestroy()
{
  // don't delete this
}

//////////////////
// Show window with delay. No delay means show now.
//
void CPopupText::ShowDelayed(UINT msec)
{
  if (msec == 0) {
    // no delay: show it now
    OnTimer(TIMER_PUPTEXT);
  } else {
    // delay: set time
    SetTimer(TIMER_PUPTEXT, msec, NULL);
  }
}

//////////////////
// Cancel text: kill timer and hide window
//
void CPopupText::Cancel()
{
  if (m_hWnd) {
    KillTimer(TIMER_PUPTEXT);
    ShowWindow(SW_HIDE);
  }
}

//////////////////
// Timer popped: display myself and kill timer
//
void CPopupText::OnTimer(UINT_PTR )
{
  ShowWindow(SW_SHOWNA);
  Invalidate();
  UpdateWindow();
  KillTimer(TIMER_PUPTEXT);
}
