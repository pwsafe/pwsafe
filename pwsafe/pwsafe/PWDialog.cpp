/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "PWDialog.h"
#include "DboxMain.h"

#if defined(POCKET_PC)
#error "TBD - define proper Dialog base class for PPC"
#endif

IMPLEMENT_DYNAMIC(CPWDialog, CDialog)

LRESULT CPWDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  // list of all the events that signify actual user activity, as opposed
  // to Windows internal events...
  if (message == WM_KEYDOWN ||
      message == WM_COMMAND ||
      message == WM_SYSCOMMAND ||
      message == WM_MOUSEMOVE ||
      message == WM_MOVE ||
      message == WM_LBUTTONDOWN ||
      message == WM_LBUTTONDBLCLK ||
      message == WM_CONTEXTMENU ||
      message == WM_MENUSELECT ||
      message == WM_VSCROLL
      ) {
    CWnd *p = GetParent();
    while (p != NULL) {
      DboxMain *dbx = dynamic_cast<DboxMain *>(p);
      if (dbx != NULL) {
        dbx->ResetIdleLockCounter();
        break;
      } else
        p = p->GetParent();
    }
    if (p == NULL)
      TRACE(_T("CPWDialog::WindowProc - couldn't find DboxMain ancestor\n"));
  }
  return CDialog::WindowProc(message, wParam, lParam);
}
