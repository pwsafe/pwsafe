/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasskeyChangeDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWScore.h" // for error statuses from CheckPassword()
#include "corelib/PWCharPool.h" // for CheckPassword()
#include "ThisMfcApp.h"
#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#include "pocketpc/PocketPC.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "PasskeyChangeDlg.h"
#include "PwFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

//-----------------------------------------------------------------------------
CPasskeyChangeDlg::CPasskeyChangeDlg(CWnd* pParent)
  : CPWDialog(CPasskeyChangeDlg::IDD, pParent)
{
  m_confirmnew = _T("");
  m_newpasskey = _T("");
  m_oldpasskey = _T("");
}

void CPasskeyChangeDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_CONFIRMNEW, (CString &)m_confirmnew);
  DDX_Text(pDX, IDC_NEWPASSKEY, (CString &)m_newpasskey);
  DDX_Text(pDX, IDC_OLDPASSKEY, (CString &)m_oldpasskey);
}

BEGIN_MESSAGE_MAP(CPasskeyChangeDlg, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
#if defined(POCKET_PC)
  ON_EN_SETFOCUS(IDC_OLDPASSKEY, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_NEWPASSKEY, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_CONFIRMNEW, OnPasskeySetfocus)
  ON_EN_KILLFOCUS(IDC_OLDPASSKEY, OnPasskeyKillfocus)
  ON_EN_KILLFOCUS(IDC_NEWPASSKEY, OnPasskeyKillfocus)
  ON_EN_KILLFOCUS(IDC_CONFIRMNEW, OnPasskeyKillfocus)
#endif
END_MESSAGE_MAP()

BOOL CPasskeyChangeDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  ApplyPasswordFont(GetDlgItem(IDC_CONFIRMNEW));
  ApplyPasswordFont(GetDlgItem(IDC_NEWPASSKEY));
  ApplyPasswordFont(GetDlgItem(IDC_OLDPASSKEY));
  ((CEdit*)GetDlgItem(IDC_CONFIRMNEW))->SetPasswordChar(PSSWDCHAR);
  ((CEdit*)GetDlgItem(IDC_NEWPASSKEY))->SetPasswordChar(PSSWDCHAR);
  ((CEdit*)GetDlgItem(IDC_OLDPASSKEY))->SetPasswordChar(PSSWDCHAR);

  return TRUE;
}

void CPasskeyChangeDlg::OnOK() 
{
  CMyString errmess;
  CString cs_msg, cs_text;

  UpdateData(TRUE);
  int rc = app.m_core.CheckPassword(app.m_core.GetCurFile(), m_oldpasskey);
  if (rc == PWScore::WRONG_PASSWORD)
    AfxMessageBox(IDS_WRONGOLDPHRASE);
  else if (rc == PWScore::CANT_OPEN_FILE)
    AfxMessageBox(IDS_CANTVERIFY);
  else if (m_confirmnew != m_newpasskey)
    AfxMessageBox(IDS_NEWOLDDONOTMATCH);
  else if (m_newpasskey.IsEmpty())
    AfxMessageBox(IDS_CANNOTBEBLANK);
  // Vox populi vox dei - folks want the ability to use a weak
  // passphrase, best we can do is warn them...
  // If someone want to build a version that insists on proper
  // passphrases, then just define the preprocessor macro
  // PWS_FORCE_STRONG_PASSPHRASE in the build properties/Makefile
  // (also used in CPasskeySetup)
  else if (!CPasswordCharPool::CheckPassword(m_newpasskey, errmess)) {
    cs_msg.Format(IDS_WEAKPASSPHRASE, errmess);
#ifndef PWS_FORCE_STRONG_PASSPHRASE
    cs_text.LoadString(IDS_USEITANYWAY);
    cs_msg += cs_text;
    int rc = AfxMessageBox(cs_msg, MB_YESNO | MB_ICONSTOP);
    if (rc == IDYES)
      CPWDialog::OnOK();
#else
    cs_text.LoadString(IDS_TRYANOTHER);
    cs_msg += cs_text;
    AfxMessageBox(cs_msg, MB_OK | MB_ICONSTOP);
#endif // PWS_FORCE_STRONG_PASSPHRASE
  } else {
    CPWDialog::OnOK();
  }
}

void CPasskeyChangeDlg::OnCancel() 
{
  CPWDialog::OnCancel();
}

void CPasskeyChangeDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#changecombo"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/change_combo.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CPasskeyChangeDlg::OnPasskeyKillfocus()
{
  EnableWordCompletion( m_hWnd );
}

/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CPasskeyChangeDlg::OnPasskeySetfocus()
{
  DisableWordCompletion( m_hWnd );
}
#endif
