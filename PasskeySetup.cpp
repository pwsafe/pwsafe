/// \file PasskeySetup.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "corelib/PWCharPool.h" // for CheckPassword()
#include "ThisMfcApp.h"
#include "PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PocketPC.h"
#else
  #include "resource.h"
#endif

#include "util.h"

#include "PasskeySetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CPasskeySetup::CPasskeySetup(CWnd* pParent)
   : super(CPasskeySetup::IDD, pParent)
{
   m_passkey = _T("");
   m_verify = _T("");
}


void CPasskeySetup::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_PASSKEY, (CString &)m_passkey);
   DDX_Text(pDX, IDC_VERIFY, (CString &)m_verify);
}


BEGIN_MESSAGE_MAP(CPasskeySetup, super)
   ON_BN_CLICKED(ID_HELP, OnHelp)
#if defined(POCKET_PC)
   ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
   ON_EN_SETFOCUS(IDC_VERIFY, OnPasskeySetfocus)
   ON_EN_KILLFOCUS(IDC_PASSKEY, OnPasskeyKillfocus)
   ON_EN_KILLFOCUS(IDC_VERIFY, OnPasskeyKillfocus)
#endif
END_MESSAGE_MAP()


void CPasskeySetup::OnCancel() 
{
   app.m_pMainWnd = NULL;
   super::OnCancel();
}


void CPasskeySetup::OnOK()
{
   UpdateData(TRUE);
   if (m_passkey != m_verify)
   {
      AfxMessageBox(_T("The two entries do not match."));
      ((CEdit*)GetDlgItem(IDC_VERIFY))->SetFocus();
      return;
   }

   if (m_passkey.IsEmpty())
   {
      AfxMessageBox(_T("Please enter a key and verify it."));
      ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetFocus();
      return;
   }

   CMyString errmess;
   if (!CPasswordCharPool::CheckPassword(m_passkey, errmess)) {
     CString msg(_T("Weak password:\n"));
     msg += CString(errmess);
     msg += _T("\nAccept anyway?");
     if (AfxMessageBox(msg, MB_YESNO) == IDNO)
       return;
   }


   app.m_pMainWnd = NULL;
   super::OnOK();
}


void CPasskeySetup::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#newdatabase"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
   //WinHelp(0x20084, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_intro.htm",
              HH_DISPLAY_TOPIC, 0);
#endif
}


#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CPasskeySetup::OnPasskeyKillfocus()
{
	EnableWordCompletion( m_hWnd );
}


/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CPasskeySetup::OnPasskeySetfocus()
{
	DisableWordCompletion( m_hWnd );
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
