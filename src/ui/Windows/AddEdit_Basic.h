/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Basic.h
//-----------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "AddEdit_PropertyPage.h"
#include "ExtThread.h"
#include "ControlExtns.h"
#include "TBMStatic.h"

#include "core/ItemData.h"

class DboxMain;

class CAddEdit_Basic : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_Basic)

  CAddEdit_Basic(CWnd *pParent, st_AE_master_data *pAEMD);

  static CString CS_SHOW, CS_HIDE, CS_EXTERNAL_EDITOR, CS_HIDDEN_NOTES;
  static HANDLE ghEvents[2];

  // Dialog Data
  //{{AFX_DATA(CAddEdit_Basic)
  enum { IDD = IDD_ADDEDIT_BASIC, IDD_SHORT = IDD_ADDEDIT_BASIC_SHORT };

  CSecString m_password, m_password2;
 
  CComboBoxExtn m_ex_group;

  CEditExtn m_ex_title;
  CEditExtn m_ex_username;
  CRichEditExtn m_ex_notes;
  CEditExtn m_ex_URL;
  CEditExtn m_ex_email;
  CEditExtn m_ex_base;

  CSecEditExtn m_ex_password, m_ex_password2;

  CStaticExtn m_stc_group;
  CStaticExtn m_stc_title;
  CStaticExtn m_stc_username;
  CStaticExtn m_stc_password;
  CStaticExtn m_stc_notes;
  CStaticExtn m_stc_URL;
  CStaticExtn m_stc_email;
  CStaticExtn m_stc_isdependent;
  CStaticExtn m_stc_dependent;

  CComboBox m_cmbDependents;
  CEditExtn m_ex_hidden_notes;
  //}}AFX_DATA

  CExtThread *m_thread; // worker thread
  static UINT ExternalEditorThread(LPVOID me);
  wchar_t m_szTempName[MAX_PATH + 1];

  bool m_isPWHidden, m_isNotesHidden;
  bool m_bWordWrap, m_bLaunchPlus;

  void CancelThreadWait()
  { SetEvent(ghEvents[1]); }

  bool IsNotesExternalEditorActive()
  { return m_bUsingNotesExternalEditor; }

  // Overrides
  // ClassWizard generate virtual function overrides

protected:
  //{{AFX_VIRTUAL(CAddEdit_Basic)
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Basic)
  afx_msg void OnHelp();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnPageKillActive(NMHDR *nmhdr, LRESULT *pLResult);
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

  afx_msg void OnENSetFocusPassword();
  afx_msg void OnENSetFocusPassword2();
  afx_msg void OnENChangePassword();
  afx_msg void OnENKillFocusNotes();
  afx_msg void OnChanged();
  afx_msg void OnENChangeNotes();
  afx_msg void OnENChangeURL();
  afx_msg void OnENChangeEmail();
  afx_msg void OnGroupComboChanged();

  afx_msg void OnGeneratePassword();
  afx_msg void OnCopyPassword();
  afx_msg void OnShowPassword();
  afx_msg void OnSTCExClicked(UINT nId);
  afx_msg void OnLaunch();
  afx_msg void OnSendEmail();

  afx_msg LRESULT OnCallExternalEditor(WPARAM, LPARAM);
  afx_msg LRESULT OnExternalEditorEnded(WPARAM, LPARAM);
  afx_msg LRESULT OnWordWrap(WPARAM, LPARAM);
  afx_msg LRESULT OnZoomNotes(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetZoomMenu();
  void SelectAllNotes();
  void ShowPassword();
  void HidePassword();
  void ShowNotes(const bool bForceShow = false);
  void HideNotes(const bool bForceHide = false);
  void ResetHiddenNotes();
  void SetUpDependentsCombo();
  void SetComboBoxWidth();

  bool CheckNewPassword(const StringX &group, const StringX &title,
                        const StringX &user, const StringX &password,
                        const bool bIsEdit, const CItemData::EntryType InputType, 
                        pws_os::CUUID &base_uuid, int &ibasedata, bool &b_msg_issued);
  void SetGroupComboBoxWidth();
  void ShowHideBaseInfo(const CItemData::EntryType &entrytype, CSecString &csBase);

  CTBMStatic m_Help1, m_Help2, m_Help3, m_Help4;

  COLORREF m_group_cfOldColour, m_title_cfOldColour, m_user_cfOldColour;
  COLORREF m_pswd_cfOldColour, m_notes_cfOldColour, m_URL_cfOldColour;
  COLORREF m_email_cfOldColour, m_protected_cfOldColour;
  BOOL m_bOKSave, m_bOKCancel;

  bool m_bInitdone;
  bool m_bUsingNotesExternalEditor;
  int m_iPointSize;

  CBitmap m_CopyPswdBitmap;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
