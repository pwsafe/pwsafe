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
  #include <errno.h>
  #include "resource.h"
#endif

// dialog boxen
#include "DboxMain.h"

#include "ClearQuestionDlg.h"
#include "FindDlg.h"
#include "PasskeyChangeDlg.h"
#include "PasskeyEntry.h"
#include "PasskeySetup.h"
#include "UsernameEntry.h"
#include "TryAgainDlg.h"
#include "ExportText.h"

// widget override?
#include "SysColStatic.h"

#ifdef POCKET_PC
  #include "pocketpc/PocketPC.h"
  #include "ShowPasswordDlg.h"
#endif

#include <afxpriv.h>
#include <stdlib.h> // for qsort

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
     m_OnEditDisabled(false), m_core(app.m_core)
{
	//{{AFX_DATA_INIT(DboxMain)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

   m_hIcon = app.LoadIcon(IDI_CORNERICON);
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

   m_bAlwaysOnTop = PWSprefs::GetInstance()->GetPref(PWSprefs::BoolPrefs::AlwaysOnTop);

   m_currbackup = // ??? move to PWScore??
     PWSprefs::GetInstance()->GetPref(PWSprefs::StringPrefs::CurrentBackup);

   m_bShowPasswordInEdit = false;
   m_bShowPasswordInList = false;
   m_bSortAscending = true;
   m_iSortedColumn = 0;
}

BEGIN_MESSAGE_MAP(DboxMain, CDialog)
	//{{AFX_MSG_MAP(DboxMain)
   ON_WM_DESTROY()
   ON_WM_SIZE()
   ON_COMMAND(ID_MENUITEM_ABOUT, OnAbout)
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
   ON_COMMAND(ID_MENUITEM_RESTORE, OnRestore)
   ON_COMMAND(ID_MENUTIME_SAVEAS, OnSaveAs)
   ON_COMMAND(ID_MENUITEM_BACKUPSAFE, OnBackupSafe)
   ON_COMMAND(ID_MENUITEM_CHANGECOMBO, OnPasswordChange)
   ON_COMMAND(ID_MENUITEM_CLEARCLIPBOARD, OnClearclipboard)
   ON_COMMAND(ID_MENUITEM_DELETE, OnDelete)
   ON_COMMAND(ID_MENUITEM_EDIT, OnEdit)
   ON_COMMAND(ID_MENUITEM_FIND, OnFind)
   ON_COMMAND(ID_MENUITEM_OPTIONS, OnOptions)
   ON_COMMAND(ID_MENUITEM_SAVE, OnSave)
   ON_COMMAND(ID_MENUITEM_LIST_VIEW, OnListView)
   ON_COMMAND(ID_MENUITEM_TREE_VIEW, OnTreeView)
   ON_COMMAND(ID_MENUITEM_OLD_TOOLBAR, OnOldToolbar)
   ON_COMMAND(ID_MENUITEM_NEW_TOOLBAR, OnNewToolbar)
   ON_COMMAND(ID_FILE_EXPORTTO_OLD1XFORMAT, OnExportV17)
   ON_COMMAND(ID_FILE_EXPORTTO_PLAINTEXT, OnExportText)
   ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
   ON_COMMAND(ID_MENUITEM_ADDGROUP, OnAddGroup)
   ON_WM_TIMER()
   ON_COMMAND(ID_MENUITEM_AUTOTYPE, OnAutoType)
#if defined(POCKET_PC)
   ON_COMMAND(ID_MENUITEM_SHOWPASSWORD, OnShowPassword)
#else
	ON_NOTIFY(NM_SETFOCUS, IDC_ITEMLIST, OnSetfocusItemlist)
	ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMLIST, OnKillfocusItemlist)
   ON_WM_DROPFILES()
#endif
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ITEMLIST, OnColumnClick)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_ENTRY1, OnUpdateMRU)
	ON_WM_INITMENUPOPUP()
   ON_COMMAND(ID_MENUITEM_EXIT, OnOK)
  ON_COMMAND(ID_MENUITEM_MINIMIZE, OnMinimize)
  ON_COMMAND(ID_MENUITEM_UNMINIMIZE, OnUnMinimize)
#if !defined(POCKET_PC)
   ON_COMMAND(ID_TOOLBUTTON_ADD, OnAdd)
   ON_COMMAND(ID_TOOLBUTTON_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_TOOLBUTTON_COPYUSERNAME, OnCopyUsername)
   ON_COMMAND(ID_TOOLBUTTON_CLEARCLIPBOARD, OnClearclipboard)
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

	ON_COMMAND_EX_RANGE(ID_FILE_MRU_ENTRY1, ID_FILE_MRU_ENTRY20, OnOpenMRU)
#if !defined(POCKET_PC)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
#endif
END_MESSAGE_MAP()


BOOL
DboxMain::OnInitDialog()
{
  ConfigureSystemMenu();

  CDialog::OnInitDialog();

  UpdateAlwaysOnTop();

  if (OpenOnInit()==FALSE) // If this function fails, abort launch
    return TRUE;

  m_windowok = true;
	
  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog

  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIcon, FALSE); // Set small icon
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

  CMenu* mmenu = GetMenu();
  CMenu* submenu = mmenu->GetSubMenu(2);

  const CString lastView = PWSprefs::GetInstance()->
    GetPref(PWSprefs::StringPrefs::LastView);

  if (lastView == _T("list")) {
    submenu->CheckMenuItem(ID_MENUITEM_LIST_VIEW, MF_CHECKED | MF_BYCOMMAND);
    submenu->CheckMenuItem(ID_MENUITEM_TREE_VIEW, MF_UNCHECKED | MF_BYCOMMAND);
  } else {
    submenu->CheckMenuItem(ID_MENUITEM_LIST_VIEW, MF_UNCHECKED | MF_BYCOMMAND);
    submenu->CheckMenuItem(ID_MENUITEM_TREE_VIEW, MF_CHECKED | MF_BYCOMMAND);
    m_ctlItemList.ShowWindow(SW_HIDE);
    m_ctlItemTree.ShowWindow(SW_SHOW);
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
    MoveWindow(&rect, TRUE);
  }
#endif

  m_core.SetUseDefUser(PWSprefs::GetInstance()->
		       GetPref(PWSprefs::BoolPrefs::UseDefUser));
  m_core.SetDefUsername(PWSprefs::GetInstance()->
		       GetPref(PWSprefs::StringPrefs::DefUserName));

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
   int rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, true);
   int rc2 = PWScore::NOT_SUCCESS;

   switch (rc)
   {
   case PWScore::SUCCESS:
      rc2 = m_core.ReadCurFile(passkey);
#if !defined(POCKET_PC)
      m_title = "Password Safe - " + m_core.GetCurFile();
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

   if (rc2 == PWScore::SUCCESS)
   {
      m_existingrestore = FALSE;
      m_needsreading = false;
	  startLockCheckTimer();
      return TRUE;
   }
   else
   {
      app.m_pMainWnd = NULL;
      CDialog::OnCancel();
      return FALSE;
   }
}


void
DboxMain::OnDestroy()
{
   //WinHelp(0L, HELP_QUIT);
   CDialog::OnDestroy();
}



void
DboxMain::OnItemDoubleClick( NMHDR *, LRESULT *)
{
#if defined(POCKET_PC)
	if ( app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dcshowspassword"), FALSE) == FALSE )
	{
		OnCopyPassword();
	}
	else
	{
		OnShowPassword();
	}
#else
	OnCopyPassword();
#endif
}


void DboxMain::OnBrowse()
{
  HINSTANCE stat = ::ShellExecute(NULL, NULL, m_BrowseURL,
				  NULL, _T("."), SW_SHOWNORMAL);
  if (int(stat) < 32) {
    AfxMessageBox("oops");
  }
}

void
DboxMain::OnCopyPassword() 
{
	bool	bCopyPassword = true;	// will get set to false if user hits cancel

	if (!SelItemOk()) 
		return;

	//Remind the user about clipboard security
	CClearQuestionDlg clearDlg(this);
	if (clearDlg.m_dontaskquestion == FALSE)
	{
		int rc = clearDlg.DoModal();
		if (rc == IDOK)
		{
		}
		else if (rc == IDCANCEL)
		{
			bCopyPassword = false;
		}
	}

	if ( !bCopyPassword )
		return;

	CItemData *ci = getSelectedItem();
	ASSERT(ci != NULL);
	CMyString curPassString = ci->GetPassword();

		
	uGlobalMemSize = (curPassString.GetLength() + 1) * sizeof(TCHAR);
	hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, uGlobalMemSize);
	// {kjp} fix to use UNICODE safe string definitions and string copy functions
	LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);
		
	strCopy( pGlobalLock, curPassString );
		
	GlobalUnlock(hGlobalMemory);	
		
	if (OpenClipboard() == TRUE) {
		if (EmptyClipboard()!=TRUE)
			AfxMessageBox(_T("The clipboard was not emptied correctly"));
		if (SetClipboardData(CLIPBOARD_TEXT_FORMAT, hGlobalMemory) == NULL)
			AfxMessageBox(_T("The data was not pasted into the clipboard correctly"));
		if (CloseClipboard() != TRUE)
			AfxMessageBox(_T("The clipboard could not be closed"));
	} else {
		AfxMessageBox(_T("The clipboard could not be opened correctly"));
	}
}

void
DboxMain::OnFind() 
{
  CFindDlg::Doit(this); // create modeless or popup existing
  // XXX Gross hack to fix aesthetic bug in tree view
  // without this, multiple "selected" displayed
  // if treeview && there's a selected item, then
#if 0
      m_ctlItemTree.SetItemState(di->tree_item,
				 TVIS_SELECTED,
				 TVIS_DROPHILITED | TVIS_SELECTED);
#endif
}



void
DboxMain::ClearClipboard()
{
   if (OpenClipboard() != TRUE)
      AfxMessageBox(_T("The clipboard could not be opened correctly"));

   if (IsClipboardFormatAvailable(CLIPBOARD_TEXT_FORMAT) != 0)
   {
      HGLOBAL hglb = GetClipboardData(CLIPBOARD_TEXT_FORMAT); 
      if (hglb != NULL)
      {
         LPTSTR lptstr = (LPTSTR)GlobalLock(hglb); 
         if (lptstr != NULL)
         {
			trashMemory( lptstr, strLength(lptstr) );
            GlobalUnlock(hglb); 
         } 
      } 
   }
   if (EmptyClipboard()!=TRUE)
      AfxMessageBox(_T("The clipboard was not emptied correctly"));
   if (CloseClipboard() != TRUE)
      AfxMessageBox(_T("The clipboard could not be closed"));
}


void
DboxMain::OnPasswordChange() 
{
   CPasskeyChangeDlg changeDlg(this);
   int rc = changeDlg.DoModal();
   if (rc == IDOK)
   {
     m_core.ChangePassword(changeDlg.m_newpasskey);
   }
   else if (rc == IDCANCEL)
   {
   }
}


void
DboxMain::OnClearclipboard() 
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
DboxMain::OnSave() 
{
   Save();
}

void
DboxMain::OnExportV17()
{
  int rc;
  CMyString newfile;

  //SaveAs-type dialog box
  while (1)
    {
      CFileDialog fd(FALSE,
                     "dat",
                     m_core.GetCurFile(),
                     OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                     |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                     "Password Safe Databases (*.dat)|*.dat|"
                     "All files (*.*)|*.*|"
                     "|",
                     this);
      fd.m_ofn.lpstrTitle =
	_T("Please name the exported database");
      rc = fd.DoModal();
      if (rc == IDOK)
	{
	  newfile = (CMyString)fd.GetPathName();
	  break;
	}
      else
	return;
    }

  rc = m_core.WriteV17File(newfile);
  if (rc == PWScore::CANT_OPEN_FILE)
    {
      CMyString temp = newfile + _T("\n\nCould not open file for writing!");
      MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
    }
}

void
DboxMain::OnExportText()
{
  CExportTextDlg et;
  int rc = et.DoModal();
  if (rc == IDOK) {
    CMyString newfile;
    CMyString pw(et.m_exportTextPassword);
    if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
      // do the export
      //SaveAs-type dialog box
      while (1) {
	CFileDialog fd(FALSE,
		       "txt",
		       "",
		       OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
		       |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
		       "Text files (*.txt)|*.txt|"
		       "CSV files (*.csv)|*.csv|"
		       "All files (*.*)|*.*|"
		       "|",
		       this);
	fd.m_ofn.lpstrTitle =
	  _T("Please name the plaintext file");
	rc = fd.DoModal();
	if (rc == IDOK) {
	  newfile = (CMyString)fd.GetPathName();
	  break;
	} else
	  return;
      } // while (1)
      rc = m_core.WritePlaintextFile(newfile);
      if (rc == PWScore::CANT_OPEN_FILE)
	{
	  CMyString temp = newfile + _T("\n\nCould not open file for writing!");
	  MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
	}
    } else {
      MessageBox(_T("Passkey incorrect"), _T("Error"));
      Sleep(3000); // protect against automatic attacks
    }
  }
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

   if (m_core.GetReadFileVersion() == PWSfile::V17) {
     CString OldName(m_core.GetCurFile());
     int dotIndex = OldName.ReverseFind(TCHAR('.'));
     if (dotIndex != -1)
       OldName = OldName.Left(dotIndex);
     OldName += _T(".old");


     CString msg = _T("The original database, \"");
     msg += CString(m_core.GetCurFile());
     msg += _T("\", is in pre-2.0 format."
	       " It will be unchanged, and renamed to \"");
     msg += OldName;
     msg += _T("\"\nYour changes will be written in the new"
	       " format, which is unusable by old versions of PasswordSafe."
	       " To save your changes in the old format, use the \"File->Export To"
	       "-> Old (1.x) format\" command.\n\n"
	       "Press OK to continue saving, or Cancel to stop.");
     if (MessageBox(msg, _T("File version warning"),
		    MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
       return PWScore::USER_CANCEL;
     if (m_core.RenameFile(m_core.GetCurFile(), OldName) != PWScore::SUCCESS) {
       MessageBox(_T("Could not rename file"), _T("File rename error"),
		  MB_OK|MB_ICONWARNING);
       return PWScore::CANT_OPEN_FILE;
     }
   }

   rc = m_core.WriteCurFile();

   if (rc == PWScore::CANT_OPEN_FILE)
   {
      CMyString temp = m_core.GetCurFile() + "\n\nCould not open file for writing!";
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
   dbox.DoModal();
}


void
DboxMain::OnCopyUsername() 
{
   if (SelItemOk() != TRUE)
      return;

   CItemData *ci = getSelectedItem();
   ASSERT(ci != NULL);
   CMyString username = ci->GetUser();

   if (username.IsEmpty())
   {
      AfxMessageBox(_T("There is no username associated with this item."));
   }
   else
   {
      uGlobalMemSize = (username.GetLength() + 1) * sizeof(TCHAR);
      hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                                  uGlobalMemSize);
      LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);
	  strCopy( pGlobalLock, username );
      GlobalUnlock(hGlobalMemory);	
		
      if (OpenClipboard() == TRUE)
      {
         if (EmptyClipboard()!=TRUE)
            AfxMessageBox(_T("The clipboard was not emptied correctly"));
         if (SetClipboardData(CLIPBOARD_TEXT_FORMAT, hGlobalMemory) == NULL)
            AfxMessageBox(_T("The data was not pasted into the clipboard correctly"));
         if (CloseClipboard() != TRUE)
            AfxMessageBox(_T("The clipboard could not be closed"));
      }
      else
      {
         AfxMessageBox(_T("The clipboard could not be opened correctly"));
      }
      //No need to remind the user about clipboard security
      //as this is only a username
   }
}


void
DboxMain::OnBackupSafe() 
{
   BackupSafe();
}


int
DboxMain::BackupSafe()
{
   int rc;
   CMyString tempname;

   //SaveAs-type dialog box
   while (1)
   {
      CFileDialog fd(FALSE,
                     _T("bak"),
                     m_currbackup,
                     OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                     | OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                     _T("Password Safe Backups (*.bak)|*.bak||"),
                     this);
      fd.m_ofn.lpstrTitle = _T("Please Choose a Name for this Backup:");

      rc = fd.DoModal();
      if (rc == IDOK)
      {
         tempname = (CMyString)fd.GetPathName();
         break;
      }
      else
         return PWScore::USER_CANCEL;
   }


   rc = m_core.WriteFile(tempname);
   if (rc == PWScore::CANT_OPEN_FILE)
   {
      CMyString temp = tempname + _T("\n\nCould not open file for writing!");
      MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      return PWScore::CANT_OPEN_FILE;
   }

   m_currbackup = tempname;
   return PWScore::SUCCESS;
}


void
DboxMain::OnOpen() 
{
   Open();
}


int
DboxMain::Open()
{
   int rc = PWScore::SUCCESS;
   CMyString newfile;

   //Open-type dialog box
   while (1)
   {
      CFileDialog fd(TRUE,
                     _T("dat"),
                     NULL,
                     OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                     _T("Password Safe Databases (*.dat)|*.dat|")
                     _T("Password Safe Backups (*.bak)|*.bak|")
                     _T("All files (*.*)|*.*|")
                     _T("|"),
                     this);
      fd.m_ofn.lpstrTitle = _T("Please Choose a Database to Open:");
      rc = fd.DoModal();
      if (rc == IDOK)
      {
         newfile = (CMyString)fd.GetPathName();

		 rc = Open( newfile );

		 if ( rc == PWScore::SUCCESS )
	         break;
      }
      else
         return PWScore::USER_CANCEL;
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
	
	if (m_core.IsChanged())
	{
		int rc2;
		
		temp =
		  _T("Do you want to save changes to the password database: ")
		  + m_core.GetCurFile()
			+ _T("?");
		rc = MessageBox(temp,
			AfxGetAppName(),
			MB_ICONQUESTION|MB_YESNOCANCEL);
		switch (rc)
		{
		case IDCANCEL:
			return PWScore::USER_CANCEL;
		case IDYES:
			rc2 = Save();
			// Make sure that writting the file was successful
			if (rc2 == PWScore::SUCCESS)
				break;
			else
				return PWScore::CANT_OPEN_FILE;
		case IDNO:
			break;
		}
	}
	
	rc = GetAndCheckPassword(pszFilename, passkey);
	switch (rc)
	{
	case PWScore::SUCCESS:
		app.GetMRU()->Add(pszFilename);
		break; // Keep going... 
	case PWScore::CANT_OPEN_FILE:
		temp = m_core.GetCurFile()
		  + "\n\nCan't open file. Please choose another.";
		MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
	case TAR_OPEN:
		return Open();
	case TAR_NEW:
		return New();
	case PWScore::WRONG_PASSWORD:
	/*
	If the user just cancelled out of the password dialog, 
	assume they want to return to where they were before... 
		*/
		return PWScore::USER_CANCEL;
	}
	
	rc = m_core.ReadFile(pszFilename, passkey);
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
	
	m_core.SetCurFile(pszFilename);
#if !defined(POCKET_PC)
	m_title = _T("Password Safe - ") + m_core.GetCurFile();
#endif
	ChangeOkUpdate();
	RefreshList();
	
	return PWScore::SUCCESS;
}

void
DboxMain::OnNew()
{
   New();
}


int
DboxMain::New() 
{
   int rc, rc2;

   if (m_core.IsChanged())
   {
      CMyString temp =
	_T("Do you want to save changes to the password database: ")
	+ m_core.GetCurFile()
         + _T("?");

      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
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

   m_core.SetCurFile(""); //Force a save as... 
#if !defined(POCKET_PC)
   m_title = _T("Password Safe - <Untitled>");
#endif
   ChangeOkUpdate();

   return PWScore::SUCCESS;
}


void
DboxMain::OnRestore()
{
   Restore();
}


int
DboxMain::Restore() 
{
   int rc;
   CMyString newback, passkey, temp;

   //Open-type dialog box
   while (1)
   {
      CFileDialog fd(TRUE,
                     _T("bak"),
                     m_currbackup,
                     OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                     _T("Password Safe Backups (*.bak)|*.bak||"),
                     this);
      fd.m_ofn.lpstrTitle = _T("Please Choose a Backup to Restore:");
      rc = fd.DoModal();
      if (rc == IDOK)
      {
         newback = (CMyString)fd.GetPathName();
         break;
      }
      else
         return PWScore::USER_CANCEL;
   }

   rc = GetAndCheckPassword(newback, passkey);
   switch (rc)
   {
   case PWScore::SUCCESS:
      break; // Keep going... 
   case PWScore::CANT_OPEN_FILE:
      temp =
	m_core.GetCurFile()
	+ _T("\n\nCan't open file. Please choose another.");
      MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
   case TAR_OPEN:
      return Open();
   case TAR_NEW:
      return New();
   case PWScore::WRONG_PASSWORD:
      /*
        If the user just cancelled out of the password dialog, 
        assume they want to return to where they were before... 
      */
      return PWScore::USER_CANCEL;
   }

   if (m_core.IsChanged())
   {
      int rc2;
	
      temp = _T("Do you want to save changes to the password list: ")
         + m_core.GetCurFile() + _T("?");

      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return PWScore::USER_CANCEL;
      case IDYES:
         rc2 = Save();
         //Make sure that writting the file was successful
         if (rc2 == PWScore::SUCCESS)
            break;
         else
            return PWScore::CANT_OPEN_FILE;
      case IDNO:
         break;
      }
   }

   rc = m_core.ReadFile(newback, passkey);
   if (rc == PWScore::CANT_OPEN_FILE)
   {
      temp = newback + _T("\n\nCould not open file for reading!");
      MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
      //Everything stays as is... Worst case, they saved their file....
      return PWScore::CANT_OPEN_FILE;
   }
	
   m_core.SetCurFile(""); //Force a save as...
   m_core.SetChanged(true); //So that the *.dat version of the file will be saved.
#if !defined(POCKET_PC)
   m_title = _T("Password Safe - <Untitled Restored Backup>");
#endif
   ChangeOkUpdate();
   RefreshList();

   return PWScore::SUCCESS;
}


void
DboxMain::OnSaveAs()
{
   SaveAs();
}


int
DboxMain::SaveAs() 
{
   int rc;
   CMyString newfile;

   if (m_core.GetReadFileVersion() == PWSfile::V17) {
     CMyString msg = _T("The original database, \"");
     msg += m_core.GetCurFile();
     msg += _T("\", is in pre-2.0 format. The data will now be written in the new"
	       " format, which is unusable by old versions of PasswordSafe."
	       " To save the data in the old format, use the \"File->Export To"
	       "-> Old (1.x) format\" command.\n\n"
	       "Press OK to continue saving, or Cancel to stop.");
     if (MessageBox(msg, _T("File version warning"),
		    MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
       return PWScore::USER_CANCEL;
   }
   //SaveAs-type dialog box
   while (1)
   {
      CFileDialog fd(FALSE,
                     _T("dat"),
                     m_core.GetCurFile(),
                     OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                     |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                     _T("Password Safe Databases (*.dat)|*.dat|")
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
      if (rc == IDOK)
      {
         newfile = (CMyString)fd.GetPathName();
         break;
      }
      else
         return PWScore::USER_CANCEL;
   }

   rc = m_core.WriteFile(newfile);
   if (rc == PWScore::CANT_OPEN_FILE)
   {
      CMyString temp = newfile + _T("\n\nCould not open file for writing!");
      MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      return PWScore::CANT_OPEN_FILE;
   }

   m_core.SetCurFile(newfile);
#if !defined(POCKET_PC)
   m_title = _T("Password Safe - ") + m_core.GetCurFile();
#endif
   ChangeOkUpdate();

   app.GetMRU()->Add( newfile );

   return PWScore::SUCCESS;
}

int
DboxMain::GetAndCheckPassword(const CMyString &filename,
			      CMyString& passkey,
			      bool first)
{
  // Called for an existing database. promt user
  // for password, verify against file
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

  CPasskeyEntry dbox_pkentry(this, filename, first);
  int rc = dbox_pkentry.DoModal();

  if (rc == IDOK)
    {
      DBGMSG("PasskeyEntry returns IDOK\n");
      passkey = dbox_pkentry.GetPasskey();
      retval = PWScore::SUCCESS;
    }
  else /*if (rc==IDCANCEL) */ //Determine reason for cancel
    {
      int cancelreturn = dbox_pkentry.GetStatus();
      switch (cancelreturn)
	{
	case TAR_OPEN:
	case TAR_NEW:
	  DBGMSG("PasskeyEntry TAR_OPEN or TAR_NEW\n");
	  retval = cancelreturn; //Return either open or new flag... 
	  break;
	default:
	  DBGMSG("Default to WRONG_PASSWORD\n");
	  retval = PWScore::WRONG_PASSWORD;	//Just a normal cancel
	  break;
	}
    }

  app.m_pMainWnd = NULL; // done with dbox_pkentry

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
       pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
   {
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

   MessageBox(_T("go away you silly git"), _T("File drop"), MB_OK);

#if 0
   // here's what we really want - sorta
   HDROP m_hDropInfo = hDropInfo;        
   CString Filename;

   if (m_hDropInfo)
   {
      int iFiles = DragQueryFile(m_hDropInfo, (UINT)-1, NULL, 0);
      for (int i=0; i<ifiles; i++)
      {
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

	if ( m_bAlwaysOnTop )
	{
		SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		sysMenu->CheckMenuItem( ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_CHECKED );
	}
	else
	{
		SetWindowPos( &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		sysMenu->CheckMenuItem( ID_SYSMENU_ALWAYSONTOP, MF_BYCOMMAND | MF_UNCHECKED );
	}
#endif
}

void 
DboxMain::OnSysCommand( UINT nID, LPARAM lParam )
{
#if !defined(POCKET_PC)
	CDialog::OnSysCommand( nID, lParam );

	if ( ID_SYSMENU_ALWAYSONTOP == nID )
	{
	  m_bAlwaysOnTop = !m_bAlwaysOnTop;
	  PWSprefs::GetInstance()->SetPref(PWSprefs::BoolPrefs::AlwaysOnTop,
					   m_bAlwaysOnTop);
	  UpdateAlwaysOnTop();
	}
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
	app.GetMRU()->UpdateMenu( pCmdUI );	
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
	
#ifdef POCKET_PC
	CMenu *hParentMenu;
#else
	HMENU hParentMenu;
#endif
	if(AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
#ifdef POCKET_PC
	else if((hParentMenu = this->GetMenu()) != NULL)
#else
	else if((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
#endif
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
#ifdef POCKET_PC
		if(pParent != NULL && (hParentMenu = pParent->GetMenu()) != NULL)
#else
		if(pParent != NULL && (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
#endif
		{
#ifdef POCKET_PC
			int nIndexMax = hParentMenu->GetMenuItemCount();
#else
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
#endif
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
#ifdef POCKET_PC
				if(::GetSubMenu(hParentMenu->GetSafeHmenu(), nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu->GetSafeHmenu());
					break;
				}
#else
				if(::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
#endif
			}
		}
	}
	
	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for(state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if(state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.
		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if(state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if(state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue; // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE); // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}
		
		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if(nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while(state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}

#if defined(POCKET_PC)
void DboxMain::OnShowPassword()
{
	if (SelItemOk() == TRUE)
	{
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

LRESULT DboxMain::OnTrayNotification(WPARAM wParam, LPARAM lParam)
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
	UINT nTimer;
	TRACE("startLockCheckTimer\n");
	if (PWSprefs::GetInstance()->
	    GetPref(PWSprefs::BoolPrefs::LockOnWindowLock )==TRUE ){
	
		TRACE("Starting timer\n");
		nTimer=SetTimer(TIMER_CHECKLOCK,100,NULL);
		TRACE("Going %d\n",nTimer);
	}
	else
		TRACE("Not Starting\n");
}
