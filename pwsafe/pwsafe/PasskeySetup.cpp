/// \file PasskeySetup.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "util.h"

#include "PasskeySetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CPasskeySetup::CPasskeySetup(CWnd* pParent)
   : CDialog(CPasskeySetup::IDD, pParent)
{
   m_passkey = "";
   m_verify = "";
}


void CPasskeySetup::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_PASSKEY, m_passkey.m_mystring);
   DDX_Text(pDX, IDC_VERIFY, m_verify.m_mystring);
}


BEGIN_MESSAGE_MAP(CPasskeySetup, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()


void CPasskeySetup::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void CPasskeySetup::OnOK()
{
   UpdateData(TRUE);
   if (m_passkey != m_verify)
   {
      AfxMessageBox("The two entries do not match.");
      ((CEdit*)GetDlgItem(IDC_VERIFY))->SetFocus();
      return;
   }

   if (m_passkey == "")
   {
      AfxMessageBox("Please enter a key and verify it.");
      ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetFocus();
      return;
   }
   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}


void CPasskeySetup::OnHelp() 
{
   //WinHelp(0x20084, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_intro.htm",
              HH_DISPLAY_TOPIC, 0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
