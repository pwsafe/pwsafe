/// \file CryptKeyEntry.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"
#include "util.h"

#include "CryptKeyEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CCryptKeyEntry::CCryptKeyEntry(CWnd* pParent)
   : CDialog(CCryptKeyEntry::IDD, pParent)
{
   m_cryptkey1 = "";
   m_cryptkey2 = "";
}


void CCryptKeyEntry::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_CRYPTKEY1, (CString &)m_cryptkey1);
   DDX_Text(pDX, IDC_CRYPTKEY2, (CString &)m_cryptkey2);
}


BEGIN_MESSAGE_MAP(CCryptKeyEntry, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()


void
CCryptKeyEntry::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CCryptKeyEntry::OnOK()
{
   UpdateData(TRUE);

   if (m_cryptkey1 != m_cryptkey2)
   {
      AfxMessageBox("The two entries do not match.");
      ((CEdit*)GetDlgItem(IDC_CRYPTKEY2))->SetFocus();
      return;
   }
   if (m_cryptkey1 == "")
   {
      AfxMessageBox("Please enter the key and verify it.");
      ((CEdit*)GetDlgItem(IDC_CRYPTKEY1))->SetFocus();
      return;
   }

   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}


void
CCryptKeyEntry::OnHelp() 
{
   //WinHelp(0x20084, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_combo_entry.htm",
              HH_DISPLAY_TOPIC, 0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
