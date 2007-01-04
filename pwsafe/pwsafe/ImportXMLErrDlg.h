/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// ImportXMLErrDlg.h : header file
//

#include "afxwin.h"
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

/////////////////////////////////////////////////////////////////////////////
// CImportXMLErrDlg dialog

class CImportXMLErrDlg : public CDialog
{
// Construction
public:
	CImportXMLErrDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CImportXMLErrDlg)
	enum { IDD = IDD_IMPORT_XML_ERRORS };
	CString	m_strActionText, m_strResultText;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportXMLErrDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportXMLErrDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCopyToClipboard();
};
