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

      for (int i = 0; i < text.GetLength() - 1; ++i) {
        if (text[i] != L'&')
          continue;
        if (text[i + 1] == L'&') {
          ++i; // literal "&&", not a mnemonic
          continue;
        }
        if (static_cast<wchar_t>(std::towupper(text[i + 1])) == ch)
          return true;
      }
    }
    return false;
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

  return CPWPropertyPage::PreTranslateMessage(pMsg);
}
