/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
  DECLARE_DYNCREATE(COptionsSecurity)

  // Construction
public:
  COptionsSecurity();
  ~COptionsSecurity();

  // Dialog Data
  //{{AFX_DATA(COptionsSecurity)
  enum { IDD = IDD_PS_SECURITY };
  BOOL m_clearclipboardonminimize;
  BOOL m_clearclipboardonexit;
  BOOL m_LockOnMinimize;
  BOOL m_confirmcopy;
  BOOL m_LockOnWindowLock;
  BOOL m_LockOnIdleTimeout;
  UINT m_IdleTimeOut;
  //}}AFX_DATA

  BOOL m_saveclearclipboardonminimize;
  BOOL m_saveclearclipboardonexit;
  BOOL m_saveLockOnMinimize;
  BOOL m_saveconfirmcopy;
  BOOL m_saveLockOnWindowLock;
  BOOL m_saveLockOnIdleTimeout;
  UINT m_saveIdleTimeOut;

  CString m_csEraserLocation;
  CString m_csEraseCmdLineParms;
  CString m_csSaveEraserLocation;
  CString m_csSaveEraseCmdLineParms;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsSecurity)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsSecurity)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
  afx_msg void OnHelp();
  afx_msg void OnLockOnIdleTimeout();
  afx_msg void OnLockOnMinimize();
  afx_msg BOOL OnKillActive();
  afx_msg void OnBrowseForLocation();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};
