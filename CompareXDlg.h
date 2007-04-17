/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once
// CompareXDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCompareXDlg dialog

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

class CCompareXDlg : public CDialog
{
// Construction
public:
	CCompareXDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompareXDlg)
	enum { IDD = IDD_COMPAREX };
	CString m_compare_subgroup_name;
	int m_compare_group, m_compare_title, m_compare_user, m_compare_notes,
    m_compare_password, m_compare_ctime, m_compare_pmtime, m_compare_atime,
    m_compare_ltime, m_compare_rmtime, m_compare_url, m_compare_autotype, 
    m_compare_pwhist, m_compare_subgroup;
	int m_subgroup_object, m_subgroup_function, m_compare_subgroup_case;

	//}}AFX_DATA

	CItemData::FieldBits m_bsCompare;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompareXDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CCompareXDlg)
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

