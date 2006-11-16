/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file PasskeyEntry.cpp
//-----------------------------------------------------------------------------

/*
  Passkey?  That's Russian for 'pass'.  You know, passkey
  down the streetsky.  [Groucho Marx]
*/

#include "PasswordSafe.h"
#include "corelib/PwsPlatform.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PocketPC.h"
#else
  #include "resource.h"
  #include "resource3.h"  // String resources
#endif

#include "corelib/MyString.h"

#include "SysColStatic.h"

#include "PasskeyEntry.h"
#include "PwFont.h"
#include "TryAgainDlg.h"
#include "DboxMain.h" // for CheckPassword()

#include "corelib/Util.h"

static TCHAR PSSWDCHAR = TCHAR('*');

int CPasskeyEntry::dialog_lookup[4] = {IDD_PASSKEYENTRY_FIRST, 
										IDD_PASSKEYENTRY, 
										IDD_PASSKEYENTRY, 
										IDD_PASSKEYENTRY_WITHEXIT};

//-----------------------------------------------------------------------------
CPasskeyEntry::CPasskeyEntry(CWnd* pParent,
                             const CString& a_filespec, int index,
			                 bool bReadOnly, bool bForceReadOnly)
   : super(dialog_lookup[index],
             pParent),
     m_index(index),
     m_filespec(a_filespec),
     m_tries(0),
     m_status(TAR_INVALID),
     m_ReadOnly(bReadOnly ? TRUE : FALSE),
     m_bForceReadOnly(bForceReadOnly)
{
  const int FILE_DISP_LEN = 45;

  //{{AFX_DATA_INIT(CPasskeyEntry)
  //}}AFX_DATA_INIT

  DBGMSG("CPasskeyEntry()\n");
  if (m_index == GCP_FIRST) {
    DBGMSG("** FIRST **\n");
  }
  
  m_passkey = _T("");

  m_hIcon = app.LoadIcon(IDI_CORNERICON);

  if (a_filespec.GetLength() > FILE_DISP_LEN) {
    // m_message = a_filespec.Right(FILE_DISP_LEN - 3); // truncate for display
    // m_message.Insert(0, _T("..."));
    // changed by karel@VanderGucht.de to see beginning + ending of 'a_filespec'
    m_message =  a_filespec.Left(FILE_DISP_LEN/2-5) + 
      _T(" ... ") + a_filespec.Right(FILE_DISP_LEN/2);
  } else {
    m_message = a_filespec;
  }
}

void CPasskeyEntry::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_PASSKEY, (CString &)m_passkey);

#if !defined(POCKET_PC)
   if (m_index == GCP_FIRST)
	DDX_Control(pDX, IDC_STATIC_LOGOTEXT, m_ctlLogoText);
#endif

   //{{AFX_DATA_MAP(CPasskeyEntry)
#if !defined(POCKET_PC)
	DDX_Control(pDX, IDC_STATIC_LOGO, m_ctlLogo);
	DDX_Control(pDX, IDOK, m_ctlOK);
#endif
	DDX_Control(pDX, IDC_PASSKEY, m_ctlPasskey);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
	DDX_Check(pDX, IDC_READONLY, m_ReadOnly);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasskeyEntry, super)
	//{{AFX_MSG_MAP(CPasskeyEntry)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
   ON_BN_CLICKED(IDC_CREATE_DB, OnCreateDb)
   ON_BN_CLICKED(IDC_EXIT, OnExit)
   ON_BN_CLICKED(IDC_READONLY, OnReadOnly)
#if defined(POCKET_PC)
   ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
   ON_EN_KILLFOCUS(IDC_PASSKEY, OnPasskeyKillfocus)
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL
CPasskeyEntry::OnInitDialog(void)
{
  SetPasswordFont(GetDlgItem(IDC_PASSKEY));

  ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetPasswordChar(PSSWDCHAR);
  switch(m_index) {
  	case GCP_FIRST:
  		// At start up - give the user the option unless file is R/O
  		if (m_bForceReadOnly)
  			GetDlgItem(IDC_READONLY)->EnableWindow(FALSE);
  		else
  			GetDlgItem(IDC_READONLY)->EnableWindow(TRUE);

  		GetDlgItem(IDC_READONLY)->ShowWindow(SW_SHOW);
  		GetDlgItem(IDC_VERSION)->SetWindowText(m_appversion);
  		break;
  	case GCP_NORMAL:
		// otherwise during open - user can - again unless file is R/O
  		if (m_bForceReadOnly)
  			GetDlgItem(IDC_READONLY)->EnableWindow(FALSE);
  		else
  			GetDlgItem(IDC_READONLY)->EnableWindow(TRUE);

  		GetDlgItem(IDC_READONLY)->ShowWindow(SW_SHOW);
  		break;
  	case GCP_UNMINIMIZE:
  	case GCP_WITHEXIT:
  		// on UnMinimize - user can't change status
  		GetDlgItem(IDC_READONLY)->EnableWindow(FALSE);
  		GetDlgItem(IDC_READONLY)->ShowWindow(SW_HIDE);
  		break;
  	default:
  		ASSERT(FALSE);
  }

#if defined(POCKET_PC)
  // If displaying IDD_PASSKEYENTRY_FIRST then bypass superclass and go
  // directly to CDialog::OnInitDialog() and display the dialog fullscreen
  // otherwise display as a centred dialogue.
  if ( m_nIDHelp == IDD ) {
      super::super::OnInitDialog();
  } else {
      super::OnInitDialog();
  }
#else
  super::OnInitDialog();
#endif

  if (m_message.IsEmpty() && m_index == GCP_FIRST) {
      m_ctlPasskey.EnableWindow(FALSE);
#if !defined(POCKET_PC)
      m_ctlOK.EnableWindow(FALSE);
#endif
      m_message.LoadString(IDS_NOCURRENTSAFE);
  }
  /*
   * this bit makes the background come out right on
   * the bitmaps
   */

#if !defined(POCKET_PC)
  if (m_index == GCP_FIRST) {
    m_ctlLogoText.ReloadBitmap(IDB_PSLOGO);
    m_ctlLogo.ReloadBitmap(IDB_CLOGO);
  } else {
    m_ctlLogo.ReloadBitmap(IDB_CLOGO_SMALL);
  }
#endif 

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog

  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIcon, FALSE); // Set small icon

  if (app.WasHotKeyPressed()) {
	  // Reset it
	  app.SetHotKeyPressed(false);

	  // Need to get it to the top as a result of a hotkey.
	  // This is "stronger" than BringWindowToTop().
	  SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	  SetActiveWindow();
	  SetForegroundWindow();
  }

  if (m_ctlPasskey.IsWindowEnabled() == TRUE) {
    m_ctlPasskey.SetFocus();
    return FALSE;
  } else
    return TRUE;
}

#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   *//* focus.                                                               */
/************************************************************************/
void CPasskeyEntry::OnPasskeyKillfocus()
{
  EnableWordCompletion( m_hWnd );
}

/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CPasskeyEntry::OnPasskeySetfocus()
{
  DisableWordCompletion( m_hWnd );
}
#endif

void
CPasskeyEntry::OnReadOnly() 
{
   m_ReadOnly = ((CButton*)GetDlgItem(IDC_READONLY))->GetCheck();
}

void
CPasskeyEntry::OnBrowse()
{
  m_status = TAR_OPEN;
  super::OnCancel();
}

void
CPasskeyEntry::OnCreateDb()
{
  m_status = TAR_NEW;
  super::OnCancel();
}

void
CPasskeyEntry::OnCancel() 
{
  m_status = TAR_CANCEL;
  super::OnCancel();
}

void
CPasskeyEntry::OnExit() 
{
  m_status = TAR_EXIT;
  super::OnCancel();
}

void
CPasskeyEntry::OnOK() 
{
  UpdateData(TRUE);

  if (m_passkey.IsEmpty()) {
    AfxMessageBox(IDS_CANNOTBEBLANK);
    m_ctlPasskey.SetFocus();
    return;
  }

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);
  if (pParent->CheckPassword(m_filespec, m_passkey) != PWScore::SUCCESS) {
    if (m_tries >= 2) {
	  CTryAgainDlg errorDlg(this);

      int nResponse = errorDlg.DoModal();
      if (nResponse == IDOK) {
      } else if (nResponse == IDCANCEL) {
        m_status = errorDlg.GetCancelReturnValue();
        super::OnCancel();
      }
    } else {
      m_tries++;
      AfxMessageBox(IDS_INCORRECTKEY);
      m_ctlPasskey.SetSel(MAKEWORD(-1, 0));
      m_ctlPasskey.SetFocus();
    }
  } else {
    super::OnOK();
  }
}

void
CPasskeyEntry::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#comboentry"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/create_new_db.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
