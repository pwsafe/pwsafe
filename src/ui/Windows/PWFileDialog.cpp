/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWFileDialog.h"

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CPWFileDialog, CFileDialog)

LRESULT CPWFileDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (app.GetMainDlg()->m_eye_catcher != NULL &&
      wcscmp(app.GetMainDlg()->m_eye_catcher, EYE_CATCHER) == 0) {
    app.GetMainDlg()->ResetIdleLockCounter(message);
  } else
    pws_os::Trace(L"CPWFileDialog::WindowProc - couldn't find DboxMain ancestor\n");

  return CFileDialog::WindowProc(message, wParam, lParam);
}

INT_PTR CPWFileDialog::DoModal()
{
  bool bAccEn = app.IsAcceleratorEnabled();
  if (bAccEn)
    app.DisableAccelerator();

  CPWDialog::GetDialogTracker()->AddOpenDialog(this);
  INT_PTR rc = CFileDialog::DoModal();
  CPWDialog::GetDialogTracker()->RemoveOpenDialog(this);

  if (bAccEn)
    app.EnableAccelerator();

  return rc;
}
