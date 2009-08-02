/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWDialog.h"

#if defined(POCKET_PC)
#error "TBD - define proper Dialog base class for PPC"
#endif

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CPWDialog, CDialog)

LRESULT CPWDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  // List of all the events that signify actual user activity, as opposed
  // to Windows internal events...
  if ((message >= WM_KEYFIRST && message <= WM_KEYLAST)     ||
      (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
      message == WM_COMMAND       ||
      message == WM_SYSCOMMAND    ||
      message == WM_VSCROLL       ||
      message == WM_HSCROLL       ||
      message == WM_MOVE          ||
      message == WM_SIZE          ||
      message == WM_CONTEXTMENU   ||
      message == WM_MENUSELECT) {
    CWnd *p = GetParent();
    while (p != NULL) {
      DboxMain *pDbx = dynamic_cast<DboxMain *>(p);
      if (pDbx != NULL && pDbx->m_eye_catcher != NULL &&
          wcscmp(pDbx->m_eye_catcher, EYE_CATCHER) == 0) {
        pDbx->ResetIdleLockCounter();
        break;
      } else
        p = p->GetParent();
    }
    if (p == NULL)
      TRACE(L"CPWFileDialog::WindowProc - couldn't find DboxMain ancestor\n");
  }
  return CDialog::WindowProc(message, wParam, lParam);
}

INT_PTR CPWDialog::DoModal()
{
  bool bAccEn = app.IsAcceleratorEnabled();
  if (bAccEn)
    app.DisableAccelerator();

  INT_PTR rc = CDialog::DoModal();

  if (bAccEn)
    app.EnableAccelerator();

  return rc;
}
