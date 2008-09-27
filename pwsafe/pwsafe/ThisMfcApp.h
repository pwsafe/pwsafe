/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file ThisMfcApp.h
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "stdafx.h"
#include "corelib/Util.h"
#include "corelib/PWScore.h"
#include "SystemTray.h"
#include "PWSRecentFileList.h"

#include <afxmt.h>
//-----------------------------------------------------------------------------

class DboxMain;

class ThisMfcApp
  : public CWinApp
{
public:
  ThisMfcApp();
  ~ThisMfcApp();

  HACCEL m_ghAccelTable;

  CPWSRecentFileList* GetMRU() { return m_pMRU; }
  void ClearMRU();
  void AddToMRU(const CString &pszFilename);

  DboxMain* m_maindlg;
  PWScore m_core;
  CMenu* m_mainmenu;
  BOOL m_mruonfilemenu;
  HINSTANCE m_hInstResDLL;
  static const UINT m_uiRegMsg;

  virtual BOOL InitInstance();
  virtual int ExitInstance();
  WCE_DEL  virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

  void EnableAccelerator() { m_bUseAccelerator = true; }
  void DisableAccelerator() { m_bUseAccelerator = false; }

  BOOL SetTooltipText(LPCTSTR ttt) {return m_TrayIcon->SetTooltipText(ttt);}
  BOOL SetMenuDefaultItem(UINT uItem)
  {return m_TrayIcon->SetMenuDefaultItem(uItem, FALSE);}
  BOOL IsIconVisible() const {return m_TrayIcon->Visible();}
  void ShowIcon() {m_TrayIcon->ShowIcon();}
  void HideIcon() {m_TrayIcon->HideIcon();}

  afx_msg void OnHelp();
  enum STATE {LOCKED, UNLOCKED, CLOSED};
  void SetSystemTrayState(STATE);
  STATE GetSystemTrayState() const {return m_TrayLockedState;}
  int SetClosedTrayIcon(int &icon, bool bSet = true);
  bool WasHotKeyPressed() {return m_HotKeyPressed;}
  void SetHotKeyPressed(bool state) {m_HotKeyPressed = state;}
  int FindMenuItem(CMenu* Menu, UINT MenuID);
  int FindMenuItem(CMenu* Menu, LPCTSTR MenuString);
  void GetApplicationVersionData();
  CString GetFileVersionString() const {return m_csFileVersionString;}
  CString GetCopyrightString() const {return m_csCopyrightString;}
  CString GetHelpFileName() const {return m_csHelpFile;}
  DWORD GetFileVersionMajorMinor() const {return m_dwMajorMinor;}
  DWORD GetFileVersionBuildRevision() const {return m_dwBuildRevision;}

  DECLARE_MESSAGE_MAP()

protected:
  CPWSRecentFileList* m_pMRU;
  bool m_bUseAccelerator;

private:
  bool ParseCommandLine(DboxMain &dbox, bool &allDone);
  void LoadLocalizedStuff();
  static BOOL CALLBACK searcher(HWND hWnd, LPARAM lParam);
  HANDLE m_hMutexOneInstance;

  HICON m_LockedIcon;
  HICON m_UnLockedIcon;
  HICON m_ClosedIcon;
  CSystemTray *m_TrayIcon; // DboxMain needs to be constructed first
  STATE m_TrayLockedState;
  bool m_HotKeyPressed;
  DWORD m_dwMajorMinor;
  DWORD m_dwBuildRevision;
  CString m_csFileVersionString;
  CString m_csCopyrightString;
  CString m_csHelpFile;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
