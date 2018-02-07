/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasskeyChangeDlg.h
//-----------------------------------------------------------------------------

#include "core/PwsPlatform.h"
#include "PKBaseDlg.h"
#include "ControlExtns.h"

class CPasskeyChangeDlg : public CPKBaseDlg
{
  // Construction
public:
  CPasskeyChangeDlg(CWnd* pParent = NULL);   // standard constructor
  ~CPasskeyChangeDlg();

  // Dialog Data
  //{{AFX_DATA(CPasskeyChangeDlg)
  enum { IDD = IDD_KEYCHANGE_DIALOG };

  CSecString m_oldpasskey;
  CSecString m_newpasskey;
  CSecString m_confirmnew;

  BOOL m_btnShowCombination;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CPasskeyChangeDlg)
protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CPasskeyChangeDlg)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnHelp();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnNewPasskeySetfocus();
  afx_msg void OnConfirmNewSetfocus();
  afx_msg void OnShowCombination();
  afx_msg void OnOldVK();
  afx_msg void OnNewVK();
  afx_msg void OnConfirmVK();
  afx_msg void OnVirtualKeyboard();
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  afx_msg void OnYubikey2Btn();
  afx_msg void OnYubikeyBtn();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  void ProcessPhrase();

  // Override base class yubi callbacks:
  void yubiInserted(void);
  void yubiRemoved(void);

private:
  CSecEditExtn *m_pctlNewPasskey;
  CSecEditExtn *m_pctlConfirmNew;
  UINT m_CtrlID;
  UINT m_LastFocus;
  bool m_Yubi1pressed; // implies old password was Yubi-based
  bool m_Yubi2pressed; // implies new password to be -"-.
  bool m_oldpasskeyConfirmed;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
