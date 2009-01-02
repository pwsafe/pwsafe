/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "PWPropertyPage.h"

class COptionsPasswordHistory : public CPWPropertyPage
{
  DECLARE_DYNCREATE(COptionsPasswordHistory)

  // Construction
public:
  COptionsPasswordHistory();
  ~COptionsPasswordHistory();
  DboxMain *m_pDboxMain;

  const TCHAR *GetHelpName() const {return _T("password_history_tab");}

  // Dialog Data
  //{{AFX_DATA(COptionsPasswordHistory)
  enum { IDD = IDD_PS_PASSWORDHISTORY };
  BOOL m_savepwhistory;
  UINT m_pwhistorynumdefault;
  int  m_pwhaction;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsPasswordHistory)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsPasswordHistory)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnKillActive();
  afx_msg void OnSavePWHistory();
  afx_msg void OnApplyPWHChanges();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  // Implementation
protected:
  BOOL PreTranslateMessage(MSG* pMsg);

private:
  CToolTipCtrl* m_ToolTipCtrl;
  afx_msg void OnPWHistoryNoAction();
  afx_msg void OnPWHistoryDoAction();
};
