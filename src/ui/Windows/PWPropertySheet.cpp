/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
  m_pDbx =  static_cast<DboxMain *>(pParent);
  m_bLongPPs = bLongPPs;

  m_psh.dwFlags |= PSH_HASHELP;
}

CPWPropertySheet::CPWPropertySheet(LPCTSTR pszCaption, CWnd* pParent, const bool bLongPPs)
  : CPropertySheet(pszCaption, pParent), m_bKeepHidden(false)
{
  m_pDbx =  static_cast<DboxMain *>(pParent);
  m_bLongPPs = bLongPPs;

  m_psh.dwFlags |= PSH_HASHELP;
}

BEGIN_MESSAGE_MAP(CPWPropertySheet, CPropertySheet)
  ON_WM_WINDOWPOSCHANGING()
  ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

LRESULT CPWPropertySheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  CWnd *pParent = GetParent();
  while (pParent != NULL) {
    DboxMain *pDbx = dynamic_cast<DboxMain *>(pParent);
    if (pDbx != NULL && pDbx->m_eye_catcher != NULL &&
        wcscmp(pDbx->m_eye_catcher, EYE_CATCHER) == 0) {
      pDbx->ResetIdleLockCounter(message);
      break;
    } else
      pParent = pParent->GetParent();
  }
  if (pParent == NULL)
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
  if (m_bLongPPs && !m_pDbx->LongPPs(this)) {
    EndDialog(-1);
    return TRUE;
  }

  // It's OK - show it
  m_bKeepHidden = false;
  ShowWindow(SW_SHOW);
  return TRUE;
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
    if (bAccEn)app.EnableAccelerator();

  return rc;
}
