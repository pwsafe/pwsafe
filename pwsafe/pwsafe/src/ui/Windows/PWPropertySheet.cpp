/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

CPWPropertySheet::CPWPropertySheet(UINT nID, CWnd *pParent)
  : CPropertySheet(nID, pParent)
{
  m_psh.dwFlags |= PSH_HASHELP;
}

CPWPropertySheet::CPWPropertySheet(LPCTSTR pszCaption, CWnd* pParent)
  : CPropertySheet(pszCaption, pParent)
{
  m_psh.dwFlags |= PSH_HASHELP;
}

bool CPWPropertySheet::chooseResource()
{
  // based on current screen height, decide if we want to display
  // the normal "tall/long" page, or the "wide/short" version (for netbooks)
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(MonitorFromWindow(m_psh.hwndParent, MONITOR_DEFAULTTONEAREST), &mi);
  const int Y = abs(mi.rcWork.bottom - mi.rcWork.top);

  return (Y > 600); // THRESHOLD = 600 - pixels or virtual-screen coordinates?
}

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
