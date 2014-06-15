/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CSDThread.h header file

#include "GetMasterPhrase.h"

#include "../../core/StringX.h"

class CVKeyBoardDlg;

class CSDThread
{

public:
  CSDThread(GetMasterPhrase *pGMP, CBitmap *pbmpDimmedScreen, const int iDialogID);
  virtual ~CSDThread();

  StringX GetPhrase() { return m_pGMP->sPhrase; }

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

 protected:
   BOOL InitInstance();
   static DWORD WINAPI ThreadProc(void *lpParameter);

   static INT_PTR CALLBACK MPDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
   static void CALLBACK TimerProc(void *lpParameter, BOOLEAN TimerOrWaitFired);
   static BOOL CALLBACK DesktopEnumProc(LPTSTR name, LPARAM lParam);

 private:
   friend class CPKBaseDlg;
   friend class CPasskeyEntry;
   friend class CPasskeyChangeDlg;

   void SetBkGndImage(HWND hwndBkGnd);
   void CheckDesktopStillPresent();

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
   HWND m_hwndStaticTimer, m_hwndVKStaticTimer;
   int m_iMinutes, m_iSeconds;
   int m_iUserTimeLimit;
   WORD m_wDialogID;

   bool m_bVKCreated, m_bDoTimerProcAction, m_bMPWindowBeingShown, m_bVKWindowBeingShown;
   bool m_bUseSecureDesktop, m_bDesktopStillPresent;
   BYTE xFlags;

   // Secure Desktop related
   void GetDimmedScreen();
   void StartThread();
   int m_iLastFocus, m_iRC;
};