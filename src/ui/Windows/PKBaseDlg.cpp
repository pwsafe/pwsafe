/*
* Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "PKBaseDlg.h"
#include "SDThread.h"
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

extern LRESULT CALLBACK MsgFilter(int code, WPARAM wParam, LPARAM lParam);

CPKBaseDlg::CPKBaseDlg(int id, CWnd *pParent, bool bUseSecureDesktop)
  : CPWDialog(id, pParent), m_bUseSecureDesktop(bUseSecureDesktop),
  m_passkey(L""), m_pctlPasskey(new CSecEditExtn),
  m_pVKeyBoardDlg(NULL), m_hwndVKeyBoard(NULL)
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

  if (m_pVKeyBoardDlg != NULL && m_pVKeyBoardDlg->SaveKLID()) {
    // Save Last Used Keyboard
    UINT uiKLID = m_pVKeyBoardDlg->GetKLID();
    std::wostringstream os;
    os.fill(L'0');
    os << std::nouppercase << std::hex << std::setw(8) << uiKLID;
    StringX cs_KLID = os.str().c_str();
    PWSprefs::GetInstance()->SetPref(PWSprefs::LastUsedKeyboard, cs_KLID);

    ::DestroyWindow(m_hwndVKeyBoard);
    delete m_pVKeyBoardDlg;
  }
}

void CPKBaseDlg::OnDestroy()
{
  CPWDialog::OnDestroy();
}

void CPKBaseDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_SD_TOGGLE, m_ctlSDToggle);

  if (!m_bUseSecureDesktop) {
    // Can't use DDX_Text for CSecEditExtn
    m_pctlPasskey->DoDDX(pDX, m_passkey);
    DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);

    DDX_Control(pDX, IDC_YUBI_PROGRESS, m_yubi_timeout);
    DDX_Control(pDX, IDC_YUBI_STATUS, m_yubi_status);
  }
}

BEGIN_MESSAGE_MAP(CPKBaseDlg, CPWDialog)
  //{{AFX_MSG_MAP(CPKBaseDlg)
  ON_BN_CLICKED(IDC_SD_TOGGLE, OnSwitchSecureDesktop)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPKBaseDlg::OnInitDialog(void)
{
  CPWDialog::OnInitDialog();

  // Add Secure Desktop Toggle button Tooltip - don't extend tooltip times
  InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0);

  AddTool(IDC_SD_TOGGLE, m_bUseSecureDesktop ? IDS_TOGGLE_SECURE_DESKTOP_ON : IDS_TOGGLE_SECURE_DESKTOP_OFF);
  ActivateToolTip();

  // Setup a timer to poll the key every 250 ms
  SetTimer(1, 250, 0);

  // This bit makes the background come out right on the bitmaps - these 2 bitmaps use white as the mask
  m_ctlSDToggle.SetBitmapMaskAndID(RGB(255, 255, 255), m_bUseSecureDesktop ? IDB_USING_SD : IDB_NOT_USING_SD);

  m_yubiLogo.LoadBitmap(IDB_YUBI_LOGO);
  m_yubiLogoDisabled.LoadBitmap(IDB_YUBI_LOGO_DIS);

  if (!m_bUseSecureDesktop) {
    Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));

    m_pctlPasskey->SetPasswordChar(PSSWDCHAR);

    CWnd *ybn = GetDlgItem(IDC_YUBIKEY_BTN);

    if (YubiExists()) {
      ybn->ShowWindow(SW_SHOW);
      m_yubi_status.ShowWindow(SW_SHOW);
    }
    else {
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
    }
    else {
      ((CButton*)ybn)->SetBitmap(m_yubiLogoDisabled);
      m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
    }
  }

  return TRUE;
}

BOOL CPKBaseDlg::PreTranslateMessage(MSG* pMsg)
{
  RelayToolTipEvent(pMsg);

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CPKBaseDlg::yubiInserted(void)
{
  CButton *ybn = (CButton*)GetDlgItem(IDC_YUBIKEY_BTN);
  // Not there if Secure Desktop enabled
  if (ybn != NULL) {
    ybn->SetBitmap(m_yubiLogo);
    ybn->ShowWindow(SW_SHOW);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
    m_yubi_status.ShowWindow(SW_SHOW);
  }
}

void CPKBaseDlg::yubiRemoved(void)
{
  CButton *ybn = (CButton*)GetDlgItem(IDC_YUBIKEY_BTN);
  // Not there if Secure Desktop enabled
  if (ybn != NULL) {
    ybn->SetBitmap(m_yubiLogoDisabled);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
  }
}

void CPKBaseDlg::yubiShowChallengeSent()
{
  // A request's in the air, setup GUI to wait for reply
  m_yubi_status.ShowWindow(SW_HIDE);
  m_yubi_status.SetWindowText(_T(""));
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
    m_yubi_status.SetWindowText(_T(""));
    TRACE(_T("yubiCheckCompleted: YKLIB_OK"));
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
    TRACE(_T("yubiCompleted(%d)\n"), yrc);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDSC_UNKNOWN_ERROR)));
    break;
  }
}

void CPKBaseDlg::OnTimer(UINT_PTR )
{
  if (!m_yubiPollDisable)
    YubiPoll();
}

void CPKBaseDlg::OnSwitchSecureDesktop()
{
  EndDialog(INT_MAX);
}

void CPKBaseDlg::StartThread(int iDialogType, HMONITOR hCurrentMonitor)
{
  // SetThreadDesktop fails in MFC because _AfxMsgFilterHook is used in every
  // Thread. Need to unhook and before calling SetThreadDesktop
  // Reset the hook again to msgfilter (equivalent to _AfxMsgFilterHook)
  // after finishing processing and before returning.

  LARGE_INTEGER liDueTime;
  HANDLE hThread(0);
  DWORD dwError, dwThreadID, dwEvent;
  bool bTimerPopped(false);

  // Set good return code
  m_dwRC = 0;

  // Clear progress flags
  BYTE xFlags = 0;

  _AFX_THREAD_STATE *pState = AfxGetThreadState();

  BOOL bReHook = UnhookWindowsHookEx(pState->m_hHookOldMsgFilter);
  if (!bReHook) {
    ASSERT(bReHook);
    // goto BadExit; nothing to cleanup, don't use goto in order
    // not to skip on CSDThread c'tor
    m_dwRC = (DWORD)-1; // Set bad return code
    return;
  }

  // Update progress
  xFlags |= WINDOWSHOOKREMOVED;

  pState->m_hHookOldMsgFilter = NULL;

  // Get current Monitor if not supplied (only from Wizard)
  if (hCurrentMonitor == NULL) {
    ASSERT(this->GetSafeHwnd());
    hCurrentMonitor = MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
  }

  // Create Dialog Thread class instance
  CSDThread thrdDlg(iDialogType, &m_GMP, hCurrentMonitor, m_bUseSecureDesktop);

  // Set up waitable timer just in case there is an issue
  thrdDlg.m_hWaitableTimer = CreateWaitableTimer(NULL, FALSE, NULL);

  if (thrdDlg.m_hWaitableTimer == NULL) {
    dwError = pws_os::IssueError(_T("CreateWaitableTimer"), false);
    ASSERT(thrdDlg.m_hWaitableTimer);
    goto BadExit;
  }

  // Update progress
  xFlags |= WAITABLETIMERCREATED;

  // Get out of Jail Free Card method in case there is a problem in the thread
  // Set the timer to go off PWSprefs::SecureDesktopTimeout after calling SetWaitableTimer.
  // Timer unit is 100-nanoseconds
  int iUserTimeLimit = PWSprefs::GetInstance()->GetPref(PWSprefs::SecureDesktopTimeout);
  liDueTime.QuadPart = -(iUserTimeLimit * 10000000);

  if (!SetWaitableTimer(thrdDlg.m_hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)) {
    dwError = pws_os::IssueError(_T("SetWaitableTimer"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update progress
  xFlags |= WAITABLETIMERSET;

  // Create thread
  hThread = CreateThread(NULL, 0, thrdDlg.SDThreadProc, (void *)&thrdDlg, CREATE_SUSPENDED, &dwThreadID);
  if (hThread == NULL) {
    dwError = pws_os::IssueError(_T("CreateThread"), false);
    ASSERT(hThread);
    goto BadExit;
  }

  // Update progress
  xFlags |= THREADCREATED;

  // Avoid polling Yubikey from > 1 thread
  m_yubiPollDisable = true;

  // Resume thread (not really necessary to create it suspended and then resume but just in
  // case we want to do extra processing between creation and running
  ResumeThread(hThread);

  // Update progress
  xFlags |= THREADRESUMED;

  // Set up array of wait handles and wait for either the timer to pop or the thread to end
  {
    HANDLE hWait[2] = { thrdDlg.m_hWaitableTimer, hThread };
    dwEvent = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);
  }

  // we can allow yubi polling again
  m_yubiPollDisable = false;

  // Find out what happened
  switch (dwEvent) {
    case WAIT_OBJECT_0 + 0:
    {
      // Timer popped
      bTimerPopped = true;

      // Update Progress
      xFlags &= ~WAITABLETIMERSET;

      // Stop thread - by simulating clicking on Cancel button
      ::SendMessage(thrdDlg.m_hwndMasterPhraseDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);

      // Now wait for thread to complete
      WaitForSingleObject(hThread, INFINITE);

      // Update progress
      xFlags &= ~THREADRESUMED;
      break;
    }
    case WAIT_OBJECT_0 + 1:
    {
      // Thread ended
      // Update progress
      xFlags &= ~THREADRESUMED;

      // Cancel timer
      if (!CancelWaitableTimer(thrdDlg.m_hWaitableTimer)) {
        dwError = pws_os::IssueError(_T("CancelWaitableTimer"), false);
        ASSERT(0);
        goto BadExit;
      }

      // Update progress
      xFlags &= ~WAITABLETIMERSET;

      break;
    }
    case WAIT_FAILED:
    {
      // Should not happen!
      dwError = pws_os::IssueError(_T("WAIT_FAILED"), false);
      ASSERT(0);
      goto BadExit;
    }
  }  // switch on dwEvent (Wait reason)

  // Close the WaitableTimer handle
  if (!CloseHandle(thrdDlg.m_hWaitableTimer)) {
    dwError = pws_os::IssueError(_T("CloseHandle - hWaitableTimer"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags &= ~(WAITABLETIMERCREATED | WAITABLETIMERSET);

  // Before deleting the thread - get its return code
  GetExitCodeThread(hThread, &m_dwRC);

  // Update Progress
  xFlags &= ~(THREADCREATED | THREADRESUMED);

  // Put hook back
  if (bReHook) {
    // Can't put old hook back as we only had its handle - not its address.
    // Put our version there
    pState->m_hHookOldMsgFilter = SetWindowsHookEx(WH_MSGFILTER, MsgFilter, NULL, GetCurrentThreadId());
    if (pState->m_hHookOldMsgFilter == NULL) {
      dwError = pws_os::IssueError(_T("SetWindowsHookEx"), false);
      ASSERT(pState->m_hHookOldMsgFilter);
    }
    xFlags &= ~WINDOWSHOOKREMOVED;
  }

  return;

BadExit:
  // Need to tidy up what was done in reverse order - ignoring what wasn't and ignore errors
  if (xFlags & THREADRESUMED) {
    // Stop thread - by simulating clicking on Cancel button
    ::SendMessage(thrdDlg.m_hwndMasterPhraseDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);

    // Now wait for thread to complete
    WaitForSingleObject(hThread, INFINITE);
  }
  if (xFlags & THREADCREATED) {
  }
  if (xFlags & WAITABLETIMERSET) {
    ::CancelWaitableTimer(thrdDlg.m_hWaitableTimer);
  }
  if (xFlags & WAITABLETIMERCREATED) {
    CloseHandle(thrdDlg.m_hWaitableTimer);
  }
  if (xFlags & WINDOWSHOOKREMOVED) {
    pState->m_hHookOldMsgFilter = SetWindowsHookEx(WH_MSGFILTER, MsgFilter, NULL, GetCurrentThreadId());
  }

  // Set bad return code
  m_dwRC = (DWORD)-1;
}
