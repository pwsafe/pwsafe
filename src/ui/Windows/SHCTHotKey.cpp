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

#include "resource.h"

// SHCTHotKey

IMPLEMENT_DYNAMIC(CSHCTHotKey, CHotKeyCtrl)

CSHCTHotKey::CSHCTHotKey()
: m_pParent(NULL), m_bHandled(false)
{
}

CSHCTHotKey::~CSHCTHotKey()
{
}

BEGIN_MESSAGE_MAP(CSHCTHotKey, CHotKeyCtrl)
  ON_WM_KILLFOCUS()
  ON_WM_KEYUP()
  ON_WM_KEYDOWN()
  ON_WM_CHAR()
END_MESSAGE_MAP()

// SHCTHotKey message handlers

void CSHCTHotKey::OnKillFocus(CWnd *pWnd)
{
  UNREFERENCED_PARAMETER(pWnd);
  if (m_pParent != NULL) {
    WORD wVK, wMod;
    GetHotKey(wVK, wMod);
    m_pParent->OnHotKeyKillFocus(wVK, wMod);
  }
}

void CSHCTHotKey::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (nChar != VK_DELETE && nChar != VK_SPACE && nChar != VK_ESCAPE)
    CHotKeyCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSHCTHotKey::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (nChar == VK_DELETE || nChar == VK_SPACE || nChar == VK_ESCAPE) {
    WORD wVK, wMod;
    GetHotKey(wVK, wMod);

    // No current key means don't want to use the current modifiers.
    if (wVK != 0)
      wMod = 0;

    // Set extended to make sure we get DEL and not NUM DECIMAL
    if (nChar == VK_DELETE)
      wMod |= HOTKEYF_EXT;

    wVK = (WORD)nChar;
    SetHotKey(wVK, wMod);

    m_pParent->SendMessage(WM_COMMAND, MAKEWPARAM(IDC_SHORTCUTHOTKEY, EN_CHANGE), 0);
  } else
    CHotKeyCtrl::OnKeyUp(nChar, nRepCnt, nFlags);

}

void CSHCTHotKey::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (nChar != ' ')
    CHotKeyCtrl::OnChar(nChar, nRepCnt, nFlags);
}
