/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file YubiMixin.h
 *
 * As the name implies, a mixin class for common Yubi-related
 * functionality to be shared by dialogs that have:
 * 1. A Yubikey button 
 * 2. A static text for displaying status
 * These are passed in SetupMixin(), which must be called ASAP.
 */

#ifndef _YUBIMXIN_H_
#define _YUBIMXIN_H_

#include "core/StringX.h"
class wxTimer;
class wxWindow;

class CYubiMixin {
 public:
  enum {POLLING_INTERVAL = 500}; // mSec
 CYubiMixin() : m_present(false), m_btn(nullptr), m_status(nullptr) {}
  ~CYubiMixin() {}

  void SetupMixin(wxWindow *btn, wxWindow *status);
  bool yubiExists() const;
  void yubiInserted(void);
  void yubiRemoved(void);
  bool IsYubiInserted() const;

  void UpdateStatus(); // calls yubiRemoved() or yubiInserted() per m_present

  // Following to override default.
  // prompt1 defaults to "<- Click on button to the left"
  // prompt2 defaults to "Now touch your YubiKey's button"
  void SetPrompt1(const wxString &prompt) { m_prompt1 = prompt; }
  void SetPrompt2(const wxString &prompt) { m_prompt2 = prompt; }

  bool PerformChallengeResponse(wxWindow *win,
             const StringX &challenge, StringX &response,
             bool oldYubiChallenge = false);
  StringX Bin2Hex(const unsigned char *buf, int len) const;

  // Following should be called in timer event handler of mixed-in class
  // Don't forget to add an entry in the event table, something like
  // EVT_TIMER(POLLING_TIMER_ID, CFoo::OnPollingTimer)
  void HandlePollingTimer(); // calls UpdateStatus() iff m_present changes

  enum { POLLING_TIMER_ID = 83 } ; 
  bool m_present; // key present?

 private:
  wxWindow *m_btn;
  wxWindow *m_status;
  wxString m_prompt1;
  wxString m_prompt2;
};

#endif /* _YUBIMXIN_H_ */
