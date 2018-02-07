/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWPropertySheet.h"

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CPWPropertySheet, CPropertySheet)

CPWPropertySheet::CPWPropertySheet(UINT nID, CWnd *pParent, const bool bLongPPs)
  : CPropertySheet(nID, pParent), m_bKeepHidden(true)
{
  m_bLongPPs = bLongPPs;

  m_psh.dwFlags |= PSH_HASHELP;
}

CPWPropertySheet::CPWPropertySheet(LPCTSTR pszCaption, CWnd* pParent, const bool bLongPPs)
  : CPropertySheet(pszCaption, pParent), m_bKeepHidden(false)
{
  m_bLongPPs = bLongPPs;

  m_psh.dwFlags |= PSH_HASHELP;
}

BEGIN_MESSAGE_MAP(CPWPropertySheet, CPropertySheet)
  ON_WM_WINDOWPOSCHANGING()
  ON_WM_SHOWWINDOW()
  ON_WM_MENUCHAR()
END_MESSAGE_MAP()

DboxMain *CPWPropertySheet::GetMainDlg() const
{
  return app.GetMainDlg();
}

LRESULT CPWPropertySheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (GetMainDlg()->m_eye_catcher != NULL &&
      wcscmp(GetMainDlg()->m_eye_catcher, EYE_CATCHER) == 0) {
    GetMainDlg()->ResetIdleLockCounter(message);
  } else
    pws_os::Trace(L"CPWPropertySheet::WindowProc - couldn't find DboxMain ancestor\n");

  return CPropertySheet::WindowProc(message, wParam, lParam);
}

void CPWPropertySheet::OnWindowPosChanging(WINDOWPOS *lpwndpos)
{
  // This ensures PropertySheet is not shown during OnInitDialog default processing
  // if we are going to end it immediately
  if(m_bKeepHidden)
    lpwndpos->flags &= ~SWP_SHOWWINDOW;

  CPropertySheet::OnWindowPosChanging(lpwndpos);
}

void CPWPropertySheet::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if(!m_bKeepHidden)
    CPropertySheet::OnShowWindow(bShow, nStatus);
}

BOOL CPWPropertySheet::OnInitDialog()
{
  CPropertySheet::OnInitDialog();

  // If started with Tall and won't fit - return to be called again with Wide
  if (m_bLongPPs && !GetMainDlg()->LongPPs(this)) {
    EndDialog(-1);
    return TRUE;
  }

  // It's OK - show it
  m_bKeepHidden = false;
  ShowWindow(SW_SHOW);
  
  return TRUE;  // return TRUE unless you set the focus to a control
}

INT_PTR CPWPropertySheet::DoModal()
{
  bool bAccEn = app.IsAcceleratorEnabled();
  if (bAccEn)
    app.DisableAccelerator();

  CPWDialog::GetDialogTracker()->AddOpenDialog(this);
  INT_PTR rc = CPropertySheet::DoModal();
  CPWDialog::GetDialogTracker()->RemoveOpenDialog(this);

  if (bAccEn)
    app.EnableAccelerator();

  return rc;
}

LRESULT CPWPropertySheet::OnMenuChar(UINT nChar, UINT nFlags, CMenu *pMenu)
{
  // Stop beeps when presing Alt+<key> in the HotKeyCtrls
  const int nID = GetFocus()->GetDlgCtrlID();

  // IDs correspond to AddEdit_Additional Entry Keyboard Shortcut Hotkey and
  // OptionsShortcuts PWS shortcut HotKeys
  if (nID == IDC_ENTKBSHCTHOTKEY || nID == IDC_SHORTCUTHOTKEY)
    return MNC_CLOSE << 16;

 return CPropertySheet::OnMenuChar(nChar, nFlags, pMenu);
}
