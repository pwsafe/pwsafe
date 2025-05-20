/*
* Copyright (c) 2025 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "StrengthMeterCtrl.h"
#include <algorithm>

IMPLEMENT_DYNAMIC(CStrengthMeterCtrl, CProgressCtrl)

BEGIN_MESSAGE_MAP(CStrengthMeterCtrl, CProgressCtrl)
    ON_WM_PAINT()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CStrengthMeterCtrl::CStrengthMeterCtrl()
    : m_nStrength(0)
    , m_crWeak(RGB(255, 0, 0))      // Red for weak
    , m_crMedium(RGB(255, 165, 0))  // Orange for medium
    , m_crStrong(RGB(0, 255, 0))    // Green for strong
{
}

CStrengthMeterCtrl::~CStrengthMeterCtrl()
{
}

void CStrengthMeterCtrl::SetStrength(int nStrength)
{
    // Ensure strength is within valid range
    m_nStrength = std::max(0, std::min(100, nStrength));
    
    // Update the progress control
    SetPos(m_nStrength);
    
    // Force a redraw
    Invalidate();
}

int CStrengthMeterCtrl::GetStrength() const
{
    return m_nStrength;
}

void CStrengthMeterCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    // Get the current position
    int nPos = GetPos();
    
    // Determine the color based on strength
    COLORREF crColor;
    if (nPos < 33)
        crColor = m_crWeak;
    else if (nPos < 66)
        crColor = m_crMedium;
    else
        crColor = m_crStrong;

    // Create a brush with the appropriate color
    CBrush brush(crColor);
    dc.FillRect(&rect, &brush);
}

HBRUSH CStrengthMeterCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CProgressCtrl::OnCtlColor(pDC, pWnd, nCtlColor);
    
    // Set the text color to black for better visibility
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);
    
    return hbr;
} 