/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once

// ImportXMLDlg.h : header file
//

#include "PWDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CImportXMLDlg dialog

class CImportXMLDlg : public CPWDialog
{
// Construction
public:
	CImportXMLDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImportXMLDlg)
	enum { IDD = IDD_IMPORT_XML };
	CString	m_groupName;
	int m_group;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportXMLDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportXMLDlg)
	afx_msg void OnNoGroup();
	afx_msg void OnYesGroup();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
