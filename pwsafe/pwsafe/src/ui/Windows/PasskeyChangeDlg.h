/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasskeyChangeDlg.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"
#include "PWDialog.h"
#include "ControlExtns.h"

class CVKeyBoardDlg;

class CPasskeyChangeDlg : public CPWDialog
{
  // Construction
public:
  CPasskeyChangeDlg(CWnd* pParent = NULL);   // standard constructor
  ~CPasskeyChangeDlg();

  HINSTANCE m_OSK_module;

  // Dialog Data
  //{{AFX_DATA(CPasskeyChangeDlg)
  enum { IDD = IDD_KEYCHANGE_DIALOG };
  CSecString m_confirmnew;
  CSecString m_newpasskey;
  CSecString m_oldpasskey;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CPasskeyChangeDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CPasskeyChangeDlg)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnHelp();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnNewPasskeySetfocus();
  afx_msg void OnConfirmNewSetfocus();
#if defined(POCKET_PC)
  afx_msg void OnPasskeyKillfocus();
#endif
  afx_msg void OnOldVK();
  afx_msg void OnNewVK();
  afx_msg void OnConfirmVK();
  afx_msg void OnVirtualKeyboard();
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CSecEditExtn * m_pctlOldPasskey;
  CSecEditExtn * m_pctlNewPasskey;
  CSecEditExtn * m_pctlConfirmNew;
  CVKeyBoardDlg *m_pVKeyBoardDlg;
  UINT m_CtrlID;
  UINT m_LastFocus;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
