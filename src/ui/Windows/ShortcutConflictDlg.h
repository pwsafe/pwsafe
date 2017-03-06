/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file ShortcutConflictDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"

#include "os/typedefs.h" // For int32

#include <vector>

// CSHCTConflictListCtrl CListCtrl

class CSHCTConflictListCtrl : public CListCtrl
{
public:
  CSHCTConflictListCtrl() {}
  ~CSHCTConflictListCtrl() {}

protected:
  //{{AFX_MSG(CSHCTConflictListCtrl)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

struct st_Conflicts {
  int32 iShortcut;
  CString cs_Menu;
  CString cs_Group;
  CString cs_Title;
  CString cs_User;
};

// CShortcutConflictDlg dialog

class CShortcutConflictDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CShortcutConflictDlg)

public:
  CShortcutConflictDlg(CWnd *pParent, std::vector<st_Conflicts> vConflicts);
	virtual ~CShortcutConflictDlg();

// Dialog Data
	enum { IDD = IDD_SHORTCUTCONFLICTDLG };

protected:
  CSHCTConflictListCtrl m_LCConflicts;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK CKBSHCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  CFont m_BoldFont;
  std::vector<st_Conflicts> m_vConflicts;

  int m_iKBSortedColumn;
  bool m_bKBSortAscending;
};
