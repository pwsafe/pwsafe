/// \file DboxMain.cpp
//
// {kjp} PocketPC - minimize application when "OK" button top right corner is
// pressed.
//-----------------------------------------------------------------------------
#include "stdafx.h" // thomas
#include "PasswordSafe.h"

#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#include <aygshell.h>

#pragma message("Linker library-search : aygshell.lib")
#pragma comment(lib, "aygshell.lib")


#else
  #include "resource.h"
#endif

// dialog boxen
#include "DboxMain.h"

#include "ClearQuestionDlg.h"
#include "ConfirmDeleteDlg.h"
#include "AddDlg.h"
#include "EditDlg.h"
#include "FindDlg.h"
#include "PasskeyChangeDlg.h"
//#include "OptionsDlg.h"
#include "PasskeyEntry.h"
#include "PasskeySetup.h"
#include "RemindSaveDlg.h"
#include "QuerySetDef.h"
#include "UsernameEntry.h"
#include "TryAgainDlg.h"

// widget override?
#include "SysColStatic.h"

#ifdef POCKET_PC
  #include <stdio.h>
  #include "pocketpc/PocketPC.h"
#else
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <errno.h>
#endif
#include <afxpriv.h>
#include <stdlib.h> // for qsort

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

#if defined(POCKET_PC)
//   virtual BOOL OnInitDialog();
#endif

protected:
   DECLARE_MESSAGE_MAP()
};

// I don't think we need this, but...
BEGIN_MESSAGE_MAP(DboxAbout, super)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

  /*
   * Compare function used by m_ctlItemList.SortItems()
   * "The comparison function must return a negative value if the first item should precede 
   * the second, a positive value if the first item should follow the second, or zero if
   * the two items are equivalent."
   *
   * If sorting is by title (username) , username (title) is the secondary field if the
   * primary fields are identical.
   */
int CALLBACK DboxMain::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	// lParamSort determines which column is getting sorted:
	// 0 - title
	// 1 - user name
	// 2 - note
	// 3 - password
	const int	nRecurseFlag		= 500;		// added to the desired sort column when recursing
	bool		bAlreadyRecursed	= false;
	int			nSortColumn			= LOWORD(lParamSort);
	CItemData*	pLHS				= (CItemData *)lParam1;
	CItemData*	pRHS				= (CItemData *)lParam2;
	CMyString	title1, username1;
	CMyString	title2, username2;

	// if the sort column is really big, then we must be being called via recursion
	if ( nSortColumn >= nRecurseFlag )
	{
		bAlreadyRecursed = true;		// prevents further recursion
		nSortColumn -= nRecurseFlag;	// normalizes sort column
	}

	int iResult;
	switch(nSortColumn) {
	case 0:
		SplitName(pLHS->GetName(), title1, username1);
		SplitName(pRHS->GetName(), title2, username2);
		iResult = ((CString)title1).CompareNoCase(title2);
		if (iResult == 0 && !bAlreadyRecursed)
		  iResult = CompareFunc(lParam1, lParam2,
					MAKELPARAM(1 + nRecurseFlag, HIWORD(lParamSort)));	// making a recursed call, add nRecurseFlag
		break;
	case 1:
		SplitName(pLHS->GetName(), title1, username1);
		SplitName(pRHS->GetName(), title2, username2);
		iResult = ((CString)username1).CompareNoCase(username2);
		if (iResult == 0 && !bAlreadyRecursed)
		  iResult = CompareFunc(lParam1, lParam2,
					MAKELPARAM(0 + nRecurseFlag, HIWORD(lParamSort)));	// making a recursed call, add nRecurseFlag
		break;
	case 2:
		iResult = ((CString)pLHS->GetNotes()).CompareNoCase(pRHS->GetNotes());
		break;
	case 3:
		iResult = ((CString)pLHS->GetPassword()).CompareNoCase(pRHS->GetPassword());
		break;
	default:
        iResult = 0; // should never happen - just keep compiler happy
		ASSERT(FALSE);
	}
	bool bSortAscending = HIWORD(lParamSort)? true: false;
	if (!bSortAscending) {
		iResult *= -1;
	}

	return iResult;
}

//-----------------------------------------------------------------------------
DboxMain::DboxMain(CWnd* pParent)
   : CDialog(DboxMain::IDD, pParent),
   m_bSizing( false )
{
	//{{AFX_DATA_INIT(DboxMain)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

   m_hwndMb = NULL;
   m_hIcon = app.LoadIcon(IDI_CORNERICON);
   m_pwlist.RemoveAll();
   // m_pwdb.Clear(); when the new backend is in...

   /*
     currently - there's a string in the resource string table, with the
     name of the default output file.  We pull it and concatenate the 
     current directory to make a default password database filename
     (which I think is the only usage of m_curdir) {jpr}
   */

   //CString temp;
   //temp.LoadString(IDS_OUTPUTFILE);
   //CString temp2 = app.m_curdir + temp;
   //m_deffile = (CMyString) ".\\pwsafe.dat"; //temp2;

   /*
    * current file and current backup file specs are stored in registry
    * Note that if m_currfile is non-empty, we will not read the registry value.
    * This will happen if a filename was given in the command line.
    */
   if (m_currfile.IsEmpty()) {
     // If there's no registry key, this is probably a fresh install.
     // CheckPassword will catch this and handle it correctly
     m_currfile = (CMyString) app.GetProfileString(_T(PWS_REG_OPTIONS), _T("currentfile"));
   }
   m_currbackup = (CMyString) app.GetProfileString(_T(PWS_REG_OPTIONS), _T("currentbackup"), NULL);

#if !defined(POCKET_PC)
   m_title = "";
#endif

   m_bAlwaysOnTop = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("alwaysontop"), FALSE);

   m_changed = FALSE;
   m_needsreading = TRUE;
   m_windowok = false;
   m_existingrestore = FALSE;

#if !defined(POCKET_PC)
   m_toolbarsSetup = FALSE;
#endif

   m_bShowPasswordInEdit = false;
   m_bShowPasswordInList = false;
   m_bSortAscending = true;
   m_iSortedColumn = 0;
}


void
DboxMain::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DboxMain)
	DDX_Control(pDX, IDC_ITEMLIST, m_ctlItemList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DboxMain, CDialog)
	//{{AFX_MSG_MAP(DboxMain)
   ON_WM_DESTROY()
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_WM_SIZE()
   ON_COMMAND(ID_MENUITEM_ABOUT, OnAbout)
   ON_COMMAND(ID_MENUITEM_COPYUSERNAME, OnCopyUsername)
#if defined(POCKET_PC)
   ON_WM_CREATE()
#else
   ON_WM_CONTEXTMENU()
#endif
   ON_NOTIFY(LVN_KEYDOWN, IDC_ITEMLIST, OnKeydownItemlist)
	ON_NOTIFY(NM_DBLCLK, IDC_ITEMLIST, OnListDoubleClick)
   ON_COMMAND(ID_MENUITEM_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_MENUITEM_NEW, OnNew)
   ON_COMMAND(ID_MENUITEM_OPEN, OnOpen)
   ON_COMMAND(ID_MENUITEM_RESTORE, OnRestore)
   ON_COMMAND(ID_MENUTIME_SAVEAS, OnSaveAs)
   ON_COMMAND(ID_MENUITEM_BACKUPSAFE, OnBackupSafe)
   ON_COMMAND(ID_MENUITEM_UPDATEBACKUPS, OnUpdateBackups)
   ON_COMMAND(ID_MENUITEM_CHANGECOMBO, OnPasswordChange)
   ON_COMMAND(ID_MENUITEM_CLEARCLIPBOARD, OnClearclipboard)
   ON_COMMAND(ID_MENUITEM_DELETE, OnDelete)
   ON_COMMAND(ID_MENUITEM_EDIT, OnEdit)
   ON_COMMAND(ID_MENUITEM_FIND, OnFind)
   ON_COMMAND(ID_MENUITEM_OPTIONS, OnOptions)
   ON_COMMAND(ID_MENUITEM_SAVE, OnSave)
   ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
#if !defined(POCKET_PC)
	ON_NOTIFY(NM_SETFOCUS, IDC_ITEMLIST, OnSetfocusItemlist)
	ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMLIST, OnKillfocusItemlist)
   ON_WM_DROPFILES()
#endif
   ON_NOTIFY(LVN_COLUMNCLICK, IDC_ITEMLIST, OnColumnClick)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_ENTRY1, OnUpdateMRU)
	ON_WM_INITMENUPOPUP()
   ON_COMMAND(ID_MENUITEM_EXIT, OnOK)
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

	m_ctlItemList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	int iColumnCount = 3;
	m_ctlItemList.InsertColumn(0, _T("Title"));
	m_ctlItemList.InsertColumn(1, _T("User Name"));
	m_ctlItemList.InsertColumn(2, _T("Notes"));

	if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwdefault"), FALSE)) {
		m_bShowPasswordInEdit = true;
	}

	if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwinlist"), FALSE)) {
		m_bShowPasswordInList = true;
	}

	CRect rect;
	m_ctlItemList.GetClientRect(&rect);
	int i1stWidth = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column1width"), rect.Width() / iColumnCount + rect.Width() % iColumnCount);
	int i2ndWidth = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column2width"), rect.Width() / iColumnCount);
	int i3rdWidth = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column3width"), rect.Width() / iColumnCount);

	m_ctlItemList.SetColumnWidth(0, i1stWidth);
	m_ctlItemList.SetColumnWidth(1, i2ndWidth);
	m_ctlItemList.SetColumnWidth(2, i3rdWidth);

	m_iSortedColumn = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("sortedcolumn"), 0);
	m_bSortAscending = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("sortascending"), 1)? true: false;

	// refresh list will add and size password column if necessary...
	RefreshList();

   ChangeOkUpdate();

   if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("donebackupchange"), FALSE) == FALSE)
      OnUpdateBackups();

#if defined(POCKET_PC)
   ::DrawMenuBar(m_hwndMb);
#endif

   setupBars(); // Just to keep things a little bit cleaner

#if !defined(POCKET_PC)
   // {kjp} Can't drag and drop files onto an application in PocketPC
   DragAcceptFiles(TRUE);

   // TODO: kinda hideous in the registry, encode as single string maybe?
   // {kjp} meaningless when target is a PocketPC device.
   rect.top = app.GetProfileInt(_TPWS_REG_POSITION), _T("top"), -1);
   rect.bottom = app.GetProfileInt(_TPWS_REG_POSITION), _T("bottom"), -1);
   rect.left = app.GetProfileInt(_TPWS_REG_POSITION), _T("left"), -1);
   rect.right = app.GetProfileInt(_TPWS_REG_POSITION), _T("right"), -1);

   if (rect.top == -1 || rect.bottom == -1 || rect.left == -1 || rect.right == -1) {
	   GetWindowRect(&rect);
	   SendMessage(WM_SIZE, SIZE_RESTORED, MAKEWPARAM(rect.Width(), rect.Height()));
   }
   else {
		MoveWindow(&rect, TRUE);
   }
#endif

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
   int rc;
   int rc2;

   rc = CheckPassword(m_currfile, passkey, true);

   switch (rc)
   {
   case SUCCESS:
      rc2 = ReadFile(m_currfile, passkey);
#if !defined(POCKET_PC)
      m_title = "Password Safe - " + m_currfile;
#endif
      break; 
   case CANT_OPEN_FILE:
      /*
       * If it is the default filename, assume that this is the first time
       * that they are starting Password Safe and don't confusing them.
       */
#if 0
      if (m_currfile != m_deffile)
      {
         CMyString temp = m_currfile
            + "\n\nCannot open database. It likely does not exist."
            + "\nA new database will be created.";
         MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
      }
      else
      {
         // of course, this will be easier under DboxPasskeyFirst's control...

         //GetDlgItem(IDC_PASSKEY)

         // here's where I'll grey out the db entry, and make them hit the
         // button instead - this is for bug #3
      }
#endif
      // currently falls thru to...
   case TAR_NEW:
      rc2 = New();
      if (USER_CANCEL == rc2) {
         // somehow, get DboxPasskeyEntryFirst redisplayed...
	  }
      break;
   case TAR_OPEN:
      rc2 = Open();
      if (USER_CANCEL == rc2) {
         // somehow, get DboxPasskeyEntryFirst redisplayed...
	  }
      break;
   case WRONG_PASSWORD:
      rc2 = NOT_SUCCESS;
      break;
   default:
      rc2 = NOT_SUCCESS;
      break;
   }

   if (rc2 == SUCCESS)
   {
      m_needsreading = FALSE;
      m_existingrestore = FALSE;
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
DboxMain::setupBars()
{
#if !defined(POCKET_PC)
   // This code is copied from the DLGCBR32 example that comes with MFC

   const UINT statustext = IDS_STATMESSAGE;


   // Add the status bar
   if (m_statusBar.Create(this))
   {
      m_statusBar.SetIndicators(&statustext, 1);
      // Make a sunken or recessed border around the first pane
      m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
   }             

   // Add the ToolBar.
   if (!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_MAINBAR))
   {
      TRACE0("Failed to create toolbar\n");
      return;      // fail to create
   }

   // TODO: Remove this if you don't want tool tips or a resizeable toolbar
   m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle()
                            | CBRS_TOOLTIPS | CBRS_FLYBY);

   CRect rect;
   RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
   RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
   m_ctlItemList.MoveWindow(&rect, TRUE);

	// Set flag
   m_toolbarsSetup = TRUE;
#endif
}


void
DboxMain::OnDestroy()
{
   //WinHelp(0L, HELP_QUIT);
   CDialog::OnDestroy();
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void
DboxMain::OnPaint() 
{
   if (IsIconic())
   {
      CPaintDC dc(this); // device context for painting

#if !defined(POCKET_PC)
      SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
#endif

      // Center icon in client rectangle
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}


// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR
DboxMain::OnQueryDragIcon()
{
   return (HCURSOR) m_hIcon;
}


//Add an item
void
DboxMain::OnAdd() 
{
   CAddDlg dataDlg(this);
   if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE) == TRUE)
   {
      dataDlg.m_username = CMyString(app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T("")));
   }

   app.DisableAccelerator();
   int rc = dataDlg.DoModal();
   app.EnableAccelerator();
	
   if (rc == IDOK)
   {
      //Check if they wish to set a default username
      if ((app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE) == FALSE)
          && (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("querysetdef"), TRUE) == TRUE)
          && (dataDlg.m_username != ""))
      {
         CQuerySetDef defDlg(this);
         defDlg.m_message =
            "Would you like to set \""
            + (const CString&)dataDlg.m_username
            + "\" as your default username?\n\nIt would then automatically be "
	        + "put in the dialog each time you add a new item.  Also only"
	        + " non-default usernames will be displayed in the main window.";
         int rc2 = defDlg.DoModal();
         if (rc2 == IDOK)
         {
            app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), TRUE);
            app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("defusername"),
                                   dataDlg.m_username);
            DropDefUsernames(&m_pwlist, dataDlg.m_username);
            RefreshList();
         }
      }
      //Finish Check (Does that make any geographical sense?)
      CItemData temp;
      CMyString temptitle;
      MakeName(temptitle, dataDlg.m_title, dataDlg.m_username);
      temp.SetName(temptitle);
      temp.SetPassword(dataDlg.m_password);
      temp.SetNotes(dataDlg.m_notes);
      POSITION curPos = m_pwlist.AddTail(temp);
      int newpos = insertItem(m_pwlist.GetAt(curPos));
      SelectEntry(newpos);
      m_ctlItemList.SetFocus();
      if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), FALSE) == TRUE)
      {
         Save();
      }
      else
      {
         m_changed = TRUE;
      }
      ChangeOkUpdate();
   }
   else if (rc == IDCANCEL)
   {
   }
}

void
DboxMain::OnListDoubleClick( NMHDR *, LRESULT *)
{
	OnCopyPassword();
}

void
DboxMain::OnCopyPassword() 
{
   if (SelItemOk() == TRUE)
   {
      POSITION itemPos = Find(getSelectedItem());
		
      CMyString curPassString;
      m_pwlist.GetAt(itemPos).GetPassword(curPassString);

	  // {kjp} fix to use the real amount of storage used
      uGlobalMemSize = (curPassString.GetLength() + 1) * sizeof(TCHAR);
      hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, uGlobalMemSize);
#if 1
	  // {kjp} fix to use UNICODE safe string definitions and string copy functions
      LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);

	  strCopy( pGlobalLock, curPassString );
#else
      char* pGlobalLock = (char*)GlobalLock(hGlobalMemory);

      memcpy(pGlobalLock, (LPCSTR)curPassString, curPassString.GetLength());

      pGlobalLock[uGlobalMemSize-1] = _T'\0');
#endif

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
         AfxMessageBox(_T("The clipboard could not be opened correctly"));
		
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
         }
      }
   }
}


void
DboxMain::OnDelete() 
{
   if (SelItemOk() == TRUE)
   {
      BOOL dodelete = TRUE;
		
      //Confirm whether to delete the file
      CConfirmDeleteDlg deleteDlg(this);
      if (deleteDlg.m_dontaskquestion == FALSE)
      {
         int rc = deleteDlg.DoModal();
         if (rc == IDOK)
         {
            dodelete = TRUE;
         }
         else if (rc == IDCANCEL)
         {
            dodelete = FALSE;
         }
      }

      if (dodelete == TRUE)
      {
         m_changed = TRUE;
         int curSel = getSelectedItem();
         POSITION listindex = Find(curSel); // Must Find before delete from m_ctlItemList
	 m_ctlItemList.DeleteItem(curSel);
         m_pwlist.RemoveAt(listindex);
         int rc = SelectEntry(curSel);
         if (rc == LB_ERR) {
	   SelectEntry(m_ctlItemList.GetItemCount() - 1);
         }
         m_ctlItemList.SetFocus();
         ChangeOkUpdate();
      }
   }
}


void
DboxMain::OnFind() 
{
  CFindDlg::Doit(this); // create modeless or popup existing
}

void
DboxMain::OnEdit() 
{
   if (SelItemOk() == TRUE)
   {
      int curSel = getSelectedItem();
		
      POSITION listindex = Find(curSel);
      CItemData item = m_pwlist.GetAt(listindex);
      //CMyString item_name;
      //item.GetName(item_name);

      CEditDlg dlg_edit(this);
      SplitName(item.GetName(),
                dlg_edit.m_title, dlg_edit.m_username);
      dlg_edit.m_realpassword = item.GetPassword();
      dlg_edit.m_password = HIDDEN_PASSWORD;
      dlg_edit.m_notes = item.GetNotes();
      dlg_edit.m_listindex = listindex;   // for future reference, this is not multi-user friendly

	  app.DisableAccelerator();
      int rc = dlg_edit.DoModal();
	  app.EnableAccelerator();

      if (rc == IDOK)
      {
         CMyString temptitle;
         MakeName(temptitle, dlg_edit.m_title, dlg_edit.m_username);
         item.SetName(temptitle);

#if 0
         // JPRFIXME - P1.2
         //Adjust for the asterisks
         if (dlg_edit.m_password.GetLength() == 0)
            item.SetPassword(dlg_edit.m_password);
         else if (dlg_edit.m_password[dlg_edit.m_password.GetLength()-1] == '*')
            item.SetPassword(dlg_edit.m_realpassword);
         else
            item.SetPassword(dlg_edit.m_password);
#endif
         item.SetPassword(dlg_edit.m_realpassword);
         item.SetNotes(dlg_edit.m_notes);

         /*
           Out with the old, in with the new
         */
         m_pwlist.RemoveAt(listindex);
         POSITION curPos = m_pwlist.AddTail(item);
		   m_ctlItemList.DeleteItem(curSel);
		   insertItem(m_pwlist.GetAt(curPos));
         if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), FALSE) == TRUE)
         {
            Save();
         }
         else
         {
            m_changed = TRUE;
         }
      }

      rc = SelectEntry(curSel);
      if (rc == LB_ERR)
      {
	      SelectEntry(m_ctlItemList.GetItemCount() - 1);
      }
      m_ctlItemList.SetFocus();
      ChangeOkUpdate();
   }
}


void
DboxMain::OnOK() 
{
   int rc, rc2;

	char *ppszAttributeNames[] = {
		"column1width",
		"column2width",
		"column3width",
		"column4width"
	};
	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_WIDTH;
	for (int i = 0; i < 4; i++) {
		if (m_ctlItemList.GetColumn(i, &lvColumn)) {
			app.WriteProfileInt(_T(PWS_REG_OPTIONS), (LPCTSTR) ppszAttributeNames[i], lvColumn.cx);
		}
	}

	if (!IsIconic()) {
	  CRect rect;
	  GetWindowRect(&rect);
	  app.WriteProfileInt(_T(PWS_REG_POSITION), _T("top"), rect.top);
	  app.WriteProfileInt(_T(PWS_REG_POSITION), _T("bottom"), rect.bottom);
	  app.WriteProfileInt(_T(PWS_REG_POSITION), _T("left"), rect.left);
	  app.WriteProfileInt(_T(PWS_REG_POSITION), _T("right"), rect.right);
	}
	app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("sortedcolumn"), m_iSortedColumn);
	app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("sortascending"), m_bSortAscending);

   if (m_changed == TRUE)
   {
      rc = MessageBox(_T("Do you want to save changes to the password list?"),
                             AfxGetAppName(),
                             MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return;
      case IDYES:
         rc2 = Save();
         if (rc2 != SUCCESS)
            return;
      case IDNO:
         ClearClipboard();
         app.m_pMainWnd = NULL;
         break;
      }
   }
   else
   {
      ClearClipboard();
      app.m_pMainWnd = NULL;
   }

   ClearData();

   //Store current filename for next time...
   if (!m_currfile.IsEmpty())
      app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("currentfile"), m_currfile);
   else
      app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("currentfile"), NULL);

   if (!m_currbackup.IsEmpty())
      app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("currentbackup"), m_currbackup);
   else
      app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("currentbackup"), NULL);

   CDialog::OnOK();
}


void
DboxMain::OnCancel()
{
   OnOK();
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
#if 1
			 // {kjp} fixed to use UNICODE safe functions
			 trashMemory( lptstr, strLength(lptstr) );
#else
			 trashMemory((unsigned char*)lptstr, strlen(lptstr));
#endif
			 GlobalUnlock(hglb); 
         } 
      } 
   }
   if (EmptyClipboard()!=TRUE)
      AfxMessageBox(_T("The clipboard was not emptied correctly"));
   if (CloseClipboard() != TRUE)
      AfxMessageBox(_T("The clipboard could not be closed"));
}

//Finds stuff based on the .GetName() part not the entire object
POSITION
DboxMain::Find(const CMyString &a_title, const CMyString &a_user)
{
   POSITION listPos = m_pwlist.GetHeadPosition();
   CMyString curthing;

   while (listPos != NULL)
   {
      m_pwlist.GetAt(listPos).GetName(curthing);
	  CMyString title, user;
	  SplitName(curthing, title, user);
      if (title == a_title && user == a_user)
         break;
      else
         m_pwlist.GetNext(listPos);
   }

   return listPos;
}

 // Find in m_pwlist entry with same title and user name as the i'th entry in m_ctlItemList
POSITION DboxMain::Find(int i)
{
  const CMyString curTitle = m_ctlItemList.GetItemText(i, 0);
  const CMyString curUser = m_ctlItemList.GetItemText(i, 1);
  return Find(curTitle, curUser);
}


#if defined(POCKET_PC)
  #if (POCKET_PC_VER == 2000)
    #define PWS_CDECL	__cdecl
  #else
    #define PWS_CDECL
  #endif
#else
  #define PWS_CDECL
#endif

// for qsort in FindAll
static int PWS_CDECL compint(const void *a1, const void *a2)
{
  // since we're sorting a list of indices, v1 == v2 should never happen.
  const int v1 = *(int *)a1, v2 = *(int *)a2;
  ASSERT(v1 != v2);
  return (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;
}

#undef PWS_CDECL

/*
 * Finds all entries in m_pwlist that contain str in name or notes field,
 * returns their sorted indices in m_listctrl via indices, which is
 * assumed to be allocated by caller to DboxMain::GetNumEntries() ints.
 * FindAll returns the number of entries that matched.
 */

int
DboxMain::FindAll(const CString &str, BOOL CaseSensitive, int *indices)
{
  ASSERT(!str.IsEmpty());
  ASSERT(indices != NULL);

  POSITION listPos = m_pwlist.GetHeadPosition();
  CMyString curname, savecurname, curnotes;
  CString searchstr(str); // Since str is const, and we might need to MakeLower
  const int NumEntries = GetNumEntries();
  bool *matchVector = new bool[NumEntries];
  int retval = 0;
  int i;

  if (!CaseSensitive)
    searchstr.MakeLower();

  for (i = 0; i < NumEntries; i++)
    matchVector[i] = false;

  while (listPos != NULL)
  {
      m_pwlist.GetAt(listPos).GetName(curname);
      savecurname = curname; // keep original for finding in m_listctrl
      m_pwlist.GetAt(listPos).GetNotes(curnotes);

      if (!CaseSensitive) {
          curname.MakeLower();
          curnotes.MakeLower();
      }
#if 1
	  if ( strFind( curname, searchstr ) || strFind( curnotes, searchstr ) )
	  {
#else
#ifdef UNICODE
      if (::wcsstr(curname, searchstr) || ::wcsstr(curnotes, searchstr)) {
#else
      if (::strstr(curname, searchstr) || ::strstr(curnotes, searchstr)) {
#endif
#endif
	// Find index in m_listctl
	CMyString listTitle;
	CMyString title, username;
	SplitName(savecurname, title, username);
	for (i = 0; i < NumEntries; i++) {
	  listTitle = CMyString(m_ctlItemList.GetItemText(i, 0));
	  if (listTitle == title && !matchVector[i]) {
	    // add to indices, bump retval
	    indices[retval++] = i;
	    matchVector[i] = true; // needed because titles are not unique
	    break;
	  } // match found in m_listctrl
	} // for
      } // match found in m_pwlist
      m_pwlist.GetNext(listPos);
  }

  delete[] matchVector;
  // Sort indices
  if (retval > 1)
    ::qsort((void *)indices, retval, sizeof(indices[0]), compint);
  return retval;
}


//Checks and sees if everything works and something is selected
BOOL
DboxMain::SelItemOk()
{
   int curSel = getSelectedItem();
   if (curSel != LB_ERR)
   {
     POSITION listindex = Find(curSel);
         if (listindex != NULL)
            return TRUE;
   }
   return FALSE;
}

BOOL DboxMain::SelectEntry(int i, BOOL MakeVisible)
{
  BOOL retval;
  retval = m_ctlItemList.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
  if (MakeVisible)
    m_ctlItemList.EnsureVisible(i, FALSE);
  return retval;
}


//Updates m_listctrl from m_pwlist
// {kjp} Updated for Pocket PC to stop it updating after each item is added
// {kjp} and to do so only after they've all been added and the list sorted.
void
DboxMain::RefreshList()
{
   if (! m_windowok)
      return;

#if defined(POCKET_PC)
	HCURSOR		waitCursor = app.LoadStandardCursor( IDC_WAIT );
#endif

   //Copy the data
#if defined(POCKET_PC)
   m_ctlItemList.SetRedraw( FALSE );
#endif
   m_ctlItemList.DeleteAllItems();
#if defined(POCKET_PC)
   m_ctlItemList.SetRedraw( TRUE );
#endif

	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_WIDTH;

	bool bPasswordColumnShowing = m_ctlItemList.GetColumn(3, &lvColumn)? true: false;
	if (m_bShowPasswordInList && !bPasswordColumnShowing) {
		m_ctlItemList.InsertColumn(3, _T("Password"));
		CRect rect;
		m_ctlItemList.GetClientRect(&rect);
		m_ctlItemList.SetColumnWidth(3, app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column4width"), rect.Width() / 4));
	}
	else if (!m_bShowPasswordInList && bPasswordColumnShowing) {
		app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("column4width"), lvColumn.cx);
		m_ctlItemList.DeleteColumn(3);
	}

#if defined(POCKET_PC)
   m_ctlItemList.SetRedraw( FALSE );
   SetCursor( waitCursor );
#endif
   POSITION listPos = m_pwlist.GetHeadPosition();
   while (listPos != NULL)
   {
		insertItem(m_pwlist.GetAt(listPos));
      m_pwlist.GetNext(listPos);
   }

	m_ctlItemList.SortItems(CompareFunc, MAKELPARAM(m_iSortedColumn, (int)m_bSortAscending));
#if defined(POCKET_PC)
	SetCursor( NULL );
	m_ctlItemList.SetRedraw( TRUE );
#endif

   //Setup the selection
   if (m_ctlItemList.GetItemCount() > 0 && getSelectedItem() < 0) {
      SelectEntry(0);
   }
}

void
DboxMain::OnPasswordChange() 
{
   /*
    * To change passkeys, the data is copied into a list of CMyStrings
    * and then re-put into the list with the new passkey
    */

   /*
    * CItemData should have a ChangePasskey method instead
    */

   /*
    * Here is my latest thought on this: It is definately possible to give
    * CItemData a ChangePasskey method. However, that would involve either
    * keeping two copies of the key schedule in memory at once, which would
    * then require a lot more overhead and variables than we currently have,
    * or recreating first the current and then the new schedule for each
    * item, which would be really slow. Which is why I think that we should
    * leave well enough alone. I mean, this function does work in the end.
    */
	
   CPasskeyChangeDlg changeDlg(this);
   int rc = changeDlg.DoModal();
   if (rc == IDOK)
   {
      m_changed = TRUE;
      //Copies the list into a plaintext list of CMyStrings
      CList<CMyString, CMyString> tempList;
      tempList.RemoveAll();
      POSITION listPos = m_pwlist.GetHeadPosition();
      while (listPos != NULL)
      {
         CItemData temp;
         temp = m_pwlist.GetAt(listPos);
         CMyString str;
         temp.GetName(str);
         tempList.AddTail(str);
         temp.GetPassword(str);
         tempList.AddTail(str);
         temp.GetNotes(str);
         tempList.AddTail(str);
         m_pwlist.GetNext(listPos);
      }
      m_pwlist.RemoveAll();
      listPos = tempList.GetHeadPosition();

      //Changes the global password. Eck.
      app.m_passkey = changeDlg.m_newpasskey;
		
      //Gets a new random value used for password authentication
      for (int x=0; x < 8; x++)
         app.m_randstuff[x] = newrand();
      /*
       * We generate 8 bytes of randomness, but m_randstuff
       * is larger: StuffSize bytes. This appears to be a bug,
       * let's at least explicitly zero the extra 2 bytes, since redefining
       * StuffSize to 8 would break every existing database...
       */
      app.m_randstuff[8] = app.m_randstuff[9] = '\0';

      GenRandhash(changeDlg.m_newpasskey,
                  app.m_randstuff,
                  app.m_randhash);

      //Puts the list of CMyStrings back into CItemData
      while (listPos != NULL)
      {
         CItemData temp;
			
         temp.SetName(tempList.GetAt(listPos));
         tempList.GetNext(listPos);
			
         temp.SetPassword(tempList.GetAt(listPos));
         tempList.GetNext(listPos);

         temp.SetNotes(tempList.GetAt(listPos));
         tempList.GetNext(listPos);

         m_pwlist.AddTail(temp);
      }
		
      RefreshList();
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
DboxMain::OnSize(UINT nType,
                 int cx,
                 int cy) 
//Note that onsize runs before InitDialog (Gee, I love MFC)
{
   CDialog::OnSize(nType, cx, cy);

// {kjp} Only SIZE_RESTORED is supported on Pocket PC.
#if !defined(POCKET_PC)
   if (nType == SIZE_MINIMIZED)
   {
	   m_ctlItemList.DeleteAllItems();
      if (app.GetProfileInt(_T(PWS_REG_OPTIONS),
                            _T("dontaskminimizeclearyesno"),
                            FALSE) == TRUE)
      {
         ClearClipboard();
      }
      if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("databaseclear"), FALSE) == TRUE)
      {
         BOOL dontask = app.GetProfileInt(_T(PWS_REG_OPTIONS),
                                          _T("dontasksaveminimize"),
                                          FALSE);
         BOOL doit = TRUE;
         if ((m_changed == TRUE)
             && (dontask == FALSE))
         {
            CRemindSaveDlg remindDlg(this);

            int rc = remindDlg.DoModal();
            if (rc == IDOK)
            {
            }
            else if (rc == IDCANCEL)
            {
               doit = FALSE;
            }
         }

         if ((doit == TRUE) && (m_existingrestore == FALSE)) 
         {
            OnSave();
            ClearData();
            m_needsreading = TRUE;
         }
      }
   }
   else if (!m_bSizing && nType == SIZE_RESTORED)	// gets called even when just resizing window
   {
#endif
      if ((m_needsreading == TRUE)
          && (m_existingrestore == FALSE)
          && (m_windowok))
      {
         m_existingrestore = TRUE;

         CMyString passkey;
         int rc, rc2;
         CMyString temp;

         rc = CheckPassword(m_currfile, passkey);
         switch (rc)
         {
         case SUCCESS:
            rc2 = ReadFile(m_currfile, passkey);
#if !defined(POCKET_PC)
            m_title = "Password Safe - " + m_currfile;
#endif
            break; 
         case CANT_OPEN_FILE:
            temp =
               m_currfile
               + "\n\nCannot open database. It likely does not exist."
               + "\nA new database will be created.";
            MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
         case TAR_NEW:
            rc2 = New();
            break;
         case TAR_OPEN:
            rc2 = Open();
            break;
         case WRONG_PASSWORD:
            rc2 = NOT_SUCCESS;
            break;
         default:
            rc2 = NOT_SUCCESS;
            break;
         }

         if (rc2 == SUCCESS)
         {
            m_needsreading = FALSE;
            m_existingrestore = FALSE;
			RefreshList();
         }
         else
         {
            app.m_pMainWnd = NULL;
            CDialog::OnCancel();
         }
      }
      RefreshList();
#if !defined(POCKET_PC)
   }
#endif

	if (m_windowok) {
		// And position the control bars
		CRect rect;
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
		m_ctlItemList.MoveWindow(&rect, TRUE);
	}

	m_bSizing = false;
}


void
DboxMain::OnSave() 
{
   Save();
}


int
DboxMain::Save()
{
   int rc;

   if (m_currfile.IsEmpty())
      return SaveAs();

   rc = WriteFile(m_currfile);

   if (rc == CANT_OPEN_FILE)
   {
      CMyString temp = m_currfile + "\n\nCould not open file for writing!";
      MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      return CANT_OPEN_FILE;
   }

   m_changed = FALSE;
   ChangeOkUpdate();
   return SUCCESS;
}


void
DboxMain::ChangeOkUpdate()
{
   if (! m_windowok)
      return;
#if defined(POCKET_PC)
	/*
	CMenu		*menu	= m_wndMenu;
	
//	menu = m_wndCommandBar->GetMenuBar( 0 );

	if ( menu != NULL )
	{
		if (m_changed == TRUE)
		{
			menu->EnableMenuItem(ID_MENUITEM_SAVE, MF_ENABLED);
		}
		else if (m_changed == FALSE)
		{
			menu->EnableMenuItem(ID_MENUITEM_SAVE, MF_GRAYED);
		}
	}
	*/
#else
   if (m_changed == TRUE)
      GetMenu()->EnableMenuItem(ID_MENUITEM_SAVE, MF_ENABLED);
   else if (m_changed == FALSE)
      GetMenu()->EnableMenuItem(ID_MENUITEM_SAVE, MF_GRAYED);

   /*
     This doesn't exactly belong here, but it makes sure that the
     title is fresh...
   */
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

   POSITION itemPos = Find(getSelectedItem());

   CMyString title, junk, username;
   m_pwlist.GetAt(itemPos).GetName(title);
   SplitName(title, junk, username);

   if (username.GetLength() == 0)
   {
      AfxMessageBox(_T("There is no username associated with this item."));
   }
   else
   {
	  // {kjp} fix to use the real amount of storage used
      uGlobalMemSize = (username.GetLength() + 1) * sizeof(TCHAR);
      hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                                  uGlobalMemSize);
#if 1
	  // {kjp} fix to use UNICODE safe string definitions and string copy functions
      LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);

	  strCopy( pGlobalLock, username );
#else
      char* pGlobalLock = (char*)GlobalLock(hGlobalMemory);
      
      memcpy(pGlobalLock, (LPCSTR)username, username.GetLength());

      pGlobalLock[uGlobalMemSize-1] = '\0';
#endif
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
DboxMain::OnContextMenu(CWnd *, CPoint point) 
{
	TRACE0("DboxMain::OnContextMenu");
	CPoint local = point;
	m_ctlItemList.ScreenToClient(&local);

	int item = m_ctlItemList.HitTest(local);

	if (item >= 0)
	{
		int rc = SelectEntry(item);
		if (rc == LB_ERR) {
			SelectEntry(m_ctlItemList.GetItemCount() - 1);
		}
		m_ctlItemList.SetFocus();

		CMenu menu;
		if (menu.LoadMenu(IDR_POPMENU))
		{
			CMenu* pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != NULL);

			pPopup->TrackPopupMenu(
#ifdef POCKET_PC
								TPM_LEFTALIGN,
#else
								TPM_LEFTALIGN | TPM_RIGHTBUTTON,
#endif
                                point.x, point.y,
                                this); // use this window for commands
		}
	}
}

/*
int
DboxMain::OnVKeyToItem(UINT nKey,
                       CListBox* pListBox,
                       UINT nIndex) 
{
   int curSel = m_ctlItemList.GetCurSel();

   switch (nKey)
   {
   case VK_DELETE:
      OnDelete();
      return -2;
   case VK_INSERT:
      OnAdd();
      return -2;
   // JPRFIXME P1.8
   case VK_PRIOR:  //Page up
      return -1; //do default
   case VK_HOME:
      m_ctlItemList.SetCurSel(0);
      m_ctlItemList.SetFocus();
      return -2;
   case VK_NEXT:   //Page Down
      return -1; // do default;
   case VK_END:
      m_ctlItemList.SetCurSel(m_ctlItemList.GetCount()-1);
      m_ctlItemList.SetFocus();
      return -2;
   case VK_UP:
   case VK_LEFT:
      if (curSel>0)
         m_ctlItemList.SetCurSel(curSel-1);
      m_ctlItemList.SetFocus();
      return -2;
   case VK_DOWN:
   case VK_RIGHT:
      if (curSel!=(m_ctlItemList.GetCount()-1))
         m_ctlItemList.SetCurSel(curSel+1);
      m_ctlItemList.SetFocus();
      return -2;
   case VK_CONTROL:
      return -2;
   }
   return CDialog::OnVKeyToItem(nKey, pListBox, nIndex);
}
*/

void DboxMain::OnKeydownItemlist(NMHDR* pNMHDR, LRESULT* pResult) {
	LV_KEYDOWN *pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

   switch (pLVKeyDow->wVKey) {
   case VK_DELETE:
      OnDelete();
      break;
   case VK_INSERT:
      OnAdd();
      break;
   }

	*pResult = 0;
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
         return USER_CANCEL;
   }

   rc = WriteFile(tempname);
   if (rc == CANT_OPEN_FILE)
   {
      CMyString temp = tempname + "\n\nCould not open file for writing!";
      MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      return CANT_OPEN_FILE;
   }

   m_currbackup = tempname;
   return SUCCESS;
}


void
DboxMain::OnOpen() 
{
   Open();
}


int
DboxMain::Open()
{
   int rc = SUCCESS;
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

		 if ( rc == SUCCESS )
	         break;
      }
      else
         return USER_CANCEL;
   }

   return rc;
}

int
DboxMain::Open( const CMyString &pszFilename )
{
	int rc;
	CMyString passkey, temp;

	//Check that this file isn't already open
	if (pszFilename == m_currfile && !m_needsreading)
	{
		//It is the same damn file
		MessageBox(_T("That file is already open."),
			_T("Oops!"),
			MB_OK|MB_ICONWARNING);
		return ALREADY_OPEN;
	}
	
	if (m_changed == TRUE)
	{
		int rc2;
		
		temp =
			"Do you want to save changes to the password database: "
			+ m_currfile
			+ "?";
		rc = MessageBox(temp,
			AfxGetAppName(),
			MB_ICONQUESTION|MB_YESNOCANCEL);
		switch (rc)
		{
		case IDCANCEL:
			return USER_CANCEL;
		case IDYES:
			rc2 = Save();
			// Make sure that writing the file was successful
			if (rc2 == SUCCESS)
				break;
			else
				return CANT_OPEN_FILE;
		case IDNO:
			break;
		}
	}
	
	rc = CheckPassword(pszFilename, passkey);
	switch (rc)
	{
	case SUCCESS:
//		app.GetMRU()->Add( LPCTSTR(pszFilename) );
		break; // Keep going... 
	case CANT_OPEN_FILE:
		temp = m_currfile + "\n\nCan't open file. Please choose another.";
		MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
	case TAR_OPEN:
		return Open();
	case TAR_NEW:
		return New();
	case WRONG_PASSWORD:
	/*
	If the user just cancelled out of the password dialog, 
	assume they want to return to where they were before... 
		*/
		return USER_CANCEL;
	}
	
	rc = ReadFile(pszFilename, passkey);
	if (rc == CANT_OPEN_FILE)
	{
		temp = pszFilename;
		temp += "\n\nCould not open file for reading!";
		MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
		/*
		Everything stays as is... Worst case,
		they saved their file....
		*/
		return CANT_OPEN_FILE;
	}
	
	m_currfile = pszFilename;
	m_changed = FALSE;
#if !defined(POCKET_PC)
	m_title = "Password Safe - " + m_currfile;
#endif
	ChangeOkUpdate();
	RefreshList();
	
	return SUCCESS;
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

   if (m_changed==TRUE)
   {
      CMyString temp =
         "Do you want to save changes to the password database: "
         + m_currfile
         + "?";

      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return USER_CANCEL;
      case IDYES:
         rc2 = Save();
         /*
           Make sure that writing the file was successful
         */
         if (rc2 == SUCCESS)
            break;
         else
            return CANT_OPEN_FILE;
      case IDNO:
         break;
      }
   }

   rc = NewFile();
   if (rc == USER_CANCEL)
      /*
        Everything stays as is... 
        Worst case, they saved their file.... 
      */
      return USER_CANCEL;

   m_currfile = ""; //Force a save as... 
   m_changed = FALSE;
#if !defined(POCKET_PC)
   m_title = "Password Safe - <Untitled>";
#endif
   ChangeOkUpdate();

   return SUCCESS;
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
         return USER_CANCEL;
   }

   rc = CheckPassword(newback, passkey);
   switch (rc)
   {
   case SUCCESS:
      break; // Keep going... 
   case CANT_OPEN_FILE:
      temp =
         m_currfile
         + "\n\nCan't open file. Please choose another.";
      MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
   case TAR_OPEN:
      return Open();
   case TAR_NEW:
      return New();
   case WRONG_PASSWORD:
      /*
        If the user just cancelled out of the password dialog, 
        assume they want to return to where they were before... 
      */
      return USER_CANCEL;
   }

   if (m_changed==TRUE)
   {
      int rc2;
	
      temp = "Do you want to save changes to the password list: "
         + m_currfile + "?";

      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return USER_CANCEL;
      case IDYES:
         rc2 = Save();
         //Make sure that writing the file was successful
         if (rc2 == SUCCESS)
            break;
         else
            return CANT_OPEN_FILE;
      case IDNO:
         break;
      }
   }

   rc = ReadFile(newback, passkey);
   if (rc == CANT_OPEN_FILE)
   {
      temp = newback + "\n\nCould not open file for reading!";
      MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
      //Everything stays as is... Worst case, they saved their file....
      return CANT_OPEN_FILE;
   }
	
   m_currfile = ""; //Force a save as...
   m_changed = TRUE; //So that the *.dat version of the file will be saved.
#if !defined(POCKET_PC)
   m_title = "Password Safe - <Untitled Restored Backup>";
#endif
   ChangeOkUpdate();
   RefreshList();

   return SUCCESS;
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

   //SaveAs-type dialog box
   while (1)
   {
      CFileDialog fd(FALSE,
                     _T("dat"),
                     m_currfile,
                     OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                     |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                     _T("Password Safe Databases (*.dat)|*.dat|")
                     _T("All files (*.*)|*.*|")
                     _T("|"),
                     this);
      if (m_currfile.IsEmpty())
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
         return USER_CANCEL;
   }

   rc = WriteFile(newfile);
   if (rc == CANT_OPEN_FILE)
   {
      CMyString temp = newfile + "\n\nCould not open file for writing!";
      MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      return CANT_OPEN_FILE;
   }

   m_currfile = newfile;
   m_changed = FALSE;
#if !defined(POCKET_PC)
   m_title = "Password Safe - " + m_currfile;
#endif
   ChangeOkUpdate();

//   app.GetMRU()->Add( newfile );

   return SUCCESS;
}

#ifdef POCKET_PC
int DboxMain::WriteCBC(FILE *fp, const CString &data, const unsigned char *salt,
		       unsigned char *ipthing)
{
	// We do a double cast because the LPCSTR cast operator is overridden by the CString class
	// to access the pointer we need,
	// but we in fact need it as an unsigned char. Grrrr.

	CMyString	myData(data);

	LPCSTR passstr = LPCSTR(app.m_passkey);
	LPCSTR datastr = LPCSTR(myData);

	return _writecbc(fp, (const unsigned char *)datastr, data.GetLength(),
		(const unsigned char *)passstr, app.m_passkey.GetLength(),
		salt, SaltLength, ipthing);
}
#else
int DboxMain::WriteCBC(int fp, const CString &data, const unsigned char *salt,
		       unsigned char *ipthing)
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.

  LPCSTR passstr = LPCSTR(app.m_passkey);
  LPCSTR datastr = LPCSTR(data);

  return _writecbc(fp, (const unsigned char *)datastr, data.GetLength(),
		   (const unsigned char *)passstr, app.m_passkey.GetLength(),
		   salt, SaltLength, ipthing);
}
#endif

int
DboxMain::WriteFile(const CMyString &filename)
{
	int x;
#ifdef POCKET_PC
#ifdef UNICODE
	FILE *out = _wfopen((LPCTSTR)filename, _T("wb") );
#else
	FILE *out = fopen((LPCTSTR)filename, _T("wb") );
#endif

	if ( out == NULL )
		return CANT_OPEN_FILE;

	fwrite( app.m_randstuff, 1,  8, out ); 
	fwrite( app.m_randhash,  1, 20, out );
#else

   int out = _open((LPCTSTR)filename,
                   _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
                   _S_IREAD | _S_IWRITE);

   if (out == -1)
      return CANT_OPEN_FILE;

   _write(out, app.m_randstuff, 8);
   _write(out, app.m_randhash, 20);
#endif

   /*
     I know salt is just salt, but randomness always makes me
     nervous - must check this out {jpr}
    */
   unsigned char* thesalt = new unsigned char[SaltLength];
   for (x=0; x<SaltLength; x++)
      thesalt[x] = newrand();

#ifdef POCKET_PC
   fwrite( thesalt, 1, SaltLength, out );
#else
   _write(out, thesalt, SaltLength);
#endif
	
   unsigned char ipthing[8];
   for (x=0; x<8; x++)
      ipthing[x] = newrand();

#ifdef POCKET_PC
   fwrite(ipthing, 1, 8, out);
#else
   _write(out, ipthing, 8);
#endif

   //Write out full names
   BOOL needexpand = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE);
   CMyString defusername = app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T(""));
   if (needexpand==TRUE)
      MakeFullNames(&m_pwlist, defusername);

   CItemData temp;
   POSITION listPos = m_pwlist.GetHeadPosition();
   CMyString tempdata;
   while (listPos != NULL)
   {
      temp = m_pwlist.GetAt(listPos);
      temp.GetName(tempdata);
      WriteCBC(out, tempdata, thesalt, ipthing);
      temp.GetPassword(tempdata);
      WriteCBC(out, tempdata, thesalt, ipthing);
      temp.GetNotes(tempdata);
      WriteCBC(out, tempdata, thesalt, ipthing);
      m_pwlist.GetNext(listPos);
   }
#ifdef POCKET_PC
   fclose(out);
#else
   _close(out);
#endif

   delete [] thesalt;

   //Restore shortened names if necessary
   if (needexpand)
      DropDefUsernames(&m_pwlist, defusername);

   m_changed = FALSE;
   ChangeOkUpdate();

   return SUCCESS;
}


int
DboxMain::CheckPassword(const CMyString &filename,
                        CMyString& passkey,
                        bool first)
{
  DBGMSG("DboxMain::CheckPassword()\n");

  unsigned char temprandstuff[8];
  unsigned char temprandhash[20];
  int retval;
  bool saved_stuff = false;

  if (filename != _T(""))
    {
      DBGMSG("filename not blank\n");

#ifdef POCKET_PC
#ifdef UNICODE
	  FILE *in = _wfopen((LPCTSTR) filename, _T("rb"));
#else
	  FILE *in = fopen((LPCTSTR) filename, _T("rb"));
#endif

	  if ( in == NULL )
#else
      int in = _open((LPCTSTR) filename,
                     _O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
                     S_IREAD | _S_IWRITE);

      if (in == -1)
#endif
	{
	  DBGMSG("open return -1\n");

	  if (! first)
            return CANT_OPEN_FILE;

	  CString Errmess(_T("Can't open database "));
	  Errmess += (const CString&)filename;
	  MessageBox(Errmess, _T("File open error"),
		     MB_OK | MB_ICONWARNING);
	}
      else
	{
	  DBGMSG("hashstuff\n");

	  //Preserve the current randstuff and hash
	  memcpy(temprandstuff, app.m_randstuff, 8);
	  memcpy(temprandhash, app.m_randhash, 20);
	  saved_stuff = true;

	  /*
	    The beginning of the database file is
	    8 bytes of randomness and a SHA1 hash {jpr}
	  */
#ifdef POCKET_PC
	  fread(app.m_randstuff, 1,  8, in);
	  fread(app.m_randhash,  1, 20, in);
	  fclose(in);
#else
	  _read(in, app.m_randstuff, 8);
	  _read(in, app.m_randhash, 20);
	  _close(in);
#endif
	}
    }

  /*
   * with my unsightly hacks of PasskeyEntry, it should now accept
   * a blank filename, which will disable passkey entry and the OK button
   */

  CPasskeyEntry dbox_pkentry(this, filename, first);
  app.m_pMainWnd = &dbox_pkentry;
  //dbox_pkentry->m_message = filename;
  int rc = dbox_pkentry.DoModal();
  //dbox_pkentry.ShowWindow()

  if (rc == IDOK)
    {
      DBGMSG("PasskeyEntry returns IDOK\n");
      passkey = dbox_pkentry.m_passkey;
      retval = SUCCESS;
    }
  else /*if (rc==IDCANCEL) */ //Determine reason for cancel
    {
      int cancelreturn = dbox_pkentry.GetStatus();
      switch (cancelreturn)
	{
	case TAR_OPEN:
	case TAR_NEW:
	  DBGMSG("PasskeyEntry TAR_OPEN or TAR_NEW\n");
	  retval = cancelreturn;		//Return either open or new flag... 
	  break;
	default:
	  DBGMSG("Default to WRONG_PASSWORD\n");
	  retval = WRONG_PASSWORD;	//Just a normal cancel
	  break;
	}
    }

  //Restore the current randstuff and hash
  if (saved_stuff)
    {
      memcpy(app.m_randstuff, temprandstuff, 8);
      memcpy(app.m_randhash, temprandhash, 20);
      trashMemory(temprandstuff, 8);
      trashMemory(temprandhash, 20);
    }

  app.m_pMainWnd = NULL; // done with dbox_pkentry

  return retval;
}

#ifdef POCKET_PC
int DboxMain::ReadCBC(FILE *fp, CMyString &data, const unsigned char *salt,
		       unsigned char *ipthing)
#else
int DboxMain::ReadCBC(int fp, CMyString &data, const unsigned char *salt,
		       unsigned char *ipthing)
#endif
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(app.m_passkey);

  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  int retval;

  retval = _readcbc(fp, buffer, buffer_len,
		   (const unsigned char *)passstr, app.m_passkey.GetLength(),
		   salt, SaltLength, ipthing);
  if (buffer_len > 0) {
    CMyString str(LPCSTR(buffer), buffer_len);
    data = str;
    trashMemory(buffer, buffer_len);
    delete[] buffer;
  } else {
    data = "";
  }
  return retval;
}


int
DboxMain::ReadFile(const CMyString &a_filename,
                   const CMyString &a_passkey)
{	
   //That passkey had better be the same one that came from CheckPassword(...)

#ifdef POCKET_PC
	int		nr;
#ifdef UNICODE
	FILE * in = _wfopen((LPCTSTR) a_filename, _T("rb") );
#else
	FILE * in = fopen((LPCTSTR) a_filename, _T("rb") );
#endif

	if (in == NULL)
#else
   int in = _open((LPCTSTR) a_filename,
                  _O_BINARY |_O_RDONLY | _O_SEQUENTIAL,
                  S_IREAD | _S_IWRITE);

   if (in == -1)
#endif
      return CANT_OPEN_FILE;

   ClearData(); //Before overwriting old data, but after opening the file... 

#ifdef POCKET_PC
   nr = fread(app.m_randstuff, 1,  8, in);
   nr = fread(app.m_randhash,  1, 20,  in);
#else
   _read(in, app.m_randstuff, 8);
   _read(in, app.m_randhash, 20);
#endif

   unsigned char* salt = new unsigned char[SaltLength];
   unsigned char ipthing[8];

#ifdef POCKET_PC
   nr = fread(salt,    1, SaltLength, in);
   nr = fread(ipthing, 1, 8, in);
#else
   _read(in, salt, SaltLength);
   _read(in, ipthing, 8);
#endif

   app.m_passkey = a_passkey;

   CItemData temp;
   CMyString tempdata;

   int numread = 0;
   numread += ReadCBC(in, tempdata, salt, ipthing);
   temp.SetName(tempdata);
   numread += ReadCBC(in, tempdata, salt, ipthing);
   temp.SetPassword(tempdata);
   numread += ReadCBC(in, tempdata, salt, ipthing);
   temp.SetNotes(tempdata);
   while (numread > 0)
   {
      m_pwlist.AddTail(temp);
      numread = 0;
      numread += ReadCBC(in, tempdata, salt, ipthing);
      temp.SetName(tempdata);
      numread += ReadCBC(in, tempdata, salt, ipthing);
      temp.SetPassword(tempdata);
      numread += ReadCBC(in, tempdata, salt, ipthing);
      temp.SetNotes(tempdata);
   }

   delete [] salt;
#ifdef POCKET_PC
   fclose(in);
#else
   _close(in);
#endif

   //Shorten names if necessary
   if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE) == TRUE)
   {
      CMyString temp = app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T(""));
      DropDefUsernames(&m_pwlist, temp);
   }

   return SUCCESS;
}


int
DboxMain::NewFile(void)
{
   CPasskeySetup dbox_pksetup(this);
   app.m_pMainWnd = &dbox_pksetup;
   int rc = dbox_pksetup.DoModal();

   if (rc == IDCANCEL)
      return USER_CANCEL;  //User cancelled password entry

   ClearData();

   app.m_passkey = dbox_pksetup.m_passkey;

   for (int x=0; x<8; x++)
      app.m_randstuff[x] = newrand();
   app.m_randstuff[8] = app.m_randstuff[9] = '\0';
   GenRandhash(app.m_passkey, app.m_randstuff, app.m_randhash);

   return SUCCESS;
}


void
DboxMain::ClearData(void)
{
  app.m_passkey.Trash();

   //Composed of ciphertext, so doesn't need to be overwritten
   m_pwlist.RemoveAll();
	
   //Because GetText returns a copy, we cannot do anything about the names
   if (m_windowok)
      //Have to make sure this doesn't cause an access violation
      m_ctlItemList.DeleteAllItems();
}


struct backup_t
{
   CMyString name;
   CMyString location;
};


void
DboxMain::OnUpdateBackups() 
{
   int rc;
   CMyString temp;
   CList<backup_t, backup_t> backuplist;

   //Collect list of backups from registry
   //This code copied almost verbatim from the old BackupDlg.cpp
   CMyString companyname;
   VERIFY(companyname.LoadString(IDS_COMPANY) != 0);
	
   //We need to use the Win32SDK method because of RegEnumKeyEx
   CMyString subkeyloc =
      (CMyString)"Software\\" 
      + companyname 
      + (CMyString) "\\Password Safe\\Backup";
   HKEY subkey;
   DWORD disposition;
   LONG result = RegCreateKeyEx(HKEY_CURRENT_USER,
                                subkeyloc, 0, NULL,
                                REG_OPTION_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &subkey, &disposition);
   if (result != ERROR_SUCCESS)
   {
      //AfxMessageBox("There was an error opening a registry key. Sorry.");
      return;
   }

   //If the key is new, it has no data
   if (disposition == REG_CREATED_NEW_KEY)
   {
      //AfxMessageBox("There are no filenames stored in the registry. Sorry.");
      RegCloseKey(subkey);
      return;
   }
	
   //Check if the key has any items (in this case, backup listings).
   //If yes, check if user wants to update them. If no, close key and return
   if (disposition == REG_OPENED_EXISTING_KEY)
   {
      DWORD test;
      rc = IDNO;
      RegQueryInfoKey(subkey,
                      NULL, NULL, NULL, NULL, NULL, NULL,
                      &test,
                      NULL, NULL, NULL, NULL);
      if (test!=0)
      {
         temp =
            (CMyString)
            "Password Safe has detected the presence of old backup records\n"
            "from Version 1.1 of this program.  If you wish, you can update\n"
            "these files to the current version (by simply adding a .bak "
            "extension).\n"
            "\nYou will be presented with a list of file locations and the "
            "opportunity\n"
            "to save them to a text file for future reference. Also, you can "
            "rerun this\n"
            "function at any time through the \"Update V1.1 Backups...\" "
            "menu item."
            "\n\nDo you wish to proceed?";

         rc = MessageBox(LPCTSTR(temp),
                                _T("Update Backups"),
                                MB_YESNOCANCEL|MB_ICONWARNING);
      }
      if (rc!= IDYES)
      {
         RegCloseKey(subkey);
         return;
      }
	
      //Ok, we have the go-ahead. Collect the data and update it
      int x = 0;
      int result = ERROR_SUCCESS;
#ifdef UNICODE
      wchar_t key[_MAX_PATH];
#else
      char key[_MAX_PATH];
#endif
      unsigned char value[_MAX_PATH];
      DWORD keylen = _MAX_PATH, valuelen = _MAX_PATH;
      DWORD valtype = REG_SZ;
      backup_t temp;

      result = RegEnumValue(subkey, x, key, &keylen, NULL,
                            &valtype, value, &valuelen);
      keylen = _MAX_PATH; valuelen = _MAX_PATH;
      while (result != ERROR_NO_MORE_ITEMS)
      {
         temp.name = key;
         temp.location = value;
			
         BOOL resp = CheckExtension(temp.location, (CMyString) ".bak");
         if (resp == FALSE) // File has wrong extension.
         {
            int ret = rename(temp.location, temp.location + ".bak");
            if (ret == 0) //Success
            {
               temp.location = temp.location + ".bak";
               backuplist.AddTail(temp);				
            }
            else if (errno == EACCES)
               // There is already a .bak version around
            {;}
            else if (errno == ENOENT)
               // The old version no longer exists
            {
               CMyString out =
                  "Please note that the backup named \""
                  + temp.name
                  + "\"\nno longer exists.It will be removed"
                  " from the registry.";
               MessageBox(out, _T("File not found."), MB_OK|MB_ICONWARNING);
               temp.location = "";
               backuplist.AddTail(temp);
            }
         }	
         else
         {
            //Test to make sure it still exists
            int ret = rename(temp.location, temp.location);
            if (ret != 0 && errno == ENOENT)
            {
               CMyString out =
                  "Please note that the backup named \""
                  + temp.name 
                  + "\"\nno longer exists. It will be removed"
                  " from the registry.";
               MessageBox(out, _T("File not found."), MB_OK|MB_ICONWARNING);
               temp.location = "";
               backuplist.AddTail(temp);
            }
         }
         x++;
         result = RegEnumValue(subkey, x, key, &keylen, NULL,
                               &valtype, value, &valuelen);
         keylen = _MAX_PATH;
         valuelen = _MAX_PATH;
      }

      CMyString out = "The following files were altered:\n\n";
      CMyString out2 = "";
      POSITION listpos = backuplist.GetHeadPosition();
      while (listpos != NULL)
      {
         backup_t temp = backuplist.GetAt(listpos);
         if (temp.location != "")
         {
            out2 = out2 + temp.name + "\t" + temp.location + "\n";
         }
         backuplist.GetNext(listpos);
      }

      if (out2 == "")
         out2 = "None.\n";

      CMyString out3 =
         (CMyString)
         "\nDo you want to save a text version of this list?\n\n"
         "(The file will be called changedbackups.txt\n"
         "and will be saved in the current directory)";

      rc = MessageBox(out+out2+out3,
                             _T("Changed Files"), MB_YESNOCANCEL);
      if (rc == IDYES)
      {
#ifdef POCKET_PC
#ifdef UNICODE
			FILE * in = _wfopen( _T("changedbackups.txt"), _T("w") );
#else
			FILE * in = fopen( _T("changedbackups.txt"), _T("w") );
#endif

			if ( in != NULL )
			{
				fwrite( LPCTSTR(out+out2), 1, wcslen(LPCTSTR(out+out2)), in );
				fclose( in );
			}
#else
         int in = _open("changedbackups.txt",
                        _O_TEXT|_O_WRONLY|_O_SEQUENTIAL|_O_APPEND|_O_CREAT,
                        _S_IREAD | _S_IWRITE);
         if (in != -1)
         {
            // No error
            _write(in, LPCTSTR(out+out2), strlen(LPCTSTR(out+out2)));
            _close(in);
         }
#endif
      }


      //Write data back to registry. This will alter the names altered and deleted
      //the names not found. Copied from backupdlg.cpp
      POSITION listPos = backuplist.GetHeadPosition();
      while (listPos != NULL)
      {
         backup_t item = backuplist.GetAt(listPos);
         if (item.location != "")
            app.WriteProfileString(_T("Backup"), item.name, item.location);
         else
            app.WriteProfileString(_T("Backup"), item.name, NULL);

         backuplist.GetNext(listPos);
      }

      //Mark that this has been done.
      app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("donebackupchange"), TRUE);
   }

   RegCloseKey(subkey);
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
#ifndef UNICODE
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
DboxMain::OnSetfocusItemlist( NMHDR *, LRESULT *) 
{
   const UINT statustext = IDS_STATMESSAGE;

   if (m_toolbarsSetup == FALSE)
      return;

   m_statusBar.SetIndicators(&statustext, 1);	
   // Make a sunken or recessed border around the first pane
   m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
}
#endif


#if !defined(POCKET_PC)
void
DboxMain::OnKillfocusItemlist( NMHDR *, LRESULT *) 
{
   const UINT statustext = IDS_STATCOMPANY;

   if (m_toolbarsSetup == FALSE)
      return;

   m_statusBar.SetIndicators(&statustext, 1);
   // Make a sunken or recessed border around the first pane
   m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
}
#endif


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

////////////////////////////////////////////////////////////////////////////////
// NOTE!
// itemData must be the actual item in the item list.  if the item is remove
// from the list, it must be removed from the display as well and vice versa.
// a pointer is associated with the item in the display that is used for
// sorting.
// {kjp} We could use itemData.GetNotes(CString&) to reduce the number of
// {kjp} temporary objects created and copied.
//
int DboxMain::insertItem(CItemData &itemData, int iIndex) {
	// TODO: sorted insert?
	int iResult = iIndex;
	if (iResult < 0) {
		iResult = m_ctlItemList.GetItemCount();
	}

	CMyString title, username;
	SplitName(itemData.GetName(), title, username);

	iResult = m_ctlItemList.InsertItem(iResult, title);
	if (iResult < 0) {
		// TODO: issue error here...
		return iResult;
	}

	// get only the first line for display
	CMyString strNotes = itemData.GetNotes();
	int iEOL = strNotes.Find('\r');
	if (iEOL >= 0 && iEOL < strNotes.GetLength()) {
		CMyString strTemp = strNotes.Left(iEOL);
		strNotes = strTemp;
	}

	m_ctlItemList.SetItemText(iResult, 1, username);
	m_ctlItemList.SetItemText(iResult, 2, strNotes);
	m_ctlItemList.SetItemData(iResult, (DWORD)&itemData);

	if (m_bShowPasswordInList) {
		m_ctlItemList.SetItemText(iResult, 3, itemData.GetPassword());
	}

	return iResult;
}

int DboxMain::getSelectedItem() {
	POSITION p = m_ctlItemList.GetFirstSelectedItemPosition();
	if (p) {
		return m_ctlItemList.GetNextSelectedItem(p);
	}
	return -1;
}

void DboxMain::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (m_iSortedColumn == pNMListView->iSubItem) {
		m_bSortAscending = !m_bSortAscending;
	}
	else {
		m_iSortedColumn = pNMListView->iSubItem;
		m_bSortAscending = true;
	}
	m_ctlItemList.SortItems(CompareFunc, MAKELPARAM(m_iSortedColumn, (int)m_bSortAscending));

	*pResult = 0;
}


/*
  The following two functions are for use when switching default
  username states.

  Should be run only if usedefuser == TRUE
*/
void
DboxMain::MakeFullNames(CList<CItemData, CItemData>* plist,
			const CMyString &defusername)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);
      //Start MakeFullName
// {kjp} BUGFIX : these are strings, not byte arrays!
//      int pos = temp.FindByte(SPLTCHR);
//      int pos2 = temp.FindByte(DEFUSERCHR);
      int pos = temp.Find(SPLTCHR);
      int pos2 = temp.Find(DEFUSERCHR);
      if (pos==-1 && pos2!=-1)
      {
         //Insert defusername if string contains defchr but not splitchr
         plist->GetAt(listPos).SetName(
            (CMyString)temp.Left(pos2) + SPLTSTR + defusername);
      }
      // End MakeFullName
      plist->GetNext(listPos);
   }
}


//Should only be run on full names...
void
DboxMain::DropDefUsernames(CList<CItemData, CItemData>* plist, const CMyString &defusername)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);
      //Start DropDefUsername
      CMyString temptitle, tempusername;
      int pos = SplitName(temp, temptitle, tempusername);
      if ((pos!=-1) && (tempusername == defusername))
      {
         //If name is splitable and username is default
         plist->GetAt(listPos).SetName(temptitle + DEFUSERCHR);
      }
      //End DropDefUsername
      plist->GetNext(listPos);
   }
}

int
DboxMain::CheckVersion(CList<CItemData, CItemData>* plist)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);

// {kjp} BUGFIX these are strings not byte arrays!
//    if (temp.FindByte(SPLTCHR) != -1)
      if (temp.Find(SPLTCHR) != -1)
         return V15;

      plist->GetNext(listPos);
   }
   
   return V10;
}



void
DboxMain::SetBlankToDef(CList<CItemData, CItemData>* plist)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);

      //Start Check
// {kjp} BUGFIX this is a string not a byte array!
//    if ((temp.FindByte(SPLTCHR) == -1)
//        && (temp.FindByte(DEFUSERCHR) == -1))
      if ((temp.Find(SPLTCHR) == -1)
          && (temp.Find(DEFUSERCHR) == -1))
      {
         plist->GetAt(listPos).SetName(temp + DEFUSERCHR);
      }
      //End Check

      plist->GetNext(listPos);
   }
}


void
DboxMain::SetBlankToName(CList<CItemData, CItemData>* plist, const CMyString &username)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);
      //Start Check
	  // {kjp} BUGFIX this is a string not a byte array
//    if ( (temp.FindByte(SPLTCHR) == -1) && (temp.FindByte(DEFUSERCHR) == -1) )
      if ( (temp.Find(SPLTCHR) == -1) && (temp.Find(DEFUSERCHR) == -1) )
      {
         plist->GetAt(listPos).SetName(temp + SPLTSTR + username);
      }
      //End Check
      plist->GetNext(listPos);
   }
}


BOOL
DboxMain::CheckExtension(const CMyString &name, const CMyString &ext) const
{
   int pos = name.Find(ext);
   return (pos == name.GetLength() - ext.GetLength()); //Is this at the end??
}


int
DboxMain::SplitName(const CMyString &name, CMyString &title, CMyString &username)
//Returns split position for a name that was split and -1 for non-split name
{
// {kjp} BUGFIX we're dealing with strings not bytes
//	int pos = name.FindByte(SPLTCHR);
	int pos = name.Find(SPLTCHR);
	if (pos==-1) //Not a split name
	{
// {kjp} BUGFIX we're dealing with strings not bytes
//		int pos2 = name.FindByte(DEFUSERCHR);
		int pos2 = name.Find(DEFUSERCHR);
		if (pos2 == -1)  //Make certain that you remove the DEFUSERCHR 
		{
			title = name;
		}
		else
		{
			title = CMyString(name.Left(pos2));
		}
		
		if ((pos2 != -1)
			&& (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE)==TRUE))
		{
			username = CMyString(app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T("")));
		}
		else
		{
			username = "";
		}
	}
	else
	{
	/*
	* There should never ever be both a SPLITCHR and a DEFUSERCHR in
	* the same string
		*/
		CMyString temp;
		temp = CMyString(name.Left(pos));
		temp.TrimRight();
		title = temp;
		temp = CMyString(name.Right(name.GetLength() - (pos+1))); // Zero-index string
		temp.TrimLeft();
		username = temp;
	}
	return pos;
}


void
DboxMain::MakeName(CMyString& name, const CMyString &title, const CMyString &username) const
{
   if (username == "")
      name = title;
   else if (((app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE))==TRUE)
            && ((const CString &)username ==
                app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T(""))))
   {
      name = title + DEFUSERCHR;
   }
   else 
   {
      name = title + SPLTSTR + username;
   }
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

		app.WriteProfileInt( _T(PWS_REG_OPTIONS), _T("alwaysontop"), m_bAlwaysOnTop );

		UpdateAlwaysOnTop();
	}
#endif
}


void
DboxMain::ConfigureSystemMenu()
{
#if defined(WIN32_PLATFORM_WFSP) || defined(POCKET_PC)
	SHMENUBARINFO mbi;
                ZeroMemory(&mbi, sizeof(SHMENUBARINFO));

                mbi.cbSize     = sizeof(SHMENUBARINFO);
				mbi.hwndParent = this->m_hWnd;
                mbi.nToolBarId = IDR_MAINMENU;
                mbi.hInstRes   = AfxGetInstanceHandle();
                mbi.dwFlags    = SHCMBF_HMENU;

                if(SHCreateMenuBar(&mbi))
                {
                    m_hwndMb          = mbi.hwndMB;
                    TBBUTTONINFO tbbi = {0};
                    tbbi.cbSize       = sizeof(tbbi);
                    tbbi.dwMask       = TBIF_LPARAM ;

					


					//::SendMessage(m_hwndMb, TB_GETBUTTONINFO, 0, (LPARAM)&tbbi);
                    //m_wndMenu           = CMenu::FromHandle((HMENU)tbbi.lParam);
                }

				// Query the SIP state and size our window appropriately.
				SIPINFO si;
				int cx, cy;
memset (&si, 0, sizeof (si));
si.cbSize = sizeof (si);
SHSipInfo(SPI_GETSIPINFO, 0, (PVOID)&si, FALSE); 

cx = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top;

// If the SIP is not shown, or is showing but not docked, the
// desktop rect doesn't include the height of the menu bar.
if (!(si.fdwFlags & SIPF_ON) ||
    ((si.fdwFlags & SIPF_ON) && !(si.fdwFlags & SIPF_DOCKED))) { 
        RECT rectMB;
		::GetWindowRect (m_hwndMb, &rectMB);
        cy -= (rectMB.bottom - rectMB.top);  
    }
::SetWindowPos (this->m_hWnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);

#endif

#if defined(POCKET_PC)
	/*
	DWORD dwAdornmentFlags = 0;

	
	m_wndCommandBar = new CCommandBar();
	if (!m_wndCommandBar->CreateEx(this) ||
        !m_wndCommandBar->InsertMenuBar(IDR_MAINMENU) ||
        !m_wndCommandBar->AddAdornments(dwAdornmentFlags))
    {
        TRACE0("Failed to create CommandBar\n");
        return;      // fail to create
    }

	m_wndCommandBar->SetBarStyle(m_wndCommandBar->GetBarStyle() |
           CBRS_SIZE_FIXED);

	m_wndMenu = CMenu::FromHandle(m_wndCommandBar->m_hMenu);

	 m_wndCommandBar->Show(TRUE);

	 BOOL res=m_wndCommandBar->DrawMenuBar(IDR_MAINMENU); // Returns TRUE  
  
     DrawMenuBar();  
*/




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
	//app.GetMRU()->UpdateMenu( pCmdUI );	
}

#if _MFC_VER > 1200
BOOL
#else
void 
#endif
DboxMain::OnOpenMRU(UINT nID)
{
	UINT	uMRUItem = nID - ID_FILE_MRU_ENTRY1;

	//CString mruItem = (*app.GetMRU())[uMRUItem];
	//Open( mruItem );
#if _MFC_VER > 1200
	return TRUE;
#endif
}

// helps with MRU by allowing ON_UPDATE_COMMAND_UI
void
DboxMain::OnInitMenuPopup(CMenu* pPopupMenu, UINT, BOOL) 
{
/*
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
*/
}
