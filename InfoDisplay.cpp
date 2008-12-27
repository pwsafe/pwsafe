/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
*  This is nearly an exact copy of CInfoDisplay class in Asynch Explorer by
*  Joseph M. Newcomer [MVP]; http://www.flounder.com
*  Only minor formatting and naming changes have been made to fit in with this
*  project.
*/

// InfoDisplay.cpp : implementation file
//

#include "stdafx.h"
#include "InfoDisplay.h"

// CInfoDisplay

IMPLEMENT_DYNAMIC(CInfoDisplay, CWnd)
CInfoDisplay::CInfoDisplay()
{
}

CInfoDisplay::~CInfoDisplay()
{
  ::DeleteObject(m_font);
}

BEGIN_MESSAGE_MAP(CInfoDisplay, CWnd)
  ON_MESSAGE(WM_SETFONT, OnSetFont)
  ON_MESSAGE(WM_GETFONT, OnGetFont)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CInfoDisplay message handlers

void CInfoDisplay::OnPaint()
{
  CPaintDC dc(this); // device context for painting

  CFont *f = GetFont();
  dc.SelectObject(f);

  // First, we compute the maximum line width, and set the rectangle wide enough to
  // hold this.  Then we use DrawText/DT_CALCRECT to compute the height
  CString text;
  GetWindowText(text);
  CSize box = CSize(0, 0);

  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm); 

  int inf; // inflation factor

  { /* compute box size */
    CString s = text;
    while(TRUE) { /* scan string */
      CString line;
      int p = s.Find(_T("\n"));
      if (p < 0)
        line = s;
      else { /* one line */
        line = s.Left(p);
        s = s.Mid(p + 1);
      } /* one line */
      CSize sz = dc.GetTextExtent(line);
      box.cx = max(box.cx, sz.cx);
      box.cy += tm.tmHeight + tm.tmInternalLeading;
      if (p < 0)
        break;
    } /* scan string */

    // Having computed the width, allow for the borders and extra space for margins
    inf = 4 * ::GetSystemMetrics(SM_CXBORDER);
    box.cx += 2 * inf;
    box.cy += 2 * inf;

    CRect r(0, 0, 0, 0);
    r.right = box.cx;
    r.bottom = box.cy;

    // Set the window size
    SetWindowPos(NULL, 0, 0, r.Width(), r.Height(), 
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  } /* compute box size */     

  CRect r;
  GetClientRect(&r);
  r.InflateRect(-inf, -inf);

  dc.SetBkMode(TRANSPARENT);
   
  CString s = text;
  int y = r.top;
  while(TRUE) { /* scan string */
    CString line;
    int p = s.Find(_T("\n"));
    if (p < 0)
      line = s;
    else { /* one line */
      line = s.Left(p);
      s = s.Mid(p + 1);
    } /* one line */
    dc.TextOut(r.left, y, line);
    y += tm.tmHeight + tm.tmInternalLeading;
    if (p < 0)
      break;
  } /* scan string */
  // Do not call CWnd::OnPaint() for painting messages
}

BOOL CInfoDisplay::OnEraseBkgnd(CDC* pDC)
{
  CRect r;
  GetClientRect(&r);

  pDC->FillSolidRect(&r, ::GetSysColor(COLOR_INFOBK));

  return TRUE;
}

BOOL CInfoDisplay::Create(int /* x */, int /* y */, LPCTSTR sztext, CWnd * parent)
{
  ASSERT(parent != NULL);
  if (parent == NULL)
    return FALSE;
   
  LPCTSTR NDclass = AfxRegisterWndClass(0,    // no class styles
                                        NULL, // default arrow cursor
                                        (HBRUSH)(COLOR_INFOBK + 1),
                                        NULL);
  ASSERT(NDclass != NULL);
  if (NDclass == NULL)
    return FALSE;
   
  CFont *f = parent->GetFont();
   
  CRect r(0, 0, 10, 10); // meaningless values, will be recomputed after creation

  BOOL result = CreateEx(WS_EX_NOACTIVATE,   // extended styles
                         NDclass,
                         sztext,
                         /* not WS_VISIBLE */ WS_POPUP | WS_BORDER,
                         r,
                         parent,
                         NULL);
  ASSERT(result);
  if (!result)
    return FALSE;

  SetFont(f);

  return TRUE;
}

void CInfoDisplay::PostNcDestroy()
{
  CWnd::PostNcDestroy();
  delete this;
}

LRESULT CInfoDisplay::OnSetFont(WPARAM wParam, LPARAM lParam)
{
  m_font = (HFONT)wParam;
  if (LOWORD(lParam)) { /* force redraw */
    Invalidate();
    UpdateWindow();
  } /* force redraw */

  return 0;
}

LRESULT CInfoDisplay::OnGetFont(WPARAM, LPARAM)
{
  return (LRESULT)m_font;
}
