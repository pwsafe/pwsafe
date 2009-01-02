/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasskeySetup.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "corelib/PWCharPool.h" // for CheckPassword()
#include "ThisMfcApp.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#include "pocketpc/PocketPC.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "corelib/util.h"

#include "PasskeySetup.h"
#include "PwFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

//-----------------------------------------------------------------------------
CPasskeySetup::CPasskeySetup(CWnd* pParent)
  : CPWDialog(CPasskeySetup::IDD, pParent)
{
  m_passkey = _T("");
  m_verify = _T("");
}

BOOL CPasskeySetup::OnInitDialog() 
{
  CPWDialog::OnInitDialog();
  ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));
  ApplyPasswordFont(GetDlgItem(IDC_VERIFY));
  ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetPasswordChar(PSSWDCHAR);
  ((CEdit*)GetDlgItem(IDC_VERIFY))->SetPasswordChar(PSSWDCHAR);

  return TRUE;
}

void CPasskeySetup::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  // Can't use DDX_Text for CSecEditExtn
  m_ctlPasskey.DoDDX(pDX, m_passkey);
  m_ctlVerify.DoDDX(pDX, m_verify);
  DDX_Control(pDX, IDC_PASSKEY, m_ctlPasskey);
  DDX_Control(pDX, IDC_VERIFY, m_ctlVerify);
}

BEGIN_MESSAGE_MAP(CPasskeySetup, CPWDialog)
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
  CPWDialog::OnCancel();
}

void CPasskeySetup::OnOK()
{
  UpdateData(TRUE);
  if (m_passkey != m_verify) {
    AfxMessageBox(IDS_ENTRIESDONOTMATCH);
    ((CEdit*)GetDlgItem(IDC_VERIFY))->SetFocus();
    return;
  }

  if (m_passkey.IsEmpty()) {
    AfxMessageBox(IDS_ENTERKEYANDVERIFY);
    ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetFocus();
    return;
  }
  // Vox populi vox dei - folks want the ability to use a weak
  // passphrase, best we can do is warn them...
  // If someone want to build a version that insists on proper
  // passphrases, then just define the preprocessor macro
  // PWS_FORCE_STRONG_PASSPHRASE in the build properties/Makefile
  // (also used in CPasskeyChangeDlg)
#ifndef _DEBUG // for debug, we want no checks at all, to save time
  StringX errmess;
  if (!CPasswordCharPool::CheckPassword(m_passkey, errmess)) {
    CString cs_msg, cs_text;
    cs_msg.Format(IDS_WEAKPASSPHRASE, errmess.c_str());
#ifndef PWS_FORCE_STRONG_PASSPHRASE
    cs_text.LoadString(IDS_USEITANYWAY);
    cs_msg += cs_text;
    int rc = AfxMessageBox(cs_msg, MB_YESNO | MB_ICONSTOP);
    if (rc == IDNO)
      return;
#else
    cs_text.LoadString(IDS_TRYANOTHER);
    cs_msg += cs_text;
    AfxMessageBox(cs_msg, MB_OK | MB_ICONSTOP);
    return;
#endif // PWS_FORCE_STRONG_PASSPHRASE
  }
#endif // _DEBUG

  CPWDialog::OnOK();
}

void CPasskeySetup::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#newdatabase"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/create_new_db.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CPasskeySetup::OnPasskeyKillfocus()
{
  EnableWordCompletion(m_hWnd);
}

/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CPasskeySetup::OnPasskeySetfocus()
{
  DisableWordCompletion(m_hWnd);
}
#endif

//-----------------------------------------------------------------------------
