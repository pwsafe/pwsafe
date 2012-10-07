/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

class CYubiMixin {
 public:
 CYubiMixin() : m_pollingTimer(NULL), m_present(false), m_btn(NULL), m_status(NULL) {}
  ~CYubiMixin() {delete m_pollingTimer;}

  void SetupMixin(wxWindow *btn, wxWindow *status);
  void yubiInserted(void);
  void yubiRemoved(void);
  bool IsYubiInserted() const;
  StringX Bin2Hex(const unsigned char *buf, int len) const;

  // Following should be called in timer event handler of mixed-in class
  // Don't forget to add an entry in the event table, something like
  // EVT_TIMER(POLLING_TIMER_ID, CFoo::OnPollingTimer)
  void HandlePollingTimer();

  enum { POLLING_TIMER_ID = 83 } ; 
  wxTimer* m_pollingTimer;
  bool m_present; // key present?

 private:
  wxWindow *m_btn;
  wxWindow *m_status;
};

#endif /* _YUBIMXIN_H_ */
