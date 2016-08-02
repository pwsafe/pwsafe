/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file DboxMain.cpp
//
// The implementation of DboxMain's member functions is spread over source
// files that start with "Main" and reflect the main menu partitioning:
// MainFile.cpp implements the methods pertaining to the functions under
// the File menu, MainView.cpp does the same for the View menu, etc.
//
//This file contains what's left.
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"

#include "AboutDlg.h"
#include "Fonts.h"
#include "MFCMessages.h"
#include "PasskeyEntry.h"
#include "ExpPWListDlg.h"
#include "GeneralMsgBox.h"
#include "InfoDisplay.h"
#include "PasskeySetup.h"

// Set Ctrl/Alt/Shift strings for menus
#include "MenuShortcuts.h"

#include "HKModifiers.h"

// widget override?
#include "SysColStatic.h"

#include "core/core.h"
#include "core/PWSprefs.h"
#include "core/PWSrand.h"
#include "core/PWSdirs.h"
#include "core/PWSFilters.h"
#include "core/PWSAuxParse.h"

#include "core/XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#include "os/file.h"
#include "os/env.h"
#include "os/dir.h"
#include "os/logit.h"
#include "os/lib.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include "psapi.h"    // For EnumProcesses
#include <afxpriv.h>
#include <stdlib.h>   // for qsort
#include <bitset>
#include <algorithm>

// Need to add Windows SDK 6.0 (or later) 'include' and 'lib' libraries to
// Visual Studio "VC++ directories" in their respective search orders to find
// 'WtsApi32.h' and 'WtsApi32.lib'
#include <WtsApi32.h> // For Terminal services to give session changes Lock etc.

// WTS constants
#include <winuser.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Also needed by CInfoDisplay and CPasswordSubsetDlg
extern HRGN GetWorkAreaRegion();
extern BOOL CALLBACK EnumScreens(HMONITOR hMonitor, HDC , LPRECT , LPARAM lParam);

IMPLEMENT_DYNAMIC(DboxMain, CDialog)

/*
* This is the string to be displayed instead of the actual password, unless
* the user chooses to see the password:
*/
const wchar_t *HIDDEN_PASSWORD = L"**************";

// Eyecatcher for looking for the parent within child windows based on
// CPWDialog, CPWFileDialog and CPWPropertySheet
const wchar_t *EYE_CATCHER = L"DBXM";

CString DboxMain::CS_SETFILTERS;
CString DboxMain::CS_CLEARFILTERS;
CString DboxMain::CS_READWRITE;
CString DboxMain::CS_READONLY;

LOGFONT dfltTreeListFont;

void DboxMain::SetLocalStrings()
{
  // Set up static versions of menu items.  Old method was to do a LoadString
  // but then we needed 2 copies - one in StringTable and one in the Menu definitions
  // Both would need to be maintained in step and during I18N.
  // Now get it from the Menu directly
  // VdG set the local strings to the language dependant values
  CS_SETFILTERS.LoadString(IDS_SETFILTERS);
  CS_CLEARFILTERS.LoadString(IDS_CLEARFILTERS);
  CS_READWRITE.LoadString(IDS_CHANGE_READWRITE);
  CS_READONLY.LoadString(IDS_CHANGE_READONLY);
}

//-----------------------------------------------------------------------------
DboxMain::DboxMain(CWnd* pParent)
  : CDialog(DboxMain::IDD, pParent),
  m_pPasskeyEntryDlg(NULL), m_bSizing(false), m_bDBNeedsReading(true), m_bInitDone(false),
  m_toolbarsSetup(FALSE),
  m_bSortAscending(true), m_iTypeSortColumn(CItemData::TITLE),
  m_core(*app.GetCore()),
  m_bTSUpdated(false),
  m_iSessionEndingStatus(IDIGNORE),
  m_pwchTip(NULL),
  m_bOpen(false), 
  m_IsStartClosed(false), m_IsStartSilent(false),
  m_bStartHiddenAndMinimized(false),
  m_bAlreadyToldUserNoSave(false), m_inExit(false),
  m_pCC(NULL), m_bBoldItem(false), m_bIsRestoring(false), m_bImageInLV(false),
  m_lastclipboardaction(L""), m_pNotesDisplay(NULL),
  m_LastFoundTreeItem(NULL), m_LastFoundListItem(-1),
  m_bFilterActive(false), m_bNumPassedFiltering(0),
  m_currentfilterpool(FPOOL_LAST), m_bDoAutoType(false),
  m_sxAutoType(L""), m_pToolTipCtrl(NULL), m_bWSLocked(false), m_bWTSRegistered(false),
  m_savedDBprefs(EMPTYSAVEDDBPREFS), m_bBlockShutdown(false),
  m_pfcnShutdownBlockReasonCreate(NULL), m_pfcnShutdownBlockReasonDestroy(NULL),
  m_bUnsavedDisplayed(false), m_RUEList(*app.GetCore()),
  m_eye_catcher(_wcsdup(EYE_CATCHER)),
  m_hUser32(NULL), m_bInAddGroup(false),
  m_wpDeleteMsg(WM_KEYDOWN), m_wpDeleteKey(VK_DELETE),
  m_wpRenameMsg(WM_KEYDOWN), m_wpRenameKey(VK_F2),
  m_wpAutotypeUPMsg(WM_KEYUP), m_wpAutotypeDNMsg(WM_KEYDOWN), m_wpAutotypeKey('T'),
  m_bDeleteCtrl(false), m_bDeleteShift(false),
  m_bRenameCtrl(false), m_bRenameShift(false),
  m_bAutotypeCtrl(false), m_bAutotypeShift(false),
  m_bInAT(false), m_bInRestoreWindowsData(false), m_bSetup(false), m_bCompareEntries(false),
  m_bInRefresh(false), m_bInRestoreWindows(false), m_bExpireDisplayed(false),
  m_bTellUserExpired(false), m_bInRename(false), m_bWhitespaceRightClick(false),
  m_ilastaction(0), m_bNoValidation(false), m_bDBInitiallyRO(false), m_bViaDCA(false),
  m_LUUIDSelectedAtMinimize(pws_os::CUUID::NullUUID()),
  m_TUUIDSelectedAtMinimize(pws_os::CUUID::NullUUID()),
  m_LUUIDVisibleAtMinimize(pws_os::CUUID::NullUUID()),
  m_TUUIDVisibleAtMinimize(pws_os::CUUID::NullUUID())
{
  // Need to do the following as using the direct calls will fail for Windows versions before Vista
  m_hUser32 = HMODULE(pws_os::LoadLibrary(L"User32.dll", pws_os::LOAD_LIBRARY_SYS));
  if (m_hUser32 != NULL) {
    m_pfcnShutdownBlockReasonCreate = PSBR_CREATE(pws_os::GetFunction(m_hUser32,
                                                                      "ShutdownBlockReasonCreate"));
    m_pfcnShutdownBlockReasonDestroy = PSBR_DESTROY(pws_os::GetFunction(m_hUser32, "ShutdownBlockReasonDestroy"));

    // Do not free library until the end or the addresses may become invalid
    // On the other hand - if either of these addresses are NULL, why keep it?
    if (m_pfcnShutdownBlockReasonCreate == NULL || 
        m_pfcnShutdownBlockReasonDestroy == NULL) {
      // Make both NULL in case only one was
      m_pfcnShutdownBlockReasonCreate = NULL;
      m_pfcnShutdownBlockReasonDestroy = NULL;
      pws_os::FreeLibrary(m_hUser32);
      m_hUser32 = NULL;
    }
  }

  // Set menus to be rebuilt with user's shortcuts
  for (int i = 0; i < NUMPOPUPMENUS; i++) {
    m_bDoShortcuts[i] = true;
  }

  m_hIcon = app.LoadIcon(IDI_CORNERICON);
  m_hIconSm = (HICON) ::LoadImage(app.m_hInstance, MAKEINTRESOURCE(IDI_CORNERICON),
                                  IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

  m_sxSelectedGroup = L"";
  m_sxVisibleGroup = L"";

  ClearData();

  m_titlebar = L"";
  m_toolbarsSetup = FALSE;

  // Zero Autotype bits
  m_btAT.reset();
}

DboxMain::~DboxMain()
{
  std::bitset<UIInterFace::NUM_SUPPORTED> bsSupportedFunctions(0);
  m_core.SetUIInterFace(NULL, UIInterFace::NUM_SUPPORTED, bsSupportedFunctions);

  ::DestroyIcon(m_hIcon);
  ::DestroyIcon(m_hIconSm);

  delete m_pwchTip;
  delete m_pToolTipCtrl;

  pws_os::FreeLibrary(m_hUser32);

  free(m_eye_catcher);
}

LRESULT DboxMain::OnAreYouMe(WPARAM, LPARAM)
{
  return (LRESULT)app.m_uiRegMsg;
}

void DboxMain::RegisterSessionNotification(const bool bRegister)
{
  /**
   * As OS's prior to XP don't support this, we try to resolve entry
   * point manually.
   */

  // For WTSRegisterSessionNotification & WTSUnRegisterSessionNotification
  typedef DWORD (WINAPI *PWTS_RegSN) (HWND, DWORD);
  typedef DWORD (WINAPI *PWTS_UnRegSN) (HWND);

  m_bWTSRegistered = false;
  HMODULE hWTSAPI32 = HMODULE(pws_os::LoadLibrary(L"wtsapi32.dll", pws_os::LOAD_LIBRARY_SYS));
  if (hWTSAPI32 == NULL)
    return;

  if (bRegister) {
    // Register for notifications
    PWTS_RegSN pfcnWTSRegSN = 
      PWTS_RegSN(pws_os::GetFunction(hWTSAPI32,
                                     "WTSRegisterSessionNotification"));

    if (pfcnWTSRegSN == NULL)
      goto exit;

    int num(5);
    while (num > 0) {
      if (pfcnWTSRegSN(GetSafeHwnd(), NOTIFY_FOR_THIS_SESSION) == TRUE) {
        m_bWTSRegistered = true;
        goto exit;
      }

      if (GetLastError() == RPC_S_INVALID_BINDING) {
        // Terminal Services not running!  Wait a very small time and try up to 5 times
        num--;
        ::Sleep(10);
      } else
        break;
    }
  } else {
    // UnRegister for notifications
    PWTS_UnRegSN pfcnWTSUnRegSN =
      PWTS_UnRegSN(pws_os::GetFunction(hWTSAPI32, 
                                       "WTSUnRegisterSessionNotification"));

    if (pfcnWTSUnRegSN != NULL)
      pfcnWTSUnRegSN(GetSafeHwnd());
  }

exit:
  pws_os::FreeLibrary(hWTSAPI32);
  // If successful, no need for Timer
  if (m_bWTSRegistered)
    KillTimer(TIMER_LOCKONWTSLOCK);
}

LRESULT DboxMain::OnWH_SHELL_CallBack(WPARAM wParam, LPARAM )
{
  // We are only being called for HSHELL_WINDOWACTIVATED
  // wParam = Process ID
  // lParam = 0

  bool brc;
  if (!m_bDoAutoType || (m_bDoAutoType && m_sxAutoType.empty())) {
    // Should never happen as we should not be active if not doing AutoType!
    brc = m_runner.UnInit();
    pws_os::Trace(L"DboxMain::OnWH_SHELL_CallBack - Error - AT_HK_UnInitialise : %s\n",
          brc ? L"OK" : L"FAILED");
    // Reset Autotype
    m_bDoAutoType = false;
    m_sxAutoType = L"";
    // Reset Keyboard/Mouse Input
    pws_os::Trace(L"DboxMain::OnWH_SHELL_CallBack - BlockInput reset\n");
    ::BlockInput(FALSE);
    return FALSE;
  }

  // Deactivate us ASAP
  brc = m_runner.UnInit();
  pws_os::Trace(L"DboxMain::OnWH_SHELL_CallBack - AT_HK_UnInitialise after callback : %s\n",
         brc ? L"OK" : L"FAILED");

  // Wait for time to do Autotype - if we can.
  // Check if process still there.
  HANDLE hProcess(NULL);
  if (wParam != 0) {
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)wParam);
  }

  if (hProcess != NULL) {
    pws_os::Trace(L"WaitForInputIdle - Process ID:%d\n", wParam);
    // Don't use INFINITE as may be a problem on user's system and do
    // not want to wait forever - pick a reasonable upperbound, say, 8 seconds?
    switch (WaitForInputIdle(hProcess, 8000)) {
      case 0:
        pws_os::Trace(L"WaitForInputIdle satisfied successfully.\n");
        break;
      case WAIT_TIMEOUT:
        // Must be a problem with the user's process!
        pws_os::Trace(L"WaitForInputIdle time interval expired.\n");
        break;
      case WAIT_FAILED:
        // Problem - wait same amount of time as if could not find process
        pws_os::IssueError(L"WaitForInputIdle", false);
        ::Sleep(2000);
        break;
      default:
        break;
    }
    CloseHandle(hProcess);
  } else {
    // Could not find process - so just wait abitrary amount
    ::Sleep(2000);
  }

  // Do Autotype!  Note: All fields were substituted before getting here
  // Pure guess to wait 1 second.  Might be more or less but certainly > 0
  ::Sleep(1000);
  DoAutoType(m_sxAutoType, m_vactionverboffsets);

  // Reset AutoType
  m_bDoAutoType = false;
  m_sxAutoType = L"";

  return 0L;
}

BEGIN_MESSAGE_MAP(DboxMain, CDialog)
  //{{AFX_MSG_MAP(DboxMain)

  ON_REGISTERED_MESSAGE(app.m_uiRegMsg, OnAreYouMe)
  ON_REGISTERED_MESSAGE(app.m_uiWH_SHELL, OnWH_SHELL_CallBack)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUTOOLBAR_START, ID_MENUTOOLBAR_END, OnUpdateMenuToolbar)

  // File Menu
  ON_COMMAND(ID_MENUITEM_NEW, OnNew)
  ON_COMMAND(ID_MENUITEM_OPEN, OnOpen)
  ON_COMMAND(ID_MENUITEM_CLOSE, OnClose)
  ON_COMMAND(ID_MENUITEM_CLEAR_MRU, OnClearMRU)
  ON_COMMAND(ID_MENUITEM_SAVE, OnSave)
  ON_COMMAND(ID_MENUITEM_SAVEAS, OnSaveAs)
  ON_COMMAND_RANGE(ID_MENUITEM_EXPORT2OLD1XFORMAT, ID_MENUITEM_EXPORT2V2FORMAT, OnExportVx)
  ON_COMMAND_RANGE(ID_MENUITEM_EXPORT2V3FORMAT, ID_MENUITEM_EXPORT2V4FORMAT, OnExportVx)
  ON_COMMAND(ID_MENUITEM_EXPORT2PLAINTEXT, OnExportText)
  ON_COMMAND(ID_MENUITEM_EXPORT2XML, OnExportXML)
  ON_COMMAND(ID_MENUITEM_IMPORT_PLAINTEXT, OnImportText)
  ON_COMMAND(ID_MENUITEM_IMPORT_KEEPASSV1CSV, OnImportKeePassV1CSV)
  ON_COMMAND(ID_MENUITEM_IMPORT_KEEPASSV1TXT, OnImportKeePassV1TXT)
  ON_COMMAND(ID_MENUITEM_IMPORT_XML, OnImportXML)
  ON_COMMAND(ID_MENUITEM_MERGE, OnMerge)
  ON_COMMAND(ID_MENUITEM_COMPARE, OnCompare)
  ON_COMMAND(ID_MENUITEM_SYNCHRONIZE, OnSynchronize)
  ON_COMMAND(ID_MENUITEM_CHANGEMODE, OnChangeMode)
  ON_COMMAND(ID_MENUITEM_PROPERTIES, OnProperties)

  // Edit Menu
  ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
  ON_COMMAND(ID_MENUITEM_ADDGROUP, OnAddGroup)
  ON_COMMAND(ID_MENUITEM_DUPLICATEGROUP, OnDuplicateGroup)
  ON_COMMAND(ID_MENUITEM_CREATESHORTCUT, OnCreateShortcut)
  ON_COMMAND(ID_MENUITEM_EDIT, OnEdit)
  ON_COMMAND(ID_MENUITEM_VIEW, OnEdit)
  ON_COMMAND(ID_MENUITEM_GROUPENTER, OnEdit)
  ON_COMMAND(ID_MENUITEM_BROWSEURL, OnBrowse)
  ON_COMMAND(ID_MENUITEM_SENDEMAIL, OnSendEmail)
  ON_COMMAND(ID_MENUITEM_BROWSEURLPLUS, OnBrowsePlus)
  ON_COMMAND(ID_MENUITEM_COPYPASSWORD, OnCopyPassword)
  ON_COMMAND(ID_MENUITEM_COPYNOTESFLD, OnCopyNotes)
  ON_COMMAND(ID_MENUITEM_COPYUSERNAME, OnCopyUsername)
  ON_COMMAND(ID_MENUITEM_COPYURL, OnCopyURL)
  ON_COMMAND(ID_MENUITEM_COPYEMAIL, OnCopyEmail)
  ON_COMMAND(ID_MENUITEM_COPYRUNCOMMAND, OnCopyRunCommand)
  ON_COMMAND(ID_MENUITEM_CLEARCLIPBOARD, OnClearClipboard)
  ON_COMMAND(ID_MENUITEM_DELETEENTRY, OnDelete)
  ON_COMMAND(ID_MENUITEM_DELETEGROUP, OnDelete)
  ON_COMMAND(ID_MENUITEM_RENAME, OnRename)
  ON_COMMAND(ID_MENUITEM_RENAMEENTRY, OnRename)
  ON_COMMAND(ID_MENUITEM_RENAMEGROUP, OnRename)
  ON_COMMAND(ID_MENUITEM_DUPLICATEENTRY, OnDuplicateEntry)
  ON_COMMAND(ID_MENUITEM_AUTOTYPE, OnAutoType)
  ON_COMMAND(ID_MENUITEM_GOTOBASEENTRY, OnGotoBaseEntry)
  ON_COMMAND(ID_MENUITEM_RUNCOMMAND, OnRunCommand)
  ON_COMMAND(ID_MENUITEM_EDITBASEENTRY, OnEditBaseEntry)
  ON_COMMAND(ID_MENUITEM_UNDO, OnUndo)
  ON_COMMAND(ID_MENUITEM_REDO, OnRedo)
  ON_COMMAND(ID_MENUITEM_EXPORTENT2PLAINTEXT, OnExportEntryText)
  ON_COMMAND(ID_MENUITEM_EXPORTENT2XML, OnExportEntryXML)
  ON_COMMAND(ID_MENUITEM_COMPARE_ENTRIES, OnCompareEntries)
  ON_COMMAND_RANGE(ID_MENUITEM_PROTECT, ID_MENUITEM_UNPROTECTGROUP, OnProtect)
  ON_COMMAND(ID_MENUITEM_EXPORTENT2DB, OnExportEntryDB)
  ON_COMMAND(ID_MENUITEM_EXPORTGRP2PLAINTEXT, OnExportGroupText)
  ON_COMMAND(ID_MENUITEM_EXPORTGRP2XML, OnExportGroupXML)
  ON_COMMAND(ID_MENUITEM_EXPORTGRP2DB, OnExportGroupDB)
  ON_COMMAND(ID_MENUITEM_EXPORT_ATTACHMENT, OnExportAttachment)

  // View Menu
  ON_COMMAND(ID_MENUITEM_LIST_VIEW, OnListView)
  ON_COMMAND(ID_MENUITEM_TREE_VIEW, OnTreeView)
  ON_COMMAND(ID_MENUITEM_SHOWHIDE_TOOLBAR, OnShowHideToolbar)
  ON_COMMAND(ID_MENUITEM_SHOWHIDE_DRAGBAR, OnShowHideDragbar)
  ON_COMMAND(ID_MENUITEM_OLD_TOOLBAR, OnOldToolbar)
  ON_COMMAND(ID_MENUITEM_NEW_TOOLBAR, OnNewToolbar)
  ON_COMMAND(ID_MENUITEM_SHOWFINDTOOLBAR, OnShowFindToolbar)
  ON_COMMAND(ID_MENUITEM_FINDELLIPSIS, OnShowFindToolbar)
  ON_COMMAND(ID_MENUITEM_EXPANDALL, OnExpandAll)
  ON_COMMAND(ID_MENUITEM_COLLAPSEALL, OnCollapseAll)
  ON_COMMAND(ID_MENUITEM_CHANGETREEFONT, OnChangeTreeFont)
  ON_COMMAND(ID_MENUITEM_CHANGEADDEDITFONT, OnChangeAddEditFont)
  ON_COMMAND(ID_MENUITEM_CHANGEPSWDFONT, OnChangePswdFont)
  ON_COMMAND(ID_MENUITEM_VKEYBOARDFONT, OnChangeVKFont)
  ON_COMMAND(ID_MENUITEM_CHANGENOTESFONT, OnChangeNotesFont)
  ON_COMMAND_RANGE(ID_MENUITEM_REPORT_COMPARE, ID_MENUITEM_REPORT_VALIDATE, OnViewReportsByID)
  ON_COMMAND_RANGE(ID_MENUITEM_REPORT_SYNCHRONIZE, ID_MENUITEM_REPORT_SYNCHRONIZE, OnViewReportsByID)
  ON_COMMAND_RANGE(ID_MENUITEM_REPORT_EXPORTTEXT, ID_MENUITEM_REPORT_EXPORTXML, OnViewReportsByID)
  ON_COMMAND_RANGE(ID_MENUITEM_REPORT_EXPORTDB, ID_MENUITEM_REPORT_EXPORTDB, OnViewReportsByID)
  ON_COMMAND_RANGE(ID_MENUITEM_REPORT_IMPORTKP1TXT, ID_MENUITEM_REPORT_IMPORTKP1CSV, OnViewReportsByID)
  ON_COMMAND(ID_MENUITEM_APPLYFILTER, OnApplyFilter)
  ON_COMMAND(ID_MENUITEM_CLEARFILTER, OnApplyFilter)
  ON_COMMAND(ID_MENUITEM_EDITFILTER, OnSetFilter)
  ON_COMMAND(ID_MENUITEM_MANAGEFILTERS, OnManageFilters)
  ON_COMMAND(ID_MENUITEM_PASSWORDSUBSET, OnDisplayPswdSubset)
  ON_COMMAND(ID_MENUITEM_REFRESH, OnRefreshWindow)
  ON_COMMAND(ID_MENUITEM_SHOWHIDE_UNSAVED, OnShowUnsavedEntries)
  ON_COMMAND(ID_MENUITEM_SHOW_ALL_EXPIRY, OnShowExpireList)

  // Manage Menu
  ON_COMMAND(ID_MENUITEM_CHANGECOMBO, OnPassphraseChange)
  ON_COMMAND(ID_MENUITEM_BACKUPSAFE, OnBackupSafe)
  ON_COMMAND(ID_MENUITEM_RESTORESAFE, OnRestoreSafe)
  ON_COMMAND(ID_MENUITEM_OPTIONS, OnOptions)
  ON_COMMAND(ID_MENUITEM_GENERATEPASSWORD, OnGeneratePassword)
  ON_COMMAND(ID_MENUITEM_YUBIKEY, OnYubikey)
  ON_COMMAND(ID_MENUITEM_PSWD_POLICIES, OnManagePasswordPolicies)

  // Help Menu
  ON_COMMAND(ID_MENUITEM_ABOUT, OnAbout)
  ON_COMMAND(ID_MENUITEM_PWSAFE_WEBSITE, OnPasswordSafeWebsite)
  ON_COMMAND(ID_MENUITEM_HELP, OnHelp)
  ON_COMMAND(ID_HELP, OnHelp)

  // List view Column Picker
  ON_COMMAND(ID_MENUITEM_COLUMNPICKER, OnColumnPicker)
  ON_COMMAND(ID_MENUITEM_RESETCOLUMNS, OnResetColumns)

  // Others
  // Double-click on R-O R/W indicator on StatusBar
  ON_COMMAND(IDS_READ_ONLY, OnChangeMode)
  // Double-click on filter indicator on StatusBar
  ON_COMMAND(IDB_FILTER_ACTIVE, OnCancelFilter)

  ON_WM_CONTEXTMENU()

  // Windows Messages
  ON_WM_DESTROY()
  ON_WM_INITMENU()
  ON_WM_INITMENUPOPUP()
  ON_WM_MOVE()
  ON_WM_SIZE()
  ON_WM_SYSCOMMAND()
  ON_WM_TIMER()
  ON_WM_WINDOWPOSCHANGING()

  ON_WM_DRAWITEM()
  ON_WM_MEASUREITEM()

  ON_NOTIFY(NM_CLICK, IDC_ITEMLIST, OnListItemSelected)
  ON_NOTIFY(NM_CLICK, IDC_ITEMTREE, OnTreeItemSelected)
  ON_NOTIFY(LVN_KEYDOWN, IDC_ITEMLIST, OnKeydownItemlist)
  ON_NOTIFY(NM_DBLCLK, IDC_ITEMLIST, OnItemDoubleClick)
  ON_NOTIFY(NM_DBLCLK, IDC_ITEMTREE, OnItemDoubleClick)
  ON_NOTIFY(LVN_COLUMNCLICK, IDC_ITEMLIST, OnColumnClick)
  ON_NOTIFY(NM_RCLICK, IDC_LIST_HEADER, OnHeaderRClick)
  ON_NOTIFY(HDN_BEGINDRAG, IDC_LIST_HEADER, OnHeaderBeginDrag)
  ON_NOTIFY(HDN_ENDDRAG, IDC_LIST_HEADER, OnHeaderEndDrag)
  ON_NOTIFY(HDN_ENDTRACK, IDC_LIST_HEADER, OnHeaderNotify)
  ON_NOTIFY(HDN_ITEMCHANGED, IDC_LIST_HEADER, OnHeaderNotify)

  ON_COMMAND(ID_MENUITEM_EXIT, OnOK)
  ON_COMMAND(ID_MENUITEM_MINIMIZE, OnMinimize)
  ON_COMMAND(ID_MENUITEM_RESTORE, OnRestore)

  ON_WM_SIZING()
  ON_COMMAND(ID_MENUITEM_TRAYLOCK, OnTrayLockUnLock)
  ON_COMMAND(ID_MENUITEM_TRAYUNLOCK, OnTrayLockUnLock)
  ON_COMMAND(ID_MENUITEM_CLEARRECENTENTRIES, OnTrayClearRecentEntries)
  ON_COMMAND(ID_TOOLBUTTON_LISTTREE, OnToggleView)
  ON_COMMAND(ID_TOOLBUTTON_VIEWREPORTS, OnViewReports)

  ON_COMMAND(ID_TOOLBUTTON_CLOSEFIND, OnHideFindToolBar)
  ON_COMMAND(ID_MENUITEM_FIND, OnToolBarFind)
  ON_COMMAND(ID_MENUITEM_FINDUP, OnToolBarFindUp)
  ON_COMMAND(ID_TOOLBUTTON_FINDCASE, OnToolBarFindCase)
  ON_COMMAND(ID_TOOLBUTTON_FINDCASE_I, OnToolBarFindCase)
  ON_COMMAND(ID_TOOLBUTTON_FINDCASE_S, OnToolBarFindCase)
  ON_COMMAND(ID_TOOLBUTTON_FINDADVANCED, OnToolBarFindAdvanced)
  ON_COMMAND(ID_TOOLBUTTON_FINDREPORT, OnToolBarFindReport)
  ON_COMMAND(ID_TOOLBUTTON_CLEARFIND, OnToolBarClearFind)
  ON_BN_CLICKED(IDOK, OnEdit)

  ON_MESSAGE(WM_WTSSESSION_CHANGE, OnSessionChange)
  ON_MESSAGE(PWS_MSG_ICON_NOTIFY, OnTrayNotification)
  ON_MESSAGE(WM_HOTKEY, OnHotKey)
  ON_MESSAGE(PWS_MSG_CCTOHDR_DD_COMPLETE, OnCCToHdrDragComplete)
  ON_MESSAGE(PWS_MSG_HDRTOCC_DD_COMPLETE, OnHdrToCCDragComplete)
  ON_MESSAGE(PWS_MSG_HDR_DRAG_COMPLETE, OnHeaderDragComplete)
  ON_MESSAGE(PWS_MSG_COMPARE_RESULT_FUNCTION, OnProcessCompareResultFunction)
  ON_MESSAGE(PWS_MSG_COMPARE_RESULT_ALLFNCTN, OnProcessCompareResultAllFunction)
  ON_MESSAGE(PWS_MSG_EXPIRED_PASSWORD_EDIT, OnEditExpiredPasswordEntry)
  ON_MESSAGE(PWS_MSG_TOOLBAR_FIND, OnToolBarFindMessage)
  ON_MESSAGE(PWS_MSG_DRAGAUTOTYPE, OnDragAutoType)
  ON_MESSAGE(PWS_MSG_EXECUTE_FILTERS, OnExecuteFilters)
  ON_MESSAGE(PWS_MSG_EDIT_APPLY, OnApplyEditChanges)
  ON_MESSAGE(PWS_MSG_DROPPED_FILE, OnDroppedFile)
  ON_MESSAGE(WM_QUERYENDSESSION, OnQueryEndSession)
  ON_MESSAGE(WM_ENDSESSION, OnEndSession)

  ON_COMMAND(ID_MENUITEM_CUSTOMIZETOOLBAR, OnCustomizeToolbar)

  //}}AFX_MSG_MAP
  ON_COMMAND_EX_RANGE(ID_FILE_MRU_ENTRY1, ID_FILE_MRU_ENTRYMAX, OnOpenMRU)
  ON_UPDATE_COMMAND_UI(ID_FILE_MRU_ENTRY1, OnUpdateMRU)  // Note: can't be in OnUpdateMenuToolbar!
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYUSERNAME1, ID_MENUITEM_TRAYCOPYUSERNAMEMAX, OnTrayCopyUsername)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYUSERNAME1, ID_MENUITEM_TRAYCOPYUSERNAMEMAX, OnUpdateTrayCopyUsername)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYPASSWORD1, ID_MENUITEM_TRAYCOPYPASSWORDMAX, OnTrayCopyPassword)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYPASSWORD1, ID_MENUITEM_TRAYCOPYPASSWORDMAX, OnUpdateTrayCopyPassword)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYNOTES1, ID_MENUITEM_TRAYCOPYNOTESMAX, OnTrayCopyNotes)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYNOTES1, ID_MENUITEM_TRAYCOPYNOTESMAX, OnUpdateTrayCopyNotes)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYBROWSE1, ID_MENUITEM_TRAYBROWSEMAX, OnTrayBrowse)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYBROWSE1, ID_MENUITEM_TRAYBROWSEMAX, OnUpdateTrayBrowse)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYDELETE1, ID_MENUITEM_TRAYDELETEMAX, OnTrayDeleteEntry)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYDELETE1, ID_MENUITEM_TRAYDELETEMAX, OnUpdateTrayDeleteEntry)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYAUTOTYPE1, ID_MENUITEM_TRAYAUTOTYPEMAX, OnTrayAutoType)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYAUTOTYPE1, ID_MENUITEM_TRAYAUTOTYPEMAX, OnUpdateTrayAutoType)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYURL1, ID_MENUITEM_TRAYCOPYURLMAX, OnTrayCopyURL)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYURL1, ID_MENUITEM_TRAYCOPYURLMAX, OnUpdateTrayCopyURL)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYRUNCMD1, ID_MENUITEM_TRAYRUNCMDMAX, OnTrayRunCommand)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYRUNCMD1, ID_MENUITEM_TRAYRUNCMDMAX, OnUpdateTrayRunCommand)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYBROWSEPLUS1, ID_MENUITEM_TRAYBROWSEPLUSMAX, OnTrayBrowse)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYBROWSEPLUS1, ID_MENUITEM_TRAYBROWSEPLUSMAX, OnUpdateTrayBrowse)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYEMAIL1, ID_MENUITEM_TRAYCOPYEMAILMAX, OnTrayCopyEmail)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYEMAIL1, ID_MENUITEM_TRAYCOPYEMAILMAX, OnUpdateTrayCopyEmail)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYSENDEMAIL1, ID_MENUITEM_TRAYSENDEMAILMAX, OnTraySendEmail)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYSENDEMAIL1, ID_MENUITEM_TRAYSENDEMAILMAX, OnUpdateTraySendEmail)
  ON_COMMAND_RANGE(ID_MENUITEM_TRAYSELECT1, ID_MENUITEM_TRAYSELECTMAX, OnTraySelect)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYSELECT1, ID_MENUITEM_TRAYSELECTMAX, OnUpdateTraySelect)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

// Command ID, OpenRW, OpenRO, Empty, Closed
const DboxMain::UICommandTableEntry DboxMain::m_UICommandTable[] = {
  // File menu
  {ID_MENUITEM_OPEN, true, true, true, true},
  {ID_MENUITEM_NEW, true, true, true, true},
  {ID_MENUITEM_CLOSE, true, true, true, false},
  {ID_MENUITEM_SAVE, true, false, true, false},
  {ID_MENUITEM_SAVEAS, true, true, true, false},
  {ID_MENUITEM_CLEAR_MRU, true, true, true, true},
  {ID_MENUITEM_EXPORT2PLAINTEXT, true, true, false, false},
  {ID_MENUITEM_EXPORT2OLD1XFORMAT, true, true, false, false},
  {ID_MENUITEM_EXPORT2V2FORMAT, true, true, false, false},
  {ID_MENUITEM_EXPORT2V3FORMAT, true, true, false, false},
  {ID_MENUITEM_EXPORT2V4FORMAT, true, true, false, false},
  {ID_MENUITEM_EXPORT2XML, true, true, false, false},
  {ID_MENUITEM_IMPORT_XML, true, false, true, false},
  {ID_MENUITEM_IMPORT_PLAINTEXT, true, false, true, false},
  {ID_MENUITEM_IMPORT_KEEPASSV1CSV, true, false, true, false},
  {ID_MENUITEM_IMPORT_KEEPASSV1TXT, true, false, true, false},
  {ID_MENUITEM_MERGE, true, false, true, false},
  {ID_MENUITEM_COMPARE, true, true, false, false},
  {ID_MENUITEM_SYNCHRONIZE, true, false, false, false},
  {ID_MENUITEM_CHANGEMODE, true, true, false, false},
  {ID_MENUITEM_PROPERTIES, true, true, true, false},
  {ID_MENUITEM_EXIT, true, true, true, true},
  // Edit menu
  {ID_MENUITEM_ADD, true, false, true, false},
  {ID_MENUITEM_EDIT, true, true, false, false},
  {ID_MENUITEM_VIEW, true, true, false, false},
  {ID_MENUITEM_GROUPENTER, true, true, false, false},
  {ID_MENUITEM_DELETEENTRY, true, false, false, false},
  {ID_MENUITEM_DELETEGROUP, true, false, false, false},
  {ID_MENUITEM_RENAMEENTRY, true, false, false, false},
  {ID_MENUITEM_RENAMEGROUP, true, false, false, false},
  {ID_MENUITEM_FINDELLIPSIS, true, true, false, false},
  {ID_MENUITEM_FIND, true, true, false, false},
  {ID_MENUITEM_FINDUP, true, true, false, false},
  {ID_MENUITEM_DUPLICATEENTRY, true, false, false, false},
  {ID_MENUITEM_ADDGROUP, true, false, true, false},
  {ID_MENUITEM_DUPLICATEGROUP, true, false, false, false},
  {ID_MENUITEM_PROTECT, true, false, false, false},
  {ID_MENUITEM_UNPROTECT, true, false, false, false},
  {ID_MENUITEM_PROTECTGROUP, true, false, false, false},
  {ID_MENUITEM_UNPROTECTGROUP, true, false, false, false},
  {ID_MENUITEM_COPYPASSWORD, true, true, false, false},
  {ID_MENUITEM_COPYUSERNAME, true, true, false, false},
  {ID_MENUITEM_COPYNOTESFLD, true, true, false, false},
  {ID_MENUITEM_CLEARCLIPBOARD, true, true, true, false},
  {ID_MENUITEM_BROWSEURL, true, true, false, false},
  {ID_MENUITEM_BROWSEURLPLUS, true, true, false, false},
  {ID_MENUITEM_SENDEMAIL, true, true, false, false},
  {ID_MENUITEM_AUTOTYPE, true, true, false, false},
  {ID_MENUITEM_RUNCOMMAND, true, true, false, false},
  {ID_MENUITEM_COPYURL, true, true, false, false},
  {ID_MENUITEM_COPYEMAIL, true, true, false, false},
  {ID_MENUITEM_COPYRUNCOMMAND, true, true, false, false},
  {ID_MENUITEM_GOTOBASEENTRY, true, true, false, false},
  {ID_MENUITEM_CREATESHORTCUT, true, false, false, false},
  {ID_MENUITEM_EDITBASEENTRY, true, true, false, false},
  {ID_MENUITEM_UNDO, true, false, true, false},
  {ID_MENUITEM_REDO, true, false, true, false},
  {ID_MENUITEM_EXPORTENT2PLAINTEXT, true, true, false, false},
  {ID_MENUITEM_EXPORTENT2XML, true, true, false, false},
  {ID_MENUITEM_COMPARE_ENTRIES, true, true, false, false},
  {ID_MENUITEM_EXPORTENT2DB, true, true, false, false},
  {ID_MENUITEM_EXPORTGRP2PLAINTEXT, true, true, false, false},
  {ID_MENUITEM_EXPORTGRP2XML, true, true, false, false},
  {ID_MENUITEM_EXPORTGRP2DB, true, true, false, false},
  {ID_MENUITEM_EXPORT_ATTACHMENT, true, true, false, false},
  // View menu
  {ID_MENUITEM_LIST_VIEW, true, true, true, false},
  {ID_MENUITEM_TREE_VIEW, true, true, true, false},
  {ID_MENUITEM_SHOWHIDE_TOOLBAR, true, true, true, true},
  {ID_MENUITEM_SHOWHIDE_DRAGBAR, true, true, true, true},
  {ID_MENUITEM_NEW_TOOLBAR, true, true, true, true},
  {ID_MENUITEM_OLD_TOOLBAR, true, true, true, true},
  {ID_MENUITEM_SHOWFINDTOOLBAR, true, true, false, false},
  {ID_MENUITEM_EXPANDALL, true, true, true, false},
  {ID_MENUITEM_COLLAPSEALL, true, true, true, false},
  {ID_MENUITEM_CHANGETREEFONT, true, true, true, true},
  {ID_MENUITEM_CHANGEADDEDITFONT, true, true, true, true},
  {ID_MENUITEM_CHANGEPSWDFONT, true, true, true, true},
  {ID_MENUITEM_CHANGENOTESFONT, true, true, true, true},
  {ID_MENUITEM_VKEYBOARDFONT, true, true, true, true},
  {ID_MENUITEM_REPORT_COMPARE, true, true, true, false},
  {ID_MENUITEM_REPORT_FIND, true, true, true, false},
  {ID_MENUITEM_REPORT_IMPORTTEXT, true, true, true, false},
  {ID_MENUITEM_REPORT_IMPORTKP1CSV, true, true, true, false},
  {ID_MENUITEM_REPORT_IMPORTKP1TXT, true, true, true, false},
  {ID_MENUITEM_REPORT_IMPORTXML, true, true, true, false},
  {ID_MENUITEM_REPORT_EXPORTTEXT, true, true, true, false},
  {ID_MENUITEM_REPORT_EXPORTXML, true, true, true, false},
  {ID_MENUITEM_REPORT_EXPORTDB, true, true, true, false},
  {ID_MENUITEM_REPORT_MERGE, true, true, true, false},
  {ID_MENUITEM_REPORT_VALIDATE, true, true, true, false},
  {ID_MENUITEM_REPORT_SYNCHRONIZE, true, true, true, false},
  {ID_MENUITEM_EDITFILTER, true, true, false, false},
  {ID_MENUITEM_APPLYFILTER, true, true, false, false},
  {ID_MENUITEM_CLEARFILTER, true, true, false, false},
  {ID_MENUITEM_MANAGEFILTERS, true, true, true, true},
  {ID_MENUITEM_PASSWORDSUBSET, true, true, false, false},
  {ID_MENUITEM_REFRESH, true, true, false, false},
  {ID_MENUITEM_SHOWHIDE_UNSAVED, true, false, false, false},
  {ID_MENUITEM_SHOW_ALL_EXPIRY, true, true, false, false},
  // Manage menu
  {ID_MENUITEM_CHANGECOMBO, true, false, true, false},
  {ID_MENUITEM_BACKUPSAFE, true, true, true, false},
  {ID_MENUITEM_RESTORESAFE, true, false, true, false},
  {ID_MENUITEM_OPTIONS, true, true, true, true},
  {ID_MENUITEM_GENERATEPASSWORD, true, true, true, true},
  {ID_MENUITEM_YUBIKEY, true, false, true, false},
  {ID_MENUITEM_PSWD_POLICIES, true, true, true, false},
  // Help Menu
  {ID_MENUITEM_PWSAFE_WEBSITE, true, true, true, true},
  {ID_MENUITEM_ABOUT, true, true, true, true},
  {ID_MENUITEM_HELP, true, true, true, true},
  {ID_HELP, true, true, true, true},
  // Column popup menu
  {ID_MENUITEM_COLUMNPICKER, true, true, true, false},
  {ID_MENUITEM_RESETCOLUMNS, true, true, true, false},
  // Compare popup menu
  {ID_MENUITEM_COMPVIEWEDIT, true, true, true, false},
  {ID_MENUITEM_COPY_TO_ORIGINAL, true, false, true, false},
  {ID_MENUITEM_COPYALL_TO_ORIGINAL, true, false, true, false},
  {ID_MENUITEM_SYNCHRONIZEALL, true, false, true, false},
  // Tray popup menu
  {ID_MENUITEM_TRAYUNLOCK, true, true, true, false},
  {ID_MENUITEM_TRAYLOCK, true, true, true, false},
  {ID_MENUITEM_CLEARRECENTENTRIES, true, true, true, false},
  {ID_MENUITEM_MINIMIZE, true, true, true, true},
  {ID_MENUITEM_RESTORE, true, true, true, true},
  // Default Main Toolbar buttons - if not menu items
  //   None
  // Optional Main Toolbar buttons
  {ID_TOOLBUTTON_LISTTREE, true, true, true, false},
  {ID_TOOLBUTTON_VIEWREPORTS, true, true, true, false},
  // Find Toolbar
  {ID_TOOLBUTTON_CLOSEFIND, true, true, true, false},
  {ID_TOOLBUTTON_FINDEDITCTRL, true, true, false, false},
  {ID_TOOLBUTTON_FINDCASE, true, true, false, false},
  {ID_TOOLBUTTON_FINDCASE_I, true, true, false, false},
  {ID_TOOLBUTTON_FINDCASE_S, true, true, false, false},
  {ID_TOOLBUTTON_FINDADVANCED, true, true, false, false},
  {ID_TOOLBUTTON_FINDREPORT, true, true, false, false},
  {ID_TOOLBUTTON_CLEARFIND, true, true, false, false},
  // Customize Main Toolbar
  {ID_MENUITEM_CUSTOMIZETOOLBAR, true, true, true, true},
};

void DboxMain::InitPasswordSafe()
{
  PWS_LOGIT;

  PWSprefs *prefs = PWSprefs::GetInstance();
  // Real initialization done here
  // Requires OnInitDialog to have passed OK
  UpdateAlwaysOnTop();
  UpdateSystemMenu();

  // ... same for UseSystemTray
  // StartSilent trumps preference (but StartClosed doesn't)
  if (!m_IsStartSilent && !prefs->GetPref(PWSprefs::UseSystemTray))
    app.HideIcon();

  m_RUEList.SetMax(prefs->GetPref(PWSprefs::MaxREItems));

  const int32 iPWSHotKeyValue = int32(prefs->GetPref(PWSprefs::HotKey));
  WORD wVirtualKeyCode =  iPWSHotKeyValue & 0xff;
  WORD wHKModifiers = iPWSHotKeyValue >> 16;
    
  // Translate from CHotKeyCtrl to CWnd & PWS modifiers
  WORD wModifiers = ConvertModifersMFC2Windows(wHKModifiers);
  WORD wPWSModifiers = ConvertModifersMFC2PWS(wHKModifiers);
  int iAppShortcut = (wPWSModifiers << 16) + wVirtualKeyCode;
  m_core.SetAppHotKey(iAppShortcut);

  // Set Hotkey, if active
  if (prefs->GetPref(PWSprefs::HotKeyEnabled)) {
    RegisterHotKey(m_hWnd, PWS_HOTKEY_ID, UINT(wModifiers), UINT(wVirtualKeyCode));
    // Registration might fail if combination already registered elsewhere,
    // but don't see any elegant way to notify the user here, so fail silently
  } else {
    // No sense in unregistering at this stage, really.
  }

  m_bInitDone = true;

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog

  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIconSm, FALSE); // Set small icon

  // Init stuff for tree view
  CBitmap bitmap;
  BITMAP bm;

  // Change all pixels in this 'grey' to transparent
  const COLORREF crTransparent = RGB(192, 192, 192);

  bitmap.LoadBitmap(IDB_GROUP);
  bitmap.GetBitmap(&bm);
  
  m_pImageList = new CImageList();
  // Number (12) corresponds to number in CPWTreeCtrl public enum
  BOOL status = m_pImageList->Create(bm.bmWidth, bm.bmHeight, 
                                     ILC_MASK | ILC_COLORDDB, 
                                     CPWTreeCtrl::NUM_IMAGES, 0);
  ASSERT(status != 0);

  // Dummy Imagelist needed if user adds then removes Icon column
  m_pImageList0 = new CImageList();
  status = m_pImageList0->Create(1, 1, ILC_MASK | ILC_COLOR, 0, 1);
  ASSERT(status != 0);

  // Order of LoadBitmap() calls matches CPWTreeCtrl public enum
  // Also now used by CListCtrl!
  //bitmap.LoadBitmap(IDB_GROUP); - already loaded above to get width
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  UINT bitmapResIDs[] = {
    IDB_NORMAL, IDB_NORMAL_WARNEXPIRED, IDB_NORMAL_EXPIRED,
    IDB_ABASE, IDB_ABASE_WARNEXPIRED, IDB_ABASE_EXPIRED,
    IDB_ALIAS,
    IDB_SBASE, IDB_SBASE_WARNEXPIRED, IDB_SBASE_EXPIRED,
    IDB_SHORTCUT,
    IDB_EMPTY_GROUP
  };

  for (int i = 0; i < sizeof(bitmapResIDs) / sizeof(bitmapResIDs[0]); i++) {
    bitmap.LoadBitmap(bitmapResIDs[i]);
    m_pImageList->Add(&bitmap, crTransparent);
    bitmap.DeleteObject();
  }

  m_ctlItemTree.SetImageList(m_pImageList, TVSIL_NORMAL);
  m_ctlItemTree.SetImageList(m_pImageList, TVSIL_STATE);

  bool showNotes = prefs->GetPref(PWSprefs::ShowNotesAsTooltipsInViews);
  m_ctlItemTree.ActivateND(showNotes);
  m_ctlItemList.ActivateND(showNotes);

  DWORD dw_ExtendedStyle = LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP;
  if (prefs->GetPref(PWSprefs::ListViewGridLines))
    dw_ExtendedStyle |= LVS_EX_GRIDLINES;

  m_ctlItemList.SetExtendedStyle(dw_ExtendedStyle);
  m_ctlItemList.Initialize();
  m_ctlItemList.SetHighlightChanges(prefs->GetPref(PWSprefs::HighlightChanges));

  // Override default HeaderCtrl ID of 0
  m_LVHdrCtrl.SetDlgCtrlID(IDC_LIST_HEADER);

  // Initialise DropTargets - should be in OnCreate()s, really
  m_LVHdrCtrl.Initialize(&m_LVHdrCtrl);
  m_ctlItemTree.Initialize();
  m_ctlItemTree.SetHighlightChanges(prefs->GetPref(PWSprefs::HighlightChanges));

  // Set up fonts before playing with Tree/List views
  LOGFONT LF;

  // Get current font (as specified in .rc file for IDD_PASSWORDSAFE_DIALOG) & save it
  // If it's not available, fall back to font used in pre-3.18 versions, rather than
  // 'System' default.
  CFont *pCurrentFont = GetFont();
  pCurrentFont->GetLogFont(&dfltTreeListFont);

  CString szTreeFont = prefs->GetPref(PWSprefs::TreeFont).c_str();
  Fonts *pFonts = Fonts::GetInstance();

  // If we didn't find font specified in rc, and user didn't select anything
  // fallback to MS Sans Serif
  if (CString(dfltTreeListFont.lfFaceName) == L"System" &&
      szTreeFont.IsEmpty()) {
    const CString MS_SanSerif8 = L"-11,0,0,0,400,0,0,0,177,1,2,1,34,MS Sans Serif";
    szTreeFont = MS_SanSerif8;
    pFonts->ExtractFont(szTreeFont, dfltTreeListFont); // Save for 'Reset font' action
  }
  
  LOGFONT tree_lf;
  if (!szTreeFont.IsEmpty()) { // either preference or our own fallback
    pFonts->ExtractFont(szTreeFont, tree_lf);
    pFonts->SetCurrentFont(&tree_lf);
  } else {
    pFonts->SetCurrentFont(&dfltTreeListFont);
  }

  // Set up Add/Edit font too.
  CString szAddEditFont = prefs->GetPref(PWSprefs::AddEditFont).c_str();

  if (!szAddEditFont.IsEmpty()) {
    pFonts->ExtractFont(szAddEditFont, LF);
    pFonts->SetAddEditFont(&LF);
  } else {
    // Not set - use add/Edit dialog font - difficult to get so use hard
    // coded default
    pFonts->GetDefaultAddEditFont(LF);
    pFonts->SetAddEditFont(&LF);
  }

  // Set up Password font too.
  CString szPasswordFont = prefs->GetPref(PWSprefs::PasswordFont).c_str();

  if (!szPasswordFont.IsEmpty()) {
    pFonts->ExtractFont(szPasswordFont, LF);
    pFonts->SetPasswordFont(&LF);
  } else {
    // Not set - use default password font
    pFonts->SetPasswordFont(NULL);
  }

  // Set up Notes font too.
  CString szNotesFont = prefs->GetPref(PWSprefs::NotesFont).c_str();

  if (!szNotesFont.IsEmpty()) {
    pFonts->ExtractFont(szNotesFont, LF);
    pFonts->SetNotesFont(&LF);
  } else {
    // Not set - use tree/list font set above
    pFonts->GetCurrentFont(&LF);
    pFonts->SetNotesFont(&LF);
  }

  // transfer the fonts to the tree windows
  m_ctlItemTree.SetUpFont();
  m_ctlItemList.SetUpFont();
  m_LVHdrCtrl.SetFont(pFonts->GetCurrentFont());

  const CString lastView = prefs->GetPref(PWSprefs::LastView).c_str();
  if (lastView != L"list")
    OnTreeView();
  else
    OnListView();

  CalcHeaderWidths();

  CString cs_ListColumns = prefs->GetPref(PWSprefs::ListColumns).c_str();
  CString cs_ListColumnsWidths = prefs->GetPref(PWSprefs::ColumnWidths).c_str();

  if (cs_ListColumns.IsEmpty())
    SetColumns();
  else
    SetColumns(cs_ListColumns);

  m_iTypeSortColumn = prefs->GetPref(PWSprefs::SortedColumn);
  switch (m_iTypeSortColumn) {
    case CItemData::UUID:  // Used for sorting on Image!
    case CItemData::GROUP:
    case CItemData::TITLE:
    case CItemData::USER:
    case CItemData::NOTES:
    case CItemData::PASSWORD:
    case CItemData::CTIME:
    case CItemData::PMTIME:
    case CItemData::ATIME:
    case CItemData::XTIME:
    case CItemData::XTIME_INT:
    case CItemData::RMTIME:
    case CItemData::URL:
    case CItemData::EMAIL:
    case CItemData::SYMBOLS:
    case CItemData::RUNCMD:
    case CItemData::AUTOTYPE:
    case CItemData::POLICY:
    case CItemData::PROTECTED:
    case CItemData::KBSHORTCUT:
      break;
    case CItemData::PWHIST:  // Not displayed in ListView
    default:
      // Title is a mandatory column - so can't go wrong!
      m_iTypeSortColumn = CItemData::TITLE;
      break;
  }

  // refresh list will add and size password column if necessary...
  RefreshViews();

  setupBars(); // Just to keep things a little bit cleaner

  ChangeOkUpdate();

  DragAcceptFiles(TRUE);

  CRect rect;
  prefs->GetPrefRect(rect.top, rect.bottom, rect.left, rect.right);
  
  HRGN hrgnWork = GetWorkAreaRegion();
  // also check that window will be visible
  if ((rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) || !RectInRegion(hrgnWork, rect)){
    GetWindowRect(&rect);
    SendMessage(WM_SIZE, SIZE_RESTORED, MAKEWPARAM(rect.Width(), rect.Height()));
  } else {
    PlaceWindow(this, &rect, SW_HIDE);
  }
  ::DeleteObject(hrgnWork);

  // Now do widths!
  if (!cs_ListColumns.IsEmpty())
    SetColumnWidths(cs_ListColumnsWidths);

  // create notes info display window
  m_pNotesDisplay = new CInfoDisplay;
  if (!m_pNotesDisplay->Create(0, 0, L"", this)) {
    // failed
    delete m_pNotesDisplay;
    m_pNotesDisplay = NULL;
  } else {
    // Set up user font
    CFont *pNotes = Fonts::GetInstance()->GetNotesFont();
    m_pNotesDisplay->SetWindowTextFont(pNotes);
  }
  
#if !defined(USE_XML_LIBRARY)
  // Don't support filter processing if we can't validate
#else
  // if there's a filter file named "autoload_filters.xml", 
  // do what its name implies...
  // Since the user may have overridden the configuration directory,
  // use the one that PWSprefs is actually using.
  PWSprefs::ConfigOption configoption;
  std::wstring wsCnfgFile = PWSprefs::GetConfigFile(configoption);
  std::wstring wsAutoLoad;
  if (configoption == PWSprefs::CF_NONE ||
      configoption == PWSprefs::CF_REGISTRY) {
    // Need to use Executable directory instead
    wsAutoLoad = PWSdirs::GetExeDir() + L"autoload_filters.xml";
  } else {
    std::wstring wsCnfgDrive, wsCnfgDir, wsCnfgFileName, wsCnfgExt;
    pws_os::splitpath(wsCnfgFile, wsCnfgDrive, wsCnfgDir, wsCnfgFileName, wsCnfgExt);
    wsAutoLoad = pws_os::makepath(wsCnfgDrive, wsCnfgDir, L"autoload_filters", L".xml");
  }

  if (pws_os::FileExists(wsAutoLoad)) {
    std::wstring strErrors;
    std::wstring XSDFilename = PWSdirs::GetXMLDir() + L"pwsafe_filter.xsd";

    if (!pws_os::FileExists(XSDFilename)) {
      CGeneralMsgBox gmb;
      CString cs_title, cs_msg, cs_temp;
      cs_temp.Format(IDSC_MISSINGXSD, L"pwsafe_filter.xsd");
      cs_msg.Format(IDS_CANTAUTOIMPORTFILTERS, cs_temp);
      cs_title.LoadString(IDSC_CANTVALIDATEXML);
      gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONSTOP);
      return;
    }

    MFCAsker q;
    int rc;
    CWaitCursor waitCursor;  // This may take a while!
    rc = m_MapFilters.ImportFilterXMLFile(FPOOL_AUTOLOAD, L"",
                                          wsAutoLoad,
                                          XSDFilename.c_str(), strErrors, &q);
    waitCursor.Restore();  // Restore normal cursor
    if (rc != PWScore::SUCCESS) {
      CGeneralMsgBox gmb;
      CString cs_msg;
      cs_msg.Format(IDS_CANTAUTOIMPORTFILTERS, strErrors.c_str());
      gmb.AfxMessageBox(cs_msg, MB_OK);
    }
  }
#endif
}

LRESULT DboxMain::OnHotKey(WPARAM wParam, LPARAM )
{
  // The main hotkey is used to invoke the app window, prompting
  // for passphrase if needed.
  if (wParam == PWS_HOTKEY_ID) {
    app.SetHotKeyPressed(true);

    // Because LockDataBase actually doesn't minimize the window,
    // have to also use the current state i.e. Locked
    if (app.GetSystemTrayState() == LOCKED || IsIconic()) {
      SendMessage(WM_COMMAND, ID_MENUITEM_RESTORE);
    }

    SetActiveWindow();
    SetForegroundWindow();
  }
  return 0L;
}

LRESULT DboxMain::OnHeaderDragComplete(WPARAM /* wParam */, LPARAM /* lParam */)
{
  MSG msg;
  while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    // so there is a message process it.
    if (!AfxGetThread()->PumpMessage())
      break;
  }

  // Now update header info
  SetHeaderInfo();

  return 0L;
}

LRESULT DboxMain::OnCCToHdrDragComplete(WPARAM wType, LPARAM afterIndex)
{
  if (wType == CItemData::UUID) {
    m_bImageInLV = true;
    m_ctlItemList.SetImageList(m_pImageList, LVSIL_NORMAL);
    m_ctlItemList.SetImageList(m_pImageList, LVSIL_SMALL);
  }

  AddColumn((int)wType, (int)afterIndex);

  // Reset values
  SetHeaderInfo();

  // Now show the user
  RefreshViews(LISTONLY);

  return 0L;
}

LRESULT DboxMain::OnHdrToCCDragComplete(WPARAM wType, LPARAM /* lParam */)
{
  if (wType == CItemData::UUID) {
    m_bImageInLV = false;
    m_ctlItemList.SetImageList(m_pImageList0, LVSIL_NORMAL);
    m_ctlItemList.SetImageList(m_pImageList0, LVSIL_SMALL);
  }

  DeleteColumn((int)wType);

  // Reset values
  SetHeaderInfo();

  // Now show the user
  RefreshViews(LISTONLY);

  return 0L;
}

BOOL DboxMain::OnInitDialog()
{
  PWS_LOGIT;

  CDialog::OnInitDialog();

  // Set up UPDATE_UI data map.
  const int num_CommandTable_entries = _countof(m_UICommandTable);
  for (int i = 0; i < num_CommandTable_entries; i++)
    m_MapUICommandTable[m_UICommandTable[i].ID] = i;

  // Install managers in window processing chain
  m_menuManager.Install(this);
  m_menuTipManager.Install(this);

  // Subclass the ListView HeaderCtrl
  CHeaderCtrl* pHeader = m_ctlItemList.GetHeaderCtrl();
  if (pHeader && pHeader->GetSafeHwnd()) {
    m_LVHdrCtrl.SubclassWindow(pHeader->GetSafeHwnd());
  }

  CMenuShortcut::InitStrings();
  SetUpInitialMenuStrings();
  SetLocalStrings();
  ConfigureSystemMenu();
  SetMenu(app.GetMainMenu());  // Now show menu...

  InitPasswordSafe();

  if (m_IsStartSilent) {
    m_bStartHiddenAndMinimized = true;
  }

  if (m_IsStartClosed) {
    Close();
    if (!m_IsStartSilent)
      ShowWindow(SW_SHOW);
  }

  BOOL bOOI(TRUE);
  if (!m_IsStartClosed && !m_IsStartSilent) {
    if (m_bSetup) { // --setup flag passed?
      // If default dbase exists, DO NOT overwrite it, else
      // prompt for new combination, create it.
      // Meant for use when running after install
      CString cf(MAKEINTRESOURCE(IDS_DEFDBNAME));
      std::wstring fname = PWSUtil::GetNewFileName(LPCWSTR(cf),
                                                   DEFAULT_SUFFIX);
      std::wstring dir = PWSdirs::GetSafeDir();
      if (dir[dir.length()-1] != L'\\') dir += L"\\";
      fname = dir + fname;
      if (pws_os::FileExists(fname)) 
        bOOI = OpenOnInit();
      else { // really first install!
        CSecString sPasskey;
        CPasskeySetup dbox_pksetup(this, m_core);
        INT_PTR rc = dbox_pksetup.DoModal();

        if (rc == IDOK)
          sPasskey = dbox_pksetup.GetPassKey();
        else {
          PostQuitMessage(0);
          return FALSE;
        }

        m_core.SetCurFile(fname.c_str());
        m_core.NewFile(sPasskey);
        m_core.SetReadOnly(false); 
        rc = m_core.WriteCurFile();
        if (rc == PWScore::CANT_OPEN_FILE) {
          CGeneralMsgBox gmb;
          CString cs_temp, cs_title(MAKEINTRESOURCE(IDS_FILEWRITEERROR));
          cs_temp.Format(IDS_CANTOPENWRITING, m_core.GetCurFile().c_str());
          gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
          PostQuitMessage(0); // can we do something better here?
          return FALSE;
        }
      } // first install
    } else
      bOOI = OpenOnInit();
    // No need for another RefreshViews as OpenOnInit does one via PostOpenProcessing
  }

  // Check if user cancelled
  if (bOOI == FALSE) {
    PostQuitMessage(0);
    return FALSE;
  }

  SetInitialDatabaseDisplay();
  if (m_bOpen && PWSprefs::GetInstance()->GetPref(PWSprefs::ShowFindToolBarOnOpen))
    SetFindToolBar(true);
  else
    OnHideFindToolBar();

  if (m_bOpen) {
    SelectFirstEntry();
  }

  if (m_runner.isValid()) {
    m_runner.Set(m_hWnd); 
  } else if (!app.NoSysEnvWarnings()) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTLOAD_AUTOTYPEDLL, MB_ICONERROR);
  }

  // Set up DragBar Tooltips
  SetDragbarToolTips();

  // Update Minidump user streams
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly());

  return TRUE;  // return TRUE unless you set the focus to a control
}

void DboxMain::SetDragbarToolTips()
{
  // Remove it if already present
  if (m_pToolTipCtrl != NULL) {
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  }

  // create tooltip unconditionally
  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create main DboxMain Dialog ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    EnableToolTips();
    m_pToolTipCtrl->Activate(TRUE);

    /* Set tooltip intervals in milli-seconds.
    In this case, the "tool's bounding rectangle" == each DragBar bitmap
    TTDT_INITIAL: Time the pointer must remain stationary within a tool's 
                  bounding rectangle before the tool tip window appears. 
    TTDT_AUTOPOP: Time the tool tip window remains visible, if the pointer
                  is stationary within a tool's bounding rectangle.
    TTDT_RESHOW:  Time it takes for subsequent tool tip windows to appear
                  as the pointer moves from one tool to another.

    Defaults are based on the 'DoubleClickTime'. For the default 
    double-click time of 500 ms, the initial, autopop, and reshow delay times
    are 500ms, 5000ms, and 100ms respectively.
    */
    m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, 1000);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 8000);
    m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW,  2000);

    // Set maximum width to force Windows to wrap the text.
    m_pToolTipCtrl->SetMaxTipWidth(300);

    // Set 
    CString cs_ToolTip, cs_field;
    cs_field.LoadString(IDS_GROUP);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGGROUP), cs_ToolTip);
    cs_field.LoadString(IDS_TITLE);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGTITLE), cs_ToolTip);
    cs_field.LoadString(IDS_USERNAME);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGUSER), cs_ToolTip);
    cs_field.LoadString(IDS_PASSWORD);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGPASSWORD), cs_ToolTip);
    cs_field.LoadString(IDS_NOTES);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGNOTES), cs_ToolTip);
    cs_field.LoadString(IDS_URL);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGURL), cs_ToolTip);
    cs_field.LoadString(IDS_EMAIL);
    cs_ToolTip.Format(IDS_DRAGTOCOPY, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGEMAIL), cs_ToolTip);
    cs_ToolTip.Format(IDS_DRAGTOAUTOTYPE, cs_field);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_DRAGAUTO), cs_ToolTip);    
  }
}

void DboxMain::SetInitialDatabaseDisplay()
{
  if (m_ctlItemTree.GetCount() > 0) {
    m_ctlItemTree.SetRestoreMode(true);
    switch (PWSprefs::GetInstance()->GetPref(PWSprefs::TreeDisplayStatusAtOpen)) {
      case PWSprefs::AllCollapsed:
        m_ctlItemTree.OnCollapseAll();
        break;
      case PWSprefs::AllExpanded:
        m_ctlItemTree.OnExpandAll();
        break;
      case PWSprefs::AsPerLastSave:
        RestoreGroupDisplayState();
        break;
      default:
        ASSERT(0);
    }
    m_ctlItemTree.SetRestoreMode(false);
    SaveGroupDisplayState();
  }
}

void DboxMain::OnDestroy()
{
  const std::wstring filename(m_core.GetCurFile().c_str());

  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.empty() && !m_core.IsReadOnly() && m_core.IsLockedFile(filename))
    m_core.UnlockFile(filename);

  // Get rid of hotkey
  UnregisterHotKey(GetSafeHwnd(), PWS_HOTKEY_ID);

  // Stop being notified about session changes
  if (m_bWTSRegistered) {
    RegisterSessionNotification(false);
  }

  // Stop subclassing the ListView HeaderCtrl
  if (m_LVHdrCtrl.GetSafeHwnd() != NULL)
    m_LVHdrCtrl.UnsubclassWindow();

  // Stop Drag & Drop OLE
  m_LVHdrCtrl.Terminate();

  // and goodbye
  CDialog::OnDestroy();
}

void DboxMain::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
  if (m_bStartHiddenAndMinimized) {
    lpwndpos->flags |= (SWP_HIDEWINDOW | SWP_NOACTIVATE);
    lpwndpos->flags &= ~SWP_SHOWWINDOW;
    PostMessage(WM_COMMAND, ID_MENUITEM_MINIMIZE);
  }

  CDialog::OnWindowPosChanging(lpwndpos);
}

void DboxMain::Execute(Command *pcmd, PWScore *pcore)
{
  if (pcore == NULL)
    pcore = &m_core;

  SaveGUIStatus();
  pcore->Execute(pcmd);

  // See if we have any special filters active that now do not have any entries
  // to display, which would have meant that the user should not be able to select them,
  // we need to cancel them
  if (m_ctlItemTree.GetCount() == 0 &&
      (CurrentFilter() == m_FilterManager.GetExpireFilter() ||
       CurrentFilter() == m_FilterManager.GetUnsavedFilter())) {
    OnCancelFilter();
  }

  UpdateToolBarDoUndo();

  SaveGUIStatusEx(BOTHVIEWS);
}

void DboxMain::OnUndo()
{
  m_core.Undo();

  // If user has set preference "Save immediately", then undoing a change 
  // should resave - ISSUE did the commands being undone change the DB?
  SetChanged(DATA);
  
  // See if we have any special filters active that now do not have any entries
  // to display, which would have meant that the user should not be able to select them,
  // we need to cancel them
  if (m_ctlItemTree.GetCount() == 0 &&
      (CurrentFilter() == m_FilterManager.GetExpireFilter() ||
       CurrentFilter() == m_FilterManager.GetUnsavedFilter())) {
    OnCancelFilter();
  }

  RestoreGUIStatus();

  UpdateToolBarDoUndo();
  UpdateMenuAndToolBar(m_bOpen);
  UpdateStatusBar();

  SaveGUIStatusEx(BOTHVIEWS);
}

void DboxMain::OnRedo()
{
  SaveGUIStatus();
  m_core.Redo();

  // If user has set preference "Save immediately", then redoing a change 
  // should resave - ISSUE do the commands being redone change the DB?
  SetChanged(DATA);

  UpdateToolBarDoUndo();
  UpdateMenuAndToolBar(m_bOpen);
  UpdateStatusBar();

  SaveGUIStatusEx(BOTHVIEWS);
}

void DboxMain::FixListIndexes()
{
  int N = m_ctlItemList.GetItemCount();
  for (int i = 0; i < N; i++) {
    CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(i);
    ASSERT(pci != NULL);
    if (m_bFilterActive &&
        !m_FilterManager.PassesFiltering(*pci, m_core))
      continue;
    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
    ASSERT(pdi != NULL);
    pdi->list_index = i;
  }
}

void DboxMain::OnItemDoubleClick(NMHDR *, LRESULT *pLResult)
{
  *pLResult = 0L;

  // Ignore double-click if multiple entries selected (List view only)
  if (m_IsListView && m_ctlItemList.GetSelectedCount() != 1)
    return;

  UnFindItem();

  // TreeView only - use DoubleClick to Expand/Collapse group
  // Skip double clicks near items that not selected (for example, clicks on hierarchy lines)
  if (m_ctlItemTree.IsWindowVisible()) {
    POINT pt;
    if(!GetCursorPos(&pt))
      return;

    m_ctlItemTree.ScreenToClient(&pt);
    HTREEITEM hItem = m_ctlItemTree.HitTest(pt);
    
    HTREEITEM hItemSel = m_ctlItemTree.GetSelectedItem();
    
    if (hItem != hItemSel) //Clicked near item, that is different from current
       return;

    // Only if a group is selected
    if ((hItem != NULL && !m_ctlItemTree.IsLeaf(hItem))) {
      // Do standard double-click processing - i.e. toggle expand/collapse!
      return;
    }
  }
  // Now set we have processed the event
  *pLResult = 1L;

  // Continue if in ListView or Leaf in TreeView
  const CItemData *pci = getSelectedItem();
  // Don't do anything if can't get the data
  if (pci == NULL)
    return;

  if (pci->IsShortcut()) {
    pci = GetBaseEntry(pci);
  }

  short iDCA;
  const bool m_bShiftKey = ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
  pci->GetDCA(iDCA, m_bShiftKey);

  if (iDCA < PWSprefs::minDCA || iDCA > PWSprefs::maxDCA)
    iDCA = (short)PWSprefs::GetInstance()->GetPref(m_bShiftKey ? 
              PWSprefs::ShiftDoubleClickAction : PWSprefs::DoubleClickAction);

  m_bViaDCA = true;  // Currently only needed for View/Edit
  switch (iDCA) {
    case PWSprefs::DoubleClickAutoType:
      OnAutoType();
      break;
    case PWSprefs::DoubleClickBrowse:
      OnBrowse();
      break;
    case PWSprefs::DoubleClickBrowsePlus:
      OnBrowsePlus();
      break;
    case PWSprefs::DoubleClickCopyNotes:
      OnCopyNotes();
      break;
    case PWSprefs::DoubleClickCopyPassword:
      OnCopyPassword();
      break;
    case PWSprefs::DoubleClickCopyUsername:
      OnCopyUsername();
      break;
    case PWSprefs::DoubleClickCopyPasswordMinimize:
      OnCopyPasswordMinimize();
      break;
    case PWSprefs::DoubleClickViewEdit:
      OnEdit();
      break;
    case PWSprefs::DoubleClickRun:
      OnRunCommand();
      break;
    case PWSprefs::DoubleClickSendEmail:
      OnSendEmail();
      break;
    default:
      ASSERT(0);
  }
  m_bViaDCA = false;
}

// Called to send an email.
void DboxMain::OnSendEmail()
{
  DoBrowse(false, true);
}

void DboxMain::OnBrowsePlus()
{
  DoBrowse(true, false);
}

// Called to open a web browser to the URL associated with an entry.
void DboxMain::OnBrowse()
{
  DoBrowse(false, false);
}

void DboxMain::DoBrowse(const bool bDoAutotype, const bool bSendEmail)
{
  CItemData *pci = getSelectedItem();
  if (pci != NULL) {
    StringX sx_pswd;
    if (pci->IsDependent()) {
      CItemData *pbci = GetBaseEntry(pci);
      ASSERT(pbci != NULL);
      sx_pswd = pbci->GetPassword();
      if (pci->IsShortcut())
        pci = pbci;
    } else
      sx_pswd = pci->GetPassword();

    CString cs_command;
    if (bSendEmail && !pci->IsEmailEmpty()) {
      cs_command = L"mailto:";
      cs_command += pci->GetEmail().c_str();
    } else {
      cs_command = pci->GetURL().c_str();
    }

    if (!cs_command.IsEmpty()) {
      const pws_os::CUUID uuid = pci->GetUUID();
      std::vector<size_t> vactionverboffsets;
      StringX sxautotype = PWSAuxParse::GetAutoTypeString(*pci, m_core,
                                                          vactionverboffsets);
      LaunchBrowser(cs_command, sxautotype, vactionverboffsets, bDoAutotype);

      if (PWSprefs::GetInstance()->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL)) {
        SetClipboardData(sx_pswd);
        UpdateLastClipboardAction(CItemData::PASSWORD);
      }
      UpdateAccessTime(uuid);
    }
  }
}

void DboxMain::OnUpdateNSCommand(CCmdUI *pCmdUI)
{
  // Use this callback  for commands that need to
  // be disabled if not supported (yet)
  pCmdUI->Enable(FALSE);
}

void DboxMain::SetStartSilent(bool state)
{
  m_IsStartSilent = state;
  if (state) {
    // start silent implies use system tray.
    PWSprefs::GetInstance()->SetPref(PWSprefs::UseSystemTray, true);
    UpdateSystemMenu();
  }
}

void DboxMain::SetChanged(ChangeType changed)
{
  PWS_LOGIT_ARGS("changed=%d", changed);

  if (m_core.IsReadOnly())
    return;

  switch (changed) {
    case DATA:
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately) &&
          m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
        Save();
      } else {
        m_core.SetDBChanged(true);
      }
      break;
    case CLEARDATA:
      m_core.SetDBChanged(false);
      m_core.SetDBPrefsChanged(false);
      m_bTSUpdated = false;
      break;
    case DBPREFS:
      m_core.SetDBPrefsChanged(true);
      break;
    case CLEARDBPREFS:
      m_core.SetDBPrefsChanged(false);
      break;
    case TIMESTAMP:
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::MaintainDateTimeStamps))
        m_bTSUpdated = true;
      break;
    case GROUPDISPLAY:
      m_core.SetGroupDisplayChanged(true);
      break;
    case CLEARGROUPDISPLAY:
      m_core.SetGroupDisplayChanged(false);
      break;
  }
}

void DboxMain::ChangeOkUpdate()
{
  if (!m_bInitDone || 
    (m_core.GetReadFileVersion() != PWSfile::V30 && m_core.GetReadFileVersion() != PWSfile::V40))
    return;

  CMenu *pmenu = GetMenu();

  // Don't need to worry about R-O, as IsChanged can't be true in this case
  pmenu->EnableMenuItem(ID_MENUITEM_SAVE,
    (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) ? MF_ENABLED : MF_GRAYED);
  if (m_toolbarsSetup == TRUE) {
    m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_SAVE,
      (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) ? TRUE : FALSE);
  }
  UpdateStatusBar();
}

void DboxMain::OnAbout()
{
  CAboutDlg about;
  about.DoModal();
}

void DboxMain::OnPasswordSafeWebsite()
{
  HINSTANCE stat = ::ShellExecute(NULL, NULL, L"https://pwsafe.org/",
                              NULL, L".", SW_SHOWNORMAL);
  if (int(stat) <= 32) {
#ifdef _DEBUG
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(L"oops");
#endif
  }
}

int DboxMain::CheckPasskey(const StringX &filename, const StringX &passkey,
                           PWScore *pcore)
{
  // To ensure values in current core are not overwritten when checking the passkey
  if (pcore == NULL)
    return m_core.CheckPasskey(filename, passkey);
  else
    return pcore->CheckPasskey(filename, passkey);
}

int DboxMain::GetAndCheckPassword(const StringX &filename,
                                  StringX &passkey,
                                  int index,
                                  int flags,
                                  PWScore *pcore)
{
  PWS_LOGIT_ARGS("index=%d; flags=0x%04x", index, flags);

  // index:
  //  GCP_FIRST         (0) First
  //  GCP_NORMAL        (1) OK, CANCEL & HELP buttons
  //  GCP_RESTORE       (2) OK, CANCEL & HELP buttons
  //  GCP_WITHEXIT      (3) OK, CANCEL, EXIT & HELP buttons
  //  GCB_CHANGEMODE    (4) OK, CANCEL & HELP buttons

  // for adv_type values, see enum in AdvancedDlg.h

  // Called for an existing database. Prompt user
  // for password, verify against file. Lock file to
  // prevent multiple r/w access.

  int retval;
  bool bFileIsReadOnly = false;

  // Get all read-only values from flags
  bool bReadOnly = (flags & GCP_READONLY) == GCP_READONLY;
  bool bForceReadOnly = (flags & GCP_FORCEREADONLY) == GCP_FORCEREADONLY;
  bool bHideReadOnly = (flags & GCP_HIDEREADONLY) == GCP_HIDEREADONLY;

  if (m_pPasskeyEntryDlg != NULL) { // can happen via systray unlock
    m_pPasskeyEntryDlg->BringWindowToTop();
    return PWScore::USER_CANCEL; // multi-thread,
    // original thread will continue processing
  }

  if (pcore == 0)
    pcore = &m_core;

  if (!filename.empty()) {
    bool exists = pws_os::FileExists(filename.c_str(), bFileIsReadOnly);

    if (!exists) {
      return PWScore::CANT_OPEN_FILE;
    } // !exists
  } // !filename.IsEmpty()


  if (bFileIsReadOnly || bForceReadOnly) {
    // As file is read-only, we must honour it and not permit user to change it
    pcore->SetReadOnly(true);
  }

  // set all field bits
  // (not possible if the user selects some or all available options)
  m_bsFields.set();

  ASSERT(m_pPasskeyEntryDlg == NULL); // should have been taken care of above

  m_pPasskeyEntryDlg = new CPasskeyEntry(this,
                                   filename.c_str(),
                                   index, bReadOnly,
                                   bFileIsReadOnly,
                                   bForceReadOnly,
                                   bHideReadOnly);

  // Ensure blank DboxMain dialog is not shown if user double-clicks
  // on SystemTray icon when being prompted for passphrase
  app.SetSystemTrayTarget(m_pPasskeyEntryDlg);
  
  INT_PTR rc = m_pPasskeyEntryDlg->DoModal();

  if (rc == IDOK) {
    DBGMSG("PasskeyEntry returns IDOK\n");

    const StringX curFile = m_pPasskeyEntryDlg->GetFileName().GetString();
    pcore->SetCurFile(curFile);
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::MaxMRUItems) != 0) {
      extern void RelativizePath(stringT &);
      stringT cf = curFile.c_str(); // relativize and set pref
      RelativizePath(cf);
      PWSprefs::GetInstance()->SetPref(PWSprefs::CurrentFile, cf.c_str());
    }

    std::wstring locker(L""); // null init is important here
    passkey = LPCWSTR(m_pPasskeyEntryDlg->GetPasskey());

    // This dialog's setting of read-only overrides file dialog
    bool bIsReadOnly = m_pPasskeyEntryDlg->IsReadOnly();
    pcore->SetReadOnly(bIsReadOnly);

    // Set read-only mode if user explicitly requested it OR
    // we could not create a lock file.
    switch (index) {
      case GCP_FIRST: // if first, then m_IsReadOnly is set in Open
        pcore->SetReadOnly(bIsReadOnly || !pcore->LockFile(curFile.c_str(), locker));
        break;
      case GCP_NORMAL:
        if (!bIsReadOnly) // !first, lock if !bIsReadOnly
          pcore->SetReadOnly(!pcore->LockFile(curFile.c_str(), locker));
        else
          pcore->SetReadOnly(bIsReadOnly);
        break;
      case GCP_RESTORE:
      case GCP_WITHEXIT:
      case GCP_CHANGEMODE:
      default:
        // user can't change R-O status
        break;
    }

    UpdateToolBarROStatus(bIsReadOnly);

    // locker won't be null IFF tried to lock and failed, in which case
    // it shows the current file locker
    if (!locker.empty()) {
      CString cs_user_and_host, cs_PID;
      cs_user_and_host = (CString)locker.c_str();
      int i_pid = cs_user_and_host.ReverseFind(L':');
      if (i_pid > -1) {
        // If PID present then it is ":%08d" = 9 chars in length
        ASSERT((cs_user_and_host.GetLength() - i_pid) == 9);
        cs_PID.Format(IDS_PROCESSID, cs_user_and_host.Right(8));
        cs_user_and_host = cs_user_and_host.Left(i_pid);
      } else
        cs_PID = L"";

      const CString cs_title(MAKEINTRESOURCE(IDS_FILEINUSE));
      CString cs_msg;
      CGeneralMsgBox gmb;
      gmb.SetTitle(cs_title);
      gmb.SetStandardIcon(MB_ICONQUESTION);
#ifdef PWS_STRICT_LOCKING // define if you don't want to allow user override
      cs_msg.Format(IDS_STRICT_LOCKED, curFile.c_str(),
                    cs_user_and_host, cs_PID);
      gmb.SetMsg(cs_msg);
      gmb.AddButton(IDS_READONLY, IDS_READONLY);
      gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
#else
      cs_msg.Format(IDS_LOCKED, curFile.c_str(), cs_user_and_host, cs_PID);
      gmb.SetMsg(cs_msg);
      gmb.AddButton(IDS_READONLY, IDS_READONLY);
      gmb.AddButton(IDS_READWRITE, IDS_READWRITE);
      gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
#endif
      INT_PTR user_choice = gmb.DoModal();
      switch (user_choice) {
        case IDS_READONLY:
          pcore->SetReadOnly(true);
          UpdateToolBarROStatus(true);
          retval = PWScore::SUCCESS;
          break;
        case IDS_READWRITE:
          pcore->SetReadOnly(false); // Caveat Emptor!
          UpdateToolBarROStatus(false);
          retval = PWScore::SUCCESS;
          break;
        case IDS_EXIT:
          retval = PWScore::USER_CANCEL;
          break;
        default:
          ASSERT(false);
          retval = PWScore::USER_CANCEL;
      }
    } else { // locker.IsEmpty() means no lock needed or lock was successful
      if (m_pPasskeyEntryDlg->GetStatus() == TAR_NEW) {
        // Save new file
        pcore->NewFile(m_pPasskeyEntryDlg->GetPasskey());
        rc = pcore->WriteCurFile();

        if (rc == PWScore::CANT_OPEN_FILE) {
          CGeneralMsgBox gmbx;
          CString cs_temp, cs_title(MAKEINTRESOURCE(IDS_FILEWRITEERROR));
          cs_temp.Format(IDS_CANTOPENWRITING, pcore->GetCurFile().c_str());
          gmbx.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
          retval = PWScore::USER_CANCEL;
        } else {
          // By definition - new files can't be read-only!
          pcore->SetReadOnly(false); 
          retval = PWScore::SUCCESS;
        }
      } else // no need to create file
        retval = PWScore::SUCCESS;
    }
  } else {/*if (rc==IDCANCEL) */ //Determine reason for cancel
    int cancelreturn = m_pPasskeyEntryDlg->GetStatus();
    switch (cancelreturn) {
      case TAR_OPEN:
      case TAR_CANCEL:
      case TAR_NEW:
        retval = PWScore::USER_CANCEL;
        break;
      case TAR_EXIT:
        retval = PWScore::USER_EXIT;
        break;
      case TAR_OPEN_NODB:
        retval = PWScore::OPEN_NODB;
        break;
      default:
        DBGMSG("Default to WRONG_PASSWORD\n");
        retval = PWScore::WRONG_PASSWORD;  //Just a normal cancel
        break;
    }
  }

  // Put us back
  app.SetSystemTrayTarget(this);

  delete m_pPasskeyEntryDlg;
  m_pPasskeyEntryDlg = NULL;
  return retval;
}

void DboxMain::CancelPendingPasswordDialog()
{
  /**
   * Called from LockDataBase(), closes any pending
   * password dialog box when locking.
   * The ensures a sane state upon restore.
   */
  if (m_pPasskeyEntryDlg == NULL)
    return; // all is well, nothing to do
  else
    m_pPasskeyEntryDlg->SendMessage(WM_CLOSE);
}

BOOL DboxMain::OnToolTipText(UINT, NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // This code is copied from the DLGCBR32 example that comes with MFC
  // Updated by MS on 25/09/2005
  ASSERT(pNotifyStruct->code == TTN_NEEDTEXTW);

  // allow top level routing frame to handle the message
  if (GetRoutingFrame() != NULL)
    return FALSE;

  // need to handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *)pNotifyStruct;
  wchar_t tc_FullText[4096];  // Maxsize of a string in a resource file
  CString cs_TipText;
  UINT nID = (UINT)pNotifyStruct->idFrom;
  if (pTTTW->uFlags & TTF_IDISHWND) {
    // idFrom is actually the HWND of the tool
    nID = ((UINT)(WORD)::GetDlgCtrlID((HWND)nID));
  }

  if (nID != 0) { // will be zero on a separator
    if (AfxLoadString(nID, tc_FullText, 4095) == 0)
      return FALSE;

    // this is the command id, not the button index
    if (AfxExtractSubString(cs_TipText, tc_FullText, 1, '\n') == FALSE)
      return FALSE;
  } else
    return FALSE;

  if (cs_TipText.IsEmpty())
    return TRUE;  // message handled

  // Assume ToolTip is greater than 80 characters in ALL cases and so use
  // the pointer approach.

  delete m_pwchTip;

  m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
  wcsncpy_s(m_pwchTip, cs_TipText.GetLength() + 1,
              cs_TipText, _TRUNCATE);

  pTTTW->lpszText = (LPWSTR)m_pwchTip;

  *pLResult = 0;

  return TRUE;    // message was handled
}

void DboxMain::UpdateAlwaysOnTop()
{
  CMenu* sysMenu = GetSystemMenu(FALSE);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    sysMenu->CheckMenuItem(ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_CHECKED);
  } else {
    SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    sysMenu->CheckMenuItem(ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_UNCHECKED);
  }
}

void DboxMain::OnSysCommand(UINT nID, LPARAM lParam)
{
  if (ID_SYSMENU_ALWAYSONTOP == nID) {
    PWSprefs *prefs = PWSprefs::GetInstance();
    bool oldAlwaysOnTop = prefs->GetPref(PWSprefs::AlwaysOnTop);
    prefs->SetPref(PWSprefs::AlwaysOnTop, !oldAlwaysOnTop);
    UpdateAlwaysOnTop();
    return;
  }

  UINT const nSysID = nID & 0xFFF0;

  PWS_LOGIT_ARGS("nID=%04X", nID);

  switch (nSysID) {
    case SC_MINIMIZE:
      break;
    case SC_CLOSE:
      if (!PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray)) {
        Close();
      } else {
        // Save expand/collapse status of groups
        m_vGroupDisplayState = GetGroupDisplayState();
      }
      break;
    case SC_MAXIMIZE:
    case SC_RESTORE:
      if (app.GetSystemTrayState() == ThisMfcApp::LOCKED &&
          !RestoreWindowsData(nSysID == SC_RESTORE))
        return; // password bad or cancel pressed
      break;
    /*
       Valid values not specifically used by PasswordSafe
    case SC_HOTKEY:
    case SC_HSCROLL:
    case SC_KEYMENU:
    case SC_MONITORPOWER:
    case SC_MOUSEMENU:
    case SC_MOVE:
    case SC_NEXTWINDOW:
    case SC_PREVWINDOW:
    case SC_SIZE:
    case SC_SCREENSAVE:
    case SC_TASKLIST:
    case SC_VSCROLL:
    */
    default:
      break;
  }

  // Let standard processing handle CLOSE, MINIMIZE, RESTORE
  // (and MAXIMIZE) and let it then call our OnSize routine
  // to do the deed
  CDialog::OnSysCommand(nID, lParam);
}

void DboxMain::ConfigureSystemMenu()
{
  CMenu *pSysMenu = GetSystemMenu(FALSE);
  const CString str(MAKEINTRESOURCE(IDS_ALWAYSONTOP));

  if (pSysMenu != NULL) {
    UINT num = pSysMenu->GetMenuItemCount();
    ASSERT(num > 2);
    pSysMenu->InsertMenu(num - 2 /* 5 */, MF_BYPOSITION | MF_STRING, ID_SYSMENU_ALWAYSONTOP, (LPCWSTR)str);
  }
}

void DboxMain::OnUpdateMRU(CCmdUI* pCmdUI)
{
  if (app.GetMRU() == NULL)
    return;

  if (!app.IsMRUOnFileMenu()) {
    if (pCmdUI->m_nIndex == 0) { // Add to popup menu
      app.GetMRU()->UpdateMenu(pCmdUI);
    } else {
      return;
    }
  } else {
    app.GetMRU()->UpdateMenu(pCmdUI);
  }
}

LRESULT DboxMain::OnTrayNotification(WPARAM , LPARAM)
{
#if 0
  return m_pTrayIcon.OnTrayNotification(wParam, lParam);
#else
  return 0L;
#endif
}

bool DboxMain::RestoreWindowsData(bool bUpdateWindows, bool bShow)
{
  PWS_LOGIT_ARGS("bUpdateWindows=%s, bShow=%s",
    bUpdateWindows ? L"true" : L"false", 
    bShow ? L"true" : L"false");

  // This restores the data in the main dialog.
  // If currently locked, it checks the user knows the correct passphrase first
  // Note: bUpdateWindows = true only when called from within OnSysCommand-SC_RESTORE
  // and via the Restore menu item via the SystemTray (OnRestore)

  /*
  pws_os::Trace(L"RestoreWindowsData:bUpdateWindows = %s; bInRestoreWindowsData %s\n",
                bUpdateWindows ? L"true" : L"false",
                m_bInRestoreWindowsData ? L"true" : L"false");
  */

  // We should not be called by a routine we call - only duplicates refreshes etc.
  if (m_bInRestoreWindowsData)
    return false;

  m_bInRestoreWindowsData = true;
  bool brc(false);

  // First - no database is currently open
  if (!m_bOpen) {
    // First they may be nothing to do!
    if (bUpdateWindows) {
      if (m_IsStartSilent) {
        // Show initial dialog ONCE (if succeeds)
        if (!m_IsStartClosed) {
          if (OpenOnInit()) {
            m_IsStartSilent = false;
            RefreshViews();
            ShowWindow(SW_RESTORE);
            SetInitialDatabaseDisplay();
            UpdateSystemTray(UNLOCKED);
          }
        } else { // m_IsStartClosed (&& m_IsStartSilent)
          m_IsStartClosed = m_IsStartSilent = false;
          ShowWindow(SW_RESTORE);
        }
        goto exit;  // return false
      } // m_IsStartSilent
      ShowWindow(SW_RESTORE);
    } // bUpdateWindows == true
    UpdateSystemTray(CLOSED);
    goto exit;  // return false
  }

  // Case 1 - data available but is currently locked
  // By definition - data available implies m_bInitDone
  if (!m_bDBNeedsReading &&
      (app.GetSystemTrayState() == ThisMfcApp::LOCKED) &&
      (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray))) {

    StringX passkey;
    int rc_passphrase;
    // Verify passphrase (dialog shows only OK, CANCEL & HELP)
    rc_passphrase = GetAndCheckPassword(m_core.GetCurFile(), passkey, GCP_RESTORE);
    if (rc_passphrase != PWScore::SUCCESS) {
      goto exit;  // return false - don't even think of restoring window!
    }

    app.SetSystemTrayState(ThisMfcApp::UNLOCKED);
    if (bUpdateWindows) {
      RefreshViews();
      ShowWindow(SW_RESTORE);
    }
    brc = true;
    goto exit;
  }

  // Case 2 - data unavailable
  if (m_bInitDone && m_bDBNeedsReading) {
    StringX passkey;
    int rc_passphrase(PWScore::USER_CANCEL), rc_readdatabase;
    const bool bUseSysTray = PWSprefs::GetInstance()->
                             GetPref(PWSprefs::UseSystemTray);

    // Hide the Window while asking for the passphrase
    if (IsWindowVisible()) {
      ShowWindow(SW_HIDE);
    }

    if (m_bOpen) {
      int flags = 0;
      if (m_core.IsReadOnly())
        flags |= GCP_READONLY;
      if (CPWDialog::GetDialogTracker()->AnyOpenDialogs() ||
          m_core.IsChanged())
        flags |= GCP_HIDEREADONLY;

      rc_passphrase = GetAndCheckPassword(m_core.GetCurFile(), passkey,
                               bUseSysTray ? GCP_RESTORE : GCP_WITHEXIT,
                               flags);
    }
    CGeneralMsgBox gmb;
    CString cs_temp, cs_title;

    switch (rc_passphrase) {
      case PWScore::SUCCESS:
        // Don't validate again
        rc_readdatabase = m_core.ReadCurFile(passkey);
        m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                           m_core.GetCurFile()).c_str();
        break;
      case PWScore::CANT_OPEN_FILE:
        cs_temp.Format(IDS_CANTOPEN, m_core.GetCurFile().c_str());
        cs_title.LoadString(IDS_FILEOPEN);
        gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
        // Drop thorugh to ask for a new database
      case TAR_NEW:
        rc_readdatabase = New();
        break;
      case TAR_OPEN:
        rc_readdatabase = Open();
        break;
      case PWScore::WRONG_PASSWORD:
        rc_readdatabase = PWScore::NOT_SUCCESS;
        break;
      case PWScore::USER_CANCEL:
        rc_readdatabase = PWScore::NOT_SUCCESS;
        break;
      case PWScore::USER_EXIT:
        m_core.UnlockFile(m_core.GetCurFile().c_str());
        PostQuitMessage(0);
        return false;
      default:
        rc_readdatabase = PWScore::NOT_SUCCESS;
        break;
    }

    if (rc_readdatabase == PWScore::SUCCESS) {
      UpdateSystemTray(UNLOCKED);
      startLockCheckTimer();
      brc = true;
      m_bDBNeedsReading = false;

      if (bShow && !bUpdateWindows)
        ShowWindow(SW_SHOW);
      if (bUpdateWindows)
        RestoreWindows();
    } else {
      ShowWindow(bUseSysTray ? SW_HIDE : SW_MINIMIZE);
    }
    goto exit;
  }

  // Case 3 - data available and not locked
  if (bUpdateWindows)
    RestoreWindows();
  brc = true;

exit:
  m_bInRestoreWindowsData = false;
  return brc;
}

void DboxMain::startLockCheckTimer()
{
  // If we successfully registered for WTS events,
  // then we don't need this timer. Otherwise, we start it
  // if user wishes to lock us on Windows lock.

  const UINT INTERVAL = 5000; // every 5 seconds should suffice

  if (!m_bWTSRegistered && PWSprefs::GetInstance()->
      GetPref(PWSprefs::LockOnWindowLock) == TRUE) {
    SetTimer(TIMER_LOCKONWTSLOCK, INTERVAL, NULL);
  }
}

void DboxMain::OnHelp()
{
  if (!app.GetHelpFileName().IsEmpty()) {
    CString cs_HelpTopic;
    cs_HelpTopic = app.GetHelpFileName() + L"::/html/Welcome.html";
    HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
  } else {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_HELP_UNAVALIABLE, MB_ICONERROR);
  }
}

BOOL DboxMain::PreTranslateMessage(MSG* pMsg)
{
  // Don't do anything if in AutoType
  if (m_bInAT)
    return TRUE;

  // Do Dragbar tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN) {
    switch (pMsg->wParam) {
      case VK_F1:
      {
        OnHelp();
        return TRUE;
      }

      case VK_ESCAPE:
      {
        // If Find Toolbar visible, close it and do not pass the ESC along.
        if (m_FindToolBar.IsVisible()) {
          OnHideFindToolBar();
          return TRUE;
        }
        // Do NOT pass the ESC along if preference EscExits is false.
        if (!PWSprefs::GetInstance()->GetPref(PWSprefs::EscExits)) {
          return TRUE;
        }
      }
      default:
        break;
    }

    // This should be an entry keyboard shortcut!
    CWnd *pWnd = FromHandle(pMsg->hwnd);
    if (pWnd == NULL)
      goto exit;

    UINT nID = pWnd->GetDlgCtrlID();
    // But only if we have Focus (Tree or List View) of FindToolBar edit box
    if (nID != IDC_ITEMTREE && nID != IDC_ITEMLIST && nID != ID_TOOLBUTTON_FINDEDITCTRL)
      goto exit;

    // Need base character excluding special keys
    short siKeyStateVirtualKeyCode = VkKeyScan(pMsg->wParam & 0xff);
    WORD wVirtualKeyCode = siKeyStateVirtualKeyCode & 0xff;

    if (wVirtualKeyCode != 0) {
      WORD wModifiers(0);
      if (GetKeyState(VK_CONTROL) & 0x8000)
        wModifiers |= MOD_CONTROL;

      if (GetKeyState(VK_MENU) & 0x8000)
        wModifiers |= MOD_ALT;

      if (GetKeyState(VK_SHIFT) & 0x8000)
        wModifiers |= MOD_SHIFT;

      if (!ProcessEntryShortcut(wVirtualKeyCode, wModifiers))
        return TRUE;
    }
  }

exit:
  return CDialog::PreTranslateMessage(pMsg);
}

BOOL DboxMain::ProcessEntryShortcut(WORD &wVirtualKeyCode, WORD &wModifiers)
{
  static const wchar_t *tcValidKeys = 
          L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // Convert ASCII letters to Upper case
  if (wVirtualKeyCode >= 'a' && wVirtualKeyCode <= 'z')
    wVirtualKeyCode -= 0x20;

  if (wVirtualKeyCode == 0 || wcschr(tcValidKeys, wVirtualKeyCode) == NULL)
    return 1L;

  // Get PWS modifiers
  WORD wPWSModifiers = ConvertModifersWindows2PWS(wModifiers);

  // If non-zero - see if it is an entry keyboard shortcut
  if (wPWSModifiers != 0) {
    int32 iKBShortcut = (wPWSModifiers << 16) + wVirtualKeyCode;
    pws_os::CUUID uuid = m_core.GetKBShortcut(iKBShortcut);

    if (uuid != pws_os::CUUID::NullUUID()) {
      // Yes - find the entry  and deselect any already selected
      ItemListIter iter = m_core.Find(uuid);
      DisplayInfo *pdi = (DisplayInfo *)iter->second.GetDisplayInfo();
      ASSERT(pdi != NULL);
      if (m_ctlItemList.IsWindowVisible()) {
        // Unselect all others first
        POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
        while (pos) {
          int iIndex = m_ctlItemList.GetNextSelectedItem(pos);
          m_ctlItemList.SetItemState(iIndex, 0, LVIS_FOCUSED | LVIS_SELECTED);
          m_ctlItemList.Update(iIndex);
        }
        
        // Get CListCtrl to do our work for us - LVN_ITEMCHANGED
        m_ctlItemList.SetItemState(pdi->list_index,
                                            LVIS_FOCUSED | LVIS_SELECTED,
                                            LVIS_FOCUSED | LVIS_SELECTED);
        m_ctlItemList.EnsureVisible(pdi->list_index, FALSE);
      } else {
        // Get CTreeCtrl to do our work for us - TVN_SELCHANGED
        m_ctlItemTree.Select(pdi->tree_item, TVGN_CARET);
        m_ctlItemTree.EnsureVisible(pdi->tree_item);
      }

      // In case FindToolBar has the focus
      if (m_ctlItemList.IsWindowVisible())
        m_ctlItemList.SetFocus();
      else
        m_ctlItemTree.SetFocus();

      // We have processed it
      return 0L;
    }
  }

  // We haven't processed it
  return 1L;
}

void DboxMain::ResetIdleLockCounter(UINT event)
{
  // List of all the events that signify actual user activity, as opposed
  // to Windows internal events...
  // This is usually called from a windows event handler hook.
  // When called from elsewhere, use default argument.

  if ((event >= WM_KEYFIRST   && event <= WM_KEYLAST)   || // all kbd events
      (event >= WM_MOUSEFIRST && event <= WM_MOUSELAST) || // all mouse events
      event == WM_COMMAND       ||
      event == WM_ENABLE        ||
      event == WM_SYSCOMMAND    ||
      event == WM_VSCROLL       ||
      event == WM_HSCROLL       ||
      event == WM_MOVE          ||
      event == WM_SIZE          ||
      event == WM_CONTEXTMENU   ||
      event == WM_SETFOCUS      ||
      event == WM_MENUSELECT)
    SetIdleLockCounter(PWSprefs::GetInstance()->GetPref(PWSprefs::IdleTimeout));
}

void DboxMain::SetIdleLockCounter(UINT iMinutes)
{
  // We set the value to "iMinutes * number of checks per minute"
  m_IdleLockCountDown = iMinutes * IDLE_CHECK_RATE;
}

bool DboxMain::DecrementAndTestIdleLockCounter()
{
  bool retval;
  if (m_IdleLockCountDown > 0)
    retval= (--m_IdleLockCountDown == 0);
  else
    retval = false; // so we return true only once if idle
  return retval;
}

LRESULT DboxMain::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  static DWORD last_t = 0;
  DWORD t = GetTickCount();
  if (t != last_t) {
    PWSrand::GetInstance()->AddEntropy((unsigned char *)&t, sizeof(t));
    last_t = t;
  }
  ResetIdleLockCounter(message);

  if (message == WM_SYSCOLORCHANGE ||
      message == WM_SETTINGCHANGE)
    RefreshImages();

  const WORD wNCode = HIWORD(wParam);
  const WORD wID = LOWORD(wParam);
  /*
    wNCode = Notification Code if from a control, 1 if from an accelerator
             and 0 if from a menu.
    wID    = Identifier of control, accelerator or menu item.
    lParam = If from a control, then its handle otherwise NULL.
  */
  if (message == WM_COMMAND && lParam == NULL &&
      wNCode == 0 && wID >= ID_LANGUAGES ) {
    // Process Language Selection
    const size_t iLang = wID - ID_LANGUAGES;
    if (iLang < app.m_vlanguagefiles.size()) {
      // Default is normal locale processing
      StringX sxLL(L""), sxCC(L"");
      LCID lcid(0);
      if ((app.m_vlanguagefiles[iLang].xFlags & 0x80) != 0x80) {
        // Wasn't checked - select it
        sxLL = app.m_vlanguagefiles[iLang].wsLL.c_str();
        sxCC = app.m_vlanguagefiles[iLang].wsCC.c_str();
        if (!sxCC.empty()) {
          sxLL += L"_";
          sxLL += sxCC;
        }
        lcid = app.m_vlanguagefiles[iLang].lcid = app.m_vlanguagefiles[iLang].lcid;
      }
      PWSprefs::GetInstance()->SetPref(PWSprefs::LanguageFile, sxLL);
      SetLanguage(lcid);
    }
  }

  if (message == WM_SYSCOMMAND && wParam == SC_KEYMENU) {
    // This should be an entry keyboard shortcut!
    CWnd *pWnd = GetFocus();
    if (pWnd == NULL)
      return 0L;

    UINT nID = pWnd->GetDlgCtrlID();
    // But only if we have Focus (Tree or List View) of FindToolBar edit box
    if (nID != IDC_ITEMTREE && nID != IDC_ITEMLIST && nID != ID_TOOLBUTTON_FINDEDITCTRL)
      goto exit;

    // Need base character excluding special keys
    short siKeyStateVirtualKeyCode = VkKeyScan(lParam & 0xff);
    WORD wVirtualKeyCode = siKeyStateVirtualKeyCode & 0xff;

    if (wVirtualKeyCode != 0) {
      WORD wModifiers(0);
      if (GetKeyState(VK_CONTROL) & 0x8000)
        wModifiers |= MOD_CONTROL;

      if (GetKeyState(VK_MENU) & 0x8000)
        wModifiers |= MOD_ALT;

      if (GetKeyState(VK_SHIFT) & 0x8000)
        wModifiers |= MOD_SHIFT;

      if (!ProcessEntryShortcut(wVirtualKeyCode, wModifiers))
        return 0;
    }
  }

exit:
  return CDialog::WindowProc(message, wParam, lParam);
}

void DboxMain::SetLanguage(LCID lcid)
{
  UNREFERENCED_PARAMETER(lcid);
  // Show wait cursor
  CWaitCursor wait;

  SetMenu(NULL);

  // Set up the new language and main menu
  app.SetLanguage();

  // Reset new language equivalent to Ctrl, Alt & shift
  CMenuShortcut::InitStrings();

  // Set up menu shortcuts
  SetUpInitialMenuStrings();

  // Set up local strings
  SetLocalStrings();

  // Now show menu...
  SetMenu(app.GetMainMenu());

  // Make sure shortcuts updated
  for (int i = 0; i < NUMPOPUPMENUS; i++) {
    m_bDoShortcuts[i] = true;
  }

  // Update Statusbar
  // Double-click action meaningless if multiple entries selected - pci should be NULL
  // List view only
  CItemData *pci(NULL);
  if (m_IsListView && m_ctlItemList.GetSelectedCount() == 1)
    pci = getSelectedItem();

  SetDCAText(pci);
  if (m_ilastaction != 0)
    UpdateLastClipboardAction(m_ilastaction);
  else
    UpdateStatusBar();

  // Set up DragBar Tooltips
  SetDragbarToolTips();

  // Update Minidump user streams - language is a string preference
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly(), us3);

  // Remove wait cursor
  wait.Restore();
}

void DboxMain::RefreshImages()
{
  m_MainToolBar.RefreshImages();
  m_FindToolBar.RefreshImages();
  m_menuManager.SetImageList(&m_MainToolBar);
}

void DboxMain::CheckExpireList(const bool bAtOpen)
{
  // Called from PostOpenProcessing, OnOptions (if user turns on warnings
  // or changes warning interval) and from OnTimer when the timer pops.
  // If we have entries with expiration dates, check them, and start a daily timer
  // to repeat the check

  // Cancel timer and restart it - even if there are no expired events this time
  // or the user has not set a warning period - they may do so in the next 24 hrs
  KillTimer(TIMER_EXPENT);
  const UINT DAY = 86400000; // 24 hours, in millisecs (24*60*60*1000)
  SetTimer(TIMER_EXPENT, DAY, NULL);

  // Check if we've any expired entries. If so, show the user.
  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn) ||
       m_core.GetExpirySize() == 0) {
    m_bTellUserExpired = false;
    return;
  }

  int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
  ExpiredList expiredEntries = m_core.GetExpired(idays);

  if (!expiredEntries.empty() && (app.GetSystemTrayState() == LOCKED || IsIconic() == TRUE || bAtOpen))
    m_bTellUserExpired = true;
}

void DboxMain::TellUserAboutExpiredPasswords()
{
  // Check once more
  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn) ||
       m_core.GetExpirySize() == 0) {
    m_bTellUserExpired = false;
    return;
  }

  // Tell user that there are expired passwords
  int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
  ExpiredList expiredEntries = m_core.GetExpired(idays);

  if (m_bTellUserExpired && !expiredEntries.empty()) {
    m_bTellUserExpired = !m_bTellUserExpired;

    // Give user option to display and edit them
    CGeneralMsgBox gmb;
    CString cs_text, cs_title(MAKEINTRESOURCE(IDS_EXPIREDPSWDSTITLE));
    cs_text.LoadString(IDS_EXPIREDPSWDSEXIST);
    INT_PTR rc = gmb.MessageBox(cs_text, cs_title, MB_ICONEXCLAMATION | MB_YESNO);

    if (rc == IDYES) {
      CExpPWListDlg dlg(this, expiredEntries, m_core.GetCurFile().c_str());
      dlg.DoModal();
    }
  }
}

void DboxMain::UpdateAccessTime(const pws_os::CUUID &uuid)
{
  // Mark access time if so configured
  // First add to RUE List
  m_RUEList.AddRUEntry(uuid);

  bool bMaintainDateTimeStamps = PWSprefs::GetInstance()->
              GetPref(PWSprefs::MaintainDateTimeStamps);

  if (!m_core.IsReadOnly() && bMaintainDateTimeStamps) {
    ItemListIter iter = Find(uuid);
    ASSERT(iter != End());
    // fail safely in Release build:
    if (iter == End())
      return;
    CItemData &item = iter->second;
    item.SetATime();
    SetChanged(TIMESTAMP);

    if (!IsGUIEmpty() &&
        (m_nColumnIndexByType[CItemData::ATIME] != -1)) {
      // Need to update view if there and the display has been
      // rebuilt/restored after unlocking or minimized
      // Get index of entry
      DisplayInfo *pdi = (DisplayInfo *)item.GetDisplayInfo();
      ASSERT(pdi != NULL);
      // Get value in correct format
      const CString cs_atime(item.GetATimeL().c_str());
      // Update it
      m_ctlItemList.SetItemText(pdi->list_index,
                                m_nColumnIndexByType[CItemData::ATIME],
                                cs_atime);
    }
  }
}

LRESULT DboxMain::OnQueryEndSession(WPARAM , LPARAM lParam)
{
    PWS_LOGIT_ARGS("lParam=%d", lParam);

  /*
    ********************************************************************************
    NOTE: If the Windows API ExitWindowsEx is called with the EWX_FORCE flag,
    then this routine will NOT be called.  Data may be lost!!!

    If the EWX_FORCEIFHUNG flag is used, Windows forces processes to terminate
    if they do not respond to the WM_QUERYENDSESSION or WM_ENDSESSION message
    within the timeout interval. Again, data may be lost!!!
    ********************************************************************************

    Timeouts are in the Registry at:
      HKCU\Control Panel\Desktop\HungAppTimeout       - default  5000 msecs (5s)
      HKCU\Control Panel\Desktop\WaitToKillAppTimeout - default 20000 msecs (20s)

    If the registry key "HKCU\Control Panel\Desktop\AutoEndTask" is changed to 1
    (enabled), Windows will automatically end tasks even if we are in the middle
    of saving the user's data.

    lParam values:
    ENDSESSION_CLOSEAPP
      The application is using a file that must be replaced, the system is being
      serviced, or system resources are exhausted.
    ENDSESSION_CRITICAL
      The application is forced to shut down.
    ENDSESSION_LOGOFF
      The user is logging off.

    Extract from MSDN:
      Applications can display a user interface prompting the user for information
      at shutdown, HOWEVER IT IS NOT RECOMMENDED. After five seconds, the system
      displays information about the applications that are preventing shutdown and
      allows the user to terminate them.

      For example, Windows XP displays a dialog box, while Windows Vista displays a
      full screen with additional information about the applications blocking shutdown.
      If your application must block or postpone system shutdown, use the 
      ShutdownBlockReasonCreate function (Vista & later).
  */

  // Set to a return code value that is not available to the user if asked
  m_iSessionEndingStatus = IDOK;

  // Save Application related preferences anyway - does no harm!
  SavePreferencesOnExit();

  // No point if database is read-only. Get out fast!
  if (m_core.IsReadOnly())
    return TRUE;

  // Don't stand in the way of a "Get Off" command! Get out fast!
  if ((lParam & (ENDSESSION_CRITICAL | ENDSESSION_CLOSEAPP)) != 0)
    return TRUE;

  // Musn't block if Vista or later and there is no reason to block it.
  // Get out fast!
  if (pws_os::IsWindowsVistaOrGreater()) {
    if (m_bBlockShutdown) {
      m_iSessionEndingStatus = IDCANCEL;
    } else {
      return TRUE;
    }
  }

  if (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) {
    // Windows XP or earlier - we ask user, Vista and later - we don't as we have
    // already set ShutdownBlockReasonCreate
    if (!pws_os::IsWindowsVistaOrGreater()) {
      CGeneralMsgBox gmb;
      CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_ENDSESSION));
      cs_msg.Format(IDS_SAVECHANGES, m_core.GetCurFile().c_str());

      // Now issue dialog with 3 second timeout!
      m_iSessionEndingStatus = gmb.MessageBoxTimeOut(cs_msg, cs_title,
                    (MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON3), 3000);
    } // Windows XP or earlier
  } // database was changed

  //  Say OK to shutdown\restart\logoff (unless Windows XP or earlier said CANCEL)
  return m_iSessionEndingStatus == IDCANCEL ? FALSE : TRUE;
}

LRESULT DboxMain::OnEndSession(WPARAM wParam, LPARAM )
{
  PWS_LOGIT_ARGS("wParam=%d", wParam);

  /*
    See comments in OnQueryEndSession above.
  */
  if (wParam == TRUE) {
    if (m_pfcnShutdownBlockReasonDestroy != NULL)
      m_pfcnShutdownBlockReasonDestroy(m_hWnd);

    switch (m_iSessionEndingStatus) {
      case IDIGNORE:
      case IDCANCEL:
        // How did we get here - IDIGNORE means we are not ending and IDCANCEL
        // means the user said to cancel the shutdown\restart\logoff!!!
        break;
      case IDOK:
        // User never asked a question (Vista or later)
        // Let them decide via the full screen display presented by the OS
        // However, be nice - take a special emergency save
        SaveDatabaseOnExit(ST_FAILSAFESAVE);
        break;
      case IDYES:
        // User said Yes - save the changes (Windows XP or earlier)
        SaveDatabaseOnExit(ST_ENDSESSIONEXIT);
        break;
      case IDNO:
        // User said No - don't save the changes (Windows XP or earlier)
        break;
      case IDTIMEOUT:
        // Windows XP or earlier, the question to the user Timed out.
        // It is now up to the user to use the OS dialog to either 'End Now'
        // PasswordSafe and take the consequences or cancel the EndSession.
        // However, be nice - take a special emergency save
        SaveDatabaseOnExit(ST_FAILSAFESAVE);
        break;
    }

    m_bBlockShutdown = false;
    CleanUpAndExit(false);
  } else {
    // Reset status since the EndSession was cancelled
    m_iSessionEndingStatus = IDIGNORE;
  }

  CDialog::OnEndSession((BOOL)wParam);
  return 0L;
}

void DboxMain::UpdateStatusBar()
{
  if (m_toolbarsSetup == TRUE) {
    CString s;

    // Set the width according to the text
    UINT uiID, uiStyle;
    int iWidth;
    CRect rectPane;
    // calculate text width
    CClientDC dc(&m_statusBar);
    CFont *pFont = m_statusBar.GetFont();
    ASSERT(pFont);
    dc.SelectObject(pFont);
    const int iBMWidth = m_statusBar.GetBitmapWidth();

    if (m_bOpen) {
      dc.DrawText(m_lastclipboardaction, &rectPane, DT_CALCRECT);
      m_statusBar.GetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_CLIPBOARDACTION, m_lastclipboardaction);

      s = m_core.IsChanged() ? L"*" : L" ";
      s += m_core.HaveDBPrefsChanged() ? L"�" : L" ";
      dc.DrawText(s, &rectPane, DT_CALCRECT);
      m_statusBar.GetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_MODIFIED, s);

      s.LoadString(m_core.IsReadOnly() ? IDS_READ_ONLY : IDS_READ_WRITE);
      dc.DrawText(s, &rectPane, DT_CALCRECT);
      m_statusBar.GetPaneInfo(CPWStatusBar::SB_READONLY, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_READONLY, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_READONLY, s);

      if (m_bFilterActive)
        s.Format(IDS_NUMITEMSFILTER, m_bNumPassedFiltering,
                 m_core.GetNumEntries());
      else
        s.Format(IDS_NUMITEMS, m_core.GetNumEntries());

      dc.DrawText(s, &rectPane, DT_CALCRECT);
      m_statusBar.GetPaneInfo(CPWStatusBar::SB_NUM_ENT, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_NUM_ENT, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_NUM_ENT, s);

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_FILTER, uiID, uiStyle, iWidth);
      uiID = m_bFilterActive ? IDB_FILTER_ACTIVE : IDS_BLANK;
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_FILTER, uiID, uiStyle | SBT_OWNERDRAW, iBMWidth);
    } else {
      s.LoadString(IDSC_STATCOMPANY);
      m_statusBar.SetPaneText(CPWStatusBar::SB_DBLCLICK, s);

      dc.DrawText(L" ", &rectPane, DT_CALCRECT);

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_CLIPBOARDACTION, L" ");

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_MODIFIED, L" ");

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_READONLY, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_READONLY, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_READONLY, L" ");

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_NUM_ENT, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_NUM_ENT, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_NUM_ENT, L" ");

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_FILTER, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_FILTER, IDS_BLANK, uiStyle | SBT_OWNERDRAW, iBMWidth);
    }
  }

  m_statusBar.Invalidate();
  m_statusBar.UpdateWindow();

  /*
  This doesn't exactly belong here, but it makes sure that the
  title is fresh...
  */
  SetWindowText(LPCWSTR(m_titlebar));
}

void DboxMain::SetDCAText(CItemData *pci)
{
  const short si_dca_default = short(PWSprefs::GetInstance()->
                       GetPref(PWSprefs::DoubleClickAction));
  short si_dca(0);
  if (pci == NULL) {
    si_dca = -1;
  } else {
    pci->GetDCA(si_dca);
    if (si_dca == -1)
      si_dca = si_dca_default;
  }

  // Double-click action meaningless if multiple entries selected - pci should be NULL
  // List View only
  if (m_IsListView && m_ctlItemList.GetSelectedCount() != 1)
    si_dca = -1;

  m_statusBar.SetPaneText(CPWStatusBar::SB_DBLCLICK, PWSprefs::GetDCAdescription(si_dca).c_str());
}

struct NoDuplicates{
  NoDuplicates(pws_os::CUUID uuid) : m_uuid(uuid) {}

  bool operator()(const CItemData& item) {
    return item.GetUUID() == m_uuid;
  }
private:
  pws_os::CUUID m_uuid;
};

// Returns a list of entries as they appear in tree in DFS order
void DboxMain::MakeOrderedItemList(OrderedItemList &OIL, HTREEITEM hItem)
{
  // Walk the Tree - either complete tree or only this group!
  if (hItem == NULL) {
    // The whole tree
    while (NULL != (hItem = const_cast<DboxMain *>(this)->m_ctlItemTree.GetNextTreeItem(hItem))) {
      if (!m_ctlItemTree.ItemHasChildren(hItem)) {
        CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
        if (pci != NULL) {
          OIL.push_back(*pci);
        }
      }
    }
  }
  else {
    // Just this group - used for Export Group
    const HTREEITEM hNextSibling = m_ctlItemTree.GetNextSiblingItem(hItem);

    // Get all of the children
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      HTREEITEM hNextItem = m_ctlItemTree.GetNextTreeItem(hItem);

      while (hNextItem != NULL && hNextItem != hNextSibling) {
        CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hNextItem);
        // Include base entires of any aliases or shortcuts
        // and ensure no duplicates (an entry could have been previously added
        // if it was the base of a previously added alias/shortcut
        if (pci != NULL) {
          if (std::find_if(OIL.begin(), OIL.end(), NoDuplicates(pci->GetUUID())) == OIL.end())
            OIL.push_back(*pci);

          if (pci->IsDependent()) {
            pci = GetBaseEntry(pci);
            if (std::find_if(OIL.begin(), OIL.end(), NoDuplicates(pci->GetUUID())) == OIL.end())
              OIL.push_back(*pci);
          }
        }
        hNextItem = m_ctlItemTree.GetNextTreeItem(hNextItem);
      }
    }
  }
}

void DboxMain::UpdateMenuAndToolBar(const bool bOpen)
{
  // Initial setup of menu items and toolbar buttons
  // First set new open/close status
  m_bOpen = bOpen;
  UpdateSystemMenu();

  // For open/close
  const UINT imenuflags = bOpen ? MF_ENABLED : MF_DISABLED | MF_GRAYED;

  // Change Main Menus if a database is Open or not
  CWnd* pMain = AfxGetMainWnd();
  CMenu* xmainmenu = pMain->GetMenu();

  // Look for "File" menu - no longer language dependent
  int pos = app.FindMenuItem(xmainmenu, ID_FILEMENU);

  CMenu* xfilesubmenu = xmainmenu->GetSubMenu(pos);
  if (xfilesubmenu != NULL) {
    // Disable/enable Export and Import menu items
    xfilesubmenu->EnableMenuItem(ID_EXPORTMENU, imenuflags);
    xfilesubmenu->EnableMenuItem(ID_IMPORTMENU, imenuflags);
  }

  if (m_toolbarsSetup == TRUE) {
    TBBUTTONINFO tbinfo;
    int i, nCount, iEnable;
    CToolBarCtrl &tbCtrl = m_MainToolBar.GetToolBarCtrl();
    nCount = tbCtrl.GetButtonCount();

    SecureZeroMemory(&tbinfo, sizeof(tbinfo));
    tbinfo.cbSize = sizeof(tbinfo);
    tbinfo.dwMask = TBIF_BYINDEX | TBIF_COMMAND | TBIF_STYLE;

    for (i = 0; i < nCount; i++) {
      tbCtrl.GetButtonInfo(i, &tbinfo);

      if (tbinfo.fsStyle & TBSTYLE_SEP || 
          tbinfo.idCommand < ID_MENUTOOLBAR_START ||
          tbinfo.idCommand > ID_MENUTOOLBAR_END)
        continue;

      iEnable = OnUpdateMenuToolbar(tbinfo.idCommand);
      if (iEnable < 0)
        continue;
      tbCtrl.EnableButton(tbinfo.idCommand, iEnable);
    }

    if (m_FindToolBar.IsVisible() && !bOpen) {
      OnHideFindToolBar();
    }
  }
}

void DboxMain::U3ExitNow()
{
  // Here upon "soft eject" from U3 device
  if (OnQueryEndSession(0, ENDSESSION_CLOSEAPP) == TRUE) {
    m_inExit = true;
    PostQuitMessage(0);
  }
}

void DboxMain::OnUpdateMenuToolbar(CCmdUI *pCmdUI)
{
  int iEnable(-1);

  switch (pCmdUI->m_nID) {
    // Set menu text to "UnLock" or "Lock" according to state
    case ID_MENUITEM_TRAYUNLOCK:
    case ID_MENUITEM_TRAYLOCK:
    {
      const int i_state = app.GetSystemTrayState();
      switch (i_state) {
        case ThisMfcApp::UNLOCKED:
        case ThisMfcApp::LOCKED:
          break;
        case ThisMfcApp::CLOSED:
        {
          const CString csClosed(MAKEINTRESOURCE(IDS_NOSAFE));
          pCmdUI->SetText(csClosed);
          if (pCmdUI->m_pMenu != NULL) {
            // We want it disabled but not greyed
            pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nID, MF_BYCOMMAND | MF_DISABLED,
                                pCmdUI->m_nID, csClosed);
            return;
          }
          break;
        }
        default:
          ASSERT(0);
          break;
      }
      if (i_state != ThisMfcApp::CLOSED) {
        // If dialog visible - obviously unlocked and no need to have option to lock
        iEnable = this->IsWindowVisible() == FALSE ? TRUE : FALSE;
      }
      break;
    }
    // Enable/Disable menu item depending on data supplied
    case ID_MENUITEM_CLEARRECENTENTRIES:
      if (pCmdUI->m_pSubMenu != NULL) {
        // disable or enable entire popup for "Recent Entries"
        // CCmdUI::Enable is a no-op for this case, so we
        //   must do what it would have done.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION |
          (app.GetSystemTrayState() == ThisMfcApp::UNLOCKED ? MF_ENABLED : MF_GRAYED));
        return;
      }
      // otherwise enable
      iEnable = m_RUEList.GetCount() != 0 ? TRUE : FALSE;
      break;
    default:
      // "Standard" processing for everything else!!!
      iEnable = OnUpdateMenuToolbar(pCmdUI->m_nID);
  }
  if (iEnable < 0)
    return;

  pCmdUI->Enable(iEnable);
}

int DboxMain::OnUpdateMenuToolbar(const UINT nID)
{
  // Return codes:
  // = -1       : don't set pCmdUI->Enable
  // = FALSE(0) : set pCmdUI-Enable(FALSE)
  // = TRUE(1)  : set pCmdUI-Enable(TRUE)

// Don't support importing XML if we can't validate
#if !defined(USE_XML_LIBRARY)
  if (nID == ID_MENUITEM_IMPORT_XML)
    return FALSE;
#endif

  // Special control IDs e.g. Language dynamic menus should always be true
  if (nID >= 64000)
    return TRUE;

  MapUICommandTableConstIter it;
  it = m_MapUICommandTable.find(nID);

  if (it == m_MapUICommandTable.end()) {
    // Don't have it - allow by default
    pws_os::Trace(L"Menu resource ID: %d not found in m_UICommandTable. Please investigate and correct.\n", nID);
    return TRUE;
  }

  int item, iEnable;
  if (!m_bOpen)
    item = UICommandTableEntry::InClosed;
  else if (m_core.IsReadOnly())
    item = UICommandTableEntry::InOpenRO;
  else
    item = UICommandTableEntry::InOpenRW;

  if (item == UICommandTableEntry::InOpenRW &&
      m_core.GetNumEntries() == 0 &&
      m_core.GetEmptyGroups().empty())
    item = UICommandTableEntry::InEmpty;  // OpenRW + empty

  iEnable = m_UICommandTable[it->second].bOK[item] ? TRUE : FALSE;

  // All following special processing will only ever DISABLE an item
  // The previous lookup table is the only mechanism to ENABLE an item

  const bool bTreeView = m_ctlItemTree.IsWindowVisible() == TRUE;
  bool bGroupSelected = false, bFileIsReadOnly(false);
  const CItemData *pci(NULL);
  CItemData::EntryType etype(CItemData::ET_INVALID);

  if (bTreeView) {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    bGroupSelected = (hi != NULL && !m_ctlItemTree.IsLeaf(hi));
    if (hi != NULL)
      pci = (CItemData *)m_ctlItemTree.GetItemData(hi);
  } else {
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos != NULL)
      pci = (CItemData *)m_ctlItemList.GetItemData((int)pos - 1);
  }

  if (pci != NULL) {
    etype = pci->GetEntryType(); // Save entry type before changing pci
    if (pci->IsShortcut()) {
      pci = GetBaseEntry(pci);
    }
  }

  // Special processing!
  switch (nID) {
    // Items not allowed if a Group is selected
    case ID_MENUITEM_DUPLICATEENTRY:
    case ID_MENUITEM_COPYPASSWORD:
    case ID_MENUITEM_AUTOTYPE:
    case ID_MENUITEM_EDIT:
    case ID_MENUITEM_PASSWORDSUBSET:
      if (bGroupSelected)
        iEnable = FALSE;
      break;
    // Not available if group selected or entry is not an alias/shortcut
    case ID_MENUITEM_GOTOBASEENTRY:
    case ID_MENUITEM_EDITBASEENTRY:
      if (bGroupSelected ||
          !(etype == CItemData::ET_SHORTCUT || etype == CItemData::ET_ALIAS))
        iEnable = FALSE;
      break;
    // Not allowed if Group selected or the item selected has an empty field
    case ID_MENUITEM_SENDEMAIL:
      if (bGroupSelected) {
        // Not allowed if a Group is selected
        iEnable = FALSE;
      } else {
        const CItemData *pcix = getSelectedItem();
        if (pcix == NULL) {
          iEnable = FALSE;
        } else {
          if (pcix->IsShortcut()) {
            pcix = GetBaseEntry(pcix);
          }

          if (pcix->IsEmailEmpty() &&
              (pcix->IsURLEmpty() ||
              (!pcix->IsURLEmpty() && !pcix->IsURLEmail()))) {
            iEnable = FALSE;
          }
        }
      }
      break;
    // Not allowed if Group selected or the item selected has an empty field
    case ID_MENUITEM_BROWSEURL:
    case ID_MENUITEM_BROWSEURLPLUS:
    case ID_MENUITEM_COPYUSERNAME:
    case ID_MENUITEM_COPYNOTESFLD:
    case ID_MENUITEM_COPYURL:
    case ID_MENUITEM_COPYEMAIL:
      if (bGroupSelected) {
        // Not allowed if a Group is selected
        iEnable = FALSE;
      } else {
        const CItemData *pcix = getSelectedItem();
        if (pcix == NULL) {
          iEnable = FALSE;
        } else {
          if (pcix->IsShortcut()) {
            pcix = GetBaseEntry(pcix);
          }

          switch (nID) {
            case ID_MENUITEM_COPYUSERNAME:
              if (pcix->IsUserEmpty()) {
                iEnable = FALSE;
              }
              break;
            case ID_MENUITEM_COPYNOTESFLD:
              if (pcix->IsNotesEmpty()) {
                iEnable = FALSE;
              }
              break;
            case ID_MENUITEM_COPYEMAIL:
              if (pcix->IsEmailEmpty() ||
                  (!pcix->IsURLEmpty() && pcix->IsURLEmail())) {
                iEnable = FALSE;
              }
              break;
            case ID_MENUITEM_BROWSEURL:
            case ID_MENUITEM_BROWSEURLPLUS:
            case ID_MENUITEM_COPYURL:
              if (pcix->IsURLEmpty()) {
                iEnable = FALSE;
              }
              break;
          }
        }
      }
      break;
    case ID_MENUITEM_RUNCOMMAND:
    case ID_MENUITEM_COPYRUNCOMMAND:
      if (pci == NULL || pci->IsRunCommandEmpty()) {
        iEnable = FALSE;
      }
      break;
    case ID_MENUITEM_CREATESHORTCUT:
    case ID_MENUITEM_RCREATESHORTCUT:
      if (bGroupSelected) {
        // Not allowed if a Group is selected
        iEnable = FALSE;
      } else {
        CItemData *pcix = getSelectedItem();
        if (pcix == NULL) {
          iEnable = FALSE;
        } else {
          // Can only define a shortcut on a normal entry or
          // one that is already a shortcut base
          if (!pcix->IsNormal() && !pcix->IsShortcutBase()) {
            iEnable = FALSE;
          }
        }
      }
      // No need to check IsDropOnMe here -- it'll be checked inside OnDrop's context menu
      // If we disable shortcut creation when IsDropOnMe() is false, we'll also disable normal
      // shortcut creation for this instance when something was dragged from this instance and then cancelled
      break;
    // Undo/Redo
    case ID_MENUITEM_UNDO:
      iEnable = m_core.AnyToUndo() ? TRUE : FALSE;
      break;
    case ID_MENUITEM_REDO:
      iEnable = m_core.AnyToRedo() ? TRUE : FALSE;
      break;
    // Items not allowed in List View
    case ID_MENUITEM_ADDGROUP:
    case ID_MENUITEM_DUPLICATEGROUP:
    case ID_MENUITEM_RENAMEENTRY:
    case ID_MENUITEM_RENAMEGROUP:
    case ID_MENUITEM_EXPANDALL:
    case ID_MENUITEM_COLLAPSEALL:
      if (m_IsListView)
        iEnable = FALSE;
      break;
    // If not changed, no need to allow Save!
    case ID_MENUITEM_SAVE:
      if ((!m_core.IsChanged() && !m_core.HaveDBPrefsChanged()) ||
            m_core.GetReadFileVersion() < PWSfile::VCURRENT)
        iEnable = FALSE;
      break;
    // Don't allow Options to be changed (as they are mostly V30 and later)
    // if a V1 or V2 database
    case ID_MENUITEM_OPTIONS:
      if (m_core.GetReadFileVersion() < PWSfile::VCURRENT)
        iEnable = FALSE;
      break;
    // Special processing for viewing reports, if they exist
    case ID_MENUITEM_REPORT_COMPARE:
    case ID_MENUITEM_REPORT_FIND:
    case ID_MENUITEM_REPORT_IMPORTTEXT:
    case ID_MENUITEM_REPORT_IMPORTXML:
    case ID_MENUITEM_REPORT_IMPORTKP1CSV:
    case ID_MENUITEM_REPORT_IMPORTKP1TXT:
    case ID_MENUITEM_REPORT_EXPORTTEXT:
    case ID_MENUITEM_REPORT_EXPORTXML:
    case ID_MENUITEM_REPORT_EXPORTDB:
    case ID_MENUITEM_REPORT_MERGE:
    case ID_MENUITEM_REPORT_SYNCHRONIZE:
    case ID_MENUITEM_REPORT_VALIDATE:
      iEnable = OnUpdateViewReports(nID);
      break;
    // Disable choice of toolbar if at low resolution
    case ID_MENUITEM_OLD_TOOLBAR:
    case ID_MENUITEM_NEW_TOOLBAR:
    {
      CDC* pDC = this->GetDC();
      int NumBits = (pDC ? pDC->GetDeviceCaps(12 /*BITSPIXEL*/) : 32);
      if (NumBits < 16 && m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) {
        // Less that 16 color bits available, no choice, disable menu items
        iEnable = FALSE;
      }
      ReleaseDC(pDC);
      break;
    }
    // Disable Minimize if already minimized
    case ID_MENUITEM_MINIMIZE:
    {
      WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
      GetWindowPlacement(&wp);
      if (wp.showCmd == SW_SHOWMINIMIZED)
        iEnable = FALSE;
      break;
    }
    // Disable Restore if already visible
    case ID_MENUITEM_RESTORE:
      if (IsWindowVisible())
        iEnable = FALSE;
      break;
    // Set the state of the "Case Sensitivity" button
    case ID_TOOLBUTTON_FINDCASE:
    case ID_TOOLBUTTON_FINDCASE_I:
    case ID_TOOLBUTTON_FINDCASE_S:
      m_FindToolBar.GetToolBarCtrl().CheckButton(m_FindToolBar.IsFindCaseSet() ?
                           ID_TOOLBUTTON_FINDCASE_S : ID_TOOLBUTTON_FINDCASE_I, 
                           m_FindToolBar.IsFindCaseSet());
      return -1;
    case ID_MENUITEM_SHOWHIDE_UNSAVED:
      // Filter sub-menu mutally exclusive with use of inernal filters for
      // display of unsaved entries or expired entries
      if (!m_core.IsChanged() || 
          (m_bFilterActive && !m_bUnsavedDisplayed))
        iEnable = FALSE;
      break;
    case ID_MENUITEM_SHOW_ALL_EXPIRY:
      // Filter sub-menu mutally exclusive with use of inernal filters for
      // display of unsaved entries or expired entries
      if (m_core.GetExpirySize() == 0 ||
          (m_bFilterActive && !m_bExpireDisplayed))
        iEnable = FALSE;
      break;
    case ID_MENUITEM_CLEAR_MRU:
      if (app.GetMRU()->IsMRUEmpty())
        iEnable = FALSE;
      break;
    case ID_MENUITEM_APPLYFILTER:
      if (m_bUnsavedDisplayed || CurrentFilter().vMfldata.empty() ||
          !CurrentFilter().IsActive())
        iEnable = FALSE;
      break;
    case ID_MENUITEM_EDITFILTER:
    case ID_MENUITEM_MANAGEFILTERS:
      if (m_bUnsavedDisplayed)
        iEnable = FALSE;
      break;
    case ID_MENUITEM_CHANGEMODE:
      // For prior versions, no DB open or file is R-O on disk -
      //  don't give the user the option to change to R/W
      pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly);
      iEnable = (m_bOpen && m_core.GetReadFileVersion() >= PWSfile::VCURRENT && !bFileIsReadOnly) ? TRUE : FALSE;
      break;
    default:
      break;
  }
  return iEnable;
}

void DboxMain::PlaceWindow(CWnd *pWnd, CRect *pRect, UINT uiShowCmd)
{
  WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
  HRGN hrgnWork = GetWorkAreaRegion();

  pWnd->GetWindowPlacement(&wp);  // Get min/max positions - then add what we know
  wp.flags = 0;
  wp.showCmd = uiShowCmd;
  wp.rcNormalPosition = *pRect;

  if (!RectInRegion(hrgnWork, &wp.rcNormalPosition)) {
    if (GetSystemMetrics(SM_CMONITORS) > 1)
      GetMonitorRect(NULL, &wp.rcNormalPosition, FALSE);
    else
      ClipRectToMonitor(NULL, &wp.rcNormalPosition, FALSE);
  }

  pWnd->SetWindowPlacement(&wp);
  ::DeleteObject(hrgnWork);
}

HRGN GetWorkAreaRegion()
{
  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);

  HDC hdc = ::GetDC(NULL);
  EnumDisplayMonitors(hdc, NULL, EnumScreens, (LPARAM)&hrgn);
  ::ReleaseDC(NULL, hdc);

  return hrgn;
}

BOOL CALLBACK EnumScreens(HMONITOR hMonitor, HDC , LPRECT , LPARAM lParam)
{
  MONITORINFO mi;
  HRGN hrgn2;

  HRGN *phrgn = (HRGN *)lParam;

  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  hrgn2 = CreateRectRgnIndirect(&mi.rcWork);
  CombineRgn(*phrgn, *phrgn, hrgn2, RGN_OR);
  ::DeleteObject(hrgn2);

  return TRUE;
}

void DboxMain::GetMonitorRect(HWND hwnd, RECT *prc, BOOL fWork)
{
  MONITORINFO mi;

  mi.cbSize = sizeof(mi);
  GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi);

  if (fWork)
    *prc = mi.rcWork;
  else
    *prc = mi.rcMonitor;
}

void DboxMain::ClipRectToMonitor(HWND hwnd, RECT *prc, BOOL fWork)
{
  RECT rc;
  int w = prc->right  - prc->left;
  int h = prc->bottom - prc->top;

  if (hwnd != NULL) {
    GetMonitorRect(hwnd, &rc, fWork);
  } else {
    MONITORINFO mi;

    mi.cbSize = sizeof(mi);
    GetMonitorInfo(MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST), &mi);

    rc = (fWork) ? mi.rcWork : mi.rcMonitor;
  }

  prc->left = max(rc.left, min(rc.right-w, prc->left));
  prc->top = max(rc.top, min(rc.bottom-h, prc->top));
  prc->right = prc->left + w;
  prc->bottom = prc->top + h;
}

void DboxMain::UpdateSystemMenu()
{
  /**
   * Currently we leave the system menu untouched, even if the
   * last entry ("X Close Alt+F4") is a bit ambiguous:
   * With UseSystemTray, we minimize to the system tray
   * Without it, we exit the application.
   *
   * Leaving this as a 'placeholder' if/when we change out minds.
   */
#if 0
  CMenu *pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu == NULL) // docs don't say when this can occur,
    return;             // but MS's example check this as well!
  BYTE flag = PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray) ? 1 : 0;
  flag |= m_bOpen ? 2 : 0;

  CString cs_text;
  switch (flag) {
    case 0: // Closed + No System Tray : Exit
      cs_text.LoadString(IDS_EXIT);
      break;
    case 1: // Closed +    System Tray : Minimize to System Tray
      cs_text.LoadString(IDS_MIN2ST);
      break;
    case 2: // Open   + No System Tray : Close DB + Exit
      cs_text.LoadString(IDS_CLOSE_EXIT);
      break;
    case 3: // Open   +    System Tray : Minimize to System Tray & maybe lock
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::DatabaseClear))
        cs_text.LoadString(IDS_MIN2STLOCK);
      else
        cs_text.LoadString(IDS_MIN2ST);
      break;
   }
   pSysMenu->ModifyMenu(SC_CLOSE, MF_BYCOMMAND, SC_CLOSE, cs_text);
#endif
}

bool DboxMain::CheckPreTranslateDelete(MSG* pMsg)
{
  // Both TreeCtrl and ListCtrl
  if (pMsg->message == m_wpDeleteMsg && pMsg->wParam == m_wpDeleteKey) {
    if (m_bDeleteCtrl  == ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) && 
        m_bDeleteShift == ((GetKeyState(VK_SHIFT)   & 0x8000) == 0x8000)) {
      if (!m_core.IsReadOnly())
        OnDelete();
      return true;
    }
  }
  return false;
}

bool DboxMain::CheckPreTranslateRename(MSG* pMsg)
{
  // Only TreeCtrl but not ListCtrl!
  if (pMsg->message == m_wpRenameMsg && pMsg->wParam == m_wpRenameKey) {
    if (m_bRenameCtrl  == ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) && 
        m_bRenameShift == ((GetKeyState(VK_SHIFT)   & 0x8000) == 0x8000)) {
      return true;
    }
  }
  return false;
}

bool DboxMain::CheckPreTranslateAutoType(MSG* pMsg)
{
  // Need to handle the keyboard shortcut for AutoType ourselves in order
  // to remove any interaction of the keyboard and AutoTyped characters
  if (m_wpAutotypeKey != 0) {
    // Process user's Autotype shortcut - (Sys)KeyDown
    if (pMsg->message == m_wpAutotypeDNMsg && pMsg->wParam == m_wpAutotypeKey) {
      if (m_bAutotypeCtrl  == ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) && 
          m_bAutotypeShift == ((GetKeyState(VK_SHIFT)   & 0x8000) == 0x8000)) {
        m_btAT.set(0);   // Virtual Key
        if (m_bAutotypeCtrl)
          m_btAT.set(1); // Ctrl Key
        if (m_bAutotypeShift)
          m_btAT.set(2); // Shift Key
        m_bInAT = true;
        return true;
      }
    }

    // Process user's Autotype shortcut - (Sys)KeyUp
    if (m_bInAT &&
        pMsg->message == m_wpAutotypeUPMsg && pMsg->wParam == m_wpAutotypeKey) {
      m_btAT.reset(0);   // Virtual Key
      if (m_btAT.none()) {
        // All keys are now up - send the message
        PerformAutoType();
        m_bInAT = false;
      }
      return true;
    }

    // Process user's Autotype shortcut - KeyUp Ctrl key
    if (m_bInAT && m_bAutotypeCtrl &&
        pMsg->message == WM_KEYUP && pMsg->wParam == VK_CONTROL) {
      m_btAT.reset(1);   // Ctrl Key
      if (m_btAT.none()) {
        // All keys are now up - send the message
        PerformAutoType();
        m_bInAT = false;
      }
      return true;
    }

    // Process user's Autotype shortcut - KeyUp Shift key
    if (m_bInAT && m_bAutotypeShift &&
        pMsg->message == WM_KEYUP && pMsg->wParam == VK_SHIFT) {
      m_btAT.reset(2);   // Shift Key
      if (m_btAT.none()) {
        // All keys are now up - send the message
        PerformAutoType();
        m_bInAT = false;
      }
      return true;
    }
  }
  return false;
}
