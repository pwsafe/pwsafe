/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
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
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif

#include "DboxMain.h"
#include "TryAgainDlg.h"
#include "ColumnPickerDlg.h"

#include "corelib/pwsprefs.h"
#include "commctrl.h"
#include <vector>

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
   * If sorting is by group (title/user), title (username), username (title) 
   * fields in brackets are the secondary fields if the primary fields are identical.
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
  int nSortColumn = self->m_iSortedColumn;
  CItemData* pLHS = (CItemData *)lParam1;
  CItemData* pRHS = (CItemData *)lParam2;
  CMyString	group1, group2;
  time_t t1, t2;

  int iResult;
  switch(nSortColumn) {
    case CItemData::GROUP:
      group1 = pLHS->GetGroup();
      group2 = pRHS->GetGroup();
      if (group1.IsEmpty())  // root?
        group1 = _T("\xff");
      if (group2.IsEmpty())  // root?
        group2 = _T("\xff");
      iResult = group1.CompareNoCase(group2);
      if (iResult == 0) {
        iResult = (pLHS->GetTitle()).CompareNoCase(pRHS->GetTitle());
        if (iResult == 0) {
          iResult = (pLHS->GetUser()).CompareNoCase(pRHS->GetUser());
        }
      }
      break;
    case CItemData::TITLE:
      iResult = (pLHS->GetTitle()).CompareNoCase(pRHS->GetTitle());
      if (iResult == 0) {
        iResult = (pLHS->GetUser()).CompareNoCase(pRHS->GetUser());
      }
      break;
    case CItemData::USER:
      iResult = (pLHS->GetUser()).CompareNoCase(pRHS->GetUser());
      if (iResult == 0) {
        iResult = (pLHS->GetTitle()).CompareNoCase(pRHS->GetTitle());
      }
    case CItemData::NOTES:
      iResult = (pLHS->GetNotes()).CompareNoCase(pRHS->GetNotes());
      break;
    case CItemData::PASSWORD:
      iResult = (pLHS->GetPassword()).CompareNoCase(pRHS->GetPassword());
      break;
    case CItemData::CTIME:
      pLHS->GetCTime(t1);
      pRHS->GetCTime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case CItemData::PMTIME:
      pLHS->GetPMTime(t1);
      pRHS->GetPMTime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case CItemData::ATIME:
      pLHS->GetATime(t1);
      pRHS->GetATime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case CItemData::LTIME:
      pLHS->GetLTime(t1);
      pRHS->GetLTime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case CItemData::RMTIME:
      pLHS->GetRMTime(t1);
      pRHS->GetRMTime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
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

static int CALLBACK ExplorerCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*closure*/)
{
  int iResult;
  CItemData* pLHS = (CItemData *)lParam1;
  CItemData* pRHS = (CItemData *)lParam2;
  //DboxMain *self = (DboxMain*)closure;
  if (pLHS == NULL)
      return -1;
  if (pRHS == NULL)
      return 1;
  
  iResult = (pLHS->GetGroup()).CompareNoCase(pRHS->GetGroup());
  if (iResult == 0) {
    iResult = (pLHS->GetTitle()).CompareNoCase(pRHS->GetTitle());
    if (iResult == 0) {
      iResult = (pLHS->GetUser()).CompareNoCase(pRHS->GetUser());
    }
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
	  // Set up Configuration source indicator (debug only)
#ifdef DEBUG
      statustext[SB_CONFIG] = PWSprefs::GetInstance()->GetConfigIndicator();
#else
      statustext[SB_CONFIG] = IDS_CONFIG_BLANK;
#endif /* DEBUG */
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

void DboxMain::UpdateListItem(const int lindex, const int type, const CString &newText)
{
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;

  int iSubItem = m_nColumnTypeToItem[type];

  if (iSubItem > 0) {
    m_ctlItemList.SetItemText(lindex, iSubItem, newText);
    if (m_iSortedColumn == type) {
      m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
      FixListIndexes();
    }
  }
}

 // Find in m_pwlist entry with same title and user name as the i'th entry in m_ctlItemList
POSITION DboxMain::Find(int i)
{
  CItemData *ci = (CItemData *)m_ctlItemList.GetItemData(i);
  ASSERT(ci != NULL);
  const CMyString curGroup = ci->GetGroup();
  const CMyString curTitle = ci->GetTitle();
  const CMyString curUser = ci->GetUser();
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

  int ititle(-1);  // Must be there as it is mandatory!
  for (int ic = 0; ic < m_nColumns; ic++) {
      if (m_nColumnTypeByItem[ic] == CItemData::TITLE) {
          ititle = ic;
          break;
      }
  }

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
		if (::_tcsstr(curtitle, searchstr) ||
			::_tcsstr(curuser, searchstr) ||
			::_tcsstr(curnotes, searchstr) ||
			::_tcsstr(curgroup, searchstr) ||
			::_tcsstr(curURL, searchstr) ||
			::_tcsstr(curAT, searchstr)) {
			// Find index in displayed list
			DisplayInfo *di = (DisplayInfo *)curitem.GetDisplayInfo();
			ASSERT(di != NULL);
			int li = di->list_index;
			ASSERT(CMyString(m_ctlItemList.GetItemText(li, ititle)) == savetitle);
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
		if (::_tcsstr(curtitle, searchstr) ||
			::_tcsstr(curuser, searchstr) ||
			::_tcsstr(curnotes, searchstr) ||
			::_tcsstr(curgroup, searchstr) ||
			::_tcsstr(curURL, searchstr) ||
			::_tcsstr(curAT, searchstr)) {
			// Find index in displayed list
			DisplayInfo *di = (DisplayInfo *)curitem.GetDisplayInfo();
			ASSERT(di != NULL);
			int li = di->list_index;
			ASSERT(CMyString(m_ctlItemList.GetItemText(li, ititle)) == savetitle);
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


// Updates m_ctlItemList and m_ctlItemTree from m_pwlist
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

  if (m_bExplorerTypeTree) {
      SortTree(TVI_ROOT);
  }

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
DboxMain::SortTree(const HTREEITEM htreeitem)
{
  TVSORTCB tvs;
  HTREEITEM hti(htreeitem);

  if (hti == NULL)
      hti = TVI_ROOT;

  tvs.hParent = hti;
  tvs.lpfnCompare = ExplorerCompareProc;
  tvs.lParam = (LPARAM)this;

  m_ctlItemTree.SortChildrenCB(&tvs);
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
          CString cs_text(MAKEINTRESOURCE(IDS_COULDNOTSAVE)), 
              cs_title(MAKEINTRESOURCE(IDS_SAVEERROR));
          MessageBox(cs_text, cs_title, MB_ICONSTOP);
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
DboxMain::OnContextMenu(CWnd* /* pWnd */, CPoint point) 
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

void DboxMain::OnKeydownItemlist(NMHDR* pNMHDR, LRESULT* pResult)
{
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
DboxMain::OnSetfocusItemlist(NMHDR* /* pNMHDR */, LRESULT* /* pResult */) 
{
  // Seems excessive to do this all the time
  // Should be done only on Open and on Change (Add, Delete, Modify)
  if (m_toolbarsSetup == TRUE)
    UpdateStatusBar();
}

void
DboxMain::OnKillfocusItemlist(NMHDR* /* pNMHDR */, LRESULT* /* pResult */) 
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

  CMyString group = itemData.GetGroup();
  CMyString title = itemData.GetTitle();
  CMyString username = itemData.GetUser();
  // get only the first line for display
  CMyString strNotes = itemData.GetNotes();
  int iEOL = strNotes.Find(TCHAR('\r'));
  if (iEOL >= 0 && iEOL < strNotes.GetLength()) {
    CMyString strTemp = strNotes.Left(iEOL);
    strNotes = strTemp;
  }
  CMyString cs_fielddata;

  // Insert the first column data
  switch (m_nColumnTypeByItem[0]) {
    case CItemData::GROUP:
      cs_fielddata = group;
      break;
    case CItemData::TITLE:
      cs_fielddata = title;
      break;
    case CItemData::USER:
      cs_fielddata = username;
      break;
    case CItemData::NOTES:
      cs_fielddata = strNotes;
      break;
    case CItemData::PASSWORD:
      cs_fielddata = itemData.GetPassword();
      break;
    case CItemData::CTIME:
      cs_fielddata = itemData.GetCTimeL();
      break;
    case CItemData::PMTIME:
      cs_fielddata = itemData.GetPMTimeL();
      break;
    case CItemData::ATIME:
      cs_fielddata = itemData.GetATimeL();
      break;
    case CItemData::LTIME:
      cs_fielddata = itemData.GetLTimeL();
      break;
    case CItemData::RMTIME:
      cs_fielddata = itemData.GetRMTimeL();
      break;
    default:
      ASSERT(0);
  }
  iResult = m_ctlItemList.InsertItem(iResult, cs_fielddata);

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
    if (!m_bExplorerTypeTree) {
      ti = m_ctlItemTree.InsertItem(treeDispString, ti, TVI_SORT);
      m_ctlItemTree.SetItemData(ti, (DWORD)&itemData);
    } else {
      ti = m_ctlItemTree.InsertItem(treeDispString, ti, TVI_LAST);
      m_ctlItemTree.SetItemData(ti, (DWORD)&itemData);
      SortTree(m_ctlItemTree.GetParentItem(ti));
    }
    time_t now, warnexptime, tLTime;
    time(&now);
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn)) {
        int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
        struct tm st;
#if _MSC_VER >= 1400
        errno_t err;
        err = localtime_s(&st, &now);  // secure version
#else
        st = *localtime(&now);
        ASSERT(st != NULL); // null means invalid time
#endif
        st.tm_mday += idays;
        warnexptime = mktime(&st);
        if (warnexptime == (time_t)-1)
          warnexptime = (time_t)0;
    } else
        warnexptime = (time_t)0;
    
    itemData.GetLTime(tLTime);
	if (tLTime != 0) {
	    if (tLTime <= now) {
    	    m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::EXPIRED_LEAF, CMyTreeCtrl::EXPIRED_LEAF);
    	} else if (tLTime < warnexptime) {
    	    m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::WARNEXPIRED_LEAF, CMyTreeCtrl::WARNEXPIRED_LEAF);
	    } else
	        m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::LEAF, CMyTreeCtrl::LEAF);
	} else
	  m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::LEAF, CMyTreeCtrl::LEAF);
	
    ASSERT(ti != NULL);
    itemData.SetDisplayInfo((void *)di);
    di->tree_item = ti;
  }

  // Set the data in the rest of the columns
  for (int i = 1; i < m_nColumns; i++) {
    switch (m_nColumnTypeByItem[i]) {
      case CItemData::GROUP:
        cs_fielddata = group;
        break;
      case CItemData::TITLE:
        cs_fielddata = title;
        break;
      case CItemData::USER:
        cs_fielddata = username;
        break;
      case CItemData::NOTES:
        cs_fielddata = strNotes;
        break;
      case CItemData::PASSWORD:
        cs_fielddata = itemData.GetPassword();
        break;
      case CItemData::CTIME:
        cs_fielddata = itemData.GetCTimeL();
        break;
      case CItemData::PMTIME:
        cs_fielddata = itemData.GetPMTimeL();
        break;
      case CItemData::ATIME:
        cs_fielddata = itemData.GetATimeL();
        break;
      case CItemData::LTIME:
        cs_fielddata = itemData.GetLTimeL();
        break;
      case CItemData::RMTIME:
        cs_fielddata = itemData.GetRMTimeL();
        break;
      default:
        ASSERT(0);
    }
    m_ctlItemList.SetItemText(iResult, i, cs_fielddata);
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

  // Get column index to CItemData value
  int iItem = pNMListView->iSubItem;
  int isortcolumn = m_nColumnTypeByItem[iItem];
  
  if (m_iSortedColumn == isortcolumn) {
    m_bSortAscending = !m_bSortAscending;
  } else {
    m_iSortedColumn = isortcolumn;
    m_bSortAscending = true;
  }

  m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
  FixListIndexes();

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  m_ctlItemList.GetHeaderCtrl()->GetItem(iItem, &hdi);
  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  m_ctlItemList.GetHeaderCtrl()->SetItem(iItem, &hdi);

  *pResult = TRUE;
}

void
DboxMain::OnHeaderRClick(NMHDR* /* pNMHDR */, LRESULT *pResult)
{
#if defined(POCKET_PC)
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN;
#else
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;
#endif
  CMenu menu;
  CPoint ptMousePos;
  GetCursorPos(&ptMousePos);

  if (menu.LoadMenu(IDR_POPCOLUMNS)) {
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);
    pPopup->TrackPopupMenu(dwTrackPopupFlags, ptMousePos.x, ptMousePos.y, this);
  }
  *pResult = TRUE;
}

void
DboxMain::OnHeaderEndDrag(NMHDR* /* pNMHDR */, LRESULT *pResult)
{
  // Called for HDN_ENDDRAG which changes the column order
  // Unfortuantely the changes are only really done when this call returns,
  // hence the PostMessage to get the information later

  // get control after operation is really complete
  PostMessage(WM_HDR_DRAG_COMPLETE);
  *pResult = FALSE;
}

void
DboxMain::OnHeaderNotify(NMHDR* pNMHDR, LRESULT *pResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
  
  *pResult = TRUE;
  if (m_nColumnWidthByItem == NULL || phdn->pitem == NULL)
      return;

  UINT mask = phdn->pitem->mask;

  switch (phdn->hdr.code) {
    case HDN_ENDTRACK:
      m_nColumnWidthByItem[phdn->iItem] = phdn->pitem->cxy;
      break;
    case HDN_ITEMCHANGED:
      if ((mask & HDI_WIDTH) == HDI_WIDTH) {
        // column width changed
        m_nColumnWidthByItem[phdn->iItem] = phdn->pitem->cxy;
      }
      break;
    default:
      break;
  }
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
      SaveDisplayStatus();
      m_lock_displaystatus = m_core.GetDisplayStatus();
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

  hDesktop = OpenDesktop(_T("default"), 0, false, DESKTOP_SWITCHDESKTOP);
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
  CFont *pOldFontTree;
  pOldFontTree = m_ctlItemTree.GetFont();

  // make sure we know what is inside the font.
  LOGFONT lf;
  pOldFontTree->GetLogFont(&lf);

  // present it and possibly change it
  CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
  if(dlg.DoModal() == IDOK) {
    m_pFontTree->DeleteObject();
    m_pFontTree->CreateFontIndirect(&lf);
    // transfer the fonts to the tree and list windows
    m_ctlItemTree.SetFont(m_pFontTree);
    m_ctlItemList.SetFont(m_pFontTree);
    m_pctlItemListHdr->SetFont(m_pFontTree);

    // Recalculate header widths
    CalcHeaderWidths();
    // Reset column widths
    ResizeColumns();

    CString str;
    str.Format(_T("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s"),
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
DboxMain::ExtractFont(CString& str, LOGFONT *ptreefont)
{
  ptreefont->lfHeight = _ttol((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfWidth = _ttol((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfEscapement = _ttol((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfOrientation = _ttol((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfWeight = _ttol((LPCTSTR)GetToken(str, _T(",")));

#pragma warning(push)
#pragma warning(disable:4244) //conversion from 'int' to 'BYTE', possible loss of data
  ptreefont->lfItalic = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfUnderline = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfStrikeOut = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfCharSet = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfOutPrecision = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfClipPrecision = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfQuality = _ttoi((LPCTSTR)GetToken(str, _T(",")));
  ptreefont->lfPitchAndFamily = _ttoi((LPCTSTR)GetToken(str, _T(",")));
#pragma warning(pop)

#if (_MSC_VER >= 1400)
  _tcscpy_s(ptreefont->lfFaceName, LF_FACESIZE, str);
#else
  _tcscpy(ptreefont->lfFaceName, str);
#endif  
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

BOOL
DboxMain::LaunchBrowser(const CString &csURL)
{
  CString csAltBrowser;
  bool useAltBrowser;
  long hinst;
  CString theURL(csURL);

  // If csURL contains "[alt]" then we'll use the alternate browser (if defined),
  // and remove the "[alt]" from the URL.
  // If csURL doesn't contain "://", then we'll prepend "http://" to it,
  // e.g., change "www.mybank.com" to "http://www.mybank.com".
  int altReplacements = theURL.Replace(_T("[alt]"), _T(""));
  if (theURL.Find(_T("://")) == -1)
    theURL = _T("http://") + theURL;

  csAltBrowser = CString(PWSprefs::GetInstance()->
                         GetPref(PWSprefs::AltBrowser));

  useAltBrowser = (altReplacements > 0) && !csAltBrowser.IsEmpty();

  if (!useAltBrowser) {
    hinst = long(::ShellExecute(NULL, NULL, theURL, NULL,
                                NULL, SW_SHOWNORMAL));
  } else {
    hinst = long(::ShellExecute(NULL, NULL, csAltBrowser, theURL,
                                NULL, SW_SHOWNORMAL));
  }

  if(hinst < 32) {
    AfxMessageBox(IDS_CANTBROWSE, MB_ICONSTOP);
    return FALSE;
  }
  return TRUE;
}

void
DboxMain::SetColumns()
{
  // User hasn't yet saved the columns he/she wants and so gets our order!
  // Or - user has reset the columns (popup menu from right click on Header)
  CString cs_header;
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;

  int ipwd = m_bShowPasswordInList ? 1 : 0;

  CRect rect;
  m_ctlItemList.GetClientRect(&rect);
  PWSprefs *prefs = PWSprefs::GetInstance();
  int i1stWidth = prefs->GetPref(PWSprefs::Column1Width,
                                 (rect.Width() / 3 + rect.Width() % 3));
  int i2ndWidth = prefs->GetPref(PWSprefs::Column2Width,
                                 rect.Width() / 3);
  int i3rdWidth = prefs->GetPref(PWSprefs::Column3Width,
                                 rect.Width() / 3);
  
  cs_header = GetHeaderText(CItemData::TITLE);
  m_ctlItemList.InsertColumn(0, cs_header);
  hdi.lParam = CItemData::TITLE;
  m_pctlItemListHdr->SetItem(0, &hdi);
  m_ctlItemList.SetColumnWidth(0, i1stWidth);
  
  cs_header = GetHeaderText(CItemData::USER);
  m_ctlItemList.InsertColumn(1, cs_header);
  hdi.lParam = CItemData::USER;
  m_pctlItemListHdr->SetItem(1, &hdi);
  m_ctlItemList.SetColumnWidth(1, i2ndWidth);

  cs_header = GetHeaderText(CItemData::NOTES);
  m_ctlItemList.InsertColumn(2, cs_header);
  hdi.lParam = CItemData::NOTES;
  m_pctlItemListHdr->SetItem(2, &hdi);
  m_ctlItemList.SetColumnWidth(1, i3rdWidth);
    
  if (m_bShowPasswordInList) {
    cs_header = GetHeaderText(CItemData::PASSWORD);
    m_ctlItemList.InsertColumn(3, cs_header);
    hdi.lParam = CItemData::PASSWORD;
    m_pctlItemListHdr->SetItem(3, &hdi);
    m_ctlItemList.SetColumnWidth(3,
           PWSprefs::GetInstance()->
           GetPref(PWSprefs::Column4Width,
           rect.Width() / 4));
  }

  cs_header = GetHeaderText(CItemData::CTIME);
  m_ctlItemList.InsertColumn(ipwd + 3, cs_header);
  hdi.lParam = CItemData::CTIME;
  m_pctlItemListHdr->SetItem(ipwd + 3, &hdi);
  
  cs_header = GetHeaderText(CItemData::PMTIME);
  m_ctlItemList.InsertColumn(ipwd + 4, cs_header);
  hdi.lParam = CItemData::PMTIME;
  m_pctlItemListHdr->SetItem(ipwd + 4, &hdi);
  
  cs_header = GetHeaderText(CItemData::ATIME);
  m_ctlItemList.InsertColumn(ipwd + 5, cs_header);
  hdi.lParam = CItemData::ATIME;
  m_pctlItemListHdr->SetItem(ipwd + 5, &hdi);
  
  cs_header = GetHeaderText(CItemData::LTIME);
  m_ctlItemList.InsertColumn(ipwd + 6, cs_header);
  hdi.lParam = CItemData::LTIME;
  m_pctlItemListHdr->SetItem(ipwd + 6, &hdi);
  
  cs_header = GetHeaderText(CItemData::RMTIME);
  m_ctlItemList.InsertColumn(ipwd + 7, cs_header);
  hdi.lParam = CItemData::RMTIME;
  m_pctlItemListHdr->SetItem(ipwd + 7, &hdi);

  m_ctlItemList.SetRedraw(FALSE);

  for (int i = ipwd + 3; i < (ipwd + 8); i++) {
	m_ctlItemList.SetColumnWidth(i, LVSCW_AUTOSIZE);
	m_ctlItemList.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	m_ctlItemList.SetColumnWidth(i, m_iDateTimeFieldWidth);
  }

  SetHeaderInfo();

  return;
}
void
DboxMain::SetColumns(const CString cs_ListColumns, const CString cs_ListColumnsWidths)
{
  //  User has saved the columns he/she wants and now we are putting them back
    CString cs_header;
    HDITEM hdi;
    hdi.mask = HDI_LPARAM;

    std::vector<int> vi_columns;
    std::vector<int> vi_widths;
    std::vector<int>::const_iterator vi_Iter;
    const TCHAR pSep[] = _T(",");
    TCHAR *pTemp, *pWidths;
  
    // Duplicate as strtok modifies the string
    pTemp = _tcsdup((LPCTSTR)cs_ListColumns);
    pWidths = _tcsdup((LPCTSTR)cs_ListColumnsWidths);
  
#if _MSC_VER >= 1400
    // Capture columns shown:
    char *next_token;
    TCHAR *token = _tcstok_s(pTemp, pSep, &next_token);
    while(token) {
      vi_columns.push_back(_ttoi(token));
      token = _tcstok_s(NULL, pSep, &next_token);
    }
    // and their widths
    next_token = NULL;
    token = _tcstok_s(pWidths, pSep, &next_token);
    while(token) {
      vi_widths.push_back(_ttoi(token));
      token = _tcstok_s(NULL, pSep, &next_token);
    }
#else
    // Capture columns shown:
    TCHAR *token = _tcstok(pTemp, pSep);
    while(token) {
      vi_columns.push_back(_ttoi(token));
      token = _tcstok(NULL, pSep);
    }
    // and their widths
    token = _tcstok(pWidths, pSep);
    while(token) {
      vi_widths.push_back(_ttoi(token));
      token = _tcstok(NULL, pSep);
    }
#endif

    ASSERT(vi_columns.size() == vi_widths.size());
    free(pTemp);
    free(pWidths);
  
    int icol = 0;

    for (vi_Iter = vi_columns.begin();
         vi_Iter != vi_columns.end();
         vi_Iter++) {
      int icolset = (int)*vi_Iter;
      int &iwidth = vi_widths.at(icol);
      cs_header = GetHeaderText(icolset);
      if (!cs_header.IsEmpty()) {
          m_ctlItemList.InsertColumn(icol, cs_header);
          m_ctlItemList.SetColumnWidth(icol, iwidth);
          hdi.lParam = icolset;
          m_pctlItemListHdr->SetItem(icol, &hdi);
          icol++;
      }
    }

    SetHeaderInfo();

    return;
}

void
DboxMain::SetColumns(const CItemData::FieldBits bscolumn)
{
  // User has changed columns he/she wants and we have deleted the ones
  // not required.
  // Now add any new columns in reverse order at the beginning of the display
    CString cs_header;
    HDITEM hdi;

    hdi.mask = HDI_LPARAM;

    for (int i = CItemData::LAST - 1; i >= 0; i--) {
      if (bscolumn.test(i)) {
        cs_header = GetHeaderText(i);
        if (!cs_header.IsEmpty()) {
            m_ctlItemList.InsertColumn(0, cs_header);
            m_ctlItemList.SetColumnWidth(0, GetHeaderWidth(i));
            hdi.lParam = i;
            m_pctlItemListHdr->SetItem(0, &hdi);
        }
      }
    }

    SetHeaderInfo();
}

void
DboxMain::SetHeaderInfo()
{
  int i;
  HDITEM hdi;
  hdi.mask = HDI_LPARAM | HDI_WIDTH | HDI_ORDER;

  m_nColumns = m_pctlItemListHdr->GetItemCount();
  ASSERT(m_nColumns > 1);  // Title & User are mandatory!

  // re-initialise array
  for (i = 0; i < CItemData::LAST; i++) {
    m_nColumnTypeToItem[i] = -1;
    m_nColumnOrderToItem[i] = -1;
    m_nColumnTypeByItem[i] = -1;
    m_nColumnWidthByItem[i] = -1;
  }

  m_pctlItemListHdr->GetOrderArray(m_nColumnOrderToItem, m_nColumns);

  for (i = 0; i < m_nColumns; i++) {
    m_pctlItemListHdr->GetItem(m_nColumnOrderToItem[i], &hdi);
    ASSERT(i == hdi.iOrder);
    m_nColumnTypeToItem[hdi.lParam] = m_nColumnOrderToItem[i];
    m_nColumnTypeByItem[i] = hdi.lParam;
    m_nColumnWidthByItem[i] = hdi.cxy;
  }

  // Check sort column still there
  if (m_nColumnTypeToItem[m_iSortedColumn] == -1) {
    // No - take highest visible
      for (i = 0; i < CItemData::LAST; i++) {
          if (m_nColumnTypeToItem[i] != -1) {
              m_iSortedColumn = i;
              break;
          }
      }
  }

  for (i = 0; i < (m_nColumns - 1); i++) {
    int itype = m_nColumnTypeByItem[m_nColumnOrderToItem[i]];
    if (m_nColumnWidthByItem[m_nColumnOrderToItem[i]] < m_nColumnHeaderWidthByType[itype]) {
      m_ctlItemList.SetColumnWidth(i, m_nColumnHeaderWidthByType[itype]);
      m_nColumnWidthByItem[m_nColumnOrderToItem[i]] = m_nColumnHeaderWidthByType[itype];
    }
  }

  // Last column is special
  m_ctlItemList.SetColumnWidth(m_nColumns - 1, LVSCW_AUTOSIZE_USEHEADER);
  m_nColumnWidthByItem[m_nColumnOrderToItem[m_nColumns - 1]] = 
    m_ctlItemList.GetColumnWidth(m_nColumns - 1);
}

void
DboxMain::OnResetColumns()
{
  int i;

  // Delete all existing columns
  for (i = 0; i < m_nColumns; i++) {
    m_ctlItemList.DeleteColumn(0);
  }

  // re-initialise array
  for (i = 0; i < CItemData::LAST; i++)
    m_nColumnTypeToItem[i] = -1;

  // Set default columns
  SetColumns();

  // Refresh the ListView
  RefreshList();

  // Reset the column widths
  ResizeColumns();
}

void
DboxMain::ResizeColumns()
{
  for (int i = 0; i < (m_nColumns - 1); i++) {
    m_ctlItemList.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int itype = m_nColumnTypeByItem[m_nColumnOrderToItem[i]];
    if (m_nColumnWidthByItem[m_nColumnOrderToItem[i]] < m_nColumnHeaderWidthByType[itype])
      m_ctlItemList.SetColumnWidth(i, m_nColumnHeaderWidthByType[itype]);
  }

  // Last column is special
  m_ctlItemList.SetColumnWidth(m_nColumns - 1, LVSCW_AUTOSIZE_USEHEADER);
}

void
DboxMain::OnColumnPicker()
{
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;
  CItemData::FieldBits bsColumn;
  CItemData::FieldBits bsOldColumn;
  CItemData::FieldBits bsNewColumn;

  int i;

  for (i = 0; i < m_nColumns; i++) {
    bsOldColumn.set(m_nColumnTypeByItem[i], true);
  }

  CColumnPickerDlg cpk;

  cpk.m_bsColumn = bsOldColumn;

  int rc = cpk.DoModal();

  if (rc == IDOK) {
    if (cpk.m_bsColumn == bsColumn)
        return;

    bsNewColumn = cpk.m_bsColumn;

    // Find unwanted columns and delete them
    bsColumn = bsOldColumn & ~bsNewColumn;
    if (bsColumn.any()) {
      for (i = CItemData::LAST - 1; i >= 0; i--) {
        if (bsColumn.test(i)) {
          m_ctlItemList.DeleteColumn(m_nColumnTypeToItem[i]);
        }
      }
    }

    // Find new columns wanted and add them to the front
    // as we can't guess the order the user wanted
    bsColumn = bsNewColumn & ~bsOldColumn;
    if (bsColumn.any())
      SetColumns(bsColumn);

    // Refresh the ListView
    RefreshList();
    ResizeColumns();
  }
}

CString DboxMain::GetHeaderText(const int ihdr)
{
  CString cs_header;
  switch (ihdr) {
    case CItemData::GROUP:
      cs_header.LoadString(IDS_GROUP);
      break;
    case CItemData::TITLE:
      cs_header.LoadString(IDS_TITLE);
      break;
    case CItemData::USER:
      cs_header.LoadString(IDS_USERNAME);
      break;
    case CItemData::PASSWORD:
      cs_header.LoadString(IDS_PASSWORD);
      break;
    case CItemData::NOTES:
      cs_header.LoadString(IDS_NOTES);
      break;
    case CItemData::CTIME:        
      cs_header.LoadString(IDS_CREATED);
      break;
    case CItemData::PMTIME:
      cs_header.LoadString(IDS_PASSWORDMODIFIED);
      break;
    case CItemData::ATIME:
      cs_header.LoadString(IDS_LASTACCESSED);
      break;
    case CItemData::LTIME:
      cs_header.LoadString(IDS_PASSWORDEXPIRYDATE);
      break;
    case CItemData::RMTIME:
      cs_header.LoadString(IDS_LASTMODIFIED);
      break;
    default:
      cs_header.Empty();
  }
  return cs_header;
}

int DboxMain::GetHeaderWidth(const int ihdr)
{
  int nWidth(0);
      
  switch (ihdr) {
    case CItemData::GROUP:
    case CItemData::TITLE:
    case CItemData::USER:
    case CItemData::PASSWORD:
    case CItemData::NOTES:
      nWidth = LVSCW_AUTOSIZE_USEHEADER;
      break;
    case CItemData::CTIME:        
    case CItemData::PMTIME:
    case CItemData::ATIME:
    case CItemData::LTIME:
    case CItemData::RMTIME:
      nWidth = m_iDateTimeFieldWidth;
      break;
    default:
      break;
  }

  return nWidth;
}

void DboxMain::CalcHeaderWidths()
{
  // Get default column width for datetime fields
  TCHAR time_str[80], datetime_str[80];
  // Use "fictitious" longest English date
  SYSTEMTIME systime;
  systime.wYear = (WORD)2000;
  systime.wMonth = (WORD)9;
  systime.wDay = (WORD)30;
  systime.wDayOfWeek = (WORD)3;
  systime.wHour = (WORD)23;
  systime.wMinute = (WORD)44;
  systime.wSecond = (WORD)55;
  systime.wMilliseconds = (WORD)0;
  TCHAR szBuf[80];
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, szBuf, 80));
  GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, szBuf, datetime_str, 80);
  szBuf[0] = _T(' ');  // Put a blank between date and time
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, &szBuf[1], 79));
  GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, szBuf, time_str, 80);
#if _MSC_VER >= 1400
  _tcscat_s(datetime_str, 80, time_str);
#else
  _tcscat(datetime_str, 80, time_str);
#endif

  m_iDateTimeFieldWidth = m_ctlItemList.GetStringWidth(datetime_str) + 6;
      
  CString cs_header;
  for (int i = 0; i < CItemData::LAST; i++) {
    switch (i) {
      case CItemData::GROUP:
        cs_header.LoadString(IDS_GROUP);
        break;
      case CItemData::TITLE:
        cs_header.LoadString(IDS_TITLE);
        break;
      case CItemData::USER:
        cs_header.LoadString(IDS_USERNAME);
        break;
      case CItemData::PASSWORD:
        cs_header.LoadString(IDS_PASSWORD);
        break;
      case CItemData::NOTES:
        cs_header.LoadString(IDS_NOTES);
        break;
      case CItemData::CTIME:        
        cs_header.LoadString(IDS_CREATED);
        break;
      case CItemData::PMTIME:
        cs_header.LoadString(IDS_PASSWORDMODIFIED);
        break;
      case CItemData::ATIME:
        cs_header.LoadString(IDS_LASTACCESSED);
        break;
      case CItemData::LTIME:
        cs_header.LoadString(IDS_PASSWORDEXPIRYDATE);
        break;
      case CItemData::RMTIME:
        cs_header.LoadString(IDS_LASTMODIFIED);
        break;
      default:
        cs_header.Empty();
    }

    if (!cs_header.IsEmpty())
      m_nColumnHeaderWidthByType[i] = m_ctlItemList.GetStringWidth(cs_header) + 20;
    else
      m_nColumnHeaderWidthByType[i] = -4;
  }
}
