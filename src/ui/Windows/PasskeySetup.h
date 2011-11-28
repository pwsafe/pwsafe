/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasskeySetup.h
//-----------------------------------------------------------------------------

#include "core/PwsPlatform.h"
#include "PKBaseDlg.h"
#include "ControlExtns.h"

class CVKeyBoardDlg;
class DboxMain;

class CPasskeySetup : public CPKBaseDlg
{
  // Construction
public:
  CPasskeySetup(CWnd* pParent = NULL);   // standard constructor
  ~CPasskeySetup();

protected:
  // Dialog Data
  //{{AFX_DATA(CPasskeySetup)
  enum { IDD = IDD_PASSKEYSETUP };
  CSecString m_verify;
  //}}AFX_DATA

  virtual BOOL OnInitDialog();
  //{{AFX_VIRTUAL(CPasskeySetup)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CPasskeySetup)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnVerifykeySetfocus();
#if defined(POCKET_PC)
  afx_msg void OnPasskeyKillfocus();
#endif
  afx_msg void OnVirtualKeyboard();
  afx_msg void OnYubikeyBtn();
  void ProcessPhrase(); // Check the passphrase, call OnOK, OnCancel or just return
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
private:
  DboxMain *m_pDbx;
  CSecEditExtn *m_pctlVerify;
  CVKeyBoardDlg *m_pVKeyBoardDlg;
  UINT m_CtrlID;
  UINT m_LastFocus;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
