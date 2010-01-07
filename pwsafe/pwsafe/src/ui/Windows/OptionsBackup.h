/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "Options_PropertyPage.h"

class COptionsBackup : public COptions_PropertyPage
{
  DECLARE_DYNCREATE(COptionsBackup)

  // Construction
public:
  COptionsBackup();
  ~COptionsBackup();

  const wchar_t *GetHelpName() const {return L"backups_tab";}

  // Should be part of ctor, but MFC doesn't requires
  // default ctor. Grrr.
  void SetCurFile(const CString &currentFile);

  // Dialog Data
  //{{AFX_DATA(COptionsBackup)
  enum { IDD = IDD_PS_BACKUP };
  CComboBox m_backupsuffix_cbox;
  CString m_userbackupprefix;
  CString m_userbackupotherlocation;
  BOOL m_saveimmediately;
  BOOL m_backupbeforesave;
  int m_backupprefix;
  int m_backuplocation;
  int m_maxnumincbackups;
  //}}AFX_DATA

  CString m_saveuserbackupprefix;
  CString m_saveuserbackupotherlocation;
  BOOL m_savesaveimmediately;
  BOOL m_savebackupbeforesave;
  int m_savebackupprefix;
  int m_savebackuplocation;
  int m_savemaxnumincbackups;
  int m_savebackupsuffix;

  int m_backupsuffix;
  int m_BKSFX_to_Index[PWSprefs::maxBKSFX + 1];
  CString m_currentFileDir;
  CString m_currentFileBasename;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsBackup)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsBackup)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg BOOL OnKillActive();
  afx_msg void OnBackupPrefix();
  afx_msg void OnBackupDirectory();
  afx_msg void OnBackupBeforeSave();
  afx_msg void OnBrowseForLocation();
  afx_msg void OnUserPrefixKillfocus();
  afx_msg void OnComboChanged();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetExample();
  CToolTipCtrl* m_pToolTipCtrl;
};

