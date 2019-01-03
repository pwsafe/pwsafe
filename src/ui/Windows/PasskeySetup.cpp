/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasskeySetup.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "Options_PropertySheet.h"
#include "PasskeySetup.h"
#include "Fonts.h"
#include "YubiCfgDlg.h"

#include "core/PWCharPool.h" // for CheckPassword()
#include "core/PwsPlatform.h"
#include "core/pwsprefs.h"
#include "core/PWScore.h"
#include "core/util.h"

#include "os/dir.h"
#include "os/rand.h"

#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "resource.h"
#include "resource3.h"  // String resources

#include <iomanip>  // For setbase and setw

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CPasskeySetup::CPasskeySetup(CWnd *pParent, PWScore &core)
  : CPKBaseDlg(CPasskeySetup::IDD, pParent),
    m_LastFocus(IDC_PASSKEY), m_core(core), m_btnShowCombination(FALSE)
{
  m_verify = L"";
  m_pctlVerify = new CSecEditExtn;
}

CPasskeySetup::~CPasskeySetup()
{
  delete m_pctlVerify;
}

void CPasskeySetup::DoDataExchange(CDataExchange* pDX)
{
  CPKBaseDlg::DoDataExchange(pDX);
  
  // Can't use DDX_Text for CSecEditExtn
  m_pctlVerify->DoDDX(pDX, m_verify);

  DDX_Control(pDX, IDC_VERIFY, *m_pctlVerify);

  DDX_Check(pDX, IDC_SHOWCOMBINATION, m_btnShowCombination);
}

BEGIN_MESSAGE_MAP(CPasskeySetup, CPKBaseDlg)
  ON_WM_TIMER()

  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_YUBIKEY_BTN, OnYubikeyBtn)
  ON_BN_CLICKED(IDC_SHOWCOMBINATION, OnShowCombination)

  ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_VERIFY, OnVerifykeySetfocus)

  ON_MESSAGE(PWS_MSG_INSERTBUFFER, OnInsertBuffer)
END_MESSAGE_MAP()

BOOL CPasskeySetup::OnInitDialog() 
{
  CPKBaseDlg::OnInitDialog();

  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_VERIFY));

  m_pctlVerify->SetPasswordChar(PSSWDCHAR);

  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CPasskeySetup::OnCancel() 
{
  CPKBaseDlg::OnCancel();
}

void CPasskeySetup::OnOK()
{
  UpdateData(TRUE);

  CGeneralMsgBox gmb;
  if (m_btnShowCombination == FALSE && m_passkey != m_verify) {
    gmb.AfxMessageBox(IDS_ENTRIESDONOTMATCH);
    ((CEdit*)GetDlgItem(IDC_VERIFY))->SetFocus();
    return;
  }

  if (m_passkey.IsEmpty()) {
    gmb.AfxMessageBox(IDS_ENTERKEYANDVERIFY);
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
    cs_msg.Format(IDS_WEAKPASSPHRASE, static_cast<LPCWSTR>(errmess.c_str()));
#ifndef PWS_FORCE_STRONG_PASSPHRASE
    cs_text.LoadString(IDS_USEITANYWAY);
    cs_msg += cs_text;
    INT_PTR rc = gmb.AfxMessageBox(cs_msg, NULL, MB_YESNO | MB_ICONSTOP);
    if (rc == IDNO)
      return;
#else
    cs_text.LoadString(IDS_TRYANOTHER);
    cs_msg += cs_text;
    gmb.AfxMessageBox(cs_msg, NULL, MB_OK | MB_ICONSTOP);
    return;
#endif // PWS_FORCE_STRONG_PASSPHRASE
  }
#endif // _DEBUG

  CPKBaseDlg::OnOK();
}

void CPasskeySetup::OnHelp() 
{
  ShowHelp(L"::/html/create_new_db.html");
}

void CPasskeySetup::OnPasskeySetfocus()
{
  m_LastFocus = IDC_PASSKEY;
}

void CPasskeySetup::OnVerifykeySetfocus()
{
  m_LastFocus = IDC_VERIFY;
}

void CPasskeySetup::OnVirtualKeyboard()
{
  // Shouldn't be here if couldn't load DLL. Static control disabled/hidden
  if (!CVKeyBoardDlg::IsOSKAvailable())
    return;

  if (m_pVKeyBoardDlg != NULL && m_pVKeyBoardDlg->IsWindowVisible()) {
    // Already there - move to top
    m_pVKeyBoardDlg->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    return;
  }

  // If not already created - do it, otherwise just reset it
  if (m_pVKeyBoardDlg == NULL) {
    StringX cs_LUKBD = PWSprefs::GetInstance()->GetPref(PWSprefs::LastUsedKeyboard);
    m_pVKeyBoardDlg = new CVKeyBoardDlg(this, cs_LUKBD.c_str());
    m_pVKeyBoardDlg->Create(CVKeyBoardDlg::IDD);
  } else {
    m_pVKeyBoardDlg->ResetKeyboard();
  }

  // Now show it and make it top
  m_pVKeyBoardDlg->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return;
}

LRESULT CPasskeySetup::OnInsertBuffer(WPARAM, LPARAM)
{
  // Update the variables
  UpdateData(TRUE);

  // Get the buffer
  CSecString vkbuffer = m_pVKeyBoardDlg->GetPassphrase();

  CSecEditExtn *m_pSecCtl(NULL);
  CSecString *m_pSecString;

  switch (m_LastFocus) {
    case IDC_PASSKEY:
      m_pSecCtl = m_pctlPasskey;
      m_pSecString = &m_passkey;
      break;
    case IDC_VERIFY:
      m_pSecCtl = m_pctlVerify;
      m_pSecString = &m_verify;
      break;
    default:
      // Error!
      ASSERT(0);
      return 0L;
  }

  // Find the selected characters - if any
  int nStartChar, nEndChar;
  m_pSecCtl->GetSel(nStartChar, nEndChar);

  // If any characters selected - delete them
  if (nStartChar != nEndChar)
    m_pSecString->Delete(nStartChar, nEndChar - nStartChar);

  // Insert the buffer
  m_pSecString->Insert(nStartChar, vkbuffer);

  // Update the dialog
  UpdateData(FALSE);

  // Put cursor at end of inserted text
  m_pSecCtl->SetSel(nStartChar + vkbuffer.GetLength(),
                    nStartChar + vkbuffer.GetLength());

  // Make us on top
  SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return 0L;
}

void CPasskeySetup::OnShowCombination()
{
  UpdateData(TRUE);

  m_pctlPasskey->SetSecure(m_btnShowCombination == TRUE ? FALSE : TRUE);

  if (m_btnShowCombination == TRUE) {
    m_pctlPasskey->SetPasswordChar(0);
    m_pctlPasskey->SetWindowText(m_passkey);

    m_pctlVerify->SetPasswordChar(0);
    m_pctlVerify->EnableWindow(FALSE);
    m_pctlVerify->SetWindowText(L"");
  } else {
    m_pctlPasskey->SetPasswordChar(PSSWDCHAR);
    m_pctlPasskey->SetSecureText(m_passkey);

    m_pctlVerify->SetPasswordChar(PSSWDCHAR);
    m_pctlVerify->EnableWindow(TRUE);
    m_pctlVerify->SetWindowText(L"");
  }
}

void CPasskeySetup::OnYubikeyBtn()
{
  UpdateData(TRUE);
  // Check that password and verification are same.
  // unlike non-Yubi usage, here we accept empty passwords,
  // which will give token-based authentication.
  // A non-empty password with Yubikey is 2-factor auth.
  if (m_passkey != m_verify) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_ENTRIESDONOTMATCH);
    ((CEdit*)GetDlgItem(IDC_VERIFY))->SetFocus();
    return;
  }
  GetDlgItem(IDOK)->EnableWindow(FALSE); // BR1465 - don't allow closing w/o yk press
  yubiRequestHMACSha1(m_passkey);
}

void CPasskeySetup::ProcessPhrase()
{
  // OnOK clears the passkey, so we save it
  const CSecString save_passkey = m_passkey;
  TRACE(L"CPasskeySetup::ProcessPhrase(%s)\n", static_cast<LPCWSTR>(m_passkey));
  CPKBaseDlg::OnOK();
  m_passkey = save_passkey;
}

void CPasskeySetup::YubiFailed()
{
    CGeneralMsgBox gmb;
    INT_PTR rc = gmb.AfxMessageBox(IDS_YUBI_UNINITIALIZED,
                                   MB_YESNO | MB_ICONQUESTION);
    if (rc == IDYES) {
      YubiInitialize();
    }
}

void CPasskeySetup::YubiInitialize()
{
#ifndef NO_YUBI
  CGeneralMsgBox gmb;
  CYubiCfgDlg ycd(this, m_core);
  unsigned char sk[CYubiCfgDlg::YUBI_SK_LEN];
  pws_os::GetRandomData(sk, CYubiCfgDlg::YUBI_SK_LEN);
  if (ycd.WriteYubiSK(sk) == YKLIB_OK) {
      m_core.SetYubiSK(sk);
      gmb.AfxMessageBox(IDS_YUBI_INIT_SUCCESS,
                        MB_OK | MB_ICONINFORMATION);
      PostMessage(WM_COMMAND, IDC_YUBIKEY_BTN);
  } else {
    gmb.AfxMessageBox(IDS_YUBI_INIT_FAILED,
                      MB_OK | MB_ICONERROR);
  }
#endif /* NO_YUBI */
}
