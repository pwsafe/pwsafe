/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "PWPropertyPage.h"

class COptionsSecurity : public CPWPropertyPage
{
  DECLARE_DYNCREATE(COptionsSecurity)

  // Construction
public:
  COptionsSecurity();
  ~COptionsSecurity();

  const TCHAR *GetHelpName() const {return _T("security_tab");}

  // Dialog Data
  //{{AFX_DATA(COptionsSecurity)
  enum { IDD = IDD_PS_SECURITY };
  BOOL	m_clearclipboard;
  BOOL	m_lockdatabase;
  BOOL	m_confirmcopy;
  BOOL	m_LockOnWindowLock;
  BOOL	m_LockOnIdleTimeout;
  UINT    m_IdleTimeOut;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsSecurity)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsSecurity)
  afx_msg void OnLockbase();
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnKillActive();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};
