/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once
// AdvancedDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg dialog

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

enum {ADV_COMPARE = 0, ADV_MERGE, ADV_EXPORT_TEXT, ADV_EXPORT_XML, ADV_LAST};

class CAdvancedDlg : public CDialog
{
// Construction
public:
	CAdvancedDlg(CWnd* pParent = NULL, int iIndex = -1);   // standard constructor

// Dialog Data
  enum { IDD_MERGE = IDD_ADVANCEDMERGE };
	//{{AFX_DATA(CADVANCEDDlg)
	enum { IDD = IDD_ADVANCED };
	CString m_subgroup_name;
	int m_group, m_title, m_user, m_notes, m_password, 
    m_ctime, m_pmtime, m_atime, m_ltime, m_rmtime,
    m_url, m_autotype, m_pwhist;
	int m_subgroup_set, m_subgroup_object, m_subgroup_function, m_subgroup_case;

	//}}AFX_DATA

	CItemData::FieldBits m_bsFields;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CADVANCEDDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   int m_iIndex;
   static int dialog_lookup[ADV_LAST];

protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CADVANCEDDlg)
	afx_msg void OnClearTimes();
	afx_msg void OnSetTimes();
	afx_msg void OnClearAll();
	afx_msg void OnSetAll();
	afx_msg void OnSetSubGroup();
  afx_msg void OnSetTitle();
  afx_msg void OnSetPassword();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

};

