/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CSDThread.h header file

#include "core/StringX.h"
#include "GetMasterPhrase.h"

class CVKeyBoardDlg;

class CSDThread
{

public:
  CSDThread(GetMasterPhrase *pGMP, CBitmap *pbmpDimmedScreen, const int iDialogID, HMONITOR hCurrentMonitor);
  virtual ~CSDThread();

  StringX GetPhrase() const { return m_pGMP->sPhrase; }

 protected:
   BOOL InitInstance();
   static DWORD WINAPI ThreadProc(void *lpParameter);

   static INT_PTR CALLBACK MPDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
   static void CALLBACK TimerProc(void *lpParameter, BOOLEAN TimerOrWaitFired);
   static BOOL CALLBACK DesktopEnumProc(LPTSTR name, LPARAM lParam);
   static BOOL CALLBACK WindowEnumProc(HWND hwnd, LPARAM lParam);

 private:
  enum {
    NEWDESKTOCREATED           = 0x01,
    SETTHREADDESKTOP           = 0x02,
    SWITCHEDDESKTOP            = 0x04,
    REGISTEREDWINDOWCLASS      = 0x08,
    BACKGROUNDWINDOWCREATED    = 0x10,
    MASTERPHRASEDIALOGCREATED  = 0x20,
    VIRTUALKEYBOARDCREATED     = 0x40,
    MASTERPHRASEDIALOGENDED    = 0x80,
  };

   friend class CPKBaseDlg;
   friend class CPasskeyEntry;
   friend class CPasskeyChangeDlg;

   void OnInitDialog();
   void OnVirtualKeyboard();
   void OnOK();
   void OnCancel();
   void OnQuit();
   void OnInsertBuffer();

   BOOL AddTooltip(UINT uiControlID, UINT uiToolString, UINT uiFormat = NULL);
   BOOL AddTooltip(UINT uiControlID, stringT sText);

   void SetBkGndImage(HWND hwndBkGnd);
   void CheckDesktop();
   void CheckWindow();

   stringT m_masterphrase;
   stringT m_sBkGrndClassName;
   stringT m_sDesktopName;

   CBitmap *m_pbmpDimmedScreen;
   GetMasterPhrase *m_pGMP;
   CVKeyBoardDlg *m_pVKeyBoardDlg;

   HINSTANCE m_hInstance;
   HWND m_hwndBkGnd, m_hwndVKeyBoard, m_hwndMasterPhraseDlg;
   HDESK m_hOriginalDesk, m_hNewDesktop;

   unsigned int m_iStartTime;
   HANDLE m_hTimer;
   HWND m_hwndStaticTimer, m_hwndStaticTimerText, m_hwndStaticSeconds;
   HWND m_hwndDlg, m_hwndTooltip;
   int m_iMinutes, m_iSeconds;
   int m_iUserTimeLimit;
   WORD m_wDialogID;
   HMONITOR m_hCurrentMonitor;
   COLORREF m_cfMask;
   int m_IDB;

   bool m_bVKCreated, m_bDoTimerProcAction, m_bMPWindowBeingShown, m_bVKWindowBeingShown;
   bool m_bUseSecureDesktop, m_bDesktopPresent, m_bWindowPresent;
   BYTE xFlags;

   // Secure Desktop related
   void GetDimmedScreen();
   void StartThread();
   int m_iLastFocus;
   DWORD m_dwRC;
};
