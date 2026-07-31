/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "StdAfx.h"
#include "PasswordSafe.h"

#include "AddEdit_Basic_Tabs.h"

#include "PWSDarkMode.h"
#include "winutils.h"

#include <cwctype>

BEGIN_MESSAGE_MAP(CAddEdit_Basic_Tabs, CPropertySheet)
  ON_WM_SIZE()
END_MESSAGE_MAP()

CAddEdit_Basic_Tabs::CAddEdit_Basic_Tabs(CWnd *pPropertySheetParent, st_AE_master_data *pAEMD)
  : CPropertySheet(L"", pPropertySheetParent),
    m_pp_notes(pPropertySheetParent, pAEMD),
    m_pp_customFields(pPropertySheetParent, pAEMD)
{
  AddPage(&m_pp_notes);
  AddPage(&m_pp_customFields);
}

void CAddEdit_Basic_Tabs::CancelThreadWait()
{
  m_pp_notes.CancelThreadWait();
}

BOOL CAddEdit_Basic_Tabs::Create(CWnd *pParentWnd, const CRect &rect)
{
  m_psh.dwFlags |= PSH_MODELESS | PSH_NOAPPLYNOW;

  if (CPropertySheet::Create(pParentWnd, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0) == FALSE)
    return FALSE;

  MoveWindow(rect);
  LayoutPages();
  return TRUE;
}

BOOL CAddEdit_Basic_Tabs::OnInitDialog()
{
  CPropertySheet::OnInitDialog();

  DarkMode::setWindowEraseBgSubclass(m_hWnd);
  DarkMode::setDarkWndNotifySafe(m_hWnd, true);

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  const UINT buttonIds[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };
  for (UINT buttonId : buttonIds) {
    CWnd *pButton = GetDlgItem(buttonId);
    if (pButton != nullptr)
      pButton->ShowWindow(SW_HIDE);
  }

  LayoutPages();
  return TRUE;
}

void CAddEdit_Basic_Tabs::OnSize(UINT nType, int cx, int cy)
{
  CPropertySheet::OnSize(nType, cx, cy);
  LayoutPages();
}

void CAddEdit_Basic_Tabs::LayoutPages()
{
  if (GetSafeHwnd() == nullptr)
    return;

  CTabCtrl *pTab = GetTabControl();
  if (pTab == nullptr || pTab->GetSafeHwnd() == nullptr)
    return;

  CRect rcClient;
  GetClientRect(&rcClient);
  pTab->MoveWindow(rcClient);

  CRect rcPage;
  pTab->GetClientRect(&rcPage);
  pTab->AdjustRect(FALSE, &rcPage);

  if (m_pp_notes.GetSafeHwnd() != nullptr)
    m_pp_notes.MoveWindow(rcPage);
  if (m_pp_customFields.GetSafeHwnd() != nullptr)
    m_pp_customFields.MoveWindow(rcPage);
}

bool CAddEdit_Basic_Tabs::IsExternalEditorActive() const
{
  return m_pp_notes.IsExternalEditorActive();
}

void CAddEdit_Basic_Tabs::ActivatePage(CAddEdit_Basic_SubPage *pPage)
{
  if (GetActivePage() != pPage)
    SetActivePage(pPage);

  pPage->FocusDefaultControl();
}

BOOL CAddEdit_Basic_Tabs::PreTranslateMessage(MSG *pMsg)
{
  // The single home for every subtab's accelerator: both CAddEdit_Basic (when
  // one of its fields has focus) and our subtabs (via
  // CAddEdit_Basic_SubPage, when they don't own the accelerator themselves)
  // route WM_SYSCHAR here.
  //
  // Each accelerator letter is marked with a leading '&' in the corresponding
  // subtab's caption, as read from the tab control for proper I18N support.
  if (pMsg->message == WM_SYSCHAR) {
    CAddEdit_Basic_SubPage *const pages[] = { &m_pp_notes, &m_pp_customFields };
    CTabCtrl *pTabCtrl = GetTabControl();
    const wchar_t ch = static_cast<wchar_t>(std::towupper(static_cast<wchar_t>(pMsg->wParam)));

    if (pTabCtrl != nullptr) {
      for (CAddEdit_Basic_SubPage *pPage : pages) {
        const int iPage = GetPageIndex(pPage);
        if (iPage < 0)
          continue;

        wchar_t szTabText[256];
        TCITEM tci = {};
        tci.mask = TCIF_TEXT;
        tci.pszText = szTabText;
        tci.cchTextMax = _countof(szTabText);

        if (pTabCtrl->GetItem(iPage, &tci) && WinUtil::GetMnemonicChar(szTabText) == ch) {
          ActivatePage(pPage);
          return TRUE;
        }
      }
    }
  }

  return CPropertySheet::PreTranslateMessage(pMsg);
}
