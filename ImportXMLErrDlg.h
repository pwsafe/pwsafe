// ImportXMLErrDlg.h : header file
//

#pragma once

#include "afxwin.h"
#include "resource.h"

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

