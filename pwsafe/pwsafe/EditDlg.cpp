/// \file EditDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "EditDlg.h"
#include "PwFont.h"

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
      ShowPassword();
   }
   else if (wndName == "&Hide Password")
   {
      m_realpassword = m_password;
      HidePassword();
   }
   else
      AfxMessageBox("Error in retrieving window text");

   UpdateData(FALSE);
}


void CEditDlg::OnOK() 
{
   UpdateData(TRUE);

   /*
    *  If the password is shown it may have been edited,
    *  so save the current text.
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

   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}


void CEditDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


BOOL CEditDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
 
   SetPasswordFont(GetDlgItem(IDC_PASSWORD));

   if (app.GetProfileInt("", "showpwdefault", FALSE) == TRUE)
   {
      ShowPassword();
   }
   else
   {
      HidePassword();
   }
   UpdateData(FALSE);
   return TRUE;
}


void CEditDlg::ShowPassword(void)
{
   m_password = m_realpassword;
   m_isPwHidden = false;
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText("&Hide Password");
   GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
}


void CEditDlg::HidePassword(void)
{
   m_password = HIDDEN_PASSWORD;
   m_isPwHidden = true;
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText("&Show Password");
   GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
}


void CEditDlg::OnRandom() 
{
   UINT pwlen = app.GetProfileInt("", "pwlendefault", 8);
   CMyString temp = GetAlphaNumPassword(pwlen);

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
      {
	m_password = HIDDEN_PASSWORD;
      }
      else if (wndName == "&Hide Password")
      {
         m_password = m_realpassword;
      }
      UpdateData(FALSE);
   }
   else if (nResponse == IDNO)
   {
   }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
