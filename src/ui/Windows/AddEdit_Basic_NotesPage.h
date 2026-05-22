/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "resource.h"

#include "AddEdit_Basic_SubPage.h"
#include "ControlExtns.h"
#include "ExtThread.h"
#include "TBMStatic.h"

class CAddEdit_Basic_NotesPage : public CAddEdit_Basic_SubPage
{
public:
  DECLARE_DYNAMIC(CAddEdit_Basic_NotesPage)

  CAddEdit_Basic_NotesPage(CWnd *pParent, st_AE_master_data *pAEMD);

  enum { IDD = IDD_ADDEDIT_BASIC_NOTES, IDD_SHORT = IDD_ADDEDIT_BASIC_NOTES_SHORT };

  static CString CS_EXTERNAL_EDITOR;
  static CString CS_HIDDEN_NOTES;
  static HANDLE ghEvents[2];

  void CancelThreadWait() { SetEvent(ghEvents[1]); }
  bool IsExternalEditorActive() const { return m_bUsingNotesExternalEditor; }

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  afx_msg void OnENChangeNotes();
  afx_msg void OnENKillFocusNotes();
  afx_msg LRESULT OnCallExternalEditor(WPARAM, LPARAM);
  afx_msg LRESULT OnExternalEditorEnded(WPARAM, LPARAM);
  afx_msg LRESULT OnWordWrap(WPARAM, LPARAM);
  afx_msg LRESULT OnZoomNotes(WPARAM, LPARAM);

  DECLARE_MESSAGE_MAP()

private:
  static UINT ExternalEditorThread(LPVOID me);

  void HideNotes(const bool bForceHide = false);
  void ResetHiddenNotes();
  void SelectAllNotes();
  void SetZoomMenu();
  void ShowNotes(const bool bForceShow = false);

  CRichEditExtn m_ex_notes;
  CEditExtn m_ex_hidden_notes;
  CTBMStatic m_Help;

  CExtThread *m_thread;
  wchar_t m_szTempName[MAX_PATH + 1];
  bool m_isNotesHidden;
  bool m_bWordWrap;
  bool m_bUsingNotesExternalEditor;
  bool m_bOKSave;
  bool m_bOKCancel;
  bool m_bInitdone;
  int m_iPointSize;
};
