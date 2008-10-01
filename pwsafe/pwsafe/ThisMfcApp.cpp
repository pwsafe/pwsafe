/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "corelib/PWSrand.h"
#include "corelib/PWSdirs.h"
#include "corelib/SysInfo.h"

#if defined(POCKET_PC)
#include "pocketpc/PocketPC.h"
#include "pocketpc/resource.h"
#else
#include <errno.h>
#include <io.h>
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#include "ThisMfcApp.h"
#include "corelib/Util.h"
#include "corelib/BlowFish.h"
#include "DboxMain.h"
#include "SingleInstance.h"

#include "CryptKeyEntry.h"
#include "PWSRecentFileList.h"
#include "corelib/PWSprefs.h"

#include "Shlwapi.h"

#include <vector>
// for finding current propertysheet:
#include "PWPropertyPage.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR *UNIQUE_PWS_GUID =
          _T("PasswordSafe-{3FE0D665-1AE6-49b2-8359-326407D56470}");

const UINT ThisMfcApp::m_uiRegMsg = RegisterWindowMessage(UNIQUE_PWS_GUID);

BEGIN_MESSAGE_MAP(ThisMfcApp, CWinApp)
  //  ON_COMMAND(ID_HELP, CWinApp::OnHelp)
  ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()

ThisMfcApp::ThisMfcApp() :
#if defined(POCKET_PC)
  m_bUseAccelerator( false ),
#else
  m_bUseAccelerator( true ),
#endif
  m_pMRU(NULL), m_TrayLockedState(LOCKED), m_TrayIcon(NULL),
  m_HotKeyPressed(false), m_hMutexOneInstance(NULL)
{
  // {kjp} Temporary until I'm sure that PwsPlatform.h configures the endianness properly
#if defined(POCKET_PC)
  // Double check that *_ENDIAN has been correctly set!
#if defined(PWS_LITTLE_ENDIAN)
  unsigned char buf[4] = { 1, 0, 0, 0 };
  unsigned int ii = 1;
#define ENDIANNESS1 0 // Little
#define ENDIANNESS2 1 // Big
#elif defined(PWS_BIG_ENDIAN)
  unsigned char buf[4] = { 0, 0, 0, 1 };
  unsigned int ii = 1;
#define ENDIANNESS1 1 // Big
#define ENDIANNESS2 0 // Little
#endif
  if (*(unsigned int*)buf != ii) {
    CString cs_msg, cs_e1, cs_e2;
    cs_e1.LoadString(ENDIANNESS1 == 0 ? IDS_LITTLEENDIAN : IDS_BIGENDIAN);
    cs_e2.LoadString(ENDIANNESS2 == 0 ? IDS_LITTLEENDIAN : IDS_BIGENDIAN);
    cs_msg.Format(IDS_ENDIANERROR, cs_e1, cs_e2);
    AfxMessageBox(cs_msg);
  }
#endif
  EnableHtmlHelp();
  CoInitialize(NULL); // Initializes the COM library (for XML processing)
  AfxOleInit();
}

ThisMfcApp::~ThisMfcApp()
{
  delete m_TrayIcon;
  delete m_pMRU;
  delete m_mainmenu;

  // Alhough the system will do this automatically - I like to be clean!
  CloseHandle(m_hMutexOneInstance);

  PWSprefs::DeleteInstance();
  PWSrand::DeleteInstance();
  CoUninitialize(); // Uninitialize COM library

#if !defined(POCKET_PC)
  // WinApp::HtmlHelp asserts that main windows is valid, which (1) isn't true
  // here, and (2) is irrelevant for HH_CLOSE_ALL, so we call ::HtmlHelp
  ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
#endif
}

#if !defined(POCKET_PC)
static void Usage()
{
  AfxMessageBox(IDS_USAGE);
}

// tests if file exists, returns true if so, displays error message if not
static bool CheckFile(const CString &fn)
{
  DWORD status = ::GetFileAttributes(fn);
  CString cs_msg(_T(""));

  if (status == INVALID_FILE_ATTRIBUTES) {
    cs_msg.Format(IDS_FILEERROR1, fn);
  } else if (status & FILE_ATTRIBUTE_DIRECTORY) {
    cs_msg.Format(IDS_FILEERROR2, fn);
  }

  if (cs_msg.IsEmpty()) {
    return true;
  } else {
    AfxMessageBox(cs_msg);
    return false;
  }
}

#endif // !POCKET_PC
int ThisMfcApp::ExitInstance()
{
  if(m_hInstResDLL != NULL)
    FreeLibrary(m_hInstResDLL);

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
  dwVerInfoSize = ::GetFileVersionInfoSize((LPTSTR)(LPCTSTR)csFileName, &dwVerHnd);
  if (dwVerInfoSize > 0) {
    char* pVersionInfo = new char[dwVerInfoSize];
    if(pVersionInfo != NULL) {
      BOOL bRet = ::GetFileVersionInfo((LPTSTR)(LPCTSTR)csFileName,
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

  We will use locale info from ::GetLocaleInfo unless PWS_LANG is defined.

  Search order will be pwsafeLL_CC.dll, followed by pwsafeLL.dll. If neither exist or
  can't be found, the resources embedded in the executable pwsafe.exe will be used 
  (US English i.e. equivalent to pwsafeEN_US.dll)).

  Likewise, we will look for localized versions of pwsafe.chm in GetHelpDir,
  defaulting to pwsafe.chm if not found.
  */

  CString cs_PWS_LANG, cs_LANG, cs_CTRY;
  BOOL bPLRC = cs_PWS_LANG.GetEnvironmentVariable(_T("PWS_LANG"));
  if (bPLRC == TRUE) { // did user override via PWS_LANG env var?
    cs_LANG = cs_PWS_LANG;
    cs_CTRY = _T("");
  } else { // no override, use Locale info
    int inum;
    TCHAR szLang[4], szCtry[4];
    inum = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, szLang, 4);
    ASSERT(inum == 3);
    _tcsupr(szLang);
    TRACE(_T("%s LOCALE_SISO639LANGNAME=%s\n"),
          PWSUtil::GetTimeStamp(), szLang);

    inum = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, szCtry, 4);
    ASSERT(inum == 3);
    TRACE(_T("%s LOCALE_SISO3166CTRYNAME=%s\n"),
          PWSUtil::GetTimeStamp(), szCtry);
    cs_LANG = szLang; cs_CTRY = szCtry;
  }

  const CString cs_ExePath(PWSdirs::GetExeDir().c_str());
  CString cs_ResPath;
  const CString format_string = (cs_CTRY.IsEmpty()) ?
    _T("%spwsafe%s%s.dll") : _T("%spwsafe%s_%s.dll");
  cs_ResPath.Format(format_string, cs_ExePath, cs_LANG, cs_CTRY);
  m_hInstResDLL = LoadLibrary(cs_ResPath);

  if(m_hInstResDLL == NULL) {
    TRACE(_T("%s Could not load language DLLs - using embedded resources.\n"),
          PWSUtil::GetTimeStamp());
  } else { // successfully loaded a resource dll, check version
    DWORD MajorMinor, BuildRevision;
    GetVersionInfoFromFile(cs_ResPath, MajorMinor, BuildRevision);

    if (MajorMinor != GetFileVersionMajorMinor()) { // ignore build for now
      CString oops;
      oops.Format(_T("Executable/language DLL (%s) version mismatch %d/%d.\n"), 
                  cs_ResPath, GetFileVersionMajorMinor(), MajorMinor);
      AfxMessageBox(oops);
      FreeLibrary(m_hInstResDLL);
      m_hInstResDLL = NULL;
    } else { // Passed version check
      TRACE(_T("%s Using language DLL '%s'.\n"),
            PWSUtil::GetTimeStamp(), cs_ResPath);
    }
  } // end of resource dll hunt

  if (m_hInstResDLL != NULL)
    AfxSetResourceHandle(m_hInstResDLL);

  /**
  * So far, we've handle the resource dll. Now go for the compiled helpfile
  * in a similar manner.
  */

  CString cs_HelpPath;
  const CString cs_HelpDir(PWSdirs::GetHelpDir().c_str());
  bool helpFileFound = false;

  CString cs_PWS_HELP;
  BOOL bPHRC = cs_PWS_HELP.GetEnvironmentVariable(_T("PWS_HELP"));
  if (bPHRC == TRUE) {
    cs_HelpPath.Format(_T("%spwsafe%s.chm"), cs_HelpDir, cs_PWS_HELP);
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
      if (m_pszHelpFilePath != NULL) free((void*)m_pszHelpFilePath);
      m_pszHelpFilePath = _tcsdup(cs_HelpPath);
      TRACE(_T("%s Help file overriden by user. Using %s.\n"),
            PWSUtil::GetTimeStamp(), cs_HelpPath);
    }
  }

  if (!helpFileFound) {
    cs_HelpPath.Format(_T("%spwsafe%s_%s.chm"), cs_HelpDir, cs_LANG, cs_CTRY);
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
    }
  }
  if (!helpFileFound) {
    cs_HelpPath.Format(_T("%spwsafe%s.chm"), cs_HelpDir, cs_LANG);
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
    }
  }
  if (!helpFileFound) {
    cs_HelpPath.Format(_T("%spwsafe.chm"), cs_HelpDir);
    if (PathFileExists(cs_HelpPath)) {
      helpFileFound = true;
    }
  }
  if (!helpFileFound) { // last resort
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];
#if _MSC_VER >= 1400
    _tsplitpath_s( m_pszHelpFilePath, NULL, 0, NULL, 0, fname,
      _MAX_FNAME, ext, _MAX_EXT );
    _tcslwr_s(fname, _MAX_FNAME);
    _tcslwr_s(ext, _MAX_EXT);
#else
    _tsplitpath( m_pszHelpFilePath, NULL, NULL, fname, ext );
    _tcslwr(ext);
    _tcslwr(fname);
#endif
    cs_HelpPath.Format(_T("%s%s"), fname, ext);
    TRACE(_T("%s Using help file: %s\n"),
          PWSUtil::GetTimeStamp(), cs_HelpPath);
  }

  if (m_pszHelpFilePath != NULL)
    free((void*)m_pszHelpFilePath);
  m_pszHelpFilePath = _tcsdup(cs_HelpPath);
  TRACE(_T("%s Using help file: %s\n"),
        PWSUtil::GetTimeStamp(), cs_HelpPath);

  m_csHelpFile = cs_HelpPath;
}

bool ThisMfcApp::ParseCommandLine(DboxMain &dbox, bool &allDone)
{
  /*
   * Command line processing:
   * Historically, it appears that if a filename was passed as a commadline argument,
   * the application would prompt the user for the password, and the encrypt or decrypt
   * the named file, based on the file's suffix. Ugh.
   *
   * What I'll do is as follows:
   * If a file is given in the command line, it is used as the database, overriding the
   * registry value. This will allow the user to have several databases, say, one for work
   * and one for personal use, and to set up a different shortcut for each.
   *
   * I think I'll keep the old functionality, but activate it with a "-e" or "-d" flag. (ronys)
   * {kjp} ... and I've removed all of it from the Pocket PC build.
   */

  allDone = false;
#if !defined(POCKET_PC)
  if (m_lpCmdLine[0] != TCHAR('\0')) {
    CString args = m_lpCmdLine;

    // Start command line parsing by pushing each argument into a vector
    // Quotes are stripped here
    vector<CString> argvec;
    int pos = 0;
    CString tok;
    do {
      if (args[pos] == TCHAR('\"'))
        tok = args.Tokenize(_T("\""), pos);
      else
        tok = args.Tokenize(_T(" "),pos);
      if (!tok.IsEmpty())
        argvec.push_back(tok);
    } while (pos != -1 && pos < args.GetLength());

    vector<CString>::iterator arg = argvec.begin();
    bool isEncrypt = false; // for -e/-d handling
    bool startSilent = false;
    bool fileGiven = false;

    while (arg != argvec.end()) {
      // everything starting with '-' is a flag,
      // everything else is the name of a file.
      if ((*arg)[0] == TCHAR('-')) {
        switch ((*arg)[1]) {
        case TCHAR('E'): case TCHAR('e'):
          isEncrypt = true;
        // deliberate fallthru
        case TCHAR('D'): case TCHAR('d'):
          allDone = true; // no need for further processing for -e/-d
        {
          // Make sure there's another argument
          // and it's not a flag, and it is an existing file
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == TCHAR('-') ||
              !CheckFile(*(arg + 1))) {
            Usage();
            return false;
          }
          // get password from user
          StringX passkey;
          CCryptKeyEntry dlg(NULL);
          INT_PTR nResponse = dlg.DoModal();

          if (nResponse == IDOK) {
            passkey = LPCTSTR(dlg.m_cryptkey1);
          } else {
            return false;
          }

          BOOL status;
          if (isEncrypt) {
            status = PWSfile::Encrypt(stringT(*(arg + 1)), passkey);
            if (!status) {
              AfxMessageBox(IDS_ENCRYPTIONFAILED);
            }
            return true;
          } else {
            status = PWSfile::Decrypt(stringT(*(arg+1)), passkey);
            if (!status) {
              // nothing to do - Decrypt displays its own error messages
            }
            return true;
          }
        } // -e or -d flag
        case TCHAR('C'): case TCHAR('c'):
          m_core.SetCurFile(_T(""));
        dbox.SetStartClosed(true);
        break;
        case TCHAR('M'): case TCHAR('m'):// closed & minimized
          m_core.SetCurFile(_T(""));
        dbox.SetStartClosed(true);
        dbox.SetStartSilent(true);
        break;
        case TCHAR('R'): case TCHAR('r'):
          m_core.SetReadOnly(true);
        break;
        case TCHAR('S'): case TCHAR('s'):
          startSilent = true;
        dbox.SetStartSilent(true);
        break;
        case TCHAR('V'): case TCHAR('v'):
          dbox.SetValidate(true);
        break;
        case TCHAR('U'): case TCHAR('u'): // set effective user
          // ensure there's another non-flag argument
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == TCHAR('-')) {
            Usage();
            return FALSE;
          } else {
            arg++;
            SysInfo::GetInstance()->SetEffectiveUser(LPCTSTR(*arg));
          }
        break;
        case TCHAR('H'): case TCHAR('h'): // set effective host
          // ensure there's another non-flag argument
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == TCHAR('-')) {
            Usage();
            return FALSE;
          } else {
            arg++;
            SysInfo::GetInstance()->SetEffectiveHost(LPCTSTR(*arg));
          }
        break;
        case TCHAR('G'): case TCHAR('g'): // override default config file
          // ensure there's another non-flag argument
          if ((arg + 1) == argvec.end() || (arg + 1)[0] == TCHAR('-')) {
            Usage();
            return FALSE;
          } else {
            arg++;
            PWSprefs::SetConfigFile(stringT(*arg));
          }
        break;
        default:
          Usage();
          return false;
        } // switch on flag
      } else { // arg isn't a flag, treat it as a filename
        if (CheckFile(*arg)) {
          fileGiven = true;
          m_core.SetCurFile(arg->GetString());
        } else {
          return false;
        }
      } // if arg is a flag or not
      arg++;
    } // while argvec
    // If start silent && no filename specified, start closed as well
    if (startSilent && !fileGiven)
      dbox.SetStartClosed(true);
  } // Command line not empty
#endif
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
  1. I would love to move this to corelib/PWSprefs.cpp, but it's a protected
  member function!!!
  2. Prior to 3.05, the value was "Counterpane Systems". See PWSprefs.cpp
  for discussion on how this is handled.
  */
  SetRegistryKey(_T("Password Safe"));

  DboxMain dbox(NULL);
  m_core.SetReadOnly(false);
  // Command line parsing MUST be done before the first PWSprefs lookup!
  // (since user/host/config file may be overriden!)
  bool allDone = false;
  bool parseVal = ParseCommandLine(dbox, allDone);

  if (allDone)
    return parseVal ? TRUE : FALSE;
  else if (!parseVal) // bad command line args
    return FALSE;

  // MUST (indirectly) create PWSprefs first
  // Ensures all things like saving locations etc. are set up.
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Check if the user allows muliple instances.
  // For this to apply, consistently, must use the same copy of PasswordSafe
  // configuration file.
  if (!prefs->GetPref(PWSprefs::MultipleInstances)) {
    bool bAlreadyRunning;

    TCHAR szName[MAX_PATH];
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

  // Get application version information
  GetApplicationVersionData();

  LoadLocalizedStuff();

  // PWScore needs it to get into database header if/when saved
  m_core.SetApplicationNameAndVersion(AfxGetAppName(), m_dwMajorMinor);

#if defined(POCKET_PC)
  SHInitExtraControls();
#endif


  if (m_core.GetCurFile().empty())
    m_core.SetCurFile(prefs->GetPref(PWSprefs::CurrentFile));

  int nMRUItems = prefs->GetPref(PWSprefs::MaxMRUItems);
  m_mruonfilemenu = prefs->GetPref(PWSprefs::MRUOnFileMenu);
  m_mainmenu = new CMenu;
  m_mainmenu->LoadMenu(IDR_MAINMENU);

  CMenu new_popupmenu;

  // Menu position indices (File & Close).
  int fpos, cpos;

  // Look for "File" menu.
  fpos = FindMenuItem(m_mainmenu, ID_FILEMENU);
  ASSERT(fpos != -1);

  CMenu* file_submenu = m_mainmenu->GetSubMenu(fpos);
  if (file_submenu != NULL) // Look for "Close Database"
    cpos = FindMenuItem(file_submenu, ID_MENUITEM_CLOSE);
  else
    cpos = -1;

  m_pMRU = new CPWSRecentFileList(0, _T("MRU"), _T("Safe%d"),
                                 ((nMRUItems != 0) ? nMRUItems : 1));
  if (nMRUItems > 0) {
    if (cpos > -1) {
      int irc;
      // Create New Popup Menu
      new_popupmenu.CreatePopupMenu();
      CString cs_recent(MAKEINTRESOURCE(IDS_RECENT));
      CString cs_recentsafes(MAKEINTRESOURCE(IDS_RECENTSAFES));

      if (!m_mruonfilemenu) { // MRU entries in popup menu
        // Insert Item onto new popup
        irc = new_popupmenu.InsertMenu(0, MF_BYPOSITION, 
                                       ID_FILE_MRU_ENTRY1, cs_recent);
        ASSERT(irc != 0);
        // Insert Popup onto main menu
        ASSERT(file_submenu != NULL);
        irc = file_submenu->InsertMenu(cpos + 2,
                                       MF_BYPOSITION | MF_POPUP,
                                       UINT_PTR(new_popupmenu.m_hMenu),
                                       cs_recentsafes);
        ASSERT(irc != 0);
      } else {  // MRU entries inline
        ASSERT(file_submenu != NULL);
        irc = file_submenu->InsertMenu(cpos + 2, MF_BYPOSITION, 
                                       ID_FILE_MRU_ENTRY1, cs_recent);
        ASSERT(irc != 0);
      } // m_mruonfilemenu

      m_pMRU->ReadList();
    } // pos > -1
  } else { // nMRUItems <= 0
    if (cpos > -1) {
      int irc;
      // Remove extra separator
      ASSERT(file_submenu != NULL);
      irc = file_submenu->RemoveMenu(cpos + 1, MF_BYPOSITION);
      ASSERT( irc != 0);
      // Remove Clear MRU menu item.
      irc = file_submenu->RemoveMenu(ID_MENUITEM_CLEAR_MRU, MF_BYCOMMAND);
      ASSERT( irc != 0);
    }
  }

#ifdef DEMO
  // add specific menu item for demo version
  int hpos = FindMenuItem(m_mainmenu, ID_HELPMENU);
  ASSERT(hpos != -1);

  CMenu* help_submenu = m_mainmenu->GetSubMenu(hpos);
  if (help_submenu != NULL) {
    help_submenu->InsertMenu(2, MF_BYPOSITION, ID_MENUITEM_U3SHOP_WEBSITE,
                             CString(MAKEINTRESOURCE(IDS_U3PURCHASE)));
  }
#endif /* DEMO */

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
  m_maindlg = &dbox;
  m_pMainWnd = m_maindlg;

  // JHF : no tray icon and menu for PPC
#if !defined(POCKET_PC)
  //HICON stIcon = app.LoadIcon(IDI_TRAY);
  //ASSERT(stIcon != NULL);
  m_LockedIcon = app.LoadIcon(IDI_LOCKEDICON);
  m_UnLockedIcon = app.LoadIcon(IDI_UNLOCKEDICON);
  int iData = prefs->GetPref(PWSprefs::ClosedTrayIconColour);
  SetClosedTrayIcon(iData);
  m_TrayIcon = new CSystemTray(NULL, WM_ICON_NOTIFY, _T("PasswordSafe"),
                               m_LockedIcon, dbox.m_RUEList,
                               WM_ICON_NOTIFY, IDR_POPTRAY);
  m_TrayIcon->SetTarget(&dbox);

#endif

  // Set up an Accelerator table
#if !defined(POCKET_PC)
  m_ghAccelTable = LoadAccelerators(AfxGetResourceHandle(),
    MAKEINTRESOURCE(IDR_ACCS));
#endif

  CLWnd ListenerWnd(dbox);
  if (SysInfo::IsUnderU3()) {
    // See comment under CLWnd to understand this.
    ListenerWnd.m_hWnd = NULL;
    if (!ListenerWnd.CreateEx(0, AfxRegisterWndClass(0),
                              _T("Pwsafe Listener"),
                              WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL)) {
      ASSERT(0);
      return FALSE;
    } 
  }

  // Run dialog - note that we don't particularly care what the response was
  (void) dbox.DoModal();

  ::DestroyIcon(m_LockedIcon);
  ::DestroyIcon(m_UnLockedIcon);
  ::DestroyIcon(m_ClosedIcon);

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
  CWnd* pMain = AfxGetMainWnd();

  CMenu* xmainmenu = pMain->GetMenu();

  // Look for "File" menu.
  int pos = FindMenuItem(xmainmenu, ID_FILEMENU);

  CMenu* xfile_submenu = xmainmenu->GetSubMenu(pos);
  if (xfile_submenu != NULL)  // Look for MRU first entry
    pos = FindMenuItem(xfile_submenu, ID_FILE_MRU_ENTRY1);
  else
    return;

  if (pos > -1) {
    // Recent databases are on the main File menu - remove them
    for (int nID = numMRU; nID > 1; nID--)
      xfile_submenu->RemoveMenu(ID_FILE_MRU_ENTRY1 + nID - 1, MF_BYCOMMAND);

    return;
  }

  // Recent databases are on the popup menu off the main File menu
  CMenu* xpopupmenu = xfile_submenu->GetSubMenu(3);
  if (xpopupmenu != NULL)  // Look for MRU first entry
    pos = FindMenuItem(xpopupmenu, ID_FILE_MRU_ENTRY1);
  else
    return;

  if (pos > -1) {
    // Recent databases are on a popup menu - remove them
    for (int nID = numMRU; nID > 1; nID--)
      xfile_submenu->RemoveMenu(ID_FILE_MRU_ENTRY1 + nID - 1, MF_BYCOMMAND);

    return;
  }
}

int ThisMfcApp::SetClosedTrayIcon(int &iData, bool bSet)
{
  int icon;
  switch (iData) {
    case PWSprefs::stiBlack:
      icon = IDI_TRAY;  // This is black.
      break;
    case PWSprefs::stiBlue:
      icon = IDI_TRAY_BLUE;
      break;
    case PWSprefs::stiWhite:
      icon = IDI_TRAY_WHITE;
      break;
    case PWSprefs::stiYellow:
      icon = IDI_TRAY_YELLOW;
      break;
    default:
      iData = PWSprefs::stiBlack;
      icon = IDI_TRAY;
      break;
  }
  if (bSet) {
    ::DestroyIcon(m_ClosedIcon);
    m_ClosedIcon = app.LoadIcon(icon);
  }

  return icon;
}

void ThisMfcApp::SetSystemTrayState(STATE s)
{
  // need to protect against null m_TrayIcon due to
  // tricky initialization order
  if (m_TrayIcon != NULL && s != m_TrayLockedState) {
    m_TrayLockedState = s;
    HICON hIcon(m_LockedIcon);
    switch (s) {
      case LOCKED:
        hIcon = m_LockedIcon;
        break;
      case UNLOCKED:
        hIcon = m_UnLockedIcon;
        break;
      case CLOSED:
        hIcon = m_ClosedIcon;
        break;
      default:
        break;
    }
    m_TrayIcon->SetIcon(hIcon);
  }
}

#if !defined(POCKET_PC)
//Copied from Knowledge Base article Q100770
//But not for WinCE {kjp}
BOOL ThisMfcApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
  if (code < 0)
    CWinApp::ProcessMessageFilter(code, lpMsg);

  if (m_bUseAccelerator && m_maindlg != NULL && m_ghAccelTable != NULL &&
      code != MSGF_MENU) {
    if (::TranslateAccelerator(m_maindlg->m_hWnd, m_ghAccelTable, lpMsg))
      return TRUE;
  }

  return CWinApp::ProcessMessageFilter(code, lpMsg);
}
#endif

void ThisMfcApp::OnHelp()
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#mainhelp"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  /*
  * Following mess because CPropertySheet is too smart for its own
  * good. The "Help" button there is mapped to the App framework,
  * and that's that...
  */
  CString cs_title;
  CWnd *wnd = CWnd::GetCapture();
  if (wnd == NULL)
    wnd = CWnd::GetFocus();
  if (wnd != NULL)
    wnd = wnd->GetParent();
  if (wnd != NULL) {
    wnd->GetWindowText(cs_title);
  }
  const CString cs_option_text(MAKEINTRESOURCE(IDS_OPTIONS));
  if (cs_title != cs_option_text) {
    ::HtmlHelp(wnd != NULL ? wnd->m_hWnd : NULL,
               (LPCTSTR)m_csHelpFile, HH_DISPLAY_TOPIC, 0);
  } else { // Options propertysheet - find out active page
    CString helptab;
    CPropertySheet *ps = dynamic_cast<CPropertySheet *>(wnd);
    ASSERT(ps != NULL);
    CPWPropertyPage *pp = dynamic_cast<CPWPropertyPage *>(ps->GetActivePage());
    if (pp != NULL)
      helptab = pp->GetHelpName();
    CString cs_HelpTopic;
    cs_HelpTopic = m_csHelpFile + _T("::/html/") + helptab +_T(".html");
    ::HtmlHelp(NULL, (LPCTSTR)cs_HelpTopic, HH_DISPLAY_TOPIC, 0);
  }

#endif
}

// FindMenuItem() will find a menu item string from the specified
// popup menu and returns its position (0-based) in the specified
// popup menu. It returns -1 if no such menu item string is found.
int ThisMfcApp::FindMenuItem(CMenu* Menu, LPCTSTR MenuString)
{
  ASSERT(Menu);
  ASSERT(::IsMenu(Menu->GetSafeHmenu()));

  int count = Menu->GetMenuItemCount();
  for (int i = 0; i < count; i++) {
    CString str;
    if (Menu->GetMenuString(i, str, MF_BYPOSITION) &&
      (_tcscmp(str, MenuString) == 0))
      return i;
  }

  return -1;
}

// FindMenuItem() will find a menu item ID from the specified
// popup menu and returns its position (0-based) in the specified
// popup menu. It returns -1 if no such menu item string is found.
int ThisMfcApp::FindMenuItem(CMenu* Menu, UINT MenuID)
{
  ASSERT(Menu);
  ASSERT(::IsMenu(Menu->GetSafeHmenu()));

  int count = Menu->GetMenuItemCount();

  // Can't use GetMenuItemID as it does not understand that with the MENUEX
  // format, Popup menus can have IDs
  MENUITEMINFO miinfo;
  memset(&miinfo, 0x00, sizeof(MENUITEMINFO));
  miinfo.cbSize = sizeof(MENUITEMINFO);
  miinfo.fMask = MIIM_ID;                // only want the wID of the menu item

  for (int i = 0; i < count; i++) {
    Menu->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1 && miinfo.wID == MenuID)
      return i;
  }

  return -1;
}

void ThisMfcApp::GetApplicationVersionData()
{
  TCHAR szFullPath[MAX_PATH];
  DWORD dwVerHnd, dwVerInfoSize;

  // Get version information from the application
  ::GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
  dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
  if (dwVerInfoSize > 0) {
    char* pVersionInfo = new char[dwVerInfoSize];
    if (pVersionInfo != NULL) {
      BOOL bRet = ::GetFileVersionInfo((LPTSTR)szFullPath,
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
          WORD wCharSet;
        };

        CString cs_text;
        TCHAR *buffer, *lpsztext;
        UINT buflen;
        TRANSARRAY* lpTransArray;

        VerQueryValue(pVersionInfo, _T("\\VarFileInfo\\Translation"),
                     (LPVOID*)&buffer, &buflen);
        lpTransArray = (TRANSARRAY*) buffer;

        // Get string File Version information 
        cs_text.Format(_T("\\StringFileInfo\\%04x%04x\\FileVersion"),
                       lpTransArray[0].wLangID, lpTransArray[0].wCharSet);
        lpsztext = cs_text.GetBuffer(cs_text.GetLength() + sizeof(TCHAR));
        bRet = ::VerQueryValue(pVersionInfo, lpsztext, (LPVOID*)&buffer, &buflen); 
        m_csFileVersionString = bRet ? buffer : _T("");
        cs_text.ReleaseBuffer();

        // Get string Legal Copyright information 
        cs_text.Format(_T("\\StringFileInfo\\%04x%04x\\LegalCopyright"),
                       lpTransArray[0].wLangID, lpTransArray[0].wCharSet);
        lpsztext = cs_text.GetBuffer(cs_text.GetLength() + sizeof(TCHAR));
        bRet = ::VerQueryValue(pVersionInfo, lpsztext, (LPVOID*)&buffer, &buflen); 
        m_csCopyrightString = bRet ? buffer : _T("All rights reserved.");
        cs_text.ReleaseBuffer();
      }
    }
    delete[] pVersionInfo;
  }
}

BOOL CALLBACK ThisMfcApp::searcher(HWND hWnd, LPARAM lParam)
{
  DWORD result;
  LRESULT ok = ::SendMessageTimeout(hWnd,
                                    m_uiRegMsg,
                                    0, 0, 
                                    SMTO_BLOCK | SMTO_ABORTIFHUNG,
                                    200,
                                    &result);
  if (ok == 0)
    return TRUE; // ignore this and continue

  if (result == m_uiRegMsg) { /* found it */
    HWND *target = (HWND *)lParam;
    *target = hWnd;
    return FALSE; // stop search
  } /* found it */

  return TRUE; // continue search
}
