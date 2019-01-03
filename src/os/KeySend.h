/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// 
// KeySend.h
// Originally Windows-only, now implemented for both Windows and Linux
// Original version by thedavecollins
// Windows implementation updated by dk
// Linux implementation by sauravg
//-----------------------------------------------------------------------------

#include "typedefs.h"
#include "../core/StringX.h"

class CKeySendImpl; // for os-specific stuff

class CKeySend
{
public:
  static bool LookupVirtualKey(const StringX &kname, WORD &kval); // os-specific
  /**
     Above maps the following into OS-specific codes:
    Enter  Enter key
    Up     Up-arrow key
    Down   Down-arrow down key
    Left   Left-arrow key
    Right  Right-arrow key
    Home   Home key
    End    End key
    PgUp   Page-up key
    PgDn   Page-down key
    Tab    Tab key
    Space  Space key
   */

  static stringT GetKeyName(WORD wVirtualKeyCode, bool bExtended);

  CKeySend(bool bForceOldMethod = false, unsigned defaultDelay = 10); // bForceOldMethod's Windows-specific
  ~CKeySend();
  void SendString(const StringX &data);
  void SendVirtualKey(WORD wVK, bool bAlt, bool bCtrl, bool bShift);
  void ResetKeyboardState() const;
  void SetDelay(unsigned d);
  void SetAndDelay(unsigned d);
  bool isCapsLocked() const ;
  void SetCapsLock(bool bstate);
  void BlockInput(bool bi) const;

  void SetOldSendMethod(bool bForceOldMethod); // currently Windows-only

  void SelectAll() const;
  void EmulateMods(bool emulate);
  bool IsEmulatingMods() const;

#ifdef __PWS_MACINTOSH__  
  bool SimulateApplicationSwitch();
#endif  

private:
  unsigned m_delayMS; //delay between keystrokes in milliseconds
  CKeySendImpl *m_impl;
};
