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

CString CMenuShortcut::FormatShortcut(MapMenuShortcutsIter iter, 
                                      MapKeyNameIDConstIter citer)
{
  CString str(_T(""));

  str.Format(_T("%s%s%s%s"),
      (iter->second.cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL  ? _T("Ctrl+")  : _T(""),
      (iter->second.cModifier & HOTKEYF_ALT    ) == HOTKEYF_ALT      ? _T("Alt+")   : _T(""),
      (iter->second.cModifier & HOTKEYF_SHIFT  ) == HOTKEYF_SHIFT    ? _T("Shift+") : _T(""),
      citer->second);

  return str;
}

CString CMenuShortcut::FormatShortcut(st_MenuShortcut mst, 
                                      MapKeyNameIDConstIter citer)
{
  CString str(_T(""));

  str.Format(_T("%s%s%s%s"),
      (mst.cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL  ? _T("Ctrl+")  : _T(""),
      (mst.cModifier & HOTKEYF_ALT    ) == HOTKEYF_ALT      ? _T("Alt+")   : _T(""),
      (mst.cModifier & HOTKEYF_SHIFT  ) == HOTKEYF_SHIFT    ? _T("Shift+") : _T(""),
      citer->second);

  return str;
}