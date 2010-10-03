/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWFileDialog.h"
#include <dlgs.h>

#include "resource.h"
#include "resource3.h"

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CPWFileDialog, CFileDialog)

BEGIN_MESSAGE_MAP(CPWFileDialog, CFileDialog)
  //{{AFX_MSG_MAP(CPWFileDialog)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

LRESULT CPWFileDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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

BOOL CPWFileDialog::OnInitDialog()
{
  CFileDialog::OnInitDialog();

  if (m_bAttachment && m_bOpenFileDialog == TRUE) {
    // Enlarge standard CFileDialog to make space for our controls
    const UINT iExtraSize = 35;
    const UINT iMoveDown = 30;
    const UINT iMoveRight = 15;

    // Get a pointer to the original dialog box.
    CWnd *wndDlg = GetParent();

    // Change OK button to say "Add File"
    CString cs_button(MAKEINTRESOURCE(IDS_ADDFILE));
    wndDlg->SetDlgItemText(IDOK, cs_button);

    // Change the size of FileOpen dialog
    RECT Rect;
    wndDlg->GetWindowRect(&Rect);
    wndDlg->SetWindowPos(NULL, 0, 0,
                         Rect.right - Rect.left,
                         Rect.bottom - Rect.top + iExtraSize,
                         SWP_NOMOVE);

    // Standard CFileDialog control ID's are defined in <dlgs.h>
    // cmb1 - standard file name comboi box control
    CWnd *wndCtrl = wndDlg->GetDlgItem(cmb1);
    ASSERT(wndCtrl != NULL);
    wndCtrl->GetWindowRect(&Rect);
    wndDlg->ScreenToClient(&Rect);

    // Put our control(s) somewhere below HIDDEN checkbox
    Rect.top += iMoveDown;
    Rect.bottom += iMoveDown;

    // Move over to make room for our static text
    Rect.left += iMoveRight;
    Rect.right += iMoveRight;

    // IMPORTANT: We must put wndDlg here as hWndParent,
    m_edtDesc.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,
                     Rect, wndDlg, IDC_ATTDESC);
    m_edtDesc.SetFont(wndCtrl->GetFont(), TRUE);

    // We also need Static Control. Get coordinates from
    // stc2 control and make it bigger
    CWnd *wndStaticCtrl = wndDlg->GetDlgItem(stc2);
    ASSERT(wndDlg != NULL);
    wndStaticCtrl->GetWindowRect(&Rect);
    wndDlg->ScreenToClient(&Rect);
    Rect.top += iMoveDown;
    Rect.bottom += iMoveDown;
    Rect.right += 20;

    // our static control
    m_stcDesc.Create(L"Optional Description:", WS_CHILD | WS_VISIBLE, Rect,
                     wndDlg, IDC_STATIC_ATTDESC);
    m_stcDesc.SetFont(wndStaticCtrl->GetFont(), TRUE);

    // Move over to make room for our static text
    wndCtrl = wndDlg->GetDlgItem(cmb13);  // File name combo-box
    wndCtrl->GetWindowRect(&Rect);
    wndDlg->ScreenToClient(&Rect); // Remember it is child controls
    Rect.left += iMoveRight;
    wndCtrl->SetWindowPos(NULL, Rect.left, Rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    wndCtrl = wndDlg->GetDlgItem(edt1); // Edit field in Filename
    if (wndCtrl != NULL) {
      // Note: In Vista, this control has been removed. Back in W7
      wndCtrl->GetWindowRect(&Rect);
      wndDlg->ScreenToClient(&Rect); // Remember it is child controls
      Rect.left += iMoveRight;
      wndCtrl->SetWindowPos(NULL, Rect.left, Rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    wndCtrl = wndDlg->GetDlgItem(cmb1); // File type combo-box
    wndCtrl->GetWindowRect(&Rect);
    wndDlg->ScreenToClient(&Rect); // Remember it is child controls
    Rect.left += iMoveRight;
    wndCtrl->SetWindowPos(NULL, Rect.left, Rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }
  return TRUE;
}

void CPWFileDialog::OnDestroy()
{
  if (m_bAttachment && m_bOpenFileDialog == TRUE) {
    m_edtDesc.GetWindowText(m_csDesc);
  }

  CFileDialog::OnDestroy();
}
