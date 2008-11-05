/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once


/// \file YubiKeyDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"

class CYubiKeyDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CYubiKeyDlg)

public:
	CYubiKeyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CYubiKeyDlg();

// Dialog Data
	enum { IDD = IDD_YUBIKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_YKpubID;
private:
   CString m_YKstatus;
public:
   afx_msg void OnOk();
};
