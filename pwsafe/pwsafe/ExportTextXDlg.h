/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
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

#include "corelib/ItemData.h"
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

	CItemData::FieldBits m_bsExport;

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
	afx_msg void OnClearTimes();
	afx_msg void OnSetTimes();
	afx_msg void OnClearAll();
	afx_msg void OnSetAll();
	afx_msg void OnSetSubGroup();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

};

