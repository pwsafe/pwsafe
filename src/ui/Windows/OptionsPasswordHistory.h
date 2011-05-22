/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

class COptionsPasswordHistory : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsPasswordHistory)

  // Construction
  COptionsPasswordHistory(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsPasswordHistory();

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsPasswordHistory)
  enum { IDD = IDD_PS_PASSWORDHISTORY };

  BOOL m_SavePWHistory;
  int m_PWHistoryNumDefault;
  int  m_PWHAction;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsPasswordHistory)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsPasswordHistory)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg BOOL OnKillActive();
  afx_msg void OnSavePWHistory();
  afx_msg void OnPWHistoryNoAction();
  afx_msg void OnPWHistoryDoAction();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CToolTipCtrl* m_pToolTipCtrl;
};
