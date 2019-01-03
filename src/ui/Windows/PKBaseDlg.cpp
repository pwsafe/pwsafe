/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "PKBaseDlg.h"
#include "Fonts.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "resource.h"

#include "core/pwsprefs.h"
#include "core/core.h" // for IDSC_UNKNOWN_ERROR
#include "core/Util.h" // for trashMemory()
#include "os/env.h"

#include <iomanip>
#include <sstream>

using namespace std;

const wchar_t CPKBaseDlg::PSSWDCHAR = L'*';

CPKBaseDlg::CPKBaseDlg(int id, CWnd *pParent)
  : CPWDialog(id, pParent),
    m_passkey(L""), m_pctlPasskey(new CSecEditExtn),
    m_pVKeyBoardDlg(NULL)
{
  if (pws_os::getenv("PWS_PW_MODE", false) == L"NORMAL")
    m_pctlPasskey->SetSecure(false);
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer event

  // Call it as it also performs important initilisation
  m_bVKAvailable = CVKeyBoardDlg::IsOSKAvailable();
}

CPKBaseDlg::~CPKBaseDlg()
{
  delete m_pctlPasskey;

  if (m_pVKeyBoardDlg != NULL) {
    if (m_pVKeyBoardDlg->SaveKLID()) {
      // Save Last Used Keyboard
      UINT uiKLID = m_pVKeyBoardDlg->GetKLID();
      std::wostringstream os;
      os.fill(L'0');
      os << std::nouppercase << std::hex << std::setw(8) << uiKLID;
      StringX cs_KLID = os.str().c_str();
      PWSprefs::GetInstance()->SetPref(PWSprefs::LastUsedKeyboard, cs_KLID);
    }

    PWSprefs::GetInstance()->SetPref(PWSprefs::VKPlaySound, m_pVKeyBoardDlg->PlaySound());

    m_pVKeyBoardDlg->DestroyWindow();
    delete m_pVKeyBoardDlg;
  }
}

void CPKBaseDlg::OnDestroy()
{
  KillTimer(TIMER_YUBIKEYPOLL);

  CPWDialog::OnDestroy();
}

void CPKBaseDlg::DoDataExchange(CDataExchange *pDX)
{
  CPWDialog::DoDataExchange(pDX);

  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);
  DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);

  DDX_Control(pDX, IDC_YUBI_PROGRESS, m_yubi_timeout);
  DDX_Control(pDX, IDC_YUBI_STATUS, m_yubi_status);
}

BEGIN_MESSAGE_MAP(CPKBaseDlg, CPWDialog)
  //{{AFX_MSG_MAP(CPKBaseDlg)
  ON_WM_CTLCOLOR()
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPKBaseDlg::OnInitDialog(void)
{
  CPWDialog::OnInitDialog();

  // Setup a timer to poll the key every 250 ms
  SetTimer(TIMER_YUBIKEYPOLL, 250, 0);

  m_yubiLogo.LoadBitmap(IDB_YUBI_LOGO);
  m_yubiLogoDisabled.LoadBitmap(IDB_YUBI_LOGO_DIS);

  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));

  m_pctlPasskey->SetPasswordChar(PSSWDCHAR);

  CWnd *ybn = GetDlgItem(IDC_YUBIKEY_BTN);

  if (YubiExists()) {
    ybn->ShowWindow(SW_SHOW);
    m_yubi_status.ShowWindow(SW_SHOW);
  } else {
    ybn->ShowWindow(SW_HIDE);
    m_yubi_status.ShowWindow(SW_HIDE);
  }

  m_yubi_timeout.ShowWindow(SW_HIDE);
  m_yubi_timeout.SetRange(0, 15);

  // MFC has ancient bug: can't render disabled version of bitmap,
  // so instead of showing drek, we roll our own, and leave enabled.
  ybn->EnableWindow(TRUE);

  bool b_yubiInserted = IsYubiInserted();
  if (b_yubiInserted) {
    ((CButton*)ybn)->SetBitmap(m_yubiLogo);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
  } else {
    ((CButton*)ybn)->SetBitmap(m_yubiLogoDisabled);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CPKBaseDlg::PreTranslateMessage(MSG *pMsg)
{
  // Show/hide caps lock indicator
  CWnd *pCapsLock = GetDlgItem(IDC_CAPSLOCK);
  if (pCapsLock != NULL) {
    pCapsLock->ShowWindow(((GetKeyState(VK_CAPITAL) & 0x0001) == 0x0001) ?
                          SW_SHOW : SW_HIDE);
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CPKBaseDlg::yubiInserted(void)
{
  CButton *ybn = (CButton*)GetDlgItem(IDC_YUBIKEY_BTN);
  ASSERT(ybn != NULL);
  ybn->SetBitmap(m_yubiLogo);
  ybn->ShowWindow(SW_SHOW);
  m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
  m_yubi_status.ShowWindow(SW_SHOW);
}

void CPKBaseDlg::yubiRemoved(void)
{
  CButton *ybn = (CButton*)GetDlgItem(IDC_YUBIKEY_BTN);
  ASSERT(ybn != NULL);
  ybn->SetBitmap(m_yubiLogoDisabled);
  m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
}

void CPKBaseDlg::yubiShowChallengeSent()
{
  // A request's in the air, setup GUI to wait for reply
  m_yubi_status.ShowWindow(SW_HIDE);
  m_yubi_status.SetWindowText(L"");
  m_yubi_timeout.ShowWindow(SW_SHOW);
  m_yubi_timeout.SetPos(15);
}

void CPKBaseDlg::yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf)
{
  switch (yrc) {
  case YKLIB_OK:
    m_yubi_status.ShowWindow(SW_SHOW);
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_timeout.SetPos(0);
    m_yubi_status.SetWindowText(L"");
    pws_os::Trace(L"yubiCheckCompleted: YKLIB_OK");
    m_passkey = Bin2Hex(respBuf, SHA1_DIGEST_SIZE);
    // The returned hash is the passkey
    ProcessPhrase();
    // If we returned from above, reset status:
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
    break;

  case YKLIB_PROCESSING:  // Still processing or waiting for the result
    break;

  case YKLIB_TIMER_WAIT:  // A given number of seconds remain 
    m_yubi_timeout.SetPos(ts);
    break;

  case YKLIB_INVALID_RESPONSE:  // Invalid or no response
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_TIMEOUT)));
    m_yubi_status.ShowWindow(SW_SHOW);
    YubiFailed(); // allow subclass to do something useful
    break;

  default:                // A non-recoverable error has occured
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.ShowWindow(SW_SHOW);
    // Generic error message
    pws_os::Trace(L"yubiCompleted(%d)\n", yrc);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDSC_UNKNOWN_ERROR)));
    break;
  }
}

void CPKBaseDlg::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent != TIMER_YUBIKEYPOLL) {
    CPWDialog::OnTimer(nIDEvent);
    return;
  }

  if (!m_yubiPollDisable)
    YubiPoll();
}

HBRUSH CPKBaseDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  if (nCtlColor == CTLCOLOR_STATIC) {
    UINT nID = pWnd->GetDlgCtrlID();
    if (nID == IDC_CAPSLOCK) {
      pDC->SetTextColor(RGB(255, 0, 0));
      pDC->SetBkMode(TRANSPARENT);
    }
  }
  return hbr;
}
