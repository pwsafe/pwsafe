/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWFileDialog.h"

IMPLEMENT_DYNAMIC(CPWFileDialog, CFileDialog)

LRESULT CPWFileDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  app.GetMainDlg()->ResetIdleLockCounter(message);
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
