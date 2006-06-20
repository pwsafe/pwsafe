#pragma once

// ImportXMLDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CImportXMLDlg dialog

class CImportXMLDlg : public CDialog
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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

