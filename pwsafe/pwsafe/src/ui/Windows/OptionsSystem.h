/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
  DECLARE_DYNCREATE(COptionsSystem)

  // Construction
public:
  COptionsSystem();
  ~COptionsSystem();

  // Dialog Data
  //{{AFX_DATA(COptionsSystem)
  enum { IDD = IDD_PS_SYSTEM };
  int m_maxreitems;
  BOOL m_usesystemtray;
  BOOL m_hidesystemtray;
  BOOL m_startup;
  int m_maxmruitems;
  BOOL m_mruonfilemenu;
  BOOL m_deleteregistry;
  BOOL m_defaultopenro;
  BOOL m_multipleinstances;
  BOOL m_migrate2appdata;
  BOOL m_initialhotkeystate;
  //}}AFX_DATA

  int m_savemaxreitems;
  BOOL m_saveusesystemtray;
  BOOL m_savehidesystemtray;
  BOOL m_savestartup;
  int m_savemaxmruitems;
  BOOL m_savemruonfilemenu;
  BOOL m_savedeleteregistry;
  BOOL m_savemigrate2appdata;
  BOOL m_savedefaultopenro;
  BOOL m_savemultipleinstances;

  static bool m_bShowConfigFile;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsSystem)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  // Implementation
protected:
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
};
