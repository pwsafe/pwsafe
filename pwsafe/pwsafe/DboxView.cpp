/// file DboxView.cpp
//
// View-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include <errno.h>
  #include "resource.h"
#endif

#include "DboxMain.h"
#include "AddDlg.h"
#include "ConfirmDeleteDlg.h"
#include "EditDlg.h"
#include "QuerySetDef.h"
#include "RemindSaveDlg.h"
#include "TryAgainDlg.h"

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
    title1 = pLHS->GetTitle();
    title2 = pRHS->GetTitle();
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
    username1 = pLHS->GetUser();
    username2 = pRHS->GetUser();
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

void
DboxMain::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(DboxMain)
  DDX_Control(pDX, IDC_ITEMLIST, m_ctlItemList);
  DDX_Control(pDX, IDC_ITEMTREE, m_ctlItemTree);
  //}}AFX_DATA_MAP
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
      temp.CreateUUID();
      temp.SetName(temptitle);
      temp.SetPassword(dataDlg.m_password);
      temp.SetNotes(dataDlg.m_notes);
      m_core.AddEntryToTail(temp);
      int newpos = insertItem(temp);
      SelectEntry(newpos);
      m_ctlItemList.SetFocus();
      if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), FALSE) == TRUE)
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
DboxMain::OnEdit() 
{
  if (SelItemOk() == TRUE)
    {
      int curSel = getSelectedItem();
		
      POSITION listindex = Find(curSel);
      CItemData item = m_core.GetEntryAt(listindex);

      CEditDlg dlg_edit(this);
      dlg_edit.m_title = item.GetTitle();
      dlg_edit.m_username = item.GetUser();
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
	  if (app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), FALSE) == TRUE)
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
      app.WriteProfileInt(_T(PWS_REG_OPTIONS), (LPCTSTR) ppszAttributeNames[i], lvColumn.cx);
    }
  }

  if (!IsIconic()) {
    CRect rect;
    GetWindowRect(&rect);
#if !defined(POCKET_PC)
    app.WriteProfileInt(_T(PWS_REG_POSITION), _T("top"), rect.top);
    app.WriteProfileInt(_T(PWS_REG_POSITION), _T("bottom"), rect.bottom);
    app.WriteProfileInt(_T(PWS_REG_POSITION), _T("left"), rect.left);
    app.WriteProfileInt(_T(PWS_REG_POSITION), _T("right"), rect.right);
#endif
  }
  app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("sortedcolumn"), m_iSortedColumn);
  app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("sortascending"), m_bSortAscending);

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
      if (app.GetProfileInt(_T(""), _T("dontaskminimizeclearyesno"), FALSE) == TRUE)
	ClearClipboard();
      app.m_pMainWnd = NULL;
    }

  ClearData();

  //Store current filename for next time...
  if (!m_core.GetCurFile().IsEmpty())
    app.WriteProfileString(_T(PWS_REG_OPTIONS), _T("currentfile"), m_core.GetCurFile());
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

  POSITION listPos = m_core.GetFirstEntryPosition();
  CMyString curtitle, curuser, curnotes, savetitle;
  CMyString listTitle;
  CString searchstr(str); // Since str is const, and we might need to MakeLower
  const int NumEntries = m_core.GetNumEntries();
  bool *matchVector = new bool[NumEntries];
  int retval = 0;
  int i;

  if (!CaseSensitive)
    searchstr.MakeLower();

  for (i = 0; i < NumEntries; i++)
    matchVector[i] = false;

  // XXX Come to think of it, why are we searching twice, once
  // XXX in the data structure and once in the display list?
  // XXX Change to search only the latter!

  while (listPos != NULL)
  {
      const CItemData &curitem = m_core.GetEntryAt(listPos);
      savetitle = curtitle = curitem.GetTitle(); // savetitle keeps orig case
      curuser =  curitem.GetUser();
      curnotes = curitem.GetNotes();

      if (!CaseSensitive) {
          curtitle.MakeLower();
          curuser.MakeLower();
          curnotes.MakeLower();
      }
      if (::strstr(curtitle, searchstr) ||
	  ::strstr(curuser, searchstr) ||
	  ::strstr(curnotes, searchstr)) {
	// Find index in displayed list
	for (i = 0; i < NumEntries; i++) {
	  listTitle = CMyString(m_ctlItemList.GetItemText(i, 0));
	  if (listTitle == savetitle && !matchVector[i]) {
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
		m_ctlItemList.SetColumnWidth(3, app.GetProfileInt(_T(PWS_REG_OPTIONS),
								  _T("column4width"),
								  rect.Width() / 4));
	}
	else if (!m_bShowPasswordInList && bPasswordColumnShowing) {
		app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("column4width"), lvColumn.cx);
		m_ctlItemList.DeleteColumn(3);
	}

   POSITION listPos = m_core.GetFirstEntryPosition();
#if defined(POCKET_PC)
   m_ctlItemList.SetRedraw( FALSE );
   SetCursor( waitCursor );
#endif
   while (listPos != NULL)
   {
     insertItem(m_core.GetEntryAt(listPos));
     m_core.GetNextEntry(listPos);
   }

   m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
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
	      if ( m_core.IsChanged() ) // only save if changed
                OnSave();
	      ClearData();
	      m_needsreading = true;
	    }
	}
    }
  else if (!m_bSizing && nType == SIZE_RESTORED)	// gets called even when just resizing window
    {
#endif

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
#if !defined(POCKET_PC)
	      m_title = _T("Password Safe - ") + m_core.GetCurFile();
#endif
	      break; 
	    case PWScore::CANT_OPEN_FILE:
	      temp =
		m_core.GetCurFile()
		+ "\n\nCannot open database. It likely does not exist."
		+ "\nA new database will be created.";
	      MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
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
	      m_needsreading = TRUE;
	      m_existingrestore = FALSE;
	      ShowWindow( SW_MINIMIZE );
	      return;
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

			pPopup->TrackPopupMenu(
WCE_INS							TPM_LEFTALIGN,
WCE_DEL							TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                                point.x, point.y,
                                this); // use this window for commands
      }
   }
}

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

	CMyString title = itemData.GetTitle();
	CMyString username = itemData.GetUser();

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

void
DboxMain::ClearData(void)
{
  m_core.ClearData();
   //Because GetText returns a copy, we cannot do anything about the names
   if (m_windowok)
      //Have to make sure this doesn't cause an access violation
      m_ctlItemList.DeleteAllItems();
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



void
DboxMain::OnListView() 
{
   CMenu* mmenu = GetMenu();
   CMenu* submenu = mmenu->GetSubMenu(2);

   UINT state = submenu->GetMenuState(ID_MENUITEM_LIST_VIEW, MF_BYCOMMAND);
   ASSERT(state != 0xFFFFFFFF);

   if (state & MF_CHECKED) {
      submenu->CheckMenuItem(ID_MENUITEM_LIST_VIEW, MF_UNCHECKED | MF_BYCOMMAND);
      submenu->CheckMenuItem(ID_MENUITEM_TREE_VIEW, MF_CHECKED | MF_BYCOMMAND);
      SetTreeView();
   } else {
      submenu->CheckMenuItem(ID_MENUITEM_LIST_VIEW, MF_CHECKED | MF_BYCOMMAND);
      submenu->CheckMenuItem(ID_MENUITEM_TREE_VIEW, MF_UNCHECKED | MF_BYCOMMAND);
      SetListView();
   }
}

void
DboxMain::OnTreeView() 
{
   CMenu* mmenu = GetMenu();
   CMenu* submenu = mmenu->GetSubMenu(2);

   UINT state = submenu->GetMenuState(ID_MENUITEM_TREE_VIEW, MF_BYCOMMAND);
   ASSERT(state != 0xFFFFFFFF);

   if (state & MF_CHECKED) {
      submenu->CheckMenuItem(ID_MENUITEM_TREE_VIEW, MF_UNCHECKED | MF_BYCOMMAND);
      submenu->CheckMenuItem(ID_MENUITEM_LIST_VIEW, MF_CHECKED | MF_BYCOMMAND);
      SetListView();
   } else {
      submenu->CheckMenuItem(ID_MENUITEM_TREE_VIEW, MF_CHECKED | MF_BYCOMMAND);
      submenu->CheckMenuItem(ID_MENUITEM_LIST_VIEW, MF_UNCHECKED | MF_BYCOMMAND);
      SetTreeView();
   }
}

void
DboxMain::SetListView()
{
  m_ctlItemTree.ShowWindow(SW_HIDE);
  m_ctlItemList.ShowWindow(SW_SHOW);
}

void
DboxMain::SetTreeView()
{
  m_ctlItemList.ShowWindow(SW_HIDE);
  m_ctlItemTree.ShowWindow(SW_SHOW);
}

