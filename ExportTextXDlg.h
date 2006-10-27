#pragma once
// ExportTextXDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportTextXDlg dialog

// JHF : added PocketPC switch
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif

#include <bitset>

class CExportTextXDlg : public CDialog
{
// Construction
public:
	CExportTextXDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportTextXDlg)
	enum { IDD = IDD_EXPORT_TEXTX };
	CString m_export_subgroup_name;
	int m_export_group_title, m_export_user, m_export_notes, m_export_password,
		m_export_ctime, m_export_pmtime, m_export_atime, m_export_ltime, m_export_rmtime,
		m_export_url, m_export_autotype, m_export_pwhist, m_export_subgroup;
	int m_subgroup_object, m_subgroup_function, m_export_subgroup_case;

	//}}AFX_DATA

	std::bitset<16> m_bsExport;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportTextXDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CExportTextXDlg)
	afx_msg void OnSetExportGroupTitle();
	afx_msg void OnSetExportUser();
	afx_msg void OnSetExportNotes();
	afx_msg void OnSetExportPassword();
	afx_msg void OnSetExportCTime();
	afx_msg void OnSetExportPMTime();
	afx_msg void OnSetExportATime();
	afx_msg void OnSetExportLTime();
	afx_msg void OnSetExportRMTime();
	afx_msg void OnSetExportUrl();
	afx_msg void OnSetExportAutotype();
	afx_msg void OnSetExportPWHist();
	afx_msg void OnClearTimes();
	afx_msg void OnSetTimes();
	afx_msg void OnClearAll();
	afx_msg void OnSetAll();
	afx_msg void OnSetSubGroup();
	afx_msg void OnSetSubgroupCase();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

};

