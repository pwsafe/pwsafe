/// file DboxView.cpp
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
#include "AddDlg.h"
#include "ConfirmDeleteDlg.h"
#include "EditDlg.h"
#include "QuerySetDef.h"
#include "RemindSaveDlg.h"
#include "TryAgainDlg.h"

#include "corelib/pwsprefs.h"
#include "KeySend.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void FixListIndexes(CListCtrl &clist)
{
  int N = clist.GetItemCount();
  for (int i = 0; i < N; i++) {
    CItemData *ci = (CItemData *)clist.GetItemData(i);
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    if (di->list_index != i)
      di->list_index = i;
  }
}

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

  const UINT statustext = IDS_STATCOMPANY;

  // Add the status bar
  if (m_statusBar.Create(this))
    {
      m_statusBar.SetIndicators(&statustext, 1);
      // Make a sunken or recessed border around the first pane
      m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
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

//Add an item
void
DboxMain::OnAdd() 
{
  CAddDlg dataDlg(this);
  m_LockDisabled = true;
  
  if (m_core.GetUseDefUser()) {
    dataDlg.m_username = m_core.GetDefUsername();
  }
  // m_TreeViewGroup may be set by OnContextMenu, if not, try to grok it
  if (m_TreeViewGroup.IsEmpty()) {
    CItemData *itemData = NULL;
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        itemData = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (itemData != NULL) { // leaf selected
          m_TreeViewGroup = itemData->GetGroup();
        } else { // node selected
          m_TreeViewGroup = CMyString(m_ctlItemTree.GetGroup(ti));
        }
      }
    } else { // list view
      // XXX TBD - get group name of currently selected list entry
    }
  }
  dataDlg.m_group = m_TreeViewGroup;
  m_TreeViewGroup = _T(""); // for next time
  app.DisableAccelerator();
  int rc = dataDlg.DoModal();
  app.EnableAccelerator();
	
  if (rc == IDOK) {
    PWSprefs *prefs = PWSprefs::GetInstance();
    //Check if they wish to set a default username
    if (!m_core.GetUseDefUser()
        && (prefs->GetPref(PWSprefs::QuerySetDef))
        && (!dataDlg.m_username.IsEmpty())) {
	  CQuerySetDef defDlg(this);
	  defDlg.m_message =
        _T("Would you like to set \"")
        + (const CString&)dataDlg.m_username
        + _T("\" as your default username?\n\nIt would then automatically be ")
	    + _T("put in the dialog each time you add a new item.");
	  int rc2 = defDlg.DoModal();
	  if (rc2 == IDOK) {
        prefs->SetPref(PWSprefs::UseDefUser, true);
        prefs->SetPref(PWSprefs::DefUserName,
                       dataDlg.m_username);
        m_core.SetUseDefUser(true);
        m_core.SetDefUsername(dataDlg.m_username);
        RefreshList();
      }
	}
    //Finish Check (Does that make any geographical sense?)
    CItemData temp;
    CMyString user;
    time_t t;
    if (dataDlg.m_username.IsEmpty() && m_core.GetUseDefUser())
      user = m_core.GetDefUsername();
    else
      user = dataDlg.m_username;
    temp.CreateUUID();
    temp.SetGroup(dataDlg.m_group);
    temp.SetTitle(dataDlg.m_title);
    temp.SetUser(user);
    temp.SetPassword(dataDlg.m_password);
    temp.SetNotes(dataDlg.m_notes);
    temp.SetURL(dataDlg.m_URL);
    temp.SetAutoType(dataDlg.m_autotype);
   	time(&t);
   	temp.SetCTime(t);
   	temp.SetLTime(dataDlg.m_tttLTime);
    m_core.AddEntryToTail(temp);
    int newpos = insertItem(m_core.GetTailEntry());
    SelectEntry(newpos);
    FixListIndexes(m_ctlItemList);
    m_ctlItemList.SetFocus();
    if (prefs->GetPref(PWSprefs::SaveImmediately))
      {
        Save();
      }
    ChangeOkUpdate();
	uuid_array_t RUEuuid;
	temp.GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);
  }
  else if (rc == IDCANCEL)
    {
    }
  m_LockDisabled = false;
}

//Add a group (tree view only)
void
DboxMain::OnAddGroup()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  if (m_ctlItemTree.IsWindowVisible()) {
    // This can be reached by right clicking over an existing group node
    // or by clicking over "whitespace".
    // If the former, add a child node to the current one
    // If the latter, add to root.
    if (m_TreeViewGroup.IsEmpty())
      m_TreeViewGroup = _T("New Group");
    else
      m_TreeViewGroup += _T(".New Group");
    HTREEITEM newGroup = m_ctlItemTree.AddGroup(m_TreeViewGroup);
    m_ctlItemTree.SelectItem(newGroup);
    m_TreeViewGroup = _T(""); // for next time
    m_ctlItemTree.EditLabel(newGroup);
  }
}

// Delete key was pressed (in list view or tree view) to delete an entry.
void
DboxMain::OnDelete() 
{
  m_LockDisabled = true;
  const bool dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);

  bool dodelete = true;
    
  //Confirm whether to delete the item
  if (!dontaskquestion) {
    CConfirmDeleteDlg deleteDlg(this);
    int rc = deleteDlg.DoModal();
    if (rc == IDCANCEL) {
      dodelete = false;
    }
  }
  
  if (!dodelete) {
    m_LockDisabled = false;
    return;
  }

  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
	//  Needed for DeleteTrayRecentEntry later on
	uuid_array_t RUEuuid;
	ci->GetUUID(RUEuuid);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    int curSel = di->list_index;
    // Find next in treeview, not always curSel after deletion
    HTREEITEM curTree_item = di->tree_item;
    HTREEITEM nextTree_item = m_ctlItemTree.GetNextItem(curTree_item,
                                                        TVGN_NEXT);
    POSITION listindex = Find(curSel); // Must Find before delete from m_ctlItemList

    m_ctlItemList.DeleteItem(curSel);
    m_ctlItemTree.DeleteWithParents(di->tree_item);
    delete di;
    m_core.RemoveEntryAt(listindex);
    FixListIndexes(m_ctlItemList);
    if (m_ctlItemList.IsWindowVisible()) {
      if (m_core.GetNumEntries() > 0) {
        SelectEntry(curSel < m_core.GetNumEntries() ? 
                    curSel : m_core.GetNumEntries() - 1);
      }
      m_ctlItemList.SetFocus();
    } else {// tree view visible
      if (nextTree_item != NULL)
        m_ctlItemTree.SelectItem(nextTree_item);
      else
        SelectEntry(0);
      m_ctlItemTree.SetFocus();
    }
    ChangeOkUpdate();
    m_RUEList.DeleteRUEntry(RUEuuid);
  } else { // !SelItemOk()
    if (m_ctlItemTree.IsWindowVisible()) {
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) {
        if (!m_ctlItemTree.IsLeafNode(ti)) {
          HTREEITEM cti = m_ctlItemTree.GetChildItem(ti);
          const bool orig_dont_ask = dontaskquestion;
          // don't question user for each leaf!
          PWSprefs::GetInstance()->
            SetPref(PWSprefs::DeleteQuestion, true);

          while (cti != NULL) {
            m_ctlItemTree.SelectItem(cti);
            OnDelete(); // recursion - I'm so lazy!
            cti = m_ctlItemTree.GetChildItem(ti);
          }

          // restore original preference after recursion
          PWSprefs::GetInstance()->
            SetPref(PWSprefs::DeleteQuestion, orig_dont_ask);

          //  delete an empty group.
          HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);            
          m_ctlItemTree.DeleteItem(ti);
          m_ctlItemTree.SelectItem(parent);
        }
      }
    }  
  }
  m_LockDisabled = false;
}

void
DboxMain::OnRename() 
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  // Renaming is only allowed while in Tree mode.
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hItem = m_ctlItemTree.GetSelectedItem();
    if (hItem != NULL)
      m_ctlItemTree.EditLabel(hItem);
  }
}

void
DboxMain::OnEdit() 
{
  // Note that Edit is also used for just viewing - don't want to disable
  // viewing in read-only mode
  m_LockDisabled = true;
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    POSITION listpos = Find(di->list_index);

    CEditDlg dlg_edit(this);
    CMyString oldGroup, oldTitle, oldUsername, oldRealPassword, oldURL,
		oldAutoType, oldNotes, oldLTime;
    oldGroup = dlg_edit.m_group = ci->GetGroup();
    oldTitle = dlg_edit.m_title = ci->GetTitle();
    oldUsername = dlg_edit.m_username = ci->GetUser();
    oldRealPassword = dlg_edit.m_realpassword = ci->GetPassword();
    oldURL = dlg_edit.m_URL = ci->GetURL();
    oldAutoType = dlg_edit.m_autotype = ci->GetAutoType();
    dlg_edit.m_password = HIDDEN_PASSWORD;
    oldNotes = dlg_edit.m_notes = ci->GetNotes();
    dlg_edit.m_listindex = listpos;   // for future reference, this is not multi-user friendly
    dlg_edit.m_IsReadOnly = m_IsReadOnly;
	
    dlg_edit.m_ascCTime = ci->GetCTime();
    dlg_edit.m_ascPMTime = ci->GetPMTime();
    dlg_edit.m_ascATime = ci->GetATime();
    dlg_edit.m_ascLTime = ci->GetLTimeN();
    if (dlg_edit.m_ascLTime.GetLength() == 0)
    	dlg_edit.m_ascLTime = "Never";
    oldLTime = dlg_edit.m_ascLTime;
    dlg_edit.m_ascRMTime = ci->GetRMTime();

    app.DisableAccelerator();
    int rc = dlg_edit.DoModal();
    app.EnableAccelerator();

    if (rc == IDOK) {
      CMyString temptitle;
      CMyString user;
      if (dlg_edit.m_username.IsEmpty() && m_core.GetUseDefUser())
        user = m_core.GetDefUsername();
      else
        user = dlg_edit.m_username;
      time_t t;
      time(&t);
      bool bpswdChanged, banotherChanged;
      bpswdChanged = banotherChanged = false;
      if (oldRealPassword != dlg_edit.m_realpassword)
      	bpswdChanged = true;
      else {
      	if (oldGroup != dlg_edit.m_group
      		|| oldTitle != dlg_edit.m_title
      		|| oldUsername != user
      		|| oldNotes != dlg_edit.m_notes
      		|| oldURL != dlg_edit.m_URL
      		|| oldAutoType != dlg_edit.m_autotype
      		|| oldLTime != dlg_edit.m_ascLTime)
      		banotherChanged = true;
      }

	  if (!bpswdChanged && !banotherChanged) {  // Nothing changed!
	  	m_LockDisabled = false;
	  	return;
	  }
	  
	  if (bpswdChanged) {
      	ci->SetPMTime(t);
      	ci->SetRMTime(t);
      }
      
      if (banotherChanged)
        ci->SetRMTime(t);

      ci->SetGroup(dlg_edit.m_group);
      ci->SetTitle(dlg_edit.m_title);
      ci->SetUser(user);
      ci->SetPassword(dlg_edit.m_realpassword);
      ci->SetNotes(dlg_edit.m_notes);
      ci->SetURL(dlg_edit.m_URL);
      ci->SetAutoType(dlg_edit.m_autotype);
      if (oldLTime != dlg_edit.m_ascLTime)
      	ci->SetLTime(dlg_edit.m_tttLTime);

      /*
        Out with the old, in with the new
      */
      CItemData editedItem(*ci); // 'cause next line deletes *ci
      m_core.RemoveEntryAt(listpos);
      m_core.AddEntryToTail(editedItem);
      m_ctlItemList.DeleteItem(di->list_index);
      m_ctlItemTree.DeleteWithParents(di->tree_item);
      di->list_index = -1; // so that insertItem will set new values
      insertItem(m_core.GetTailEntry());
      FixListIndexes(m_ctlItemList);
      if (PWSprefs::GetInstance()->
          GetPref(PWSprefs::SaveImmediately)) {
        Save();
      }
      rc = SelectEntry(di->list_index);
      if (rc == LB_ERR) {
        SelectEntry(m_ctlItemList.GetItemCount() - 1);
      }
      ChangeOkUpdate();
    } // rc == IDOK
  } else { // entry item not selected - perhaps here on Enter on tree item?
    // perhaps not the most elegant solution to improving non-mouse use,
    // but it works. If anyone knows how Enter/Return gets mapped to OnEdit,
    // let me know...
    CItemData *itemData = NULL;
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        itemData = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (itemData == NULL) { // node selected
          m_ctlItemTree.Expand(ti, TVE_TOGGLE);
        }
      }
    }
  }
  m_LockDisabled = false;
}

// Duplicate selected entry but make title unique
void
DboxMain::OnDuplicateEntry() 
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  m_LockDisabled = true;
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
      
    // Get information from current selected entry
    CMyString ci2_group = ci->GetGroup();
    CMyString ci2_user = ci->GetUser();
    CMyString ci2_title0 = ci->GetTitle();
    CMyString ci2_title;
      
    // Find a unique "Title"
    POSITION listpos = NULL;
    int i = 0;
    CString s_copy;
    do {
      i++;
      s_copy.Format(_T("%d"), i);
      ci2_title = ci2_title0 + _T(" Copy #") + CMyString(s_copy);
      listpos = m_core.Find(ci2_group, ci2_title, ci2_user);
    } while (listpos != NULL);
      
    // Set up new entry
    CItemData ci2;
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetPassword( ci->GetPassword() );
    ci2.SetNotes( ci->GetNotes() );
      
    // Add it to the end of the list      
    m_core.AddEntryToTail(ci2);
    di->list_index = -1; // so that insertItem will set new values
    insertItem(m_core.GetTailEntry());
    FixListIndexes(m_ctlItemList);
    if (PWSprefs::GetInstance()->
        GetPref(PWSprefs::SaveImmediately)) {
      Save();
    }
    int rc = SelectEntry(di->list_index);
    if (rc == LB_ERR) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();
	uuid_array_t RUEuuid;
	ci2.GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);

  }
  m_LockDisabled = false;
}

void
DboxMain::OnOK() 
{
  int rc, rc2;

  PWSprefs::IntPrefs WidthPrefs[] = {
    PWSprefs::Column1Width,
    PWSprefs::Column2Width,
    PWSprefs::Column3Width,
    PWSprefs::Column4Width,
  };
  PWSprefs *prefs = PWSprefs::GetInstance();

  LVCOLUMN lvColumn;
  lvColumn.mask = LVCF_WIDTH;
  for (int i = 0; i < 4; i++) {
    if (m_ctlItemList.GetColumn(i, &lvColumn)) {
      prefs->SetPref(WidthPrefs[i], lvColumn.cx);
    }
  }

#if !defined(POCKET_PC)
  if (!IsIconic()) {
    CRect rect;
    GetWindowRect(&rect);
    prefs->SetPrefRect(rect.top, rect.bottom, rect.left, rect.right);
  }
#endif
  prefs->SetPref(PWSprefs::SortedColumn, m_iSortedColumn);
  prefs->SetPref(PWSprefs::SortAscending, m_bSortAscending);

  if (m_core.IsChanged()) {
    rc = MessageBox(_T("Do you want to save changes to the password list?"),
                    AfxGetAppName(),
                    MB_ICONQUESTION|MB_YESNOCANCEL);
    switch (rc) {
	case IDCANCEL:
	  return;
	case IDYES:
	  rc2 = Save();
	  if (rc2 != PWScore::SUCCESS)
        return;
	case IDNO:
	  break;
    }
  } // core.IsChanged()

  // Clear clipboard on Exit? If the app is minimized and the systemtray
  // is enabled, we should do so, regardless of the DontAskMinimizeClearYesNo
  // pref
  if (!IsWindowVisible() &&
      prefs->GetPref(PWSprefs::UseSystemTray)) {
    ClearClipboard();
  } else if (prefs->GetPref(PWSprefs::DontAskMinimizeClearYesNo))
    ClearClipboard();

  ClearData();

  //Store current filename for next time...
  if (m_saveMRU)
	  prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());
  else
	  prefs->SetPref(PWSprefs::CurrentFile, "");

  CDialog::OnOK();
}


void
DboxMain::OnCancel()
{
  // If system tray is enabled, cancel (X on title bar) closes
  // window, else exit application
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray))
    ShowWindow(SW_MINIMIZE);
  else
    OnOK();
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

  POSITION listPos = m_core.GetFirstEntryPosition();
  CMyString curtitle, curuser, curnotes, curgroup, savetitle;
  CMyString listTitle;
  CString searchstr(str); // Since str is const, and we might need to MakeLower
  int retval = 0;

  if (!CaseSensitive)
    searchstr.MakeLower();

  while (listPos != NULL)
    {
      const CItemData &curitem = m_core.GetEntryAt(listPos);
      savetitle = curtitle = curitem.GetTitle(); // savetitle keeps orig case
      curuser =  curitem.GetUser();
      curnotes = curitem.GetNotes();
      curgroup = curitem.GetGroup();

      if (!CaseSensitive) {
        curtitle.MakeLower();
        curuser.MakeLower();
        curnotes.MakeLower();
        curgroup.MakeLower();
      }
      if (::strstr(curtitle, searchstr) ||
          ::strstr(curuser, searchstr) ||
          ::strstr(curnotes, searchstr) ||
          ::strstr(curgroup, searchstr)) {
        // Find index in displayed list
        DisplayInfo *di = (DisplayInfo *)curitem.GetDisplayInfo();
        ASSERT(di != NULL);
        int li = di->list_index;
        ASSERT(CMyString(m_ctlItemList.GetItemText(li, 0)) == savetitle);
        // add to indices, bump retval
        indices[retval++] = li;
      } // match found in m_pwlist
      m_core.GetNextEntry(listPos);
    }

  // Sort indices
  if (retval > 1)
    ::qsort((void *)indices, retval, sizeof(indices[0]), compint);
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
// updates of windows suspended untill all data is in.
void
DboxMain::RefreshList()
{
  if (! m_windowok)
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

  FixListIndexes(m_ctlItemList);
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
      ClearClipboard();
    if (prefs->GetPref(PWSprefs::DatabaseClear)) {
      bool dontask = prefs->GetPref(PWSprefs::DontAskSaveMinimize);
      bool doit = true;
      if ((m_core.IsChanged()) && !dontask) {
        CRemindSaveDlg remindDlg(this);

        int rc = remindDlg.DoModal();
        if (rc == IDOK) {
        } else if (rc == IDCANCEL) {
          doit = FALSE;
        }
      }

      if (doit) {
        if ( m_core.IsChanged() ) // only save if changed
          OnSave();
        ClearData(false);
        m_needsreading = true;
      }
    }
    if (PWSprefs::GetInstance()->
        GetPref(PWSprefs::UseSystemTray)) {      
      app.SetMenuDefaultItem(ID_MENUITEM_UNMINIMIZE);
      ShowWindow(SW_HIDE);
    }
 
  }
  else if (!m_bSizing && nType == SIZE_RESTORED) { // gets called even when just resizing window
#endif
    app.SetMenuDefaultItem(ID_MENUITEM_MINIMIZE);
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
DboxMain::OnSetfocusItemlist( NMHDR *, LRESULT *) 
{
  const int dca = int(PWSprefs::GetInstance()->
		      GetPref(PWSprefs::DoubleClickAction));
  UINT statustext;
  switch (dca) {
  case PWSprefs::DoubleClickCopy: statustext = IDS_STATCOPY; break;
  case PWSprefs::DoubleClickEdit: statustext = IDS_STATEDIT; break;
  case PWSprefs::DoubleClickAutoType: statustext = IDS_STATAUTOTYPE; break;
  case PWSprefs::DoubleClickBrowse: statustext = IDS_STATBROWSE; break;
  default: ASSERT(0);
  }

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
    if (tLTime != 0 && tLTime < now)
    	m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::EXPIRED_LEAF, CMyTreeCtrl::EXPIRED_LEAF);
    else    
    m_ctlItemTree.SetItemImage(ti, CMyTreeCtrl::LEAF, CMyTreeCtrl::LEAF);
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
  UpdateSystemTray(LOCKED);
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
  FixListIndexes(m_ctlItemList);
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
     * Also, if m_LockDisabled is set, do nothing - this is set when
     * a dialog box is open.
     */
    if((!m_core.IsChanged() || Save() == PWScore::SUCCESS) &&
       !m_LockDisabled){
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

// onAutoType handles menu item ID_MENUITEM_AUTOTYPE

void
DboxMain::OnAutoType()
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
	uuid_array_t RUEuuid;
	ci->GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);
	if (!m_IsReadOnly && m_bMaintainDateTimeStamps) {
   		ci->SetATime();
       	SetChanged(true);
    }
    // All code using ci must be before this AutoType since the
	// latter may trash *ci if lock-on-minimize
	AutoType(*ci);
  }
}

void
DboxMain::AutoType(const CItemData &ci)
{
  CMyString AutoCmd = ci.GetAutoType();
 const CMyString user(ci.GetUser());
 const CMyString pwd(ci.GetPassword());
 if(AutoCmd.IsEmpty()){
   // checking for user and password for default settings
   if(!pwd.IsEmpty()){
     if(!user.IsEmpty())
       AutoCmd="\\u\\t\\p\\n";
     else
       AutoCmd="\\p\\n";
   }
 }
		
 CMyString tmp;
 char curChar;
 const int N = AutoCmd.GetLength();
 CKeySend ks;
 ks.ResetKeyboardState();

 // Note that minimizing the window before calling ci.Get*()
 // will cause garbage to be read if "lock on minimize" selected,
 // since that will clear the data [Bugs item #1026630]
 // (this is why we read user & pwd before actual use)

 ShowWindow(SW_MINIMIZE);
 Sleep(1000); // Karl Student's suggestion, to ensure focus set correctly on minimize.

 for(int n=0; n < N; n++){
   curChar=AutoCmd[n];
   if(curChar=='\\'){
     n++;
     if(n<N)
       curChar=AutoCmd[n];
     switch(curChar){
     case '\\':
       tmp += '\\';
       break;
     case 'n':case 'r':
       tmp += '\r';
       break;
     case 't':
       tmp += '\t';
       break;
     case 'u':
       tmp += user;
       break;
     case 'p':
       tmp += pwd;
       break;
     case 'd': {
       // Delay is going to change - send what we have with old delay
       ks.SendString(tmp);
       // start collecting new delay
       tmp = "";
       int newdelay = 0;
       int gNumIts = 0;
						
       for(n++; n<N && (gNumIts < 3); ++gNumIts, n++)
         if(isdigit(AutoCmd[n])){
           newdelay *= 10;
           newdelay += (AutoCmd[n]-'0');
         } else
           break; // for loop
       n--;							
       ks.SetAndDelay(newdelay);

       break; // case
     }
     default:
       tmp+="\\"+curChar;
       break;
     }
   }
   else
     tmp += curChar;
 }
 ks.SendString(tmp);
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
DboxMain::UpdateSystemTray(STATE s)
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
