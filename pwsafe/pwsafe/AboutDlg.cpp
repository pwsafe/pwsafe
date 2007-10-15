/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/// \file AboutDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "AboutDlg.h"
#include "resource.h"
#include "resource3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAboutDlg::CAboutDlg(CWnd* pParent)
   : CPWDialog(CAboutDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CAboutDlg, CPWDialog)
   ON_BN_CLICKED(IDC_CHECKNEWVER, &CAboutDlg::OnBnClickedCheckNewVer)
END_MESSAGE_MAP()

BOOL
CAboutDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();
#ifdef DEMO
  m_appversion += _T(" ") + CString(MAKEINTRESOURCE(IDS_DEMO));
#endif
  GetDlgItem(IDC_APPVERSION)->SetWindowText(m_appversion);
  GetDlgItem(IDC_APPCOPYRIGHT)->SetWindowText(m_appcopyright);

  return TRUE;
}

void CAboutDlg::OnBnClickedCheckNewVer()
{
  // Get the latest.xml file from our site, compare to version,
  // and notify the user
  // First, make sure database is closed: Sensitive data with an
  // open socket makes me uneasy...
  DboxMain *dbx = static_cast<DboxMain *>(GetParent());

  if (dbx->GetNumEntries() != 0) {
    const CString cs_txt(MAKEINTRESOURCE(IDS_CLOSE_B4_CHECK));
    const CString cs_title(MAKEINTRESOURCE(IDS_CONFIRM_CLOSE));
    int rc = MessageBox(cs_txt, cs_title,
                        (MB_ICONQUESTION | MB_OKCANCEL));
    if (rc == IDCANCEL)
      return; // no hard feelings
    // Close database, prompt for save if changed
    dbx->SendMessage(WM_COMMAND, ID_MENUITEM_CLOSE);
    // User could have cancelled save, need to check if really closed:
    if (dbx->GetNumEntries() != 0)
      return;
  }
  ASSERT(dbx->GetNumEntries() == 0);
  // safe to open external connection
}
