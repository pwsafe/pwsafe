// file DboxMain.cpp
//
// Methods related to viewing the actual password database are in DboxView.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

// dialog boxen
#include "DboxMain.h"

#include "ClearQuestionDlg.h"
#include "FindDlg.h"
#include "PasskeyChangeDlg.h"
#include "PasskeyEntry.h"
#include "PasskeySetup.h"
#include "TryAgainDlg.h"
#include "ExportText.h"
#include "ImportDlg.h"

// widget override?
#include "SysColStatic.h"

#ifdef POCKET_PC
  #include "pocketpc/PocketPC.h"
  #include "ShowPasswordDlg.h"
#endif

#include <afxpriv.h>
#include <stdlib.h> // for qsort
#if _MSC_VER > 1200 // compile right under .Net
#include <strstream>
#define OSTRSTREAM std::ostrstream
#define ENDS std::ends
#else
#include <strstrea.h>
#define OSTRSTREAM ostrstream
#define ENDS ends
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(UNICODE)
  #define CLIPBOARD_TEXT_FORMAT	CF_UNICODETEXT
#else
  #define CLIPBOARD_TEXT_FORMAT	CF_TEXT
#endif

/*
 * This is the string to be displayed instead of the actual password, unless
 * the user chooses to see the password:
 */

const TCHAR *HIDDEN_PASSWORD = _T("**************");

#define DEFAULT_SUFFIX _T("psafe3")
#define SUFFIX_FILTERS _T("Password Safe Databases (*.psafe3; *.dat)|*.psafe3; *.dat|")

//-----------------------------------------------------------------------------
class DboxAbout
#if defined(POCKET_PC)
   : public CPwsPopupDialog
#else
   : public CDialog
#endif
{
public:
#if defined(POCKET_PC)
  typedef CPwsPopupDialog	super;
#else
  typedef CDialog			super;
#endif

  DboxAbout()
    : super(DboxAbout::IDD)
  {}

  enum { IDD = IDD_ABOUTBOX };

protected:
  virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV support
  {
    super::DoDataExchange(pDX);
  }

protected:
  DECLARE_MESSAGE_MAP()
    };

// I don't think we need this, but...
BEGIN_MESSAGE_MAP(DboxAbout, super)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
DboxMain::DboxMain(CWnd* pParent)
   : CDialog(DboxMain::IDD, pParent),
     m_bSizing( false ), m_needsreading(true), m_windowok(false),
     m_existingrestore(FALSE), m_toolbarsSetup(FALSE),
     m_bShowPasswordInEdit(false), m_bShowPasswordInList(false),
     m_bSortAscending(true), m_iSortedColumn(0),
     m_lastFindCS(FALSE), m_lastFindStr(_T("")),
     m_core(app.m_core), m_LockDisabled(false), m_IsReadOnly(false),
     m_IsStartSilent(false), m_clipboard_set(false),
     m_selectedAtMinimize(NULL)
{
  //{{AFX_DATA_INIT(DboxMain)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT

  m_hIcon = app.LoadIcon(IDI_CORNERICON);
  m_hIconSm = (HICON) ::LoadImage(app.m_hInstance, MAKEINTRESOURCE(IDI_CORNERICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

  ClearData();


  /*
   * current file and current backup file specs are stored in registry
   * Note that if m_currfile is non-empty, we will not read the registry value.
   * This will happen if a filename was given in the command line.
   */
  if (m_core.GetCurFile().IsEmpty()) {
    // If there's no stored preference, this is probably a fresh install.
    // CheckPassword will catch this and handle it correctly
    m_core.SetCurFile(PWSprefs::GetInstance()->
                      GetPref(PWSprefs::StringPrefs::CurrentFile));
  }
#if !defined(POCKET_PC)
  m_title = _T("");
  m_toolbarsSetup = FALSE;
#endif

  m_bShowPasswordInEdit = false;
  m_bShowPasswordInList = false;
  m_bSortAscending = true;
  m_iSortedColumn = 0;
  m_hFontTree = NULL;
}

DboxMain::~DboxMain()
{
  ::DeleteObject( m_hFontTree );
}

BEGIN_MESSAGE_MAP(DboxMain, CDialog)
	//{{AFX_MSG_MAP(DboxMain)
   ON_WM_DESTROY()
   ON_WM_SIZE()
   ON_COMMAND(ID_MENUITEM_ABOUT, OnAbout)
   ON_COMMAND(ID_PWSAFE_WEBSITE, OnPasswordSafeWebsite)
   ON_COMMAND(ID_MENUITEM_COPYUSERNAME, OnCopyUsername)
#if defined(POCKET_PC)
   ON_WM_CREATE()
#else
   ON_WM_CONTEXTMENU()
#endif
   ON_NOTIFY(LVN_KEYDOWN, IDC_ITEMLIST, OnKeydownItemlist)
   ON_NOTIFY(NM_DBLCLK, IDC_ITEMLIST, OnItemDoubleClick)
   ON_NOTIFY(NM_DBLCLK, IDC_ITEMTREE, OnItemDoubleClick)
   ON_COMMAND(ID_MENUITEM_BROWSE, OnBrowse)
   ON_COMMAND(ID_MENUITEM_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_MENUITEM_NEW, OnNew)
   ON_COMMAND(ID_MENUITEM_OPEN, OnOpen)
   ON_COMMAND(ID_MENUITEM_MERGE, OnMerge)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_MERGE, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_RESTORE, OnRestore)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_RESTORE, OnUpdateROCommand)
   ON_COMMAND(ID_MENUTIME_SAVEAS, OnSaveAs)
   ON_COMMAND(ID_MENUITEM_BACKUPSAFE, OnBackupSafe)
   ON_COMMAND(ID_MENUITEM_CHANGECOMBO, OnPasswordChange)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_CHANGECOMBO, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_CLEARCLIPBOARD, OnClearClipboard)
   ON_COMMAND(ID_MENUITEM_DELETE, OnDelete)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_DELETE, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_EDIT, OnEdit)
   ON_COMMAND(ID_MENUITEM_RENAME, OnRename)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_RENAME, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_FIND, OnFind)
   ON_COMMAND(ID_MENUITEM_DUPLICATEENTRY, OnDuplicateEntry)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_DUPLICATEENTRY, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_OPTIONS, OnOptions)
   ON_COMMAND(ID_MENUITEM_SAVE, OnSave)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_SAVE, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_LIST_VIEW, OnListView)
   ON_COMMAND(ID_MENUITEM_TREE_VIEW, OnTreeView)
   ON_COMMAND(ID_MENUITEM_OLD_TOOLBAR, OnOldToolbar)
   ON_COMMAND(ID_MENUITEM_NEW_TOOLBAR, OnNewToolbar)
   ON_COMMAND(ID_MENUITEM_EXPANDALL, OnExpandAll)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_EXPANDALL, OnUpdateTVCommand)
   ON_COMMAND(ID_MENUITEM_COLLAPSEALL, OnCollapseAll)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_COLLAPSEALL, OnUpdateTVCommand)
   ON_COMMAND(ID_MENUITEM_CHANGEFONT, OnChangeFont)
   ON_COMMAND_RANGE(ID_FILE_EXPORTTO_OLD1XFORMAT, ID_FILE_EXPORTTO_V2FORMAT, OnExportVx)
   ON_COMMAND(ID_FILE_EXPORTTO_PLAINTEXT, OnExportText)
   ON_COMMAND(ID_FILE_EXPORTTO_XML, OnExportXML)
   ON_COMMAND(ID_FILE_IMPORT_PLAINTEXT, OnImportText)
   ON_UPDATE_COMMAND_UI(ID_FILE_IMPORT_PLAINTEXT, OnUpdateROCommand)
   ON_COMMAND(ID_FILE_IMPORT_KEEPASS, OnImportKeePass)
   ON_UPDATE_COMMAND_UI(ID_FILE_IMPORT_KEEPASS, OnUpdateROCommand)
   ON_COMMAND(ID_FILE_IMPORT_XML, OnImportXML)
   ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_ADD, OnUpdateROCommand)
   ON_COMMAND(ID_MENUITEM_ADDGROUP, OnAddGroup)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_ADDGROUP, OnUpdateROCommand)
   ON_WM_TIMER()
   ON_COMMAND(ID_MENUITEM_AUTOTYPE, OnAutoType)
#if defined(POCKET_PC)
   ON_COMMAND(ID_MENUITEM_SHOWPASSWORD, OnShowPassword)
#else
   ON_NOTIFY(NM_SETFOCUS, IDC_ITEMLIST, OnSetfocusItemlist)
   ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMLIST, OnKillfocusItemlist)
   ON_NOTIFY(NM_SETFOCUS, IDC_ITEMTREE, OnSetfocusItemlist)
   ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMTREE, OnKillfocusItemlist)
   ON_WM_DROPFILES()
#endif
   ON_NOTIFY(LVN_COLUMNCLICK, IDC_ITEMLIST, OnColumnClick)
   ON_UPDATE_COMMAND_UI(ID_FILE_MRU_ENTRY1, OnUpdateMRU)
   ON_WM_INITMENUPOPUP()
   ON_COMMAND(ID_MENUITEM_EXIT, OnOK)
   ON_COMMAND(ID_MENUITEM_MINIMIZE, OnMinimize)
   ON_COMMAND(ID_MENUITEM_UNMINIMIZE, OnUnMinimize)
#ifndef POCKET_PC
   ON_COMMAND(ID_MENUITEM_TRAYLOCKUNLOCK, OnTrayLockUnLock)
   ON_UPDATE_COMMAND_UI(ID_MENUITEM_TRAYLOCKUNLOCK, OnUpdateTrayLockUnLockCommand)
   ON_COMMAND(ID_TRAYRECENT_ENTRY_CLEAR, OnTrayClearRecentEntries)
   ON_UPDATE_COMMAND_UI(ID_TRAYRECENT_ENTRY_CLEAR, OnUpdateTrayClearRecentEntries)
   ON_WM_INITMENU()
   ON_COMMAND(ID_TOOLBUTTON_ADD, OnAdd)
   ON_COMMAND(ID_TOOLBUTTON_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_TOOLBUTTON_COPYUSERNAME, OnCopyUsername)
   ON_COMMAND(ID_TOOLBUTTON_CLEARCLIPBOARD, OnClearClipboard)
   ON_COMMAND(ID_TOOLBUTTON_DELETE, OnDelete)
   ON_COMMAND(ID_TOOLBUTTON_EDIT, OnEdit)
   ON_COMMAND(ID_TOOLBUTTON_NEW, OnNew)
   ON_COMMAND(ID_TOOLBUTTON_OPEN, OnOpen)
   ON_COMMAND(ID_TOOLBUTTON_SAVE, OnSave)
#endif
   ON_WM_SYSCOMMAND()
#if !defined(POCKET_PC)
   ON_BN_CLICKED(IDOK, OnEdit)
   ON_WM_SIZING()
#endif
   ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	//}}AFX_MSG_MAP

   ON_COMMAND_EX_RANGE(ID_FILE_MRU_ENTRY1, ID_FILE_MRU_ENTRYMAX, OnOpenMRU)
#ifndef POCKET_PC
   ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYUSERNAME1, ID_MENUITEM_TRAYCOPYUSERNAMEMAX, OnTrayCopyUsername)
   ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYUSERNAME1, ID_MENUITEM_TRAYCOPYUSERNAMEMAX, OnUpdateTrayCopyUsername)
   ON_COMMAND_RANGE(ID_MENUITEM_TRAYCOPYPASSWORD1, ID_MENUITEM_TRAYCOPYPASSWORDMAX, OnTrayCopyPassword)
   ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYCOPYPASSWORD1, ID_MENUITEM_TRAYCOPYPASSWORDMAX, OnUpdateTrayCopyPassword)
   ON_COMMAND_RANGE(ID_MENUITEM_TRAYBROWSE1, ID_MENUITEM_TRAYBROWSEMAX, OnTrayBrowse)
   ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYBROWSE1, ID_MENUITEM_TRAYBROWSEMAX, OnUpdateTrayBrowse)
   ON_COMMAND_RANGE(ID_MENUITEM_TRAYDELETE1, ID_MENUITEM_TRAYDELETEMAX, OnTrayDeleteEntry)
   ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYDELETE1, ID_MENUITEM_TRAYDELETEMAX, OnUpdateTrayDeleteEntry)       
   ON_COMMAND_RANGE(ID_MENUITEM_TRAYAUTOTYPE1, ID_MENUITEM_TRAYAUTOTYPEMAX, OnTrayAutoType)
   ON_UPDATE_COMMAND_UI_RANGE(ID_MENUITEM_TRAYAUTOTYPE1, ID_MENUITEM_TRAYAUTOTYPEMAX, OnUpdateTrayAutoType)   
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
#endif
END_MESSAGE_MAP()

void
DboxMain::InitPasswordSafe()
{
  // Real initialization done here
  // Requires OnInitDialog to have passed OK
  // AlwaysOnTop preference read from database, if possible, hence set after OpenOnInit
  m_bAlwaysOnTop = PWSprefs::GetInstance()->GetPref(PWSprefs::BoolPrefs::AlwaysOnTop);
  UpdateAlwaysOnTop();

  // ... same for UseSystemTray
  // StartSilent trumps preference
  if (!m_IsStartSilent && !PWSprefs::GetInstance()->
      GetPref(PWSprefs::BoolPrefs::UseSystemTray))
    app.HideIcon();

  m_RUEList.SetMax(PWSprefs::GetInstance()->GetPref(PWSprefs::IntPrefs::MaxREItems));
  
  // Set timer for user-defined lockout, if selected
  if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::BoolPrefs::LockOnIdleTimeout)) {
    const UINT MINUTE = 60*1000;
    TRACE(_T("Starting Idle time lock timer"));
    SetTimer(TIMER_USERLOCK, MINUTE, NULL);
    ResetIdleLockCounter();
  }

  // JHF : no hotkeys on WinCE
#if !defined(POCKET_PC)
  // Set Hotkey, if active
  if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::BoolPrefs::HotKeyEnabled)) {
    const DWORD value = DWORD(PWSprefs::GetInstance()->
                              GetPref(PWSprefs::IntPrefs::HotKey));
    // Following contortions needed 'cause MS couldn't get their
    // act together and keep a consistent interface. Argh.
    WORD v = WORD((value & 0xff) | ((value & 0xff0000) >> 8));
    SendMessage(WM_SETHOTKEY, v);

  }
#endif

  m_windowok = true;
	
  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog

  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIconSm, FALSE); // Set small icon
  // Init stuff for tree view
  CImageList *pImageList = new CImageList();
  BOOL status = pImageList->Create(9, 9, ILC_COLOR, 2, 0);
  ASSERT(status != 0);
  CBitmap bitmap;

  // Order of LoadBitmap() calls matches CMyTreeCtrl public enum
  bitmap.LoadBitmap(IDB_NODE);
  pImageList->Add(&bitmap, (COLORREF)0x0);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_LEAF);
  pImageList->Add(&bitmap, (COLORREF)0x0);
  bitmap.DeleteObject();
  m_ctlItemTree.SetImageList(pImageList, TVSIL_NORMAL);

  // Init stuff for list view
  m_ctlItemList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  int iColumnCount = 3;
  m_ctlItemList.InsertColumn(0, _T("Title"));
  m_ctlItemList.InsertColumn(1, _T("User Name"));
  m_ctlItemList.InsertColumn(2, _T("Notes"));

  m_bShowPasswordInEdit = PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::ShowPWDefault);

  m_bShowPasswordInList = PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::ShowPWInList);

  const CString lastView = PWSprefs::GetInstance()->
    GetPref(PWSprefs::StringPrefs::LastView);
  m_IsListView = true;
  if (lastView != _T("list")) {
    // not list mode, so start in tree view.
    m_ctlItemList.ShowWindow(SW_HIDE);
    m_ctlItemTree.ShowWindow(SW_SHOW);
    m_IsListView = false;
  }

  CRect rect;
  m_ctlItemList.GetClientRect(&rect);
  int i1stWidth = PWSprefs::GetInstance()->GetPref(PWSprefs::IntPrefs::Column1Width,
                                                   (rect.Width() / iColumnCount +
                                                    rect.Width() % iColumnCount));
  int i2ndWidth = PWSprefs::GetInstance()->GetPref(PWSprefs::IntPrefs::Column2Width,
                                                   rect.Width() / iColumnCount);
  int i3rdWidth = PWSprefs::GetInstance()->GetPref(PWSprefs::IntPrefs::Column3Width,
                                                   rect.Width() / iColumnCount);

  m_ctlItemList.SetColumnWidth(0, i1stWidth);
  m_ctlItemList.SetColumnWidth(1, i2ndWidth);
  m_ctlItemList.SetColumnWidth(2, i3rdWidth);

  m_iSortedColumn = PWSprefs::GetInstance()->GetPref(PWSprefs::IntPrefs::SortedColumn);
  m_bSortAscending = PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::SortAscending);

  // refresh list will add and size password column if necessary...
  RefreshList();
  ChangeOkUpdate();

  setupBars(); // Just to keep things a little bit cleaner

#if !defined(POCKET_PC)
  // {kjp} Can't drag and drop files onto an application in PocketPC
  DragAcceptFiles(TRUE);

  // {kjp} meaningless when target is a PocketPC device.
  PWSprefs::GetInstance()->GetPrefRect(rect.top, rect.bottom, rect.left, rect.right);

  if (rect.top == -1 || rect.bottom == -1 || rect.left == -1 || rect.right == -1) {
    GetWindowRect(&rect);
    SendMessage(WM_SIZE, SIZE_RESTORED, MAKEWPARAM(rect.Width(), rect.Height()));
  } else {
    // Sanity checks on stored rect - displays change...
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (rect.right > screenWidth || rect.bottom > screenHeight ||
        rect.left > screenWidth || rect.top > screenHeight) {
      // if any corner is out of screen, fallback to sane values
      rect.top = rect.left = 10;
      rect.bottom = 320; rect.right = 230;
    }
    MoveWindow(&rect, TRUE);
  }
#endif

  m_core.SetUseDefUser(PWSprefs::GetInstance()->
                       GetPref(PWSprefs::BoolPrefs::UseDefUser));
  m_core.SetDefUsername(PWSprefs::GetInstance()->
                        GetPref(PWSprefs::StringPrefs::DefUserName));

  CString szTreeFont = PWSprefs::GetInstance()->
    GetPref(PWSprefs::StringPrefs::TreeFont);

  if (szTreeFont != _T("")) {
    ExtractFont(szTreeFont);
    m_hFontTree = ::CreateFontIndirect(&m_treefont);
    // transfer the fonts to the tree windows
    m_ctlItemTree.SendMessage(WM_SETFONT, (WPARAM) m_hFontTree, true);
    m_ctlItemList.SendMessage(WM_SETFONT, (WPARAM) m_hFontTree, true);
  }
  	       
  SetMenu(app.m_mainmenu);  // Now show menu...
}

BOOL
DboxMain::OnInitDialog()
{
  ConfigureSystemMenu();

  CDialog::OnInitDialog();

  if (!m_IsStartSilent && (OpenOnInit() == FALSE))
      return TRUE;

  InitPasswordSafe();
  
  return TRUE;  // return TRUE unless you set the focus to a control
}


BOOL
DboxMain::OpenOnInit(void)
{
  /*
    Routine to account for the differences between opening PSafe for
    the first time, and just opening a different database or
    un-minimizing the application
  */
  CMyString passkey;
  int rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, GCP_FIRST);  // First
  int rc2 = PWScore::NOT_SUCCESS;

  switch (rc) {
  case PWScore::SUCCESS:
    rc2 = m_core.ReadCurFile(passkey);
#if !defined(POCKET_PC)
    m_title = "Password Safe - " + m_core.GetCurFile();
    UpdateSystemTray(UNLOCKED);
#endif
    break; 
  case PWScore::CANT_OPEN_FILE:
    if (m_core.GetCurFile().IsEmpty()) {
      // Empty filename. Assume they are starting Password Safe 
      // for the first time and don't confuse them.
      // fallthrough to New()
    } else {
      // Here if there was a filename saved from last invocation, but it couldn't
      // be opened. It was either removed or renamed, so ask the user what to do
      CMyString msg = _T("The database ") + m_core.GetCurFile();
      msg += _T(" couldn't be opened.\nDo you wish to look for it elsewhere (Yes), ");
      msg += _T("create a new database (No), or exit (Cancel)?");
      int rc3 = MessageBox(msg, AfxGetAppName(), (MB_ICONQUESTION | MB_YESNOCANCEL));
      switch (rc3) {
      case IDYES:
        rc2 = Open();
        break;
      case IDNO:
        rc2 = New();
        break;
      case IDCANCEL:
        rc2 = PWScore::USER_CANCEL;
        break;
      }
      break;
    }
  case TAR_NEW:
    rc2 = New();
    if (PWScore::USER_CANCEL == rc2) {
      // somehow, get DboxPasskeyEntryFirst redisplayed...
    }
    break;
  case TAR_OPEN:
    rc2 = Open();
    if (PWScore::USER_CANCEL == rc2) {
      // somehow, get DboxPasskeyEntryFirst redisplayed...
    }
    break;
  case PWScore::WRONG_PASSWORD:
  default:
    break;
  }

  switch (rc2) {
  case PWScore::BAD_DIGEST: {
    CMyString msg(m_core.GetCurFile());
    msg += _T("\n\nFile corrupt or truncated!\n");
    msg += _T("Data may have been lost or modified.\nContinue anyway?");
    const int yn = MessageBox(msg, _T("File Read Error"),
                              MB_YESNO|MB_ICONERROR);
    if (yn == IDNO) {
      CDialog::OnCancel();
      return FALSE;
    }
  }
    // DELIBERATE FALL-THRU if user chose YES
  case PWScore::SUCCESS:
    m_existingrestore = FALSE;
    m_needsreading = false;
    startLockCheckTimer();
    UpdateSystemTray(UNLOCKED);
    return TRUE;
  default:
    CDialog::OnCancel();
    return FALSE;
  }
}


void
DboxMain::OnDestroy()
{
  const CMyString filename(m_core.GetCurFile());
  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.IsEmpty() && !m_IsReadOnly && m_core.IsLockedFile(filename))
    m_core.UnlockFile(filename);
  CDialog::OnDestroy();
}



void
DboxMain::OnItemDoubleClick( NMHDR *, LRESULT *)
{
#if defined(POCKET_PC)
  if ( app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dcshowspassword"), FALSE) == FALSE ) {
    OnCopyPassword();
  } else {
    OnShowPassword();
  }
#else
  switch (PWSprefs::GetInstance()->
          GetPref(PWSprefs::IntPrefs::DoubleClickAction)) {
  case PWSprefs::DoubleClickCopy:
    OnCopyPassword();
    break;
  case PWSprefs::DoubleClickEdit:
    PostMessage(WM_COMMAND, ID_MENUITEM_EDIT);
    break;
  case PWSprefs::DoubleClickAutoType:
    PostMessage(WM_COMMAND, ID_MENUITEM_AUTOTYPE);
    break;
  case PWSprefs::DoubleClickBrowse:
    PostMessage(WM_COMMAND, ID_MENUITEM_BROWSE);
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
  if(ci != NULL) {
	if (!ci->GetURL().IsEmpty()) {
			LaunchBrowser(m_BrowseURL);
			uuid_array_t RUEuuid;
			ci->GetUUID(RUEuuid);
			m_RUEList.AddRUEntry(RUEuuid);
	}
  }
}

void DboxMain::ToClipboard(const CMyString &data)
{
  uGlobalMemSize = (data.GetLength() + 1) * sizeof(TCHAR);
  hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, uGlobalMemSize);
  // {kjp} fix to use UNICODE safe string definitions and string copy functions
  LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);
		
  strCopy( pGlobalLock, data);
		
  GlobalUnlock(hGlobalMemory);	
		
  if (OpenClipboard() == TRUE) {
    if (EmptyClipboard()!=TRUE) {
      DBGMSG("The clipboard was not emptied correctly");
    }
    if (SetClipboardData(CLIPBOARD_TEXT_FORMAT, hGlobalMemory) == NULL) {
      DBGMSG("The data was not pasted into the clipboard correctly");
      GlobalFree(hGlobalMemory); // wasn't passed to Clipboard
    } else {
      // identify data in clipboard as ours, so as not to clear the wrong data later
      // of course, we don't want an extra copy of a password floating around
      // in memory, so we'll use the hash
      const char *str = (const char *)data;
      SHA1 ctx;
      ctx.Update((const unsigned char *)str, data.GetLength());
      ctx.Final(m_clipboard_digest);
      m_clipboard_set = true;
    }
    if (CloseClipboard() != TRUE) {
      DBGMSG("The clipboard could not be closed");
    }
  } else {
    DBGMSG("The clipboard could not be opened correctly");
    GlobalFree(hGlobalMemory); // wasn't passed to Clipboard
  }
}

void
DboxMain::OnCopyPassword() 
{
  if (!SelItemOk()) 
    return;

  //Remind the user about clipboard security
  CClearQuestionDlg clearDlg(this);
  if (clearDlg.m_dontaskquestion == FALSE &&
      clearDlg.DoModal() == IDCANCEL)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);
  ToClipboard(ci->GetPassword());
  uuid_array_t RUEuuid;
  ci->GetUUID(RUEuuid);
  m_RUEList.AddRUEntry(RUEuuid);
}

void
DboxMain::OnCopyUsername() 
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);
  const CMyString username = ci->GetUser();

  if (!username.IsEmpty()) {
    ToClipboard(username);
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::ClearClipboard()
{
  // Clear the clipboard IFF its value is the same as last set by this app.
  if (!m_clipboard_set)
    return;
  if (OpenClipboard() != TRUE) {
    DBGMSG("The clipboard could not be opened correctly");
    return;
  }
  if (IsClipboardFormatAvailable(CLIPBOARD_TEXT_FORMAT) != 0) {
    HGLOBAL hglb = GetClipboardData(CLIPBOARD_TEXT_FORMAT); 
    if (hglb != NULL) {
      LPTSTR lptstr = (LPTSTR)GlobalLock(hglb); 
      if (lptstr != NULL) {
        // check identity of data in clipboard
        unsigned char digest[20];
        SHA1 ctx;
        ctx.Update((const unsigned char *)lptstr, strLength(lptstr));
        ctx.Final(digest);
        if (memcmp(digest, m_clipboard_digest, sizeof(digest)) == 0) {
          trashMemory( lptstr, strLength(lptstr));
          GlobalUnlock(hglb);
          if (EmptyClipboard() == TRUE) {
            m_clipboard_set = false;
          } else {
            DBGMSG("The clipboard was not emptied correctly");
          }
        } else { // hashes match 
          GlobalUnlock(hglb);
        }
      } // lptstr != NULL
    } // hglb != NULL
  } // IsClipboardFormatAvailable
  if (CloseClipboard() != TRUE) {
    DBGMSG("The clipboard could not be closed");
  }
}


void
DboxMain::OnFind() 
{
  m_LockDisabled = true;
  CFindDlg::Doit(this, &m_lastFindCS,
                 &m_lastFindStr); // create modeless or popup existing
  // XXX Gross hack to fix aesthetic bug in tree view
  // without this, multiple "selected" displayed
  // if treeview && there's a selected item, then
#if 0
  m_ctlItemTree.SetItemState(di->tree_item,
                             TVIS_SELECTED,
                             TVIS_DROPHILITED | TVIS_SELECTED);
#endif
  m_LockDisabled = false;
}



// Change the master password for the database.
void
DboxMain::OnPasswordChange() 
{
  if (m_IsReadOnly) // disable in read-only mode
    return;
  m_LockDisabled = true;
  CPasskeyChangeDlg changeDlg(this);
  app.DisableAccelerator();
  int rc = changeDlg.DoModal();
  app.EnableAccelerator();
  m_LockDisabled = false;
  if (rc == IDOK) {
    m_core.ChangePassword(changeDlg.m_newpasskey);
  }
}


void
DboxMain::OnClearClipboard() 
{
   ClearClipboard();
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

void
DboxMain::OnUpdateROCommand(CCmdUI *pCmdUI)
{
  // Use this callback  for commands that need to
  // be disabled in read-only mode
  pCmdUI->Enable(m_IsReadOnly ? FALSE : TRUE);
}

void
DboxMain::OnUpdateTVCommand(CCmdUI *pCmdUI)
{
  // Use this callback for commands that need to
  // be disabled in ListView mode
  pCmdUI->Enable(m_IsListView ? FALSE : TRUE);
}

void
DboxMain::OnSave() 
{
  if (m_IsReadOnly) // disable in read-only mode
    return;
  m_LockDisabled = true;
  Save();
  m_LockDisabled = false;
}

void
DboxMain::OnExportVx(UINT nID)
{
  int rc;
  CMyString newfile;

  //SaveAs-type dialog box
  while (1) {
    m_LockDisabled = true;
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   m_core.GetCurFile(),
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   SUFFIX_FILTERS
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle =
      _T("Please name the exported database");
    rc = fd.DoModal();
    m_LockDisabled = false;
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();
      break;
    } else
      return;
  }

  switch (nID) {
  case ID_FILE_EXPORTTO_OLD1XFORMAT:
    rc = m_core.WriteV17File(newfile);
    break;
  case ID_FILE_EXPORTTO_V2FORMAT:
    rc = m_core.WriteV2File(newfile);
    break;
  default:
    ASSERT(0);
    rc = PWScore::FAILURE;
    break;
  }
  if (rc == PWScore::CANT_OPEN_FILE) {
    CMyString temp = newfile + _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
  }
}

void
DboxMain::OnExportText()
{
  CExportTextDlg et;
  m_LockDisabled = true;
  int rc = et.DoModal();
  m_LockDisabled = false;
  if (rc == IDOK) {
    CMyString newfile;
    CMyString pw(et.m_exportTextPassword);
    if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
      // do the export
      //SaveAs-type dialog box
      while (1) {
        CFileDialog fd(FALSE,
                       _T("txt"),
                       _T(""),
                       OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                       |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                       _T("Text files (*.txt)|*.txt|")
                       _T("CSV files (*.csv)|*.csv|")
                       _T("All files (*.*)|*.*|")
                       _T("|"),
                       this);
        fd.m_ofn.lpstrTitle =
          _T("Please name the plaintext file");
        m_LockDisabled = true;
        rc = fd.DoModal();
        m_LockDisabled = false;
        if (rc == IDOK) {
          newfile = (CMyString)fd.GetPathName();
          break;
        } else
          return;
      } // while (1)

      if (et.m_querysetexpdelim == 1) {
        char delimiter;
        delimiter = et.m_defexpdelim[0];
        rc = m_core.WritePlaintextFile(newfile, delimiter);
      } else {
        rc = m_core.WritePlaintextFile(newfile);
      }
		
      if (rc == PWScore::CANT_OPEN_FILE)        {
        CMyString temp = newfile + _T("\n\nCould not open file for writing!");
        MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      }
    } else {
      MessageBox(_T("Passkey incorrect"), _T("Error"));
      Sleep(3000); // protect against automatic attacks
    }
  }
}

void
DboxMain::OnExportXML()
{
  m_LockDisabled = true;
    // TODO - currently disabled in menubar
  m_LockDisabled = false;
}

void
DboxMain::OnImportText()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  CImportDlg dlg;
  int status = dlg.DoModal();
  
  if (status == IDCANCEL)
    return;

  CMyString ImportedPrefix(dlg.m_groupName);
  TCHAR fieldSeparator(dlg.m_Separator[0]);
  CFileDialog fd(TRUE,
                 _T("txt"),
                 NULL,
                 OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                 _T("Text files (*.txt)|*.txt|")
                 _T("CSV files (*.csv)|*.csv|")
                 _T("All files (*.*)|*.*|")
                 _T("|"),
                 this);
  fd.m_ofn.lpstrTitle = _T("Please Choose a Text File to Import");
  m_LockDisabled = true;
  int rc = fd.DoModal();
  m_LockDisabled = false;
  if (rc == IDOK) {
    CMyString newfile = (CMyString)fd.GetPathName();
    int numImported = 0, numSkipped = 0;
    char delimiter;
    if (dlg.m_querysetimpdelim == 1) {
      delimiter = dlg.m_defimpdelim[0];
    } else {
      delimiter = '\0';
    }
    rc = m_core.ImportPlaintextFile(ImportedPrefix, newfile, fieldSeparator,
                                    delimiter, numImported, numSkipped);

    switch (rc) {
    case PWScore::CANT_OPEN_FILE:
      {
        CMyString temp = newfile + _T("\n\nCould not open file for reading!");
        MessageBox(temp, _T("File open error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::INVALID_FORMAT:
      {
        CMyString temp = newfile + _T("\n\nInvalid format");
        MessageBox(temp, _T("File read error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::SUCCESS:
    default:
      {
        OSTRSTREAM os;
        os << "Read " << numImported << " record";
        if (numImported != 1) os << "s";
        if (numSkipped != 0) {
          os << "\nCouldn't read " << numSkipped << " record";
          if (numSkipped > 1) os << "s";
        }
        os << ENDS;
        CMyString temp(os.str());
        MessageBox(temp, _T("Status"), MB_ICONINFORMATION|MB_OK);
      }
      RefreshList();
      break;
    } // switch
  }
}

void
DboxMain::OnImportKeePass()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  CFileDialog fd(TRUE,
                 _T("txt"),
                 NULL,
                 OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                 _T("Text files (*.txt)|*.txt|")
                 _T("CSV files (*.csv)|*.csv|")
                 _T("All files (*.*)|*.*|")
                 _T("|"),
                 this);
  fd.m_ofn.lpstrTitle = _T("Please Choose a KeePass Text File to Import");
  m_LockDisabled = true;
  int rc = fd.DoModal();
  m_LockDisabled = false;
  if (rc == IDOK) {
    CMyString newfile = (CMyString)fd.GetPathName();
    rc = m_core.ImportKeePassTextFile(newfile);
    switch (rc) {
    case PWScore::CANT_OPEN_FILE:
      {
        CMyString temp = newfile + _T("\n\nCould not open file for reading!");
        MessageBox(temp, _T("File open error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::INVALID_FORMAT:
      {
        CMyString temp = newfile + _T("\n\nInvalid format");
        MessageBox(temp, _T("File read error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::SUCCESS:
    default:
      RefreshList();
      break;
    } // switch
  }
}

void
DboxMain::OnImportXML()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  m_LockDisabled = true;
  // TODO - currently disabled in menubar
  m_LockDisabled = false;
}


void DboxMain::SetChanged(bool changed) // for MyTreeCtrl
{
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::BoolPrefs::SaveImmediately)) {
    Save();
  } else {
    m_core.SetChanged(changed);
  }
}


int
DboxMain::Save()
{
  int rc;

  if (m_core.GetCurFile().IsEmpty())
    return SaveAs();

  if (m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
    m_core.BackupCurFile(); // to save previous reversion
  } else { // file version mis-match
    CString NewName(m_core.GetCurFile());
    int dotIndex = NewName.ReverseFind(TCHAR('.'));
    if (dotIndex != -1)
      NewName = NewName.Left(dotIndex+1);
    NewName += DEFAULT_SUFFIX;


    CString msg = _T("The original database, \"");
    msg += CString(m_core.GetCurFile());
    msg += _T("\", is in pre-3.0 format."
              " It will be unchanged.\n");
    msg += _T("Your changes will be written as \"");
    msg += NewName;
    msg += _T("\" in the new"
              " format, which is unusable by old versions of PasswordSafe."
              " To save your changes in the old format, use the \"File->Export To"
              "-> Old (1.x or 2) format\" command.\n\n"
              "Press OK to continue saving, or Cancel to stop.");
    if (MessageBox(msg, _T("File version warning"),
                   MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
      return PWScore::USER_CANCEL;
    m_core.SetCurFile(NewName);
#if !defined(POCKET_PC)
    m_title = _T("Password Safe - ") + m_core.GetCurFile();
    app.SetTooltipText(m_core.GetCurFile());
#endif
  }
  rc = m_core.WriteCurFile();

  if (rc == PWScore::CANT_OPEN_FILE) {
    CMyString temp = m_core.GetCurFile() +
      _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error"), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  ChangeOkUpdate();
  return PWScore::SUCCESS;
}


void
DboxMain::ChangeOkUpdate()
{
  if (! m_windowok)
    return;

#if defined(POCKET_PC)
  CMenu *menu	= m_wndMenu;
#else
  CMenu *menu	= GetMenu();
#endif

  menu->EnableMenuItem(ID_MENUITEM_SAVE,
                       m_core.IsChanged() ? MF_ENABLED : MF_GRAYED);

  /*
    This doesn't exactly belong here, but it makes sure that the
    title is fresh...
  */
#if !defined(POCKET_PC)
  SetWindowText(LPCTSTR(m_title));
#endif
}


void
DboxMain::OnAbout() 
{
  DboxAbout dbox;
  m_LockDisabled = true;
  dbox.DoModal();
  m_LockDisabled = false;
}


void DboxMain::OnPasswordSafeWebsite()
{
  HINSTANCE stat = ::ShellExecute(NULL, NULL, "http://passwordsafe.sourceforge.net/",
                                  NULL, _T("."), SW_SHOWNORMAL);
  if (int(stat) < 32) {
#ifdef _DEBUG
    AfxMessageBox("oops");
#endif
  }
}


void
DboxMain::OnBackupSafe() 
{
  m_LockDisabled = true;
  BackupSafe();
  m_LockDisabled = false;
}


int
DboxMain::BackupSafe()
{
  int rc;
  PWSprefs *prefs = PWSprefs::GetInstance();
  CMyString tempname;
  CMyString currbackup =
    prefs->GetPref(PWSprefs::StringPrefs::CurrentBackup);


  //SaveAs-type dialog box
  while (1)
    {
      CFileDialog fd(FALSE,
                     _T("bak"),
                     currbackup,
                     OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                     | OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                     _T("Password Safe Backups (*.bak)|*.bak||"),
                     this);
      fd.m_ofn.lpstrTitle = _T("Please Choose a Name for this Backup:");

      rc = fd.DoModal();
      if (rc == IDOK) {
        tempname = (CMyString)fd.GetPathName();
        break;
      } else
        return PWScore::USER_CANCEL;
    }


  rc = m_core.WriteFile(tempname);
  if (rc == PWScore::CANT_OPEN_FILE) {
    CMyString temp = tempname + _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  prefs->SetPref(PWSprefs::StringPrefs::CurrentBackup, tempname);
  return PWScore::SUCCESS;
}


void
DboxMain::OnOpen() 
{
  m_LockDisabled = true;
  Open();
  m_LockDisabled = false;
}


int
DboxMain::Open()
{
  int rc = PWScore::SUCCESS;
  CMyString newfile;

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   DEFAULT_SUFFIX,
                   NULL,
                   OFN_FILEMUSTEXIST|OFN_LONGNAMES,
                   SUFFIX_FILTERS
                   _T("Password Safe Backups (*.bak)|*.bak|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = _T("Please Choose a Database to Open:");
    rc = fd.DoModal();
    const bool last_ro = m_IsReadOnly; // restore if user cancels
    m_IsReadOnly = (fd.GetReadOnlyPref() == TRUE);
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();

      rc = Open( newfile );

      if ( rc == PWScore::SUCCESS ) {
        UpdateSystemTray(UNLOCKED);
        m_RUEList.ClearEntries();
        break;
      }
    } else {
      m_IsReadOnly = last_ro;
      return PWScore::USER_CANCEL;
    }
  }
  return rc;
}

int
DboxMain::Open( const CMyString &pszFilename )
{
  int rc;
  CMyString passkey, temp;

  //Check that this file isn't already open
  if (pszFilename == m_core.GetCurFile() && !m_needsreading)
	{
      //It is the same damn file
      MessageBox(_T("That file is already open."),
                 _T("Oops!"),
                 MB_OK|MB_ICONWARNING);
      return PWScore::ALREADY_OPEN;
	}	
	
  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS)
    return rc;

  // if we were using a different file, unlock it
  // do this before GetAndCheckPassword() as that
  // routine gets a lock on the new file
  if( !m_core.GetCurFile().IsEmpty() ) {
    m_core.UnlockFile(m_core.GetCurFile());
  }

  // clear the data before loading the new file
  ClearData();

  rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL);  // OK, CANCEL, HELP
  switch (rc) {
  case PWScore::SUCCESS:
    app.GetMRU()->Add(pszFilename);
    break; // Keep going... 
  case PWScore::CANT_OPEN_FILE:
    temp = m_core.GetCurFile()
      + _T("\n\nCan't open file. Please choose another.");
    MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
  case TAR_OPEN:
    return Open();
  case TAR_NEW:
    return New();
  case PWScore::WRONG_PASSWORD:
  case PWScore::USER_CANCEL:
    /*
      If the user just cancelled out of the password dialog, 
      assume they want to return to where they were before... 
    */
    return PWScore::USER_CANCEL;
  default:
    ASSERT(0); // we should take care of all cases explicitly
    return PWScore::USER_CANCEL; // conservative behaviour for release version
  }
	
  temp = pszFilename;
  rc = m_core.ReadFile(pszFilename, passkey);
  switch (rc) {
  case PWScore::SUCCESS:
    break;
  case PWScore::CANT_OPEN_FILE:
    temp += _T("\n\nCould not open file for reading!");
    MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
    /*
      Everything stays as is... Worst case,
      they saved their file....
    */
    return PWScore::CANT_OPEN_FILE;
  case PWScore::BAD_DIGEST: {
    temp += _T("\n\nFile corrupt or truncated!\n");
    temp += _T("Data may have been lost or modified.\nContinue anyway?");
    const int yn = MessageBox(temp, _T("File Read Error"),
                              MB_YESNO|MB_ICONERROR);
    if (yn == IDYES)
      break;
    else
      return rc;
  }
  default:
    temp += _T("Unknown error");
    MessageBox(temp, _T("File read error."), MB_OK|MB_ICONERROR);
	return rc;
  }
  m_core.SetCurFile(pszFilename);
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - ") + m_core.GetCurFile();
#endif
  ChangeOkUpdate();
  RefreshList();
	
  return PWScore::SUCCESS;
}

int
DboxMain::Merge()
{
   int rc = PWScore::SUCCESS;
   CMyString newfile;

   //Open-type dialog box
   while (1)
   {
      CFileDialog fd(TRUE,
                     DEFAULT_SUFFIX,
                     NULL,
                     OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                     SUFFIX_FILTERS
                     _T("Password Safe Backups (*.bak)|*.bak|")
                     _T("All files (*.*)|*.*|")
                     _T("|"),
                     this);
      fd.m_ofn.lpstrTitle = _T("Please Choose a Database to Merge");
      rc = fd.DoModal();
      if (rc == IDOK)
      {
         newfile = (CMyString)fd.GetPathName();

		 rc = Merge( newfile );

		 if ( rc == PWScore::SUCCESS )
	         break;
      }
      else
         return PWScore::USER_CANCEL;
   }

   return rc;
}

int
DboxMain::Merge(const CMyString &pszFilename) {
  /* open file they want to merge */
  int rc = PWScore::SUCCESS;
  CMyString passkey, temp;

  //Check that this file isn't already open
  if (pszFilename == m_core.GetCurFile())
	{
      //It is the same damn file
      MessageBox(_T("That file is already open."),
                 _T("Oops!"),
                 MB_OK|MB_ICONWARNING);
      return PWScore::ALREADY_OPEN;
	}
	
  rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL );  // OK, CANCEL, HELP
  switch (rc)
	{
	case PWScore::SUCCESS:
      app.GetMRU()->Add(pszFilename);
      break; // Keep going... 
	case PWScore::CANT_OPEN_FILE:
      temp = m_core.GetCurFile()
        + _T("\n\nCan't open file. Please choose another.");
      MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
	case TAR_OPEN:
      return Open();
	case TAR_NEW:
      return New();
	case PWScore::WRONG_PASSWORD:
	case PWScore::USER_CANCEL:
      /*
        If the user just cancelled out of the password dialog, 
        assume they want to return to where they were before... 
      */
      return PWScore::USER_CANCEL;
	}
	
  PWScore otherCore;
  otherCore.ReadFile(pszFilename, passkey);
	
  if (rc == PWScore::CANT_OPEN_FILE)
	{
      temp = pszFilename;
      temp += _T("\n\nCould not open file for reading!");
      MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
      /*
		Everything stays as is... Worst case,
		they saved their file....
      */
      return PWScore::CANT_OPEN_FILE;
	}
   
  otherCore.SetCurFile(pszFilename);

  /* Put up hourglass...this might take a while */
  CWaitCursor waitCursor;

  /*
    Purpose:
    Merge entries from otherCore to m_core
	  
    Algorithm:
    Foreach entry in otherCore
    Find in m_core
    if find a match
    if pw, notes, & group also matches
    no merge
    else
    add to m_core with new title suffixed with -merged-HHMMSS-DDMMYY
    else
    add to m_core directly
  */
  int numAdded = 0;
  int numConflicts = 0;

  POSITION otherPos = otherCore.GetFirstEntryPosition();
  while (otherPos) {
    CItemData otherItem = otherCore.GetEntryAt(otherPos);
    CMyString otherGroup = otherItem.GetGroup();
    CMyString otherTitle = otherItem.GetTitle();
    CMyString otherUser = otherItem.GetUser();
		
    POSITION foundPos = m_core.Find(otherGroup, otherTitle, otherUser);
    if (foundPos) {
      /* found a match, see if the pw & notes also match */
      CItemData curItem = m_core.GetEntryAt(foundPos);
      if (otherItem.GetPassword() != curItem.GetPassword() ||
          otherItem.GetNotes() != curItem.GetNotes()) {
        /* have a match on title/user, but not on pw/notes 
           add an entry suffixed with -merged-HHMMSS-DDMMYY */
        CTime curTime = CTime::GetCurrentTime();
        CMyString newTitle = otherItem.GetTitle();
        newTitle += _T("-merged-");
        CMyString timeStr = curTime.Format("%H%M%S-%m%d%y");
        newTitle = newTitle + timeStr;

        /* note it as an issue for the user */
        CMyString warnMsg;
        warnMsg = _T("Conflicting entries for ") +
          otherItem.GetGroup() + _T(",") + 
          otherItem.GetTitle() + _T(",") +
          otherItem.GetUser() + _T("\n");
        warnMsg += _T("Adding new entry as ") +
          newTitle + _T(",") + 
          otherItem.GetUser() + _T("\n");

        /* tell the user the bad news */
        MessageBox(warnMsg,
                   _T("Merge conflict"),
                   MB_OK|MB_ICONWARNING);				

        /* do it */
        otherItem.SetTitle(newTitle);
        m_core.AddEntryToTail(otherItem);

        numConflicts++;
      }
    } else {
      /* didn't find any match...add it directly */
      m_core.AddEntryToTail(otherItem);
      numAdded++;
    }

    otherCore.GetNextEntry(otherPos);
  }

  waitCursor.Restore(); /* restore normal cursor */

  /* tell the user we're done & provide short merge report */
  int totalAdded = numAdded+numConflicts;
  CString resultStr;
  resultStr.Format(_T("Merge complete:\n%d entr%s added (%d conflict%s)"),
                   totalAdded,
                   totalAdded == 1 ? _T("y") : _T("ies"),
                   numConflicts,
                   numConflicts == 1 ? _T("") : _T("s"));
  MessageBox(resultStr, _T("Merge Complete"), MB_OK);

  ChangeOkUpdate();
  RefreshList();
   
  return rc;
}


void
DboxMain::OnMerge()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  m_LockDisabled = true;
  Merge();
  m_LockDisabled = false;
}


void
DboxMain::OnNew()
{
  m_LockDisabled = true;
  New();
  m_LockDisabled = false;
}


int
DboxMain::New() 
{
  int rc, rc2;

  if (m_core.IsChanged()) {
    CMyString temp =
      _T("Do you want to save changes to the password database: ")
      + m_core.GetCurFile()
      + _T("?");

    rc = MessageBox(temp,
                    AfxGetAppName(),
                    MB_ICONQUESTION|MB_YESNOCANCEL);
    switch (rc) {
    case IDCANCEL:
      return PWScore::USER_CANCEL;
    case IDYES:
      rc2 = Save();
      /*
        Make sure that writing the file was successful
      */
      if (rc2 == PWScore::SUCCESS)
        break;
      else
        return PWScore::CANT_OPEN_FILE;
    case IDNO:
      break;
    }
  }

  rc = NewFile();
  if (rc == PWScore::USER_CANCEL)
    /*
      Everything stays as is... 
      Worst case, they saved their file.... 
    */
    return PWScore::USER_CANCEL;

  m_core.SetCurFile(_T("")); //Force a save as... 
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - <Untitled>");
  app.SetTooltipText(_T("PasswordSafe"));
#endif
  ChangeOkUpdate();

  return PWScore::SUCCESS;
}


void
DboxMain::OnRestore()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  m_LockDisabled = true;
  Restore();
  m_LockDisabled = false;
}

int DboxMain::SaveIfChanged()
{
  // offer to save existing database ifit was modified.
  // used before loading another
  // returns PWScore::SUCCESS if save succeeded or if user decided
  // not to save

  if (m_core.IsChanged()) {
    int rc, rc2;
    CMyString temp =
      _T("Do you want to save changes to the password database: ")
      + m_core.GetCurFile()
      + _T("?");
    rc = MessageBox(temp,
                    AfxGetAppName(),
                    MB_ICONQUESTION|MB_YESNOCANCEL);
    switch (rc) {
    case IDCANCEL:
      return PWScore::USER_CANCEL;
    case IDYES:
      rc2 = Save();
      // Make sure that file was successfully written
      if (rc2 == PWScore::SUCCESS)
        break;
      else
        return PWScore::CANT_OPEN_FILE;
    case IDNO:
      break;
    }
  }
  return PWScore::SUCCESS;
}

int
DboxMain::Restore() 
{
  int rc;
  CMyString backup, passkey, temp;
  CMyString currbackup =
    PWSprefs::GetInstance()->GetPref(PWSprefs::StringPrefs::CurrentBackup);

  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS)
    return rc;

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   _T("bak"),
                   currbackup,
                   OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                   _T("Password Safe Backups (*.bak)|*.bak||"),
                   this);
    fd.m_ofn.lpstrTitle = _T("Please Choose a Backup to restore:");
    rc = fd.DoModal();
    if (rc == IDOK) {
      backup = (CMyString)fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  rc = GetAndCheckPassword(backup, passkey, GCP_NORMAL );  // OK, CANCEL, HELP
  switch (rc) {
  case PWScore::SUCCESS:
    break; // Keep going... 
  case PWScore::CANT_OPEN_FILE:
    temp =
      backup
      + _T("\n\nCan't open file. Please choose another.");
    MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
  case TAR_OPEN:
    ASSERT(0); return PWScore::FAILURE; // shouldn't be an option here
  case TAR_NEW:
    ASSERT(0); return PWScore::FAILURE; // shouldn't be an option here
  case PWScore::WRONG_PASSWORD:
    /*
      If the user just cancelled out of the password dialog, 
      assume they want to return to where they were before... 
    */
    return PWScore::USER_CANCEL;
  }

  // unlock the file we're leaving
  if( !m_core.GetCurFile().IsEmpty() ) {
    m_core.UnlockFile(m_core.GetCurFile());
  }

  // clear the data before restoring
  ClearData();

  rc = m_core.ReadFile(backup, passkey);
  if (rc == PWScore::CANT_OPEN_FILE) {
    temp = backup + _T("\n\nCould not open file for reading!");
    MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
	
  m_core.SetCurFile(""); //Force a Save As...
  m_core.SetChanged(true); //So that the restored file will be saved
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - <Untitled Restored Backup>");
  app.SetTooltipText(_T("PasswordSafe"));
#endif
  ChangeOkUpdate();
  RefreshList();

  return PWScore::SUCCESS;
}


void
DboxMain::OnSaveAs()
{
  m_LockDisabled = true;
  SaveAs();
  m_LockDisabled = false;
}


int
DboxMain::SaveAs() 
{
  int rc;
  CMyString newfile;

  if (m_core.GetReadFileVersion() != PWSfile::VCURRENT &&
      m_core.GetReadFileVersion() != PWSfile::UNKNOWN_VERSION) {
    CMyString msg = _T("The original database, \"");
    msg += m_core.GetCurFile();
    msg += _T("\", is in pre-3.0 format. The data will now be written in the new"
              " format, which is unusable by old versions of PasswordSafe."
              " To save the data in the old format, use the \"File->Export To"
              "-> Old (1.x or 2) format\" command.\n\n"
              "Press OK to continue saving, or Cancel to stop.");
    if (MessageBox(msg, _T("File version warning"),
                   MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
      return PWScore::USER_CANCEL;
  }
  //SaveAs-type dialog box
  CMyString v3FileName(m_core.GetCurFile());
  v3FileName.Replace(_T("dat"), DEFAULT_SUFFIX);
  while (1) {
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   v3FileName,
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   SUFFIX_FILTERS
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    if (m_core.GetCurFile().IsEmpty())
      fd.m_ofn.lpstrTitle =
        _T("Please Choose a Name for the Current (Untitled) Database:");
    else
      fd.m_ofn.lpstrTitle =
        _T("Please Choose a New Name for the Current Database:");
    rc = fd.DoModal();
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }
  CMyString locker(_T("")); // null init is important here
  if (!m_core.LockFile(newfile, locker)) {
    CMyString temp = newfile + _T("\n\nFile is currently locked by ") + locker;
    MessageBox(temp, _T("File lock error"), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  rc = m_core.WriteFile(newfile);
  if (rc == PWScore::CANT_OPEN_FILE) {
    m_core.UnlockFile(newfile);
    CMyString temp = newfile + _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error"), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  if (!m_core.GetCurFile().IsEmpty())
    m_core.UnlockFile(m_core.GetCurFile());
  m_core.SetCurFile(newfile);
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - ") + m_core.GetCurFile();
  app.SetTooltipText(m_core.GetCurFile());
#endif
  ChangeOkUpdate();

  app.GetMRU()->Add( newfile );

  return PWScore::SUCCESS;
}

int
DboxMain::GetAndCheckPassword(const CMyString &filename,
			      CMyString& passkey,
			      int index)
{
  // index:
  //	GCP_FIRST    (0) first
  //	GCP_NORMAL   (1) normal
  //    GCP_WITHEXIT (2) with Exit button

  // Called for an existing database. Prompt user
  // for password, verify against file. Lock file to
  // prevent multiple r/w access.
  int retval;

  if (!filename.IsEmpty())
    {
      bool exists = m_core.FileExists(filename);

      if (!exists) {
        // Used to display an error message, but this is really the caller's business
        return PWScore::CANT_OPEN_FILE;
      } // !exists
    } // !filename.IsEmpty()

  /*
   * with my unsightly hacks of PasskeyEntry, it should now accept
   * a blank filename, which will disable passkey entry and the OK button
   */

  CPasskeyEntry dbox_pkentry(this, filename, m_IsReadOnly, index);
  app.DisableAccelerator();
  int rc = dbox_pkentry.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    DBGMSG("PasskeyEntry returns IDOK\n");
    CMyString locker(_T("")); // null init is important here
    passkey = dbox_pkentry.GetPasskey();
    // Set read-only mode if user explicitly requested it OR
    // we could not create a lock file.
    // Note that we depend on lazy evaluation: if the 1st is true,
    // the 2nd won't be called!
    if (index == 0) // if !first, then m_IsReadOnly is set in Open
      m_IsReadOnly =  (dbox_pkentry.IsReadOnly() || !m_core.LockFile(filename, locker));
    else if (!m_IsReadOnly) // !first, lock if !m_IsReadOnly
      m_IsReadOnly = !m_core.LockFile(filename, locker);
    // locker won't be null IFF tried to lock and failed, in which case
    // it shows the current file locker
    if (!locker.IsEmpty()) {
      CString str = _T("The database ");
      str += CString(filename);
      str += _T(" is apparently being used by ");
      str += CString(locker);
      str += _T(".\r\nOpen the database for read-only (Yes),");
      str += _T("read-write (No), or exit (Cancel)?");
      str += _T("\r\n\r\nNote: Choose \"No\" only if you are certain ");
      str += _T("that the file is in fact not being used by anyone else.");
      switch( MessageBox(str, _T("File In Use"),
                         MB_YESNOCANCEL|MB_ICONQUESTION)) {
      case IDYES:  retval = PWScore::SUCCESS; break;
      case IDNO: m_IsReadOnly = false; // Caveat Emptor!
        retval = PWScore::SUCCESS; 
        break;
      case IDCANCEL: retval = PWScore::USER_CANCEL;
        break;
      default:
        ASSERT(false); retval = PWScore::USER_CANCEL;
      }
    } else // locker.IsEmpty() means no lock needed or lock was successful
      retval = PWScore::SUCCESS;
  } else {/*if (rc==IDCANCEL) */ //Determine reason for cancel
    int cancelreturn = dbox_pkentry.GetStatus();
    switch (cancelreturn)
      {
      case TAR_OPEN:
      case TAR_NEW:
        DBGMSG("PasskeyEntry TAR_OPEN or TAR_NEW\n");
        retval = cancelreturn; //Return either open or new flag... 
        break;
      case TAR_CANCEL:
        retval = PWScore::USER_CANCEL;
        break;
      case TAR_EXIT:
        retval = PWScore::USER_EXIT;
        break;
      default:
        DBGMSG("Default to WRONG_PASSWORD\n");
        retval = PWScore::WRONG_PASSWORD;	//Just a normal cancel
        break;
      }
  }

  return retval;
}

int
DboxMain::NewFile(void)
{
  CPasskeySetup dbox_pksetup(this);
  //app.m_pMainWnd = &dbox_pksetup;
  int rc = dbox_pksetup.DoModal();

  if (rc == IDCANCEL)
    return PWScore::USER_CANCEL;  //User cancelled password entry

  ClearData();
  const CMyString filename(m_core.GetCurFile());
  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.IsEmpty() && !m_IsReadOnly && m_core.IsLockedFile(filename))
    m_core.UnlockFile(filename);
  m_IsReadOnly = false; // new file can't be read-only...
  m_core.NewFile(dbox_pksetup.m_passkey);
  m_needsreading = false;
  startLockCheckTimer();
  return PWScore::SUCCESS;
}

BOOL
DboxMain::OnToolTipText(UINT,
                        NMHDR* pNMHDR,
                        LRESULT* pResult)
// This code is copied from the DLGCBR32 example that comes with MFC
{
#if !defined(POCKET_PC)
  ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

  // allow top level routing frame to handle the message
  if (GetRoutingFrame() != NULL)
    return FALSE;

  // need to handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
  TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
  TCHAR szFullText[256];
  CString strTipText;
  UINT nID = pNMHDR->idFrom;
  if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
      pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {
    // idFrom is actually the HWND of the tool
    nID = ((UINT)(WORD)::GetDlgCtrlID((HWND)nID));
  }

  if (nID != 0) // will be zero on a separator
    {
      AfxLoadString(nID, szFullText);
      // this is the command id, not the button index
      AfxExtractSubString(strTipText, szFullText, 1, '\n');
    }
#ifndef _UNICODE
  if (pNMHDR->code == TTN_NEEDTEXTA)
    lstrcpyn(pTTTA->szText, strTipText,
             (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
#if 0 // build problem with new cl? - jpr
  else
    _mbstowcsz(pTTTW->szText, strTipText,
               (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif // 0
#else
  if (pNMHDR->code == TTN_NEEDTEXTA)
    _wcstombsz(pTTTA->szText, strTipText,
               (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
  else
    lstrcpyn(pTTTW->szText, strTipText,
             (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif
  *pResult = 0;

  // bring the tooltip window above other popup windows
  ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
#endif

  return TRUE;    // message was handled
}



#if !defined(POCKET_PC)
void
DboxMain::OnDropFiles(HDROP hDrop)
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



BOOL
DboxMain::CheckExtension(const CMyString &name, const CMyString &ext) const
{
  int pos = name.Find(ext);
  return (pos == name.GetLength() - ext.GetLength()); //Is this at the end??
}

void
DboxMain::UpdateAlwaysOnTop()
{
#if !defined(POCKET_PC)
  CMenu*	sysMenu = GetSystemMenu( FALSE );

  if ( m_bAlwaysOnTop ) {
    SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    sysMenu->CheckMenuItem( ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_CHECKED );
  } else {
    SetWindowPos( &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    sysMenu->CheckMenuItem( ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_UNCHECKED );
  }
#endif
}

void 
DboxMain::OnSysCommand( UINT nID, LPARAM lParam )
{
#if !defined(POCKET_PC)
  m_LockDisabled = true;
  CDialog::OnSysCommand( nID, lParam );

  if ( ID_SYSMENU_ALWAYSONTOP == nID ) {
    m_bAlwaysOnTop = !m_bAlwaysOnTop;
    PWSprefs::GetInstance()->SetPref(PWSprefs::BoolPrefs::AlwaysOnTop,
                                     m_bAlwaysOnTop);
    UpdateAlwaysOnTop();
  }
  m_LockDisabled = false;
#endif
}


void
DboxMain::ConfigureSystemMenu()
{
#if defined(POCKET_PC)
  m_wndCommandBar = (CCeCommandBar*) m_pWndEmptyCB;
  m_wndMenu		= m_wndCommandBar->InsertMenuBar( IDR_MAINMENU );

  ASSERT( m_wndMenu != NULL );
#else
  CMenu*	sysMenu = GetSystemMenu( FALSE );
  CString	str;

  str.LoadString( IDS_ALWAYSONTOP );

  sysMenu->InsertMenu( 5, MF_BYPOSITION | MF_STRING, ID_SYSMENU_ALWAYSONTOP, (LPCTSTR)str );
#endif
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void
DboxMain::OnUpdateMRU(CCmdUI* pCmdUI) 
{
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

#if _MFC_VER > 1200
BOOL
#else
void 
#endif
DboxMain::OnOpenMRU(UINT nID)
{
  UINT	uMRUItem = nID - ID_FILE_MRU_ENTRY1;

  CString mruItem = (*app.GetMRU())[uMRUItem];

  Open( mruItem );
#if _MFC_VER > 1200
  return TRUE;
#endif
}


// Called just before any pulldown or popup menu is displayed, so that menu items
// can be enabled/disabled or checked/unchecked dynamically.
void DboxMain::OnInitMenu(CMenu* pMenu)
{
  const BOOL bTreeView = m_ctlItemTree.IsWindowVisible();
  const BOOL bItemSelected = SelItemOk();

  UINT uiItemCmdFlags = MF_BYCOMMAND | (bItemSelected ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_MENUITEM_COPYPASSWORD, uiItemCmdFlags);
  pMenu->EnableMenuItem(ID_MENUITEM_COPYUSERNAME, uiItemCmdFlags);
  pMenu->EnableMenuItem(ID_MENUITEM_EDIT, uiItemCmdFlags);
#if defined(POCKET_PC)
  pMenu->EnableMenuItem(ID_MENUITEM_SHOWPASSWORD, uiItemCmdFlags);
#endif
  pMenu->EnableMenuItem(ID_MENUITEM_AUTOTYPE, uiItemCmdFlags);
  pMenu->EnableMenuItem(ID_MENUITEM_DUPLICATEENTRY, uiItemCmdFlags);


  bool bGroupSelected = false;
  bool bEmptyGroupSelected = false;
  if (bTreeView) {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    bGroupSelected = (hi != NULL && !m_ctlItemTree.IsLeafNode(hi));
    bEmptyGroupSelected = (bGroupSelected && !m_ctlItemTree.ItemHasChildren(hi));
  }
  pMenu->EnableMenuItem(ID_MENUITEM_DELETE, ((bItemSelected || bEmptyGroupSelected) ? MF_ENABLED : MF_GRAYED));
  pMenu->EnableMenuItem(ID_MENUITEM_RENAME, ((bTreeView && (bItemSelected || bGroupSelected)) ? MF_ENABLED : MF_GRAYED));
  pMenu->EnableMenuItem(ID_MENUITEM_ADDGROUP, (bTreeView ? MF_ENABLED : MF_GRAYED));

  pMenu->CheckMenuRadioItem(ID_MENUITEM_LIST_VIEW, ID_MENUITEM_TREE_VIEW, 
                            (bTreeView ? ID_MENUITEM_TREE_VIEW : ID_MENUITEM_LIST_VIEW), MF_BYCOMMAND);


  CDC* pDC = this->GetDC();
  int NumBits = ( pDC ? pDC->GetDeviceCaps(12 /*BITSPIXEL*/) : 32 );
  // JHF m_toolbarMode is not for WinCE (as in .h)
#if !defined(POCKET_PC)
  if (NumBits < 16 && m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) {
    // Less that 16 color bits available, no choice, disable menu items
    pMenu->EnableMenuItem(ID_MENUITEM_NEW_TOOLBAR, MF_GRAYED | MF_BYCOMMAND);
    pMenu->EnableMenuItem(ID_MENUITEM_OLD_TOOLBAR, MF_GRAYED | MF_BYCOMMAND);
  } else {
    // High-color screen mode so all choices available.
    // (or a low-color screen, but leave choices enabled so that the user still can switch.)
    pMenu->EnableMenuItem(ID_MENUITEM_NEW_TOOLBAR, MF_ENABLED | MF_BYCOMMAND);
    pMenu->EnableMenuItem(ID_MENUITEM_OLD_TOOLBAR, MF_ENABLED | MF_BYCOMMAND);
  }

  pMenu->CheckMenuRadioItem(ID_MENUITEM_NEW_TOOLBAR, ID_MENUITEM_OLD_TOOLBAR, 
                            m_toolbarMode, MF_BYCOMMAND);
#endif


  pMenu->EnableMenuItem(ID_MENUITEM_SAVE,
                        m_core.IsChanged() ? MF_ENABLED : MF_GRAYED);

  // enable/disable w.r.t read-only mode
  // is handled via ON_UPDATE_COMMAND_UI/OnUpdateROCommand
}


// helps with MRU by allowing ON_UPDATE_COMMAND_UI
void
DboxMain::OnInitMenuPopup(CMenu* pPopupMenu, UINT, BOOL) 
{
  // http://www4.ncsu.edu:8030/~jgbishop/codetips/dialog/updatecommandui_menu.html
  // This code comes from the MFC Documentation, and is adapted from CFrameWnd::OnInitMenuPopup() in WinFrm.cpp.
  ASSERT(pPopupMenu != NULL);
  CCmdUI state; // Check the enabled state of various menu items
  state.m_pMenu = pPopupMenu;
  ASSERT(state.m_pOther == NULL);
  ASSERT(state.m_pParentMenu == NULL);
    
  // Is the menu in question a popup in the top-level menu? If so, set m_pOther
  // to this menu. Note that m_pParentMenu == NULL indicates that the menu is a
  // secondary popup.
  CMenu *hParentMenu;
  if(AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu) {
    state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
  } else if((hParentMenu = this->GetMenu()) != NULL) {
    CWnd* pParent = this;
    // Child windows don't have menus--need to go to the top!
    if(pParent != NULL && (hParentMenu = pParent->GetMenu()) != NULL) {
      int nIndexMax = hParentMenu->GetMenuItemCount();
      for (int nIndex = 0; nIndex < nIndexMax; nIndex++) {
        CMenu *submenu = hParentMenu->GetSubMenu(nIndex);
        if(submenu != NULL && submenu->m_hMenu == pPopupMenu->m_hMenu) {
          // When popup is found, m_pParentMenu is containing menu.
          state.m_pParentMenu = CMenu::FromHandle(hParentMenu->GetSafeHmenu());
          break;
        }
      }
    }
  }
    
  state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
  for(state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++) {
    state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
    if(state.m_nID == 0)
      continue; // Menu separator or invalid cmd - ignore it.
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pMenu != NULL);
    if(state.m_nID == (UINT)-1) {
      // Possibly a popup menu, route to first item of that popup.
      state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
      if(state.m_pSubMenu == NULL ||
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
    if(nCount < state.m_nIndexMax) {
      state.m_nIndex -= (state.m_nIndexMax - nCount);
      while(state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID) {
        state.m_nIndex++;
      }
    }
    state.m_nIndexMax = nCount;
  }
}

#if defined(POCKET_PC)
void DboxMain::OnShowPassword()
{
  if (SelItemOk() == TRUE) {
    CItemData			item;
    CMyString			password;
    CMyString			name;
    CMyString			title;
    CMyString			username;
    CShowPasswordDlg	pwDlg( this );

    item	= m_pwlist.GetAt( Find(getSelectedItem()) );

    item.GetPassword(password);
    item.GetName( name );

    SplitName( name, title, username );

    pwDlg.SetTitle( title );
    pwDlg.SetPassword( password );
    pwDlg.DoModal();
  }
}
#endif

LRESULT DboxMain::OnTrayNotification(WPARAM , LPARAM )
{
#if 0
  return m_TrayIcon.OnTrayNotification(wParam, lParam);
#else
  return 0;
#endif
}


void DboxMain::OnMinimize()
{
  ShowWindow(SW_MINIMIZE);
}

void DboxMain::OnUnMinimize()
{
  ShowWindow(SW_RESTORE);
}


void
DboxMain::startLockCheckTimer(){
  const UINT INTERVAL = 5000; // every 5 seconds should suffice

  if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::BoolPrefs::LockOnWindowLock )==TRUE){
    TRACE(_T("startLockCheckTimer: Starting timer\n"));
    SetTimer(TIMER_CHECKLOCK, INTERVAL, NULL);
  } else
    TRACE(_T("Not Starting\n"));
}

BOOL DboxMain::PreTranslateMessage(MSG* pMsg)
{
  // Do NOT pass the ESC along if preference EscExits is false.
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE &&
      !PWSprefs::GetInstance()->
      GetPref(PWSprefs::BoolPrefs::EscExits)) {
    return TRUE;
  }

  return CDialog::PreTranslateMessage(pMsg);
}

void DboxMain::ResetIdleLockCounter()
{
  m_IdleLockCountDown = PWSprefs::GetInstance()->
    GetPref(PWSprefs::IntPrefs::IdleTimeout);

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
      message == WM_VSCROLL
      )
    ResetIdleLockCounter();
  return CDialog::WindowProc(message, wParam, lParam);
}
