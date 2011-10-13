/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// OptionsBackup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsBackup dialog
#include "resource.h"
#include "Options_PropertyPage.h"
#include "core/PWSprefs.h"

class COptionsBackup : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsBackup)

  // Construction
  COptionsBackup(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsBackup();

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsBackup)
  enum { IDD = IDD_PS_BACKUP, IDD_SHORT = IDD_PS_BACKUP_SHORT };

  CComboBox m_backupsuffix_cbox;

  CString m_UserBackupPrefix;
  CString m_UserBackupOtherLocation;
  BOOL m_SaveImmediately;
  BOOL m_BackupBeforeSave;
  int m_BackupPrefix;
  int m_BackupLocation;
  int m_MaxNumIncBackups;
  int m_BackupSuffix;
  //}}AFX_DATA
 
  int m_BKSFX_to_Index[PWSprefs::maxBKSFX + 1];
  CString m_currentFile;
  CString m_currentFileDir;
  CString m_currentFileBasename;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsBackup)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsBackup)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg BOOL OnKillActive();
  afx_msg void OnBackupPrefix();
  afx_msg void OnBackupDirectory();
  afx_msg void OnBackupBeforeSave();
  afx_msg void OnBrowseForLocation();
  afx_msg void OnUserPrefixKillfocus();
  afx_msg void OnComboChanged();
  afx_msg void OnPreferencesHelp();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetExample();
  CToolTipCtrl* m_pToolTipCtrl;
};

