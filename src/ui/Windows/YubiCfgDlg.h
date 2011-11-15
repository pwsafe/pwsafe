/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// \file YubiCfgDlg.h

#include "PWDialog.h"

class CYubiCfgDlg : public CPWDialog
{
public:
	CYubiCfgDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CYubiCfgDlg();

// Dialog Data
	enum { IDD = IDD_YUBIKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    CString m_YubiSN;
    CString m_YubiSK;
public:
    afx_msg void OnYubiGenBn();
    afx_msg void OnBnClickedOk();
};
