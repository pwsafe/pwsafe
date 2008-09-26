/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// 
// KeySend.h
// thedavecollins 2004-08-07
// sends keystrokes
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "corelib/StringX.h"

class CKeySend
{
public:
  CKeySend(void);
  ~CKeySend(void);
  void SendString(const StringX &data);
  void ResetKeyboardState();
  void SendChar(TCHAR c);
  void SetDelay(int d);
  void SetAndDelay(int d);
  void SetCapsLock(const bool bstate);

private:
  void OldSendChar(TCHAR c);
  void NewSendChar(TCHAR c);
  int m_delay;
  HKL m_hlocale;
  bool m_isOldOS;
};

