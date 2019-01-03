/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

  // These map between slider values and
  // MIN_HASH_ITERATIONS..MAX_USABLE_HASH_ITERS
  uint32 GetHashIter() const {return m_HashIter;}
  void SetHashIter(uint32 value);
  void UpdateHashIter(); // slider to value
  
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

  enum {MinHIslider = 0, MaxHIslider = 31};
  int m_HashIterSliderValue;
  uint32 m_HashIter;

  CButtonExtn m_chkbox[2];
  CTBMStatic m_Help1, m_Help2, m_Help3;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsSecurity)
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsSecurity)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
  afx_msg void OnHelp();
  afx_msg void OnLockOnIdleTimeout();
  afx_msg void OnLockOnMinimize();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};
