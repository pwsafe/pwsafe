/*
* Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * CPKBaseDlg is a common base class for all dialogs that deal with the master passkey
 * In particular, it has common code for dealing with YubiKey authentication
 */

#pragma once

#include "PWDialog.h"
#include "ControlExtns.h"
#include "GetMasterPhrase.h"
#include "YubiMixin.h"

#include <limits>

class CVKeyBoardDlg;

/**
 * Base class for all dialog boxes that handle master passwords.
 *
 * For Yubikey support, the principle is that yubi-specific controls are
 * totally hidden unless/until user inserts a Yubikey for the 1st time
 * in the app's lifetime, after which controls are visible, but enabled/disabled
 * to reflect inserted/removed state of the device.
 */

class CPKBaseDlg : public CPWDialog , public CYubiMixin {
public:
  CPKBaseDlg(int id, CWnd *pParent, bool bUseSecureDesktop);
  virtual ~CPKBaseDlg();
  BOOL OnInitDialog(void);
  BOOL PreTranslateMessage(MSG* pMsg);
  void DoDataExchange(CDataExchange* pDX);

  const CSecString &GetPassKey() const {return m_passkey;}

protected:
  friend class CWZSelectDB;

  CButtonBitmapExtn m_ctlSDToggle;
  CSecString m_passkey;
  CSecEditExtn *m_pctlPasskey;
  CVKeyBoardDlg *m_pVKeyBoardDlg;
  static const wchar_t PSSWDCHAR;
  int m_index;
  bool m_bVKAvailable;

  CToolTipCtrl *m_pToolTipCtrl;

  // Generated message map functions
  //{{AFX_MSG(CPKBaseDlg)
  afx_msg void OnSwitchSecureDesktop();
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  // non-Secure Desktop use of the virtual keyboard
  HWND m_hwndVKeyBoard;

  // Secure Desktop
  void StartThread(int iDialogType, HMONITOR hCurrentMonitor = NULL);
  GetMasterPhrase m_GMP;
  DWORD m_dwRC;  // SD Thread exit code
  bool m_bUseSecureDesktop;

  virtual void yubiShowChallengeSent(); // request's in the air, setup GUI to wait for reply
  virtual void yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf);
  virtual void yubiInserted(void); // called when Yubikey's inserted
  virtual void yubiRemoved(void);  // called when Yubikey's removed

  // Indicate that we're waiting for user to activate YubiKey:
  CProgressCtrl m_yubi_timeout;
  // Show user what's going on / what we're waiting for:
  CEdit m_yubi_status;
  CBitmap m_yubiLogo;
  CBitmap m_yubiLogoDisabled;

private:
  enum {
     WINDOWSHOOKREMOVED         = 0x01,
     WAITABLETIMERCREATED       = 0x02,
     WAITABLETIMERSET           = 0x03,
     THREADCREATED              = 0x08,
     THREADRESUMED              = 0x10,
   };

};
