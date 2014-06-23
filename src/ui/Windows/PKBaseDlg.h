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
#include "SysColStatic.h"
#include "GetMasterPhrase.h"

#include "os/windows/yubi/YkLib.h"

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

class CPKBaseDlg : public CPWDialog {
public:
  CPKBaseDlg(int id, CWnd *pParent, bool bUseSecureDesktop);
  virtual ~CPKBaseDlg();
  BOOL OnInitDialog(void);

  const CSecString &GetPassKey() const {return m_passkey;}

  // Following help us assure that if a YubiKey's
  // inserted in *any* dbox that uses it, others will reflect this.
  static bool YubiExists() {return s_yubiDetected;}
  static void SetYubiExists() {s_yubiDetected = true;}

protected:
  friend class CWZSelectDB;

  CSysColStatic m_ctlSDToggle;
  CSecString m_passkey;
  CSecEditExtn *m_pctlPasskey;
  CVKeyBoardDlg *m_pVKeyBoardDlg;
  static const wchar_t PSSWDCHAR;
  int m_index;
  bool m_bVKAvailable;

  // Generated message map functions
  //{{AFX_MSG(CPKBaseDlg)
  afx_msg void OnSwitchSecureDesktop();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  virtual void ProcessPhrase() {}; // Check the passphrase, call OnOK, OnCancel or just return
  virtual void YubiFailed() {};
  virtual void DoDataExchange(CDataExchange* pDX);
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT_PTR nIDEvent);

  // non-Secure Desktop use of the virtual keyboard
  HWND m_hwndVKeyBoard;

  // Secure Desktop
  void GetDimmedScreen(CBitmap &bmpDimmedScreen);
  void StartThread(int iDialogType);
  GetMasterPhrase m_GMP;
  DWORD m_dwRC;  // SD Thread exit code
  bool m_bUseSecureDesktop;

  // Yubico-related:
  bool IsYubiInserted() const;
  // Callbacks:
  virtual void yubiInserted(void); // called when Yubikey's inserted
  virtual void yubiRemoved(void);  // called when Yubikey's removed
  void yubiCheckCompleted(); // called when request pending and timer fired

  void yubiRequestHMACSha1(); // request HMAC of m_passkey
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
     WAITABLETIMERSET           = 0x04,
     DIMMENDSCREENBITMAPCREATED = 0x08,
     THREADCREATED              = 0x10,
     THREADRESUMED              = 0x20,
   };

  // Yubico-related:
  static bool s_yubiDetected; // set if yubikey was inserted in the app's lifetime.
  mutable CYkLib m_yk;
  bool m_pending; // request pending?
  bool m_present; // key present?
  mutable CMutex m_mutex; // protect against race conditions when calling Yubi API
};
