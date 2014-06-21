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
bool CPKBaseDlg::s_yubiDetected = false;

extern LRESULT CALLBACK MsgFilter(int code, WPARAM wParam, LPARAM lParam);

CPKBaseDlg::CPKBaseDlg(int id, CWnd *pParent, bool bUseSecureDesktop)
  : CPWDialog(id, pParent), m_bUseSecureDesktop(bUseSecureDesktop),
  m_passkey(L""), m_pctlPasskey(new CSecEditExtn),
m_pVKeyBoardDlg(NULL), m_pending(false), m_hwndVKeyBoard(NULL)
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

  DDX_Control(pDX, IDC_SDSWITCH, m_ctlSDToggle);

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
  ON_STN_CLICKED(IDC_SDSWITCH, OnSwitchSecureDesktop)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


bool CPKBaseDlg::IsYubiInserted() const
{
  if (m_pending)
    return true; // can't check in the middle of a request
  else {
    CSingleLock singeLock(&m_mutex);
    singeLock.Lock();
    return (m_yk.enumPorts() == 1);
  }
}

BOOL CPKBaseDlg::OnInitDialog(void)
{
  CPWDialog::OnInitDialog();

  // Setup a timer to poll the key every 250 ms
  SetTimer(1, 250, 0);

  // This bit makes the background come out right on the bitmaps
  if (m_bUseSecureDesktop)
  {
    m_ctlSDToggle.ReloadBitmap(IDB_USING_SD);
  }
  else
  {
    m_ctlSDToggle.ReloadBitmap(IDB_NOT_USING_SD);
  }

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

void CPKBaseDlg::yubiInserted(void)
{
  CButton *ybn = (CButton*)GetDlgItem(IDC_YUBIKEY_BTN);
  ybn->SetBitmap(m_yubiLogo);
  ybn->ShowWindow(SW_SHOW);
  m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
  m_yubi_status.ShowWindow(SW_SHOW);
}

void CPKBaseDlg::yubiRemoved(void)
{
  ((CButton*)GetDlgItem(IDC_YUBIKEY_BTN))->SetBitmap(m_yubiLogoDisabled);
  m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
}

static StringX Bin2Hex(const unsigned char *buf, int len)
{
  wostringstream os;
  os << setw(2);
  os << setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << hex << setw(2) << int(buf[i]);
  }
  return StringX(os.str().c_str());
}

void CPKBaseDlg::yubiCheckCompleted()
{
  // We now wait for a response with the HMAC-SHA1 digest
  BYTE respBuf[SHA1_DIGEST_SIZE];
  unsigned short timer;
  CSingleLock singeLock(&m_mutex);
  singeLock.Lock();
  YKLIB_RC rc = m_yk.waitForCompletion(YKLIB_NO_WAIT,
                                       respBuf, sizeof(respBuf), &timer);
  switch (rc) {
  case YKLIB_OK:
    m_yubi_status.ShowWindow(SW_SHOW);
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_timeout.SetPos(0);
    m_yubi_status.SetWindowText(_T(""));
    TRACE(_T("yubiCheckCompleted: YKLIB_OK"));
    m_pending = false;
    m_yk.closeKey();
    m_passkey = Bin2Hex(respBuf, SHA1_DIGEST_SIZE);
    // The returned hash is the passkey
    ProcessPhrase();
    // If we returned from above, reset status:
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
    break;

  case YKLIB_PROCESSING:  // Still processing or waiting for the result
    break;

  case YKLIB_TIMER_WAIT:  // A given number of seconds remain 
    m_yubi_timeout.SetPos(timer);
    break;

  case YKLIB_INVALID_RESPONSE:  // Invalid or no response
    m_pending = false;
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_TIMEOUT)));
    m_yubi_status.ShowWindow(SW_SHOW);
    m_yk.closeKey();
    YubiFailed(); // allow subclass to do something useful
    break;

  default:                // A non-recoverable error has occured
    m_pending = false;
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.ShowWindow(SW_SHOW);
    m_yk.closeKey();
    // Generic error message
    TRACE(_T("yubiCompleted(%d)\n"), rc);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDSC_UNKNOWN_ERROR)));
    break;
  }
}

void CPKBaseDlg::yubiRequestHMACSha1()
{
  if (m_pending) {
    // no-op if a request's already in the air
  } else {
    CSingleLock singeLock(&m_mutex);
    singeLock.Lock();
    // open key
    // if zero or >1 key, we'll fail
    if (m_yk.openKey() != YKLIB_OK) {
      return;
    }

    // Prepare the HMAC-SHA1 challenge here

    BYTE chalBuf[SHA1_MAX_BLOCK_SIZE];
    BYTE chalLength = BYTE(m_passkey.GetLength()*sizeof(TCHAR));
    memset(chalBuf, 0, SHA1_MAX_BLOCK_SIZE);
    if (chalLength > SHA1_MAX_BLOCK_SIZE)
      chalLength = SHA1_MAX_BLOCK_SIZE;

    memcpy(chalBuf, m_passkey, chalLength);

    // Initiate HMAC-SHA1 operation now

    if (m_yk.writeChallengeBegin(YKLIB_SECOND_SLOT, YKLIB_CHAL_HMAC,
                                 chalBuf, chalLength) == YKLIB_OK) {
      // request's in the air, setup GUI to wait for reply
      m_pending = true;
      m_yubi_status.ShowWindow(SW_HIDE);
      m_yubi_status.SetWindowText(_T(""));
      m_yubi_timeout.ShowWindow(SW_SHOW);
      m_yubi_timeout.SetPos(15);
    } else {
      TRACE(_T("m_yk.writeChallengeBegin() failed"));
    }
    trashMemory(chalBuf, chalLength);
  }
}

void CPKBaseDlg::OnTimer(UINT_PTR )
{
  // Ignore if Secure Desktop
  if (m_bUseSecureDesktop)
    return;

  // If an operation is pending, check if it has completed
  if (m_pending) {
    yubiCheckCompleted();
  } else {
    // No HMAC operation is pending - check if one and only one key is present
    bool inserted = IsYubiInserted();
    // call relevant callback if something's changed
    if (inserted != m_present) {
      m_present = inserted;
      if (m_present) {
        SetYubiExists(); // proof that user has a yubikey!
        yubiInserted();
      } else
        yubiRemoved();
    }
  }
}

void CPKBaseDlg::OnSwitchSecureDesktop()
{
  EndDialog(INT_MAX);
}

void CPKBaseDlg::StartThread(int iDialogType)
{
  // SetThreadDesktop fails in MFC because _AfxMsgFilterHook is used in every
  // Thread. Need to unhook and before calling SetThreadDesktop
  // Reset the hook again to msgfilter (equivalent to _AfxMsgFilterHook)
  // after finishing processing and before returning.

  CSDThread *pThrdDlg(NULL);

  CBitmap bmpDimmedScreen;
  LARGE_INTEGER liDueTime;
  HANDLE hThread(0), hWaitableTimer(0);
  DWORD dwError, dwThreadID, dwEvent, dwThreadExitCode(0);
  bool bTimerPopped(false);

  // Set timer constants
  const int nTimerUnitsPerSecond = 10000000;

  // Set good return code
  m_iRC = 0;

  // Clear progress flags
  BYTE xFlags = 0;

  _AFX_THREAD_STATE *pState = AfxGetThreadState();

  BOOL bReHook = UnhookWindowsHookEx(pState->m_hHookOldMsgFilter);
  if (!bReHook) {
    ASSERT(bReHook);
    goto BadExit;
  }

  // Update progress
  xFlags |= WINDOWSHOOKREMOVED;

  pState->m_hHookOldMsgFilter = NULL;

  // Set up waitable timer just in case there is an issue
  hWaitableTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  if (hWaitableTimer == NULL) {
    dwError = pws_os::IssueError(_T("CreateWaitableTimer"), false);
    ASSERT(hWaitableTimer);
    goto BadExit;
  }

  // Update progress
  xFlags |= WAITABLETIMERCREATED;

  // Get out of Jail Free Card method in case there is a problem in the thread
  // Set the timer to go off PWSprefs::SecureDesktopTimeout after calling SetWaitableTimer.
  // Timer unit is 100-nanoseconds
  int iUserTimeLimit = PWSprefs::GetInstance()->GetPref(PWSprefs::SecureDesktopTimeout);
  liDueTime.QuadPart = -(iUserTimeLimit * nTimerUnitsPerSecond);

  if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)) {
    dwError = pws_os::IssueError(_T("SetWaitableTimer"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update progress
  xFlags |= WAITABLETIMERSET;

  // Get orignal desktop screen shot
  GetDimmedScreen(bmpDimmedScreen);

  // Update progress
  xFlags |= DIMMENDSCREENBITMAPCREATED;

  // Create Dialog Thread class instance
  pThrdDlg = new CSDThread(&m_GMP, &bmpDimmedScreen, iDialogType);

  // Create thread
  hThread = CreateThread(NULL, 0, pThrdDlg->ThreadProc, (void *)pThrdDlg, CREATE_SUSPENDED, &dwThreadID);
  if (hThread == NULL) {
    dwError = pws_os::IssueError(_T("CreateThread"), false);
    ASSERT(hThread);
    goto BadExit;
  }

  // Update progress
  xFlags |= THREADCREATED;

  // Resume thread (not really necessary to create it suspended and then resume but just in
  // case we want to do extra processing between creation and running
  ResumeThread(hThread);

  // Update progress
  xFlags |= THREADRESUMED;

  // Set up array of wait handles and wait for either the timer to pop or the thread to end
  {
    HANDLE hWait[2] = { hWaitableTimer, hThread };
    dwEvent = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);
  }

  // Find out what happened
  switch (dwEvent) {
  case WAIT_OBJECT_0 + 0:
  {
    // Timer popped
    bTimerPopped = true;

    // Update Progress
    xFlags &= ~WAITABLETIMERSET;

    // Stop thread - by simulating clicking on Cancel button
    ::SendMessage(pThrdDlg->m_hwndMasterPhraseDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);

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
    if (!CancelWaitableTimer(hWaitableTimer)) {
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
  }

  // Close the WaitableTimer handle
  if (!CloseHandle(hWaitableTimer)) {
    dwError = pws_os::IssueError(_T("CloseHandle - hWaitableTimer"), false);
    ASSERT(0);
    goto BadExit;
  }

  // Update Progress
  xFlags &= ~(WAITABLETIMERCREATED | WAITABLETIMERSET);

  // Before deleting the thread - get its return code
  GetExitCodeThread(hThread, &dwThreadExitCode);

  delete pThrdDlg;

  // Update Progress
  xFlags &= ~(THREADCREATED | THREADRESUMED);

  pThrdDlg = NULL;

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

  // Tidy up GDI resources
  bmpDimmedScreen.DeleteObject();

  // Update Progress
  xFlags &= ~DIMMENDSCREENBITMAPCREATED;

  // Set return code to that of the thread's
  m_iRC = dwThreadExitCode;
  return;

BadExit:
  // Need to tidy up what was done in reverse order - ignoring what wasn't and ignore errors
  if (xFlags & THREADRESUMED) {
    // Stop thread - by simulating clicking on Cancel button
    ::SendMessage(pThrdDlg->m_hwndMasterPhraseDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);

    // Now wait for thread to complete
    WaitForSingleObject(hThread, INFINITE);
  }
  if (xFlags & THREADCREATED) {
    delete pThrdDlg;
    pThrdDlg = NULL;
  }
  if (xFlags & DIMMENDSCREENBITMAPCREATED) {
    bmpDimmedScreen.DeleteObject();
  }
  if (xFlags & WAITABLETIMERSET) {
    ::CancelWaitableTimer(hWaitableTimer);
  }
  if (xFlags & WAITABLETIMERCREATED) {
    CloseHandle(hWaitableTimer);
  }
  if (xFlags & WINDOWSHOOKREMOVED) {
    pState->m_hHookOldMsgFilter = SetWindowsHookEx(WH_MSGFILTER, MsgFilter, NULL, GetCurrentThreadId());
  }

  // Set bad return code
  m_iRC = -1;
}

void CPKBaseDlg::GetDimmedScreen(CBitmap &bmpDimmedScreen)
{
  /*
  Fairly involved process but not difficult!
  1. Get size of screen (including all monitors)
  2. Create a memory DC corresponding to the screen
  3. Create a bitmap in that DC
  4. Copy across the current screen image to this bitmap

  5. Now create final dimmed screen memory DC
  6. Create a bitmap in that DC
  7. Copy the screen image here

  8. Create memory DC for black rectangle
  9. Create a bitmap in that DC
  10. Fill bitmap with a black rectangle

  11. Load the PWS bitmap
  12. Create another memory DC for this bitmap and select the PWS bitmap
  13. Create another memory DC for the tiled bitmap
  14. Create a bitmap in the tiled DC

  15. Tile the PWS bitmap over the new bitmap

  16. Alphablend the tiled PWS bitmap onto the black rectangle

  17. Alphablend the combined tiled PWS & black rectangle over the screen image

  18. Tidy up graphics resources
  */

  // This needs to be here to get the screen shot of the original desktop
  /* 1 */
  CRect rect(0, 0, ::GetSystemMetrics(SM_CXVIRTUALSCREEN), ::GetSystemMetrics(SM_CYVIRTUALSCREEN));

  // Create a screen and a memory device context
  HDC hDCScreen = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
  CDC *pDCScreen = CDC::FromHandle(hDCScreen);

  /* 2 */
  CDC memDC_Screen;
  memDC_Screen.CreateCompatibleDC(pDCScreen);

  // Create a compatible bitmap and select it in the memory DC
  /* 3 */
  CBitmap bmp_Screen;
  bmp_Screen.CreateCompatibleBitmap(pDCScreen, rect.Width(), rect.Height());
  HBITMAP hBmpOld = (HBITMAP)::SelectObject(memDC_Screen, bmp_Screen);

  // bit-blit from screen to memory device context. Note: CAPTUREBLT needed to capture overlayed images
  /* 4 */
  const DWORD dwRop = SRCCOPY | CAPTUREBLT;
  memDC_Screen.BitBlt(0, 0, rect.Width(), rect.Height(), pDCScreen, rect.left, rect.top, dwRop);

  // Create offscreen buffer to compose the final image which consists of
  // an alphablended rectangle + PWS bitmap on top of a background image
  /* 5 */
  CDC memDC_DimmedScreen;
  memDC_DimmedScreen.CreateCompatibleDC(pDCScreen);

  /* 6 */
  bmpDimmedScreen.CreateCompatibleBitmap(pDCScreen, rect.Width(), rect.Height());
  CBitmap *pOldbmp = memDC_DimmedScreen.SelectObject(&bmpDimmedScreen);

  // Copy the background image into this DC
  /* 7 */
  memDC_DimmedScreen.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC_Screen, 0, 0, SRCCOPY);

  // Create another memory DC to draw black rectangle
  /* 8 */
  CDC memDC_Rectangle;
  memDC_Rectangle.CreateCompatibleDC(pDCScreen);

  /* 9 */
  CBitmap bmp_Rectangle;
  bmp_Rectangle.CreateCompatibleBitmap(pDCScreen, rect.Width(), rect.Height());
  CBitmap *pOldbmpRect = memDC_Rectangle.SelectObject(&bmp_Rectangle);

  // Draw the black rectangle
  /* 10 */
  memDC_Rectangle.FillSolidRect(CRect(0, 0, rect.Width(), rect.Height()), RGB(0, 0, 0));

  // Copy PWS bitmap onto rectangle
  /* 11 */
  CBitmap bmp_PWS;
  bmp_PWS.LoadBitmap(IDB_PWSBITMAP);
  BITMAP bPWSmap;
  bmp_PWS.GetBitmap(&bPWSmap);
  int bmw = bPWSmap.bmWidth;
  int bmh = bPWSmap.bmHeight;

  /* 12 */
  CDC memDC_PWSbitmap;
  memDC_PWSbitmap.CreateCompatibleDC(pDCScreen);
  CBitmap *pOldbmpPWS = memDC_PWSbitmap.SelectObject(&bmp_PWS);

  /*13 */
  CDC memDC_TiledPWSbitmap;
  memDC_TiledPWSbitmap.CreateCompatibleDC(pDCScreen);

  /* 14 */
  CBitmap bmp_TiledPWSbitmap;
  bmp_TiledPWSbitmap.CreateCompatibleBitmap(pDCScreen, rect.Width(), rect.Height());
  CBitmap *pOldbmpTiledPWSbitmap = memDC_TiledPWSbitmap.SelectObject(&bmp_TiledPWSbitmap);

  /* 15 */
  for (int y = 0; y < rect.Height(); y += bmh)
  {
    for (int x = 0; x < rect.Width(); x += bmw)
    {
      memDC_TiledPWSbitmap.BitBlt(x, y, rect.Width(), rect.Height(), &memDC_PWSbitmap, 0, 0, SRCCOPY);
    }
  }

  BLENDFUNCTION bf;
  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = 127;
  bf.AlphaFormat = 0;

  /* 16 */
  // Blend the tiled PWS image into the rectangle
  memDC_Rectangle.AlphaBlend(0, 0, rect.Width(), rect.Height(), &memDC_TiledPWSbitmap, 0, 0, rect.Width(), rect.Height(), bf);

  /* 17 */
  // Combine the image containing the background and the rectangle + tiled image
  memDC_DimmedScreen.AlphaBlend(0, 0, rect.Width(), rect.Height(), &memDC_Rectangle, 0, 0, rect.Width(), rect.Height(), bf);

  /* 18 */
  // Reset everything
  ::SelectObject(memDC_Screen, hBmpOld);
  memDC_DimmedScreen.SelectObject(pOldbmp);
  memDC_Rectangle.SelectObject(pOldbmpRect);
  memDC_PWSbitmap.SelectObject(pOldbmpPWS);
  memDC_TiledPWSbitmap.SelectObject(pOldbmpTiledPWSbitmap);
  bmp_PWS.DeleteObject();

  ::DeleteDC(hDCScreen);
}
