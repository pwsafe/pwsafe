/// \file TryAgainDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
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
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CTryAgainDlg::OnTryagain() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}


void
CTryAgainDlg::OnHelp() 
{
   WinHelp(0x2008F, HELP_CONTEXT);
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
