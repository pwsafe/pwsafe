/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// OptionsMisc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc dialog

class COptionsMisc : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsMisc)

// Construction
public:
	COptionsMisc();
	~COptionsMisc();

// Dialog Data
	//{{AFX_DATA(COptionsMisc)
	enum { IDD = IDD_PS_MISC };
	BOOL	m_confirmdelete;
	BOOL	m_maintaindatetimestamps;
	BOOL	m_escexits;
	BOOL    m_hotkey_enabled;
	// JHF : class CHotKeyCtrl not supported by WinCE
#if !defined(POCKET_PC)
	CHotKeyCtrl	m_hotkey;
#endif
	CComboBox m_dblclk_cbox;
	BOOL	m_usedefuser;
	BOOL	m_querysetdef;
	CString	m_defusername;
	CString m_otherbrowserlocation;
	//}}AFX_DATA
	DWORD m_hotkey_value;
	int     m_doubleclickaction;
	int		m_DCA_to_Index[PWSprefs::maxDCA + 1];
	CString m_csBrowser;
	CString m_csAutotype;
    BOOL m_minauto;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsMisc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	// Generated message map functions
	//{{AFX_MSG(COptionsMisc)
	afx_msg void OnEnableHotKey();
	afx_msg void OnUsedefuser();
	afx_msg void OnBrowseForLocation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnComboChanged();
	BOOL PreTranslateMessage(MSG* pMsg);

private:
	CToolTipCtrl* m_ToolTipCtrl;
};
