/*
* Copyright (c) 2025 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "StrengthMeterCtrl.h"
#include "resource3.h"
#include <algorithm>

IMPLEMENT_DYNAMIC(CStrengthMeterCtrl, CProgressCtrl)

BEGIN_MESSAGE_MAP(CStrengthMeterCtrl, CProgressCtrl)
    ON_WM_PAINT()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CStrengthMeterCtrl::CStrengthMeterCtrl()
    : m_nStrength(0.0)
    , m_crWeak(RGB(255, 0, 0))      // Red for weak
    , m_crMedium(RGB(255, 165, 0))  // Orange for medium
    , m_crStrong(RGB(0, 255, 0))    // Green for strong
{
}

CStrengthMeterCtrl::~CStrengthMeterCtrl()
{
}

void CStrengthMeterCtrl::SetStrength(double nStrength)
{
    // Ensure strength is within valid range
    m_nStrength = std::max(0.0, std::min(100.0, nStrength));
    
    // Update the progress control
    SetPos(static_cast<int>(m_nStrength));
    
    // Update tooltip text
    CString tipText;
    UINT nID = 0;
    if (m_nStrength < 33.0)
        nID = IDS_PWSTRNGTH_WEAK;
    else if (m_nStrength < 66.0)
        nID = IDS_PWSTRNGTH_MEDIUM;
    else
        nID = IDS_PWSTRNGTH_STRONG;
    if (nID != 0)
        tipText.LoadString(nID);
    if (m_tooltip.GetSafeHwnd())
        m_tooltip.UpdateTipText(tipText, this);
    
    // Force a redraw
    Invalidate();
}

void CStrengthMeterCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    int nPos = GetPos();

    // Determine the color based on strength
    COLORREF crColor;
    if (nPos < 33)
        crColor = m_crWeak;
    else if (nPos < 66)
        crColor = m_crMedium;
    else
        crColor = m_crStrong;

    // Calculate the width to fill
    int fillWidth = rect.Width() * nPos / 100;

    // Draw the filled (strength) part
    if (fillWidth > 0) {
        CRect fillRect = rect;
        fillRect.right = fillRect.left + fillWidth;
        CBrush fillBrush(crColor);
        dc.FillRect(&fillRect, &fillBrush);
    }

    // Draw the remaining (background) part
    if (fillWidth < rect.Width()) {
        CRect bgRect = rect;
        bgRect.left += fillWidth;
        CBrush bgBrush(GetSysColor(COLOR_3DFACE));
        dc.FillRect(&bgRect, &bgBrush);
    }
}

HBRUSH CStrengthMeterCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CProgressCtrl::OnCtlColor(pDC, pWnd, nCtlColor);
    
    // Set the text color to black for better visibility
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);
    
    return hbr;
}

BOOL CStrengthMeterCtrl::PreTranslateMessage(MSG* pMsg)
{
    if (m_tooltip.GetSafeHwnd())
        m_tooltip.RelayEvent(pMsg);
    return CProgressCtrl::PreTranslateMessage(pMsg);
}

void CStrengthMeterCtrl::PreSubclassWindow()
{
    CProgressCtrl::PreSubclassWindow();
    if (!m_tooltip.GetSafeHwnd()) {
        m_tooltip.Create(this);
        m_tooltip.AddTool(this, L"");
        m_tooltip.Activate(TRUE);
    }
} 
