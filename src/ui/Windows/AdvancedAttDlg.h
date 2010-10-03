/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once
// AdvancedDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvancedAttDlg dialog

#include "PWDialog.h"
#include "corelib/attachments.h"

class CAdvancedAttDlg : public CPWDialog
{
public:
  CAdvancedAttDlg(CWnd* pParent, ATFVector &vatf);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CAdvancedAttDlg)
  enum { IDD = IDD_ADVANCED_ATT };
  enum {NUM_TESTS = 3};
  CString m_test_value[NUM_TESTS];
  int m_test_set[NUM_TESTS], m_test_object[NUM_TESTS], 
      m_test_function[NUM_TESTS], m_test_case[NUM_TESTS];;

  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAdvancedAttDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAdvancedAttDlg)
  afx_msg void OnSetTest(UINT id);
  afx_msg void OnHelp();
  virtual void OnOK();
  afx_msg void OnSelectedItemchanging(NMHDR * pNMHDR, LRESULT * pResult);

  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

private:
  ATFVector &m_vatf;

  static const UINT ui_tests[NUM_TESTS];
  static const UINT ui_fncts[NUM_TESTS];
  static const UINT ui_names[NUM_TESTS];
  static const UINT ui_objs[NUM_TESTS];
  static const UINT ui_cases[NUM_TESTS];
  static const UINT ui_texts[NUM_TESTS];
  static const UINT ui_where[NUM_TESTS];
};
