/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "PWSRecentFileList.h"
#include "PWSFaultHandler.h"

#include "core/Util.h"
#include "core/PWScore.h"

#include "os/run.h"

#include <afxmt.h>
//-----------------------------------------------------------------------------

// Structure for saving information on what language/help files are installed.
struct LANGHELPFILE {
  LCID lcid;                 // LCID for the language
  std::wstring wsLL;         // 2-character language code
  std::wstring wsCC;         // 2-character country code if needed
  std::wstring wsLanguage;   // Name of language for menu item

  /*
    1... .... 0x80 Current
    .1.. .... 0x40 Help file also present
  */
  BYTE xFlags;               // Flags
};

class DboxMain;

class ThisMfcApp : public CWinApp
{
public:
  ThisMfcApp();
  ~ThisMfcApp();

  CPWSRecentFileList* GetMRU() { return m_pMRU; }
  void ClearMRU();
  void AddToMRU(const CString &pszFilename);

  DboxMain *GetMainDlg() {return m_pDbx;}
  PWScore *GetCore() {return &m_core;}
  CMenu *GetMainMenu() {return m_pMainMenu;}

  BOOL IsMRUOnFileMenu() {return m_mruonfilemenu;}

  std::vector<LANGHELPFILE> m_vlanguagefiles;

  HACCEL m_ghAccelTable;
  static const UINT m_uiRegMsg;
  static const UINT m_uiWH_SHELL;

  void EnableAccelerator() { m_bUseAccelerator = true; }
  void DisableAccelerator() { m_bUseAccelerator = false; }
  bool IsAcceleratorEnabled() { return m_bUseAccelerator;}

  bool WasHotKeyPressed() {return m_HotKeyPressed;}
  void SetHotKeyPressed(bool state) {m_HotKeyPressed = state;}
  int FindMenuItem(CMenu* Menu, UINT MenuID);
  int FindMenuItem(CMenu* Menu, LPCWSTR MenuString);
  void GetApplicationVersionData();
  void GetDLLVersionData(const CString &cs_dll, int &wLangID);
  CString GetFileVersionString() const {return m_csFileVersionString;}
  CString GetCopyrightString() const {return m_csCopyrightString;}
  CString GetHelpFileName() const {return m_csHelpFile;}
  DWORD GetFileVersionMajorMinor() const {return m_dwMajorMinor;}
  DWORD GetFileVersionBuildRevision() const {return m_dwBuildRevision;}
  void SetACCELTableCreated() {m_bACCEL_Table_Created = true;}
  bool NoSysEnvWarnings() const {return m_noSysEnvWarnings;}
  bool PermitTestdump() const {return m_bPermitTestdump;}
  DWORD GetBaseThreadID() {return m_nBaseThreadID;}
  void GetLanguageFiles();
  void SetLanguage();
  void SetMinidumpUserStreams(const bool bOpen, const bool bRW, UserStream iStream = usAll);

  DWORD GetOSMajorMinor() { return m_dwMajorMinor; }

protected:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

  DECLARE_MESSAGE_MAP()

private:
  bool ParseCommandLine(DboxMain &dbox, bool &allDone);
  bool GetConfigFromCommandLine(StringX &sxConfigFile, StringX &sxHost, StringX &sxUser);
  void LoadLocalizedStuff();
  void SetupMenu();

  static BOOL CALLBACK searcher(HWND hWnd, LPARAM lParam);

  DboxMain *m_pDbx;
  PWScore m_core;

  CMenu *m_pMainMenu, *m_pMRUMenu;
  CPWSRecentFileList *m_pMRU;
  bool m_bUseAccelerator;

  HANDLE m_hMutexOneInstance;
  HINSTANCE m_hInstResDLL;

  bool m_HotKeyPressed, m_bACCEL_Table_Created;
  DWORD m_dwMajorMinor;
  DWORD m_dwBuildRevision;
  CString m_csFileVersionString;
  CString m_csCopyrightString;
  CString m_csHelpFile;
  int m_AppLangID, m_ResLangID;

  BOOL m_mruonfilemenu;

  // Following set by command line arguments
  bool m_noSysEnvWarnings; // '-q'
  bool m_bPermitTestdump;  // '--testdump'

  // Used to check if called from a thread
  DWORD m_nBaseThreadID;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
