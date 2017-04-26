/*
* Copyright (c) 2009-2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// VKBButton.cpp
//

#include "../stdafx.h"
#include "VKBButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Special Flat button for Virtual Keyboards
// Also, if a Push button, will show pushed state by change of colour (unless disabled)

const COLORREF crefGreen  = (RGB(222, 255, 222));  // Light green
const COLORREF crefYellow = (RGB(255, 255, 228));  // Very light yellow
const COLORREF crefOrange = (RGB(255, 208, 192));  // Light Orange
const COLORREF crefPink   = (RGB(255, 222, 222));  // Light Pink

CVKBButton::CVKBButton()
  : m_bMouseInWindow(false), m_bDeadKey(false),
  m_bFlat(true), m_bPushed(false), m_bChangePushColour(true)
{
}

CVKBButton::~CVKBButton()
{
}

BEGIN_MESSAGE_MAP(CVKBButton, CButton)
  //{{AFX_MSG_MAP(CVKBButton)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_MOUSEMOVE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVKBButton message handlers

void CVKBButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  CDC* pDC   = CDC::FromHandle(lpDrawItemStruct->hDC);
  CRect rect = lpDrawItemStruct->rcItem;
  UINT state = lpDrawItemStruct->itemState;

  CString strText;
  GetWindowText(strText);

  // draw the control edges (DrawFrameControl is handy!)
  if (state & ODS_SELECTED)
    pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED |
                                (m_bFlat ? DFCS_FLAT : 0));
  else
    pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH |
                                (m_bFlat ? DFCS_FLAT : 0));

  // Fill the interior colour if necessary
  rect.DeflateRect(CSize(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE)));
  if (m_bDeadKey) {
    pDC->FillSolidRect(rect, crefOrange);
  } else {
    COLORREF crefColour;
    if (m_bPushed)
      crefColour = m_bMouseInWindow ? crefGreen : (m_bChangePushColour ? crefPink : crefYellow);
    else
      crefColour = m_bMouseInWindow ? crefGreen : crefYellow;

    pDC->FillSolidRect(rect, crefColour);
  }

  // Draw the text
  if (!strText.IsEmpty()) {
    CSize Extent = pDC->GetTextExtent(strText);
    CPoint pt(rect.CenterPoint().x - Extent.cx / 2, rect.CenterPoint().y - Extent.cy / 2);

    if (state & ODS_SELECTED)
      pt.Offset(1, 1);

    int nMode = pDC->SetBkMode(TRANSPARENT);

    if (state & ODS_DISABLED)
      pDC->DrawState(pt, Extent, strText, DSS_DISABLED, TRUE, 0, (HBRUSH)NULL);
    else
      pDC->TextOut(pt.x, pt.y, strText);

    pDC->SetBkMode(nMode);
  }
}

void CVKBButton::PreSubclassWindow()
{
  CButton::PreSubclassWindow();

  ModifyStyle(0, BS_OWNERDRAW);  // make the button owner drawn
}

void CVKBButton::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!m_bMouseInWindow) {
    m_bMouseInWindow = true;
    Invalidate();
    UpdateWindow();

    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0};
    VERIFY(TrackMouseEvent(&tme));
  }

  CButton::OnMouseMove(nFlags, point);
}

LRESULT CVKBButton::OnMouseLeave(WPARAM, LPARAM)
{
  m_bMouseInWindow = false;
  // Reset background
  Invalidate();
  UpdateWindow();

  return 0L;
}
