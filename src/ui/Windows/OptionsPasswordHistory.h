/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// OptionsPasswordHistory.h : header file
//

#include "DboxMain.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory dialog
#include "Options_PropertyPage.h"
#include "TBMStatic.h"

class COptionsPasswordHistory : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsPasswordHistory)

  // Construction
  COptionsPasswordHistory(CWnd *pParent, st_Opt_master_data *pOPTMD);

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsPasswordHistory)
  enum { IDD = IDD_PS_PASSWORDHISTORY };

  BOOL m_SavePWHistory, mApplyToProtected;
  int m_PWHistoryNumDefault;
  int m_PWHAction;
  int m_PWHDefExpDays;
  //}}AFX_DATA

  CButtonExtn m_chkbox;
  CTBMStatic m_Help1, m_Help2, m_Help3, m_Help4;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsPasswordHistory)
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsPasswordHistory)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnSavePWHistory();
  afx_msg void OnPWHistoryNoAction();
  afx_msg void OnPWHistoryDoAction();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

};
