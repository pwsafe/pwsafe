/// file MainView.cpp
//
// View-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "DboxMain.h"
#include "TryAgainDlg.h"

#include "corelib/pwsprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


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
  time_t t1, t2;

  // if the sort column is really big, then we must be being called via recursion
  if ( nSortColumn >= nRecurseFlag )
    {
      bAlreadyRecursed = true;		// prevents further recursion
      nSortColumn -= nRecurseFlag;	// normalizes sort column
    }

  int iResult;
  if (self->m_nColumns == 9) {
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
		case 4:
			pLHS->GetCTime(t1);
			pRHS->GetCTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 5:
			pLHS->GetPMTime(t1);
			pRHS->GetPMTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 6:
			pLHS->GetATime(t1);
			pRHS->GetATime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 7:
			pLHS->GetLTime(t1);
			pRHS->GetLTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 8:
			pLHS->GetRMTime(t1);
			pRHS->GetRMTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
  default:
		    iResult = 0; // should never happen - just keep compiler happy
			ASSERT(FALSE);
	}
  } else {
	switch(nSortColumn) {
		case 0:
			title1 = pLHS->GetTitle();
			title2 = pRHS->GetTitle();
			iResult = ((CString)title1).CompareNoCase(title2);
			if (iResult == 0 && !bAlreadyRecursed) {
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
			pLHS->GetCTime(t1);
			pRHS->GetCTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 4:
			pLHS->GetPMTime(t1);
			pRHS->GetPMTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 5:
			pLHS->GetATime(t1);
			pRHS->GetATime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 6:
			pLHS->GetLTime(t1);
			pRHS->GetLTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 7:
			pLHS->GetRMTime(t1);
			pRHS->GetRMTime(t2);
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		default:
    iResult = 0; // should never happen - just keep compiler happy
    ASSERT(FALSE);
	}
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
DboxMain::SetReadOnly(bool state)
{
	m_IsReadOnly = state;
	if (m_toolbarsSetup == TRUE) {
		m_wndToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBUTTON_ADD, m_IsReadOnly ? FALSE : TRUE);
		m_wndToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBUTTON_DELETE, m_IsReadOnly ? FALSE : TRUE);
		m_wndToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBUTTON_SAVE, m_IsReadOnly ? FALSE : TRUE);
	}
}

void
DboxMain::setupBars()
{
#if !defined(POCKET_PC)
  // This code is copied from the DLGCBR32 example that comes with MFC
  
  // Add the status bar
  if (m_statusBar.Create(this)) {
	  // Set up DoubleClickAction text
	  const int dca = int(PWSprefs::GetInstance()->
		  GetPref(PWSprefs::DoubleClickAction));
	  switch (dca) {
		case PWSprefs::DoubleClickAutoType: statustext[SB_DBLCLICK] = IDS_STATAUTOTYPE; break;
		case PWSprefs::DoubleClickBrowse: statustext[SB_DBLCLICK] = IDS_STATBROWSE; break;
		case PWSprefs::DoubleClickCopyNotes: statustext[SB_DBLCLICK] = IDS_STATCOPYNOTES; break;
		case PWSprefs::DoubleClickCopyPassword: statustext[SB_DBLCLICK] = IDS_STATCOPYPASSWORD; break;
      	case PWSprefs::DoubleClickCopyUsername: statustext[SB_DBLCLICK] = IDS_STATCOPYUSERNAME; break;
		case PWSprefs::DoubleClickViewEdit: statustext[SB_DBLCLICK] = IDS_STATVIEWEDIT; break;
		default: statustext[SB_DBLCLICK] = IDS_STATCOMPANY;
	  }
	  // Set up Configuration text
	  const int iConfigOptions = PWSprefs::GetInstance()->GetConfigOptions();
	  switch (iConfigOptions) {
	   	case PWSprefs::CF_NONE: statustext[SB_CONFIG] = IDS_CONFIG_NONE; break;
	    case PWSprefs::CF_REGISTRY: statustext[SB_CONFIG] = IDS_CONFIG_REGISTRY; break;
	    case PWSprefs::CF_FILE_RW:
		case PWSprefs::CF_FILE_RW_NEW: statustext[SB_CONFIG] = IDS_CONFIG_FILE_RW; break;
	    case PWSprefs::CF_FILE_RO: statustext[SB_CONFIG] = IDS_CONFIG_FILE_RO; break;
    	default: ASSERT(0);
	  }
	  // Set up the rest
	  statustext[SB_MODIFIED] = IDS_MODIFIED;
	  statustext[SB_READONLY] = IDS_READ_ONLY;
	  statustext[SB_NUM_ENT] = IDS_STAT_NUM_IN_DB;

	  // And show
	  m_statusBar.SetIndicators(statustext, SB_TOTAL);

      // Make a sunken or recessed border around the first pane
      m_statusBar.SetPaneInfo(SB_DBLCLICK, m_statusBar.GetItemID(SB_DBLCLICK), SBPS_STRETCH, NULL);
  }             

  // Add the ToolBar.
  if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
                             WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
      !m_wndToolBar.LoadToolBar(IDB_TOOLBAR1))
    {
      TRACE0("Failed to create toolbar\n");
      return;      // fail to create
    }

  // Set toolbar according to graphic capabilities, overridable by user choice.
  CDC* pDC = this->GetDC();
  int NumBits = ( pDC ? pDC->GetDeviceCaps(12 /*BITSPIXEL*/) : 32 );
  if (NumBits < 16 || !PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar))  {
    SetToolbar(ID_MENUITEM_OLD_TOOLBAR);
  } else {
    SetToolbar(ID_MENUITEM_NEW_TOOLBAR);
  }

  // Set flag
  m_toolbarsSetup = TRUE;
  SetReadOnly(m_IsReadOnly);
#endif
}

void DboxMain::UpdateListItemTitle(int lindex, const CString &newTitle)
{
  m_ctlItemList.SetItemText(lindex, 0, newTitle);
}

void DboxMain::UpdateListItemUser(int lindex, const CString &newName)
{
  m_ctlItemList.SetItemText(lindex, 1, newName);
}

 // Find in m_pwlist entry with same title and user name as the i'th entry in m_ctlItemList
POSITION DboxMain::Find(int i)
{
  CItemData *ci = (CItemData *)m_ctlItemList.GetItemData(i);
  ASSERT(ci != NULL);
  const CMyString curGroup = ci->GetGroup();
  const CMyString curTitle = m_ctlItemList.GetItemText(i, 0);
  const CMyString curUser = m_ctlItemList.GetItemText(i, 1);
  return Find(curGroup, curTitle, curUser);
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
 * Finds all entries in m_pwlist that contain str in title, user, group or notes
 * field, returns their sorted indices in m_listctrl via indices, which is
 * assumed to be allocated by caller to DboxMain::GetNumEntries() ints.
 * FindAll returns the number of entries that matched.
 */

int
DboxMain::FindAll(const CString &str, BOOL CaseSensitive, int *indices)
{
  ASSERT(!str.IsEmpty());
  ASSERT(indices != NULL);

  POSITION listPos;
  CMyString curtitle, curuser, curnotes, curgroup, curURL, curAT;
  CMyString listTitle, savetitle;
  CString searchstr(str); // Since str is const, and we might need to MakeLower
  int retval = 0;

  if (!CaseSensitive)
    searchstr.MakeLower();

  if (m_IsListView) {
	listPos = m_core.GetFirstEntryPosition();
	while (listPos != NULL) {
		const  CItemData &curitem = m_core.GetEntryAt(listPos);

		savetitle = curtitle = curitem.GetTitle(); // savetitle keeps orig case
		curuser =  curitem.GetUser();
		curnotes = curitem.GetNotes();
		curgroup = curitem.GetGroup();
		curURL = curitem.GetURL();
		curAT = curitem.GetAutoType();

		if (!CaseSensitive) {
			curtitle.MakeLower();
			curuser.MakeLower();
			curnotes.MakeLower();
			curgroup.MakeLower();
			curURL.MakeLower();
			curAT.MakeLower();
		}
		if (::strstr(curtitle, searchstr) ||
			::strstr(curuser, searchstr) ||
			::strstr(curnotes, searchstr) ||
			::strstr(curgroup, searchstr) ||
			::strstr(curURL, searchstr) ||
			::strstr(curAT, searchstr)) {
			// Find index in displayed list
			DisplayInfo *di = (DisplayInfo *)curitem.GetDisplayInfo();
			ASSERT(di != NULL);
			int li = di->list_index;
			ASSERT(CMyString(m_ctlItemList.GetItemText(li, 0)) == savetitle);
			// add to indices, bump retval
			indices[retval++] = li;
		} // match found in m_pwlist
		m_core.GetNextEntry(listPos);
	} // while
	// Sort indices if in List View
	if (retval > 1)
		::qsort((void *)indices, retval, sizeof(indices[0]), compint);
  } else {
    ItemList sortedItemList;
    MakeSortedItemList(sortedItemList);
    listPos = sortedItemList.GetHeadPosition();
	while (listPos != NULL) {
		const CItemData &curitem = sortedItemList.GetAt(listPos);

	    savetitle = curtitle = curitem.GetTitle(); // savetitle keeps orig case
		curuser =  curitem.GetUser();
		curnotes = curitem.GetNotes();
		curgroup = curitem.GetGroup();
		curURL = curitem.GetURL();
		curAT = curitem.GetAutoType();

		if (!CaseSensitive) {
			curtitle.MakeLower();
			curuser.MakeLower();
			curnotes.MakeLower();
			curgroup.MakeLower();
			curURL.MakeLower();
			curAT.MakeLower();
		}
		if (::strstr(curtitle, searchstr) ||
			::strstr(curuser, searchstr) ||
			::strstr(curnotes, searchstr) ||
			::strstr(curgroup, searchstr) ||
			::strstr(curURL, searchstr) ||
			::strstr(curAT, searchstr)) {
			// Find index in displayed list
			DisplayInfo *di = (DisplayInfo *)curitem.GetDisplayInfo();
			ASSERT(di != NULL);
			int li = di->list_index;
			ASSERT(CMyString(m_ctlItemList.GetItemText(li, 0)) == savetitle);
			// add to indices, bump retval
			indices[retval++] = li;
		} // match found in m_pwlist
		sortedItemList.GetNext(listPos);
    } // while
	sortedItemList.RemoveAll();
  }

  return retval;
}


//Checks and sees if everything works and something is selected
BOOL
DboxMain::SelItemOk()
{
  CItemData *ci = getSelectedItem();
  return (ci == NULL) ? FALSE : TRUE;
}

BOOL DboxMain::SelectEntry(int i, BOOL MakeVisible)
{
  BOOL retval;
  if (m_ctlItemList.GetItemCount() == 0)
    return false;

  if (m_ctlItemList.IsWindowVisible()) {
    retval = m_ctlItemList.SetItemState(i,
                                        LVIS_FOCUSED | LVIS_SELECTED,
                                        LVIS_FOCUSED | LVIS_SELECTED);
    if (MakeVisible) {
      m_ctlItemList.EnsureVisible(i, FALSE);
    }
  } else { //Tree view active
    CItemData *ci = (CItemData *)m_ctlItemList.GetItemData(i);
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    ASSERT(di->list_index == i);

	HTREEITEM hti=m_ctlItemTree.GetSelectedItem();  //was there anything selected before?
	if (hti!=NULL)  //NULL means nothing was selected.
      {   //time to remove the old "fake selection" (a.k.a. drop-hilite) 
		m_ctlItemTree.SetItemState(hti,0,TVIS_DROPHILITED);//make sure to undo "MakeVisible" on the previous selection.
      }


    retval = m_ctlItemTree.SelectItem(di->tree_item);
    if (MakeVisible) {// Following needed to show selection when Find dbox has focus. Ugh.
      m_ctlItemTree.SetItemState(di->tree_item,
                                 TVIS_DROPHILITED | TVIS_SELECTED,
                                 TVIS_DROPHILITED | TVIS_SELECTED);
    }
  }
  return retval;
}


//Updates m_ctlItemList and m_ctlItemTree from m_pwlist
// updates of windows suspended until all data is in.
void
DboxMain::RefreshList()
{
  if (!m_windowok)
    return;

#if defined(POCKET_PC)
  HCURSOR		waitCursor = app.LoadStandardCursor( IDC_WAIT );
#endif

  // can't use LockWindowUpdate 'cause only one window at a time can be locked
  m_ctlItemList.SetRedraw( FALSE );
  m_ctlItemTree.SetRedraw( FALSE );
  m_ctlItemList.DeleteAllItems();
  m_ctlItemTree.DeleteAllItems();

  LVCOLUMN lvColumn;
  lvColumn.mask = LVCF_WIDTH;

  bool bPasswordColumnShowing;
  m_ctlItemList.GetColumn(3, &lvColumn);
  if (m_ctlItemList.GetHeaderCtrl()->GetItemCount() == 9)
  	bPasswordColumnShowing = true;
  else
    bPasswordColumnShowing = false;
  if (m_bShowPasswordInList && !bPasswordColumnShowing) {
    m_ctlItemList.InsertColumn(3, _T("Password"));
	m_nColumns++;
    CRect rect;
    m_ctlItemList.GetClientRect(&rect);
    m_ctlItemList.SetColumnWidth(3,
                                 PWSprefs::GetInstance()->
                                 GetPref(PWSprefs::Column4Width,
                                         rect.Width() / 4));
  }
  else if (!m_bShowPasswordInList && bPasswordColumnShowing) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::Column4Width,
                                     lvColumn.cx);
    m_ctlItemList.DeleteColumn(3);
	m_nColumns--;
  }

  POSITION listPos = m_core.GetFirstEntryPosition();
#if defined(POCKET_PC)
  SetCursor( waitCursor );
#endif
  while (listPos != NULL) {
    CItemData &ci = m_core.GetEntryAt(listPos);
    DisplayInfo *di = (DisplayInfo *)ci.GetDisplayInfo();
    if (di != NULL)
      di->list_index = -1; // easier, but less efficient, to delete di
    insertItem(ci);
    m_core.GetNextEntry(listPos);
  }

  m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
#if defined(POCKET_PC)
  SetCursor( NULL );
#endif
  m_ctlItemTree.RestoreExpanded();
  // re-enable and force redraw!
  m_ctlItemList.SetRedraw( TRUE ); m_ctlItemList.Invalidate();
  m_ctlItemTree.SetRedraw( TRUE ); m_ctlItemTree.Invalidate();

  FixListIndexes();
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
//  Also, OnSize is called AFTER the function has been peformed.
//  To verify IF the fucntion should be done at all, it must be checked in OnSysCommand.
{
  CDialog::OnSize(nType, cx, cy);

  if (m_windowok) {
    // Position the control bars
    CRect rect;
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
    m_ctlItemList.MoveWindow(&rect, TRUE);
    m_ctlItemTree.MoveWindow(&rect, TRUE);
  }

  // {kjp} Only SIZE_RESTORED is supported on Pocket PC.
#if !defined(POCKET_PC)
  if (nType == SIZE_MINIMIZED) {
    PWSprefs *prefs = PWSprefs::GetInstance();

    m_selectedAtMinimize = getSelectedItem();
    m_ctlItemList.DeleteAllItems();
    m_ctlItemTree.DeleteAllItems();

    if (prefs->GetPref(PWSprefs::DontAskMinimizeClearYesNo))
      app.ClearClipboardData();
    if (prefs->GetPref(PWSprefs::DatabaseClear)) {
      if (m_core.IsChanged() ||  m_bTSUpdated)
        if (Save() != PWScore::SUCCESS) {
          // If we don't warn the user, data may be lost!
          MessageBox(_T("Couldn't save database - Please save manually"),
                     _T("Save error"),
                     MB_ICONSTOP);
          ShowWindow(SW_SHOW);
          return;
        }
      ClearData(false);
      m_needsreading = true;
    }
    if (PWSprefs::GetInstance()->
        GetPref(PWSprefs::UseSystemTray)) {      
      app.SetMenuDefaultItem(ID_MENUITEM_UNMINIMIZE);
      ShowWindow(SW_HIDE);
    } 
  } else if (!m_bSizing && nType == SIZE_RESTORED) {
    // gets called even when just resizing window
#endif
    app.SetMenuDefaultItem(ID_MENUITEM_MINIMIZE);
    UnMinimize(false);
    RefreshList();
    if (m_selectedAtMinimize != NULL)
      SelectEntry(((DisplayInfo *)m_selectedAtMinimize->GetDisplayInfo())->list_index, false);
#if !defined(POCKET_PC)
  } // !m_bSizing && nType == SIZE_RESTORED
#endif
  m_bSizing = false;
}

// Called when right-click is invoked in the client area of the window.
void
DboxMain::OnContextMenu(CWnd *, CPoint point) 
{
#if defined(POCKET_PC)
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN;
#else
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;
#endif

  CPoint local = point;
  int item = -1;
  CItemData *itemData = NULL;
  CMenu menu;

  if (m_ctlItemList.IsWindowVisible()) {
    // currently in flattened list view.
    m_ctlItemList.ScreenToClient(&local);
    item = m_ctlItemList.HitTest(local);
    if (item < 0)
      return; // right click on empty list
    itemData = (CItemData *)m_ctlItemList.GetItemData(item);
    int rc = SelectEntry(item);
    if (rc == LB_ERR) {
      return; // ? is this possible ?
    }
    m_ctlItemList.SetFocus();
  } else {
    // currently in tree view
    ASSERT(m_ctlItemTree.IsWindowVisible());
    m_ctlItemTree.ScreenToClient(&local);
    HTREEITEM ti = m_ctlItemTree.HitTest(local);
    if (ti != NULL) {
      itemData = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (itemData != NULL) {
        // right-click was on an item (LEAF)
        DisplayInfo *di = (DisplayInfo *)itemData->GetDisplayInfo();
        ASSERT(di != NULL);
        ASSERT(di->tree_item == ti);
        item = di->list_index;
        m_ctlItemTree.SelectItem(ti); // So that OnEdit gets the right one
      } else {
        // right-click was on a group (NODE)
        m_ctlItemTree.SelectItem(ti); 
        if (menu.LoadMenu(IDR_POPGROUP)) {
          CMenu* pPopup = menu.GetSubMenu(0);
          ASSERT(pPopup != NULL);
          m_TreeViewGroup = CMyString(m_ctlItemTree.GetGroup(ti));
          pPopup->TrackPopupMenu(dwTrackPopupFlags, point.x, point.y, this); // use this window for commands
        }
      }
    } else {
      // not over anything
      if (menu.LoadMenu(IDR_POPTREE)) {
        CMenu* pPopup = menu.GetSubMenu(0);
        ASSERT(pPopup != NULL);
        pPopup->TrackPopupMenu(dwTrackPopupFlags, point.x, point.y, this); // use this window for commands
      }
    }
    m_ctlItemTree.SetFocus();
  } // tree view handling

  if (item >= 0) {
    menu.LoadMenu(IDR_POPMENU);
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    ASSERT(itemData != NULL);

    if (itemData->GetURL().IsEmpty()) {
      ASSERT(itemData->GetURL().IsEmpty());
      pPopup->EnableMenuItem(ID_MENUITEM_BROWSE, MF_GRAYED);
    } else {
      ASSERT(!itemData->GetURL().IsEmpty());
      pPopup->EnableMenuItem(ID_MENUITEM_BROWSE, MF_ENABLED);
    }

    pPopup->TrackPopupMenu(dwTrackPopupFlags, point.x, point.y, this); // use this window for commands

  } // if (item >= 0)
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
DboxMain::OnSetfocusItemlist(NMHDR * , LRESULT * ) 
{
  // Seems excessive to do this all the time
  // Should be done only on Open and on Change (Add, Delete, Modify)
  if (m_toolbarsSetup == TRUE)
    UpdateStatusBar();
}

void
DboxMain::OnKillfocusItemlist( NMHDR *, LRESULT *) 
{
  // Seems excessive to do this all the time
  // Should be done only on Open and on Change (Add, Delete, Modify)
  if (m_toolbarsSetup == TRUE)
    UpdateStatusBar();
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
int DboxMain::insertItem(CItemData &itemData, int iIndex)
{
  if (itemData.GetDisplayInfo() != NULL &&
      ((DisplayInfo *)itemData.GetDisplayInfo())->list_index != -1) {
    // true iff item already displayed
    return iIndex;
  }

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
  DisplayInfo *di = (DisplayInfo *)itemData.GetDisplayInfo();
  if (di == NULL)
    di = new DisplayInfo;
  di->list_index = iResult;
  {
    HTREEITEM ti;
    CMyString treeDispString = title;
    CMyString user = itemData.GetUser();
    treeDispString += _T(" [");
    treeDispString += user;
    treeDispString += _T("]");
    if (m_bShowPasswordInList) {
		CMyString newPassword = itemData.GetPassword();
		treeDispString += _T(" [");
		treeDispString += newPassword;
		treeDispString += _T("]");
	}
    // get path, create if necessary, add title as last node
    ti = m_ctlItemTree.AddGroup(itemData.GetGroup());
    ti = m_ctlItemTree.InsertItem(treeDispString, ti, TVI_SORT);
    time_t now, tLTime;
    time(&now);
    itemData.GetLTime(tLTime);
	if (tLTime != 0 && tLTime < now) {
    	m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::EXPIRED_LEAF, CMyTreeCtrl::EXPIRED_LEAF);
	} else {
		m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::LEAF, CMyTreeCtrl::LEAF);
	}
    m_ctlItemTree.SetItemData(ti, (DWORD)&itemData);
    di->tree_item = ti;
  }

  itemData.SetDisplayInfo((void *)di);
  // get only the first line for display
  CMyString strNotes = itemData.GetNotes();
  int iEOL = strNotes.Find('\r');
  if (iEOL >= 0 && iEOL < strNotes.GetLength()) {
    CMyString strTemp = strNotes.Left(iEOL);
    strNotes = strTemp;
  }

  m_ctlItemList.SetItemText(iResult, 1, username);
  m_ctlItemList.SetItemText(iResult, 2, strNotes);

  if (m_bShowPasswordInList) {
    m_ctlItemList.SetItemText(iResult, 3, itemData.GetPassword());
    m_ctlItemList.SetItemText(iResult, 4, itemData.GetCTimeN());
    m_ctlItemList.SetItemText(iResult, 5, itemData.GetPMTimeN());
    m_ctlItemList.SetItemText(iResult, 6, itemData.GetATimeN());
    m_ctlItemList.SetItemText(iResult, 7, itemData.GetLTimeN());
    m_ctlItemList.SetItemText(iResult, 8, itemData.GetRMTimeN());
  } else {
  	m_ctlItemList.SetItemText(iResult, 3, itemData.GetCTimeN());
  	m_ctlItemList.SetItemText(iResult, 4, itemData.GetPMTimeN());
  	m_ctlItemList.SetItemText(iResult, 5, itemData.GetATimeN());
  	m_ctlItemList.SetItemText(iResult, 6, itemData.GetLTimeN());
  	m_ctlItemList.SetItemText(iResult, 7, itemData.GetRMTimeN());
  }
  m_ctlItemList.SetItemData(iResult, (DWORD)&itemData);
  return iResult;
}

CItemData *DboxMain::getSelectedItem()
{
  CItemData *retval = NULL;
  if (m_ctlItemList.IsWindowVisible()) {
    // flattened list mode.
    POSITION p = m_ctlItemList.GetFirstSelectedItemPosition();
    if (p) {
      int i = m_ctlItemList.GetNextSelectedItem(p);
      retval = (CItemData *)m_ctlItemList.GetItemData(i);
      ASSERT(retval != NULL);
      DisplayInfo *di = (DisplayInfo *)retval->GetDisplayInfo();
      ASSERT(di != NULL && di->list_index == i);
    }
  } else {
    // heirarchy tree mode; go from HTREEITEM to index
    HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
    if (ti != NULL) {
      retval = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (retval != NULL) {  // leaf node
        DisplayInfo *di = (DisplayInfo *)retval->GetDisplayInfo();
        ASSERT(di != NULL && di->tree_item == ti);
      }
    }    
  }
  return retval;
}

void
DboxMain::ClearData(bool clearMRE)
{
  // Iterate over item list, delete DisplayInfo
  POSITION listPos = m_core.GetFirstEntryPosition();
  while (listPos != NULL) {
    CItemData &ci = m_core.GetEntryAt(listPos);
    delete ci.GetDisplayInfo(); // no need to Set to NULL
    m_core.GetNextEntry(listPos);
  }
  m_core.ClearData();

  if (m_bOpen)
	  UpdateSystemTray(LOCKED);
  else
	  UpdateSystemTray(CLOSED);

  // If data is cleared, m_selectedAtMinimize is useless,
  // since it will be deleted and rebuilt from the file.
  // This means that selection won't be restored in this case.
  // Tough.
  m_selectedAtMinimize = NULL;
  // Ditto for expanded groups, unfortunately
  m_ctlItemTree.ClearExpanded();

  if (clearMRE)
    m_RUEList.ClearEntries();

  //Because GetText returns a copy, we cannot do anything about the names
  if (m_windowok) {
    // For long lists, this is painful, so we disable updates
    m_ctlItemList.LockWindowUpdate();
    m_ctlItemList.DeleteAllItems();
    m_ctlItemList.UnlockWindowUpdate();
    m_ctlItemTree.LockWindowUpdate();
    m_ctlItemTree.DeleteAllItems();
    m_ctlItemTree.UnlockWindowUpdate();
  }
}

void DboxMain::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
  if (m_iSortedColumn == pNMListView->iSubItem) {
    m_bSortAscending = !m_bSortAscending;
  } else {
    m_iSortedColumn = pNMListView->iSubItem;
    m_bSortAscending = true;
  }
  m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
  FixListIndexes();
#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
  HDITEM HeaderItem;
  HeaderItem.mask = HDI_FORMAT;
  m_ctlItemList.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
  // Turn off all arrows
  HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  m_ctlItemList.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);

  *pResult = 0;
}



void
DboxMain::OnListView() 
{
  SetListView();
  m_IsListView = true;
}

void
DboxMain::OnTreeView() 
{
  SetTreeView();
  m_IsListView = false;
}

void
DboxMain::SetListView()
{
  m_ctlItemTree.ShowWindow(SW_HIDE);
  m_ctlItemList.ShowWindow(SW_SHOW);
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView,
				   _T("list"));
}

void
DboxMain::SetTreeView()
{
  m_ctlItemList.ShowWindow(SW_HIDE);
  m_ctlItemTree.ShowWindow(SW_SHOW);
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView,
                                   _T("tree"));
}

void
DboxMain::OnOldToolbar() 
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, false);
  SetToolbar(ID_MENUITEM_OLD_TOOLBAR);
  SetReadOnly(m_IsReadOnly);
}

void
DboxMain::OnNewToolbar() 
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, true);
  SetToolbar(ID_MENUITEM_NEW_TOOLBAR);
  SetReadOnly(m_IsReadOnly);
}

void
DboxMain::SetToolbar(int menuItem)
{
  UINT Flags = 0;
  CBitmap bmTemp; 
  COLORREF Background = RGB(192, 192, 192);

  switch (menuItem) {
  case ID_MENUITEM_NEW_TOOLBAR: {
    int NumBits = 32;
    CDC* pDC = this->GetDC();
    if ( pDC )  {
      NumBits = pDC->GetDeviceCaps(12 /*BITSPIXEL*/);
    }
    if (NumBits >= 32) {
      bmTemp.LoadBitmap(IDB_TOOLBAR1);
      Flags = ILC_MASK | ILC_COLOR32;
    } else {
      bmTemp.LoadBitmap(IDB_TOOLBAR2);
      Flags = ILC_MASK | ILC_COLOR8;
      Background = RGB( 196,198,196 );
    }
    break;
  }
  case ID_MENUITEM_OLD_TOOLBAR:
    bmTemp.LoadBitmap(IDB_TOOLBAR3);
    Flags = ILC_MASK | ILC_COLOR8;
    break;
  default:
    ASSERT(false);
    return;
  }
  m_toolbarMode = menuItem;

  CToolBarCtrl& tbcTemp = m_wndToolBar.GetToolBarCtrl();
  CImageList ilTemp; 
  ilTemp.Create(16, 16, Flags, 10, 10);
  ilTemp.Add(&bmTemp, Background);
  tbcTemp.SetImageList(&ilTemp);
  ilTemp.Detach();
  bmTemp.Detach();

  m_wndToolBar.Invalidate();

  CRect rect;
  RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
  RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
  m_ctlItemList.MoveWindow(&rect, TRUE);
  m_ctlItemTree.MoveWindow(&rect, TRUE); // Fix Bug 940585
}

void
DboxMain::OnExpandAll()
{
  m_ctlItemTree.OnExpandAll();
}

void
DboxMain::OnCollapseAll()
{
  m_ctlItemTree.OnCollapseAll();
}

void
DboxMain::OnTimer(UINT nIDEvent )
{			
  if ((nIDEvent == TIMER_CHECKLOCK && IsWorkstationLocked()) ||
      (nIDEvent == TIMER_USERLOCK && DecrementAndTestIdleLockCounter())) {
    /*
     * Since we clear the data, any unchanged changes will be lost,
     * so we force a save if database is modified, and fail
     * to lock if the save fails.
     */
    if (!(m_core.IsChanged() || m_bTSUpdated) ||
        Save() == PWScore::SUCCESS) {
      TRACE("locking database\n");
      ClearData();
      if(IsWindowVisible()){
        ShowWindow(SW_MINIMIZE);
      }
      m_needsreading = true;
      if (nIDEvent == TIMER_CHECKLOCK)
        KillTimer(TIMER_CHECKLOCK);
    }
  }
}

// This function determines if the workstation is locked.
BOOL DboxMain::IsWorkstationLocked() const
{
  HDESK hDesktop; 
  BOOL Result = false;

  hDesktop = OpenDesktop("default", 0, false, DESKTOP_SWITCHDESKTOP);
  if( hDesktop != 0 ) {
    // SwitchDesktop fails if hDesktop invisible, screensaver or winlogin.
    Result = ! SwitchDesktop(hDesktop);
    CloseDesktop(hDesktop);
  }
  return Result;
}


void
DboxMain::OnChangeFont() 
{
  HFONT hOldFontTree = (HFONT) m_ctlItemTree.SendMessage(WM_GETFONT);

  // make sure we know what is inside the font.
  LOGFONT lf;
  ::GetObject(hOldFontTree, sizeof lf, &lf);

  // present it and possibly change it
  CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
  if(dlg.DoModal() == IDOK) {
    m_hFontTree = ::CreateFontIndirect(&lf);
    // transfer the fonts to the tree and list windows
    m_ctlItemTree.SendMessage(WM_SETFONT, (WPARAM) m_hFontTree, true);
    m_ctlItemList.SendMessage(WM_SETFONT, (WPARAM) m_hFontTree, true);
    // now can get rid of the old font
    ::DeleteObject(hOldFontTree);
        
    CString str;
    str.Format("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s",
               lf.lfHeight,
               lf.lfWidth,
               lf.lfEscapement,
               lf.lfOrientation,
               lf.lfWeight,
               lf.lfItalic,
               lf.lfUnderline,
               lf.lfStrikeOut,
               lf.lfCharSet,
               lf.lfOutPrecision,
               lf.lfClipPrecision,
               lf.lfQuality,
               lf.lfPitchAndFamily,
               lf.lfFaceName);
        	
    PWSprefs *prefs = PWSprefs::GetInstance();        	
    prefs->SetPref(PWSprefs::TreeFont, str);
  }
}

void
DboxMain::ExtractFont(CString& str)
{
#pragma warning(push)
#pragma warning(disable:4244)  // possible loss of data 'int' to 'unsigned char'
  m_treefont.lfHeight=atol((LPCTSTR)GetToken(str, ","));
  m_treefont.lfWidth=atol((LPCTSTR)GetToken(str, ","));
  m_treefont.lfEscapement=atol((LPCTSTR)GetToken(str, ","));
  m_treefont.lfOrientation=atol((LPCTSTR)GetToken(str, ","));
  m_treefont.lfWeight=atol((LPCTSTR)GetToken(str, ","));
  m_treefont.lfItalic=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfUnderline=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfStrikeOut=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfCharSet=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfOutPrecision=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfClipPrecision=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfQuality=atoi((LPCTSTR)GetToken(str, ","));
  m_treefont.lfPitchAndFamily=atoi((LPCTSTR)GetToken(str, ","));
  strcpy(m_treefont.lfFaceName, str);
#pragma warning(pop)
}

CString
DboxMain::GetToken(CString& str, LPCTSTR c)
{
  int pos;
  CString token;

  pos = str.Find(c);
  token = str.Left(pos);
  str = str.Mid(pos + 1);

  return token;
}

void
DboxMain::UpdateSystemTray(const STATE s)
{
  switch (s) {
  case LOCKED:
    app.SetSystemTrayState(ThisMfcApp::LOCKED);
    if (!m_core.GetCurFile().IsEmpty())
      app.SetTooltipText(_T("[") + m_core.GetCurFile() + _T("]"));
    break;
  case UNLOCKED:
    app.SetSystemTrayState(ThisMfcApp::UNLOCKED);
    if (!m_core.GetCurFile().IsEmpty())
      app.SetTooltipText(m_core.GetCurFile());
    break;
  case CLOSED:
    app.SetSystemTrayState(ThisMfcApp::CLOSED);
    break;
  default:
    ASSERT(0);
  }
}
#ifdef _DEBUG
static BOOL MakeErrorString(long status, const CString & name, CString &mess)
{
  BOOL retval = FALSE;
  switch(status) {
  case 0:
    mess.Format(_T("The system is out of memory or resources."));
    retval = TRUE;
    break;
  case ERROR_FILE_NOT_FOUND:
    mess.Format(_T("File '%s' not found."), name);
    retval = TRUE;
    break;
  case ERROR_PATH_NOT_FOUND:
    mess.Format(_T("Path of file '%s' not found."), name);
    retval = TRUE;
    break;
  case ERROR_BAD_FORMAT:
    mess.Format(_T("Executable '%s' is invalid (non-Win32® .exe or error in .exe image)."), name);
    retval = TRUE;
    break;
  case SE_ERR_ACCESSDENIED:
    mess.Format(_T("The operating system denied access to file '%s'."), name);
    retval = TRUE;
    break;
  case SE_ERR_ASSOCINCOMPLETE:
    mess.Format(_T("Name association for file %s' is incomplete or invalid."), name);
    retval = TRUE;
    break;
  case SE_ERR_DDEBUSY:
    mess.Format(_T("DDE transaction could not be completed: other DDE transactions being processed."));
    retval = TRUE;
    break;
  case SE_ERR_DDEFAIL:
    mess.Format(_T("DDE transaction failed."));
    retval = TRUE;
    break;
  case SE_ERR_DDETIMEOUT:
    mess.Format(_T("DDE transaction could not be completed: request timed out."));
    retval = TRUE;
    break;
  case SE_ERR_DLLNOTFOUND:
    mess.Format(_T("The specified dynamic-link library was not found."));
    retval = TRUE;
    break;
  case SE_ERR_NOASSOC:
    mess.Format(_T("No association for file type of '%s' found."), name);
    retval = TRUE;
    break;
  case SE_ERR_OOM:
    mess.Format(_T("The system is out of memory or resources."));
    retval = TRUE;
    break;
  case SE_ERR_SHARE:
    mess.Format(_T("A sharing violation occurred."));
    retval = TRUE;
    break;
 default:
   if(status < 32) {
     mess.Format(_T("Unknown error %d returned from FindExecutable()."), status);
     retval = TRUE;
   }
   break;
  }
  return retval;
}

#endif

BOOL
DboxMain::LaunchBrowser(const CString &csURL)
{
  CString csBrowser, csTempFileName, csMsg;
  char lpPathBuffer[MAX_PATH];
  HANDLE hTempFile;
  BOOL bError = FALSE;
  long hinst;

  // If csURL doesn't contain "://", then we'll prepend "http://" to it,
  // e.g., change "www.mybank.com" to "http://www.mybank.com".
  CString theURL(csURL);

  if (theURL.Find(_T("://")) == -1)
    theURL = _T("http://") + theURL;

  if (!app.m_csDefault_Browser.IsEmpty()) {
    hinst = long(::ShellExecute(NULL, NULL, app.m_csDefault_Browser, theURL,
                                NULL, SW_SHOWNORMAL));
    return TRUE;
  }

  // Get the temp path.
  DWORD dwRetVal = GetTempPath(MAX_PATH - 14, lpPathBuffer);

  if(dwRetVal > MAX_PATH - 14) {
    csMsg.Format(_T("GetTempPath failed with error %d."), GetLastError());
    AfxMessageBox(csMsg, MB_ICONSTOP);
    return FALSE;
  }

  hTempFile = INVALID_HANDLE_VALUE;  // silly compiler warning!
  // Create a temporary file.
  for(int i = 1; i < 99999; i++) {
    csTempFileName.Format(_T("%sPWS%.5d.html)"), lpPathBuffer, i);
    hTempFile = CreateFile(csTempFileName, 					// file name
                           GENERIC_READ | GENERIC_WRITE,	// open r-w
                           0,								// do not share
                           NULL,							// default security
                           CREATE_NEW,						// must be new
                           FILE_ATTRIBUTE_NORMAL,			// normal file
                           NULL);							// no template
    if(hTempFile != INVALID_HANDLE_VALUE)
      break;
  }

  if(hTempFile == INVALID_HANDLE_VALUE) {
    csMsg.Format(_T("Couldn't create a temporary file to determine default browser."));
    AfxMessageBox(csMsg, MB_ICONSTOP);
    return FALSE;
  }

  if(!CloseHandle(hTempFile)) {
    csMsg.Format(_T("CloseHandle failed with error %d."), GetLastError());
    AfxMessageBox(csMsg, MB_ICONSTOP);
    return FALSE;
  }

  hinst = long(::FindExecutable(csTempFileName, NULL,
                                csBrowser.GetBufferSetLength(MAX_PATH)));
  csBrowser.ReleaseBuffer();

  if(!DeleteFile(csTempFileName)) {
    csMsg.Format(_T("DeleteFile failed with error %d."), GetLastError());
    bError = TRUE;
  }
#ifdef _DEBUG
  bError = MakeErrorString(hinst, csTempFileName, csMsg);

  if (bError == TRUE) {
    AfxMessageBox(csMsg, MB_ICONSTOP);
    return FALSE;
  }
#else
  if(hinst < 32) {
    // Display it the old way - re-use any open browser window!
    hinst = long(::ShellExecute(NULL, NULL, theURL, NULL,
                                NULL, SW_SHOWNORMAL));
    if(hinst < 32) {
      AfxMessageBox(_T("oops - can't display URL"), MB_ICONSTOP);
      return FALSE;
    }
    return TRUE;
  }
#endif
  hinst = long(::ShellExecute(NULL, NULL, csBrowser, theURL,
                              NULL, SW_SHOWNORMAL));
#ifdef _DEBUG
  bError = MakeErrorString(hinst, csBrowser, csMsg);

  if (bError == TRUE) {
    AfxMessageBox(csMsg, MB_ICONSTOP);
    return FALSE;
  }
#else
  if(hinst < 32) {
    AfxMessageBox(_T("oops can't display URL"), MB_ICONSTOP);
    return FALSE;
  }
#endif
  // Save default browser - so we do not have to do this again!
  app.m_csDefault_Browser = csBrowser;
  return TRUE;
}
