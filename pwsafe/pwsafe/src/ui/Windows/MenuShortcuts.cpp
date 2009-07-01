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

CString CMenuShortcut::FormatShortcut(MapMenuShortcutsIter iter, 
                                      MapKeyNameIDConstIter citer)
{
  CString str(L"");

  str.Format(L"%s%s%s%s",
      (iter->second.cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL  ? CS_CTRLP  : L"",
      (iter->second.cModifier & HOTKEYF_ALT    ) == HOTKEYF_ALT      ? CS_ALTP   : L"",
      (iter->second.cModifier & HOTKEYF_SHIFT  ) == HOTKEYF_SHIFT    ? CS_SHIFTP : L"",
      citer->second);

  return str;
}

CString CMenuShortcut::FormatShortcut(st_MenuShortcut mst, 
                                      MapKeyNameIDConstIter citer)
{
  CString str(L"");

  str.Format(L"%s%s%s%s",
      (mst.cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL  ? CS_CTRLP  : L"",
      (mst.cModifier & HOTKEYF_ALT    ) == HOTKEYF_ALT      ? CS_ALTP   : L"",
      (mst.cModifier & HOTKEYF_SHIFT  ) == HOTKEYF_SHIFT    ? CS_SHIFTP : L"",
      citer->second);

  return str;
}