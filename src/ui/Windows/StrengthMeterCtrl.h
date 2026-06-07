/*
* Copyright (c) 2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include <afxcmn.h>

// CStrengthMeterCtrl
// A control that displays password strength using a progress bar
class CStrengthMeterCtrl : public CProgressCtrl
{
    DECLARE_DYNAMIC(CStrengthMeterCtrl)

public:
    CStrengthMeterCtrl();
    virtual ~CStrengthMeterCtrl();

    // Set the strength value (0-100)
    void SetStrength(double nStrength);

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;
    virtual void PreSubclassWindow() override;

private:
    double m_nStrength;  // Current strength value (0-100)
    COLORREF m_crWeak;    // Color for weak passwords
    COLORREF m_crMedium;  // Color for medium strength passwords
    COLORREF m_crStrong;  // Color for strong passwords
    CToolTipCtrl m_tooltip;
}; 