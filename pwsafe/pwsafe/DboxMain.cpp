/// \file DboxMain.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

// dialog boxen
#include "DboxMain.h"

#include "ClearQuestionDlg.h"
#include "ConfirmDeleteDlg.h"
#include "AddDlg.h"
#include "EditDlg.h"
#include "FindDlg.h"
#include "PasskeyChangeDlg.h"
#include "PasskeyEntry.h"
#include "PasskeySetup.h"
#include "RemindSaveDlg.h"
#include "QuerySetDef.h"
#include "UsernameEntry.h"
#include "TryAgainDlg.h"

// widget override?
#include "SysColStatic.h"

#include <afxpriv.h>
#include <stdlib.h> // for qsort

/*
 * This is the string to be displayed instead of the actual password, unless
 * the user chooses to see the password:
 */

const TCHAR *HIDDEN_PASSWORD = _T("**************");


//-----------------------------------------------------------------------------
class DboxAbout
   : public CDialog
{
public:
   DboxAbout()
      : CDialog(DboxAbout::IDD)
   {}

   enum { IDD = IDD_ABOUTBOX };

protected:
   virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV support
   {
      CDialog::DoDataExchange(pDX);
   }

protected:
   DECLARE_MESSAGE_MAP()
};

// I don't think we need this, but...
BEGIN_MESSAGE_MAP(DboxAbout, CDialog)
END_MESSAGE_MAP()

  /*
   * Compare function used by m_ctlItemList.SortItems()
   * "The comparison function must return a negative value if the first item should precede 
   * the second, a positive value if the first item should follow the second, or zero if
   * the two items are equivalent."
   *
   * If sorting is by title (username) , username (title) is the secondary field if the
   * primary fields are identical.
   */
int CALLBACK DboxMain::CompareFunc(LPARAM lParam1, LPARAM lParam2,
				   LPARAM closure)
{
  // closure is "this" of the calling DboxMain, from which we use:
  // m_iSortedColumn to determine which column is getting sorted:
  // 0 - title
  // 1 - user name
  // 2 - note
  // 3 - password
  // m_bSortAscending to determine the direction of the sort (duh)
  // m_core.SplitName()

  DboxMain *self = (DboxMain*)closure;
	const int	nRecurseFlag		= 500; // added to the desired sort column when recursing
	bool		bAlreadyRecursed	= false;
	int		nSortColumn		= self->m_iSortedColumn;
	CItemData*	pLHS			= (CItemData *)lParam1;
	CItemData*	pRHS			= (CItemData *)lParam2;
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
		self->m_core.SplitName(pLHS->GetName(), title1, username1);
		self->m_core.SplitName(pRHS->GetName(), title2, username2);
		iResult = ((CString)title1).CompareNoCase(title2);
		if (iResult == 0 && !bAlreadyRecursed) {
		  // making a recursed call, add nRecurseFlag
		  const int savedSortColumn = self->m_iSortedColumn;
		  self->m_iSortedColumn = 1 + nRecurseFlag;
		  iResult = CompareFunc(lParam1, lParam2, closure);
		  self->m_iSortedColumn = savedSortColumn;
		}
		break;
	case 1:
		self->m_core.SplitName(pLHS->GetName(), title1, username1);
		self->m_core.SplitName(pRHS->GetName(), title2, username2);
		iResult = ((CString)username1).CompareNoCase(username2);
		if (iResult == 0 && !bAlreadyRecursed) {
		  // making a recursed call, add nRecurseFlag
		  const int savedSortColumn = self->m_iSortedColumn;
		  self->m_iSortedColumn = 0 + nRecurseFlag;
		  iResult = CompareFunc(lParam1, lParam2, closure);
		  self->m_iSortedColumn = savedSortColumn;
		}
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
	if (!self->m_bSortAscending) {
		iResult *= -1;
	}
	return iResult;
}

//-----------------------------------------------------------------------------
DboxMain::DboxMain(CWnd* pParent)
   : CDialog(DboxMain::IDD, pParent),
     m_bSizing( false ), m_needsreading(true), m_windowok(false),
     m_existingrestore(FALSE), m_toolbarsSetup(FALSE),
     m_bShowPasswordInEdit(false), m_bShowPasswordInList(false),
     m_bSortAscending(true), m_iSortedColumn(0)
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
     // If there's no registry key, this is probably a fresh install.
     // CheckPassword will catch this and handle it correctly
     m_core.SetCurFile((CMyString) app.GetProfileString(_T(""),
							_T("currentfile")));
   }

   m_currbackup = // ??? move to PWScore??
      (CMyString) app.GetProfileString(_T(""), _T("currentbackup"), NULL);
   m_title = _T("");

   m_bAlwaysOnTop = app.GetProfileInt(_T(""), _T("alwaysontop"), FALSE);
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
   ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_KEYDOWN, IDC_ITEMLIST, OnKeydownItemlist)
	ON_NOTIFY(NM_DBLCLK, IDC_ITEMLIST, OnListDoubleClick)
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
   ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
	ON_NOTIFY(NM_SETFOCUS, IDC_ITEMLIST, OnSetfocusItemlist)
	ON_NOTIFY(NM_KILLFOCUS, IDC_ITEMLIST, OnKillfocusItemlist)
   ON_WM_DROPFILES()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ITEMLIST, OnColumnClick)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_ENTRY1, OnUpdateMRU)
	ON_WM_INITMENUPOPUP()
   ON_COMMAND(ID_MENUITEM_EXIT, OnOK)
   ON_COMMAND(ID_TOOLBUTTON_ADD, OnAdd)
   ON_COMMAND(ID_TOOLBUTTON_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_TOOLBUTTON_COPYUSERNAME, OnCopyUsername)
   ON_COMMAND(ID_TOOLBUTTON_CLEARCLIPBOARD, OnClearclipboard)
   ON_COMMAND(ID_TOOLBUTTON_DELETE, OnDelete)
   ON_COMMAND(ID_TOOLBUTTON_EDIT, OnEdit)
   ON_COMMAND(ID_TOOLBUTTON_NEW, OnNew)
   ON_COMMAND(ID_TOOLBUTTON_OPEN, OnOpen)
   ON_COMMAND(ID_TOOLBUTTON_SAVE, OnSave)
   ON_WM_SYSCOMMAND()
   ON_BN_CLICKED(IDOK, OnEdit)
	ON_WM_SIZING()
	//}}AFX_MSG_MAP

	ON_COMMAND_EX_RANGE(ID_FILE_MRU_ENTRY1, ID_FILE_MRU_ENTRY20, OnOpenMRU)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
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
	m_ctlItemList.InsertColumn(0, "Title");
	m_ctlItemList.InsertColumn(1, "User Name");
	m_ctlItemList.InsertColumn(2, "Notes");

	if (app.GetProfileInt("", "showpwdefault", FALSE)) {
		m_bShowPasswordInEdit = true;
	}

	if (app.GetProfileInt("", "showpwinlist", FALSE)) {
		m_bShowPasswordInList = true;
	}

	CRect rect;
	m_ctlItemList.GetClientRect(&rect);
	int i1stWidth = app.GetProfileInt("", "column1width", rect.Width() / iColumnCount + rect.Width() % iColumnCount);
	int i2ndWidth = app.GetProfileInt("", "column2width", rect.Width() / iColumnCount);
	int i3rdWidth = app.GetProfileInt("", "column3width", rect.Width() / iColumnCount);

	m_ctlItemList.SetColumnWidth(0, i1stWidth);
	m_ctlItemList.SetColumnWidth(1, i2ndWidth);
	m_ctlItemList.SetColumnWidth(2, i3rdWidth);

	m_iSortedColumn = app.GetProfileInt("", "sortedcolumn", 0);
	m_bSortAscending = app.GetProfileInt("", "sortascending", 1)? true: false;

	// refresh list will add and size password column if necessary...
	RefreshList();

   ChangeOkUpdate();

   setupBars(); // Just to keep things a little bit cleaner

   DragAcceptFiles(TRUE);

   // TODO: kinda hideous in the registry, encode as single string maybe?
   rect.top = app.GetProfileInt("", "top", -1);
   rect.bottom = app.GetProfileInt("", "bottom", -1);
   rect.left = app.GetProfileInt("", "left", -1);
   rect.right = app.GetProfileInt("", "right", -1);

   if (rect.top == -1 || rect.bottom == -1 || rect.left == -1 || rect.right == -1) {
	   GetWindowRect(&rect);
	   SendMessage(WM_SIZE, SIZE_RESTORED, MAKEWPARAM(rect.Width(), rect.Height()));
   }
   else {
		MoveWindow(&rect, TRUE);
   }

   UINT usedefuser = app.GetProfileInt("", "usedefuser", FALSE);
   m_core.SetUseDefUser(usedefuser == TRUE); // plain usedefuser generates bogus compiler warning. grrrr
   m_core.SetDefUsername(app.GetProfileString("", "defusername", ""));

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

   rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, true);

   switch (rc)
   {
   case PWScore::SUCCESS:
      rc2 = m_core.ReadCurFile(passkey);
      m_title = "Password Safe - " + m_core.GetCurFile();
      break; 
   case PWScore::CANT_OPEN_FILE:
      /*
       * If it is the default filename, assume that this is the first time
       * that they are starting Password Safe and don't confusing them.
       */
      // currently falls thru to...
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
      rc2 = PWScore::NOT_SUCCESS;
      break;
   default:
      rc2 = PWScore::NOT_SUCCESS;
      break;
   }

   if (rc2 == PWScore::SUCCESS)
   {
      m_existingrestore = FALSE;
      m_needsreading = false;
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

      SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
   if (m_core.GetUseDefUser())
   {
      dataDlg.m_username = m_core.GetDefUsername();
   }

   app.DisableAccelerator();
   int rc = dataDlg.DoModal();
   app.EnableAccelerator();
	
   if (rc == IDOK)
   {
      //Check if they wish to set a default username
      if (!m_core.GetUseDefUser()
          && (app.GetProfileInt("", "querysetdef", TRUE) == TRUE)
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
            app.WriteProfileInt("", "usedefuser", TRUE);
            app.WriteProfileString("", "defusername",
                                   dataDlg.m_username);
	    m_core.SetUseDefUser(true);
	    m_core.SetDefUsername(dataDlg.m_username);
            m_core.DropDefUsernames(dataDlg.m_username);
            RefreshList();
         }
      }
      //Finish Check (Does that make any geographical sense?)
      CItemData temp;
      CMyString temptitle;
      m_core.MakeName(temptitle, dataDlg.m_title, dataDlg.m_username);
      temp.SetName(temptitle);
      temp.SetPassword(dataDlg.m_password);
      temp.SetNotes(dataDlg.m_notes);
      m_core.AddEntryToTail(temp);
      int newpos = insertItem(temp);
      SelectEntry(newpos);
      m_ctlItemList.SetFocus();
      if (app.GetProfileInt("", "saveimmediately", FALSE) == TRUE)
      {
         Save();
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
		
      CMyString curPassString = m_core.GetEntryAt(itemPos).GetPassword();

      uGlobalMemSize = curPassString.GetLength()+1;
      hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, uGlobalMemSize);
      char* pGlobalLock = (char*)GlobalLock(hGlobalMemory);

      memcpy(pGlobalLock, curPassString, curPassString.GetLength());
		
      pGlobalLock[uGlobalMemSize-1] = '\0';
      GlobalUnlock(hGlobalMemory);	
		
      if (OpenClipboard() == TRUE)
      {
         if (EmptyClipboard()!=TRUE)
            AfxMessageBox("The clipboard was not emptied correctly");
         if (SetClipboardData(CF_TEXT, hGlobalMemory) == NULL)
            AfxMessageBox("The data was not pasted into the clipboard "
                          "correctly");
         if (CloseClipboard() != TRUE)
            AfxMessageBox("The clipboard could not be closed");
      }
      else
         AfxMessageBox("The clipboard could not be opened correctly");
		
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
         int curSel = getSelectedItem();
         POSITION listindex = Find(curSel); // Must Find before delete from m_ctlItemList
	 m_ctlItemList.DeleteItem(curSel);
         m_core.RemoveEntryAt(listindex);
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
      CItemData item = m_core.GetEntryAt(listindex);

      CEditDlg dlg_edit(this);
      m_core.SplitName(item.GetName(),
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
         m_core.MakeName(temptitle, dlg_edit.m_title, dlg_edit.m_username);
         item.SetName(temptitle);

         item.SetPassword(dlg_edit.m_realpassword);
         item.SetNotes(dlg_edit.m_notes);

         /*
           Out with the old, in with the new
         */
         m_core.RemoveEntryAt(listindex);
         m_core.AddEntryToTail(item);
	 m_ctlItemList.DeleteItem(curSel);
	 insertItem(item);
         if (app.GetProfileInt("", "saveimmediately", FALSE) == TRUE)
         {
            Save();
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
			app.WriteProfileInt("", ppszAttributeNames[i], lvColumn.cx);
		}
	}

	if (!IsIconic()) {
	  CRect rect;
	  GetWindowRect(&rect);
	  app.WriteProfileInt("", "top", rect.top);
	  app.WriteProfileInt("", "bottom", rect.bottom);
	  app.WriteProfileInt("", "left", rect.left);
	  app.WriteProfileInt("", "right", rect.right);
	}
	app.WriteProfileInt("", "sortedcolumn", m_iSortedColumn);
	app.WriteProfileInt("", "sortascending", m_bSortAscending);

   if (m_core.IsChanged())
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
         if (rc2 != PWScore::SUCCESS)
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
   if (!m_core.GetCurFile().IsEmpty())
      app.WriteProfileString("", "currentfile", m_core.GetCurFile());
   else
      app.WriteProfileString("", "currentfile", NULL);

   if (!m_currbackup.IsEmpty())
      app.WriteProfileString("", "currentbackup", m_currbackup);
   else
      app.WriteProfileString("", "currentbackup", NULL);

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
      AfxMessageBox("The clipboard could not be opened correctly");

   if (IsClipboardFormatAvailable(CF_TEXT) != 0)
   {
      HGLOBAL hglb = GetClipboardData(CF_TEXT); 
      if (hglb != NULL)
      {
         LPTSTR lptstr = (LPTSTR)GlobalLock(hglb); 
         if (lptstr != NULL)
         {
            trashMemory((unsigned char*)lptstr, strlen(lptstr));
            GlobalUnlock(hglb); 
         } 
      } 
   }
   if (EmptyClipboard()!=TRUE)
      AfxMessageBox("The clipboard was not emptied correctly");
   if (CloseClipboard() != TRUE)
      AfxMessageBox("The clipboard could not be closed");
}


 // Find in m_pwlist entry with same title and user name as the i'th entry in m_ctlItemList
POSITION DboxMain::Find(int i)
{
  const CMyString curTitle = m_ctlItemList.GetItemText(i, 0);
  const CMyString curUser = m_ctlItemList.GetItemText(i, 1);
  return Find(curTitle, curUser);
}


// for qsort in FindAll
static int compint(const void *a1, const void *a2)
{
  // since we're sorting a list of indices, v1 == v2 should never happen.
  const int v1 = *(int *)a1, v2 = *(int *)a2;
  ASSERT(v1 != v2);
  return (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;
}

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

  POSITION listPos = m_core.GetFirstEntryPosition();
  CMyString curname, savecurname, curnotes;
  CString searchstr(str); // Since str is const, and we might need to MakeLower
  const int NumEntries = m_core.GetNumEntries();
  bool *matchVector = new bool[NumEntries];
  int retval = 0;
  int i;

  if (!CaseSensitive)
    searchstr.MakeLower();

  for (i = 0; i < NumEntries; i++)
    matchVector[i] = false;

  while (listPos != NULL)
  {
      curname = m_core.GetEntryAt(listPos).GetName();
      savecurname = curname; // keep original for finding in m_listctrl
      curnotes = m_core.GetEntryAt(listPos).GetNotes();

      if (!CaseSensitive) {
          curname.MakeLower();
          curnotes.MakeLower();
      }
      if (::strstr(curname, searchstr) || ::strstr(curnotes, searchstr)) {
	// Find index in m_listctl
	CMyString listTitle;
	CMyString title, username;
	m_core.SplitName(savecurname, title, username);
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
      m_core.GetNextEntry(listPos);
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
  retval = m_ctlItemList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
  if (MakeVisible)
    m_ctlItemList.EnsureVisible(i, FALSE);
  return retval;
}


//Updates m_listctrl from m_pwlist
void
DboxMain::RefreshList()
{
   if (! m_windowok)
      return;

   //Copy the data
   m_ctlItemList.DeleteAllItems();

	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_WIDTH;

	bool bPasswordColumnShowing = m_ctlItemList.GetColumn(3, &lvColumn)? true: false;
	if (m_bShowPasswordInList && !bPasswordColumnShowing) {
		m_ctlItemList.InsertColumn(3, "Password");
		CRect rect;
		m_ctlItemList.GetClientRect(&rect);
		m_ctlItemList.SetColumnWidth(3, app.GetProfileInt("", "column4width", rect.Width() / 4));
	}
	else if (!m_bShowPasswordInList && bPasswordColumnShowing) {
		app.WriteProfileInt("", "column4width", lvColumn.cx);
		m_ctlItemList.DeleteColumn(3);
	}

   POSITION listPos = m_core.GetFirstEntryPosition();
   while (listPos != NULL)
   {
     insertItem(m_core.GetEntryAt(listPos));
     m_core.GetNextEntry(listPos);
   }

   m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);

   //Setup the selection
   if (m_ctlItemList.GetItemCount() > 0 && getSelectedItem() < 0) {
      SelectEntry(0);
   }
}

void
DboxMain::OnPasswordChange() 
{
   CPasskeyChangeDlg changeDlg(this);
   int rc = changeDlg.DoModal();
   if (rc == IDOK)
   {
     m_core.ChangePassword(changeDlg.m_newpasskey);
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
	CDialog::OnSizing(fwSide, pRect);
	
	m_bSizing = true;
}

void
DboxMain::OnSize(UINT nType,
                 int cx,
                 int cy) 
//Note that onsize runs before InitDialog (Gee, I love MFC)
{
   CDialog::OnSize(nType, cx, cy);

   if (nType == SIZE_MINIMIZED)
   {
	   m_ctlItemList.DeleteAllItems();
      if (app.GetProfileInt("",
                            "dontaskminimizeclearyesno",
                            FALSE) == TRUE)
      {
         ClearClipboard();
      }
      if (app.GetProfileInt("", "databaseclear", FALSE) == TRUE)
      {
         BOOL dontask = app.GetProfileInt("",
                                          "dontasksaveminimize",
                                          FALSE);
         BOOL doit = TRUE;
         if ((m_core.IsChanged())
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
            m_needsreading = true;
         }
      }
   }
   else if (!m_bSizing && nType == SIZE_RESTORED)	// gets called even when just resizing window
   {
      if ((m_needsreading)
          && (m_existingrestore == FALSE)
          && (m_windowok))
      {
         m_existingrestore = TRUE;

         CMyString passkey;
         int rc, rc2;
         CMyString temp;

         rc = GetAndCheckPassword(m_core.GetCurFile(), passkey);
         switch (rc)
         {
         case PWScore::SUCCESS:
            rc2 = m_core.ReadCurFile(passkey);
            m_title = "Password Safe - " + m_core.GetCurFile();
            break; 
         case PWScore::CANT_OPEN_FILE:
            temp =
	      m_core.GetCurFile()
               + "\n\nCannot open database. It likely does not exist."
               + "\nA new database will be created.";
            MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
         case TAR_NEW:
            rc2 = New();
            break;
         case TAR_OPEN:
            rc2 = Open();
            break;
         case PWScore::WRONG_PASSWORD:
            rc2 = PWScore::NOT_SUCCESS;
            break;
         default:
            rc2 = PWScore::NOT_SUCCESS;
            break;
         }

         if (rc2 == PWScore::SUCCESS)
         {
            m_needsreading = false;
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
   }

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

   if (m_core.GetCurFile().IsEmpty())
      return SaveAs();

   rc = m_core.WriteCurFile();

   if (rc == PWScore::CANT_OPEN_FILE)
   {
      CMyString temp = m_core.GetCurFile() + "\n\nCould not open file for writting!";
      MessageBox(temp, "File write error.", MB_OK|MB_ICONWARNING);
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

   GetMenu()->EnableMenuItem(ID_MENUITEM_SAVE,
			     m_core.IsChanged() ? MF_ENABLED : MF_GRAYED);

   /*
     This doesn't exactly belong here, but it makes sure that the
     title is fresh...
   */
   SetWindowText(LPCTSTR(m_title));
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
   title = m_core.GetEntryAt(itemPos).GetName();
   m_core.SplitName(title, junk, username);

   if (username.GetLength() == 0)
   {
      AfxMessageBox(_T("There is no username associated with this item."));
   }
   else
   {
      uGlobalMemSize = username.GetLength()+1;
      hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                                  uGlobalMemSize);
      char* pGlobalLock = (char*)GlobalLock(hGlobalMemory);
      
      memcpy(pGlobalLock, username, username.GetLength());

      pGlobalLock[uGlobalMemSize-1] = '\0';
      GlobalUnlock(hGlobalMemory);	
		
      if (OpenClipboard() == TRUE)
      {
         if (EmptyClipboard()!=TRUE)
            AfxMessageBox("The clipboard was not emptied correctly");
         if (SetClipboardData(CF_TEXT, hGlobalMemory) == NULL)
            AfxMessageBox("The data was not pasted into the "
                          "clipboard correctly");
         if (CloseClipboard() != TRUE)
            AfxMessageBox("The clipboard could not be closed");
      }
      else
      {
         AfxMessageBox("The clipboard could not be opened correctly");
      }
      //No need to remind the user about clipboard security
      //as this is only a username
   }
}


void
DboxMain::OnContextMenu(CWnd *, CPoint point) 
{
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

         pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
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
                     "bak",
                     m_currbackup,
                     OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                     | OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                     "Password Safe Backups (*.bak)|*.bak||",
                     this);
      fd.m_ofn.lpstrTitle = "Please Choose a Name for this Backup:";

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
      CMyString temp = tempname + "\n\nCould not open file for writting!";
      MessageBox(temp, "File write error.", MB_OK|MB_ICONWARNING);
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
                     "dat",
                     NULL,
                     OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                     "Password Safe Databases (*.dat)|*.dat|"
                     "Password Safe Backups (*.bak)|*.bak|"
                     "All files (*.*)|*.*|"
                     "|",
                     this);
      fd.m_ofn.lpstrTitle = "Please Choose a Database to Open:";
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
DboxMain::Open( const char* pszFilename )
{
	int rc;
	CMyString passkey, temp;

	//Check that this file isn't already open
	if (pszFilename == m_core.GetCurFile() && !m_needsreading)
	{
		//It is the same damn file
		MessageBox("That file is already open.",
			"Oops!",
			MB_OK|MB_ICONWARNING);
		return PWScore::ALREADY_OPEN;
	}
	
	if (m_core.IsChanged())
	{
		int rc2;
		
		temp =
			"Do you want to save changes to the password database: "
		  + m_core.GetCurFile()
			+ "?";
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
		app.GetMRU()->Add( pszFilename );
		break; // Keep going... 
	case PWScore::CANT_OPEN_FILE:
		temp = m_core.GetCurFile()
		  + "\n\nCan't open file. Please choose another.";
		MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
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
		temp += "\n\nCould not open file for reading!";
		MessageBox(temp, "File read error.", MB_OK|MB_ICONWARNING);
		/*
		Everything stays as is... Worst case,
		they saved their file....
		*/
		return PWScore::CANT_OPEN_FILE;
	}
	
	m_core.SetCurFile(pszFilename);
	m_title = "Password Safe - " + m_core.GetCurFile();
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
         "Do you want to save changes to the password database: "
	+ m_core.GetCurFile()
         + "?";

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
           Make sure that writting the file was successful
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
   m_title = "Password Safe - <Untitled>";
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
                     "bak",
                     m_currbackup,
                     OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                     "Password Safe Backups (*.bak)|*.bak||",
                     this);
      fd.m_ofn.lpstrTitle = "Please Choose a Backup to Restore:";
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
	+ "\n\nCan't open file. Please choose another.";
      MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
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
	
      temp = "Do you want to save changes to the password list: "
         + m_core.GetCurFile() + "?";

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
      temp = newback + "\n\nCould not open file for reading!";
      MessageBox(temp, "File read error.", MB_OK|MB_ICONWARNING);
      //Everything stays as is... Worst case, they saved their file....
      return PWScore::CANT_OPEN_FILE;
   }
	
   m_core.SetCurFile(""); //Force a save as...
   m_core.SetChanged(true); //So that the *.dat version of the file will be saved.
   m_title = "Password Safe - <Untitled Restored Backup>";
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
      if (m_core.GetCurFile().IsEmpty())
         fd.m_ofn.lpstrTitle =
            "Please Choose a Name for the Current (Untitled) Database:";
      else
         fd.m_ofn.lpstrTitle =
            "Please Choose a New Name for the Current Database:";
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
      CMyString temp = newfile + "\n\nCould not open file for writing!";
      MessageBox(temp, "File write error.", MB_OK|MB_ICONWARNING);
      return PWScore::CANT_OPEN_FILE;
   }

   m_core.SetCurFile(newfile);
   m_title = "Password Safe - " + m_core.GetCurFile();
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
	CString Errmess(_T("Can't open database "));
	Errmess += (const CString&)filename;
	MessageBox(Errmess, _T("File open error"),
		   MB_OK | MB_ICONWARNING);
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
   return PWScore::SUCCESS;
}

void
DboxMain::ClearData(void)
{
  m_core.ClearData();
   //Because GetText returns a copy, we cannot do anything about the names
   if (m_windowok)
      //Have to make sure this doesn't cause an access violation
      m_ctlItemList.DeleteAllItems();
}

BOOL
DboxMain::OnToolTipText(UINT,
                        NMHDR* pNMHDR,
                        LRESULT* pResult)
// This code is copied from the DLGCBR32 example that comes with MFC
{
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

   return TRUE;    // message was handled
}


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


void
DboxMain::OnDropFiles(HDROP hDrop)
{
   //SetActiveWindow();
   SetForegroundWindow();

   MessageBox("go away you silly git", "File drop", MB_OK);

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

////////////////////////////////////////////////////////////////////////////////
// NOTE!
// itemData must be the actual item in the item list.  if the item is remove
// from the list, it must be removed from the display as well and vice versa.
// a pointer is associated with the item in the display that is used for
// sorting.
//
int DboxMain::insertItem(CItemData &itemData, int iIndex) {
	// TODO: sorted insert?
	int iResult = iIndex;
	if (iResult < 0) {
		iResult = m_ctlItemList.GetItemCount();
	}

	CMyString title, username;
	m_core.SplitName(itemData.GetName(), title, username);

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
	m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);

	*pResult = 0;
}



BOOL
DboxMain::CheckExtension(const CMyString &name, const CMyString &ext) const
{
   int pos = name.Find(ext);
   return (pos == name.GetLength() - ext.GetLength()); //Is this at the end??
}

void
DboxMain::UpdateAlwaysOnTop()
{
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
}

void 
DboxMain::OnSysCommand( UINT nID, LPARAM lParam )
{
	CDialog::OnSysCommand( nID, lParam );

	if ( ID_SYSMENU_ALWAYSONTOP == nID )
	{
		m_bAlwaysOnTop = !m_bAlwaysOnTop;

		app.WriteProfileInt( "", "alwaysontop", m_bAlwaysOnTop );

		UpdateAlwaysOnTop();
	}
}


void
DboxMain::ConfigureSystemMenu()
{
	CMenu*	sysMenu = GetSystemMenu( FALSE );
	CString	str;

	str.LoadString( IDS_ALWAYSONTOP );

	sysMenu->InsertMenu( 5, MF_BYPOSITION | MF_STRING, ID_SYSMENU_ALWAYSONTOP, (LPCTSTR)str );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void
DboxMain::OnUpdateMRU(CCmdUI* pCmdUI) 
{
	app.GetMRU()->UpdateMenu( pCmdUI );	
}

void 
DboxMain::OnOpenMRU(UINT nID)
{
	UINT	uMRUItem = nID - ID_FILE_MRU_ENTRY1;

	CString mruItem = (*app.GetMRU())[uMRUItem];

	Open( mruItem );
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
	
	HMENU hParentMenu;
	if(AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
	else if((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if(pParent != NULL && (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if(::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
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
