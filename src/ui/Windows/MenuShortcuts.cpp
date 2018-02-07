/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file MenuShortcuts.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "MenuShortcuts.h"
#include "resource3.h"

CString CMenuShortcut::CS_CTRLP(L"Error");
CString CMenuShortcut::CS_ALTP(L"Error");
CString CMenuShortcut::CS_SHIFTP(L"Error");

void CMenuShortcut::InitStrings()
{
  CS_CTRLP.LoadString(IDS_CTRLP);
  CS_ALTP.LoadString(IDS_ALTP);
  CS_SHIFTP.LoadString(IDS_SHIFTP);
}

CString CMenuShortcut::FormatShortcut(WORD wPWSModifiers, WORD wVirtualKeyCode)
{
  CString str(L"");

  if (wVirtualKeyCode && IsNormalShortcut(wVirtualKeyCode)){
    str = CHotKeyCtrl::GetKeyName(wVirtualKeyCode,
                        (wPWSModifiers & PWS_HOTKEYF_EXT) == PWS_HOTKEYF_EXT);
    if (str.GetLength() == 1)
      str.MakeUpper();
    if ((wPWSModifiers & PWS_HOTKEYF_SHIFT) == PWS_HOTKEYF_SHIFT)
      str = CS_SHIFTP + str;
    if ((wPWSModifiers & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL)
      str = CS_CTRLP + str;
    if ((wPWSModifiers & PWS_HOTKEYF_ALT) == PWS_HOTKEYF_ALT)
      str = CS_ALTP + str;
  }
  return str;
}

bool CMenuShortcut::IsNormalShortcut(WORD wVirtualKeyCode)
{
  //reserved or unassigned by Windows
  //http://msdn.microsoft.com/en-us/library/dd375731%28v=VS.85%29.aspx
  return (wVirtualKeyCode  < 0xE0) && //OEM specific and "Media"  keys
         (wVirtualKeyCode != 0x07) && (wVirtualKeyCode != 0x0A) &&
         (wVirtualKeyCode != 0x0B) && (wVirtualKeyCode != 0x0E) &&
         (wVirtualKeyCode != 0x0F) && (wVirtualKeyCode != 0x16) && 
         (wVirtualKeyCode != 0x1A) && (wVirtualKeyCode != 0x1B) && //0x1B -- Esc
         ((wVirtualKeyCode < 0x3A) || (wVirtualKeyCode > 0x40)) &&
         //0x5B -- Left Win, 0x5C -- Right Win, 0x5D -- App key, 0x5F -- Sleep
         ((wVirtualKeyCode < 0x5B) || (wVirtualKeyCode > 0x5F)) &&
         ((wVirtualKeyCode < 0x88) || (wVirtualKeyCode > 0x8F)) &&
         //0x92-0x96 -- OEM specific 
         ((wVirtualKeyCode < 0x92) || (wVirtualKeyCode > 0x9F)) &&
         //0xA6-0xB7 -- "Media" keys
         ((wVirtualKeyCode < 0xA6) || (wVirtualKeyCode > 0xB9)) &&
         ((wVirtualKeyCode < 0xC1) || (wVirtualKeyCode > 0xDA));
}
