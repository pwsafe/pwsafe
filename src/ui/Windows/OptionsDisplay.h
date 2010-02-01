/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "Options_PropertyPage.h"

class COptionsDisplay : public COptions_PropertyPage
{
  DECLARE_DYNCREATE(COptionsDisplay)

  // Construction
public:
  COptionsDisplay();
  ~COptionsDisplay();

  // Dialog Data
  //{{AFX_DATA(COptionsDisplay)
  enum { IDD = IDD_PS_DISPLAY };
  BOOL m_alwaysontop;
  BOOL m_showusernameintree;
  BOOL m_showpasswordintree;
  BOOL m_shownotesastipsinviews;
  BOOL m_explorertree;
  BOOL m_enablegrid;
  BOOL m_pwshowinedit;
  BOOL m_notesshowinedit;
  BOOL m_wordwrapnotes;
  BOOL m_preexpirywarn;
  BOOL m_highlightchanges;
#if defined(POCKET_PC)
  BOOL m_dcshowspassword;
#endif
  int m_treedisplaystatusatopen;
  int m_preexpirywarndays;
  int m_trayiconcolour;
  //}}AFX_DATA

  BOOL m_savealwaysontop;
  BOOL m_saveshowusernameintree;
  BOOL m_saveshowpasswordintree;
  BOOL m_saveshownotesastipsinviews;
  BOOL m_saveexplorertree;
  BOOL m_saveenablegrid;
  BOOL m_savepwshowinedit;
  BOOL m_savenotesshowinedit;
  BOOL m_savewordwrapnotes;
  BOOL m_savepreexpirywarn;
  int m_savetreedisplaystatusatopen;
  int m_savepreexpirywarndays;
  int m_savetrayiconcolour;

  BOOL m_MustHaveUsernames;
  CSecString m_csUserDisplayToolTip;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsDisplay)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsDisplay)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnPreWarn();
  afx_msg void OnDisplayUserInTree();
  afx_msg BOOL OnKillActive();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CToolTipCtrl* m_pToolTipCtrl;
};
