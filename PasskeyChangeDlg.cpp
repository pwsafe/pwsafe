/// \file PasskeyChangeDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "corelib/PWCharPool.h" // for CheckPassword()
#include "ThisMfcApp.h"
#include "resource.h"

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
   m_confirmnew = _T("");
   m_newpasskey = _T("");
   m_oldpasskey = _T("");
}


void
CPasskeyChangeDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_CONFIRMNEW, (CString &)m_confirmnew);
   DDX_Text(pDX, IDC_NEWPASSKEY, (CString &)m_newpasskey);
   DDX_Text(pDX, IDC_OLDPASSKEY, (CString &)m_oldpasskey);
}


BEGIN_MESSAGE_MAP(CPasskeyChangeDlg, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()


void
CPasskeyChangeDlg::OnOK() 
{
   CMyString errmess;

   UpdateData(TRUE);
   if (m_oldpasskey != global.m_passkey)
      AfxMessageBox(_T("The old safe combination is not correct"));
   else if (m_confirmnew != m_newpasskey)
      AfxMessageBox(_T("New safe combination and confirmation do not match"));
   else if (m_newpasskey.IsEmpty())
      AfxMessageBox(_T("The new safe combination cannot be blank."));
   else if (!CPasswordCharPool::CheckPassword(m_newpasskey, errmess)) {
     CString msg(_T("Weak password:\n"));
     msg += CString(errmess);
     msg += _T("\nAccept anyway?");
     if (AfxMessageBox(msg, MB_YESNO) == IDYES) {
       app.m_pMainWnd = NULL;
       CDialog::OnOK();
     }
   } else {
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
   //WinHelp(0x20083, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_combo_chg.htm",
              HH_DISPLAY_TOPIC, 0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
