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
	CAdvancedDlg(CWnd* pParent = NULL, int iIndex = -1,
    CItemData::FieldBits bsFields = 0, CString subgroup_name = _T(""),
    int subgroup_set = BST_UNCHECKED, 
    int subgroup_object = 0, int subgroup_function = 0);   // standard constructor
  virtual CAdvancedDlg::~CAdvancedDlg();

// Dialog Data
  enum { IDD_MERGE = IDD_ADVANCEDMERGE };
	//{{AFX_DATA(CADVANCEDDlg)
	enum { IDD = IDD_ADVANCED };
	CString m_subgroup_name;
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
  afx_msg void OnSetSubGroup();
	afx_msg void OnSelectSome();
	afx_msg void OnSelectAll();
	afx_msg void OnDeselectSome();
	afx_msg void OnDeselectAll();
	afx_msg void OnHelp();
	virtual void OnOK();
  afx_msg void OnSelectedItemchanging(NMHDR * pNMHDR, LRESULT * pResult);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  BOOL PreTranslateMessage(MSG* pMsg);

private:
  static int CALLBACK AdvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  CListCtrl *m_pLC_List, *m_pLC_Selected;
  CToolTipCtrl* m_ToolTipCtrl;
  int m_index;
};
