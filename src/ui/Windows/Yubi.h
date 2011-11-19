/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * Interface / wrapper to YubiKey API
 */
#pragma once

#import <YubiClientAPI.dll> no_namespace, named_guids

#include "SecString.h"

class Yubi {
 public:
  Yubi(CCmdTarget *owner);
  void Init(); // Call in owner's OnInit
  void Destroy(); // Call in Owner's OnDestroy
  bool isEnabled() const { return m_isInit; }
  bool isInserted() const;
  bool RequestHMACSha1(const CSecString &value); // send async request
  void RetrieveHMACSha1(CSecString &value); // get when request completed
  static stringT RetCode2String(int rc);
private:
  IYubiClient *m_obj;
  DWORD m_eventCookie;
  ycENCODING m_encoding;
  CCmdTarget *m_owner;
  bool m_isInit;
};
