/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// ExportXML.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportXML dialog

#include "SecString.h"
#include "ControlExtns.h"
#include "PWDialog.h"
#include "AdvancedAttDlg.h"

#include "corelib/attachments.h"

class CVKeyBoardDlg;

class CExportAttDlg : public CPWDialog
{
  // Construction
public:
  CExportAttDlg(CWnd* pParent, ATFVector &vatf);   // standard constructor
  ~CExportAttDlg();

  const CSecString &GetPasskey() const {return m_passkey;}

  // Dialog Data
  //{{AFX_DATA(CExportAttDlg)
  enum { IDD = IDD_EXPORT_ATTACHMENT };
  //}}AFX_DATA

  CString m_test_value[CAdvancedAttDlg::NUM_TESTS];
  int m_test_set[CAdvancedAttDlg::NUM_TESTS], m_test_object[CAdvancedAttDlg::NUM_TESTS], 
      m_test_function[CAdvancedAttDlg::NUM_TESTS];

  ATFVector &m_vatf;

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CExportAttDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CExportAttDlg)
  afx_msg void OnAdvanced();
  afx_msg void OnHelp();
  virtual void OnOK();
  afx_msg void OnVirtualKeyboard();
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

private:
  CSecEditExtn *m_pctlPasskey;
  CSecString m_passkey;
  CVKeyBoardDlg *m_pVKeyBoardDlg;
};
