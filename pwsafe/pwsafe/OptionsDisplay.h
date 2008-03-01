/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// OptionsDisplay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay dialog
#include "PWPropertyPage.h"

class COptionsDisplay : public CPWPropertyPage
{
  DECLARE_DYNCREATE(COptionsDisplay)

  // Construction
public:
  COptionsDisplay();
  ~COptionsDisplay();

  const TCHAR *GetHelpName() const {return _T("display_tab");}
  // Dialog Data
  //{{AFX_DATA(COptionsDisplay)
  enum { IDD = IDD_PS_DISPLAY };
  BOOL m_alwaysontop;
  BOOL m_showusernameintree;
  BOOL m_showpasswordintree;
  BOOL m_shownotesastipsintree;
  BOOL m_explorertree;
  BOOL m_enablegrid;
  BOOL m_pwshowinedit;
  BOOL m_notesshowinedit;
  BOOL m_preexpirywarn;
#if defined(POCKET_PC)
  BOOL m_dcshowspassword;
#endif
  int m_treedisplaystatusatopen;
  int m_preexpirywarndays;
  int m_trayiconcolour;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsDisplay)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsDisplay)
  virtual BOOL OnInitDialog();
  afx_msg void OnPreWarn();
  afx_msg void OnDisplayUserInTree();
  afx_msg BOOL OnKillActive();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
