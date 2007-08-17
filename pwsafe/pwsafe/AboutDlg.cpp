/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
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
