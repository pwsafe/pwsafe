#if !defined(AFX_ExportXML_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_)
#define AFX_ExportXML_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportXML.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportXML dialog

// JHF : added PocketPC switch
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

class CExportXMLDlg : public CDialog
{
// Construction
public:
	CExportXMLDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportXMLDlg)
	enum { IDD = IDD_EXPORT_XML };
	CString	m_ExportXMLPassword;
	CString m_defexpdelim;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportXMLDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CExportXMLDlg)
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void AFXAPI DDV_CheckExpDelimiter(CDataExchange* pDX, const CString &delimiter);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ExportXML_H__CCC6EEF6_3DF0_41A2_BDA0_0437BBFAC672__INCLUDED_)
