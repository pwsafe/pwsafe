/// \file EditDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "EditDlg.h"
#include "PwFont.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(POCKET_PC)
  #define SHOW_PASSWORD_TXT	_T("S")
  #define HIDE_PASSWORD_TXT	_T("H")
#else
  #define SHOW_PASSWORD_TXT	_T("&Show Password")
  #define HIDE_PASSWORD_TXT	_T("&Hide Password")
#endif


void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_NOTES, (CString &)m_notes);
   DDX_Text(pDX, IDC_PASSWORD, (CString &)m_password);
   DDX_Text(pDX, IDC_USERNAME, (CString &)m_username);
   DDX_Text(pDX, IDC_TITLE, (CString &)m_title);
   DDX_Text(pDX, IDC_GROUP, (CString &)m_group);
}


BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
   ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowpassword)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_RANDOM, OnRandom)
#if defined(POCKET_PC)
   ON_WM_SHOWWINDOW()
   ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetfocus)
   ON_EN_KILLFOCUS(IDC_PASSWORD, OnPasskeyKillfocus)
#endif
END_MESSAGE_MAP()


void CEditDlg::OnShowpassword() 
{
   UpdateData(TRUE);

   CMyString wndName;
   GetDlgItem(IDC_SHOWPASSWORD)->GetWindowText(wndName);

   if (wndName == SHOW_PASSWORD_TXT)
   {
      ShowPassword();
   }
   else if (wndName == HIDE_PASSWORD_TXT)
   {
      m_realpassword = m_password;
      HidePassword();
   }
   else
      AfxMessageBox(_T("Error in retrieving window text"));

   UpdateData(FALSE);
}


void
CEditDlg::OnOK() 
{
   UpdateData(TRUE);

   /*
    *  If the password is shown it may have been edited,
    *  so save the current text.
    */

   if (! m_isPwHidden)
      m_realpassword = m_password;

   //Check that data is valid
   if (m_title.IsEmpty())
   {
      AfxMessageBox(_T("This entry must have a title."));
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
      return;
   }
   if (m_password.IsEmpty())
   {
      AfxMessageBox(_T("This entry must have a password."));
      ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
      return;
   }
   //End check

   DboxMain* pParent = (DboxMain*) GetParent();
   ASSERT(pParent != NULL);

   POSITION listindex = pParent->Find(m_title, m_username);
   /*
    *  If there is a matching entry in our list, and that
    *  entry is not the same one we started editing, tell the
    *  user to try again.
    */
   if ((listindex != NULL) &&
       (m_listindex != listindex))
   {
      CMyString temp =
         "An item with Title \""
         + m_title + "\" and User Name \"" + m_username
         + "\" already exists.";
      AfxMessageBox(temp);
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
   }
   else
   {
      app.m_pMainWnd = NULL;
      CDialog::OnOK();
   }
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

   if (app.GetProfileInt(_T(PWS_REG_OPTIONS),
			 _T("showpwdefault"), FALSE) == TRUE)
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
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(HIDE_PASSWORD_TXT);
   GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
}


void CEditDlg::HidePassword(void)
{
   m_password = HIDDEN_PASSWORD;
   m_isPwHidden = true;
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(SHOW_PASSWORD_TXT);
   GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
}


void CEditDlg::OnRandom() 
{
   DboxMain* pParent = (DboxMain*) GetParent();
   ASSERT(pParent != NULL);
   CMyString temp = pParent->GetPassword();

   UpdateData(TRUE);
   CMyString msg;
   int nResponse;
 
   //Ask if something's there
   if (!m_password.IsEmpty())
   {
      msg =
         "The randomly generated password is: \""
         + temp
         + "\" \n(without the quotes). Would you like to use it?";
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

      if (wndName == SHOW_PASSWORD_TXT)
      {
	m_password = HIDDEN_PASSWORD;
      }
      else if (wndName == HIDE_PASSWORD_TXT)
      {
         m_password = m_realpassword;
      }
      UpdateData(FALSE);
   }
   else if (nResponse == IDNO)
   {
   }
}


void CEditDlg::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#editview"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
	::HtmlHelp(
		NULL,
		"pwsafe.chm::/html/pws_edit.htm",
		HH_DISPLAY_TOPIC, 0);

#endif
}


#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CEditDlg::OnPasskeyKillfocus()
{
	EnableWordCompletion( m_hWnd );
}


/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CEditDlg::OnPasskeySetfocus()
{
	DisableWordCompletion( m_hWnd );
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
