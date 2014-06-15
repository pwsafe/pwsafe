/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// VKBButton.cpp
//

/*

  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!

*/

#include "../stdafx.h"

#include "VKBButton.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

extern std::wstring get_window_text(HWND hWnd);

// Special Flat button for Virtual Keyboards
// Also, if a Push button, will show pushed state by change of colour (unless disabled)

const COLORREF crefGreen  = (RGB(222, 255, 222));  // Light green
const COLORREF crefYellow = (RGB(255, 255, 228));  // Very light yellow
const COLORREF crefOrange = (RGB(255, 208, 192));  // Light Orange
const COLORREF crefPink   = (RGB(255, 222, 222));  // Light Pink

CVKBButton::CVKBButton()
  : /*m_bMouseInWindow(false), */ m_bDeadKey(false),
  m_bFlat(true), m_bPushed(false), m_bChangePushColour(true)
{
}

CVKBButton::~CVKBButton()
{
}

/////////////////////////////////////////////////////////////////////////////
// CVKBButton message handlers

void CVKBButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  BOOL brc;
  const HDC hdc = lpDrawItemStruct->hDC;
  const UINT state = lpDrawItemStruct->itemState;
  RECT rect = lpDrawItemStruct->rcItem;

  wstring stxt;
  stxt = get_window_text(m_hWnd);

  // draw the control edges (DrawFrameControl is handy!)
  if (state & ODS_SELECTED) {
    brc = DrawFrameControl(hdc, &rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED |
    (m_bFlat ? DFCS_FLAT : 0));
  }
  else
  {
    brc = DrawFrameControl(hdc, &rect, DFC_BUTTON, DFCS_BUTTONPUSH |
    (m_bFlat ? DFCS_FLAT : 0));
    ASSERT(brc);
  }

  // Fill the interior colour if necessary
  const SIZE sz = { GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE) };

  rect.bottom -= sz.cy;
  rect.right -= sz.cx;

  rect.top += sz.cy;
  rect.left += sz.cx;

  if (m_bDeadKey) {
    brc = FillRect(hdc, &rect, CreateSolidBrush(crefOrange));
    ASSERT(brc);
  } else {
    COLORREF crefColour;
    DWORD mdw = GetMessagePos();
    POINT mpt = { GET_X_LPARAM(mdw), GET_Y_LPARAM (mdw) };  // In Screen co-ordinates

    RECT wrect;
    GetWindowRect(m_hWnd, &wrect); // In Screen co-ordinates
    BOOL bMouseInWindow = PtInRect(&wrect, mpt);

    if (m_bPushed)
      crefColour = bMouseInWindow ? crefGreen : (m_bChangePushColour ? crefPink : crefYellow);
    else
      crefColour = bMouseInWindow ? crefGreen : crefYellow;

    brc = FillRect(hdc, &rect, CreateSolidBrush(crefColour));
    ASSERT(brc);
  }

  // Draw the text
  if (!stxt.empty()) {
    RECT rExtent;
    SIZE szExtent;
    DrawText(hdc, stxt.c_str(), -1, &rExtent, DT_CALCRECT);
    szExtent.cx = abs(rExtent.left - rExtent.right);
    szExtent.cy = abs(rExtent.top - rExtent.bottom);

    POINT pt = { ((rect.left + rect.right) - szExtent.cx) / 2,
      ((rect.top + rect.bottom) - szExtent.cy) / 2 };

    if (state & ODS_SELECTED)
    {
      pt.x += 1;
      pt.y += 1;
    }

    int nMode = SetBkMode(hdc, TRANSPARENT);

    if (state & ODS_DISABLED)
    {
      brc = DrawState(hdc, (HBRUSH)NULL, NULL, (LPARAM)stxt.c_str(), NULL, pt.x, pt.y, szExtent.cx, szExtent.cy, DST_TEXT | DSS_DISABLED);
      ASSERT(brc);
    }
    else
    {
      brc = TextOut(hdc, pt.x, pt.y, stxt.c_str(), stxt.length());
      ASSERT(brc);
    }

    SetBkMode(hdc, nMode);
  }
}

