/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "PWStatusBar.h"
#include "resource.h"

// CPWStatusBar

IMPLEMENT_DYNAMIC(CPWStatusBar, CStatusBar)

CPWStatusBar::CPWStatusBar()
  : m_bFilterStatus(false)
{
  m_FilterBitmap.LoadBitmap(IDB_FILTER_ACTIVE);
  BITMAP bm;

  m_FilterBitmap.GetBitmap(&bm);
  m_bmWidth = bm.bmWidth;
  m_bmHeight = bm.bmHeight;
}

CPWStatusBar::~CPWStatusBar()
{
  m_FilterBitmap.DeleteObject();
}

BEGIN_MESSAGE_MAP(CPWStatusBar, CStatusBar)
  //{{AFX_MSG_MAP(CPWStatusBar)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWStatusBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  switch (lpDrawItemStruct->itemID) {
    case SB_FILTER:
      // Attach to a CDC object
      CDC dc;
      dc.Attach(lpDrawItemStruct->hDC);

      // Get the pane rectangle and calculate text coordinates
      CRect rect(&lpDrawItemStruct->rcItem);
      if (m_bFilterStatus) {
        // Centre bitmap in pane.
        int ileft = rect.left +  rect.Width() / 2 - m_bmWidth / 2;
        int itop = rect.top +  rect.Height() / 2 - m_bmHeight / 2;

        CBitmap* pBitmap = &m_FilterBitmap;
        CDC srcDC; // select current bitmap into a compatible CDC
        srcDC.CreateCompatibleDC(NULL);
        CBitmap* pOldBitmap = srcDC.SelectObject(pBitmap);
        dc.BitBlt(ileft, itop, m_bmWidth, m_bmHeight,
                  &srcDC, 0, 0, SRCCOPY); // BitBlt to pane rect
        srcDC.SelectObject(pOldBitmap);
      } else {
        dc.FillSolidRect(&rect, ::GetSysColor(COLOR_BTNFACE));
      }
      // Detach from the CDC object, otherwise the hDC will be
      // destroyed when the CDC object goes out of scope
      dc.Detach();

      return;
  }

  CStatusBar::DrawItem(lpDrawItemStruct);
}
