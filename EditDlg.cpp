/// \file EditDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "EditDlg.h"
#include "PwFont.h"
#include "OptionsPasswordPolicy.h"
#include "corelib/PWCharPool.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#include ".\editdlg.h"
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
	DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
	DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
	DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
	DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
	DDX_Text(pDX, IDC_URL, (CString&)m_URL);
	DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);

	if(!pDX->m_bSaveAndValidate) {
		// We are initializing the dialog.  Populate the groups combo box.
		CComboBox comboGroup;
		comboGroup.Attach(GetDlgItem(IDC_GROUP)->GetSafeHwnd());
		// For some reason, MFC calls us twice when initializing.
		// Populate the combo box only once.
		if(0 == comboGroup.GetCount()) {
			CStringArray aryGroups;
			app.m_core.GetUniqueGroups(aryGroups);
			for(int igrp=0; igrp<aryGroups.GetSize(); igrp++) {
				comboGroup.AddString((LPCTSTR)aryGroups[igrp]);
			}
		}
		comboGroup.Detach();
	}
	DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
	DDX_Control(pDX, IDC_MORE, m_moreLessBtn);
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
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
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
   if (!m_group.IsEmpty() && m_group[0] == '.')
   {
      AfxMessageBox(_T("A dot is invalid as the first character of the Group field."));
      ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
      return;
   }
   //End check

   DboxMain* pParent = (DboxMain*) GetParent();
   ASSERT(pParent != NULL);

   POSITION listindex = pParent->Find(m_group, m_title, m_username);
   /*
    *  If there is a matching entry in our list, and that
    *  entry is not the same one we started editing, tell the
    *  user to try again.
    */
   if ((listindex != NULL) &&
       (m_listindex != listindex))
   {
      CMyString temp =
         _T("An item with Group \"") + m_group
	 + _T("\", Title \"") + m_title 
	 + _T("\" and User Name \"") + m_username
         + _T("\" already exists.");
      AfxMessageBox(temp);
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
   }
   else
   {
      CDialog::OnOK();
   }
}


void CEditDlg::OnCancel() 
{
   CDialog::OnCancel();
}


BOOL CEditDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
 
   SetPasswordFont(GetDlgItem(IDC_PASSWORD));

   if (PWSprefs::GetInstance()->GetPref(PWSprefs::BoolPrefs::ShowPWDefault))
   {
      ShowPassword();
   }
   else
   {
      HidePassword();
   }
   UpdateData(FALSE);
   if (m_IsReadOnly) {
     GetDlgItem(IDOK)->EnableWindow(FALSE);
   }
    m_isExpanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::DisplayExpandedAddEditDlg);
	ResizeDialog();
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
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);
  UpdateData(TRUE);
  if (pParent->MakeRandomPassword(this, m_realpassword))
    {
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
}


void CEditDlg::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#editview"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
	::HtmlHelp(
		NULL,
		"pwsafe.chm::/html/entering_pwd.html",
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

void CEditDlg::OnBnClickedOk()
{
	OnOK();
}

void CEditDlg::OnBnClickedMore()
{
	m_isExpanded = !m_isExpanded;
	ResizeDialog();
}

void CEditDlg::ResizeDialog()
{
	int TopHideableControl = IDC_URL;
	int BottomHideableControl = IDC_AUTOTYPE;
	int controls[]={
IDC_URL,
IDC_AUTOTYPE,
IDC_STATIC_URL,
IDC_STATIC_AUTO};

	
	for(int n = 0; n<sizeof(controls)/sizeof(IDC_URL);n++)
	{
		CWnd* pWind;
		pWind = (CWnd *)GetDlgItem(controls[n]);
		pWind->ShowWindow(m_isExpanded);
	}
	
	RECT curDialogRect;
	
	this->GetWindowRect(&curDialogRect);

	RECT newDialogRect=curDialogRect;


	RECT curLowestCtlRect;
	CWnd* pLowestCtl;
	int newHeight;
  if (m_isExpanded) {
    // from less to more
	  pLowestCtl = (CWnd *)GetDlgItem(BottomHideableControl);
	  
	  pLowestCtl->GetWindowRect(&curLowestCtlRect);

	  newHeight =  curLowestCtlRect.bottom + 15  - newDialogRect.top;
    m_moreLessBtn.SetWindowText(_T("<< Less"));
  } else {
    
	  // from more to less
	  pLowestCtl = (CWnd *)GetDlgItem(TopHideableControl);
	  pLowestCtl->GetWindowRect(&curLowestCtlRect);

	  newHeight =  curLowestCtlRect.top + 5  - newDialogRect.top;

    m_moreLessBtn.SetWindowText(_T("More >>"));
  }
  

  this->SetWindowPos(NULL,0,0,
		newDialogRect.right - newDialogRect.left ,
		newHeight , 
		SWP_NOMOVE );

}