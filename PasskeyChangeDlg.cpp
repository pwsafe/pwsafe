/// \file PasskeyChangeDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "PasskeyChangeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CPasskeyChangeDlg::CPasskeyChangeDlg(CWnd* pParent)
   : CDialog(CPasskeyChangeDlg::IDD, pParent)
{
   m_confirmnew = "";
   m_newpasskey = "";
   m_oldpasskey = "";
}


void
CPasskeyChangeDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_CONFIRMNEW, m_confirmnew.m_mystring);
   DDX_Text(pDX, IDC_NEWPASSKEY, m_newpasskey.m_mystring);
   DDX_Text(pDX, IDC_OLDPASSKEY, m_oldpasskey.m_mystring);
}


BEGIN_MESSAGE_MAP(CPasskeyChangeDlg, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()


void
CPasskeyChangeDlg::OnOK() 
{
   UpdateData(TRUE);
   if (m_oldpasskey != app.m_passkey)
      AfxMessageBox("The old safe combination is not correct");
   else if (m_confirmnew != m_newpasskey)
      AfxMessageBox("New safe combination and confirmation do not match");
   else if (m_newpasskey == "")
      AfxMessageBox("The new safe combination cannot be blank.");
   else
   {
      app.m_pMainWnd = NULL;
      CDialog::OnOK();
   }
}


void
CPasskeyChangeDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CPasskeyChangeDlg::OnHelp() 
{
   WinHelp(0x20083, HELP_CONTEXT);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
