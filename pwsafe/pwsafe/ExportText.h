#if !defined(AFX_EXPORTTEXT_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_)
#define AFX_EXPORTTEXT_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportText.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportText dialog

#include "resource.h"

class CExportTextDlg : public CDialog
{
// Construction
public:
	CExportTextDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportTextDlg)
	enum { IDD = IDD_EXPORT_TEXT };
	CString	m_exportTextPassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportTextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportTextDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTTEXT_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_)
