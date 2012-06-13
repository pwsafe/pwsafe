/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

CString CMenuShortcut::FormatShortcut(unsigned char cModifier, unsigned char cVirtKey)
{
  CString str(L"");

  if (cVirtKey && IsNormalShortcut(cModifier, cVirtKey)){
    str = CHotKeyCtrl::GetKeyName(cVirtKey, cModifier & HOTKEYF_EXT);
    if (str.GetLength() == 1)
      str.MakeUpper();
  if ((cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL)
      str = CS_CTRLP + str;
  if ((cModifier & HOTKEYF_ALT) == HOTKEYF_ALT)
      str = CS_ALTP + str;
  if ((cModifier & HOTKEYF_SHIFT) == HOTKEYF_SHIFT)
      str = CS_SHIFTP + str;
  }
  return str;
}

bool CMenuShortcut::IsNormalShortcut(unsigned char cModifier, unsigned char cVirtKey)
{
  UNREFERENCED_PARAMETER(cModifier);
  //reserved or unassigned by Windows
  //http://msdn.microsoft.com/en-us/library/dd375731%28v=VS.85%29.aspx
  return (cVirtKey < 0xE0) &&//OEM specific and "Media"  keys
         (cVirtKey != 0x07) && (cVirtKey != 0x0A) &&
         (cVirtKey != 0x0B) && (cVirtKey != 0x0E) &&
         (cVirtKey != 0x0F) && (cVirtKey != 0x16) && 
         (cVirtKey != 0x1A) && (cVirtKey != 0x1B) && //0x1B -- Esc
         ((cVirtKey < 0x3A) || (cVirtKey > 0x40)) &&
         ((cVirtKey < 0x5B) || (cVirtKey > 0x5F)) &&//0x5B -- Left Win, 0x5C -- Right Win, 0x5D -- App key, 0x5F -- Sleep
         ((cVirtKey < 0x88) || (cVirtKey > 0x8F)) &&
         ((cVirtKey < 0x92) || (cVirtKey > 0x9F)) &&//0x92-0x96 -- OEM specific 
         ((cVirtKey < 0xA6) || (cVirtKey > 0xB9)) &&//0xA6-0xB7 -- "Media" keys
         ((cVirtKey < 0xC1) || (cVirtKey > 0xDA));
}