/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// EditDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "ControlExtns.h"
#include "ExtThread.h"

class CItemData;

class CEditDlg : public CPWDialog
{

public:
  // default constructor
  CEditDlg(CItemData *ci, CWnd* pParent = NULL);
  virtual ~CEditDlg();

  enum { IDD = IDD_EDIT };
  CMyString m_defusername, m_username;
  bool m_Edit_IsReadOnly;

  void ShowPassword();
  void HidePassword();
  void ShowNotes();
  void HideNotes();

private:
  CItemData *m_ci; // The entry being edited
  CMyString m_title;
  CMyString m_group;
  CMyString m_realpassword, m_oldRealPassword;
  CMyString m_password, m_password2;
  CMyString m_notes, m_realnotes;
  CMyString m_URL;
  CMyString m_autotype;
  CMyString m_locCTime;
  CMyString m_locPMTime, m_locATime, m_locLTime, m_locRMTime;
  time_t m_tttLTime;
  bool m_bIsModified;
  // Password History related stuff
  size_t m_NumPWHistory;
  size_t m_MaxPWHistory;
  BOOL m_SavePWHistory;
  PWHistList m_PWHistList;
  CMyString m_PWHistory;

  bool m_isPwHidden, m_isNotesHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  // following two are not directly derived from CItemData
  CMyString m_oldlocLTime;
  int m_oldMaxPWHistory;
  void ResizeDialog();
  void UpdateHistory();

  CComboBoxExtn m_ex_group;
  CEditExtn m_ex_password;
  CEditExtn m_ex_password2;
  CEditExtn m_ex_username;
  CEditExtn m_ex_title;
  CEditExtn m_ex_URL;
  CEditExtn m_ex_autotype;
  CEditExtn *m_pex_notes;

  static CMyString HIDDEN_NOTES;
  static CString CS_SHOW, CS_HIDE, CS_ON, CS_OFF;

  CExtThread *m_thread; // worker thread
  static UINT ExternalEditorThread(LPVOID me);
  TCHAR m_szTempName[MAX_PATH + 1];

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  afx_msg void OnShowPassword();
  virtual void OnOK();
  virtual BOOL OnInitDialog();
  afx_msg void OnRandom();
  afx_msg void OnHelp();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();
  afx_msg LRESULT OnCallExternalEditor(WPARAM, LPARAM);
  afx_msg LRESULT OnExternalEditorEnded(WPARAM, LPARAM);

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
  afx_msg void OnBnClickedClearLTime();
  afx_msg void OnBnClickedSetLTime();
  afx_msg void OnBnClickedPwhist();

  CButton m_MoreLessBtn;

  afx_msg void OnEnSetfocusNotes();
  afx_msg void OnEnKillfocusNotes();
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
