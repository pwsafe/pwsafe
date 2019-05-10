/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ThisMfcApp.cpp
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------

/*
*  The code for checking on multiple instances comes from the article
*  "Avoiding Multiple Instances of an Application" by
*  Joseph M. Newcomer [MVP]; http://www.flounder.com/nomultiples.htm
*/

#include "PasswordSafe.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "SingleInstance.h"
#include "CryptKeyEntry.h"
#include "PWSRecentFileList.h"
#include "MFCMessages.h"
#include "GeneralMsgBox.h"
#include "PWSFaultHandler.h"
#include "Fonts.h"
#include "PWSversion.h"

#include "core/Util.h"
#include "core/BlowFish.h"
#include "core/PWSprefs.h"
#include "core/PWSrand.h"
#include "core/PWSdirs.h"
#include "core/SysInfo.h"
#include "core/PWSLog.h"

#include "os/windows/pws_autotype/pws_at.h"
#include "os/dir.h"
#include "os/file.h"
#include "os/env.h"
#include "os/lib.h"
#include "os/debug.h"

#include "Shlwapi.h"

#include <vector>
#include <errno.h>
#include <fstream>
#include <io.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT ThisMfcApp::m_uiRegMsg   = RegisterWindowMessage(UNIQUE_PWS_GUID);
const UINT ThisMfcApp::m_uiWH_SHELL = RegisterWindowMessage(UNIQUE_PWS_SHELL);

BEGIN_MESSAGE_MAP(ThisMfcApp, CWinApp)
END_MESSAGE_MAP()

static MFCReporter aReporter;
static MFCAsker    anAsker;
#ifndef _DEBUG
extern wchar_t *wcRevision;
extern wchar_t *wcMsg1;
extern wchar_t *wcMsg2;
extern wchar_t *wcMsg3;
extern wchar_t *wcCaption;
#endif

ThisMfcApp::ThisMfcApp() :
  m_bUseAccelerator(true),
  m_pMRU(NULL),
  m_HotKeyPressed(false), m_hMutexOneInstance(NULL),
  m_ghAccelTable(NULL), m_pMainMenu(NULL),
  m_bACCEL_Table_Created(false), m_noSysEnvWarnings(false),
  m_bPermitTestdump(false), m_hInstResDLL(NULL), m_ResLangID(0),
  m_pMRUMenu(NULL)
{
  // Get my Thread ID
  m_nBaseThreadID = AfxGetThread()->m_nThreadID;

  // Get application version information
  GetApplicationVersionData();

  // Only produce minidumps in release code
#ifndef _DEBUG
  // ONLY add minidump generation to Release versions!
  HMODULE module = GetModuleHandle(NULL);
  DWORD timestamp = GetTimestampForLoadedLibrary(module);

  DWORD dwMajorMinor = GetFileVersionMajorMinor();
  DWORD dwBuildRevision = GetFileVersionBuildRevision();

  int iMajor(0), iMinor(0), iBuild(0);
  if (dwMajorMinor > 0) {
    iMajor = HIWORD(dwMajorMinor);
    iMinor = LOWORD(dwMajorMinor);
    iBuild = HIWORD(dwBuildRevision);
  }

  CString csFileVersionString, csRevision(L"");
  csFileVersionString = GetFileVersionString();
  int revIndex = csFileVersionString.ReverseFind(L',');
  if (revIndex >= 0) {
    int len = csFileVersionString.GetLength();
    csRevision = csFileVersionString.Right(len - revIndex - 1);
    csRevision.Trim();
  }

  LPTSTR revision = csRevision.GetBuffer(csRevision.GetLength() + 1);
  InstallFaultHandler(iMajor, iMinor, iBuild, revision, timestamp);
  csRevision.ReleaseBuffer();
#endif

  // Set this process to be one of the first to be shut down:
  SetProcessShutdownParameters(0x3ff, 0);

  PWSprefs::SetReporter(&aReporter);
  PWScore::SetReporter(&aReporter);
  PWScore::SetAsker(&anAsker);

  EnableHtmlHelp();

  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Initializes the COM library
  //                                                 (for XML and Yubikeyprocessing)
  
  AfxEnableControlContainer();
  AfxOleInit();
}

ThisMfcApp::~ThisMfcApp()
{
  delete m_pMRU;

  // Remove MRU menu
  if (m_pMRUMenu) {
    m_pMRUMenu->DestroyMenu();
    delete m_pMRUMenu;
    m_pMRUMenu = NULL;
  }

  // Remove main menu
  if (m_pMainMenu) {
    m_pMainMenu->DestroyMenu();
    delete m_pMainMenu;
  }

  // Rules state - if create then must destroy
  if (m_bACCEL_Table_Created && app.m_ghAccelTable != NULL)
    DestroyAcceleratorTable(app.m_ghAccelTable);

  // Although the system will do this automatically - I like to be clean!
  CloseHandle(m_hMutexOneInstance);

  PWSprefs::DeleteInstance();
  PWSrand::DeleteInstance();
  PWSversion::DeleteInstance();
  Fonts::GetInstance()->DeleteInstance();
  PWSLog::DeleteLog();

  CoUninitialize(); // Uninitialize COM library

  // WinApp::HtmlHelp asserts that main windows is valid, which (1) isn't true
  // here, and (2) is irrelevant for HH_CLOSE_ALL, so we call ::HtmlHelp
  ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
#ifndef _DEBUG
  RemoveFaultHandler();
#endif
}

static void Usage()
{
  CGeneralMsgBox gmb;
  CString csMsg(MAKEINTRESOURCE(IDS_USAGE)), csTitle(MAKEINTRESOURCE(IDS_INVALIDCOMMANDLINE));
  gmb.AfxMessageBox(csMsg, csTitle, MB_OK);
}

// tests if file exists, returns true if so, displays error message if not
static bool CheckFile(const CString &fn)
{
  DWORD status = ::GetFileAttributes(fn);
  CString cs_msg(L"");

  if (status == INVALID_FILE_ATTRIBUTES) {
    cs_msg.Format(IDS_FILEERROR1, static_cast<LPCWSTR>(fn));
  } else if (status & FILE_ATTRIBUTE_DIRECTORY) {
    cs_msg.Format(IDS_FILEERROR2, static_cast<LPCWSTR>(fn));
  }

  if (cs_msg.IsEmpty()) {
    return true;
  } else {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(cs_msg);
    return false;
  }
}

void ThisMfcApp::SetMinidumpUserStreams(const bool bOpen, const bool bRW, UserStream iStream)
{
#ifndef _DEBUG
  PWSprefs *prefs = PWSprefs::GetInstance();
  if (prefs != NULL) {
    PopulateMinidumpUserStreams(prefs, bOpen, bRW, iStream);
  }
#else
  UNREFERENCED_PARAMETER(bOpen);
  UNREFERENCED_PARAMETER(bRW);
  UNREFERENCED_PARAMETER(iStream);
#endif
}

int ThisMfcApp::ExitInstance()
{
  if (m_hInstResDLL != m_hInstance)
    pws_os::FreeLibrary(m_hInstResDLL);

  CWinApp::ExitInstance();
  return 0;
}

// Listener window whose sole purpose in life is to receive
// the "AppStop" message from the U3 framework and then
// cause a semi-graceful exit.

class CLWnd : public CWnd
{
public:
  CLWnd(DboxMain &dbox) : m_dbox(dbox) {}
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
  {
    if (message != (WM_APP+0x765))
      return CWnd::WindowProc(message, wParam, lParam);
    else
      m_dbox.U3ExitNow();
    return 0L;
  }
private:
  DboxMain &m_dbox;
};

static void GetVersionInfoFromFile(const CString &csFileName,
                                   DWORD &MajorMinor, DWORD &BuildRevision)
{
  MajorMinor = BuildRevision = DWORD(-1);
  DWORD dwVerHnd, dwVerInfoSize;

  // Get version information from the given file
  dwVerInfoSize = ::GetFileVersionInfoSize((LPWSTR)(LPCWSTR)csFileName, &dwVerHnd);
  if (dwVerInfoSize > 0) {
    char *pVersionInfo = new char[dwVerInfoSize];
    if (pVersionInfo != NULL) {
      BOOL bRet = ::GetFileVersionInfo((LPWSTR)(LPCWSTR)csFileName,
                    (DWORD)dwVerHnd, (DWORD)dwVerInfoSize, (LPVOID)pVersionInfo);

      if (bRet) {
        // get binary file version information
        VS_FIXEDFILEINFO *szVer = NULL;
        UINT uVerLength; 
        bRet = ::VerQueryValue(pVersionInfo, TEXT("\\"), (LPVOID*)&szVer, &uVerLength);
        if (bRet) {
          MajorMinor = szVer->dwProductVersionMS;
          BuildRevision = szVer->dwProductVersionLS;
        }
      }
    }
    delete[] pVersionInfo;
  }
}

void ThisMfcApp::LoadLocalizedStuff()
{
  /*
  Looks for localized version of resources and help files, loads them if found
  (Also tries to setup on-screen keyboard)

  Format of resource-only DLL names (in dir returned by GetExeDir)
    pwsafeLL_CC.dll
      or
    pwsafeLL.dll

  where LL = ISO 639-1 two-character Language code e.g. EN, FR, DE, HE...
              see http://www.loc.gov/standards/iso639-2/
  and   CC = ISO 3166-1 two-character Country code e.g. US, GB, FR, CA...
              see http://www.iso.org/iso/en/prods-services/iso3166ma/index.html

  Although ISO 639 has been superceded, MS only supports the new RFC 3066bis in
  .NET V2 and later applications (CultureInfo Class) or under Vista (via LOCALE_SNAME).
  Older native and .NET V1 applications only support the ISO 639-1 two character
  language codes.

  Search order will be pwsafeLL_CC.dll, followed by pwsafeLL.dll. If neither exist or
  can't be found, the resources embedded in the executable pwsafe.exe will be used 
  (US English i.e. equivalent to pwsafeEN_US.dll)).

  Likewise, we will look for localized versions of pwsafe.chm in GetHelpDir,
  defaulting to pwsafe.chm if not found.
  */

  CString cs_ResPath, cs_HelpPath, cs_LANG(L""), cs_CTRY(L"");
  const CString cs_HelpDir(PWSdirs::GetHelpDir().c_str());
  const CString cs_ExePath(PWSdirs::GetExeDir().c_str());
  
  // See if user overridden default via Environment Variable
  bool bPLRC(FALSE);
  
  // See if user has set preference instead.
  StringX sxLL = PWSprefs::GetInstance()->GetPref(PWSprefs::LanguageFile);

  if (!sxLL.empty()) {
    // Preferences trumps Environment Variable!!!!!
    size_t len = sxLL.length();
    ASSERT(len == 2 || len == 5);
    if (len == 2) {
      cs_LANG = sxLL.c_str();
    } else {
      cs_LANG = sxLL.substr(0,2).c_str();
      cs_CTRY = sxLL.substr(3,2).c_str();
    }
    bPLRC = true;
  }

  if (cs_LANG == L"EN" && cs_CTRY.IsEmpty()) {
    // This means use the embedded resources!
    pws_os::FreeLibrary(m_hInstResDLL);
    m_hInstResDLL = NULL;
    pws_os::Trace(L"Using embedded resources\n");

    //  and supplied Help file
    free((void *)m_pszHelpFilePath);

    cs_HelpPath.Format(L"%spwsafe.chm", static_cast<LPCWSTR>(cs_HelpDir));
    if (PathFileExists(cs_HelpPath)) {
      m_pszHelpFilePath = _wcsdup(cs_HelpPath);
      m_csHelpFile = cs_HelpPath;
      pws_os::Trace(L"Using help file: %s\n", static_cast<LPCWSTR>(cs_HelpPath));
    } else {
      m_pszHelpFilePath = NULL;
      m_csHelpFile = L"";
      pws_os::Trace(L"Not using a help file!\n");
    }

    return;
  }

  if (!bPLRC) {
    // no override, use Locale info
    int inum;
    wchar_t szLang[4], szCtry[4];
    inum = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, szLang, 4);
    ASSERT(inum == 3);

    _wcsupr_s(szLang, 4);

    inum = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, szCtry, 4);
    ASSERT(inum == 3);

    cs_LANG = szLang; cs_CTRY = szCtry;
  }

  // Find resource-only DLL if requested
  const CString format_string = (cs_CTRY.IsEmpty()) ?
                      L"pwsafe%s%s.dll" : L"pwsafe%s_%s.dll";
  cs_ResPath.Format(format_string, static_cast<LPCWSTR>(cs_LANG),
                       static_cast<LPCWSTR>(cs_CTRY));
  m_hInstResDLL = HMODULE(pws_os::LoadLibrary(LPCTSTR(cs_ResPath),
                                              pws_os::loadLibraryTypes::RESOURCE));

  if (m_hInstResDLL == NULL && !cs_CTRY.IsEmpty()) {
    // Now try base
    cs_ResPath.Format(L"pwsafe%s.dll", static_cast<LPCWSTR>(cs_LANG));
    m_hInstResDLL = HMODULE(pws_os::LoadLibrary(LPCTSTR(cs_ResPath),
                                                pws_os::loadLibraryTypes::RESOURCE));
  }

  if (m_hInstResDLL == NULL) {
    pws_os::Trace(L"Could not load language DLLs - using embedded resources.\n");
    // If the requested DLL is not default English, show an error so we know the specifics.
    // At a minimum this will show the actual error code.
    if (cs_LANG != L"EN") {
		  CString errMessage;
		  errMessage.Format(L"Attempt to load %s", LPCTSTR(cs_ResPath));
		  pws_os::IssueError(LPCTSTR(errMessage));
	  }
  } else { // successfully loaded a resource dll, check version
    DWORD dw_fileMajorMinor, dw_fileBuildRevision;
    GetVersionInfoFromFile(cs_ResPath, dw_fileMajorMinor, dw_fileBuildRevision);

    if (dw_fileMajorMinor != m_dwMajorMinor) { // ignore build for now
      CGeneralMsgBox gmb;
      CString oops;
      oops.Format(L"Executable/language DLL (%s) version mismatch %d.%d.%d/%d.%d.%d.\n", 
                  static_cast<LPCWSTR>(cs_ResPath),
                  HIWORD(dw_fileMajorMinor), LOWORD(dw_fileMajorMinor), HIWORD(dw_fileBuildRevision),
                  HIWORD(m_dwMajorMinor), LOWORD(m_dwMajorMinor), HIWORD(m_dwBuildRevision));
      gmb.AfxMessageBox(oops);
      pws_os::FreeLibrary(m_hInstResDLL);
      m_hInstResDLL = NULL;
    } else { // Passed version check
      pws_os::Trace(L"Using language DLL '%s'.\n", static_cast<LPCWSTR>(cs_ResPath));
      GetDLLVersionData(cs_ResPath, m_ResLangID);
    }
  } // end of resource dll hunt

  if (m_hInstResDLL != NULL)
    AfxSetResourceHandle(m_hInstResDLL);

  // Find requested Help file
  bool helpFileFound = false;

  if (!cs_LANG.IsEmpty() && !cs_CTRY.IsEmpty()) {
    cs_HelpPath.Format(L"%spwsafe%s_%s.chm", static_cast<LPCWSTR>(cs_HelpDir),
                       static_cast<LPCWSTR>(cs_LANG),
                       static_cast<LPCWSTR>(cs_CTRY));
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
    }
  }

  if (!helpFileFound && !cs_LANG.IsEmpty()) {
    cs_HelpPath.Format(L"%spwsafe%s.chm", static_cast<LPCWSTR>(cs_HelpDir),
                        static_cast<LPCWSTR>(cs_LANG));
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
    }
  }

  if (!helpFileFound) {
    cs_HelpPath.Format(L"%spwsafe.chm",static_cast<LPCWSTR>( cs_HelpDir));
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
    }
  }

  if (!helpFileFound && m_pszHelpFilePath != NULL && _tcslen(m_pszHelpFilePath) > 0) { // last resort
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    _wsplitpath_s(m_pszHelpFilePath, NULL, 0, NULL, 0, fname,
                  _MAX_FNAME, ext, _MAX_EXT);
    _wcslwr_s(fname, _MAX_FNAME);
    _wcslwr_s(ext, _MAX_EXT);
    cs_HelpPath.Format(L"%s%s", fname, ext);
  }

  if (helpFileFound) {
    free((void*)m_pszHelpFilePath);

    m_pszHelpFilePath = _wcsdup(cs_HelpPath);
    m_csHelpFile = cs_HelpPath;
    pws_os::Trace(L"Using help file: %s\n", static_cast<LPCWSTR>(cs_HelpPath));
  }
}

void ThisMfcApp::SetupMenu()
{
  // Remove MRU menu
  if (m_pMRUMenu) {
    m_pMRUMenu->DestroyMenu();
    delete m_pMRUMenu;
    m_pMRUMenu = NULL;
  }

  // Remove Main menu
  if (m_pMainMenu) {
    m_pMainMenu->DestroyMenu();
    delete m_pMainMenu;
    m_pMainMenu = NULL;
  }

  //Reload accelerators for current locale
  if (app.m_ghAccelTable)
    DestroyAcceleratorTable(app.m_ghAccelTable);
  app.m_ghAccelTable=LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCS));

  int nMRUItems = PWSprefs::GetInstance()->GetPref(PWSprefs::MaxMRUItems);
  m_mruonfilemenu = PWSprefs::GetInstance()->GetPref(PWSprefs::MRUOnFileMenu);
  m_pMainMenu = new CMenu;
  m_pMainMenu->LoadMenu(IDR_MAINMENU);

  m_pMRUMenu = new CMenu;

  MENUINFO minfo = {0};
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;

  // Menu position indices (File & Close).
  int pos1, pos2;
  CMenu *pMenu1, *pMenu2;

  // Look for "File" menu.
  pos1 = app.FindMenuItem(m_pMainMenu, ID_FILEMENU);
  ASSERT(pos1 != -1);

  pMenu1 = m_pMainMenu->GetSubMenu(pos1);
  minfo.dwMenuData = ID_FILEMENU;
  
  if (pMenu1 != NULL) { // Look for "Lock Database"
    pMenu1->SetMenuInfo(&minfo);
    pos2 = FindMenuItem(pMenu1, ID_MENUITEM_LOCK);
  }
  else
    pos2 = -1;

  if (m_pMRU != NULL) {
    delete m_pMRU;
    m_pMRU = NULL;
  }

  m_pMRU = new CPWSRecentFileList(0, L"MRU", L"Safe%d",
                                 ((nMRUItems != 0) ? nMRUItems : 1));
  if (nMRUItems > 0) {
    if (pos2 > -1) {
      int irc;
      // Create New Popup Menu
      m_pMRUMenu->CreatePopupMenu();
      CString cs_recent(MAKEINTRESOURCE(IDS_RECENT));
      CString cs_recentsafes(MAKEINTRESOURCE(IDS_RECENTSAFES));

      if (!m_mruonfilemenu) { // MRU entries in popup menu
        // Insert Item onto new popup
        irc = m_pMRUMenu->InsertMenu(0, MF_BYPOSITION, 
                                       ID_FILE_MRU_ENTRY1, cs_recent);
        ASSERT(irc != 0);
        // Insert Popup onto main menu
        ASSERT(pMenu1 != NULL);
        irc = pMenu1->InsertMenu(pos2 + 2,
                                 MF_BYPOSITION | MF_POPUP,
                                 UINT_PTR(m_pMRUMenu->m_hMenu),
                                 cs_recentsafes);
        ASSERT(irc != 0);
      } else {  // MRU entries inline
        ASSERT(pMenu1 != NULL);
        irc = pMenu1->InsertMenu(pos2 + 2, MF_BYPOSITION, 
                                 ID_FILE_MRU_ENTRY1, cs_recent);
        ASSERT(irc != 0);
      } // m_mruonfilemenu

      m_pMRU->ReadList();
    } // pos > -1
  } else { // nMRUItems <= 0
    if (pos2 > -1) {
      int irc;
      // Remove extra separator
      ASSERT(pMenu1 != NULL);
      irc = pMenu1->RemoveMenu(pos2 + 1, MF_BYPOSITION);
      ASSERT(irc != 0);
      // Remove Clear MRU menu item.
      irc = pMenu1->RemoveMenu(ID_MENUITEM_CLEAR_MRU, MF_BYCOMMAND);
      ASSERT(irc != 0);
    }
  }

  // Do File Menu Export submenu
  pos2 = app.FindMenuItem(pMenu1, ID_EXPORTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_EXPORTMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do File Menu Import submenu
  pos2 = app.FindMenuItem(pMenu1, ID_IMPORTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_IMPORTMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do Edit Menu
  pos1 = app.FindMenuItem(m_pMainMenu, ID_EDITMENU);
  ASSERT(pos1 != -1);

  pMenu1 = m_pMainMenu->GetSubMenu(pos1);
  minfo.dwMenuData = ID_EDITMENU;
  pMenu1->SetMenuInfo(&minfo);

  // Do Edit Menu Export Entry submenu
  pos2 = app.FindMenuItem(pMenu1, ID_EXPORTENTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_EXPORTENTMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do View Menu
  pos1 = app.FindMenuItem(m_pMainMenu, ID_VIEWMENU);
  ASSERT(pos1 != -1);

  pMenu1 = m_pMainMenu->GetSubMenu(pos1);
  minfo.dwMenuData = ID_VIEWMENU;
  pMenu1->SetMenuInfo(&minfo);

  // Do View Menu Subview submenu
  pos2 = app.FindMenuItem(pMenu1, ID_SUBVIEWMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_SUBVIEWMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do View Menu Filter submenu
  pos2 = app.FindMenuItem(pMenu1, ID_FILTERMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_FILTERMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do View Menu ChangeFont submenu
  pos2 = app.FindMenuItem(pMenu1, ID_CHANGEFONTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_CHANGEFONTMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do View Menu Reports submenu
  pos2 = app.FindMenuItem(pMenu1, ID_REPORTSMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  minfo.dwMenuData = ID_REPORTSMENU;
  pMenu2->SetMenuInfo(&minfo);

  // Do Manage Menu
  pos1 = app.FindMenuItem(m_pMainMenu, ID_MANAGEMENU);
  ASSERT(pos1 != -1);

  pMenu1 = m_pMainMenu->GetSubMenu(pos1);
  minfo.dwMenuData = ID_MANAGEMENU;
  pMenu1->SetMenuInfo(&minfo);

  // No need to do the Manage Menu Languages submenu as rebuilt each time

  // Do Help Menu
  pos1 = app.FindMenuItem(m_pMainMenu, ID_HELPMENU);
  ASSERT(pos1 != -1);

  pMenu1 = m_pMainMenu->GetSubMenu(pos1);
  minfo.dwMenuData = ID_HELPMENU;
  pMenu1->SetMenuInfo(&minfo);
}

void ThisMfcApp::SetLanguage()
{
  // Free old resource only DLL
  if (m_hInstResDLL != m_hInstance)
    FreeLibrary(m_hInstResDLL);

  m_hInstResDLL = NULL;

  // Close the current Help file
  ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);

  // Go set it up again
  LoadLocalizedStuff();

  if (m_hInstResDLL == NULL) {
    // OK - not to use resource-only DLL - need to set to embedded resources
    AfxSetResourceHandle(m_hInstance);
    m_ResLangID = m_AppLangID;
  }

  // Setup main menu again
  SetupMenu();
}

bool ThisMfcApp::GetConfigFromCommandLine(StringX &sxConfigFile, StringX &sxHost, StringX &sxUser)
{
  // ONLY get config file, host & user name if supplied
  // This MUST be done before PWSprefs is first initialised
  sxConfigFile = sxHost = sxUser = L"";
  wstring sConfig;

  if (m_lpCmdLine[0] != L'\0') {
    CString args = m_lpCmdLine;

    // Start command line parsing by pushing each argument into a vector
    // Quotes are stripped here
    vector<CString> argvec;
    int pos = 0;
    CString tok;
    do {
      if (args[pos] == L'\"')
        tok = args.Tokenize(L"\"", pos);
      else
        tok = args.Tokenize(L" ", pos);
      if (!tok.IsEmpty())
        argvec.push_back(tok);
    } while (pos != -1 && pos < args.GetLength());

    vector<CString>::iterator arg = argvec.begin();

    while (arg != argvec.end()) {
      if ((*arg)[0] == L'-') {
        switch ((*arg)[1]) {
        case L'U': case L'u': // set effective user
          // ensure there's another non-flag argument
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == L'-') {
            // Better stop as we don't know how bad things are!
            // However - tell user about issue!
            Usage();
            return false;
          } else {
            arg++;
            sxUser = *arg;
          }
          break;
        case L'H': case L'h': // set effective host
          // ensure there's another non-flag argument
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == L'-') {
            // Better stop as we don't know how bad things are!
            // However - tell user about issue!
            Usage();
            return false;
          } else {
            arg++;
            sxHost = *arg;
          }
          break;
        case L'G': case L'g': // override default config file
          // ensure there's another non-flag argument
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == L'-') {
            // Better stop as we don't know how bad things are!
            // However - tell user about issue!
            Usage();
            return false;
          } else {
            arg++;
            bool bRelative(false);
            if (PathIsRelative(*arg)) {
              // As per Help entry - use executable directory unless overridden
              // Don't use PWSdirs::GetConfigDir()
              wstring sPWS_PREFSDIR = pws_os::getenv("PWS_PREFSDIR", true);
              if (sPWS_PREFSDIR.empty()) {
                sConfig = pws_os::getexecdir() + wstring(*arg);
              } else {
                sConfig = sPWS_PREFSDIR + wstring(*arg);
              }
              bRelative = true;
            } else {
              sConfig = *arg;
            }

            // Now resolve and tidy up absolute path
            wchar_t full[_MAX_PATH + 1];
            if (_wfullpath(full, sConfig.c_str(), _MAX_PATH) != NULL) {
              sConfig = full;
            }

            if (!pws_os::FileExists(sConfig)) {
              // FileExists returns false if specified file not found
              CString cs_ErrorMsg, csConfigfile;
              // If the user hasn't specified a language for issueing error messages
              // during command line processing and before the configuration file
              // has been opened, this message will be in English (as will any display
              // of the Usage.
              if (bRelative) {
                // Also tell user the full path we checked as well.
                csConfigfile.Format(L"%s\n  (%s)", static_cast<LPCWSTR>(*arg), 
                  static_cast<LPCWSTR>(sConfig.c_str()));
              } else {
                // Just tell the user the absolute path they specified
                csConfigfile = *arg;
              }

              CGeneralMsgBox gmb;
              cs_ErrorMsg.Format(L"Configuration file not found.\n  %s\n\nDo you wish to create it?",
                static_cast<LPCWSTR>(csConfigfile));

              INT_PTR rc = gmb.AfxMessageBox(cs_ErrorMsg, L"Error finding configuration file",
                                 MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON2);

              if (rc == IDNO) {
                // Better stop as we don't know how bad things are!
                return false;
              }

              // Try to create it
              FILE *configfile = pws_os::FOpen(sConfig.c_str(), _T("w"));
              if (configfile == NULL) {
                // Can't create it either!!!
                // Give detailed error message, if possible
                DWORD error = GetLastError();
                LPTSTR lpMsgBuf = NULL;
                DWORD dw = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL,
                                  error,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (LPTSTR)&lpMsgBuf,
                                  0, NULL);
                if (dw != 0) {
                  CGeneralMsgBox gmb_err;
                  cs_ErrorMsg.Format(L"Unable to create configuration file:\n\n\t%s", lpMsgBuf);

                  gmb_err.AfxMessageBox(cs_ErrorMsg, L"Error",
                    MB_OK | MB_ICONINFORMATION);

                  // Free buffer
                  LocalFree(lpMsgBuf);
                }
                // Better stop as we don't know how bad things are!
                return false;
              }

              // Close & delete it & drop through
              fclose(configfile);
              pws_os::DeleteAFile(sConfig);
            }
          }
          sxConfigFile = sConfig.c_str();
          break;
        } // switch on flag
      }
      arg++;
    } // while argvec
  } // Command line not empty

  return true;
}

bool ThisMfcApp::ParseCommandLine(DboxMain &dbox, bool &allDone)
{
  /*
   * Command line processing:
   * Historically, it appears that if a filename was passed as a commandline argument,
   * the application would prompt the user for the password, and the encrypt or decrypt
   * the named file, based on the file's suffix. Ugh.
   *
   * What I'll do is as follows:
   * If a file is given in the command line, it is used as the database, overriding the
   * registry value. This will allow the user to have several databases, say, one for work
   * and one for personal use, and to set up a different shortcut for each.
   *
   * I think I'll keep the old functionality, but activate it with a "-e" or "-d" flag. (ronys)
   */

  int dialogOrientation = -1; // update pref only if set

  allDone = false;
  if (m_lpCmdLine[0] != L'\0') {
    CString args = m_lpCmdLine;

    // Start command line parsing by pushing each argument into a vector
    // Quotes are stripped here
    vector<CString> argvec;
    int pos = 0;
    CString tok;
    do {
      if (args[pos] == L'\"')
        tok = args.Tokenize(L"\"", pos);
      else
        tok = args.Tokenize(L" ",pos);
      if (!tok.IsEmpty())
        argvec.push_back(tok);
    } while (pos != -1 && pos < args.GetLength());

    vector<CString>::iterator arg = argvec.begin();
    bool isEncrypt = false; // for -e/-d handling
    bool startSilent = false;
    bool fileGiven = false;

    while (arg != argvec.end()) {
      // Everything starting with '-' is a normal flag,
      // everything else is the name of a file, unless an
      // "extended flags" which starts with '--' and so must
      // be dealt with first.
      if ((*arg).Left(2) == L"--") {
        // Extended flags must be in full (no abbreviations)
        // If extended flag is not recognised - just ignore
        // If a normal flag is not recognised - show Usage
        if ((*arg) == L"--testdump") {
          m_bPermitTestdump = true;
        } else if ((*arg) == L"--fix-utf8") { // for reading non-utf8 non-English databases
          // Databases created with ~3.05.02 and used non-English text were incorrectly
          // encoded. This flag allows them to be read correctly. Saving them will
          // then encode them as utf8
          pws_os::setenv("PWS_CP_ACP", "1");
        } else if ((*arg) == L"--setup") {
          /**
           * '--setup' is meant to be used when invoking PasswordSafe at the end of the installation process.
           * It will cause the application to create a new database with the default name at the default location,
           * prompting the user for the safe combination.
           */
          dbox.SetSetup();
        } else if ((*arg) == L"--novalidate") {
          /**
           * '--novalidate' prevents SOME of validation during open
           */
          dbox.NoValidation();
        } else if ((*arg) == L"--cetreeview") {
          /**
           * '--cetreeview' will allow the user to select 2 entries and compare them
           * TEMPORARY solution until multi-select is coded in the main Tree view
           * Supported natively in List View
           */
          dbox.AllowCompareEntries();
        } else if ((*arg) == L"--do-tall") {
          dialogOrientation = PWSprefs::TALL;
        } else if ((*arg) == L"--do-wide") {
          dialogOrientation = PWSprefs::WIDE;
        } else if ((*arg) == L"--do-auto") {
          dialogOrientation = PWSprefs::AUTO;
        } else {
          // unrecognized extended flag. Silently ignore.
        }
        arg++;
        continue;
      }

      if ((*arg)[0] == L'-') {
        switch ((*arg)[1]) {
        case L'E': case L'e':
          isEncrypt = true;
        // deliberate fallthrough
        case L'D': case L'd':
          allDone = true; // no need for further processing for -e/-d
        {
          // Make sure there's another argument
          // and it's not a flag, and it is an existing file
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == L'-' ||
              !CheckFile(*(arg + 1))) {
            Usage();
            return false;
          }

          // get password from user
          CCryptKeyEntry dlg(isEncrypt);
          INT_PTR nResponse = dlg.DoModal();

          if (nResponse != IDOK) {
            return false;
          }
          StringX passkey = LPCWSTR(dlg.m_cryptkey1);

          BOOL status;
          if (isEncrypt) {
            std::wstring errstr;
            status = PWSfile::Encrypt(std::wstring(*(arg + 1)), passkey, errstr);
            if (!status) {
              CGeneralMsgBox gmb;
              gmb.AfxMessageBox(errstr.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
            }
          } else {
            std::wstring errstr;
            status = PWSfile::Decrypt(std::wstring(*(arg+1)), passkey, errstr);
            if (!status) {
              CGeneralMsgBox gmb;
              gmb.AfxMessageBox(errstr.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
            }
          }
          return true;
        } // -e or -d flag
        case L'C': case L'c':
          m_core.SetCurFile(L"");
          dbox.SetStartNoDB();
          dbox.SetStartClosed();
          break;
        case L'M': case L'm':// closed & minimized
          m_core.SetCurFile(L"");
          dbox.SetStartNoDB();
          dbox.SetStartMinimized();
          break;
        case L'R': case L'r':
          m_core.SetReadOnly(true);
          /* Set initially RO too to prevent possible RO/RW mode change issues on open,
            it will be set to real open state value later (in DboxMain::PostOpenProcessing) */
          dbox.SetDBInitiallyRO(true);
          break;
        case L'S': case L's':
          startSilent = true;
          dbox.SetStartSilent();
          break;
        case L'V': case L'v':
          // Obsolete - databases are always validated during opening unless --novalidate specified
          break;
        case L'U': case L'u': // set effective user
        case L'H': case L'h': // set effective host
        case L'G': case L'g': // override default config file
          // These have already been processed by member function GetConfigFromCommandLine
          arg++;
          break;
        case L'Q': case L'q': // be Quiet re missing fonts, dlls, etc.
          m_noSysEnvWarnings = true;
          break;
        default:
          Usage();
          return false;
        } // switch on flag
      } else { // arg isn't a flag, treat it as a filename
        if (CheckFile(*arg)) {
          fileGiven = true;
          // We send core the full path 'cause otherwise incr. backups & such get confused.
          std::wstring abspath = pws_os::fullpath(arg->GetString());
          if (abspath.empty()) // Should never happen, but try to slog on if so.
            abspath = arg->GetString();
          m_core.SetCurFile(abspath.c_str());
        } else {
          return false;
        }
      } // if arg is a flag or not
      arg++;
    } // while argvec

    // If start silent && no filename specified, start closed as well
    if (startSilent && !fileGiven)
      dbox.SetStartNoDB();
    // start silent implies system tray:
    if (startSilent)
      PWSprefs::GetInstance()->SetPref(PWSprefs::UseSystemTray, true);
  } // Command line not empty

  if (!allDone) {
    // Update dialog orientation preference if user specified:
    if (dialogOrientation != -1)
      PWSprefs::GetInstance()->SetPref(PWSprefs::DlgOrientation, dialogOrientation);
  } // !allDone
  return true;
}

BOOL ThisMfcApp::InitInstance()
{
  /*
  * It's always best to start at the beginning.  [Glinda, Witch of the North]
  */

  /*
  "SetRegistryKey" MUST be called before there is any reference to 'PWSprefs'
  in this application.

  This instructs the app to use the registry instead of .ini files.  The
  path ends up being

  HKEY_CURRENT_USER\Software\(companyname)\(appname)\(sectionname)\(valuename)
  where companyname is what's set here, and appname is taken from
  AFX_IDS_APP_TITLE (actually, CWinApp::m_pszAppName).

  Notes:
  1. I would love to move this to core/PWSprefs.cpp, but it's a protected
  member function!!!
  2. Prior to 3.05, the value was "Counterpane Systems". See PWSprefs.cpp
  for discussion on how this is handled.
  */
  SetRegistryKey(L"Password Safe");

  m_core.SetReadOnly(false);
  // Command line parsing MUST be done before the first PWSprefs lookup!
  // (since user/host/config file may be overridden!)
  bool allDone = false;

  // Get config information from the command line before "really" parsing the command line!
  StringX sxConfigFile, sxHost, sxUser;
  if (GetConfigFromCommandLine(sxConfigFile, sxHost, sxUser)) {
    // No errors - so set config file, user & host if supplied
    // Set up config file
    if (!sxConfigFile.empty())
      PWSprefs::SetConfigFile(sxConfigFile.c_str());

    if (!sxHost.empty())
      SysInfo::GetInstance()->SetEffectiveHost(sxHost.c_str());

    if (!sxUser.empty())
      SysInfo::GetInstance()->SetEffectiveUser(sxUser.c_str());
  } else {
    // Issues - get out
    return FALSE;
  }

  // Set up PWSprefs for this PasswordSafe session.
  // Whilst the language may be changed by the user via a menu, there is no user
  // facility to change the config file, host or user later.
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Now load translations (localized messages used in Usage and encrypt/decrypt dialogs)
  LoadLocalizedStuff();

#ifndef _DEBUG
  if (m_hInstResDLL != NULL)
    LocalizeFaultHandler(m_hInstResDLL);
#endif

  // Do not create dbox before config data obtained as it would create PWSprefs
  // using the potentially incorrect config data
  DboxMain dbox(m_core);

  // Parse the command line again.  If there were errors getting the config file,
  // host or user before, then this time around we will issue messages but they
  // will probably be in English unless the config data was OK previously and
  // the config file specifies a different language.
  bool parseVal = ParseCommandLine(dbox, allDone);

  // allDone will be true iff -e or -d options given, in which case
  // we're just a batch encryptor/decryptor
  if (allDone)
    return parseVal ? TRUE : FALSE;
  else if (!parseVal) // bad command line args
    return FALSE;

  // And the others - even if not referenced here
  Fonts *pFonts = Fonts::GetInstance();
  PWSversion *pPWSver = PWSversion::GetInstance();

  // Stop compiler warnings
  UNREFERENCED_PARAMETER(pFonts);
  UNREFERENCED_PARAMETER(pPWSver);

  // Update Quiet value if via environment variable rather than 
  // command line flag
  CString cs_PWS_QUIET;
  if (cs_PWS_QUIET.GetEnvironmentVariable(L"PWS_QUIET") != FALSE)
    m_noSysEnvWarnings = true;

  // Check if the user allows multiple instances.
  // For this to apply, consistently, must use the same copy of PasswordSafe
  // configuration file.
  if (!prefs->GetPref(PWSprefs::MultipleInstances)) {
    bool bAlreadyRunning;

    wchar_t szName[MAX_PATH];
    m_hMutexOneInstance = CreateMutex(NULL, FALSE, 
                              CreateUniqueName(UNIQUE_PWS_GUID, szName, 
                                               MAX_PATH, SI_DESKTOP_UNIQUE));

    bAlreadyRunning = (::GetLastError() == ERROR_ALREADY_EXISTS || 
                       ::GetLastError() == ERROR_ACCESS_DENIED);

     // The call fails with ERROR_ACCESS_DENIED if the Mutex was 
     // created in a different users session because of passing
     // NULL for the SECURITY_ATTRIBUTES on Mutex creation);

    if (bAlreadyRunning) { /* kill this */
      HWND hOther = NULL;
      EnumWindows(searcher, (LPARAM)&hOther);

      if (hOther != NULL) { /* pop up */
        ::SetForegroundWindow(hOther);

        if (IsIconic(hOther)) { /* restore */
          ::ShowWindow(hOther, SW_RESTORE);
        } /* restore */
      } /* pop up */

      return FALSE; // terminates the creation
    } /* kill this */
  }

  // Allow anyone else to take us to the foreground
  AllowSetForegroundWindow(ASFW_ANY);

  // Needed for RichEditCtrls in Dialogs (i.e. not explicitly "created").
  AfxInitRichEdit2();

  // PWScore needs it to get into database header if/when saved
  m_core.SetApplicationNameAndVersion(AfxGetAppName(), m_dwMajorMinor, m_dwBuildRevision);

  if (!m_core.IsDbOpen()) {
    std::wstring path = prefs->GetPref(PWSprefs::CurrentFile).c_str();
    pws_os::AddDrive(path);
    m_core.SetCurFile(path.c_str());
  }

  SetupMenu();

  /*
  * normal startup
  */

  /*
  Here's where PWS currently does DboxMain, which in turn will do
  the initial PasskeyEntry (the one that looks like a splash screen).
  This makes things very hard to control.
  The app object (here) should instead do the initial PasskeyEntry,
  and, if successful, move on to DboxMain.  I think. {jpr}
  */
  m_pDbx = &dbox;
  m_pMainWnd = m_pDbx;

  CLWnd ListenerWnd(dbox);
  if (SysInfo::IsUnderU3()) {
    // See comment under CLWnd to understand this.
    ListenerWnd.m_hWnd = NULL;
    if (!ListenerWnd.CreateEx(0, AfxRegisterWndClass(0),
                              L"Pwsafe Listener",
                              WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL)) {
      ASSERT(0);
      return FALSE;
    } 
  }

  // Run dialog - note that we don't particularly care what the response was
  dbox.DoModal();

  // Since the dialog has been closed, return FALSE so that we exit the
  // application, rather than start the application's message pump.
  return FALSE;
}

void ThisMfcApp::AddToMRU(const CString &pszFilename)
{
  if (m_pMRU == NULL)
    return;

  CString csMRUFilename(pszFilename);
  csMRUFilename.Trim();
  if (!csMRUFilename.IsEmpty()) {
    m_pMRU->Add(csMRUFilename);
    m_pMRU->WriteList();
  }
}

void ThisMfcApp::ClearMRU()
{
  if (m_pMRU == NULL)
    return;

  int numMRU = m_pMRU->GetSize();
  for (int i = numMRU; i > 0; i--)
    m_pMRU->Remove(i - 1);

  m_pMRU->WriteList();

  // Can't get the MRU list on the menu to tidy up automatically
  // Do it manually!
  CWnd *pMain = AfxGetMainWnd();

  CMenu *pMainMenu = pMain->GetMenu();

  // Look for "File" menu.
  int pos = FindMenuItem(pMainMenu, ID_FILEMENU);

  CMenu *pFile_Submenu = pMainMenu->GetSubMenu(pos);
  if (pFile_Submenu != NULL)  // Look for MRU first entry
    pos = FindMenuItem(pFile_Submenu, ID_FILE_MRU_ENTRY1);
  else
    return;

  if (pos > -1) {
    // Recent databases are on the main File menu - remove them
    for (int nID = numMRU; nID > 1; nID--)
      pFile_Submenu->RemoveMenu(ID_FILE_MRU_ENTRY1 + nID - 1, MF_BYCOMMAND);

    return;
  }

  // Recent databases are on the popup menu off the main File menu
  CMenu *pPopupMenu = pFile_Submenu->GetSubMenu(3);
  if (pPopupMenu != NULL)  // Look for MRU first entry
    pos = FindMenuItem(pPopupMenu, ID_FILE_MRU_ENTRY1);
  else
    return;

  if (pos > -1) {
    // Recent databases are on a popup menu - remove them
    for (int nID = numMRU; nID > 1; nID--)
      pFile_Submenu->RemoveMenu(ID_FILE_MRU_ENTRY1 + nID - 1, MF_BYCOMMAND);

    return;
  }
}

//Copied from Knowledge Base article Q100770
BOOL ThisMfcApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
  if (code < 0)
    CWinApp::ProcessMessageFilter(code, lpMsg);

  if (m_bUseAccelerator && m_pDbx != NULL && m_ghAccelTable != NULL &&
      code != MSGF_MENU) {
    if (::TranslateAccelerator(m_pDbx->m_hWnd, m_ghAccelTable, lpMsg))
      return TRUE;
  }

  return CWinApp::ProcessMessageFilter(code, lpMsg);
}

// FindMenuItem() will find a menu item string from the specified
// popup menu and returns its position (0-based) in the specified
// popup menu. It returns -1 if no such menu item string is found.
int ThisMfcApp::FindMenuItem(CMenu *Menu, LPCWSTR MenuString)
{
  ASSERT(Menu);
  ASSERT(::IsMenu(Menu->GetSafeHmenu()));

  const int count = Menu->GetMenuItemCount();
  for (int i = 0; i < count; i++) {
    CString str;
    if (Menu->GetMenuString(i, str, MF_BYPOSITION) &&
      (wcscmp(str, MenuString) == 0))
      return i;
  }

  return -1;
}

// FindMenuItem() will find a menu item ID from the specified
// popup menu and returns its position (0-based) in the specified
// popup menu. It returns -1 if no such menu item string is found.
int ThisMfcApp::FindMenuItem(CMenu *Menu, UINT MenuID)
{
  ASSERT(Menu);
  ASSERT(::IsMenu(Menu->GetSafeHmenu()));

  const int count = Menu->GetMenuItemCount();

  // Can't use GetMenuItemID as it does not understand that with the MENUEX
  // format, Popup menus can have IDs
  MENUITEMINFO miteminfo = {0};
  miteminfo.cbSize = sizeof(MENUITEMINFO);
  miteminfo.fMask = MIIM_ID;                // only want the wID of the menu item

  for (int i = 0; i < count; i++) {
    Menu->GetMenuItemInfo(i, &miteminfo, TRUE);
    if (miteminfo.wID >= 1 && miteminfo.wID == MenuID)
      return i;
  }

  return -1;
}

void ThisMfcApp::GetApplicationVersionData()
{
  wchar_t szFullPath[MAX_PATH];
  DWORD dwVerHnd, dwVerInfoSize;

  // Get version information from the application
  ::GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
  dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
  if (dwVerInfoSize > 0) {
    char *pVersionInfo = new char[dwVerInfoSize];
    if (pVersionInfo != NULL) {
      BOOL bRet = ::GetFileVersionInfo((LPWSTR)szFullPath,
                                       (DWORD)dwVerHnd,
                                       (DWORD)dwVerInfoSize,
                                       (LPVOID)pVersionInfo);
      VS_FIXEDFILEINFO *szVer = NULL;
      UINT uVerLength; 
      if (bRet) {
        // get binary file version information
        bRet = ::VerQueryValue(pVersionInfo, TEXT("\\"),
          (LPVOID*)&szVer, &uVerLength);
        if (bRet) {
          m_dwMajorMinor = szVer->dwProductVersionMS;
          m_dwBuildRevision = szVer->dwProductVersionLS;
        } else {
          m_dwMajorMinor = m_dwBuildRevision = (DWORD)-1;
        }

        struct TRANSARRAY {
          WORD wLangID;
          WORD wCodePage;
        };

        CString cs_text;
        wchar_t *buffer, *lpsztext;
        UINT buflen;
        TRANSARRAY *lpTransArray;

        VerQueryValue(pVersionInfo, L"\\VarFileInfo\\Translation",
                     (LPVOID*)&buffer, &buflen);
        lpTransArray = (TRANSARRAY *)buffer;

        // Save for later
        m_AppLangID = lpTransArray[0].wLangID;
        
        // Until we know better
        m_ResLangID = m_AppLangID;

        // Get string File Version information 
        cs_text.Format(L"\\StringFileInfo\\%04x%04x\\FileVersion",
                       lpTransArray[0].wLangID, lpTransArray[0].wCodePage);
        lpsztext = cs_text.GetBuffer(cs_text.GetLength() + sizeof(wchar_t));
        bRet = ::VerQueryValue(pVersionInfo, lpsztext, (LPVOID*)&buffer, &buflen); 
        m_csFileVersionString = bRet ? buffer : L"";
        cs_text.ReleaseBuffer();

        // Get string Legal Copyright information 
        cs_text.Format(L"\\StringFileInfo\\%04x%04x\\LegalCopyright",
                       lpTransArray[0].wLangID, lpTransArray[0].wCodePage);
        lpsztext = cs_text.GetBuffer(cs_text.GetLength() + sizeof(wchar_t));
        bRet = ::VerQueryValue(pVersionInfo, lpsztext, (LPVOID*)&buffer, &buflen); 
        m_csCopyrightString = bRet ? buffer : L"All rights reserved.";
        cs_text.ReleaseBuffer();
      }
    }
    delete[] pVersionInfo;
  }
}

BOOL CALLBACK ThisMfcApp::searcher(HWND hWnd, LPARAM lParam)
{
  DWORD_PTR dwresult;
  LRESULT ok = ::SendMessageTimeout(hWnd,
                                    m_uiRegMsg,
                                    0, 0, 
                                    SMTO_BLOCK | SMTO_ABORTIFHUNG,
                                    200,
                                    &dwresult);
  if (ok == 0)
    return TRUE; // ignore this and continue

  if (dwresult == m_uiRegMsg) { /* found it */
    HWND *target = (HWND *)lParam;
    *target = hWnd;
    return FALSE; // stop search
  } /* found it */

  return TRUE; // continue search
}

void ThisMfcApp::GetDLLVersionData(const CString &cs_dll, int &wLangID)
{
  wLangID = 0;
  DWORD dwVerHnd, dwVerInfoSize;

  // Get version information from the DLL
  dwVerInfoSize = ::GetFileVersionInfoSize(cs_dll, &dwVerHnd);
  if (dwVerInfoSize > 0) {
    char *pVersionInfo = new char[dwVerInfoSize];
    if (pVersionInfo != NULL) {
      BOOL bRet = ::GetFileVersionInfo((LPCWSTR)cs_dll,
                                       (DWORD)dwVerHnd,
                                       (DWORD)dwVerInfoSize,
                                       (LPVOID)pVersionInfo);

      if (bRet) {
        struct TRANSARRAY {
          WORD wLangID;
          WORD wCodePage;
        };

        wchar_t *buffer;
        UINT buflen;

        VerQueryValue(pVersionInfo, L"\\VarFileInfo\\Translation",
                     (LPVOID*)&buffer, &buflen);
        TRANSARRAY *lpTransArray = (TRANSARRAY *)buffer;
        wLangID = lpTransArray[0].wLangID;
      }
    }
    delete[] pVersionInfo;
  }
}

void ThisMfcApp::GetLanguageFiles()
{
  // NOTE: Assumption that a Help file can only exist if a resource file also exists
  const CString cs_ExePath(PWSdirs::GetExeDir().c_str());
  CString cs_ResPath = cs_ExePath + L"pwsafe*.dll";
  //     L"pwsafeLL.dll" : L"pwsafeLL_CC.dll";

  // Find all language DLLs
  m_vlanguagefiles.clear();

  LANGHELPFILE st_lng;
  const StringX sxLL = PWSprefs::GetInstance()->GetPref(PWSprefs::LanguageFile);

  // Add default embedded language - English
  const LCID AppLCID = MAKELCID(m_AppLangID, SORT_DEFAULT); // 0x0409 - English US

  wchar_t *szLanguage_Native(NULL), *szLanguage_English(NULL);
  wchar_t *szCountry_Native(NULL), *szCountry_English(NULL);

  int inum = ::GetLocaleInfo(AppLCID, LOCALE_SNATIVELANGNAME, szLanguage_Native, 0);
  if (inum > 0) {
    szLanguage_Native = new wchar_t[inum + 1];
    ::GetLocaleInfo(AppLCID, LOCALE_SNATIVELANGNAME, szLanguage_Native, inum);

    st_lng.lcid = AppLCID;
    st_lng.xFlags = (wcscmp(sxLL.c_str(), L"EN") == 0) ? 0xC0 : 0x40;
    st_lng.wsLL = L"EN";
    st_lng.wsCC = L"";
    st_lng.wsLanguage = szLanguage_Native;

    m_vlanguagefiles.push_back(st_lng);
    delete[] szLanguage_Native;
  }

  CFileFind finder;
  BOOL bWorking = finder.FindFile(cs_ResPath);

  while (bWorking) {
    bWorking = finder.FindNextFile();
    CString cs_temp, cs_helpfile;
    CString cs_dll = finder.GetFileName();

    // Parse file name
    CString cs_LL(L""), cs_CC(L""), cs_language;
    //                               012345678901           012345678901234
    // length should be either 10+2 (pwsafeLL.dll) or 10+5 (pwsafeLL_CC.dll)
    int len = cs_dll.GetLength();

    if (len != 12 && len != 15)
      continue;

    cs_LL = cs_dll.Mid(6, 2);
    if (len == 15)
      cs_CC = cs_dll.Mid(9, 2);

    cs_temp.Format(L"pwsafe%s.chm", static_cast<LPCWSTR>(cs_LL));
    cs_helpfile = cs_ExePath + cs_temp;
    std::wstring wsHelpFile = (LPCWSTR)cs_helpfile;
    bool bHelpFileExists = pws_os::FileExists(wsHelpFile);
    int FoundResLangID;
    GetDLLVersionData(cs_ExePath + cs_dll, FoundResLangID);

    // If the Resource-only DLL has same LCID as application, it is back level
    // and we can't use it.
    if (FoundResLangID == m_AppLangID)
      continue;

    // Create LCID
    LCID lcid = MAKELCID(FoundResLangID, SORT_DEFAULT);

    inum = ::GetLocaleInfo(lcid, LOCALE_SNATIVELANGNAME, NULL, 0);
    if (inum > 0) {
      // Get language name in that language
      szLanguage_Native = new wchar_t[inum + 1];
      ::GetLocaleInfo(lcid, LOCALE_SNATIVELANGNAME, szLanguage_Native, inum);

      int jnum = 0;
      if (!cs_CC.IsEmpty()) {
        // Get Country name in that language
        jnum = ::GetLocaleInfo(lcid, LOCALE_SNATIVECTRYNAME, NULL, 0);
        if (jnum > 0) {
          szCountry_Native = new wchar_t[jnum + 1];
          ::GetLocaleInfo(lcid, LOCALE_SNATIVECTRYNAME, szCountry_Native, jnum);
        }
        // Get Country name in English
        int knum = ::GetLocaleInfo(lcid, LOCALE_SENGCOUNTRY, NULL, 0);
        if (knum > 0) {
          szCountry_English = new wchar_t[knum + 1];
          ::GetLocaleInfo(lcid, LOCALE_SENGCOUNTRY, szCountry_English, knum);
        }
      }

      // Now try to make first character of language name in that language upper case
      // Should use LCMapStringEx but that is for Vista + and we support XP & later
      wchar_t *szLanguage_NativeUpper(NULL);
      int iu = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE,
                           szLanguage_Native, inum,
                           szLanguage_NativeUpper, 0);
      if (iu > 0) {
        szLanguage_NativeUpper = new wchar_t[iu + 1];
        iu = LCMapString(lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE,
                         szLanguage_Native, inum,
                         szLanguage_NativeUpper, iu);
        if (szLanguage_NativeUpper[0] != L'?') {
          // Assume all OK and language supports Upper case and is read Left->Right
          // Seems to translate non-Latin characters from Hindi, Punjabi, Hebrew, Korean & Chinese
          // to the same character - so not issue (and one of them reads Right->Left)
          // Must be because they do not have a concept of upper & lower case.
          // Works for Russian Cyrillic alphabet though. Can't promise for future languages!
          szLanguage_Native[0] = szLanguage_NativeUpper[0];
        }
        delete[] szLanguage_NativeUpper;
      }
      // Get language name in English
      int lnum = ::GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, NULL, 0);
      if (lnum > 0) {
        szLanguage_English = new wchar_t[lnum + 1];
        ::GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, szLanguage_English, lnum);
        if (wcscmp(szLanguage_Native, szLanguage_English) != 0) {
          if (jnum > 0)
            cs_language.Format(L"%s - %s (%s - %s)", szLanguage_Native, szCountry_Native, szLanguage_English, szCountry_English);
          else
            cs_language.Format(L"%s (%s)", szLanguage_Native, szLanguage_English);
        } else {
          if (jnum > 0)
            cs_language.Format(L"%s - %s", szLanguage_Native, szCountry_Native);
          else
            cs_language.Format(L"%s", szLanguage_Native);
        }
      } else
        cs_language.Format(L"%s", szLanguage_Native);

      st_lng.lcid = lcid;
      st_lng.xFlags = (m_ResLangID == FoundResLangID) ? 0x80 : 0x00;
      st_lng.xFlags |= bHelpFileExists ? 0x40 : 0x00;
      st_lng.wsLL = (LPCWSTR)cs_LL;
      st_lng.wsCC = (LPCWSTR)cs_CC;
      st_lng.wsLanguage = (LPCWSTR)cs_language;

      m_vlanguagefiles.push_back(st_lng);
      delete[] szLanguage_Native;
      delete[] szLanguage_English;
      delete[] szCountry_Native;
      delete[] szCountry_English;
    }
  }
  finder.Close();
}
