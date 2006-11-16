/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file TryAgainDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "TryAgainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CTryAgainDlg::CTryAgainDlg(CWnd* pParent)
   : CDialog(CTryAgainDlg::IDD, pParent)
{
   cancelreturnval = TAR_INVALID;
}


void
CTryAgainDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTryAgainDlg, CDialog)
   ON_BN_CLICKED(IDC_QUIT, OnQuit)
   ON_BN_CLICKED(IDC_TRYAGAIN, OnTryagain)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_OPEN, OnOpen)
   ON_BN_CLICKED(IDC_NEW, OnNew)
END_MESSAGE_MAP()


void
CTryAgainDlg::OnQuit() 
{
   CDialog::OnCancel();
}


void
CTryAgainDlg::OnTryagain() 
{
   CDialog::OnOK();
}


void
CTryAgainDlg::OnHelp() 
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/create_new_db.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}


void
CTryAgainDlg::OnOpen() 
{
   cancelreturnval = TAR_OPEN;
   CDialog::OnCancel();
}


void
CTryAgainDlg::OnNew() 
{
   cancelreturnval = TAR_NEW;
   CDialog::OnCancel();
}


int
CTryAgainDlg::GetCancelReturnValue()
{
   return cancelreturnval;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
