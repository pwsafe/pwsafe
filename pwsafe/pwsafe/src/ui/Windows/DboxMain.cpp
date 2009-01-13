/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "AboutDlg.h"
#include "PwFont.h"
#include "MFCMessages.h"
#include "version.h"

#include "corelib/corelib.h"
#include "corelib/PWSprefs.h"
#include "corelib/PWSrand.h"
#include "corelib/PWSdirs.h"
#include "os/file.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

// dialog boxen
#include "DboxMain.h"

#include "TryAgainDlg.h"
#include "PasskeyEntry.h"
#include "ExpPWListDlg.h"
#include "GeneralMsgBox.h"
#include "InfoDisplay.h"
#include "corelib/PWSFilters.h"

// widget override?
#include "SysColStatic.h"

#ifdef POCKET_PC
#include "pocketpc/PocketPC.h"
#include "ShowPasswordDlg.h"
#endif

#include <afxpriv.h>
#include <stdlib.h> // for qsort
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
* This is the string to be displayed instead of the actual password, unless
* the user chooses to see the password:
*/

const TCHAR *HIDDEN_PASSWORD = _T("**************");
CString DboxMain::CS_DELETEENTRY;
CString DboxMain::CS_DELETEGROUP;
CString DboxMain::CS_RENAMEENTRY;
CString DboxMain::CS_RENAMEGROUP;
CString DboxMain::CS_EDITENTRY;
CString DboxMain::CS_VIEWENTRY;
CString DboxMain::CS_BROWSEURL;
CString DboxMain::CS_SENDEMAIL;
CString DboxMain::CS_COPYURL;
CString DboxMain::CS_COPYEMAIL;
CString DboxMain::CS_EXPCOLGROUP;
CString DboxMain::CS_SETFILTERS;
CString DboxMain::CS_CLEARFILTERS;
CString DboxMain::CS_CREATESHORTCUT;
CString DboxMain::CS_GOTOBASEENTRY;
CString DboxMain::CS_DUPLICATEENTRY;
CString DboxMain::CS_COPYPASSWORD;
CString DboxMain::CS_COPYUSERNAME;
CString DboxMain::CS_COPYNOTESFLD;
CString DboxMain::CS_AUTOTYPE;

//-----------------------------------------------------------------------------
DboxMain::DboxMain(CWnd* pParent)
  : CDialog(DboxMain::IDD, pParent),
  m_bSizing(false), m_needsreading(true), m_windowok(false),
  m_toolbarsSetup(FALSE),
  m_bSortAscending(true), m_iTypeSortColumn(CItemData::TITLE),
  m_core(app.m_core), m_pFontTree(NULL),
  m_selectedAtMinimize(NULL), m_bTSUpdated(false),
  m_iSessionEndingStatus(IDIGNORE),
  m_pchTip(NULL), m_pwchTip(NULL),
  m_bValidate(false), m_bOpen(false), 
  m_IsStartClosed(false), m_IsStartSilent(false),
  m_bStartHiddenAndMinimized(false),
  m_bAlreadyToldUserNoSave(false), m_inExit(false),
  m_pCC(NULL), m_bBoldItem(false), m_bIsRestoring(false), m_bImageInLV(false),
  m_lastclipboardaction(_T("")), m_pNotesDisplay(NULL),
  m_LastFoundTreeItem(NULL), m_bFilterActive(false), m_bNumPassedFiltering(0),
  m_currentfilterpool(FPOOL_LAST)
{
  // Set up static versions of menu items.  Old method was to do a LoadString
  // but then we needed 2 copies - one in StringTable and one in the Menu definitions
  // Both would need to be maintained in step and during I18N.
  // Now get it from the Menu directly

  // Now for ones not in the main Menu at startup
  CS_EXPCOLGROUP.LoadString(IDS_MENUEXPCOLGROUP);
  CS_EDITENTRY.LoadString(IDS_MENUEDITENTRY);
  CS_VIEWENTRY.LoadString(IDS_MENUVIEWENTRY);
  CS_DELETEGROUP.LoadString(IDS_MENUDELETEGROUP);
  CS_RENAMEGROUP.LoadString(IDS_MENURENAMEGROUP);
  CS_SENDEMAIL.LoadString(IDS_MENUSENDEMAIL);
  CS_COPYEMAIL.LoadString(IDS_MENUCOPYEMAIL);
  CS_CREATESHORTCUT.LoadString(IDS_MENUCREATESHORTCUT);
  CS_SETFILTERS.LoadString(IDS_SETFILTERS);
  CS_CLEARFILTERS.LoadString(IDS_CLEARFILTERS);

  //{{AFX_DATA_INIT(DboxMain)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT

  m_hIcon = app.LoadIcon(IDI_CORNERICON);
  m_hIconSm = (HICON) ::LoadImage(app.m_hInstance, MAKEINTRESOURCE(IDI_CORNERICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

  ClearData();

#if !defined(POCKET_PC)
  m_titlebar = _T("");
  m_toolbarsSetup = FALSE;
#endif
}

DboxMain::~DboxMain()
{
  m_core.UnRegisterOnListModified();
  ::DestroyIcon(m_hIcon);
  ::DestroyIcon(m_hIconSm);

  delete m_pchTip;
  delete m_pwchTip;
  delete m_pFontTree;
  DeletePasswordFont();
}

LRESULT DboxMain::OnAreYouMe(WPARAM, LPARAM)
{
  return (LRESULT)app.m_uiRegMsg;
}

BEGIN_MESSAGE_MAP(DboxMain, CDialog)
  //{{AFX_MSG_MAP(DboxMain)

  ON_REGISTERED_MESSAGE(app.m_uiRegMsg, OnAreYouMe)
  ON_UPDATE_COMMAND_UI_RANGE(ID_MENUTOOLBAR_START, ID_MENUTOOLBAR_END, OnUpdateMenuToolbar)

  // File Menu
  ON_COMMAND(ID_MENUITEM_NEW, OnNew)
  ON_COMMAND(ID_MENUITEM_OPEN, OnOpen)
  ON_COMMAND(ID_MENUITEM_CLOSE, OnClose)
  ON_COMMAND(ID_MENUITEM_CLEAR_MRU, OnClearMRU)
  ON_COMMAND(ID_MENUITEM_SAVE, OnSave)
  ON_COMMAND(ID_MENUITEM_SAVEAS, OnSaveAs)
  ON_COMMAND_RANGE(ID_MENUITEM_EXPORT2OLD1XFORMAT, ID_MENUITEM_EXPORT2V2FORMAT, OnExportVx)
  ON_COMMAND(ID_MENUITEM_EXPORT2PLAINTEXT, OnExportText)
  ON_COMMAND(ID_MENUITEM_EXPORT2XML, OnExportXML)
  ON_COMMAND(ID_MENUITEM_IMPORT_PLAINTEXT, OnImportText)
  ON_COMMAND(ID_MENUITEM_IMPORT_KEEPASS, OnImportKeePass)
  ON_COMMAND(ID_MENUITEM_IMPORT_XML, OnImportXML)
  ON_COMMAND(ID_MENUITEM_MERGE, OnMerge)
  ON_COMMAND(ID_MENUITEM_COMPARE, OnCompare)
  ON_COMMAND(ID_MENUITEM_PROPERTIES, OnProperties)

  // Edit Menu
  ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
  ON_COMMAND(ID_MENUITEM_ADDGROUP, OnAddGroup)
  ON_COMMAND(ID_MENUITEM_CREATESHORTCUT, OnCreateShortcut)
  ON_COMMAND(ID_MENUITEM_EDIT, OnEdit)
  ON_COMMAND(ID_MENUITEM_GROUPENTER, OnEdit)
  ON_COMMAND(ID_MENUITEM_BROWSEURL, OnBrowse)
  ON_COMMAND(ID_MENUITEM_SENDEMAIL, OnBrowse)
  ON_COMMAND(ID_MENUITEM_COPYPASSWORD, OnCopyPassword)
  ON_COMMAND(ID_MENUITEM_COPYNOTESFLD, OnCopyNotes)
  ON_COMMAND(ID_MENUITEM_COPYUSERNAME, OnCopyUsername)
  ON_COMMAND(ID_MENUITEM_COPYURL, OnCopyURL)
  ON_COMMAND(ID_MENUITEM_COPYEMAIL, OnCopyURL)
  ON_COMMAND(ID_MENUITEM_CLEARCLIPBOARD, OnClearClipboard)
  ON_COMMAND(ID_MENUITEM_DELETE, OnDelete)
  ON_COMMAND(ID_MENUITEM_RENAME, OnRename)
  ON_COMMAND(ID_MENUITEM_FIND, OnFind)
  ON_COMMAND(ID_MENUITEM_DUPLICATEENTRY, OnDuplicateEntry)
  ON_COMMAND(ID_MENUITEM_AUTOTYPE, OnAutoType)
  ON_COMMAND(ID_MENUITEM_GOTOBASEENTRY, OnGotoBaseEntry)

  // View Menu
  ON_COMMAND(ID_MENUITEM_LIST_VIEW, OnListView)
  ON_COMMAND(ID_MENUITEM_TREE_VIEW, OnTreeView)
  ON_COMMAND(ID_MENUITEM_SHOWHIDE_TOOLBAR, OnShowHideToolbar)
  ON_COMMAND(ID_MENUITEM_SHOWHIDE_DRAGBAR, OnShowHideDragbar)
  ON_COMMAND(ID_MENUITEM_OLD_TOOLBAR, OnOldToolbar)
  ON_COMMAND(ID_MENUITEM_NEW_TOOLBAR, OnNewToolbar)
  ON_COMMAND(ID_MENUITEM_EXPANDALL, OnExpandAll)
  ON_COMMAND(ID_MENUITEM_COLLAPSEALL, OnCollapseAll)
  ON_COMMAND(ID_MENUITEM_CHANGETREEFONT, OnChangeTreeFont)
  ON_COMMAND(ID_MENUITEM_CHANGEPSWDFONT, OnChangePswdFont)
  ON_COMMAND_RANGE(ID_MENUITEM_REPORT_COMPARE, ID_MENUITEM_REPORT_VALIDATE, OnViewReports)
  ON_COMMAND(ID_MENUITEM_APPLYFILTER, OnApplyFilter)
  ON_COMMAND(ID_MENUITEM_EDITFILTER, OnSetFilter)
  ON_COMMAND(ID_MENUITEM_MANAGEFILTERS, OnManageFilters)
  ON_COMMAND(ID_MENUITEM_PASSWORDSUBSET, OnDisplayPswdSubset)
  ON_COMMAND(ID_MENUITEM_REFRESH, OnRefreshWindow)

  // Manage Menu
  ON_COMMAND(ID_MENUITEM_CHANGECOMBO, OnPasswordChange)
  ON_COMMAND(ID_MENUITEM_BACKUPSAFE, OnBackupSafe)
  ON_COMMAND(ID_MENUITEM_RESTORE, OnRestore)
  ON_COMMAND(ID_MENUITEM_OPTIONS, OnOptions)

  // Help Menu
  ON_COMMAND(ID_MENUITEM_ABOUT, OnAbout)
  ON_COMMAND(ID_MENUITEM_PWSAFE_WEBSITE, OnPasswordSafeWebsite)
  ON_COMMAND(ID_MENUITEM_U3SHOP_WEBSITE, OnU3ShopWebsite)

  // List view Column Picker
  ON_COMMAND(ID_MENUITEM_COLUMNPICKER, OnColumnPicker)
  ON_COMMAND(ID_MENUITEM_RESETCOLUMNS, OnResetColumns)

  // Others
  ON_COMMAND(ID_MENUITEM_VALIDATE, OnValidate)

#if defined(POCKET_PC)
  ON_WM_CREATE()
#else
  ON_WM_CONTEXTMENU()
#endif

  // Windows Messages
  ON_WM_DESTROY()
  ON_WM_ENDSESSION()
  ON_WM_INITMENU()
  ON_WM_INITMENUPOPUP()
  ON_WM_QUERYENDSESSION()
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
  ON_COMMAND(ID_MENUITEM_UNMINIMIZE, OnUnMinimize)

#if defined(POCKET_PC)
  ON_COMMAND(ID_MENUITEM_SHOWPASSWORD, OnShowPassword)
#else
  ON_WM_DROPFILES()
  ON_WM_SIZING()
  ON_NOTIFY(NM_SETFOCUS, IDC_ITEMLIST, OnChangeItemFocus)
  ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMLIST, OnChangeItemFocus)
  ON_NOTIFY(NM_SETFOCUS, IDC_ITEMTREE, OnChangeItemFocus)
  ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMTREE, OnChangeItemFocus)
  ON_COMMAND(ID_MENUITEM_TRAYLOCK, OnTrayLockUnLock)
  ON_COMMAND(ID_MENUITEM_TRAYUNLOCK, OnTrayLockUnLock)
  ON_COMMAND(ID_MENUITEM_CLEARRECENTENTRIES, OnTrayClearRecentEntries)
  ON_COMMAND(ID_TOOLBUTTON_LISTTREE, OnToggleView)
  ON_COMMAND(ID_TOOLBUTTON_VIEWREPORTS, OnViewReports)

  ON_COMMAND(ID_TOOLBUTTON_CLOSEFIND, OnHideFindToolBar)
  ON_COMMAND(ID_TOOLBUTTON_FIND, OnToolBarFind)
  ON_COMMAND(ID_TOOLBUTTON_FINDCASE, OnToolBarFindCase)
  ON_COMMAND(ID_TOOLBUTTON_FINDCASE_I, OnToolBarFindCase)
  ON_COMMAND(ID_TOOLBUTTON_FINDCASE_S, OnToolBarFindCase)
  ON_COMMAND(ID_TOOLBUTTON_FINDADVANCED, OnToolBarFindAdvanced)
  ON_COMMAND(ID_TOOLBUTTON_FINDREPORT, OnToolBarFindReport)
  ON_COMMAND(ID_TOOLBUTTON_CLEARFIND, OnToolBarClearFind)
#endif

#ifndef POCKET_PC
  ON_BN_CLICKED(IDOK, OnEdit)
#endif

  ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
  ON_MESSAGE(WM_HOTKEY, OnHotKey)
  ON_MESSAGE(WM_CCTOHDR_DD_COMPLETE, OnCCToHdrDragComplete)
  ON_MESSAGE(WM_HDRTOCC_DD_COMPLETE, OnHdrToCCDragComplete)
  ON_MESSAGE(WM_HDR_DRAG_COMPLETE, OnHeaderDragComplete)
  ON_MESSAGE(WM_COMPARE_RESULT_FUNCTION, OnProcessCompareResultFunction)
  ON_MESSAGE(WM_TOOLBAR_FIND, OnToolBarFindMessage)
  ON_MESSAGE(WM_EXECUTE_FILTERS, OnExecuteFilters)

  ON_COMMAND(ID_MENUITEM_CUSTOMIZETOOLBAR, OnCustomizeToolbar)

  //}}AFX_MSG_MAP
  ON_COMMAND_EX_RANGE(ID_FILE_MRU_ENTRY1, ID_FILE_MRU_ENTRYMAX, OnOpenMRU)
  ON_UPDATE_COMMAND_UI(ID_FILE_MRU_ENTRY1, OnUpdateMRU)  // Note: can't be in OnUpdateMenuToolbar!
#ifndef POCKET_PC
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
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
#endif
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
  {ID_MENUITEM_EXPORT2XML, true, true, false, false},
  {ID_MENUITEM_IMPORT_XML, true, false, true, false},
  {ID_MENUITEM_IMPORT_PLAINTEXT, true, false, true, false},
  {ID_MENUITEM_IMPORT_KEEPASS, true, false, true, false},
  {ID_MENUITEM_MERGE, true, false, true, false},
  {ID_MENUITEM_COMPARE, true, true, false, false},
  {ID_MENUITEM_PROPERTIES, true, true, true, false},
  {ID_MENUITEM_EXIT, true, true, true, true},
  // Edit menu
  {ID_MENUITEM_ADD, true, false, true, false},
  {ID_MENUITEM_CREATESHORTCUT, true, false, false, false},
  {ID_MENUITEM_EDIT, true, true, false, false},
  {ID_MENUITEM_GROUPENTER, true, true, false, false},
  {ID_MENUITEM_DELETE, true, false, false, false},
  {ID_MENUITEM_RENAME, true, false, false, false},
  {ID_MENUITEM_FIND, true, true, false, false},
  {ID_MENUITEM_DUPLICATEENTRY, true, false, false, false},
  {ID_MENUITEM_ADDGROUP, true, false, true, false},
  {ID_MENUITEM_COPYPASSWORD, true, true, false, false},
  {ID_MENUITEM_COPYUSERNAME, true, true, false, false},
  {ID_MENUITEM_COPYNOTESFLD, true, true, false, false},
  {ID_MENUITEM_CLEARCLIPBOARD, true, true, true, false},
  {ID_MENUITEM_BROWSEURL, true, true, false, false},
  {ID_MENUITEM_SENDEMAIL, true, true, false, false},
  {ID_MENUITEM_AUTOTYPE, true, true, false, false},
  {ID_MENUITEM_COPYURL, true, true, false, false},
  {ID_MENUITEM_COPYEMAIL, true, true, false, false},
  {ID_MENUITEM_GOTOBASEENTRY, true, true, false, false},
  // View menu
  {ID_MENUITEM_LIST_VIEW, true, true, true, false},
  {ID_MENUITEM_TREE_VIEW, true, true, true, false},
  {ID_MENUITEM_SHOWHIDE_TOOLBAR, true, true, true, true},
  {ID_MENUITEM_SHOWHIDE_DRAGBAR, true, true, true, true},
  {ID_MENUITEM_NEW_TOOLBAR, true, true, true, true},
  {ID_MENUITEM_OLD_TOOLBAR, true, true, true, true},
  {ID_MENUITEM_EXPANDALL, true, true, true, false},
  {ID_MENUITEM_COLLAPSEALL, true, true, true, false},
  {ID_MENUITEM_CHANGETREEFONT, true, true, true, false},
  {ID_MENUITEM_CHANGEPSWDFONT, true, true, true, false},
  {ID_MENUITEM_REPORT_COMPARE, true, true, true, true},
  {ID_MENUITEM_REPORT_FIND, true, true, true, true},
  {ID_MENUITEM_REPORT_IMPORTTEXT, true, true, true, true},
  {ID_MENUITEM_REPORT_IMPORTXML, true, true, true, true},
  {ID_MENUITEM_REPORT_MERGE, true, true, true, true},
  {ID_MENUITEM_REPORT_VALIDATE, true, true, true, true},
  {ID_MENUITEM_EDITFILTER, true, true, false, false},
  {ID_MENUITEM_APPLYFILTER, true, true, false, false},
  {ID_MENUITEM_MANAGEFILTERS, true, true, true, true},
  {ID_MENUITEM_PASSWORDSUBSET, true, true, false, false},
  {ID_MENUITEM_REFRESH, true, true, false, false},
  // Manage menu
  {ID_MENUITEM_CHANGECOMBO, true, false, true, false},
  {ID_MENUITEM_BACKUPSAFE, true, true, true, false},
  {ID_MENUITEM_RESTORE, true, false, true, false},
  {ID_MENUITEM_OPTIONS, true, true, true, true},
  {ID_MENUITEM_VALIDATE, true, false, false, true},
  // Help Menu
  {ID_MENUITEM_PWSAFE_WEBSITE, true, true, true, true},
  {ID_MENUITEM_ABOUT, true, true, true, true},
  {ID_MENUITEM_U3SHOP_WEBSITE, true, true, true, true},
  // Column popup menu
  {ID_MENUITEM_COLUMNPICKER, true, true, true, false},
  {ID_MENUITEM_RESETCOLUMNS, true, true, true, false},
  // Compare popup menu
  {ID_MENUITEM_COMPVIEWEDIT, true, false, true, false},
  {ID_MENUITEM_COPY_TO_ORIGINAL, true, false, true, false},
  {ID_MENUITEM_COPY_TO_COMPARISON, true, true, true, false},
  // Tray popup menu
  {ID_MENUITEM_TRAYUNLOCK, true, true, true, false},
  {ID_MENUITEM_TRAYLOCK, true, true, true, false},
  {ID_MENUITEM_CLEARRECENTENTRIES, true, true, true, false},
  {ID_MENUITEM_MINIMIZE, true, true, true, true},
  {ID_MENUITEM_UNMINIMIZE, true, true, true, true},
  // Default Main Toolbar buttons - if not menu items
  //   None
  // Optional Main Toolbar buttons
  {ID_TOOLBUTTON_LISTTREE, true, true, true, false},
  {ID_TOOLBUTTON_VIEWREPORTS, true, true, true, false},
  // Find Toolbar
  {ID_TOOLBUTTON_CLOSEFIND, true, true, true, false},
  {ID_TOOLBUTTON_FINDEDITCTRL, true, true, false, false},
  {ID_TOOLBUTTON_FIND, true, true, false, false},
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
  PWSprefs *prefs = PWSprefs::GetInstance();
  // Real initialization done here
  // Requires OnInitDialog to have passed OK
  UpdateAlwaysOnTop();

  // ... same for UseSystemTray
  // StartSilent trumps preference (but StartClosed doesn't)
  if (!m_IsStartSilent && !prefs->GetPref(PWSprefs::UseSystemTray))
    app.HideIcon();

  m_RUEList.SetMax(prefs->GetPref(PWSprefs::MaxREItems));

  // Set timer for user-defined lockout, if selected
  if (prefs->GetPref(PWSprefs::LockOnIdleTimeout)) {
    const UINT MINUTE = 60*1000;
    SetTimer(TIMER_USERLOCK, MINUTE, NULL);
    ResetIdleLockCounter();
  }

  // JHF : no hotkeys on WinCE
#if !defined(POCKET_PC)
  // Set Hotkey, if active
  if (prefs->GetPref(PWSprefs::HotKeyEnabled)) {
    const DWORD value = DWORD(prefs->GetPref(PWSprefs::HotKey));
    WORD wVirtualKeyCode = WORD(value & 0xffff);
    WORD mod = WORD(value >> 16);
    WORD wModifiers = 0;
    // Translate between CWnd & CHotKeyCtrl modifiers
    if (mod & HOTKEYF_ALT)
      wModifiers |= MOD_ALT;
    if (mod & HOTKEYF_CONTROL)
      wModifiers |= MOD_CONTROL;
    if (mod & HOTKEYF_SHIFT)
      wModifiers |= MOD_SHIFT;
    RegisterHotKey(m_hWnd, PWS_HOTKEY_ID, UINT(wModifiers), UINT(wVirtualKeyCode));
    // registration might fail if combination already registered elsewhere,
    // but don't see any elegant way to notify the user here, so fail silently
  } else {
    // No sense in unregistering at this stage, really.
  }
#endif

  m_windowok = true;

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog

  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIconSm, FALSE); // Set small icon

  // Init stuff for tree view
  CBitmap bitmap;
  BITMAP bm;

  // Change all pixels in this 'grey' to transparent
  const COLORREF crTransparent = RGB(192, 192, 192);

  bitmap.LoadBitmap(IDB_NODE);
  bitmap.GetBitmap(&bm);
  
  m_pImageList = new CImageList();
  // Number (12) corresponds to number in CPWTreeCtrl public enum
  BOOL status = m_pImageList->Create(bm.bmWidth, bm.bmHeight, 
                                     ILC_MASK | ILC_COLOR, 12, 0);
  ASSERT(status != 0);

  // Dummy Imagelist needed if user adds then removes Icon column
  m_pImageList0 = new CImageList();
  status = m_pImageList0->Create(1, 1, ILC_MASK | ILC_COLOR, 0, 1);
  ASSERT(status != 0);

  // Order of LoadBitmap() calls matches CPWTreeCtrl public enum
  // Also now used by CListCtrl!
  //bitmap.LoadBitmap(IDB_NODE); - already loaded above to get width
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_NORMAL);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_NORMAL_WARNEXPIRED);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_NORMAL_EXPIRED);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_ABASE);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_ABASE_WARNEXPIRED);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_ABASE_EXPIRED);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_ALIAS);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_SBASE);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_SBASE_WARNEXPIRED);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_SBASE_EXPIRED);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_SHORTCUT);
  m_pImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();

  m_ctlItemTree.SetImageList(m_pImageList, TVSIL_NORMAL);
  m_ctlItemTree.SetImageList(m_pImageList, TVSIL_STATE);

  if (prefs->GetPref(PWSprefs::ShowNotesAsTooltipsInViews)) {
    m_ctlItemTree.ActivateND(true);
    m_ctlItemList.ActivateND(true);
  } else {
    m_ctlItemTree.ActivateND(false);
    m_ctlItemList.ActivateND(false);
  }

  DWORD dw_ExtendedStyle = LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP;
  if (prefs->GetPref(PWSprefs::ListViewGridLines))
    dw_ExtendedStyle |= LVS_EX_GRIDLINES;

  m_ctlItemList.SetExtendedStyle(dw_ExtendedStyle);
  m_ctlItemList.Initialize();

  // Override default HeaderCtrl ID of 0
  m_LVHdrCtrl.SetDlgCtrlID(IDC_LIST_HEADER);

  // Initialise DropTargets - should be in OnCreate()s, really
  m_LVHdrCtrl.Initialize(&m_LVHdrCtrl);
  m_ctlItemTree.Initialize();

  // Set up fonts before playing with Tree/List views
  m_pFontTree = new CFont;
  CString szTreeFont = prefs->GetPref(PWSprefs::TreeFont).c_str();

  if (szTreeFont != _T("")) {
    LOGFONT *ptreefont = new LOGFONT;
    memset(ptreefont, 0, sizeof(LOGFONT)); 
    ExtractFont(szTreeFont, ptreefont);
    m_pFontTree->CreateFontIndirect(ptreefont);
    // transfer the fonts to the tree windows
    m_ctlItemTree.SetFont(m_pFontTree);
    m_ctlItemList.SetFont(m_pFontTree);
    m_LVHdrCtrl.SetFont(m_pFontTree);
    delete ptreefont;
  }

  // Set up fonts before playing with Tree/List views
  CString szPasswordFont = prefs->GetPref(PWSprefs::PasswordFont).c_str();

  if (szPasswordFont != _T("")) {
    LOGFONT *pPasswordfont = new LOGFONT;
    memset(pPasswordfont, 0, sizeof(LOGFONT)); 
    ExtractFont(szPasswordFont, pPasswordfont);
    SetPasswordFont(pPasswordfont);
    delete pPasswordfont;
  }

  const CString lastView = prefs->GetPref(PWSprefs::LastView).c_str();
  if (lastView != _T("list"))
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
    case CItemData::AUTOTYPE:
    case CItemData::POLICY:
    break;
    case CItemData::PWHIST:  // Not displayed in ListView
    default:
      // Title is a mandatory column - so can't go wrong!
      m_iTypeSortColumn = CItemData::TITLE;
      break;
  }

  // refresh list will add and size password column if necessary...
  RefreshViews();

  ChangeOkUpdate();

  setupBars(); // Just to keep things a little bit cleaner

#if !defined(POCKET_PC)
  // {kjp} Can't drag and drop files onto an application in PocketPC
  DragAcceptFiles(TRUE);

  // {kjp} meaningless when target is a PocketPC device.
  CRect rect;
  prefs->GetPrefRect(rect.top, rect.bottom, rect.left, rect.right);

  if (rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) {
    GetWindowRect(&rect);
    SendMessage(WM_SIZE, SIZE_RESTORED, MAKEWPARAM(rect.Width(), rect.Height()));
  } else {
    PlaceWindow(&rect, SW_HIDE);
  }
#endif

  m_core.SetUseDefUser(prefs->GetPref(PWSprefs::UseDefaultUser));
  m_core.SetDefUsername(prefs->GetPref(PWSprefs::DefaultUsername));

  SetMenu(app.m_mainmenu);  // Now show menu...

  // Now do widths!
  if (!cs_ListColumns.IsEmpty())
    SetColumnWidths(cs_ListColumnsWidths);

  // create notes info display window
  m_pNotesDisplay = new CInfoDisplay;
  if (!m_pNotesDisplay->Create(0, 0, _T(""), this)) {
    // failed
    delete m_pNotesDisplay;
    m_pNotesDisplay = NULL;
  }
#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
  // Don't support filter processing on non-Windows platforms 
  // using Microsoft XML libraries
#else
  // if there's a filter file named "autoload_filters.xml", 
  // do what its name implies...
  CString cs_temp = CString(PWSdirs::GetConfigDir().c_str()) +
    _T("autoload_filters.xml");
  if (pws_os::FileExists(cs_temp.GetString())) {
    stringT strErrors;
    stringT XSDFilename = PWSdirs::GetXMLDir() + _T("pwsafe_filter.xsd");

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  // Expat is a non-validating parser - no use for Schema!
  if (!pws_os::FileExists(XSDFilename)) {
    CString cs_title, cs_msg;
    cs_temp.Format(IDSC_MISSINGXSD, _T("pwsafe_filter.xsd"));
    cs_msg.Format(IDS_CANTAUTOIMPORTFILTERS, cs_temp);
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    MessageBox(cs_msg, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }
#endif

    CWaitCursor waitCursor;  // This may take a while!

    MFCAsker q;
    int rc = m_MapFilters.ImportFilterXMLFile(FPOOL_AUTOLOAD, _T(""),
                                              stringT(cs_temp),
                                              XSDFilename.c_str(), strErrors, &q);
    waitCursor.Restore();  // Restore normal cursor
    if (rc != PWScore::SUCCESS){
      CString cs_msg;
      cs_msg.Format(IDS_CANTAUTOIMPORTFILTERS, strErrors.c_str());
      AfxMessageBox(cs_msg, MB_OK);
    }
  }
#endif
}

LRESULT DboxMain::OnHotKey(WPARAM , LPARAM)
{
  // since we only have a single HotKey, the value assigned
  // to it is meaningless & unused, hence params ignored
  // The hotkey is used to invoke the app window, prompting
  // for passphrase if needed.

  app.SetHotKeyPressed(true);
  if (IsIconic()) {
    SendMessage(WM_COMMAND, ID_MENUITEM_UNMINIMIZE);
  }
  SetActiveWindow();
  SetForegroundWindow();
  return 0;
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

  RefreshViews(iListOnly);

  return 0L;
}

BOOL DboxMain::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Set up UPDATE_UI data map.
  const int num_CommandTable_entries = sizeof(m_UICommandTable) / sizeof(UICommandTableEntry);
  for (int i = 0; i < num_CommandTable_entries; i++)
    m_MapUICommandTable[m_UICommandTable[i].ID] = i;

  // Install menu popups for full path on MRU entries
  m_menuManager.Install(AfxGetMainWnd());
  m_menuTipManager.Install(AfxGetMainWnd());

  // Subclass the ListView HeaderCtrl
  CHeaderCtrl* pHeader;
  pHeader = m_ctlItemList.GetHeaderCtrl();
  if(pHeader && pHeader->GetSafeHwnd()) {
    m_LVHdrCtrl.SubclassWindow(pHeader->GetSafeHwnd());
  }

  ConfigureSystemMenu();
  InitPasswordSafe();

  if (m_IsStartSilent) {
    m_bStartHiddenAndMinimized = true;
  }

  if (m_IsStartClosed) {
    Close();
    if (!m_IsStartSilent)
      ShowWindow(SW_SHOW);
  }

  if (!m_IsStartClosed && !m_IsStartSilent) {
    OpenOnInit();
    m_ctlItemTree.SetRestoreMode(true);
    RefreshViews();
    m_ctlItemTree.SetRestoreMode(false);
  }

  SetInitialDatabaseDisplay();
  if (m_bOpen && PWSprefs::GetInstance()->GetPref(PWSprefs::ShowFindToolBarOnOpen) == TRUE)
    SetFindToolBar(true);
  else
    OnHideFindToolBar();

  if (m_bOpen) {
    SelectFirstEntry();
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

void DboxMain::SetInitialDatabaseDisplay()
{
  if (m_ctlItemTree.GetCount() > 0)
    switch (PWSprefs::GetInstance()->GetPref(PWSprefs::TreeDisplayStatusAtOpen)) {
      case PWSprefs::AllCollapsed:
        m_ctlItemTree.OnCollapseAll();
        break;
      case PWSprefs::AllExpanded:
        m_ctlItemTree.OnExpandAll();
        break;
      case PWSprefs::AsPerLastSave:
        RestoreDisplayStatus();
        break;
      default:
        ASSERT(0);
    }
}

void DboxMain::OnDestroy()
{
  const stringT filename(m_core.GetCurFile().c_str());
  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.empty() && !m_core.IsReadOnly() && m_core.IsLockedFile(filename))
    m_core.UnlockFile(filename);

  // Get rid of hotkey
  UnregisterHotKey(m_hWnd, PWS_HOTKEY_ID);

  // Stop subclassing the ListView HeaderCtrl
  if (m_LVHdrCtrl.GetSafeHwnd() != NULL)
    m_LVHdrCtrl.UnsubclassWindow();

  // Stop Drag & Drop OLE
  m_LVHdrCtrl.Terminate();

  // and goodbye
  CDialog::OnDestroy();
}

void DboxMain::OnWindowPosChanging( WINDOWPOS* lpwndpos )
{
  if (m_bStartHiddenAndMinimized) {
    lpwndpos->flags |= (SWP_HIDEWINDOW | SWP_NOACTIVATE);
    lpwndpos->flags &= ~SWP_SHOWWINDOW;
    PostMessage(WM_COMMAND, ID_MENUITEM_MINIMIZE);
  }

  CDialog::OnWindowPosChanging(lpwndpos);
}

void DboxMain::OnMove(int x, int y)
{
  CDialog::OnMove(x, y);
  // turns out that minimizing calls this
  // with x = y = -32000. Oh joy.
  if (m_windowok && IsWindowVisible() == TRUE &&
      x >= 0 && y >= 0) {
    CRect rect;
    GetWindowRect(&rect);
    PWSprefs::GetInstance()->SetPrefRect(rect.top, rect.bottom,
                                         rect.left, rect.right);
  }
}

void DboxMain::FixListIndexes()
{
  int N = m_ctlItemList.GetItemCount();
  for (int i = 0; i < N; i++) {
    CItemData *ci = (CItemData *)m_ctlItemList.GetItemData(i);
    ASSERT(ci != NULL);
    if (m_bFilterActive && !PassesFiltering(*ci, m_currentfilter))
      continue;
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    if (di->list_index != i)
      di->list_index = i;
  }
}

void DboxMain::OnItemDoubleClick(NMHDR * /* pNotifyStruct */, LRESULT *pLResult)
{
  *pLResult = 0L;
  UnFindItem();

  // TreeView only - use DoubleClick to Expand/Collapse group
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hItem = m_ctlItemTree.GetSelectedItem();
    // Only if a group is selected
    if ((hItem != NULL && !m_ctlItemTree.IsLeaf(hItem))) {
      // Do standard double-click processing - i.e. toggle expand/collapse!
      return;
    }
  }

  // Now set we have processed the event
  *pLResult = 1L;

  // Continue if in ListView  or Leaf in TreeView

#if defined(POCKET_PC)
  if ( app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dcshowspassword"), FALSE) == FALSE ) {
    OnCopyPassword();
  } else {
    OnShowPassword();
  }
#else
  switch (PWSprefs::GetInstance()->GetPref(PWSprefs::DoubleClickAction)) {
    case PWSprefs::DoubleClickAutoType:
      PostMessage(WM_COMMAND, ID_MENUITEM_AUTOTYPE);
      break;
    case PWSprefs::DoubleClickBrowse:
      PostMessage(WM_COMMAND, ID_MENUITEM_BROWSEURL);
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
      PostMessage(WM_COMMAND, ID_MENUITEM_EDIT);
      break;
    default:
      ASSERT(0);
  }
#endif
}

// Called to open a web browser to the URL associated with an entry.
void DboxMain::OnBrowse()
{
  CItemData *ci = getSelectedItem();
  CItemData *ci_original(ci);

  if(ci != NULL) {
    if (ci->IsShortcut()) {
      // This is an shortcut
      uuid_array_t entry_uuid, base_uuid;
      ci->GetUUID(entry_uuid);
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

      ItemListIter iter = m_core.Find(base_uuid);
      if (iter != End()) {
        ci = &iter->second;
      }
    }

    if (!ci->IsURLEmpty()) {
      LaunchBrowser(ci->GetURL().c_str());
      UpdateAccessTime(ci_original);
    }
  }
}

// this tells OnSize that the user is currently
// changing the size of the dialog, and not restoring it
void DboxMain::OnSizing(UINT fwSide, LPRECT pRect)
{
#if !defined(POCKET_PC)
  CDialog::OnSizing(fwSide, pRect);

  m_bSizing = true;
#endif
}

void DboxMain::OnUpdateNSCommand(CCmdUI *pCmdUI)
{
  // Use this callback  for commands that need to
  // be disabled if not supported (yet)
  pCmdUI->Enable(FALSE);
}

void  DboxMain::SetStartSilent(bool state)
{
  m_IsStartSilent = state;
  if (state) {
    // start silent implies use system tray.
    PWSprefs::GetInstance()->SetPref(PWSprefs::UseSystemTray, true);  
  }
}

void DboxMain::SetChanged(ChangeType changed)
{
  switch (changed) {
    case Data:
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately)) {
        Save();
      } else {
        m_core.SetChanged(true);
      }
      break;
    case Clear:
      m_core.SetChanged(false);
      m_bTSUpdated = false;
      break;
    case TimeStamp:
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::MaintainDateTimeStamps))
        m_bTSUpdated = true;
      break;
    case DBPrefs:
      m_core.SetDBPrefsChanged(true);
      break;
    case ClearDBPrefs:
      m_core.SetDBPrefsChanged(false);
      break;
    default:
      ASSERT(0);
  }
}

void DboxMain::ChangeOkUpdate()
{
  if (!m_windowok)
    return;

#if defined(POCKET_PC)
  CMenu *menu = m_wndMenu;
#else
  CMenu *menu = GetMenu();
#endif

  // Don't need to worry about R-O, as IsChanged can't be true in this case
  menu->EnableMenuItem(ID_MENUITEM_SAVE,
    (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) ? MF_ENABLED : MF_GRAYED);
  if (m_toolbarsSetup == TRUE) {
    m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_SAVE,
      (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) ? TRUE : FALSE);
  }
#ifdef DEMO
  int update = OnUpdateMenuToolbar(ID_MENUITEM_ADD);
  // Cheat, as we know that the logic for ADD applies to others, in DEMO mode
  // see OnUpdateMenuToolbar
  if (m_MainToolBar.GetSafeHwnd() != NULL && update != -1) {
    BOOL state = update ? TRUE : FALSE;
    m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_ADD, state);
    m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_IMPORT_PLAINTEXT, state);
    m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_IMPORT_XML, state);
    m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_MERGE, state);
  }
#endif
  UpdateStatusBar();
}

void DboxMain::OnAbout()
{
  CAboutDlg about;
  about.DoModal();
}

void DboxMain::OnPasswordSafeWebsite()
{
  HINSTANCE stat = ::ShellExecute(NULL, NULL, _T("http://passwordsafe.sourceforge.net/"),
    NULL, _T("."), SW_SHOWNORMAL);
  if (int(stat) <= 32) {
#ifdef _DEBUG
    AfxMessageBox(_T("oops"));
#endif
  }
}

void DboxMain::OnU3ShopWebsite()
{
#ifdef DEMO
  ::ShellExecute(NULL, NULL,
    _T("http://software.u3.com/Product_Details.aspx?productId=294&Selection=7"),
    NULL, _T("."), SW_SHOWNORMAL);
  /*
  * TBD - point to language-specific sites:
  * FRENCH: Same as above plus "&Lang=f"
  * ITALIAN: "&Lang=it"
  * GERMAN:  "&Lang=de"
  * SPANISH: "&Lang=es "
  */
#endif
}

int DboxMain::GetAndCheckPassword(const StringX &filename,
                                  StringX &passkey,
                                  int index,
                                  bool bReadOnly,
                                  bool bForceReadOnly,
                                  PWScore *pcore,
                                  int adv_type)
{
  // index:
  //  GCP_FIRST      (0) first
  //  GCP_NORMAL     (1) OK, CANCEL & HELP buttons
  //  GCP_UNMINIMIZE (2) OK, CANCEL & HELP buttons
  //  GCP_WITHEXIT   (3) OK, CANCEL, EXIT & HELP buttons
  //  GCP_ADVANCED   (4) OK, CANCEL, HELP & ADVANCED buttons

  // for adv_type values, see enum in AdvancedDlg.h

  // Called for an existing database. Prompt user
  // for password, verify against file. Lock file to
  // prevent multiple r/w access.
  int retval;
  bool bFileIsReadOnly = false;

  if (pcore == 0) pcore = &m_core;

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

  static CPasskeyEntry *dbox_pkentry = NULL;
  INT_PTR rc = 0;
  if (dbox_pkentry == NULL) {
    dbox_pkentry = new CPasskeyEntry(this, filename.c_str(),
                                     index, bReadOnly || bFileIsReadOnly,
                                     bFileIsReadOnly || bForceReadOnly,
                                     adv_type);

    int nMajor(0), nMinor(0), nBuild(0);
    DWORD dwMajorMinor = app.GetFileVersionMajorMinor();
    DWORD dwBuildRevision = app.GetFileVersionBuildRevision();

    if (dwMajorMinor > 0) {
      nMajor = HIWORD(dwMajorMinor);
      nMinor = LOWORD(dwMajorMinor);
      nBuild = HIWORD(dwBuildRevision);
    }
    if (nBuild == 0)
      dbox_pkentry->m_appversion.Format(_T("Version %d.%02d%s"),
                                        nMajor, nMinor, SPECIAL_BUILD);
    else
      dbox_pkentry->m_appversion.Format(_T("Version %d.%02d.%02d%s"),
                                        nMajor, nMinor, nBuild, SPECIAL_BUILD);

    app.DisableAccelerator();
    rc = dbox_pkentry->DoModal();
    app.EnableAccelerator();

    if (rc == IDOK && index == GCP_ADVANCED) {
      m_bAdvanced = dbox_pkentry->m_bAdvanced;
      m_bsFields = dbox_pkentry->m_bsFields;
      m_subgroup_set = dbox_pkentry->m_subgroup_set;
      m_treatwhitespaceasempty = dbox_pkentry->m_treatwhitespaceasempty;
      if (m_subgroup_set == BST_CHECKED) {
        m_subgroup_name = dbox_pkentry->m_subgroup_name;
        m_subgroup_object = dbox_pkentry->m_subgroup_object;
        m_subgroup_function = dbox_pkentry->m_subgroup_function;
      }
    }
  } else { // already present - bring to front
    dbox_pkentry->BringWindowToTop(); // can happen with systray lock
    return PWScore::USER_CANCEL; // multi-thread,
    // original thread will continue processing
  }

  if (rc == IDOK) {
    DBGMSG("PasskeyEntry returns IDOK\n");
    const StringX curFile = dbox_pkentry->GetFileName().GetString();
    pcore->SetCurFile(curFile);
    stringT locker(_T("")); // null init is important here
    passkey = LPCTSTR(dbox_pkentry->GetPasskey());
    // This dialog's setting of read-only overrides file dialog
    bool bIsReadOnly = dbox_pkentry->IsReadOnly();
    pcore->SetReadOnly(bIsReadOnly);
    // Set read-only mode if user explicitly requested it OR
    // we could not create a lock file.
    switch (index) {
      case GCP_FIRST: // if first, then m_IsReadOnly is set in Open
        pcore->SetReadOnly(bIsReadOnly || !pcore->LockFile(curFile.c_str(), locker));
        break;
      case GCP_NORMAL:
      case GCP_ADVANCED:
        if (!bIsReadOnly) // !first, lock if !bIsReadOnly
          pcore->SetReadOnly(!pcore->LockFile(curFile.c_str(), locker));
        else
          pcore->SetReadOnly(bIsReadOnly);
        break;
      case GCP_UNMINIMIZE:
      case GCP_WITHEXIT:
      default:
        // user can't change R-O status
        break;
    }
    UpdateToolBar(bIsReadOnly);
    // locker won't be null IFF tried to lock and failed, in which case
    // it shows the current file locker
    if (!locker.empty()) {
      CString cs_user_and_host, cs_PID;
      cs_user_and_host = (CString)locker.c_str();
      int i_pid = cs_user_and_host.ReverseFind(_T(':'));
      if (i_pid > -1) {
        // If PID present then it is ":%08d" = 9 chars in length
        ASSERT((cs_user_and_host.GetLength() - i_pid) == 9);
        cs_PID.Format(IDS_PROCESSID, cs_user_and_host.Right(8));
        cs_user_and_host = cs_user_and_host.Left(i_pid);
      } else
        cs_PID = _T("");

      const CString cs_title(MAKEINTRESOURCE(IDS_FILEINUSE));
      CString cs_msg;
      CGeneralMsgBox gmb;
      gmb.SetTitle(cs_title);
      gmb.SetStandardIcon(MB_ICONQUESTION);
#ifdef PWS_STRICT_LOCKING // define if you don't want to allow user override
      cs_msg.Format(IDS_STRICT_LOCKED, curFile.c_str(),
                    cs_user_and_host, cs_PID);
      gmb.SetMsg(cs_msg);
      gmb.AddButton(1, IDS_READONLY);
      gmb.AddButton(3, IDS_EXIT, TRUE, TRUE);
#else
      cs_msg.Format(IDS_LOCKED, curFile.c_str(), cs_user_and_host, cs_PID);
      gmb.SetMsg(cs_msg);
      gmb.AddButton(1, IDS_READONLY);
      gmb.AddButton(2, IDS_READWRITE);
      gmb.AddButton(3, IDS_EXIT, TRUE, TRUE);
#endif
      INT_PTR user_choice = gmb.DoModal();
      switch (user_choice) {
        case 1:
          pcore->SetReadOnly(true);
          UpdateToolBar(true);
          retval = PWScore::SUCCESS;
          break;
        case 2:
          pcore->SetReadOnly(false); // Caveat Emptor!
          UpdateToolBar(false);
          retval = PWScore::SUCCESS;
          break;
        case 3:
          retval = PWScore::USER_CANCEL;
          break;
        default:
          ASSERT(false);
          retval = PWScore::USER_CANCEL;
      }
    } else { // locker.IsEmpty() means no lock needed or lock was successful
      if (dbox_pkentry->GetStatus() == TAR_NEW) {
        // Save new file
        pcore->NewFile(dbox_pkentry->GetPasskey());
        rc = pcore->WriteCurFile();

        if (rc == PWScore::CANT_OPEN_FILE) {
          CString cs_temp, cs_title(MAKEINTRESOURCE(IDS_FILEWRITEERROR));
          cs_temp.Format(IDS_CANTOPENWRITING, pcore->GetCurFile().c_str());
          MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
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
    int cancelreturn = dbox_pkentry->GetStatus();
    switch (cancelreturn) {
      case TAR_OPEN:
        ASSERT(0); // now handled entirely in CPasskeyEntry
      case TAR_CANCEL:
      case TAR_NEW:
        retval = PWScore::USER_CANCEL;
        break;
      case TAR_EXIT:
        retval = PWScore::USER_EXIT;
        break;
      default:
        DBGMSG("Default to WRONG_PASSWORD\n");
        retval = PWScore::WRONG_PASSWORD;  //Just a normal cancel
        break;
    }
  }
  delete dbox_pkentry;
  dbox_pkentry = NULL;
  return retval;
}

BOOL
DboxMain::OnToolTipText(UINT,
                        NMHDR* pNMHDR,
                        LRESULT* pResult)
                        // This code is copied from the DLGCBR32 example that comes with MFC
                        // Updated by MS on 25/09/2005
{
#if !defined(POCKET_PC)
  ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

  // allow top level routing frame to handle the message
  if (GetRoutingFrame() != NULL)
    return FALSE;

  // need to handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
  TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
  TCHAR tc_FullText[4096];  // Maxsize of a string in a resource file
  CString cs_TipText;
  UINT nID = pNMHDR->idFrom;
  if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
      pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {
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

  if (cs_TipText.GetLength() == 0)
    return TRUE;  // message handled

  // Assume ToolTip is greater than 80 characters in ALL cases and so use
  // the pointer approach.
  // Otherwise comment out the definition of LONG_TOOLTIPS below

#define LONG_TOOLTIPS

#ifdef LONG_TOOLTIPS
#ifndef _UNICODE
  if (pNMHDR->code == TTN_NEEDTEXTA) {
    delete m_pchTip;

    m_pchTip = new char[cs_TipText.GetLength() + 1];
    lstrcpyn(m_pchTip, cs_TipText, cs_TipText.GetLength()+1);
    pTTTA->lpszText = (LPSTR)m_pchTip;
  } else {
    delete m_pwchTip;

    m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
#if _MSC_VER >= 1400
    size_t numconverted;
    mbstowcs_s(&numconverted, m_pwchTip, cs_TipText.GetLength() + 1, cs_TipText, 
               cs_TipText.GetLength() + 1);
#else
    mbstowcs(m_pwchTip, cs_TipText, cs_TipText.GetLength() + 1);
#endif
    pTTTW->lpszText = m_pwchTip;
  }
#else
  if (pNMHDR->code == TTN_NEEDTEXTA) {
    delete m_pchTip;

    m_pchTip = new char[cs_TipText.GetLength() + 1];
#if (_MSC_VER >= 1400)
    size_t num_converted;
    wcstombs_s(&num_converted, m_pchTip, cs_TipText.GetLength() + 1, cs_TipText,
               cs_TipText.GetLength() + 1);
#else
    wcstombs(m_pchTip, cs_TipText, cs_TipText.GetLength() + 1);
#endif
    pTTTA->lpszText = (LPSTR)m_pchTip;
  } else {
    delete m_pwchTip;

    m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
    lstrcpyn(m_pwchTip, cs_TipText, cs_TipText.GetLength() + 1);
    pTTTW->lpszText = (LPWSTR)m_pwchTip;
  }
#endif

#else // Short Tooltips!

#ifndef _UNICODE
  if (pNMHDR->code == TTN_NEEDTEXTA)
#if _MSC_VER >= 1400
    _tcsncpy_s(pTTTA->szText, (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])),
               cs_TipText, _TRUNCATE);
#else
    _tcsncpy(pTTTA->szText, cs_TipText, (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
#endif
  else {
    int n = MultiByteToWideChar(CP_ACP, 0, cs_TipText, -1, pTTTW->szText,
                                sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0]));
    if (n > 0)
      pTTTW->szText[n - 1] = 0;
  }
#else
  if (pNMHDR->code == TTN_NEEDTEXTA) {
    int n = WideCharToMultiByte(CP_ACP, 0, cs_TipText, -1,
                                pTTTA->szText,
                                sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0]),
                                NULL, NULL);
    if (n > 0)
      pTTTA->szText[n - 1] = 0;
  } else
#if _MSC_VER >= 1400
    _tcsncpy_s(pTTTW->szText, (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])),
    cs_TipText, _TRUNCATE);
#else
    _tcsncpy(pTTTW->szText, cs_TipText, (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif
#endif // _UNICODE
#endif // LONG_TOOLTIPS

  *pResult = 0;

  // bring the tooltip window above other popup windows
  ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
#endif  // POCKET_PC
  return TRUE;    // message was handled
}

#if !defined(POCKET_PC)
void DboxMain::OnDropFiles(HDROP hDrop)
{
  //SetActiveWindow();
  SetForegroundWindow();

#if 0
  // here's what we really want - sorta
  HDROP m_hDropInfo = hDropInfo;
  CString Filename;

  if (m_hDropInfo) {
    int iFiles = DragQueryFile(m_hDropInfo, (UINT)-1, NULL, 0);
    for (int i=0; i<ifiles; i++) {
      char* pFilename = Filename.GetBuffer(_MAX_PATH);
      // do whatever...
    }   // for each files...
  }       // if DropInfo

  DragFinish(m_hDropInfo);

  m_hDropInfo = 0;
#endif

  DragFinish(hDrop);
}
#endif

void DboxMain::UpdateAlwaysOnTop()
{
#if !defined(POCKET_PC)
  CMenu* sysMenu = GetSystemMenu( FALSE );

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
    SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    sysMenu->CheckMenuItem( ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_CHECKED );
  } else {
    SetWindowPos( &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    sysMenu->CheckMenuItem( ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_UNCHECKED );
  }
#endif
}

void DboxMain::OnSysCommand( UINT nID, LPARAM lParam )
{
#if !defined(POCKET_PC)
  if (ID_SYSMENU_ALWAYSONTOP == nID) {
    PWSprefs *prefs = PWSprefs::GetInstance();
    bool oldAlwaysOnTop = prefs->GetPref(PWSprefs::AlwaysOnTop);
    prefs->SetPref(PWSprefs::AlwaysOnTop, !oldAlwaysOnTop);
    UpdateAlwaysOnTop();
    return;
  }

  if ((nID & 0xFFF0) == SC_RESTORE) {
    UnMinimize(true);
    if (!m_passphraseOK)  // password bad or cancel pressed
      return;
  }

  if ((nID & 0xFFF0) == SC_MINIMIZE || (nID & 0xFFF0) == SC_CLOSE) {
    // Save expand/collapse status of groups
    m_displaystatus = GetGroupDisplayStatus();
  }

  CDialog::OnSysCommand(nID, lParam);

#endif
}

void DboxMain::ConfigureSystemMenu()
{
#if defined(POCKET_PC)
  m_wndCommandBar = (CCeCommandBar*) m_pWndEmptyCB;
  m_wndMenu = m_wndCommandBar->InsertMenuBar( IDR_MAINMENU );

  ASSERT( m_wndMenu != NULL );
#else
  CMenu* sysMenu = GetSystemMenu( FALSE );
  const CString str(MAKEINTRESOURCE(IDS_ALWAYSONTOP));

  sysMenu->InsertMenu( 5, MF_BYPOSITION | MF_STRING, ID_SYSMENU_ALWAYSONTOP, (LPCTSTR)str );
#endif
}

void DboxMain::OnUpdateMRU(CCmdUI* pCmdUI)
{
  if (app.GetMRU() == NULL)
    return;

  if (!app.m_mruonfilemenu) {
    if (pCmdUI->m_nIndex == 0) { // Add to popup menu
      app.GetMRU()->UpdateMenu( pCmdUI );
    } else {
      return;
    }
  } else {
    app.GetMRU()->UpdateMenu( pCmdUI );
  }
}

void DboxMain::SetUpMenuStrings(CMenu *pPopupMenu)
{
  // Can't use GetMenuItemID, as it does not understand that with the MENUEX
  // format, Popup menus can have IDs
  ASSERT(pPopupMenu);

  const int count = pPopupMenu->GetMenuItemCount();

  TCHAR tcMenuString[_MAX_PATH] = {0};
  MENUITEMINFO miinfo;
  memset(&miinfo, 0x00, sizeof(MENUITEMINFO));
  miinfo.cbSize = sizeof(MENUITEMINFO);
  miinfo.fMask = MIIM_ID | MIIM_STRING;
  miinfo.dwTypeData = tcMenuString;  

  std::bitset<10> bsMenuItems;
  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, _MAX_PATH * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pPopupMenu->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      switch(miinfo.wID) {
        case ID_MENUITEM_DELETE:              // bitset position 0
          CS_DELETEENTRY = tcMenuString;
          bsMenuItems.set(0);
          break;
        case ID_MENUITEM_RENAME:              // bitset position 1
          CS_RENAMEENTRY = tcMenuString;
          bsMenuItems.set(1);
          break;
        case ID_MENUITEM_BROWSEURL:           // bitset position 2
          CS_BROWSEURL = tcMenuString;
          bsMenuItems.set(2);
          break;
        case ID_MENUITEM_COPYURL:             // bitset position 3
          CS_COPYURL = tcMenuString;
          bsMenuItems.set(3);
          break;
        case ID_MENUITEM_GOTOBASEENTRY:       // bitset position 4
          CS_GOTOBASEENTRY = tcMenuString;
          bsMenuItems.set(4);
          break;
        case ID_MENUITEM_DUPLICATEENTRY:      // bitset position 5
          CS_DUPLICATEENTRY = tcMenuString;
          bsMenuItems.set(5);
          break;
        case ID_MENUITEM_COPYPASSWORD:        // bitset position 6
          CS_COPYPASSWORD = tcMenuString;
          bsMenuItems.set(6);
          break;
        case ID_MENUITEM_COPYUSERNAME:        // bitset position 7
          CS_COPYUSERNAME = tcMenuString;
          bsMenuItems.set(7);
          break;
        case ID_MENUITEM_COPYNOTESFLD:        // bitset position 8
          CS_COPYNOTESFLD = tcMenuString;
          bsMenuItems.set(8);
          break;
        case ID_MENUITEM_AUTOTYPE:            // bitset position 9
          CS_AUTOTYPE = tcMenuString;
          bsMenuItems.set(9);
          break;
        default:
          break;
      }
    }
    // If we have them all - leave now
    if (bsMenuItems.count() == bsMenuItems.size())
      break;
  }
}

void DboxMain::CustomiseMenu(CMenu *pPopupMenu, const UINT uiMenuID)
{
  // Original OnInitMenu code
  // All main menus are POPUPs (see PasswordSafe2.rc2)

  // Set up menu string values
  if (CS_DELETEENTRY.IsEmpty())
    SetUpMenuStrings(pPopupMenu);

  // This routine changes the text in the menu via "ModifyMenu" and
  // adds the "check" mark via CheckMenuRadioItem for view type and toolbar.
  // It now tailors the Edit menu depending on whether a Group or Entry is selected.
  // "EnableMenuItem" is handled by the UPDATE_UI routines
  const bool bTreeView = m_ctlItemTree.IsWindowVisible() == TRUE;
  const bool bItemSelected = (SelItemOk() == TRUE);

  bool bGroupSelected(false);
  if (bTreeView) {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    bGroupSelected = (hi != NULL && !m_ctlItemTree.IsLeaf(hi));
  }

  // If Edit menu selected (contains 'Delete' menu item)
  if (uiMenuID == ID_EDITMENU) {
    if (bGroupSelected) {
      // Group selected
      pPopupMenu->ModifyMenu(ID_MENUITEM_DELETE, MF_BYCOMMAND,
                             ID_MENUITEM_DELETE, CS_DELETEGROUP);
      pPopupMenu->ModifyMenu(ID_MENUITEM_RENAME, MF_BYCOMMAND,
                             ID_MENUITEM_RENAME, CS_RENAMEGROUP);
      pPopupMenu->ModifyMenu(ID_MENUITEM_EDIT, MF_BYCOMMAND,
                             ID_MENUITEM_GROUPENTER, CS_EXPCOLGROUP);

      if (app.FindMenuItem(pPopupMenu, ID_EDITMENU_SEPARATOR3) != -1) {
        // No entry selected - remove inappropriate menu items
        pPopupMenu->RemoveMenu(ID_MENUITEM_DUPLICATEENTRY, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_EDITMENU_SEPARATOR3, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_COPYPASSWORD, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_COPYUSERNAME, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_COPYNOTESFLD, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_COPYURL, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_BROWSEURL, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_AUTOTYPE, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND);
        pPopupMenu->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
      }
    } else {
      // Entry selected
      pPopupMenu->ModifyMenu(ID_MENUITEM_DELETE, MF_BYCOMMAND,
                             ID_MENUITEM_DELETE, CS_DELETEENTRY);
      pPopupMenu->ModifyMenu(ID_MENUITEM_RENAME, MF_BYCOMMAND,
                             ID_MENUITEM_RENAME, CS_RENAMEENTRY);
      if (m_core.IsReadOnly()) {
        // Do both - not sure what was last selected!
        pPopupMenu->ModifyMenu(ID_MENUITEM_EDIT, MF_BYCOMMAND,
                               ID_MENUITEM_EDIT, CS_VIEWENTRY);
        pPopupMenu->ModifyMenu(ID_MENUITEM_GROUPENTER, MF_BYCOMMAND,
                               ID_MENUITEM_EDIT, CS_VIEWENTRY);
      } else {
        // Do both - not sure what was last selected!
        pPopupMenu->ModifyMenu(ID_MENUITEM_EDIT, MF_BYCOMMAND,
                               ID_MENUITEM_EDIT, CS_EDITENTRY);
        pPopupMenu->ModifyMenu(ID_MENUITEM_GROUPENTER, MF_BYCOMMAND,
                               ID_MENUITEM_EDIT, CS_EDITENTRY);
      }
      if (app.FindMenuItem(pPopupMenu, ID_EDITMENU_SEPARATOR3) == -1) {
        // Add missing menu items (deleted when menu shown and group selected)
        pPopupMenu->InsertMenu(ID_EDITMENU_SEPARATOR1, MF_BYCOMMAND | MF_ENABLED | MF_STRING,
                               ID_MENUITEM_DUPLICATEENTRY, CS_DUPLICATEENTRY);
        pPopupMenu->AppendMenu(MF_SEPARATOR, ID_EDITMENU_SEPARATOR3, (LPCTSTR)NULL);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_COPYPASSWORD, CS_COPYPASSWORD);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_COPYUSERNAME, CS_COPYUSERNAME);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_COPYNOTESFLD, CS_COPYNOTESFLD);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_COPYURL, CS_COPYURL);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_BROWSEURL, CS_BROWSEURL);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_AUTOTYPE, CS_AUTOTYPE);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_CREATESHORTCUT, CS_CREATESHORTCUT);
      }
    }
  
    if (bItemSelected) {
      CItemData *ci = getSelectedItem();
      ASSERT(ci != NULL);
  
      CItemData::EntryType etype = ci->GetEntryType();
      if (app.FindMenuItem(pPopupMenu, ID_MENUITEM_CREATESHORTCUT) == -1 &&
          app.FindMenuItem(pPopupMenu, ID_MENUITEM_GOTOBASEENTRY) == -1 &&
          etype != CItemData::ET_ALIASBASE) {
        // Add missing menu items (deleted when menu shown and group selected)
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_CREATESHORTCUT, CS_CREATESHORTCUT);
      }
      switch (etype) {
        case CItemData::ET_NORMAL:
        case CItemData::ET_SHORTCUTBASE:
          // Allow creation of a shortcut
          pPopupMenu->ModifyMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND,
                                 ID_MENUITEM_CREATESHORTCUT, CS_CREATESHORTCUT);
          break;
        case CItemData::ET_ALIASBASE:
          // Can't have a shortcut to an AliasBase entry
          pPopupMenu->RemoveMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND);
          pPopupMenu->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
          break;
        case CItemData::ET_ALIAS:
        case CItemData::ET_SHORTCUT:
          // Allow going to the appropriate base entry
          pPopupMenu->ModifyMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND,
                                 ID_MENUITEM_GOTOBASEENTRY, CS_GOTOBASEENTRY);
          break;
        default:
          ASSERT(0);
      }
      
      if (ci->IsShortcut()) {
        // This is an shortcut
        uuid_array_t entry_uuid, base_uuid;
        ci->GetUUID(entry_uuid);
        m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);
  
        ItemListIter iter = m_core.Find(base_uuid);
        if (iter != End()) {
          ci = &iter->second;
        }
      }
  
      if (!ci->IsURLEmpty()) {
        const bool bIsEmail = ci->IsURLEmail();
        if (bIsEmail) {
          pPopupMenu->ModifyMenu(ID_MENUITEM_BROWSEURL, MF_BYCOMMAND,
                                 ID_MENUITEM_SENDEMAIL, CS_SENDEMAIL);
          pPopupMenu->ModifyMenu(ID_MENUITEM_COPYURL, MF_BYCOMMAND,
                                 ID_MENUITEM_COPYEMAIL, CS_COPYEMAIL);
        } else {
          pPopupMenu->ModifyMenu(ID_MENUITEM_SENDEMAIL, MF_BYCOMMAND,
                                 ID_MENUITEM_BROWSEURL, CS_BROWSEURL);
          pPopupMenu->ModifyMenu(ID_MENUITEM_COPYEMAIL, MF_BYCOMMAND,
                                 ID_MENUITEM_COPYURL, CS_COPYURL);
        }
        UpdateBrowseURLSendEmailButton(bIsEmail);
      } else {
        pPopupMenu->ModifyMenu(ID_MENUITEM_SENDEMAIL, MF_BYCOMMAND,
                               ID_MENUITEM_BROWSEURL, CS_BROWSEURL);
        pPopupMenu->ModifyMenu(ID_MENUITEM_COPYEMAIL, MF_BYCOMMAND,
                               ID_MENUITEM_COPYURL, CS_COPYURL);
      }
    }
  }  // Edit menu

  // If View menu selected (contains 'Flattened &List' menu item)
  if (uiMenuID == ID_VIEWMENU) {
    pPopupMenu->CheckMenuRadioItem(ID_MENUITEM_LIST_VIEW, ID_MENUITEM_TREE_VIEW,
                                   bTreeView ? ID_MENUITEM_TREE_VIEW : ID_MENUITEM_LIST_VIEW,
                                   MF_BYCOMMAND);

    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_TOOLBAR, MF_BYCOMMAND |
                              m_MainToolBar.IsWindowVisible() ? MF_CHECKED : MF_UNCHECKED);

    bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_DRAGBAR, MF_BYCOMMAND |
                              bDragBarState ? MF_CHECKED : MF_UNCHECKED);
  
    pPopupMenu->ModifyMenu(ID_MENUITEM_APPLYFILTER, MF_BYCOMMAND |
                           m_bFilterActive ? MF_CHECKED : MF_UNCHECKED,
                           ID_MENUITEM_APPLYFILTER,
                           m_bFilterActive ? CS_CLEARFILTERS : CS_SETFILTERS);

  // JHF m_toolbarMode is not for WinCE (as in .h)
#if !defined(POCKET_PC)
    pPopupMenu->CheckMenuRadioItem(ID_MENUITEM_NEW_TOOLBAR, ID_MENUITEM_OLD_TOOLBAR,
                                   m_toolbarMode, MF_BYCOMMAND);
#endif
  }  // View menu
}

// helps with MRU by allowing ON_UPDATE_COMMAND_UI
void DboxMain::OnInitMenuPopup(CMenu* pPopupMenu, UINT, BOOL)
{
  // Original OnInitMenu code
  // All main menus are POPUPs (see PasswordSafe2.rc2)

  // Don't do the old OnInitMenu processing if the right-click context menu
  // (IDR_POPEDITMENU or IDR_POPEDITGROUP) is being processed. Only for the Main Menu
  // (ID_EDITMENU).
  ASSERT(pPopupMenu != NULL);

  MENUINFO minfo;
  memset(&minfo, 0x00, sizeof(minfo));
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;
  pPopupMenu->GetMenuInfo(&minfo);
  if (minfo.dwMenuData == ID_EDITMENU || minfo.dwMenuData == ID_VIEWMENU)
    CustomiseMenu(pPopupMenu, minfo.dwMenuData);

  // http://www4.ncsu.edu:8030/~jgbishop/codetips/dialog/updatecommandui_menu.html
  // This code comes from the MFC Documentation, and is adapted from 
  // CFrameWnd::OnInitMenuPopup() in WinFrm.cpp.
  CCmdUI state; // Check the enabled state of various menu items
  state.m_pMenu = pPopupMenu;
  ASSERT(state.m_pOther == NULL);
  ASSERT(state.m_pParentMenu == NULL);

  // Is the menu in question a popup in the top-level menu? If so, set m_pOther
  // to this menu. Note that m_pParentMenu == NULL indicates that the menu is a
  // secondary popup.
  CMenu *hParentMenu;
  if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu) {
    state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
  } else
  if ((hParentMenu = this->GetMenu()) != NULL) {
    CWnd* pParent = this;
    // Child windows don't have menus--need to go to the top!
    if (pParent != NULL && (hParentMenu = pParent->GetMenu()) != NULL) {
      int nIndexMax = hParentMenu->GetMenuItemCount();
      for (int nIndex = 0; nIndex < nIndexMax; nIndex++) {
        CMenu *submenu = hParentMenu->GetSubMenu(nIndex);
        if (submenu != NULL && submenu->m_hMenu == pPopupMenu->m_hMenu) {
          // When popup is found, m_pParentMenu is containing menu.
          state.m_pParentMenu = CMenu::FromHandle(hParentMenu->GetSafeHmenu());
          break;
        }
      }
    }
  }

  state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
  for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++) {
    state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
    if (state.m_nID == 0)
      continue; // Menu separator or invalid cmd - ignore it.
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pMenu != NULL);
    if (state.m_nID == (UINT)-1) {
      // Possibly a popup menu, route to first item of that popup.
      state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
      if (state.m_pSubMenu == NULL ||
          (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
          state.m_nID == (UINT)-1) {
        continue; // First item of popup can't be routed to.
      }
      state.DoUpdate(this, TRUE); // Popups are never auto disabled.
    } else {
      // Normal menu item.
      // Auto enable/disable if frame window has m_bAutoMenuEnable
      // set and command is _not_ a system command.
      state.m_pSubMenu = NULL;
      state.DoUpdate(this, FALSE);
    }

    // Adjust for menu deletions and additions.
    UINT nCount = pPopupMenu->GetMenuItemCount();
    if (nCount < state.m_nIndexMax) {
      state.m_nIndex -= (state.m_nIndexMax - nCount);
      while(state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID) {
        state.m_nIndex++;
      }
    }
    state.m_nIndexMax = nCount;
  }

  // Ignore SystemTray popup menu
  if (!m_bImageInLV || minfo.dwMenuData != IDR_POPTRAY)
    return;

  // System Tray Popup menu processing only
  minfo.fMask = MIM_STYLE;
  minfo.dwStyle = MNS_CHECKORBMP | MNS_AUTODISMISS;
  pPopupMenu->SetMenuInfo(&minfo);

  MENUITEMINFO miinfo;
  memset(&miinfo, 0x00, sizeof(miinfo));
  miinfo.cbSize = sizeof(miinfo);
  CRUEItemData *pmd;

  for (UINT pos = 0; pos < pPopupMenu->GetMenuItemCount(); pos++) {
    miinfo.fMask = MIIM_FTYPE | MIIM_DATA;
    pPopupMenu->GetMenuItemInfo(pos, &miinfo, TRUE);
    pmd = (CRUEItemData *)miinfo.dwItemData;

    if (pmd && pmd->IsRUEID() && !(miinfo.fType & MFT_OWNERDRAW) &&
        pmd->nImage >= 0) {
      miinfo.fMask = MIIM_FTYPE | MIIM_BITMAP;
      miinfo.hbmpItem = HBMMENU_CALLBACK;
      miinfo.fType = MFT_STRING;

      pPopupMenu->SetMenuItemInfo(pos, &miinfo, TRUE);
    }
  }
}

#if defined(POCKET_PC)
void DboxMain::OnShowPassword()
{
  if (SelItemOk() == TRUE) {
    CItemData item;
    StringX password;
    StringX name;
    StringX title;
    StringX username;
    CShowPasswordDlg pwDlg( this );

    item = m_pwlist.GetAt(Find(getSelectedItem()));

    item.GetPassword(password);
    item.GetName(name);

    SplitName(name, title, username);

    pwDlg.SetTitle(title);
    pwDlg.SetPassword(password);
    pwDlg.DoModal();
  }
}
#endif

LRESULT DboxMain::OnTrayNotification(WPARAM , LPARAM)
{
#if 0
  return m_TrayIcon.OnTrayNotification(wParam, lParam);
#else
  return 0L;
#endif
}

void DboxMain::OnMinimize()
{
  // Called when the System Tray Minimize menu option is used
  if (m_bStartHiddenAndMinimized)
    m_bStartHiddenAndMinimized = false;

  // Save expand/collapse status of groups
  m_displaystatus = GetGroupDisplayStatus();

  ShowWindow(SW_MINIMIZE);
}

void DboxMain::OnUnMinimize()
{
  // Called when the System Tray Restore menu option is used
  UnMinimize(true);
}

void DboxMain::UnMinimize(bool update_windows)
{
  m_passphraseOK = false;
  if (!m_bOpen) {
    // first they may be nothing to do!
    if (update_windows) {
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
          UpdateSystemTray(UNLOCKED);
        }
        return;
      } // m_IsStartSilent
      ShowWindow(SW_RESTORE);
    } // update_windows
    UpdateSystemTray(CLOSED);
    return;
  }

  // Case 1 - data available but is currently locked
  if (!m_needsreading &&
      (app.GetSystemTrayState() == ThisMfcApp::LOCKED) &&
      (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray))) {

    StringX passkey;
    int rc;
    rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, GCP_UNMINIMIZE);  // OK, CANCEL, HELP
    if (rc != PWScore::SUCCESS)
      return;  // don't even think of restoring window!

    app.SetSystemTrayState(ThisMfcApp::UNLOCKED);
    m_passphraseOK = true;
    if (update_windows) {
      RefreshViews();
      ShowWindow(SW_RESTORE);
    }
    return;
  }

  // Case 2 - data unavailable
  if (m_needsreading && m_windowok) {
    StringX passkey;
    int rc, rc2;
    const bool useSysTray = PWSprefs::GetInstance()->
                            GetPref(PWSprefs::UseSystemTray);

    rc = PWScore::USER_CANCEL;
    if (m_bOpen)
      rc = GetAndCheckPassword(m_core.GetCurFile(), passkey,
                               useSysTray ? GCP_UNMINIMIZE : GCP_WITHEXIT,
                               m_core.IsReadOnly());
    CString cs_temp, cs_title;
    switch (rc) {
      case PWScore::SUCCESS:
        rc2 = m_core.ReadCurFile(passkey);
#if !defined(POCKET_PC)
        m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") +
                                           m_core.GetCurFile()).c_str();
#endif
        break;
      case PWScore::CANT_OPEN_FILE:
        cs_temp.Format(IDS_CANTOPEN, m_core.GetCurFile().c_str());
        cs_title.LoadString(IDS_FILEOPEN);
        MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
      case TAR_NEW:
        rc2 = New();
        break;
      case TAR_OPEN:
        rc2 = Open();
        break;
      case PWScore::WRONG_PASSWORD:
        rc2 = PWScore::NOT_SUCCESS;
        break;
      case PWScore::USER_CANCEL:
        rc2 = PWScore::NOT_SUCCESS;
        break;
      case PWScore::USER_EXIT:
        m_core.UnlockFile(m_core.GetCurFile().c_str());
        PostQuitMessage(0);
        return;
      default:
        rc2 = PWScore::NOT_SUCCESS;
        break;
    }

    if (rc2 == PWScore::SUCCESS) {
      UpdateSystemTray(UNLOCKED);
      startLockCheckTimer();
      m_passphraseOK = true;
      m_needsreading = false;
      if (update_windows) {
        ShowWindow(SW_RESTORE);
        SetGroupDisplayStatus(m_displaystatus);
        BringWindowToTop();
      }
    } else {
      ShowWindow(useSysTray ? SW_HIDE : SW_MINIMIZE);
    }
    return;
  }
  if (update_windows) {
    ShowWindow(SW_RESTORE);
    SetGroupDisplayStatus(m_displaystatus);
    BringWindowToTop();
  }
}

void
DboxMain::startLockCheckTimer(){
  const UINT INTERVAL = 5000; // every 5 seconds should suffice

  if (PWSprefs::GetInstance()->
        GetPref(PWSprefs::LockOnWindowLock) == TRUE) {
    SetTimer(TIMER_CHECKLOCK, INTERVAL, NULL);
  }
}

BOOL DboxMain::PreTranslateMessage(MSG* pMsg)
{
  // Do NOT pass the ESC along if preference EscExits is false.
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
    if (!PWSprefs::GetInstance()->GetPref(PWSprefs::EscExits)) {
      if (m_FindToolBar.IsVisible())
        OnHideFindToolBar();
      return TRUE;
    }
  }
  return CDialog::PreTranslateMessage(pMsg);
}

void DboxMain::ResetIdleLockCounter()
{
  m_IdleLockCountDown = PWSprefs::GetInstance()->
                          GetPref(PWSprefs::IdleTimeout);
}

bool DboxMain::DecrementAndTestIdleLockCounter()
{
  if (m_IdleLockCountDown > 0)
    return (--m_IdleLockCountDown == 0);
  else
    return false; // so we return true only once if idle
}

LRESULT DboxMain::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  static DWORD last_t = 0;
  DWORD t = GetTickCount();
  if (t != last_t) {
    PWSrand::GetInstance()->AddEntropy((unsigned char *)&t, sizeof(t));
    last_t = t;
  }
  // list of all the events that signify actual user activity, as opposed
  // to Windows internal events...
  if (message == WM_KEYDOWN ||
      message == WM_COMMAND ||
      message == WM_SYSCOMMAND ||
      message == WM_MOUSEMOVE ||
      message == WM_MOVE ||
      message == WM_LBUTTONDOWN ||
      message == WM_LBUTTONDBLCLK ||
      message == WM_CONTEXTMENU ||
    // JHF undeclared identifier -> removed to get code to compile
#if !defined(POCKET_PC)
      message == WM_MENUSELECT ||
#endif
      message == WM_VSCROLL ||
      message == WM_HSCROLL)
    ResetIdleLockCounter();

  if (message == WM_SYSCOLORCHANGE ||
      message == WM_SETTINGCHANGE)
    RefreshImages();

  return CDialog::WindowProc(message, wParam, lParam);
}

void DboxMain::RefreshImages()
{
  m_MainToolBar.RefreshImages();
  m_FindToolBar.RefreshImages();
  m_menuManager.SetImageList(&m_MainToolBar);
}

void DboxMain::CheckExpiredPasswords()
{
  time_t now, exptime, XTime;
  time(&now);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn)) {
    int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
    struct tm st;
#if _MSC_VER >= 1400
    errno_t err;
    err = localtime_s(&st, &now);  // secure version
    ASSERT(err == 0);
#else
    st = *localtime(&now);
#endif
    st.tm_mday += idays;
    exptime = mktime(&st);
    if (exptime == (time_t)-1)
      exptime = now;
  } else
    exptime = now;

  ExpiredList expPWList;

  ItemListConstIter listPos;
  for (listPos = m_core.GetEntryIter();
       listPos != m_core.GetEntryEndIter();
       listPos++) {
    const CItemData &curitem = m_core.GetEntry(listPos);
    if (curitem.IsAlias())
      continue;

    curitem.GetXTime(XTime);
    if (((long)XTime != 0) && (XTime < exptime)) {
      ExpPWEntry exppwentry(curitem, now, XTime);
      expPWList.push_back(exppwentry);
    }
  }

  if (!expPWList.empty()) {
    CExpPWListDlg dlg(this, expPWList, m_core.GetCurFile().c_str());
    dlg.DoModal();
  }
}

void DboxMain::UpdateAccessTime(CItemData *ci)
{
  // Mark access time if so configured
  ASSERT(ci != NULL);

  // First add to RUE List
  uuid_array_t RUEuuid;
  ci->GetUUID(RUEuuid);
  m_RUEList.AddRUEntry(RUEuuid);

  bool bMaintainDateTimeStamps = PWSprefs::GetInstance()->
              GetPref(PWSprefs::MaintainDateTimeStamps);

  if (!m_core.IsReadOnly() && bMaintainDateTimeStamps) {
    ci->SetATime();
    SetChanged(TimeStamp);
    // Need to update view if there
    if (m_nColumnIndexByType[CItemData::ATIME] != -1) {
      // Get index of entry
      DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
      // Get value in correct format
      CString cs_atime = ci->GetATimeL().c_str();
      // Update it
      m_ctlItemList.SetItemText(di->list_index,
        m_nColumnIndexByType[CItemData::ATIME], cs_atime);
    }
  }
}

BOOL DboxMain::OnQueryEndSession()
{
  m_iSessionEndingStatus = IDOK;

  PWSprefs *prefs = PWSprefs::GetInstance();
  if (!m_core.GetCurFile().empty())
    prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());
  // Save Application related preferences
  prefs->SaveApplicationPreferences();

  if (m_core.IsReadOnly())
    return TRUE;

  bool autoSave = true; // false if user saved or decided not to 
  BOOL retval = TRUE;

  if (m_core.IsChanged()) {
    CString cs_msg;
    cs_msg.Format(IDS_SAVECHANGES, m_core.GetCurFile().c_str());
    m_iSessionEndingStatus = AfxMessageBox(cs_msg,
                             (MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3));
    switch (m_iSessionEndingStatus) {
      case IDCANCEL:
        // Cancel shutdown\restart\logoff
        return FALSE;
      case IDYES:
        // Save the changes and say OK to shutdown\restart\logoff
        Save();
        autoSave = false;
        retval = TRUE;
        break;
      case IDNO:
        // Don't save the changes but say OK to shutdown\restart\logoff
        autoSave = false;
        retval = TRUE;
        break;
    }
  } // database was changed

  if (autoSave) {
    //Store current filename for next time
    if ((m_bTSUpdated || m_core.WasDisplayStatusChanged())
      && m_core.GetNumEntries() > 0) {
        Save();
        return TRUE;
    }
  }

  return retval;
}

void DboxMain::OnEndSession(BOOL bEnding)
{
  if (bEnding == TRUE) {
    switch (m_iSessionEndingStatus) {
      case IDOK:
        // Either not changed, R/O or already saved timestamps
      case IDCANCEL:
        // Shouldn't happen as the user said NO to shutdown\restart\logoff
      case IDNO:
        // User said NO to saving - so don't!
        break;
      case IDYES:
        // User said YES they wanted to save - so do!
        OnOK();
        break;
      default:
        ASSERT(0);
    }
  } else {
    // Reset status since the EndSession was cancelled
    m_iSessionEndingStatus = IDIGNORE;
  }
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

    if (m_bOpen) {
      dc.DrawText(m_lastclipboardaction, &rectPane, DT_CALCRECT);
      m_statusBar.GetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_CLIPBOARDACTION, m_lastclipboardaction);

      s = m_core.IsChanged() ? _T("*") : _T(" ");
      s += m_core.HaveDBPrefsChanged() ? _T("°") : _T(" ");
      dc.DrawText(s, &rectPane, DT_CALCRECT);
      m_statusBar.GetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_MODIFIED, s);

      s = m_core.IsReadOnly() ? _T("R-O") : _T("R/W");
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
    } else {
      s.LoadString(IDS_STATCOMPANY);
      m_statusBar.SetPaneText(CPWStatusBar::SB_DBLCLICK, s);

      dc.DrawText(_T(" "), &rectPane, DT_CALCRECT);

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_CLIPBOARDACTION, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_CLIPBOARDACTION, _T(" "));

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_MODIFIED, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_MODIFIED, _T(" "));

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_READONLY, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_READONLY, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_READONLY, _T(" "));

      m_statusBar.GetPaneInfo(CPWStatusBar::SB_NUM_ENT, uiID, uiStyle, iWidth);
      m_statusBar.SetPaneInfo(CPWStatusBar::SB_NUM_ENT, uiID, uiStyle, rectPane.Width());
      m_statusBar.SetPaneText(CPWStatusBar::SB_NUM_ENT, _T(" "));
    }
  }

  /*
  This doesn't exactly belong here, but it makes sure that the
  title is fresh...
  */
#if !defined(POCKET_PC)
  SetWindowText(LPCTSTR(m_titlebar));
#endif
}

void DboxMain::SetDCAText()
{
  const int dca = int(PWSprefs::GetInstance()->
                       GetPref(PWSprefs::DoubleClickAction));
  int i_dca_text;
  switch (dca) {
    case PWSprefs::DoubleClickAutoType: i_dca_text = IDS_STATAUTOTYPE; break;
    case PWSprefs::DoubleClickBrowse: i_dca_text = IDS_STATBROWSE; break;
    case PWSprefs::DoubleClickCopyNotes: i_dca_text = IDS_STATCOPYNOTES; break;
    case PWSprefs::DoubleClickCopyPassword: i_dca_text = IDS_STATCOPYPASSWORD; break;
    case PWSprefs::DoubleClickCopyUsername: i_dca_text = IDS_STATCOPYUSERNAME; break;
    case PWSprefs::DoubleClickViewEdit: i_dca_text = IDS_STATVIEWEDIT; break;
	case PWSprefs::DoubleClickCopyPasswordMinimize: i_dca_text = IDS_STATCOPYPASSWORDMIN; break;
    default: i_dca_text = IDS_STATCOMPANY;
  }
  CString s;
  s.LoadString(i_dca_text);
  m_statusBar.SetPaneText(CPWStatusBar::SB_DBLCLICK, s);
}

// Returns a list of entries as they appear in tree in DFS order
void DboxMain::MakeOrderedItemList(OrderedItemList &il)
{
  // Walk the Tree!
  HTREEITEM hItem = NULL;
  while ( NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem)) ) {
    if (!m_ctlItemTree.ItemHasChildren(hItem)) {
      CItemData *ci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
      if (ci != NULL) {// NULL if there's an empty group [bug #1633516]
        il.push_back(*ci);
      }
    }
  }
}

// Returns the number of children of this group
int DboxMain::CountChildren(HTREEITEM hStartItem)
{
  // Walk the Tree!
  int num = 0;
  if (hStartItem != NULL && m_ctlItemTree.ItemHasChildren(hStartItem)) {
    HTREEITEM hChildItem = m_ctlItemTree.GetChildItem(hStartItem);

    while (hChildItem != NULL) {
      if (m_ctlItemTree.ItemHasChildren(hChildItem)) {
        num += CountChildren(hChildItem);
        hChildItem = m_ctlItemTree.GetNextSiblingItem(hChildItem);
      } else {
        hChildItem = m_ctlItemTree.GetNextSiblingItem(hChildItem);
        num++;
      }
    }
  }
  return num;
}

void DboxMain::UpdateMenuAndToolBar(const bool bOpen)
{
  // Initial setup of menu items and toolbar buttons
  // First set new open/close status
  m_bOpen = bOpen;

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

    memset(&tbinfo, 0x00, sizeof(tbinfo));
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
  if (OnQueryEndSession()) {
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
          iEnable = FALSE;
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

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
// Don't support importing XML or filter processing on non-Windows platforms 
// using Microsoft XML libraries
  switch (nID) {
    case ID_MENUITEM_IMPORT_XML:
    case ID_FILTERMENU:
    case ID_MENUITEM_APPLYFILTER:
    case ID_MENUITEM_EDITFILTER:
    case ID_MENUITEM_MANAGEFILTERS:
      return FALSE;
    default:
      break;
  }
#endif

  MapUICommandTableConstIter it;
  it = m_MapUICommandTable.find(nID);

  if (it == m_MapUICommandTable.end()) {
    // Don't have it - allow by default
    TRACE(_T("m_UICommandTable is out of date - i18n issue?"));
    return TRUE;
  }

  int item, iEnable;
  if (!m_bOpen)
    item = UICommandTableEntry::InClosed;
  else if (m_core.IsReadOnly())
    item = UICommandTableEntry::InOpenRO;
  else
    item = UICommandTableEntry::InOpenRW;

  if (item == UICommandTableEntry::InOpenRW && m_core.GetNumEntries() == 0)
    item = UICommandTableEntry::InEmpty;  // OpenRW + empty

  iEnable = m_UICommandTable[it->second].bOK[item] ? TRUE : FALSE;

  // All following special processing will only ever DISABLE an item
  // The previous lookup table is the only mechanism to ENABLE an item

  const bool bTreeView = m_ctlItemTree.IsWindowVisible() == TRUE;
  bool bGroupSelected = false, bAliasOrShortcut(false);
  CItemData *ci(NULL);
  if (bTreeView) {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    bGroupSelected = (hi != NULL && !m_ctlItemTree.IsLeaf(hi));
    if (hi != NULL)
      ci = (CItemData *)m_ctlItemTree.GetItemData(hi);
  } else {
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos != NULL)
      ci = (CItemData *)m_ctlItemList.GetItemData((int)pos - 1);
  }
  if (ci != NULL)
    bAliasOrShortcut = ci->GetEntryType() == CItemData::ET_ALIAS ||
                       ci->GetEntryType() == CItemData::ET_SHORTCUT;

  // Special processing!
  switch (nID) {
    // Items not allowed if a Group is selected
    case ID_MENUITEM_DUPLICATEENTRY:
    case ID_MENUITEM_COPYPASSWORD:
    case ID_MENUITEM_COPYUSERNAME:
    case ID_MENUITEM_COPYNOTESFLD:
    case ID_MENUITEM_AUTOTYPE:
    case ID_MENUITEM_EDIT:
    case ID_MENUITEM_PASSWORDSUBSET:
#if defined(POCKET_PC)
    case ID_MENUITEM_SHOWPASSWORD:
#endif
      if (bGroupSelected)
        iEnable = FALSE;
      break;
    // Not available if group selected or entry is not an alias/shortcut
    case ID_MENUITEM_GOTOBASEENTRY:
      if (bGroupSelected || !bAliasOrShortcut)
        iEnable = FALSE;
      break;
    // Not allowed if Group selected or the item selected has an empty URL
    case ID_MENUITEM_BROWSEURL:
    case ID_MENUITEM_SENDEMAIL:
    case ID_MENUITEM_COPYURL:
    case ID_MENUITEM_COPYEMAIL:
      if (bGroupSelected) {
        // Not allowed if a Group is selected
        iEnable = FALSE;
      } else {
        CItemData *ci = getSelectedItem();
        if (ci == NULL) {
          iEnable = FALSE;
        } else {
          if (ci->IsShortcut()) {
            // This is an shortcut
            uuid_array_t entry_uuid, base_uuid;
            ci->GetUUID(entry_uuid);
            m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

            ItemListIter iter = m_core.Find(base_uuid);
            if (iter != End()) {
              ci = &iter->second;
            }
          }

          if (ci->IsURLEmpty()) {
            iEnable = FALSE;
          }
        }
      }
      break;
    case ID_MENUITEM_CREATESHORTCUT:
    case ID_MENUITEM_RCREATESHORTCUT:
      if (bGroupSelected) {
        // Not allowed if a Group is selected
        iEnable = FALSE;
      } else {
        CItemData *ci = getSelectedItem();
        if (ci == NULL) {
          iEnable = FALSE;
        } else {
          // Can only define a shortcut on a normal entry or
          // one that is already a shortcut base
          if (!ci->IsNormal() && !ci->IsShortcutBase()) {
            iEnable = FALSE;
          }
        }
      }
      // Create shortcut is only within the same instance
      if (ID_MENUITEM_RCREATESHORTCUT && !m_ctlItemTree.IsDropOnMe())
        iEnable = FALSE;      
      break;
    // Items not allowed in List View
    case ID_MENUITEM_ADDGROUP:
    case ID_MENUITEM_RENAME:
    case ID_MENUITEM_EXPANDALL:
    case ID_MENUITEM_COLLAPSEALL:
      if (m_IsListView)
        iEnable = FALSE;
      break;
    // If not changed, no need to allow Save!
    case ID_MENUITEM_SAVE:
      if (!m_core.IsChanged())
        iEnable = FALSE;
      break;
    // Special processing for viewing reports, if they exist
    case ID_MENUITEM_REPORT_COMPARE:
    case ID_MENUITEM_REPORT_FIND:
    case ID_MENUITEM_REPORT_IMPORTTEXT:
    case ID_MENUITEM_REPORT_IMPORTXML:
    case ID_MENUITEM_REPORT_MERGE:
    case ID_MENUITEM_REPORT_VALIDATE:
      iEnable = OnUpdateViewReports(nID);
      break;
    // Disable choice of toolbar if at low resolution
    case ID_MENUITEM_OLD_TOOLBAR:
    case ID_MENUITEM_NEW_TOOLBAR:
    {
#if !defined(POCKET_PC)
      // JHF m_toolbarMode is not for WinCE (as in .h)
      CDC* pDC = this->GetDC();
      int NumBits = (pDC ? pDC->GetDeviceCaps(12 /*BITSPIXEL*/) : 32);
      if (NumBits < 16 && m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) {
        // Less that 16 color bits available, no choice, disable menu items
        iEnable = FALSE;
      }
      ReleaseDC(pDC);
#endif
      break;
    }
    // Disable Minimize if already minimized
    case ID_MENUITEM_MINIMIZE:
    {
      WINDOWPLACEMENT wndpl;
      GetWindowPlacement(&wndpl);
      if (wndpl.showCmd == SW_SHOWMINIMIZED)
        iEnable = FALSE;
      break;
    }
    // Disable Restore if already visible
    case ID_MENUITEM_UNMINIMIZE:
    {
      WINDOWPLACEMENT wndpl;
      GetWindowPlacement(&wndpl);
      if (wndpl.showCmd != SW_SHOWMINIMIZED)
        iEnable = FALSE;
      break;
    }
    // Set the state of the "Case Sensitivity" button
    case ID_TOOLBUTTON_FINDCASE:
    case ID_TOOLBUTTON_FINDCASE_I:
    case ID_TOOLBUTTON_FINDCASE_S:
      m_FindToolBar.GetToolBarCtrl().CheckButton(m_FindToolBar.IsFindCaseSet() ?
                           ID_TOOLBUTTON_FINDCASE_S : ID_TOOLBUTTON_FINDCASE_I, 
                           m_FindToolBar.IsFindCaseSet());
      return -1;
    case ID_MENUITEM_CLEAR_MRU:
      if (app.GetMRU()->IsMRUEmpty())
        iEnable = FALSE;
      break;
    case ID_MENUITEM_APPLYFILTER:
      if (m_currentfilter.vMfldata.size() == 0 || 
          (m_currentfilter.num_Mactive + m_currentfilter.num_Hactive + 
                                         m_currentfilter.num_Pactive) == 0)
        iEnable = FALSE;
      break;
    case ID_MENUITEM_EDITFILTER:
    case ID_MENUITEM_MANAGEFILTERS:
      break;
    default:
      break;
  }
  // Last but not least, DEMO build support:
#ifdef DEMO
  if (!m_core.IsReadOnly()) {
    bool isLimited = (m_core.GetNumEntries() >= MAXDEMO);
    if (isLimited) {
      switch (nID) {
        case ID_MENUITEM_ADD:
        case ID_MENUITEM_ADDGROUP:
        case ID_MENUITEM_DUPLICATEENTRY:
        case ID_MENUITEM_IMPORT_KEEPASS:
        case ID_MENUITEM_IMPORT_PLAINTEXT:
        case ID_MENUITEM_IMPORT_XML:
        case ID_MENUITEM_MERGE:
          iEnable = FALSE;
          break;
        default:
          break;
      }
    }
  }
#endif
  return iEnable;
}

void DboxMain::PlaceWindow(CRect *prect, UINT showCmd)
{
  WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
  HRGN hrgnWork = GetWorkAreaRegion();

  GetWindowPlacement(&wp);  // Get min/max positions - then add what we know
  wp.flags = 0;
  wp.showCmd = showCmd;
  wp.rcNormalPosition = *prect;

  if (!RectInRegion(hrgnWork, &wp.rcNormalPosition)) {
    if (GetSystemMetrics(SM_CMONITORS) > 1)
      GetMonitorRect(NULL, &wp.rcNormalPosition, FALSE);
    else
      ClipRectToMonitor(NULL, &wp.rcNormalPosition, FALSE);
  }

  SetWindowPlacement(&wp);
  ::DeleteObject(hrgnWork);
}

HRGN DboxMain::GetWorkAreaRegion()
{
  HRGN hrgn;
  hrgn = CreateRectRgn(0, 0, 0, 0);

  HDC hdc = ::GetDC(NULL);
  EnumDisplayMonitors(hdc, NULL, EnumScreens, (LPARAM)&hrgn);
  ::ReleaseDC(NULL, hdc);

  return hrgn;
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

    if (fWork)
      rc = mi.rcWork;
    else
      rc = mi.rcMonitor;
  }

  prc->left = max(rc.left, min(rc.right-w, prc->left));
  prc->top = max(rc.top, min(rc.bottom-h, prc->top));
  prc->right = prc->left + w;
  prc->bottom = prc->top + h;
}

BOOL CALLBACK DboxMain::EnumScreens(HMONITOR hMonitor, HDC /* hdc */, 
                                    LPRECT /* prc */, LPARAM lParam)
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
