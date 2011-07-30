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
#include "Yubi.h"

class CPKBaseDlg : public CPWDialog {
 public:
 CPKBaseDlg(int id, CWnd *pParent) : CPWDialog(id, pParent) {}
  virtual ~CPKBaseDlg();
 protected:
  CSecString m_passkey;
  virtual void ProcessPhrase() = 0; // Check the passphrase, call OnOK, OnCancel or just return
  // Yubico-related:
  Yubi *m_yubi;
	void yubiInserted(void);
	void yubiRemoved(void);
  void yubiCompleted(ycRETCODE rc);
  void yubiWait(WORD seconds);
  // Indicate that we're waiting for user to activate YubiKey:
  CProgressCtrl m_yubi_timeout;
  // Show user what's going on / what we're waiting for:
  CEdit m_yubi_status;
};
