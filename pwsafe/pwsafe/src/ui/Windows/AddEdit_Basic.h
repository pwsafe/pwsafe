/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Basic.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"
#include "ExtThread.h"
#include "ControlExtns.h"
#include "corelib/ItemData.h"

class DboxMain;

class CAddEdit_Basic : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_Basic)

  CAddEdit_Basic(CWnd *pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_Basic();

  const TCHAR *GetHelpName() const {return _T("TO_DO!");}

  static CString CS_SHOW, CS_HIDE;
  static CSecString HIDDEN_NOTES;

    // Dialog Data
  //{{AFX_DATA(CAddEdit_Basic)
  enum { IDD = IDD_ADDEDIT_BASIC };

  CSecString m_password, m_password2;
  CSecString m_notes, m_notesww;

  CComboBoxExtn m_ex_group;

  CEditExtn m_ex_title;
  CEditExtn m_ex_username;
  CEditExtn *m_pex_notes;
  CEditExtn *m_pex_notesww;
  CEditExtn m_ex_URL;

  CSecEditExtn m_ex_password, m_ex_password2;

  CStaticExtn m_stc_group;
  CStaticExtn m_stc_title;
  CStaticExtn m_stc_username;
  CStaticExtn m_stc_password;
  CStaticExtn m_stc_notes;
  CStaticExtn m_stc_URL;

  CButton m_ViewDependentsBtn;
  //}}AFX_DATA

  CExtThread *m_thread; // worker thread
  static UINT ExternalEditorThread(LPVOID me);
  TCHAR m_szTempName[MAX_PATH + 1];

  bool m_isPWHidden, m_isNotesHidden;
  bool m_bWordWrap, m_bLaunchPlus;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Basic)
protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Basic)
  afx_msg void OnHelp();
  afx_msg BOOL OnKillActive();
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnRandom();
  afx_msg void OnShowPassword();
  afx_msg void OnPasskeySetFocus();
  afx_msg void OnENSetFocusNotes();
  afx_msg void OnENKillFocusNotes();
  afx_msg void OnSTCExClicked(UINT nId);
  afx_msg void OnViewDependents();
  afx_msg void OnLaunch();
  afx_msg void OnChangeURL();
  afx_msg LRESULT OnCallExternalEditor(WPARAM, LPARAM);
  afx_msg LRESULT OnExternalEditorEnded(WPARAM, LPARAM);
  afx_msg LRESULT OnWordWrap(WPARAM, LPARAM);
  afx_msg LRESULT OnShowNotes(WPARAM, LPARAM);
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SelectAllNotes();
  void ShowPassword();
  void HidePassword();
  void ShowNotes();
  void HideNotes();

  COLORREF m_group_cfOldColour, m_title_cfOldColour, m_user_cfOldColour;
  COLORREF m_pswd_cfOldColour, m_notes_cfOldColour, m_URL_cfOldColour;
  BOOL m_bOKSave, m_bOKCancel;

  CToolTipCtrl *m_pToolTipCtrl;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
