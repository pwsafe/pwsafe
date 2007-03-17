/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once
// ColumnPickerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColumnPickerDlg dialog

// JHF : added PocketPC switch
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif

#include "corelib/ItemData.h"

class CColumnPickerDlg : public CDialog
{
// Construction
public:
	CColumnPickerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CColumnPickerDlg)
	enum { IDD = IDD_COLUMNPICKER };
	int m_column_group, m_column_title, m_column_user, m_column_notes, m_column_password,
		m_column_ctime, m_column_pmtime, m_column_atime, m_column_ltime, m_column_rmtime;
	//}}AFX_DATA

	CItemData::FieldBits m_bsColumn;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColumnPickerDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CColumnPickerDlg)
	afx_msg void OnSetColumnTitle(); // mandatory column
	afx_msg void OnSetColumnUser();  // mandatory column
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

};
