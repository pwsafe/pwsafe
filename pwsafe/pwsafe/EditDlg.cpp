/// \file EditDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "EditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_NOTES, (CString &)m_notes);
   DDX_Text(pDX, IDC_PASSWORD, (CString &)m_password);
   DDX_Text(pDX, IDC_USERNAME, (CString &)m_username);
   DDX_Text(pDX, IDC_TITLE, (CString &)m_title);
}


BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
   ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowpassword)
   ON_BN_CLICKED(IDHELP, OnHelp)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_RANDOM, OnRandom)
END_MESSAGE_MAP()


void CEditDlg::OnShowpassword() 
{
   UpdateData(TRUE);

   CMyString wndName;
   GetDlgItem(IDC_SHOWPASSWORD)->GetWindowText(wndName);

   if (wndName == "&Show Password")
   {
      m_password = m_realpassword;
      GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText("&Hide Password");
      GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
      m_isPwHidden = false;
   }
   else if (wndName == "&Hide Password")
   {
      m_realpassword = m_password;
      m_password = GetAsterisk(m_realpassword);
      GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText("&Show Password");
      GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
      m_isPwHidden = true;
   }
   else
      AfxMessageBox("Error in retrieving window text");

   UpdateData(FALSE);
}


void CEditDlg::OnOK() 
{
   UpdateData(TRUE);

   /*
    * they may have changed the password
    */

   if (! m_isPwHidden)
      m_realpassword = m_password;

   //Check that data is valid
   if (m_title == "")
   {
      AfxMessageBox("This entry must have a title.");
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
      return;
   }

   // JPRFIXME - P2.4
#if 0
   if(m_password == "")
   {
      AfxMessageBox("This entry must have a password.");
      ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
      return;
   }
#endif

   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}


void CEditDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


CMyString CEditDlg::GetAsterisk(CMyString base)
{
   CMyString temp;

   for (int x=0; x<base.GetLength(); x++)
      temp += "*";
   return temp;
}


BOOL CEditDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
 
   if (app.GetProfileInt("", "showpwdefault", FALSE) == TRUE)
   {
      m_password = m_realpassword;
      GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText("&Hide Password");
      GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
      UpdateData(FALSE);
   }
   return TRUE;
}


void CEditDlg::OnRandom() 
{
   CMyString temp;

   for (int x=0; x<8; x++)
      temp += GetRandAlphaNumChar();

   UpdateData(TRUE);
   CMyString msg;
   int nResponse;
 
   //Ask if something's there
   if (m_password != "")
   {
      msg =
         "The randomly generated password is: \""
         + temp
         + "\" (without\nthe quotes). Would you like to use it?";
      nResponse = MessageBox(msg, 
                             AfxGetAppName(),
                             MB_ICONEXCLAMATION|MB_YESNO);
   }
   else 
      nResponse = IDYES;

   if (nResponse == IDYES)
   {
      m_realpassword = temp;

      CMyString wndName;
      GetDlgItem(IDC_SHOWPASSWORD)->GetWindowText(wndName);

      if (wndName == "&Show Password")
         m_password = GetAsterisk(m_realpassword);
      else if (wndName == "&Hide Password")
         m_password = m_realpassword;
      UpdateData(FALSE);
   }
   else if (nResponse == IDNO)
   {
   }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
