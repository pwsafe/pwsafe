/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// OptionsSystem.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem dialog
#include "Options_PropertyPage.h"

class COptionsSystem : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsSystem)

  // Construction
  COptionsSystem(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsSystem();

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsSystem)
  enum { IDD = IDD_PS_SYSTEM, IDD_SHORT = IDD_PS_SYSTEM_SHORT };
  BOOL m_UseSystemTray;
  BOOL m_HideSystemTray;
  BOOL m_Startup;
  BOOL m_MRUOnFileMenu;
  BOOL m_DefaultOpenRO;
  BOOL m_MultipleInstances;
  int m_MaxREItems;
  int m_MaxMRUItems;

  BOOL m_DeleteRegistry, m_saveDeleteRegistry;
  BOOL m_Migrate2Appdata, m_saveMigrate2Appdata;
  BOOL m_InitialHotkeyState;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsSystem)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsSystem)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnUseSystemTray();
  afx_msg void OnStartup();
  afx_msg void OnSetDeleteRegistry();
  afx_msg void OnSetMigrate2Appdata();
  afx_msg void OnApplyConfigChanges();
  afx_msg void OnNeverSaveDBNames();
  afx_msg BOOL OnKillActive();
  afx_msg BOOL OnSetActive();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CToolTipCtrl* m_pToolTipCtrl;
  static bool m_bShowConfigFile;
};
