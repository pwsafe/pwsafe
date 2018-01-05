/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * CYubiMixin is a means of sharing the common YubiKey controls
 * and related code across dialogs that provide YubiKey authentication
 *
 * Compiles to empty stub under NO_YUBI
 */

#pragma once

#include "SecString.h"
#include "os/windows/yubi/YkLib.h"

#ifndef NO_YUBI
#include <afxmt.h> // for CMutex

class CYubiMixin {
 public:
  CYubiMixin();
  // Following help us assure that if a YubiKey's
  // inserted in *any* dbox that uses it, others will reflect this.
  static bool YubiExists() {return s_yubiDetected;}
  static void SetYubiExists() {s_yubiDetected = true;}
 protected:
  void YubiPoll(); // Call this in Timer callback

  virtual void ProcessPhrase() {}; // Check the passphrase, call OnOK, OnCancel or just return
  virtual void YubiFailed() {};

  // Yubico-related:
  bool IsYubiInserted() const;
  void yubiCheckCompleted(); // called when request pending and timer fired
  // Callbacks:
  virtual void yubiShowChallengeSent() = 0; // request's in the air, setup GUI to wait for reply
  virtual void yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf) = 0; // called by yubiCheckCompleted()
  virtual void yubiInserted(void) = 0; // called when Yubikey's inserted
  virtual void yubiRemoved(void) = 0;  // called when Yubikey's removed

  void yubiRequestHMACSha1(const CSecString &challenge); // request HMAC of passkey

  StringX Bin2Hex(const unsigned char *buf, int len);
  static bool s_yubiDetected; // set if yubikey was inserted in the app's lifetime.
  mutable CYkLib m_yk;
  bool m_yubiPollDisable;
  bool m_pending; // request pending?
  bool m_present; // key present?
  mutable CMutex m_mutex; // protect against race conditions when calling Yubi API
};

#else /* NO_YUBI - all no-op */

class CYubiMixin {
public:
  CYubiMixin() {}
  static bool YubiExists() {return false;}
  static void SetYubiExists() {}
protected:
  void YubiPoll() {}

  virtual void ProcessPhrase() {}; // Check the passphrase, call OnOK, OnCancel or just return
  virtual void YubiFailed() {};

  // Yubico-related:
  bool IsYubiInserted() const {return false;}
  void yubiCheckCompleted() {}
  // Callbacks:
  virtual void yubiShowChallengeSent() = 0; // request's in the air, setup GUI to wait for reply
  virtual void yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf) = 0; // called by yubiCheckCompleted()
  virtual void yubiInserted(void) = 0; // called when Yubikey's inserted
  virtual void yubiRemoved(void) = 0;  // called when Yubikey's removed

  void yubiRequestHMACSha1(const CSecString &) {}

  StringX Bin2Hex(const unsigned char *, int ) {return StringX(L"");}

  bool m_yubiPollDisable;
  bool m_pending; // request pending?
  bool m_present; // key present?
};
#endif /* NO_YUBI */
