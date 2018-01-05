/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// EntryKBHotKey.cpp : implementation file
//

#include "stdafx.h"
#include "EntryKBHotKey.h"
#include "AddEdit_Additional.h"

#include "resource.h"

// EntryKBHotKey

IMPLEMENT_DYNAMIC(CEntryKBHotKey, CHotKeyCtrl)

CEntryKBHotKey::CEntryKBHotKey()
: m_pParent(NULL)
{
}

CEntryKBHotKey::~CEntryKBHotKey()
{
}

BEGIN_MESSAGE_MAP(CEntryKBHotKey, CHotKeyCtrl)
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// EntryKBHotKey message handlers

BOOL CEntryKBHotKey::PreTranslateMessage(MSG *pMsg)
{
  static const wchar_t *wcValidKeys = 
             L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // Verify valid Entry Keyboard shortcut character (alphanumeric only)
  if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) {
    switch (pMsg->wParam) {
      case VK_SHIFT:
      case VK_CONTROL:
      case VK_MENU:
        break;
      default:
        if (wcschr(wcValidKeys, (wchar_t)pMsg->wParam) == NULL)
          return TRUE;
    }
  }

  // If "context code" [bit 29] set with a system key
  if ((pMsg->lParam & 0x20000000) &&
      (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_SYSKEYUP)) {
    switch (pMsg->wParam) {
      case VK_SHIFT:
      case VK_CONTROL:
      case VK_MENU:
        break;
      default:
        if (wcschr(wcValidKeys, (wchar_t)pMsg->wParam) == NULL)
          return TRUE;
    }
  }
  return CHotKeyCtrl::PreTranslateMessage(pMsg);
}

void CEntryKBHotKey::OnKillFocus(CWnd *pNewWnd)
{
  CWnd::OnKillFocus(pNewWnd);

  if (m_pParent != NULL) {
    m_pParent->OnEntryHotKeyKillFocus();
  }
}

void CEntryKBHotKey::OnSetFocus(CWnd *pOldWnd)
{
  CWnd::OnSetFocus(pOldWnd);

  if (m_pParent != NULL) {
    m_pParent->OnEntryHotKeySetFocus();
  }
}
