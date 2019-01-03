/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "TBMStatic.h"
#include "Fonts.h"

#include "resource.h"

class COptionsDisplay : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsDisplay)

  // Construction
  COptionsDisplay(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsDisplay();

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsDisplay)
  enum { IDD = IDD_PS_DISPLAY, IDD_SHORT = IDD_PS_DISPLAY_SHORT };

  BOOL m_AlwaysOnTop;
  BOOL m_ShowUsernameInTree;
  BOOL m_ShowPasswordInTree;
  BOOL m_ShowNotesAsTipsInViews;
  BOOL m_ExplorerTypeTree;
  BOOL m_EnableGrid;
  BOOL m_ShowPasswordInEdit;
  BOOL m_ShowNotesInEdit;
  BOOL m_WordWrapNotes;
  BOOL m_PreExpiryWarn;
  BOOL m_HighlightChanges;
  BOOL m_EnableTransparency;
  int m_TreeDisplayStatusAtOpen;
  int m_PreExpiryWarnDays;
  int m_PercentTransparency;
  //}}AFX_DATA

  CButtonExtn m_chkbox[4];
  CButtonExtn m_radiobtn[3];

  CTBMStatic m_Help1;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsDisplay)
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsDisplay)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnPreWarn();
  afx_msg void OnDisplayUserInTree();
  afx_msg void OnEnabletransparency();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};
