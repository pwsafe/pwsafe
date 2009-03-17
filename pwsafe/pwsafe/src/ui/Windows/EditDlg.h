/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// EditDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "ControlExtns.h"
#include "ExtThread.h"
#include "corelib/ItemData.h"
#include "corelib/PWHistory.h"

class DboxMain;

class CEditDlg : public CPWDialog
{
public:
  // default constructor
  CEditDlg(CItemData *ci, const StringX currentDB, CWnd* pParent = NULL);
  virtual ~CEditDlg();

  enum { IDD = IDD_EDIT };
  CSecString m_defusername, m_username, m_dependents, m_base;
  bool m_Edit_IsReadOnly;
  int m_num_dependents;
  enum CItemData::EntryType m_original_entrytype;
  int m_ibasedata;
  uuid_array_t m_base_uuid;
  PWPolicy m_pwp;
  StringX m_currentDB;

  void ShowPassword();
  void HidePassword();
  void ShowNotes();
  void HideNotes();

protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnShowPassword();
  afx_msg void OnRandom();
  afx_msg void OnHelp();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
  afx_msg void OnBnClickViewDependents();
  afx_msg void OnBnClickedClearXTime();
  afx_msg void OnBnClickedSetXTime();
  afx_msg void OnBnClickedPwhist();
  afx_msg void OnBnClickedOverridePolicy();
  afx_msg void OnBnClickedLaunch();
  afx_msg void OnStcClicked(UINT nId);
  afx_msg void OnEnChangeUrl();
  afx_msg void OnEnSetfocusNotes();
  afx_msg void OnEnKillfocusNotes();
  afx_msg LRESULT OnCallExternalEditor(WPARAM, LPARAM);
  afx_msg LRESULT OnExternalEditorEnded(WPARAM, LPARAM);
  afx_msg LRESULT OnWordWrap(WPARAM, LPARAM);
  afx_msg LRESULT OnShowNotes(WPARAM, LPARAM);

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;

  CItemData *m_ci; // The entry being edited
  CSecString m_title;
  CSecString m_group;
  CSecString m_realpassword, m_oldRealPassword;
  CSecString m_password, m_password2;
  CSecString m_notes, m_notesww, m_realnotes;
  CSecString m_URL;
  CSecString m_autotype;
  CSecString m_runcommand;
  CSecString m_locCTime;
  CSecString m_locPMTime, m_locATime, m_locXTime, m_locRMTime;
  time_t m_tttXTime;
  time_t m_tttCPMTime;  // Password creation or last changed datetime
  int m_XTimeInt, m_oldXTimeInt;

  // Password History related stuff
  size_t m_NumPWHistory;
  size_t m_MaxPWHistory;
  BOOL m_SavePWHistory;
  PWHistList m_PWHistList;
  CSecString m_PWHistory;

  bool m_isPwHidden, m_isNotesHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  // following two are not directly derived from CItemData
  CSecString m_oldlocXTime;
  int m_oldMaxPWHistory;
  void ResizeDialog();
  void UpdateHistory();
  void SelectAllNotes();

  CComboBoxExtn m_ex_group;
  CSecEditExtn m_ex_password;
  CSecEditExtn m_ex_password2;
  CEditExtn m_ex_username;
  CEditExtn m_ex_title;
  CEditExtn m_ex_URL;
  CEditExtn m_ex_autotype;
  CEditExtn m_ex_runcommand;
  CEditExtn *m_pex_notes;
  CEditExtn *m_pex_notesww;

  CStaticExtn m_stc_group;
  CStaticExtn m_stc_title;
  CStaticExtn m_stc_username;
  CStaticExtn m_stc_password;
  CStaticExtn m_stc_notes;
  CStaticExtn m_stc_URL;
  CStaticExtn m_stc_autotype; 
  CStaticExtn m_stc_runcommand;

  CButton m_MoreLessBtn;
  CButton m_ViewDependentsBtn;

  static CSecString HIDDEN_NOTES;
  static CString CS_SHOW, CS_HIDE, CS_ON, CS_OFF;

  CExtThread *m_thread; // worker thread
  static UINT ExternalEditorThread(LPVOID me);
  TCHAR m_szTempName[MAX_PATH + 1];
  CToolTipCtrl* m_ToolTipCtrl;

  BOOL m_OverridePolicy;
  BOOL m_bWordWrap;
  BOOL m_bShowNotes;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
