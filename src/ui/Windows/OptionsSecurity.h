/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// OptionsSecurity.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity dialog
#include "Options_PropertyPage.h"

class COptionsSecurity : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsSecurity)

  // Construction
  COptionsSecurity(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsSecurity();

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsSecurity)
  enum { IDD = IDD_PS_SECURITY };

  BOOL m_ClearClipboardOnMinimize;
  BOOL m_ClearClipboardOnExit;
  BOOL m_LockOnMinimize;
  BOOL m_ConfirmCopy;
  BOOL m_LockOnWindowLock;
  BOOL m_LockOnIdleTimeout;
  BOOL m_CopyPswdBrowseURL;
  int m_IdleTimeOut;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsSecurity)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsSecurity)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
  afx_msg void OnHelp();
  afx_msg void OnLockOnIdleTimeout();
  afx_msg void OnLockOnMinimize();
  afx_msg BOOL OnKillActive();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static const UINT uiDBPrefs[];
};

