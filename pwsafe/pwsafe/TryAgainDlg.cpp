/// \file TryAgainDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "PwsPlatform.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "TryAgainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CTryAgainDlg::CTryAgainDlg(CWnd* pParent)
   : super(CTryAgainDlg::IDD, pParent)
{
   cancelreturnval = TAR_INVALID;
}


void
CTryAgainDlg::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTryAgainDlg, super)
   ON_BN_CLICKED(IDC_QUIT, OnQuit)
   ON_BN_CLICKED(IDC_TRYAGAIN, OnTryagain)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_OPEN, OnOpen)
   ON_BN_CLICKED(IDC_NEW, OnNew)
END_MESSAGE_MAP()


void
CTryAgainDlg::OnQuit() 
{
   app.m_pMainWnd = NULL;
   super::OnCancel();
}


void
CTryAgainDlg::OnTryagain() 
{
   app.m_pMainWnd = NULL;
   super::OnOK();
}


void
CTryAgainDlg::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#comboerror"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
   //WinHelp(0x2008F, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_combo_err.htm",
              HH_DISPLAY_TOPIC, 0);
#endif
}


void
CTryAgainDlg::OnOpen() 
{
   cancelreturnval = TAR_OPEN;
   super::OnCancel();
}


void
CTryAgainDlg::OnNew() 
{
   cancelreturnval = TAR_NEW;
   super::OnCancel();
}


int
CTryAgainDlg::GetCancelReturnValue()
{
   return cancelreturnval;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
