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
#include "corelib/pwsprefs.h"
#include "os/dir.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"

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

#include <iomanip>  // For setbase and setw

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static wchar_t PSSWDCHAR = L'*';

//-----------------------------------------------------------------------------
CPasskeySetup::CPasskeySetup(CWnd* pParent)
  : CPWDialog(CPasskeySetup::IDD, pParent), m_pVKeyBoardDlg(NULL),
  m_OSK_module(NULL), m_LastFocus(IDC_PASSKEY)
{
  m_passkey = L"";
  m_verify = L"";

  m_pctlPasskey = new CSecEditExtn;
  m_pctlVerify = new CSecEditExtn;
}

CPasskeySetup::~CPasskeySetup()
{
  delete m_pctlPasskey;
  delete m_pctlVerify;

  if (m_OSK_module != NULL) {
    BOOL brc = FreeLibrary(m_OSK_module);
    pws_os::Trace(L"CPasskeySetup::~CPasskeySetup - Free OSK DLL: %s\n",
                  brc == TRUE ? L"OK" : L"FAILED");
    m_OSK_module = NULL;
  }

  if (m_pVKeyBoardDlg != NULL) {
    // Save Last Used Keyboard
    UINT uiKLID = m_pVKeyBoardDlg->GetKLID();
    std::wostringstream os;
    os.fill(L'0');
    os << std::nouppercase << std::hex << std::setw(8) << uiKLID;
    StringX cs_KLID = os.str().c_str();
    PWSprefs::GetInstance()->SetPref(PWSprefs::LastUsedKeyboard, cs_KLID);

    m_pVKeyBoardDlg->DestroyWindow();
    delete m_pVKeyBoardDlg;
  }
}

void CPasskeySetup::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  
  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);
  m_pctlVerify->DoDDX(pDX, m_verify);

  DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);
  DDX_Control(pDX, IDC_VERIFY, *m_pctlVerify);
}

BEGIN_MESSAGE_MAP(CPasskeySetup, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
  ON_MESSAGE(WM_INSERTBUFFER, OnInsertBuffer)
  ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_VERIFY, OnVerifykeySetfocus)
#if defined(POCKET_PC)
  ON_EN_KILLFOCUS(IDC_PASSKEY, OnPasskeyKillfocus)
  ON_EN_KILLFOCUS(IDC_VERIFY, OnPasskeyKillfocus)
#endif
END_MESSAGE_MAP()

BOOL CPasskeySetup::OnInitDialog() 
{
  CPWDialog::OnInitDialog();
  ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));
  ApplyPasswordFont(GetDlgItem(IDC_VERIFY));

  m_pctlPasskey->SetPasswordChar(PSSWDCHAR);
  m_pctlVerify->SetPasswordChar(PSSWDCHAR);

  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  return TRUE;
}

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
  CreateProcess(L"PegHelp.exe", L"pws_ce_help.html#newdatabase", NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/create_new_db.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
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
#endif

void CPasskeySetup::OnPasskeySetfocus()
{
  m_LastFocus = IDC_PASSKEY;

#if defined(POCKET_PC)
/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
  DisableWordCompletion(m_hWnd);
#endif
}

void CPasskeySetup::OnVerifykeySetfocus()
{
  m_LastFocus = IDC_VERIFY;

#if defined(POCKET_PC)
/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
  DisableWordCompletion(m_hWnd);
#endif
}

void CPasskeySetup::OnVirtualKeyboard()
{
  // Shouldn't be here if couldn't load DLL. Static control disabled/hidden
  if (!CVKeyBoardDlg::IsOSKAvailable())
    return;

  if (m_pVKeyBoardDlg != NULL && m_pVKeyBoardDlg->IsWindowVisible()) {
    // Already there - move to top
    m_pVKeyBoardDlg->SetWindowPos(&wndTop , 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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
  m_pVKeyBoardDlg->SetWindowPos(&wndTop , 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return;
}

LRESULT CPasskeySetup::OnInsertBuffer(WPARAM, LPARAM)
{
  // Update the variables
  UpdateData(TRUE);

  // Get the buffer
  CSecString vkbuffer = m_pVKeyBoardDlg->GetPassphrase();

  CSecEditExtn * m_pSecCtl(NULL);
  CSecString * m_pSecString;

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

  return 0L;
}
