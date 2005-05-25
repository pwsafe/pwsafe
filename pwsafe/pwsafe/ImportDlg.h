#if !defined(AFX_IMPORTDLG_H__99190147_AD17_4596_849E_A5264AAA7CBB__INCLUDED_)
#define AFX_IMPORTDLG_H__99190147_AD17_4596_849E_A5264AAA7CBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportDlg.h : header file
//

#include "resource.h"

void AFXAPI DDV_CheckImpDelimiter(CDataExchange* pDX, const CString &delimiter);

/////////////////////////////////////////////////////////////////////////////
// CImportDlg dialog

class CImportDlg : public CDialog
{
// Construction
public:
	CImportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImportDlg)
	enum { IDD = IDD_IMPORT_TEXT };
	CString	m_groupName;
	CString	m_Separator;
	CString m_defimpdelim;
	int m_tab;
	int m_group;
	int m_querysetimpdelim;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportDlg)
	afx_msg void OnOther();
	afx_msg void OnComma();
	afx_msg void OnTab();
	afx_msg void OnNoGroup();
	afx_msg void OnYesGroup();
	afx_msg void OnSetMultilineImportNotesDelimiter();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTDLG_H__99190147_AD17_4596_849E_A5264AAA7CBB__INCLUDED_)
