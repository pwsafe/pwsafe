/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// GridListCtrl.cpp : implementation file
//

/*
* Courtesy of Zafir Anjum from www.codeguru.com 6 August 1998
* "Drawing horizontal and vertical gridlines" but now made
* a separate class.
* Updated due to comments (pen colour) and adjustable columnns.
* Also support our non-standard Header control ID
*/

#include "stdafx.h"
#include "GridListCtrl.h"

// CGridListCtrl

IMPLEMENT_DYNAMIC(CGridListCtrl, CListCtrl)

CGridListCtrl::CGridListCtrl()
  : m_HeaderCtrlID(0), m_RGBLineColour(RGB(112, 112, 112))
{
}

CGridListCtrl::~CGridListCtrl()
{
}

BEGIN_MESSAGE_MAP(CGridListCtrl, CListCtrl)
  ON_WM_PAINT()
END_MESSAGE_MAP()

// CGridListCtrl message handlers

void CGridListCtrl::PreSubclassWindow()
{
  CListCtrl::PreSubclassWindow();

  // Remove grid lines as this class adds them
  SetExtendedStyle(GetExtendedStyle() & (~LVS_EX_GRIDLINES));
}

void CGridListCtrl::OnPaint()
{
  // Make the gridlines easier to see than default light grey
  // First let the control do its default drawing.
  const MSG *pMsg = GetCurrentMessage();
  DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);

  // Draw the lines only for LVS_REPORT mode
  if ((GetStyle() & LVS_TYPEMASK) == LVS_REPORT) {
    CClientDC dc(this);
    CPen NewPen(PS_SOLID, 0, m_RGBLineColour);
    CPen *pOldPen = dc.SelectObject(&NewPen);

    // Get the number of columns
    CHeaderCtrl *pHeader = (CHeaderCtrl *)GetDlgItem(m_HeaderCtrlID);
    int nColumnCount = pHeader->GetItemCount();

    // The bottom of the header corresponds to the top of the line
    RECT rect;
    pHeader->GetClientRect(&rect);
    int top = rect.bottom;

    // Now get the client rect so we know the line length and when to stop
    GetClientRect(&rect);

    // The border of the column is offset by the horz scroll
    int borderx = 0 - GetScrollPos(SB_HORZ);
    for (int i = 0; i < nColumnCount; i++) {
      // Get the next border
      borderx += GetColumnWidth(pHeader->OrderToIndex(i));

      // if next border is outside client area, break out
      if (borderx >= rect.right) break;

      // Draw the line.
      dc.MoveTo(borderx - 1, top);
      dc.LineTo(borderx - 1, rect.bottom);
    }

    // Draw the horizontal grid lines
    // First get the height
    if (!GetItemRect(0, &rect, LVIR_BOUNDS))
      return;

    int height = rect.bottom - rect.top;

    GetClientRect(&rect);
    int width = rect.right;

    for (int i = 1; i <= GetCountPerPage(); i++) {
      dc.MoveTo(0, top + height * i);
      dc.LineTo(width, top + height * i);
    }

    dc.SelectObject(pOldPen);
  }
}
