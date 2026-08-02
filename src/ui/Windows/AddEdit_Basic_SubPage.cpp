/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "StdAfx.h"
#include "AddEdit_Basic_SubPage.h"
#include "AddEdit_PropertySheet.h"
#include "resource3.h"
#include "winutils.h"

#include <cwctype>

namespace {
  // True if a direct child of pWnd displays an (unescaped) "&x" mnemonic that
  // case-insensitively matches ch - i.e. pWnd's own dialog-message handling
  // has a real reason to claim a WM_SYSCHAR for ch.
  bool HasLocalMnemonic(CWnd *pWnd, wchar_t ch)
  {
    ch = static_cast<wchar_t>(std::towupper(ch));

    for (CWnd *pChild = pWnd->GetWindow(GW_CHILD); pChild != nullptr;
         pChild = pChild->GetWindow(GW_HWNDNEXT)) {
      CString text;
      pChild->GetWindowText(text);
      if (WinUtil::GetMnemonicChar(text) == ch)
        return true;
    }
    return false;
  }

  // True for a key the currently focused window has explicitly claimed via
  // WM_GETDLGCODE (arrows/paging, or plain characters), meaning it should go
  // straight to that window rather than through this page's own
  // dialog-navigation handling. Tab/Return/Escape/F1 are left alone even when
  // claimed, since other levels rely on handling those themselves.
  bool FocusWantsKey(const MSG *pMsg)
  {
    CWnd *pFocus = CWnd::GetFocus();
    if (pFocus == nullptr)
      return false;

    const UINT dlgCode = static_cast<UINT>(
        pFocus->SendMessage(WM_GETDLGCODE, 0, reinterpret_cast<LPARAM>(pMsg)));

    if (pMsg->message == WM_CHAR)
      return (dlgCode & DLGC_WANTCHARS) != 0;

    if (pMsg->message != WM_KEYDOWN)
      return false;

    switch (pMsg->wParam) {
      case VK_TAB: case VK_RETURN: case VK_ESCAPE: case VK_F1:
        return false;
      case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
      case VK_HOME: case VK_END: case VK_PRIOR: case VK_NEXT:
        return (dlgCode & DLGC_WANTARROWS) != 0;
      default:
        // Not a navigation key: WM_KEYDOWN must still be translated ourselves
        // for type-ahead's WM_CHAR to ever get generated at all.
        return (dlgCode & DLGC_WANTCHARS) != 0;
    }
  }
}

IMPLEMENT_DYNAMIC(CAddEdit_Basic_SubPage, CPWPropertyPage)

CAddEdit_Basic_SubPage::CAddEdit_Basic_SubPage(CWnd *pParent, UINT nID,
                                               UINT nID_Short,
                                               st_AE_master_data *pAEMD)
  : CPWPropertyPage(pAEMD->bLongPPs ? nID : nID_Short),
    m_AEMD(*pAEMD),
    m_ae_psh((CAddEdit_PropertySheet *)pParent)
{
}

void CAddEdit_Basic_SubPage::NotifyChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  m_ae_psh->SetChanged(true);
}

BOOL CAddEdit_Basic_SubPage::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_SYSCHAR &&
      !HasLocalMnemonic(this, static_cast<wchar_t>(pMsg->wParam))) {
    // None of this page's own controls are a mnemonic for this key, so this
    // page has nothing useful to do with it. Offer it to our parent directly
    // instead of just leaving it alone for the framework's own
    // PreTranslateMessage tree-walk to (maybe) carry further up: that walk
    // does not reliably reach past this page for every kind of hosted control.
    CWnd *pParent = GetParent();
    if (pParent != nullptr && pParent->PreTranslateMessage(pMsg))
      return TRUE;
  }

  if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_CHAR) && FocusWantsKey(pMsg)) {
    // The base class's default handling (CDialog::PreTranslateMessage) calls
    // IsDialogMessage() scoped to this page's own window, an embedded WS_CHILD
    // dialog that was never actually activated - which doesn't reliably pass a
    // key through to a control that asked for it via WM_GETDLGCODE. Deliver it
    // directly instead.
    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return TRUE;
  }

  return CPWPropertyPage::PreTranslateMessage(pMsg);
}
