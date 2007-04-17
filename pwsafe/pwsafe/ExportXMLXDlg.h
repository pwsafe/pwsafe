/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once
// ExportXMLXDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportXMLXDlg dialog

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

class CExportXMLXDlg : public CDialog
{
// Construction
public:
	CExportXMLXDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportXMLXDlg)
	enum { IDD = IDD_EXPORT_XMLX };
	CString m_exportxml_subgroup_name;
	int m_exportxml_group, m_exportxml_title, m_exportxml_user, m_exportxml_notes, m_exportxml_password,
		m_exportxml_ctime, m_exportxml_pmtime, m_exportxml_atime, m_exportxml_ltime, m_exportxml_rmtime,
		m_exportxml_url, m_exportxml_autotype, m_exportxml_pwhist, m_exportxml_subgroup;
	int m_subgroup_object, m_subgroup_function, m_exportxml_subgroup_case;

	//}}AFX_DATA

	CItemData::FieldBits m_bsExport;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportXMLXDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CExportXMLXDlg)
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

