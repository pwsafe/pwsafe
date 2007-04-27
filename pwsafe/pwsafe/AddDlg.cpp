/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file AddDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "AddDlg.h"
#include "PwFont.h"
#include "corelib/PWCharPool.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

CString CAddDlg::CS_SHOW;
CString CAddDlg::CS_HIDE;

//-----------------------------------------------------------------------------
CAddDlg::CAddDlg(CWnd* pParent)
  : CDialog(CAddDlg::IDD, pParent), m_password(_T("")), m_notes(_T("")),
    m_username(_T("")), m_title(_T(""))
{

  if (CS_SHOW.IsEmpty()) {
#if defined(POCKET_PC)
	CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT1);
	CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT1);
#else
	CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT2);
	CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT2);
#endif
  }
}


BOOL CAddDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
 
   SetPasswordFont(GetDlgItem(IDC_PASSWORD));

   return TRUE;
}


void CAddDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
   DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
   DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
   DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
}


BEGIN_MESSAGE_MAP(CAddDlg, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_RANDOM, OnRandom)
#if defined(POCKET_PC)
   ON_WM_SHOWWINDOW()
   ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetfocus)
   ON_EN_KILLFOCUS(IDC_PASSWORD, OnPasskeyKillfocus)
#endif
END_MESSAGE_MAP()


void
CAddDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CAddDlg::OnOK() 
{
  if(UpdateData(TRUE) != TRUE)
	  return;

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }
  if (m_password.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVEPASSWORD);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  }
/// \todo add m_password2
/*  if (m_isPwHidden && (m_password.Compare(m_password2) != 0)) {
    AfxMessageBox(IDS_PASSWORDSNOTMATCH);
    UpdateData(FALSE);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  } */
  //End check

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);

  /// \todo add m_group 
  //if (pParent->Find(m_group, m_title, m_username) != NULL) {
  if (pParent->Find(m_title, m_username) != NULL)
  {
    CMyString temp;
    //temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
	temp.Format(IDS_ENTRYEXISTS, _T(""), m_title, m_username);
    AfxMessageBox(temp);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
  } else {
	app.m_pMainWnd = NULL;
    CDialog::OnOK();
  }
}


void CAddDlg::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
   //WinHelp(0x2008E, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_add_data.htm",
              HH_DISPLAY_TOPIC, 0);
#endif
}


void CAddDlg::OnRandom() 
{
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);

  CMyString temp = pParent->GetPassword();

  UpdateData(TRUE);

  int nResponse;
  if (m_password == "")
	nResponse = IDYES;
  else
  {
	CMyString msg;
	msg = _T("The randomly generated password is: \"")
	   + temp
	   + _T("\" \n(without the quotes). Would you like to use it?");
	nResponse = MessageBox(msg, AfxGetAppName(),
						   MB_ICONEXCLAMATION|MB_YESNO);
  }

  if (nResponse == IDYES)
  {
	m_password = temp;
	UpdateData(FALSE);
  }
}


#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CAddDlg::OnPasskeyKillfocus()
{
	EnableWordCompletion( m_hWnd );
}


/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CAddDlg::OnPasskeySetfocus()
{
	DisableWordCompletion( m_hWnd );
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
