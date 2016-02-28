/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file HKModifiers.h
//-----------------------------------------------------------------------------

#pragma once

#include "os/typedefs.h"

// Hotkey conversion routines!
/*
  Wonderful MS - different definitions for everything!

  Used by Windows to Register a Hotkey and also the WM_HOTKEY message
    MOD_ALT             0x01
    MOD_CONTROL         0x02
    MOD_SHIFT           0x04

  Used by CHotKeyCtrl to Get/Set the Hotkey in the control
    HOTKEYF_SHIFT       0x01 
    HOTKEYF_CONTROL     0x02
    HOTKEYF_ALT         0x04
    HOTKEYF_EXT         0x08

  Our internal versions as Windows is different to Linux/Mac
    PWS_HOTKEYF_ALT     0x01
    PWS_HOTKEYF_CONTROL 0x02
    PWS_HOTKEYF_SHIFT   0x04
    PWS_HOTKEYF_EXT     0x08

*/

inline WORD ConvertModifersPWS2MFC(const WORD &wPWSModifiers)
{
  WORD wHKModifiers(0);

  if (wPWSModifiers & PWS_HOTKEYF_ALT)
    wHKModifiers |= HOTKEYF_ALT;
  if (wPWSModifiers & PWS_HOTKEYF_CONTROL)
    wHKModifiers |= HOTKEYF_CONTROL;
  if (wPWSModifiers & PWS_HOTKEYF_SHIFT)
    wHKModifiers |= HOTKEYF_SHIFT;
  if (wPWSModifiers & PWS_HOTKEYF_EXT)
    wHKModifiers |= HOTKEYF_EXT;
    
  return wHKModifiers;
}

inline WORD ConvertModifersWindows2MFC(const WORD &wModifiers)
{
  WORD wHKModifiers(0);

  if (wModifiers & MOD_ALT)
    wHKModifiers |= HOTKEYF_ALT;
  if (wModifiers & MOD_CONTROL)
    wHKModifiers |= HOTKEYF_CONTROL;
  if (wModifiers & MOD_SHIFT)
    wHKModifiers |= HOTKEYF_SHIFT;
    
  return wHKModifiers;
}

inline WORD ConvertModifersPWS2Windows(const WORD &wPWSModifiers)
{
  WORD wModifiers(0);

  if (wPWSModifiers & PWS_HOTKEYF_ALT)
    wModifiers |= MOD_ALT;
  if (wPWSModifiers & PWS_HOTKEYF_CONTROL)
    wModifiers |= MOD_CONTROL;
  if (wPWSModifiers & PWS_HOTKEYF_SHIFT)
    wModifiers |= MOD_SHIFT;
  // No MOD_EXT !
    
  return wModifiers;
}

inline WORD ConvertModifersMFC2Windows(const WORD &wHKModifiers)
{
  WORD wModifiers(0);

  if (wHKModifiers & HOTKEYF_ALT)
    wModifiers |= MOD_ALT;
  if (wHKModifiers & HOTKEYF_CONTROL)
    wModifiers |= MOD_CONTROL;
  if (wHKModifiers & HOTKEYF_SHIFT)
    wModifiers |= MOD_SHIFT;
  // No MOD_EXT !
    
  return wModifiers;
}

inline WORD ConvertModifersWindows2PWS(const WORD &wModifiers)
{
  WORD wPWSModifiers(0);

  if (wModifiers & MOD_ALT)
    wPWSModifiers |= PWS_HOTKEYF_ALT;
  if (wModifiers & MOD_CONTROL)
    wPWSModifiers |= PWS_HOTKEYF_CONTROL;
  if (wModifiers & MOD_SHIFT)
    wPWSModifiers |= PWS_HOTKEYF_SHIFT;
  // No MOD_EXT
     
  return wPWSModifiers;
}

inline WORD ConvertModifersMFC2PWS(const WORD &wHKModifiers)
{
  WORD wPWSModifiers(0);

  if (wHKModifiers & HOTKEYF_ALT)
    wPWSModifiers |= PWS_HOTKEYF_ALT;
  if (wHKModifiers & HOTKEYF_CONTROL)
    wPWSModifiers |= PWS_HOTKEYF_CONTROL;
  if (wHKModifiers & HOTKEYF_SHIFT)
    wPWSModifiers |= PWS_HOTKEYF_SHIFT;
  if (wHKModifiers & HOTKEYF_EXT)
    wPWSModifiers |= PWS_HOTKEYF_EXT;
     
  return wPWSModifiers;
}
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
