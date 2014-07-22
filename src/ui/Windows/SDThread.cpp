/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ThreadedDlg.cpp : implementation file
//

#include "stdafx.h"

#include "SDThread.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "resource3.h"

#include "core/core.h" // for IDSC_UNKNOWN_ERROR
#include "core/PWSprefs.h"
#include "core/PWPolicy.h"
#include "core/PWCharPool.h" // for CheckPassword()
#include "os/debug.h"

#include "resource.h"

#include <aclapi.h>
#include <tlhelp32.h>
#include <algorithm>

// Following makes debugging SD UI changes feasible
// Of course, remove if/when debugging the Secure Desktop funtionality itself...
#ifdef _DEBUG
//#define NO_NEW_DESKTOP
#endif

int iStartTime;  // Start time for SD timer - does get reset by edit changes or mouse clicks (VK)

extern ThisMfcApp app;

extern LRESULT CALLBACK MsgFilter(int code, WPARAM wParam, LPARAM lParam);

template<class T> void  RDLL_LoadAString(HINSTANCE hInstResDLL, T &s, int id)
{
  // Load a String from the Application or Resource DLL (can't use MFC approach
  // or MFC CString class)
  TCHAR *psBuffer;
  int len = LoadString(hInstResDLL, id, reinterpret_cast<LPTSTR>(&psBuffer), 0);

  if (len)
    s = T(psBuffer, len);
  else
    s = T();
}

CSDThread::CSDThread(int iDialogID, GetMasterPhrase *pGMP,
                     HMONITOR hCurrentMonitor, bool bUseSecureDesktop)
  : m_pGMP(pGMP), m_wDialogID((WORD)iDialogID),
  m_hCurrentMonitor(hCurrentMonitor), m_bUseSecureDesktop(bUseSecureDesktop),
  m_passkeyID(-1), m_hNewDesktop(NULL), m_hwndMasterPhraseDlg(NULL), m_pVKeyBoardDlg(NULL),
  m_bDoTimerProcAction(false), m_bMPWindowBeingShown(false), m_bVKWindowBeingShown(false),
  m_iMinutes(-1), m_iSeconds(-1), m_hWaitableTimer(0)
{
  InitInstance();
}

CSDThread::~CSDThread()
{
}

BOOL CSDThread::InitInstance()
{
  // Get us
  m_hInstance = GetModuleHandle(NULL);

  // Only called once the Thread is "Resumed"
  _AFX_THREAD_STATE *pState = AfxGetThreadState();

  if (pState->m_hHookOldMsgFilter) {
    if (!UnhookWindowsHookEx(pState->m_hHookOldMsgFilter)) {
      pws_os::IssueError(_T("UnhookWindowsHookEx"), false);
      ASSERT(0);
    }
    pState->m_hHookOldMsgFilter = NULL;
  }

  m_pGMP->clear();

  m_iUserTimeLimit = PWSprefs::GetInstance()->GetPref(PWSprefs::SecureDesktopTimeout);
  m_hTimer = NULL;

  // Clear progress flags
  xFlags = 0;

  // Get Resource DLL (if any)
  m_hInstResDLL = app.GetResourceDLL();
  if (!m_hInstResDLL)
    m_hInstResDLL = m_hInstance;

  return TRUE;
}

DWORD WINAPI CSDThread::SDThreadProc(LPVOID lpParameter)
{
  CSDThread *self = (CSDThread *)lpParameter;

  return self->ThreadProc();
}

DWORD CSDThread::ThreadProc()
{
  WNDCLASS wc = { 0 };

  StringX sxTemp, sxPrefix;
  DWORD dwError;
  m_dwRC = (DWORD)-1;

  m_pGMP->clear();
  m_hwndVKeyBoard = NULL;

  PWPolicy policy;

  // Get Monitor Images before we create new desktop
  GetMonitorImages();

  // Update Progress
  xFlags |= MONITORIMAGESCREATED;

  // Get uppercase prefix - 1st character for Desktop name, 2nd for Window Class name
  policy.flags = PWPolicy::UseUppercase;
  policy.length = 2;
  policy.upperminlength = 2;

  sxPrefix = policy.MakeRandomPassword();

  // Future use of this is for the next 15 characters of Dekstop & Window Class names
  policy.flags = PWPolicy::UseLowercase | PWPolicy::UseUppercase | PWPolicy::UseDigits;
  policy.length = 15;
  policy.lowerminlength = policy.upperminlength = policy.digitminlength = 1;

#ifndef NO_NEW_DESKTOP
  m_hOriginalDesk = GetThreadDesktop(GetCurrentThreadId());

  // Ensure we don't use an existing Desktop (very unlikely but....)
  do {
    //Create random Desktop name
    sxTemp = sxPrefix.substr(0, 1) + policy.MakeRandomPassword();

    m_sDesktopName = sxTemp.c_str();

    // Check not already there
    CheckDesktop();
  } while (m_bDesktopPresent);

  // Windows starts a copy of ctfmon.exe per Desktop for support of Microsoft Text
  // Services. Ctfmon.exe will also prevent a new Desktop closing after all Windows
  // on it have closed.
  // After the thread thread has switched back to the original desktop (but before
  // the new Desktop is closed), we terminate all processes running on that new
  // Desktop.  This includes ctfmon.exe.

  // Try to create the necessary Security Attributes i.e. only giving the
  // current user Desktop specific access rights and standard access to the Desktop
  // On systems running NVIDIA Display Driver Service (nvsvc), it will normally stop CloseDesktop
  // completing the close.  By setting these Security Asttributes, nvsvc is prevented from
  // grabbing the new Desktop.

  // DO NOT add access DESKTOP_HOOKCONTROL as it will allow other processes to grap the
  // passphrase and/or record mouse clicks on the Virtual Keyboard.
  DWORD dwDesiredAccess = DESKTOP_CREATEWINDOW | DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | DESKTOP_SWITCHDESKTOP |
    DELETE | READ_CONTROL;
#ifdef _DEBUG
  dwDesiredAccess |= DESKTOP_ENUMERATE;
#endif

  SECURITY_ATTRIBUTES sa;
  PSECURITY_DESCRIPTOR pSD(NULL);
  PACL pACL(NULL);
  PSID pCurrentUserSID(NULL);
  PSID pOwnerSID(NULL);
  bool bSA_Created = CreateSA(sa, dwDesiredAccess, pSD, pACL, pOwnerSID, pCurrentUserSID);

  pws_os::Trace(_T("NewDesktop %s\n"), m_sDesktopName.c_str());
  m_hNewDesktop = CreateDesktop(m_sDesktopName.c_str(), NULL, NULL, 0, dwDesiredAccess, bSA_Created ? &sa : NULL);

  // Free security data no longer required
  if (pOwnerSID)
    FreeSid(pOwnerSID);
  if (pCurrentUserSID)
    FreeSid(pCurrentUserSID);
  if (pACL)
    LocalFree(pACL);
  if (pSD)
    LocalFree(pSD);

  if (m_hNewDesktop == NULL) {
    dwError = pws_os::IssueError(_T("CreateDesktop (new)"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags |= NEWDESKTOCREATED;

  if (!SetThreadDesktop(m_hNewDesktop)) {
    dwError = pws_os::IssueError(_T("SetThreadDesktop to new"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags |= SETTHREADDESKTOP;
#endif

  // Ensure we don't use an existing Window Class Name (very unlikely but....)
  do {
    // Create random Modeless Overlayed Background Window Class Name
    sxTemp = sxPrefix.substr(1, 1) + policy.MakeRandomPassword();

    m_sBkGrndClassName = sxTemp.c_str();

    // Check not already there
    CheckWindow();
  } while (m_bWindowPresent);

  // Register the Window Class Name
  wc.lpfnWndProc = ::DefWindowProc;
  wc.hInstance = m_hInstance;
  wc.lpszClassName = m_sBkGrndClassName.c_str();
  if (!RegisterClass(&wc)) {
    dwError = pws_os::IssueError(_T("RegisterClass - Background Window"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags |= REGISTEREDWINDOWCLASS;

  for (MonitorImageInfoIter it = m_vMonitorImageInfo.begin(); it != m_vMonitorImageInfo.end(); it++) {
    // Window styles: WS_EX_LAYERED for easy painting, WS_EX_TOOLWINDOW so not to appear in Alt+Tab
    HWND hwndBkGrndWindow = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW,
      m_sBkGrndClassName.c_str(), NULL, WS_POPUP | WS_VISIBLE,
      it->left, it->top, it->width, it->height,
      NULL, NULL, m_hInstance, NULL);

    if (!hwndBkGrndWindow) {
      dwError = pws_os::IssueError(_T("CreateWindowEx - Background"), false);
      ASSERT(0);
      goto BadExit;
    }

    // Save handle to background window
    it->hwndBkGrndWindow = hwndBkGrndWindow;

    HDC hdcScreen = GetDC(0);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdc, it->hbmDimmendMonitorImage);

    // Get the size of the bitmap
    BITMAP bm;
    GetObject(it->hbmDimmendMonitorImage, sizeof(bm), &bm);
    SIZE sizeBkGnd = {bm.bmWidth, bm.bmHeight};

    // Use the source image's alpha channel for blending
    BLENDFUNCTION bf = {0};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    POINT ptZero = { 0 ,0 };

    // Paint the window (in the right location) with the alpha-blended bitmap
    UpdateLayeredWindow(hwndBkGrndWindow, NULL, NULL, &sizeBkGnd,
      hdc, &ptZero, RGB(0, 0, 0), &bf, ULW_OPAQUE);

    // Don't allow any action if this is clicked!
    EnableWindow(hwndBkGrndWindow, FALSE);

    // Tidy up GDI resources
    ::ReleaseDC(NULL, hdcScreen);
    ::SelectObject(hdc, hbmOld);
    ::DeleteDC(hdc);
  }

  // Update Progress
  xFlags |= BACKGROUNDWINDOWSCREATED;


#ifndef NO_NEW_DESKTOP
  if (!SwitchDesktop(m_hNewDesktop)) {
    dwError = pws_os::IssueError(_T("SwitchDesktop to new"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags |= SWITCHEDDESKTOP;
#endif

  m_hwndMasterPhraseDlg = CreateDialogParam(m_hInstResDLL, MAKEINTRESOURCE(m_wDialogID),
    HWND_DESKTOP, (DLGPROC)MPDialogProc, reinterpret_cast<LPARAM>(this));

  if (!m_hwndMasterPhraseDlg) {
    dwError = pws_os::IssueError(_T("CreateDialogParam - IDD_SDGETPHRASE"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags |= MASTERPHRASEDIALOGCREATED;

  // Use first monitor's background window as owner for virtual keyboard
  m_pVKeyBoardDlg = new CVKeyBoardDlg(m_hInstResDLL, m_vMonitorImageInfo[0].hwndBkGrndWindow, m_hwndMasterPhraseDlg);

  // Update Progress
  xFlags |= VIRTUALKEYBOARDCREATED;

  ShowWindow(m_hwndMasterPhraseDlg, SW_SHOW);

  MSG msg;
  BOOL brc;

  // Message loop - break out on WM_QUIT or error
  while ((brc = GetMessage(&msg, 0, 0, 0)) != 0) {
    if (brc == -1)
      break;

    // Do not allow Escape to end this dialog
    if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_ESCAPE)
      continue;

    if (!IsDialogMessage(m_hwndMasterPhraseDlg, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  // Call DialogProc directly to clear "self".
  // NOTE: - it would NEVER get the WM_QUIT message EVER as this
  // is used to get out of the message loop above(see GetMessage)
  MPDialogProc(NULL, WM_QUIT, NULL, NULL);

  // Update Progress
  xFlags |= MASTERPHRASEDIALOGENDED;

  // Destroy Masterphrase window
  if (!DestroyWindow(m_hwndMasterPhraseDlg)) {
    dwError = pws_os::IssueError(_T("DestroyWindow - IDD_SDGETPHRASE"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags &= ~MASTERPHRASEDIALOGCREATED;

  // Delete Virtual Keyboard instance
  delete m_pVKeyBoardDlg;

  // Update Progress
  xFlags &= ~VIRTUALKEYBOARDCREATED;

  // Destroy background layered windows, images & monitor DCs
  for (MonitorImageInfoIter it = m_vMonitorImageInfo.begin(); it != m_vMonitorImageInfo.end(); it++) {
    if (it->hwndBkGrndWindow != NULL)
      DestroyWindow(it->hwndBkGrndWindow);
    if (it->hbmDimmendMonitorImage)
      ::DeleteObject(it->hbmDimmendMonitorImage);
    if (it->hdcMonitor)
      ::DeleteDC(it->hdcMonitor);
  }

  // Update Progress
  xFlags &= ~(BACKGROUNDWINDOWSCREATED | MONITORIMAGESCREATED);

  // Unregister it
  if (!UnregisterClass(m_sBkGrndClassName.c_str(), m_hInstance)) {
    dwError = pws_os::IssueError(_T("UnregisterClass - Background"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags &= ~REGISTEREDWINDOWCLASS;

#ifndef NO_NEW_DESKTOP
  if (xFlags & SWITCHEDDESKTOP) {
    // Switch back to the initial desktop - due to the possibility that
    // the user may have locked the screen (manually or automatically),
    // or the computer may have gone to sleep/hibernated, we can't switch
    // back to the original Desktop if the MS WinLogon Desktop has control.
    // We ask to be informed when there is a desktop switch and so we can try again.
    // However, for example if the user switches to another account, it might
    // not be for us and so we wait for the next notification etc.
    // Eventually, it should allow us to switch back.

    HANDLE hDesktopSwitch = OpenEvent(SYNCHRONIZE, false, _T("WinSta0_DesktopSwitch"));
    if (!hDesktopSwitch)
      dwError = pws_os::IssueError(_T("OpenEvent - SYNCHRONIZE - WinSta0_DesktopSwitch"), false);

    pws_os::Trace(_T("ThreadProc SwitchDesktop\n"));
    while (!SwitchDesktop(m_hOriginalDesk)) {
      pws_os::Trace(_T("SwitchDesktop(m_hOriginalDesk)\n"));
      dwError = pws_os::IssueError(_T("SwitchDesktop - back to original"), false);
      if (dwError != ERROR_SUCCESS)
        ASSERT(0);

      if (hDesktopSwitch) {
        WaitForSingleObject(hDesktopSwitch, INFINITE);
        pws_os::Trace(_T("Wait for WinSta0_DesktopSwitch signalled\n"));
      }

      // Try again soon
      ::Sleep(500);
    }

    // Close Handle
    if (hDesktopSwitch)
      CloseHandle(hDesktopSwitch);

    // Update Progress
    xFlags &= ~SWITCHEDDESKTOP;
  }

  if (xFlags & SETTHREADDESKTOP) {
    // Switch thread back to original Desktop
    if (!SetThreadDesktop(m_hOriginalDesk)) {
      dwError = pws_os::IssueError(_T("SetThreadDesktop - back to original"), false);
      ASSERT(0);
      goto BadExit;
    }
    // Update Progress
    xFlags &= ~SETTHREADDESKTOP;
  }

  // Now that thread is ending - close new Desktop
  if (xFlags & NEWDESKTOCREATED) {
    // Terminate processes still on new Desktop BEFORE we close it
    TerminateProcesses();

    // Note: There can be a good return code even if it does not close
    // due to programs external to PWS keeping it around!
    if (!CloseDesktop(m_hNewDesktop)) {
      dwError = pws_os::IssueError(_T("CloseDesktop (new)"), false);
      ASSERT(0);
    } else {
      // Update Progress
      xFlags &= ~NEWDESKTOCREATED;
    }
  }
#endif

  // Clear variables - just in case someone decides to reuse this instance
  m_pVKeyBoardDlg = NULL;
  m_hwndMasterPhraseDlg = NULL;
  m_sBkGrndClassName.clear();
  m_sDesktopName.clear();
  m_vMonitorImageInfo.clear();

  return m_dwRC;

BadExit:
  // Need to tidy up what was done in reverse order - ignoring what wasn't and ignore errors
  // First clear anything the user has entered
  m_pGMP->clear();

  if (xFlags & VIRTUALKEYBOARDCREATED) {
    // Delete Virtual Keyboard instance
    delete m_pVKeyBoardDlg;

    // Update Progress
    xFlags &= ~VIRTUALKEYBOARDCREATED;
  }

  if (xFlags & MASTERPHRASEDIALOGCREATED) {
    // Destroy master phrase dialog window
    DestroyWindow(m_hwndMasterPhraseDlg);

    // Update Progress
    xFlags &= ~MASTERPHRASEDIALOGCREATED;
  }

  if (xFlags & BACKGROUNDWINDOWSCREATED || xFlags & MONITORIMAGESCREATED) {
    // Destroy background layered window's, images & monitor DCs
    for (MonitorImageInfoIter it = m_vMonitorImageInfo.begin(); it != m_vMonitorImageInfo.end(); it++) {
      if (it->hwndBkGrndWindow)
        DestroyWindow(it->hwndBkGrndWindow);
      if (it->hbmDimmendMonitorImage)
        ::DeleteObject(it->hbmDimmendMonitorImage);
      if (it->hdcMonitor)
        ::DeleteDC(it->hdcMonitor);
    }

    // Update Progress
    xFlags &= ~(BACKGROUNDWINDOWSCREATED | MONITORIMAGESCREATED);
  }

  if (xFlags & REGISTEREDWINDOWCLASS) {
    // Unregister background windows' registered class
    UnregisterClass(m_sBkGrndClassName.c_str(), m_hInstance);

    // Update Progress
    xFlags &= ~REGISTEREDWINDOWCLASS;
  }

  if (xFlags & SWITCHEDDESKTOP) {
    // Switch back to the initial desktop - see comments above
    HANDLE hDesktopSwitch = OpenEvent(SYNCHRONIZE, false, _T("WinSta0_DesktopSwitch"));
    if (!hDesktopSwitch)
      dwError = pws_os::IssueError(_T("OpenEvent - SYNCHRONIZE - WinSta0_DesktopSwitch (bad exit)"), false);

    while (!SwitchDesktop(m_hOriginalDesk)) {
      dwError = pws_os::IssueError(_T("SwitchDesktop - back to original (bad exit)"), false);
      if (dwError != ERROR_SUCCESS)
        ASSERT(0);

      if (hDesktopSwitch) {
        WaitForSingleObject(hDesktopSwitch, INFINITE);
        pws_os::Trace(_T("Wait for WinSta0_DesktopSwitch signalled (bad exit)\n"));
      }

      // Try again soon
      ::Sleep(500);
    }

    // Close Handle
    if (hDesktopSwitch)
      CloseHandle(hDesktopSwitch);

    // Update Progress
    xFlags &= ~SWITCHEDDESKTOP;
  }

  if (xFlags & SETTHREADDESKTOP) {
    // Switch thread back to initial desktop
    if (!SetThreadDesktop(m_hOriginalDesk)) {
      dwError = pws_os::IssueError(_T("SetThreadDesktop - back to original (bad exit)"), false);
      ASSERT(0);
    } else {
      // Update Progress
      xFlags &= ~SETTHREADDESKTOP;
    }
  }
  if (xFlags & NEWDESKTOCREATED) {
    // Terminate processes still on new Desktop BEFORE we close it
    TerminateProcesses();

    // Close the new desktop (subject to programs external to PWS keeping it around!)
    CloseDesktop(m_hNewDesktop);

    // Update Progress
    xFlags &= ~NEWDESKTOCREATED;
  }

  // Clear variables - just in case someone decides to reuse this instance
  m_pVKeyBoardDlg = NULL;
  m_hwndMasterPhraseDlg = NULL;
  m_sBkGrndClassName.clear();
  m_sDesktopName.clear();
  m_vMonitorImageInfo.clear();

  return (DWORD)-1;
}

// Is Desktop there?
BOOL CALLBACK CSDThread::DesktopEnumProc(LPTSTR name, LPARAM lParam)
{
  CSDThread *self = (CSDThread *)lParam;

  // If already there, set flag and no need to be called again
  if (_tcsicmp(name, self->m_sDesktopName.c_str()) == 0) {
    self->m_bDesktopPresent = true;
    return FALSE;
  }

  return TRUE;
}

void CSDThread::CheckDesktop()
{
  m_bDesktopPresent = false;

  // Check if Desktop already created and still there
  HWINSTA station = GetProcessWindowStation();
  EnumDesktops(station, (DESKTOPENUMPROC)DesktopEnumProc, reinterpret_cast<LPARAM>(this));
  CloseWindowStation(station);
}

// Is Window there?
BOOL CALLBACK CSDThread::WindowEnumProc(HWND hwnd, LPARAM lParam)
{
  CSDThread *self = (CSDThread *)lParam;

  // Get Window Class Name
  const int nMaxCOunt = 256;
  TCHAR szClassName[nMaxCOunt] = { 0 };
  int irc = GetClassName(hwnd, szClassName, nMaxCOunt);
  if (irc == 0) {
    pws_os::IssueError(_T("WindowEnumProc - Error return from GetClassName"), false);
    ASSERT(0);
    self->m_bWindowPresent = true;
    return FALSE;
  }

  // If already there, set flag and no need to be called again
  if (_tcsicmp(szClassName, self->m_sBkGrndClassName.c_str()) == 0) {
    self->m_bWindowPresent = true;
    return FALSE;
  }

  return TRUE;
}

void CSDThread::CheckWindow()
{
  m_bWindowPresent = false;

  // Populate vector with desktop names.
  HWINSTA station = GetProcessWindowStation();
  EnumWindows((WNDENUMPROC)WindowEnumProc, reinterpret_cast<LPARAM>(this));
  CloseWindowStation(station);
}

void CSDThread::YubiControlsUpdate(bool insertedOrRemoved)
{
  HWND hwndYbn = GetDlgItem(m_hwndDlg, IDC_YUBIKEY_BTN);
  HWND hwndYbn2 = GetDlgItem(m_hwndDlg, IDC_YUBIKEY2_BTN); // only in Change Combination
  HWND hwndYstatus = GetDlgItem(m_hwndDlg, IDC_YUBI_STATUS);
  HWND hwndYprog = GetDlgItem(m_hwndDlg, IDC_YUBI_PROGRESS);

  if (!YubiExists()) {
    // if YubiKey was never detected, hide relevant controls
    if (hwndYbn != NULL) ShowWindow(hwndYbn, SW_HIDE);
    if (hwndYbn2 != NULL) ShowWindow(hwndYbn2, SW_HIDE);
    if (hwndYstatus != NULL) ShowWindow(hwndYstatus, SW_HIDE);
    if (hwndYprog != NULL) ShowWindow(hwndYprog, SW_HIDE);
  } else { // YubiExists - deal with it
    HBITMAP hbm;
    stringT sMessage;

    if (insertedOrRemoved) {
      hbm = HBITMAP(m_yubiLogo);
    } else {
      hbm = HBITMAP(m_yubiLogoDisabled);
    }
    RDLL_LoadAString(m_hInstResDLL, sMessage, insertedOrRemoved ? IDS_YUBI_CLICK_PROMPT : IDS_YUBI_INSERT_PROMPT);

    if (hwndYstatus != NULL) {
      SetWindowText(hwndYstatus, sMessage.c_str());
      ShowWindow(hwndYstatus, SW_SHOW);
    }

    if (hwndYbn != NULL) {
      SendMessage(hwndYbn, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbm);
      ShowWindow(hwndYbn, SW_SHOW);
    }
    if (hwndYbn2 != NULL) {
      SendMessage(hwndYbn2, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbm);
      ShowWindow(hwndYbn2, SW_SHOW);
    }
  }
}

void CSDThread::yubiShowChallengeSent()
{
  // A request's in the air, setup GUI to wait for reply

  // Since we can't get here unless the yubi button was clicked,
  // we can assume the relevant windows are there.
  HWND hwndYstatus = GetDlgItem(m_hwndDlg, IDC_YUBI_STATUS);
  HWND hwndYprog = GetDlgItem(m_hwndDlg, IDC_YUBI_PROGRESS);

  ShowWindow(hwndYstatus, SW_HIDE);
  SetWindowText(hwndYstatus, _T(""));
  ShowWindow(hwndYprog, SW_SHOW);
  SendMessage(hwndYprog, PBM_SETPOS, 15, 0);
}

void CSDThread::yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf)
{
  ASSERT(m_passkeyID == IDC_PASSKEY || m_passkeyID == IDC_NEWPASSKEY);
  // Since we can't get here unless the yubi button was clicked,
  // we can assume the relevant windows are there.
  HWND hwndYstatus = GetDlgItem(m_hwndDlg, IDC_YUBI_STATUS);
  HWND hwndYprog = GetDlgItem(m_hwndDlg, IDC_YUBI_PROGRESS);
  stringT sMessage;

  switch (yrc) {
  case YKLIB_OK:
    SendMessage(hwndYprog, PBM_SETPOS, 0, 0);
    ShowWindow(hwndYprog, SW_HIDE);
    SetWindowText(hwndYstatus, _T(""));
    ShowWindow(hwndYstatus, SW_SHOW);
    if (m_passkeyID == IDC_PASSKEY) {
      m_yubiResp[0] = Bin2Hex(respBuf, SHA1_DIGEST_SIZE);
      if (m_wDialogID != IDD_SDKEYCHANGE)
        ::SendMessage(m_hwndMasterPhraseDlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
    } else { // IDC_NEWPASSKEY
      m_yubiResp[1] = Bin2Hex(respBuf, SHA1_DIGEST_SIZE);
    }
    break;

  case YKLIB_PROCESSING:  // Still processing or waiting for the result
    break;

  case YKLIB_TIMER_WAIT:  // A given number of seconds remain
    SendMessage(hwndYprog, PBM_SETPOS, ts, 0);
    break;

  case YKLIB_INVALID_RESPONSE:  // Invalid or no response
    ShowWindow(hwndYprog, SW_HIDE);
    RDLL_LoadAString(m_hInstResDLL, sMessage, IDS_YUBI_TIMEOUT);
    SetWindowText(hwndYstatus, sMessage.c_str());
    ShowWindow(hwndYstatus, SW_SHOW);
    break;

  default:                // A non-recoverable error has occured
    ShowWindow(hwndYprog, SW_HIDE);
    RDLL_LoadAString(m_hInstResDLL, sMessage, IDSC_UNKNOWN_ERROR);
    SetWindowText(hwndYstatus, sMessage.c_str());
    ShowWindow(hwndYstatus, SW_SHOW);
    break;
  }
}

void CSDThread::yubiInserted(void)
{
  YubiControlsUpdate(true);
}

void CSDThread::yubiRemoved(void)
{
  YubiControlsUpdate(false);
}

static StringX GetControlText(const HWND hwnd)
{
  int n = GetWindowTextLength(hwnd) + 1;
  if (n > 1) {
    StringX s(n, 0);
    GetWindowText(hwnd, &s[0], n);
    s.pop_back();  // Remove trailing NULL [C++11 feature]
    return s;
  }
  return _T("");
}

INT_PTR CSDThread::MPDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static CSDThread *self(NULL);

  if (uMsg != WM_INITDIALOG && self == NULL)
    return FALSE;

  switch (uMsg) {
    case WM_INITDIALOG:
    {
      // Perform our intial dialog processing
      // Note: The dialog has already set up by Windows before this is called.
      self = (CSDThread *)lParam;
      self->m_hwndDlg = hwndDlg;

      self->OnInitDialog();

      return TRUE; // Processed - special case - focus default control from RC file
    }

    // WM_ACTIVATEAPP - Cancel Secure Dialog if another application is activated
    // Gets called before WM_WTSSESSION_CHANGE, WM_POWERBROADCAST or WM_QUERYENDSESSION
    case WM_ACTIVATEAPP:
    {
      pws_os::Trace(_T("WM_ACTIVATEAPP. wParam = %d\n"), wParam);
      self->CancelSecureDesktop();
      return TRUE;
    }

    case WM_QUIT:
    {
      // Special handling for self generated WM_QUIT message, which it would NEVER EVER
      // get normally as this message cancels the standard Windows Message Loop
      ASSERT(self);

      self->OnQuit();

      // Don't need it any more
      self = NULL;

      return TRUE;
    }  // WM_QUIT
  }
  return self->DialogProc(hwndDlg, uMsg, wParam, lParam);
}

INT_PTR CSDThread::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  /**
    MS documentation is contradictory.

    DialogProc documentation http://msdn.microsoft.com/en-gb/library/windows/desktop/ms645469(v=vs.85).aspx states:
      Typically, the dialog box procedure should return TRUE if it processed the message and
      FALSE if it did not. If the dialog box procedure returns FALSE, the dialog manager
      performs the default dialog operation in response to the message.

    However, individual Windows Message documentation often state something different and
    we have used the above rules except for the special cases below.

    WM_CTLCOLORSTATIC (special case)
      If an application processes this message, the return value is a handle to a brush
      that the system uses to paint the background of the static control (cast to INT_PTR)
      otherwise return FALSE.
    PWS_MSG_INSERTBUFFER
      Set return code to caller via: SetWindowLong (hwndDlg, DWL_MSGRESULT, lResult);
      Return TRUE if it processed the message and FALSE if it did not.
    PWS_MSG_RESETTIMER
     Set return code to caller via: SetWindowLong (hwndDlg, DWL_MSGRESULT, lResult);
     Return TRUE if it processed the message and FALSE if it did not.

  **/

  switch (uMsg) {
    case WM_SHOWWINDOW:
    {
      m_bMPWindowBeingShown = (BOOL)wParam == TRUE;
      return TRUE;  // Processed!
    }

    case WM_COMMAND:
    {
      const int iControlID = LOWORD(wParam);
      const int iNotificationCode = HIWORD(wParam);

      // lParam == handle to the control window
      switch (iControlID) {
      case IDC_VKB:
        if (iNotificationCode == BN_CLICKED) {
          OnVirtualKeyboard();
          return TRUE;  // Processed
        }
        break;

      case IDC_PASSKEY:
      case IDC_NEWPASSKEY:
      case IDC_VERIFY:
      case IDC_CONFIRMNEW:
        if (iNotificationCode == EN_SETFOCUS) {
          // Remember last edit control as we need to know where to insert characters
          // if the user uses the Virtual Keyboard
          m_iLastFocus = iControlID;
          return TRUE;  // Processed
        }
        if (iNotificationCode == EN_CHANGE) {
          // Reset timer start time
          ResetTimer();
          return TRUE;  // Processed
        }
        break;

      case IDC_YUBIKEY_BTN:
        if (iNotificationCode == BN_CLICKED) {
          HWND hwndPassKey = GetDlgItem(m_hwndDlg, IDC_PASSKEY);
          const StringX sxPassKey = GetControlText(hwndPassKey);
          m_passkeyID = IDC_PASSKEY;
          yubiRequestHMACSha1(sxPassKey.c_str());
          return TRUE; // Processed
        }
        break;

      case IDC_YUBIKEY2_BTN: // in Change Combination, this is the new
        if (iNotificationCode == BN_CLICKED) {
          HWND hwndNewPassKey = GetDlgItem(m_hwndDlg, IDC_NEWPASSKEY);
          const StringX sxPassKey = GetControlText(hwndNewPassKey);
          m_passkeyID = IDC_NEWPASSKEY;
          yubiRequestHMACSha1(sxPassKey.c_str());
          return TRUE; // Processed
        }
        break;

      case IDOK:
        if (iNotificationCode == BN_CLICKED) {
          OnOK();
          return TRUE;  // Processed
        }
        break;

      case IDCANCEL:
        if (iNotificationCode == BN_CLICKED) {
          OnCancel();
          return TRUE;  // Processed
        }
        break;

      case IDC_SD_TOGGLE:
        if (iNotificationCode == BN_CLICKED) {
          // Clear user input
          m_pGMP->clear();

          // Tell caller to toggle SD
          PostQuitMessage(INT_MAX);
          m_dwRC = INT_MAX;
          return TRUE; // Processed
        }
        break;
      }  // switch (iControlID)
      break;
    }  // WM_COMMAND

    case WM_DRAWITEM:
    {
      if (wParam == IDC_SD_TOGGLE) {
        // Draw Secure Desktop toggle bitmap with transparency
        DRAWITEMSTRUCT *pDrawItemStruct = (DRAWITEMSTRUCT *)lParam;
        CDC dc;
        dc.Attach(pDrawItemStruct->hDC);

        CBitmap bmp;
        bmp.LoadBitmap(m_IDB);

        BITMAP bitMapInfo;
        bmp.GetBitmap(&bitMapInfo);

        CDC memDC;
        memDC.CreateCompatibleDC(&dc);

        memDC.SelectObject(&bmp);
        int bmw = bitMapInfo.bmWidth;
        int bmh = bitMapInfo.bmHeight;

        // Draw button image transparently
        ::TransparentBlt(dc.GetSafeHdc(), 0, 0, bmw, bmh, memDC.GetSafeHdc(), 0, 0, bmw, bmh, m_cfMask);
        return TRUE;  // Processed
      } else {
        return FALSE;  // Not processed
      }
    }  // WM_DRAWITEM

    case WM_CTLCOLORSTATIC:
    {
      if (!IsWindowEnabled(hwndDlg))
        return FALSE;  // Not processed

      // Red text for Timer static controls - not yet working as text is overwritten
      switch (GetWindowLong((HWND)lParam, GWL_ID))
      {
        case IDC_STATIC_TIMER:
        case IDC_STATIC_TIMERTEXT:
        case IDC_STATIC_SECONDS:
          if (IsWindowVisible((HWND)lParam)) {
            SetTextColor((HDC)wParam, RGB(255, 0, 0));
            SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
            return (INT_PTR)(HBRUSH)GetStockObject(HOLLOW_BRUSH);
          }
      } // switch on control ID

      return FALSE;  // Not processed
    }  // WM_CTLCOLORSTATIC

    case PWS_MSG_INSERTBUFFER:
    {
      OnInsertBuffer();
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
      return TRUE; // Processed
    }  // PWS_MSG_INSERTBUFFER

    case PWS_MSG_RESETTIMER:
    {
      ResetTimer();
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
      return TRUE; // Processed
    }  // PWS_MSG_RESETTIMER:
  }  // switch (uMsg)

  // Anything else is "not processed"
  return FALSE;
}

void CSDThread::OnInitDialog()
{
  BOOL brc;
  DWORD dwError;

  m_hwndStaticTimer = GetDlgItem(m_hwndDlg, IDC_STATIC_TIMER);
  m_hwndStaticTimerText = GetDlgItem(m_hwndDlg, IDC_STATIC_TIMERTEXT);
  m_hwndStaticSeconds = GetDlgItem(m_hwndDlg, IDC_STATIC_SECONDS);

  int iMinutes = m_iUserTimeLimit / 60;
  int iSeconds = m_iUserTimeLimit - (60 * iMinutes);
  stringT sTime;
  Format(sTime, _T("%02d:%02d"), iMinutes, iSeconds);
  SetWindowText(m_hwndStaticTimer, sTime.c_str());

  m_yubiLogo.LoadBitmap(IDB_YUBI_LOGO);
  m_yubiLogoDisabled.LoadBitmap(IDB_YUBI_LOGO_DIS);
  HWND hwndYprog = GetDlgItem(m_hwndDlg, IDC_YUBI_PROGRESS);
  if (hwndYprog != NULL)
    SendMessage(hwndYprog, PBM_SETRANGE, 0, MAKELPARAM(0, 15));
  YubiControlsUpdate(IsYubiInserted());

  // Secure Desktop toggle button image transparent mask
  m_cfMask = RGB(255, 255, 255);

  if (m_bUseSecureDesktop) {
    // Seure Desktop toggle bitmap
    m_IDB = IDB_USING_SD;

    // Set up timer - fires every 100 milliseconds
    brc = CreateTimerQueueTimer(&(m_hTimer), NULL, (WAITORTIMERCALLBACK)TimerProc,
      this, 0, 100, 0);

    if (brc == 0) {
      dwError = pws_os::IssueError(_T("CreateTimerQueueTimer"), false);
      ASSERT(brc);
    }

    // Get start time in milliseconds
    iStartTime = GetTickCount();
  } else {
    // Seure Desktop toggle bitmap
    m_IDB = IDB_NOT_USING_SD;

    // Not using Secure Desktop - hide timer
    ShowWindow(m_hwndStaticTimer, SW_HIDE);
    ShowWindow(m_hwndStaticTimerText, SW_HIDE);
    ShowWindow(m_hwndStaticSeconds, SW_HIDE);
  }

  // Create the tooltip
  m_hwndTooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
    WS_POPUP | WS_EX_TOOLWINDOW | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    m_hwndDlg, NULL, GetModuleHandle(NULL), NULL);

  if (!m_hwndTooltip)
    ASSERT(0);

  SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);

  //int iTime = SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_AUTOPOP, NULL);
  SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 1000);       // Default  500 ms
  SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 5000);       // Default 5000 ms
  SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_RESHOW, 1000);       // Default  100 ms

  AddTooltip(IDC_SD_TOGGLE, IDS_TOGGLE_SECURE_DESKTOP_ON);

  // Activate tooltips
  SendMessage(m_hwndTooltip, TTM_ACTIVATE, TRUE, NULL);

  // Centre in monitor having previous dialog
  // Get current Monitor information
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(m_hCurrentMonitor, &mi);

  // Get Window rectangle
  CRect wRect;
  GetWindowRect(m_hwndDlg, &wRect);

  // Get windows width/height
  int wWidth = wRect.right - wRect.left;
  int wHeight = wRect.bottom - wRect.top;

  // Centre it on the current monitor
  int wLeft = mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left - wWidth) / 2;
  int wTop = mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top - wHeight) / 2;

  SetWindowPos(m_hwndDlg, HWND_TOP, wLeft, wTop, 0, 0, SWP_NOSIZE);

  // Tell TimerProc to do its thing
  m_bDoTimerProcAction = true;
}

void CSDThread::OnVirtualKeyboard()
{
  DWORD dwError;

  // Shouldn't be here if couldn't load DLL. Static control disabled/hidden
  if (!CVKeyBoardDlg::IsOSKAvailable())
    return;

  // Reset timer start time
  ResetTimer();

  if (m_hwndVKeyBoard != NULL && IsWindowVisible(m_hwndVKeyBoard)) {
    // Already there - move to top and enable it
    SetWindowPos(m_hwndVKeyBoard, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    EnableWindow(m_hwndVKeyBoard, TRUE);
    return;
  }

  // If not already created - do it, otherwise just reset it
  if (m_hwndVKeyBoard == NULL) {
    StringX cs_LUKBD = PWSprefs::GetInstance()->GetPref(PWSprefs::LastUsedKeyboard);
    m_hwndVKeyBoard = CreateDialogParam(m_hInstResDLL, MAKEINTRESOURCE(IDD_SDVKEYBOARD),
      m_hwndMasterPhraseDlg, (DLGPROC)(m_pVKeyBoardDlg->VKDialogProc), (LPARAM)(m_pVKeyBoardDlg));

    if (m_hwndVKeyBoard == NULL) {
      dwError = pws_os::IssueError(_T("CreateDialogParam - IDD_SDVKEYBOARD"), false);
      ASSERT(m_hwndVKeyBoard);
    }
  } else {
    m_pVKeyBoardDlg->ResetKeyboard();
  }

  // Now show it and make it top & enable it
  SetWindowPos(m_hwndVKeyBoard, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
  EnableWindow(m_hwndVKeyBoard, TRUE);
}

void CSDThread::OnOK()
{
  DWORD dwError;
  stringT sErrorMsg;

  /*
  self->m_wDialogID

  IDD_SDGETPHRASE      IDC_PASSKEY, IDC_VKB, IDC_YUBIKEY_BTN, IDOK, IDCANCEL
  IDD_SDKEYCHANGE      IDC_PASSKEY, IDC_NEWPASSKEY, IDC_CONFIRMNEW, IDC_VKB, IDC_YUBIKEY_BTN[2], IDOK, IDCANCEL
  IDD_SDPASSKEYSETUP   IDC_PASSKEY, IDC_VERIFY, IDC_VKB, IDC_YUBIKEY_BTN, IDOK, IDCANCEL
  */

  StringX sxPassKey, sxNewPassKey1, sxNewPassKey2, sxVerifyPassKey;

  HWND hwndPassKey = GetDlgItem(m_hwndDlg, IDC_PASSKEY);

  // bPhraseEntered will be set here IFF we were called by yubiProcessCompleted
  // in which case the text is the response from the YubiKey
  if (m_yubiResp[0].empty())
    sxPassKey = GetControlText(hwndPassKey);
  else
    sxPassKey = m_yubiResp[0];

  if (!sxPassKey.empty()) {
    m_pGMP->sPhrase = sxPassKey;
    m_pGMP->bPhraseEntered = true;
  } else if (m_yubiResp[1].empty()) { // not an error if yubikey password change
    RDLL_LoadAString(m_hInstResDLL, sErrorMsg,
                m_wDialogID == IDD_SDPASSKEYSETUP ? IDS_ENTERKEYANDVERIFY : IDS_CANNOTBEBLANK);
    MessageBox(m_hwndDlg, sErrorMsg.c_str(), NULL, MB_OK);
    SetFocus(hwndPassKey);
    return;
  }

  switch (m_wDialogID) {
    case IDD_SDGETPHRASE:
    {
      // Just verify IDC_PASSKEY - already done before switch statement
      break;
    }
    case IDD_SDKEYCHANGE:
    {
      // Verify DC_PASSKEY, IDC_NEWPASSKEY, IDC_CONFIRMNEW
      UINT iMsgID(0);
      int rc = app.GetCore()->CheckPasskey(app.GetCore()->GetCurFile(), sxPassKey);

      HWND hwndFocus = hwndPassKey;

      if (rc == PWScore::WRONG_PASSWORD)
        iMsgID = IDS_WRONGOLDPHRASE;
      else if (rc == PWScore::CANT_OPEN_FILE)
        iMsgID = IDS_CANTVERIFY;
      else { // old passphrase verified, check new
        HWND hwndNewPassKey1 = GetDlgItem(m_hwndDlg, IDC_NEWPASSKEY);
        sxNewPassKey1 = GetControlText(hwndNewPassKey1);

        HWND hwndNewPassKey2 = GetDlgItem(m_hwndDlg, IDC_CONFIRMNEW);
        sxNewPassKey2 = GetControlText(hwndNewPassKey2);

        if (sxNewPassKey1.empty() && m_yubiResp[1].empty()) {
          iMsgID = IDS_CANNOTBEBLANK;
          hwndFocus = hwndNewPassKey1;
        }
        else if (sxNewPassKey1 != sxNewPassKey2) {
          iMsgID = IDS_NEWOLDDONOTMATCH;
          hwndFocus = hwndNewPassKey2;
        }
      }

      if (iMsgID != 0) {
        RDLL_LoadAString(m_hInstResDLL, sErrorMsg, iMsgID);
        MessageBox(m_hwndDlg, sErrorMsg.c_str(), NULL, MB_OK | MB_ICONSTOP);
        SetFocus(hwndFocus);
        return;
      }

      StringX sxErrorMsg;
      if (m_yubiResp[1].empty() && !CPasswordCharPool::CheckPassword(sxNewPassKey1, sxErrorMsg)) {
        stringT sMsg, sText;
        Format(sMsg, IDS_WEAKPASSPHRASE, sxErrorMsg.c_str());

#ifndef PWS_FORCE_STRONG_PASSPHRASE
        RDLL_LoadAString(m_hInstResDLL, sText, IDS_USEITANYWAY);
        sMsg += sText;
        INT_PTR rc = MessageBox(m_hwndDlg, sMsg.c_str(), NULL, MB_YESNO | MB_ICONSTOP);
        if (rc == IDNO)
          return;
#else
        RDLL_LoadAString(m_hInstResDLL, sText, IDS_TRYANOTHER);
        sMsg += sText;
        MessageBox(m_hwndDlg, sMsg.c_str(), NULL, MB_OK | MB_ICONSTOP);
        return;
#endif  // PWS_FORCE_STRONG_PASSPHRASE
      }
      if (m_yubiResp[1].empty())
        m_pGMP->sNewPhrase = sxNewPassKey1;
      else
        m_pGMP->sNewPhrase = m_yubiResp[1];
      m_pGMP->bNewPhraseEntered = true;
      break;
    }
    case IDD_SDPASSKEYSETUP:
    {
      // Verify IDC_PASSKEY, IDC_VERIFY
      UINT iMsgID(0);
      HWND hwndFocus = hwndPassKey;
      // sxNewPassKey may be from Yubi, so we get the control text again:
      const StringX sxNewPassKey0 = GetControlText(hwndPassKey);
      HWND hwndNewPassKey1 = GetDlgItem(m_hwndDlg, IDC_VERIFY);
      sxNewPassKey1 = GetControlText(hwndNewPassKey1);

      if (sxNewPassKey0 != sxNewPassKey1) {
        iMsgID = IDS_ENTRIESDONOTMATCH;
        hwndFocus = hwndNewPassKey1;
      }

      if (iMsgID != 0) {
        RDLL_LoadAString(m_hInstResDLL, sErrorMsg, iMsgID);
        MessageBox(m_hwndDlg, sErrorMsg.c_str(), NULL, MB_OK | MB_ICONSTOP);
        SetFocus(hwndFocus);
        return;
      }

      StringX sxErrorMsg;
      if (m_yubiResp[0].empty() && !CPasswordCharPool::CheckPassword(sxNewPassKey0, sxErrorMsg)) {
        stringT sMsg, sText;
        Format(sMsg, IDS_WEAKPASSPHRASE, sErrorMsg.c_str());

#ifndef PWS_FORCE_STRONG_PASSPHRASE
        RDLL_LoadAString(m_hInstResDLL, sText, IDS_USEITANYWAY);
        sMsg += sText;
        INT_PTR rc = MessageBox(m_hwndDlg, sMsg.c_str(), NULL, MB_YESNO | MB_ICONSTOP);
        if (rc == IDNO)
          return;
#else
        RDLL_LoadAString(m_hInstResDLL, sText, IDS_TRYANOTHER);
        sMsg += sText;
        MessageBox(m_hwndDlg, sMsg.c_str(), NULL, MB_OK | MB_ICONSTOP);
        return;
#endif  // PWS_FORCE_STRONG_PASSPHRASE
      }
      if (m_yubiResp[1].empty())
        m_pGMP->sNewPhrase = sxNewPassKey1;
      else
        m_pGMP->sNewPhrase = m_yubiResp[1];
      m_pGMP->bNewPhraseEntered = true;
      break;
    }
    default:
      ASSERT(0);
  }  // switch m_wDialogID

  // Tell TimerProc to do nothing
  m_bDoTimerProcAction = false;

  if (m_hwndVKeyBoard != NULL) {
    ::SendMessage(m_hwndVKeyBoard, WM_QUIT, 0, 0);
    if (!DestroyWindow(m_hwndVKeyBoard)) {
      dwError = pws_os::IssueError(_T("DestroyWindow - IDD_SDVKEYBOARD - IDOK"), false);
      ASSERT(0);
    }

    m_hwndVKeyBoard = NULL;
  }

  PostQuitMessage(IDOK);
  m_dwRC = IDOK;
}

void CSDThread::OnCancel()
{
  DWORD dwError;

  // Tell TimerProc to do nothing
  m_bDoTimerProcAction = false;

  // Clear user input
  m_pGMP->clear();

  if (m_hwndVKeyBoard != NULL) {
    ::SendMessage(m_hwndVKeyBoard, WM_QUIT, 0, 0);
    if (!DestroyWindow(m_hwndVKeyBoard)) {
      dwError = pws_os::IssueError(_T("DestroyWindow - IDD_SDVKEYBOARD - IDCANCEL"), false);
      ASSERT(0);
    }

    m_hwndVKeyBoard = NULL;
  }

  PostQuitMessage(IDCANCEL);
  m_dwRC = IDCANCEL;
}

void CSDThread::OnQuit()
{
  DWORD dwError;

  if (m_hwndVKeyBoard != NULL) {
    ::SendMessage(m_hwndVKeyBoard, WM_QUIT, 0, 0);
    if (!DestroyWindow(m_hwndVKeyBoard)) {
      dwError = pws_os::IssueError(_T("DestroyWindow - IDD_SDVKEYBOARD - WM_QUIT"), false);
      ASSERT(0);
    }

    m_hwndVKeyBoard = NULL;
  }

  // Delete timer (only if set)
  if (m_hTimer != NULL) {
    // Tell TimerProc to do nothing
    m_bDoTimerProcAction = false;

    // Create an event for timer deletion
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
      dwError = pws_os::IssueError(_T("CreateEvent in MPDialogProc"), false);
      ASSERT(hEvent);
    }

    // Delete all timers in the timer queue
    BOOL brc;
    do {
      brc = DeleteTimerQueueTimer(NULL, m_hTimer, hEvent);
      if (brc == 0) {
        // No need to call again if error code is ERROR_IO_PENDING.
        // Note description of ERROR_IO_PENDING is "Overlapped I/O operation is in progress"
        if (GetLastError() == ERROR_IO_PENDING)
          break;

        // Otherwise debug write out other error messages and try again
        dwError = pws_os::IssueError(_T("DeleteTimerQueueTimer"), false);
      }
    } while (brc == 0);

    // Wait for timer queue to go
    WaitForSingleObject(hEvent, INFINITE);

    // Close the handle - NOT the timer handles
    CloseHandle(hEvent);
  }
}

void CSDThread::OnInsertBuffer()
{
  // Get the buffer
  StringX vkbuffer = m_pVKeyBoardDlg->GetPassphrase();

  // Find the selected characters - if any
  DWORD nStartChar, nEndChar;
  HWND hedtPhrase = GetDlgItem(m_hwndDlg, m_iLastFocus);

  SendMessage(hedtPhrase, EM_GETSEL, (WPARAM)&nStartChar, (LPARAM)&nEndChar);

  // Replace them or, if none selected, put at current cursor position
  SendMessage(hedtPhrase, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)(LPCWSTR)vkbuffer.c_str());

  // Put cursor at end of inserted text
  SendMessage(hedtPhrase, EM_SETSEL, nStartChar + vkbuffer.length(), nStartChar + vkbuffer.length());
}

void CALLBACK CSDThread::TimerProc(LPVOID lpParameter, BOOLEAN /* TimerOrWaitFired */)
{
  CSDThread *self = (CSDThread *)lpParameter;

  // Don't do anything if closing down
  if (!self->m_bDoTimerProcAction)
    return;

  // Don't do anything if windows aren't visible
  if (!self->m_bMPWindowBeingShown && !self->m_bVKWindowBeingShown)
    return;

  // Do Yubi polling
  self->YubiPoll(); // Mixins rock!

  // Get time left in seconds
  int iTimeLeft = self->m_iUserTimeLimit - (GetTickCount() - iStartTime) / 1000;

  int iShow = (iTimeLeft <= self->m_iUserTimeLimit / 4) ? SW_SHOW : SW_HIDE;

  if (self->m_bMPWindowBeingShown || IsWindowVisible(self->m_hwndMasterPhraseDlg)) {
    ShowWindow(self->m_hwndStaticTimer, iShow);
    ShowWindow(self->m_hwndStaticTimerText, iShow);
    ShowWindow(self->m_hwndStaticSeconds, iShow);
  }

  if (self->m_bVKWindowBeingShown || IsWindowVisible(self->m_hwndVKeyBoard)) {
    ShowWindow(self->m_pVKeyBoardDlg->m_hwndVKStaticTimer, iShow);
    ShowWindow(self->m_pVKeyBoardDlg->m_hwndVKStaticTimerText, iShow);
    ShowWindow(self->m_pVKeyBoardDlg->m_hwndVKStaticSeconds, iShow);
  }

  if (iShow == SW_HIDE)
    return;

  int iMinutes = iTimeLeft / 60;
  int iSeconds = iTimeLeft - (60 * iMinutes);
  if (self->m_iMinutes != iMinutes || self->m_iSeconds != iSeconds) {
    stringT sTime;
    Format(sTime, _T("%02d:%02d"), iMinutes, iSeconds);

    if (self->m_bMPWindowBeingShown || IsWindowVisible(self->m_hwndMasterPhraseDlg)) {
      SetWindowText(self->m_hwndStaticTimer, sTime.c_str());
    }

    if (self->m_bVKWindowBeingShown || IsWindowVisible(self->m_hwndVKeyBoard)) {
      SetWindowText(self->m_pVKeyBoardDlg->m_hwndVKStaticTimer, sTime.c_str());
    }

    self->m_iMinutes = iMinutes;
    self->m_iSeconds = iSeconds;
  }
}

void CSDThread::ResetTimer()
{
  LARGE_INTEGER liDueTime;
  DWORD dwError;

  if (m_hWaitableTimer == NULL)
    return;

  // Now reset it - calling SetWaitableTimer, stops and then restarts it
  int iUserTimeLimit = PWSprefs::GetInstance()->GetPref(PWSprefs::SecureDesktopTimeout);
  liDueTime.QuadPart = -(iUserTimeLimit * 10000000);

  if (!SetWaitableTimer(m_hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)) {
    dwError = pws_os::IssueError(_T("SetWaitableTimer"), false);
    ASSERT(0);
  }

  // Reset tick count for static text display
  iStartTime = GetTickCount();
}

// AddTooltip is a modified form from MSDN: http://msdn.microsoft.com/en-us/library/bb760252(v=vs.85).aspx
BOOL CSDThread::AddTooltip(UINT uiControlID, stringT sText)
{
  if (!uiControlID || sText.empty())
    return FALSE;

  // Get the window of the tool.
  HWND hwndTool = GetDlgItem(m_hwndDlg, uiControlID);

  // Associate the tooltip with the tool.
  TOOLINFO ti = { 0 };
  ti.cbSize = sizeof(ti);
  ti.hwnd = m_hwndDlg;
  ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_CENTERTIP | TTF_TRANSPARENT;
  ti.uId = (UINT_PTR)hwndTool;
  ti.lpszText = (LPWSTR)sText.c_str();

  return (BOOL)SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);//TTM_ADDTOOL returns BOOL
}

BOOL CSDThread::AddTooltip(UINT uiControlID, UINT uiToolString, UINT uiFormat)
{
  if (!uiControlID || !uiToolString)
    return FALSE;

  stringT sText;
  RDLL_LoadAString(m_hInstResDLL, sText, uiToolString);
  if (sText.empty())
    return FALSE;

  if (uiFormat != NULL) {
    Format(sText, uiFormat, sText.c_str());
  }

  return AddTooltip(uiControlID, sText);
}

void CSDThread::GetMonitorImages()
{
  HDC hDesktopDC = ::GetDC(NULL);
  EnumDisplayMonitors(hDesktopDC, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(this));

  // Cleanup GDI resources
  ::ReleaseDC(NULL, hDesktopDC);
}

BOOL CALLBACK CSDThread::MonitorEnumProc(HMONITOR /*hMonitor*/, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
  CSDThread *self = (CSDThread *)dwData;

  HDC hdcCapture(0);
  HBITMAP hbmDimmendMonitorImage;

  // Get monitor information
  const int nScreenWidth = GetDeviceCaps(hdcMonitor, HORZRES);
  const int nScreenHeight = GetDeviceCaps(hdcMonitor, VERTRES);

  self->CaptureMonitorImage(hdcMonitor, hdcCapture, hbmDimmendMonitorImage,
    lprcMonitor->left, lprcMonitor->top, nScreenWidth, nScreenHeight);

  // Save new monitor HDC (hdcCapture) with its Bitmap & position/size
  st_MonitorImageInfo st_mi;
  st_mi.hdcMonitor = hdcCapture;
  st_mi.left = lprcMonitor->left;
  st_mi.top = lprcMonitor->top;
  st_mi.width = nScreenWidth;
  st_mi.height = nScreenHeight;
  st_mi.hbmDimmendMonitorImage = hbmDimmendMonitorImage;
  self->m_vMonitorImageInfo.push_back(st_mi);

  return TRUE;
}

void CSDThread::CaptureMonitorImage(HDC &hdcMonitor, HDC &hdcCapture, HBITMAP &hbmDimmendMonitorImage,
  const int left, const int top, const int nScreenWidth, const int nScreenHeight)
{
  // Create a memory DC compatible with the monitor
  hdcCapture = CreateCompatibleDC(hdcMonitor);

  // Create a bitmap compatible with the monitor and of the correct size
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcMonitor, nScreenWidth, nScreenHeight);

  // Select it into the specified DC
  HGDIOBJ hOldBitmap = SelectObject(hdcCapture, hBitmap);

  // Bit-Blit the contents of the Desktop DC into the created compatible DC
  // Only time the monitor's position is needed (left, top)
  BitBlt(hdcCapture, 0, 0, nScreenWidth, nScreenHeight, hdcMonitor, left, top, SRCCOPY | CAPTUREBLT);

  // Alpha-blend with black rectangle and PWS image
  DimMonitorImage(hdcMonitor, hdcCapture, hbmDimmendMonitorImage);

  // Cleanup GDI resources
  ::DeleteObject(hBitmap);
  ::DeleteObject(hOldBitmap);
}

void CSDThread::DimMonitorImage(HDC &hdcMonitor, HDC &hdcCapture, HBITMAP &hbmDimmendMonitorImage)
{
  /*
    Create a dimmed image of what is currently displayed on this monitor
    Overlay this with a blended black rectangle and tiled PWS motif bitmap

    Note: All offsets are (0,0) irrespective of the monitor's relative position in the virtual screen.
  */

  const int nScreenWidth = GetDeviceCaps(hdcMonitor, HORZRES);
  const int nScreenHeight = GetDeviceCaps(hdcMonitor, VERTRES);

  // Create final dimmed screen memory DC
  HDC hdcDimmedScreen = CreateCompatibleDC(hdcCapture);

  // Create the final bitmap in that DC
  hbmDimmendMonitorImage = CreateCompatibleBitmap(hdcCapture, nScreenWidth, nScreenHeight);
  HGDIOBJ hbmOldDimmendMonitorImage = SelectObject(hdcDimmedScreen, hbmDimmendMonitorImage);

  // Copy this monitor's screen image here
  BitBlt(hdcDimmedScreen, 0, 0, nScreenWidth, nScreenHeight, hdcCapture, 0, 0, SRCCOPY);

  // Create memory DC for black rectangle
  HDC hdcRectangle = CreateCompatibleDC(hdcCapture);

  // Create a bitmap in that DC
  HBITMAP hbmRectangle = CreateCompatibleBitmap(hdcCapture, nScreenWidth, nScreenHeight);
  HGDIOBJ hbmOldRectangle = SelectObject(hdcRectangle, hbmRectangle);

  // Fill bitmap with a black rectangle
  RECT rcFill = {0, 0, nScreenWidth, nScreenHeight};
  HBRUSH hbrBlack = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hdcRectangle, &rcFill, hbrBlack);

  // Load the PWS Logo bitmap
  HBITMAP hbmPWSLogo = LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_PWSBITMAP));
  BITMAP bmPWSLogo;
  GetObject(hbmPWSLogo, sizeof(BITMAP), &bmPWSLogo);
  const int bm_width = bmPWSLogo.bmWidth;
  const int bm_height = bmPWSLogo.bmHeight;

  // Create another memory DC for this bitmap and select the PWS bitmap
  HDC hdcPWSLogo = CreateCompatibleDC(hdcCapture);
  HGDIOBJ hbmOldPWSLogo = SelectObject(hdcPWSLogo, hbmPWSLogo);

  // Create another memory DC for the tiled bitmap
  HDC hdcTiledPWSLogo = CreateCompatibleDC(hdcCapture);

  // Create a bitmap in the tiled DC
  HBITMAP hbmpTiledPWSLogo = CreateCompatibleBitmap(hdcCapture, nScreenWidth, nScreenHeight);
  HGDIOBJ hbmOldTiledPWSLogo = SelectObject(hdcTiledPWSLogo, hbmpTiledPWSLogo);

  // Tile the PWS bitmap over the new bitmap
  for (int y = 0; y < nScreenHeight; y += bm_height) {
    for (int x = 0; x < nScreenWidth; x += bm_width) {
      BitBlt(hdcTiledPWSLogo, x, y, nScreenWidth, nScreenHeight, hdcPWSLogo, 0, 0, SRCCOPY);
    }
  }

  // Required for the following 2 alpha blends
  BLENDFUNCTION bf;
  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = 127;
  bf.AlphaFormat = 0;

  // Alpha blend the tiled PWS bitmap onto the black rectangle
  AlphaBlend(hdcRectangle, 0, 0, nScreenWidth, nScreenHeight,
    hdcTiledPWSLogo, 0, 0, nScreenWidth, nScreenHeight, bf);

  // Alpha blend(combine) the image containing the background and the rectangle + tiled image
  AlphaBlend(hdcDimmedScreen, 0, 0, nScreenWidth, nScreenHeight,
    hdcRectangle, 0, 0, nScreenWidth, nScreenHeight, bf);

  // Tidy up graphics resources
  // Reset everything first
  ::SelectObject(hdcRectangle, hbmOldRectangle);
  ::SelectObject(hdcPWSLogo, hbmOldPWSLogo);
  ::SelectObject(hdcTiledPWSLogo, hbmOldTiledPWSLogo);
  ::SelectObject(hdcDimmedScreen, hbmOldDimmendMonitorImage);

  // Now delete created GDI objects
  ::DeleteObject(hbrBlack);
  ::DeleteObject(hbmPWSLogo);

  ::DeleteDC(hdcRectangle);
  ::DeleteDC(hdcPWSLogo);
  ::DeleteDC(hdcTiledPWSLogo);
  ::DeleteDC(hdcDimmedScreen);
}

bool CSDThread::GetLogonSID(PSID &logonSID)
{
  HANDLE hToken = NULL;
  bool res = false;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)){
    pws_os::IssueError(_T("OpenProcessToken"), false);
    return false;
  }

  PTOKEN_GROUPS pTG = NULL;
  DWORD dwBytesNeeed = 0;
  DWORD dwError;

  // get buffer size
  if (!GetTokenInformation(hToken, TokenGroups, pTG, 0,  &dwBytesNeeed)){
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER){
      goto Cleanup;
    }
  }

  pTG = (PTOKEN_GROUPS)LocalAlloc(LPTR, dwBytesNeeed);

  if (!pTG) {
    dwError = pws_os::IssueError(_T("LocalAlloc"), false);
    goto Cleanup;
  }


  if (!GetTokenInformation(hToken, TokenGroups, pTG, dwBytesNeeed, &dwBytesNeeed)){
    dwError = pws_os::IssueError(_T("GetTokenInformation"), false);
    goto Cleanup;
  }

  for (DWORD i = 0; i < pTG->GroupCount; i++) {
    if ((pTG->Groups[i].Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID) {
      dwBytesNeeed = GetLengthSid(pTG->Groups[i].Sid);
      logonSID = (PSID)LocalAlloc(LPTR, dwBytesNeeed);
      if (!logonSID) {
        dwError = pws_os::IssueError(_T("LocalAlloc"), false);
        break;
      }
      if (!CopySid(dwBytesNeeed, logonSID, pTG->Groups[i].Sid)) {
        dwError = pws_os::IssueError(_T("CopySid"), false);
        break;
      }
      res = true;
      break;
    }
  }

Cleanup:
  if (pTG)
    LocalFree(pTG);

  CloseHandle(hToken);
  return res;
}

bool CSDThread::CreateSA(SECURITY_ATTRIBUTES &sa, DWORD dwAccessMask, PSECURITY_DESCRIPTOR &pSD, PACL &pACL,
  PSID &pOwnerSID, PSID &pCurrentUserSID)
{
  DWORD dwResult, dwError;
  /* We need to forbid WRITE_DAC and WRITE_OWNER to prevent DAC changes by malicious process*/
  DWORD dwForbidden = WRITE_DAC | WRITE_OWNER;

  EXPLICIT_ACCESS ea[2];
  SID_IDENTIFIER_AUTHORITY SIDAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;
  // Create a SID for CREATOR_OWNER_RIGHTS (don't confuse with CREATOR_OWNER!)
  if (!AllocateAndInitializeSid(&SIDAuthCreator, 1, SECURITY_CREATOR_OWNER_RIGHTS_RID, 0, 0, 0, 0, 0, 0, 0, &pOwnerSID)) {
    dwError = pws_os::IssueError(L"AllocateAndInitializeSid - CREATOR_OWNER_RIGHTS", false);
    return false;
  }
  if (!GetLogonSID(pCurrentUserSID)) {
    ASSERT(0);
    return false;
  }

  // Exclude forbidden rights from mask
  if (dwAccessMask & dwForbidden){
    dwAccessMask &= ~dwForbidden;
    pws_os::Trace(_T("Forbidden rights were removed from access mask\n"));
  }

  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  ZeroMemory(&ea, sizeof(ea));

  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  // The ACE will allow the current user Desktop specific access rights and
  // standard access to the Desktop.

  ea[0].grfAccessPermissions = dwAccessMask;
  ea[0].grfAccessMode = SET_ACCESS;
  ea[0].grfInheritance = NO_INHERITANCE;
  ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
  ea[0].Trustee.ptstrName = (LPTSTR)pCurrentUserSID;

  // We need to explicitely forbid it for owner, otherwise it will have them implicitly
  ea[1].grfAccessPermissions = dwForbidden;
  ea[1].grfAccessMode = DENY_ACCESS;
  ea[1].grfInheritance = NO_INHERITANCE;
  ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
  ea[1].Trustee.ptstrName = (LPTSTR)pOwnerSID;

  // Create a new ACL that contains the new ACEs.
  dwResult = SetEntriesInAcl(sizeof(ea) / sizeof(ea[0]), ea, NULL, &pACL);
  if (dwResult != ERROR_SUCCESS) {
    SetLastError(dwResult); // to be caught by GetLastError()
    dwError = pws_os::IssueError(_T("SetEntriesInAcl"), false);
    return false;
  }

  // Get a security descriptor.
  pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
  if (pSD == NULL) {
    dwError = pws_os::IssueError(_T("LocalAlloc"), false);
    return false;
  }

  // Initialise it
  if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
    dwError = pws_os::IssueError(_T("InitializeSecurityDescriptor"), false);
    return false;
  }

  // Add the ACL to the security descriptor.
  if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
    dwError = pws_os::IssueError(_T("SetSecurityDescriptorDacl"), false);
    return false;
  }

  // Initialize the security attributes structure.
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = pSD;
  sa.bInheritHandle = FALSE;

  // Set success - we can use Security Attributes
  return true;
}

void CSDThread::CancelSecureDesktop()
{
  DWORD dwError;

  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_hWaitableTimer != NULL) {
    // Set flag to stop TimerProc
    m_bDoTimerProcAction = false;

    // Cancel timer
    pws_os::Trace(_T("CancelSecureDesktop - CancelWaitableTimer\n"));
    if (!CancelWaitableTimer(m_hWaitableTimer)) {
      dwError = pws_os::IssueError(_T("CancelWaitableTimer"), false);
      ASSERT(0);
    }

    // Close the timer handle
    if (!CloseHandle(m_hWaitableTimer)) {
      pws_os::Trace(_T("CancelSecureDesktop - CloseHandle\n"));
      dwError = pws_os::IssueError(_T("CloseHandle - hWaitableTimer"), false);
      ASSERT(0);
    }

    // Clear timer handle to prevent caller trying to cancel it
    m_hWaitableTimer = NULL;

    // Reset TickCount so no messages should appear (shouldn't anyway as we have told
    // the TimerProc not do do anything and hopefully the timer has been cancelled above.
    iStartTime = GetTickCount();
  }

  // Can't reset thread desktop whilst window is still open otherwise get
  // error code 170: The requested resource is in use.
  // Have to wait for the WM_QUIT message and then for WinLogon desktop to deactivate

  // Cancel SD dialog
  OnCancel();
}

bool CSDThread::TerminateProcesses()
{
  DWORD dwError;

  // Initialise vector
  m_vPIDs.clear();

  if (!GetProcessKillList())
    return false;

  pws_os::Trace(_T("TerminateProcesses - found %d unique process%s to terminate\n"),
    m_vPIDs.size(), m_vPIDs.size() == 1 ? _T("") : _T("es"));

  // Set default return code
  bool brc = true;

  for (size_t i = 0; i < m_vPIDs.size(); i++) {
    // Get a handle to the process.  It will only return a valid handle if we do have terminate authority.
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE,
      FALSE, m_vPIDs[i]);

    // Get the process name and save PID for all ctfmon.exe processes
    if (hProcess != NULL) {
      // Terminate it!
      if (!TerminateProcess(hProcess, 0)) {
        dwError = pws_os::IssueError(_T("TerminateProcess"), false);
        brc = false;
      }
      // Release the handle to the process.
      CloseHandle(hProcess);
    }
  }
  return brc;
}

bool CSDThread::GetProcessKillList()
{
  HANDLE hProcessSnap, hThreadSnap;
  PROCESSENTRY32 pe32;
  THREADENTRY32 te32;

  std::vector<DWORD> vChildPIDs;

  DWORD dwCurrentPID = GetCurrentProcessId();

  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    pws_os::Trace(_T("CreateToolhelp32Snapshot of processes\n"));
    return false;
  }

  // Set the size of the structure before using it.
  pe32.dwSize = sizeof(PROCESSENTRY32);

  // Retrieve information about the first process
  if (!Process32First(hProcessSnap, &pe32)) {
    pws_os::Trace(_T("Process32First\n"));
    CloseHandle(hProcessSnap);
    return false;
  }

  // Now walk the snapshot of processes - only keep those that
  // have PWS as parent
  do
  {
    if (dwCurrentPID == pe32.th32ParentProcessID) {
      vChildPIDs.push_back(pe32.th32ProcessID);
    }

  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);

  // If no child processes - no need to look at threads
  if (vChildPIDs.empty())
    return false;

  // Take a snapshot of all running threads
  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap == INVALID_HANDLE_VALUE)
    return false;

  // Fill in the size of the structure before using it.
  te32.dwSize = sizeof(THREADENTRY32);

  // Retrieve information about the first thread,
  // and exit if unsuccessful
  if (!Thread32First(hThreadSnap, &te32)) {
    pws_os::IssueError(_T("Thread32First"), false);
    CloseHandle(hThreadSnap);
    return false;
  }

  // Now walk the thread list of the system looking for child processes
  // using the new Desktop
  BOOL brc = false;
  do
  {
    // Only check process ID if PWS is its parent
    if (std::find(vChildPIDs.begin(), vChildPIDs.end(), te32.th32OwnerProcessID) != vChildPIDs.end()) {
      HDESK hdesk = GetThreadDesktop(te32.th32ThreadID);
      if (hdesk != 0) {
        wchar_t buffer[1024];
        DWORD dwLengthNeeded(0);
        if (!GetUserObjectInformation(hdesk, UOI_NAME, buffer, sizeof(buffer), &dwLengthNeeded)) {
          pws_os::IssueError(_T("GetUserObjectInformation - Thread Desktop Name"), false);
          continue;
        }
        if (_wcsicmp(buffer, m_sDesktopName.c_str()) == 0) {
          m_vPIDs.push_back(te32.th32OwnerProcessID);
          pws_os::Trace(_T("Found process to terminate\n"));
          brc = true;
        }
      } else {
        pws_os::IssueError(_T("GetThreadDesktop"), false);
        pws_os::Trace(_T("GetThreadDesktop: ThreadID=%d; OwnerProcessID=%d\n"), te32.th32ThreadID, te32.th32OwnerProcessID);
      }
    }
  } while (Thread32Next(hThreadSnap, &te32));

  CloseHandle(hThreadSnap);

  // Sort vector of PIDs and ensure no duplicates
  std::sort(m_vPIDs.begin(), m_vPIDs.end());
  std::vector<DWORD>::iterator iNewEnd = std::unique(m_vPIDs.begin(), m_vPIDs.end());
  m_vPIDs.erase(iNewEnd, m_vPIDs.end());

  return true;
}
