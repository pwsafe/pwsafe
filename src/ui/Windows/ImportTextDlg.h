/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once
// ImportTextDlg.h : header file
//

#include "PWDialog.h"

void AFXAPI DDV_CheckImpDelimiter(CDataExchange* pDX, const CString &delimiter);

/////////////////////////////////////////////////////////////////////////////
// CImportTextDlg dialog

class CImportTextDlg : public CPWDialog
{
  // Construction
public:
  CImportTextDlg(CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CImportTextDlg)
  enum { IDD = IDD_IMPORT_TEXT };
  CString m_groupName;
  CString m_Separator;
  CString m_defimpdelim;
  int m_tab;
  int m_group;
  BOOL m_bImportPSWDsOnly;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CImportTextDlg)
protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CImportTextDlg)
  afx_msg void OnOther();
  afx_msg void OnComma();
  afx_msg void OnTab();
  afx_msg void OnNoGroup();
  afx_msg void OnYesGroup();
  afx_msg void OnImportPSWDsOnly();
  afx_msg void OnHelp();
  virtual void OnOK();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
