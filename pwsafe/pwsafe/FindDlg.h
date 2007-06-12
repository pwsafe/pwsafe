/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// FindDlg.h : header file
//
#include <vector>
#include "corelib/PwsPlatform.h"
#include "corelib/MyString.h"
#include "corelib/ItemData.h"

#if defined(POCKET_PC)
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #define SUPERCLASS	CDialog
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

class CFindDlg : public SUPERCLASS
{
  // Construction
 public:
	typedef SUPERCLASS		super;

  static void Doit(CWnd* pParent, BOOL *isCS, CMyString *lastFind,
                   bool *bFindWraps); // implement Singleton pattern
  ~CFindDlg();
  static void EndIt();
  // Dialog Data
  //{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_FIND };
  BOOL m_cs_search;
  BOOL m_FindWraps;
  CMyString	m_search_text;
  CString	m_status;
	//}}AFX_DATA

  bool m_bAdvanced;
  CItemData::FieldBits m_bsFields;
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function;

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CFindDlg)
 protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
 protected:

  // Generated message map functions
  //{{AFX_MSG(CFindDlg)
  afx_msg void OnFind();
  afx_msg void OnWrap();
  afx_msg void OnAdvanced();
#if defined(POCKET_PC)
  afx_msg void OnCancel();
#else
  afx_msg void OnClose();
#endif
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()

 private:
  CFindDlg(CWnd* pParent, BOOL *isCS, CMyString *lastFind);
  static CFindDlg *self;
  std::vector<int> m_indices; // array of found items
  int m_lastshown; // last index selected, -1 indicates no search done yet
  int m_numFound; // number of items that matched, as returned by DboxMain::FindAll
  CMyString m_last_search_text;
  BOOL m_last_cs_search;
  CItemData::FieldBits m_last_bsFields;
  CString m_last_subgroup_name;
  int m_last_subgroup_set, m_last_subgroup_object, m_last_subgroup_function;

  CMyString *m_lastTextPtr;
  BOOL *m_lastCSPtr;
  bool m_bLastView;
};

#undef SUPERCLASS
