#if !defined(AFX_EXPORTTEXTSETTINGSDLG_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_)
#define AFX_EXPORTTEXTSETTINGSDLG_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportTextSettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportText dialog

#include "resource.h"

void AFXAPI DDV_CheckExpDelimiter(CDataExchange* pDX, CString delimiter);

class CExportTextSettingsDlg : public CDialog
{
// Construction
public:
	CExportTextSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportTextSettingsDlg)
	enum { IDD = IDD_EXPORT_TEXT_SETTINGSDLG };
	CString m_defexpdelim;
	int m_querysetexpdelim;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportTextSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportTextSettingsDlg)
	afx_msg void OnSetMultilineExportNotesDelimiter();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTTEXTSETTINGSDLG_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_)
