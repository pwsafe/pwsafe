/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "WZPropertyPage.h"
#include "ControlExtns.h"
#include "YubiMixin.h"

#include "resource.h"

class CVKeyBoardDlg;

class CWZSelectDB : public CWZPropertyPage, public CYubiMixin
{
public:
  DECLARE_DYNAMIC(CWZSelectDB)

  CWZSelectDB(CWnd *pParent, int idd, UINT nIDCaption, const int nType);
  ~CWZSelectDB();

  CString m_defexpdelim;

protected:
  CSecEditExtn *m_pctlPasskey, *m_pctlPasskey2, *m_pctlVerify2;
  CEditExtn *m_pctlDB;
  CSecString m_passkey, m_passkey2, m_verify2;
  CStaticExtn m_stc_warning;
  CString m_filespec;
  int m_tries, m_state;
  int m_bAdvanced, m_bExportDBFilters;
  CButtonBitmapExtn m_ctlSDToggle;

  BOOL OnInitDialog();
  void DoDataExchange(CDataExchange* pDX);
  LRESULT OnWizardNext();

  // Generated message map functions
  //{{AFX_MSG(CWZSelectDB)
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  virtual BOOL OnSetActive();
  afx_msg void OnHelp();
  afx_msg void OnPassKeyChange();
  afx_msg void OnPassKey2Change();
  afx_msg void OnVerify2Change();
  afx_msg void OnDatabaseChange();
  afx_msg void OnOpenFileBrowser();
  afx_msg void OnVirtualKeyboard();
  afx_msg void OnAdvanced();
  afx_msg void OnExportFilters();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskey2Setfocus();
  afx_msg void OnVerify2keySetfocus();
  afx_msg void OnYubikeyBtn();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg LRESULT OnInsertBuffer(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void AFXAPI DDV_CheckExpDelimiter(CDataExchange* pDX,
                                    const CString &delimiter);
  enum {KEYPRESENT = 1, DBPRESENT = 2, BOTHPRESENT = 3,
        KEY2PRESENT = 4, VERIFY2PRESENT = 8, KEY2_EQ_VERIFY2 = 16, ALLPRESENT = 31};

  COLORREF m_syncwarning_cfOldColour;

  bool ProcessPhrase(const StringX &filename, const StringX &passkey);
  bool m_bFileExistsUserAsked;

  CVKeyBoardDlg *m_pVKeyBoardDlg;
  bool m_bVKAvailable;

  HWND m_hwndVKeyBoard;

  st_SaveAdvValues *m_pst_SADV;
  CFont m_WarningFont;

  // Following should be private inheritance of CPKBaseDlg,
  // but MFC doesn't allow us to do this. So much for OOD.
  static const wchar_t PSSWDCHAR;

  UINT m_CtrlID;
  UINT m_LastFocus;

  // Yubico-related:
  // Callbacks:
  virtual void yubiShowChallengeSent(); // request's in the air, setup GUI to wait for reply
  virtual void yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf); // called by yubiCheckCompleted()
  virtual void yubiInserted(void); // called when Yubikey's inserted
  virtual void yubiRemoved(void);  // called when Yubikey's removed

  // Indicate that we're waiting for user to activate YubiKey:
  CProgressCtrl m_yubi_timeout;
  // Show user what's going on / what we're waiting for:
  CEdit m_yubi_status;
  CBitmap m_yubiLogo;
  CBitmap m_yubiLogoDisabled;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
