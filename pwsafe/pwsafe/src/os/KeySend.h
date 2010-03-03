/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// 
// KeySend.h
// Adapted from ui/Windows/KeySend.h
//-----------------------------------------------------------------------------

#include "../corelib/StringX.h"

class CKeySend
{
public:
  CKeySend();
  ~CKeySend();
  void SendString(const StringX &data);
  void ResetKeyboardState();
  void SetDelay(unsigned d);
  void SetAndDelay(unsigned d);
  void SetCapsLock(bool bstate);

protected:
  unsigned m_delayMS; //delay after each key in milliseconds
};

