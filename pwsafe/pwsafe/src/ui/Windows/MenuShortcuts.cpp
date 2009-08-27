/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

CString CMenuShortcut::FormatShortcut(unsigned char cModifier, MapKeyNameIDConstIter citer)
{
  CString str(L"");

  if ((cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL)
    str += CS_CTRLP;
  if ((cModifier & HOTKEYF_ALT) == HOTKEYF_ALT)
    str += CS_ALTP;
  if ((cModifier & HOTKEYF_SHIFT) == HOTKEYF_SHIFT)
    str += CS_SHIFTP;

  str += citer->second;

  return str;
}
