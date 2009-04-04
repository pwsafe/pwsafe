/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SHCTHotKey.cpp : implementation file
//

#include "stdafx.h"
#include "SHCTHotKey.h"
#include "SHCTListCtrl.h"

// SHCTHotKey

IMPLEMENT_DYNAMIC(CSHCTHotKey, CHotKeyCtrl)

CSHCTHotKey::CSHCTHotKey()
: m_pParent(NULL)
{
}

CSHCTHotKey::~CSHCTHotKey()
{
}

BEGIN_MESSAGE_MAP(CSHCTHotKey, CHotKeyCtrl)
  ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// SHCTHotKey message handlers

void CSHCTHotKey::OnKillFocus(CWnd *)
{
  if (m_pParent != NULL) {
    WORD vVK, vMod;
    GetHotKey(vVK, vMod);
    m_pParent->OnHotKeyKillFocus(vVK, vMod);
  }
}
