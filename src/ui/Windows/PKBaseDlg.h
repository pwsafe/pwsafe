/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "Yubi.h"

class CPKBaseDlg : public CPWDialog {
 public:
  CPKBaseDlg(int id, CWnd *pParent);
  virtual ~CPKBaseDlg();
  BOOL OnInitDialog(void);

  CSecString GetPassKey() const {return m_passkey;}

 protected:
  CSecString m_passkey;
  CSecEditExtn *m_pctlPasskey;
  static const wchar_t PSSWDCHAR;

  virtual void ProcessPhrase() {}; // Check the passphrase, call OnOK, OnCancel or just return
  virtual void DoDataExchange(CDataExchange* pDX);
  afx_msg void OnDestroy();
  // Yubico-related:
  bool IsYubiEnabled() const {return m_yubi->isEnabled();}
  bool IsYubiInserted() const {return m_yubi->isInserted();}
  // Callbacks:
	void yubiInserted(void); // called when Yubikey's inserted
	void yubiRemoved(void);  // called when Yubikey's removed
  void yubiCompleted(ycRETCODE rc); // called when done with request
  void yubiWait(WORD seconds); // called when waiting for user activation

  void yubiRequestHMACSha1(); // request HMAC of m_passkey
  // Indicate that we're waiting for user to activate YubiKey:
  CProgressCtrl m_yubi_timeout;
  // Show user what's going on / what we're waiting for:
  CEdit m_yubi_status;
	DECLARE_INTERFACE_MAP()
 private:
  Yubi *m_yubi; // Interface to Yubikey API  
  bool m_waited; // needed to discern between timeout and unconfigured yubikey
};
