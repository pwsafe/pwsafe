/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasskeyChangeDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "PasskeyChangeDlg.h"
#include "Fonts.h"

#include "core/PwsPlatform.h"
#include "core/PWScore.h"    // for error statuses from CheckPasskey()
#include "core/PWCharPool.h" // for CheckPassword()
#include "core/pwsprefs.h"

#include "os/dir.h"

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
CPasskeyChangeDlg::CPasskeyChangeDlg(CWnd* pParent)
  : CPKBaseDlg(CPasskeyChangeDlg::IDD, pParent),
    m_LastFocus(IDC_PASSKEY), m_Yubi1pressed(false), m_Yubi2pressed(false),
    m_oldpasskeyConfirmed(false), m_btnShowCombination(FALSE)
{
  m_newpasskey = L"";
  m_confirmnew = L"";

  m_pctlNewPasskey = new CSecEditExtn;
  m_pctlConfirmNew = new CSecEditExtn;
}

CPasskeyChangeDlg::~CPasskeyChangeDlg()
{
  delete m_pctlNewPasskey;
  delete m_pctlConfirmNew;
}

void CPasskeyChangeDlg::DoDataExchange(CDataExchange* pDX)
{
  CPKBaseDlg::DoDataExchange(pDX);

  // Can't use DDX_Text for CSecEditExtn
  m_pctlNewPasskey->DoDDX(pDX, m_newpasskey);
  m_pctlConfirmNew->DoDDX(pDX, m_confirmnew);

  DDX_Control(pDX, IDC_CONFIRMNEW, *m_pctlConfirmNew);
  DDX_Control(pDX, IDC_NEWPASSKEY, *m_pctlNewPasskey);
  DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);

  DDX_Check(pDX, IDC_SHOWCOMBINATION, m_btnShowCombination);
}

BEGIN_MESSAGE_MAP(CPasskeyChangeDlg, CPKBaseDlg)
  ON_WM_TIMER()

  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)

  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_YUBIKEY2_BTN, OnYubikey2Btn)
  ON_BN_CLICKED(IDC_YUBIKEY_BTN, OnYubikeyBtn)
  ON_BN_CLICKED(IDC_SHOWCOMBINATION, OnShowCombination)

  ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_NEWPASSKEY, OnNewPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_CONFIRMNEW, OnConfirmNewSetfocus)

  ON_MESSAGE(PWS_MSG_INSERTBUFFER, OnInsertBuffer)
END_MESSAGE_MAP()

BOOL CPasskeyChangeDlg::OnInitDialog()
{
  CPKBaseDlg::OnInitDialog();

  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_NEWPASSKEY));
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_CONFIRMNEW));

  m_pctlNewPasskey->SetPasswordChar(PSSWDCHAR);
  m_pctlConfirmNew->SetPasswordChar(PSSWDCHAR);

  // Base class handles 1 Yubi btn, here we have 2, so we have to manage the
  // 2nd on our lonely.
  CButton *ybn2 = (CButton *)GetDlgItem(IDC_YUBIKEY2_BTN);
  // Hide 2nd Yubi btn if Yubikey never detected
  ybn2->ShowWindow(YubiExists() ? SW_SHOW : SW_HIDE);
  // "Enable" 2nd Yubi btn iff Yubi's connected
  ybn2->SetBitmap(IsYubiInserted() ? m_yubiLogo : m_yubiLogoDisabled);
 
  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CPasskeyChangeDlg::yubiInserted(void)
{
  CPKBaseDlg::yubiInserted();
  CButton *ybn2 = (CButton *)GetDlgItem(IDC_YUBIKEY2_BTN);
  ybn2->ShowWindow(SW_SHOW); // needed for 1st time
  ybn2->SetBitmap(m_yubiLogo);
}

void CPasskeyChangeDlg::yubiRemoved(void)
{
  CPKBaseDlg::yubiRemoved();
  ((CButton *)GetDlgItem(IDC_YUBIKEY2_BTN))->SetBitmap(m_yubiLogoDisabled);
}

void CPasskeyChangeDlg::OnOK() 
{
  StringX errmess;
  CString cs_msg, cs_text;

  UpdateData(TRUE);
  if (!m_oldpasskey.IsEmpty()) {
    m_passkey = m_oldpasskey; // old passkey is from Yubikey
  }

  CGeneralMsgBox gmb;
  int rc = app.GetCore()->CheckPasskey(app.GetCore()->GetCurFile(), m_passkey);
  if (rc == PWScore::WRONG_PASSWORD)
    gmb.AfxMessageBox(IDS_WRONGOLDPHRASE);
  else if (rc == PWScore::CANT_OPEN_FILE)
    gmb.AfxMessageBox(IDS_CANTVERIFY);
  else if (m_btnShowCombination == FALSE && m_confirmnew != m_newpasskey)
    gmb.AfxMessageBox(IDS_NEWOLDDONOTMATCH);
  else if (m_newpasskey.IsEmpty())
    gmb.AfxMessageBox(IDS_CANNOTBEBLANK);

  // Vox populi vox dei - folks want the ability to use a weak
  // passphrase, best we can do is warn them...
  // If someone want to build a version that insists on proper
  // passphrases, then just define the preprocessor macro
  // PWS_FORCE_STRONG_PASSPHRASE in the build properties/Makefile
  // (also used in CPasskeySetup)
  else if (!CPasswordCharPool::CheckPassword(m_newpasskey, errmess)) {
    cs_msg.Format(IDS_WEAKPASSPHRASE, static_cast<LPCWSTR>(errmess.c_str()));

#ifndef PWS_FORCE_STRONG_PASSPHRASE
    cs_text.LoadString(IDS_USEITANYWAY);
    cs_msg += cs_text;
    rc = (int)gmb.AfxMessageBox(cs_msg, NULL, MB_YESNO | MB_ICONSTOP);
    if (rc == IDYES)
      CPKBaseDlg::OnOK();
#else
    cs_text.LoadString(IDS_TRYANOTHER);
    cs_msg += cs_text;
    gmb.AfxMessageBox(cs_msg, NULL, MB_OK | MB_ICONSTOP);
#endif // PWS_FORCE_STRONG_PASSPHRASE
  } else {
    CPKBaseDlg::OnOK();
  }
}

void CPasskeyChangeDlg::OnCancel() 
{
  CPKBaseDlg::OnCancel();
}

void CPasskeyChangeDlg::OnHelp() 
{
  ShowHelp(L"::/html/change_combo.html");
}

void CPasskeyChangeDlg::OnPasskeySetfocus()
{
  m_LastFocus = IDC_PASSKEY;
}

void CPasskeyChangeDlg::OnNewPasskeySetfocus()
{
  m_LastFocus = IDC_NEWPASSKEY;
}

void CPasskeyChangeDlg::OnConfirmNewSetfocus()
{
  m_LastFocus = IDC_CONFIRMNEW;
}

void CPasskeyChangeDlg::OnShowCombination()
{
  UpdateData(TRUE);

  m_pctlPasskey->SetSecure(m_btnShowCombination == TRUE ? FALSE : TRUE);
  m_pctlNewPasskey->SetSecure(m_btnShowCombination == TRUE ? FALSE : TRUE);

  if (m_btnShowCombination == TRUE) {
    m_pctlPasskey->SetPasswordChar(0);
    m_pctlPasskey->SetWindowText(m_passkey);

    m_pctlNewPasskey->SetPasswordChar(0);
    m_pctlNewPasskey->SetWindowText(m_newpasskey);

    m_pctlConfirmNew->SetPasswordChar(0);
    m_pctlConfirmNew->EnableWindow(FALSE);
    m_pctlConfirmNew->SetWindowText(L"");
  } else {
    m_pctlPasskey->SetPasswordChar(PSSWDCHAR);
    m_pctlPasskey->SetSecureText(m_passkey);

    m_pctlNewPasskey->SetPasswordChar(PSSWDCHAR);
    m_pctlNewPasskey->SetSecureText(m_newpasskey);

    m_pctlConfirmNew->SetPasswordChar(PSSWDCHAR);
    m_pctlConfirmNew->EnableWindow(TRUE);
    m_pctlConfirmNew->SetWindowText(L"");
  }
}

void CPasskeyChangeDlg::OnVirtualKeyboard()
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

LRESULT CPasskeyChangeDlg::OnInsertBuffer(WPARAM, LPARAM)
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
    case IDC_NEWPASSKEY:
      m_pSecCtl = m_pctlNewPasskey;
      m_pSecString = &m_newpasskey;
      break;
    case IDC_CONFIRMNEW:
      m_pSecCtl = m_pctlConfirmNew;
      m_pSecString = &m_confirmnew;
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

void CPasskeyChangeDlg::OnYubikeyBtn()
{
  // This is for existing password verification
  UpdateData(TRUE);

  m_Yubi1pressed = true;
  yubiRequestHMACSha1(m_passkey);
}

void CPasskeyChangeDlg::OnYubikey2Btn()
{
  UpdateData(TRUE);

  if (m_btnShowCombination == FALSE && m_confirmnew != m_newpasskey) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_NEWOLDDONOTMATCH);
  } else {
    m_Yubi2pressed = true;
    m_oldpasskey = m_passkey; // might need for confirmation
    m_passkey = m_newpasskey;
    yubiRequestHMACSha1(m_newpasskey);
    GetDlgItem(IDOK)->EnableWindow(FALSE); // BR1465 - don't allow closing w/o yk press
  }
}

void CPasskeyChangeDlg::ProcessPhrase()
{
  if (m_Yubi1pressed) { // verify existing password
    m_Yubi1pressed = false;
    CGeneralMsgBox gmb;
    int rc = app.GetCore()->CheckPasskey(app.GetCore()->GetCurFile(),
                                         m_passkey);
    if (rc == PWScore::WRONG_PASSWORD)
      gmb.AfxMessageBox(IDS_WRONGOLDPHRASE);
    else if (rc == PWScore::CANT_OPEN_FILE)
      gmb.AfxMessageBox(IDS_CANTVERIFY);
    else {
      m_oldpasskey = m_passkey;
      m_oldpasskeyConfirmed = true;
    }
  } else if (m_Yubi2pressed) { // set new yubi-passwd
    m_Yubi2pressed = false;
    if (!m_oldpasskeyConfirmed) {
      // perhaps old passkey's w/o yubikey - check it that way
      int rc = app.GetCore()->CheckPasskey(app.GetCore()->GetCurFile(),
                                           m_oldpasskey);
      m_oldpasskeyConfirmed = rc == PWScore::SUCCESS;
    }
    if (m_oldpasskeyConfirmed) {
      // OnOK clears the passkey, so we save it
      const CSecString save_passkey = m_passkey;
      CPKBaseDlg::OnOK(); // skip our OnOK(), irrelevant
      m_newpasskey = save_passkey;
    } else {
      m_yubi_status.SetWindowText(_T("Please confirm old passphrase"));
    }
  } else {
    ASSERT(0);
  }
}
