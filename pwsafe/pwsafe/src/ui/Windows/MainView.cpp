/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// file MainView.cpp
//
// View-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "AddEdit_PropertySheet.h"
#include "DboxMain.h"
#include "TryAgainDlg.h"
#include "ColumnChooserDlg.h"
#include "GeneralMsgBox.h"
#include "PWFontDialog.h"
#include "PWFont.h"
#include "InfoDisplay.h"
#include "ViewReport.h"

#include "VirtualKeyboard\VKeyBoardDlg.h"

#include "corelib/pwsprefs.h"
#include "corelib/UUIDGen.h"
#include "corelib/corelib.h"
#include "corelib/PWHistory.h"
#include "corelib/StringXStream.h"

#include "os/Debug.h"
#include "os/dir.h"
#include "os/run.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#include "commctrl.h"
#include <shlwapi.h>
#include <vector>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

extern const wchar_t *GROUP_SEP2;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DboxMain::DatabaseModified(bool bChanged)
{
  // Callback from PWScore if the database has been changed
  // entries or preferences stored in the database

  // Callback from PWScore if the password list has been changed,
  // invalidating the indices vector in Find
  InvalidateSearch();
  OnHideFindToolBar();

  // This is to prevent Windows (Vista & later) from shutting down
  // if the database has been modified (including preferences stored in the DB)
  static bool bCurrentState(false);

  // Don't do anything if status unchanged or not at least Vista
  if (m_WindowsMajorVersion < 6 || 
      m_core.IsReadOnly() || bChanged == bCurrentState)
    return;

  bCurrentState = bChanged;

  // Only supported on Vista and later
  if (bCurrentState) {
    if (m_pfcnShutdownBlockReasonCreate != NULL) {
      CString cs_stopreason;
      cs_stopreason.Format(IDS_STOPREASON, m_core.GetCurFile().c_str());
      m_pfcnShutdownBlockReasonCreate(m_hWnd, cs_stopreason);
      m_bBlockShutdown = true;
    }
  } else {
    if (m_pfcnShutdownBlockReasonDestroy != NULL) {
      m_pfcnShutdownBlockReasonDestroy(m_hWnd);
      m_bBlockShutdown = false;
    }
  }
}

void DboxMain::UpdateGUI(UpdateGUICommand::GUI_Action ga, 
                         uuid_array_t &entry_uuid, CItemData::FieldType ft,
                         bool bUpdateGUI)
{
  // Callback from PWScore if GUI needs updating
  // Note: For some values of 'ga', 'ci' & ft are invalid and not used.
 
  // "bUpdateGUI" is only used by GUI_DELETE_ENTRY when called as part
  // of the Edit Entry Command where the entry is deleted and then added and
  // the GUI should not be updated until after the Add.
  CItemData *pci(NULL);

  ItemListIter pos = Find(entry_uuid);
  if (pos != End()) {
    pci = &pos->second;
  }

  PWSprefs *prefs = PWSprefs::GetInstance();

  switch (ga) {
    case UpdateGUICommand::GUI_UPDATE_STATUSBAR:
      UpdateToolBarDoUndo();
      UpdateStatusBar();
      break;
    case UpdateGUICommand::GUI_ADD_ENTRY:
      AddToGUI(*pci);
      break;
    case UpdateGUICommand::GUI_DELETE_ENTRY:
      RemoveFromGUI(*pci, bUpdateGUI);
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRYFIELD:
      RefreshEntryFieldInGUI(*pci, ft);
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD:
      RefreshEntryPasswordInGUI(*pci);
      break;
    case UpdateGUICommand::GUI_REDO_IMPORT:
    case UpdateGUICommand::GUI_UNDO_IMPORT:
    case UpdateGUICommand::GUI_REDO_MERGESYNC:
    case UpdateGUICommand::GUI_UNDO_MERGESYNC:
      // During these processes, many entries may be added/removed
      // To stop the UI going nuts, updates to the UI are suspended until
      // the action is complete - when these calls are then sent
      RebuildGUI();
      break;
    case UpdateGUICommand::GUI_REFRESH_TREE:
      // Rebuid the entire tree view
      RebuildGUI(iTreeOnly);
      break;
    case UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED:
      // Change any impact on the application due to a database preference change
      // Currently - only Idle Timeout values
      KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
      ResetIdleLockCounter();
      if (prefs->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
        SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
      }
      break;
    default:
      break;
  }
}

// Called from PWScore to get GUI to update its reserved field
void DboxMain::GUISetupDisplayInfo(CItemData &ci)
{
  ci.SetDisplayInfo(new DisplayInfo);
}

void DboxMain::GUIRefreshEntry(const CItemData &ci)
{
  UpdateEntryImages(ci);
}

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
  // m_iTypeSortColumn to determine which column is getting sorted
  // which is by column data TYPE and NOT by column index!
  // m_bSortAscending to determine the direction of the sort (duh)

  DboxMain *self = (DboxMain *)closure;
  ASSERT(self != NULL);

  const int nTypeSortColumn = self->m_iTypeSortColumn;
  CItemData* pLHS = (CItemData *)lParam1;
  CItemData* pRHS = (CItemData *)lParam2;
  StringX group1, group2;
  time_t t1, t2;
  int xint1, xint2;

  int iResult;
  switch(nTypeSortColumn) {
    case CItemData::UUID:  // Image
      iResult = (pLHS->GetEntryType() < pRHS->GetEntryType()) ? -1 : 1;
      break;
    case CItemData::GROUP:
      group1 = pLHS->GetGroup();
      group2 = pRHS->GetGroup();
      if (group1.empty())  // root?
        group1 = L"\xff";
      if (group2.empty())  // root?
        group2 = L"\xff";
      iResult = CompareNoCase(group1, group2);
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS->GetTitle(), pRHS->GetTitle());
        if (iResult == 0) {
          iResult = CompareNoCase(pLHS->GetUser(), pRHS->GetUser());
        }
      }
      break;
    case CItemData::TITLE:
      iResult = CompareNoCase(pLHS->GetTitle(), pRHS->GetTitle());
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS->GetUser(), pRHS->GetUser());
      }
      break;
    case CItemData::USER:
      iResult = CompareNoCase(pLHS->GetUser(), pRHS->GetUser());
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS->GetTitle(), pRHS->GetTitle());
      }
      break;
    case CItemData::NOTES:
      iResult = CompareNoCase(pLHS->GetNotes(), pRHS->GetNotes());
      break;
    case CItemData::PASSWORD:
      iResult = CompareNoCase(pLHS->GetPassword(), pRHS->GetPassword());
      break;
    case CItemData::URL:
      iResult = CompareNoCase(pLHS->GetURL(), pRHS->GetURL());
      break;
    case CItemData::EMAIL:
      iResult = CompareNoCase(pLHS->GetEmail(), pRHS->GetEmail());
      break;
    case CItemData::RUNCMD:
      iResult = CompareNoCase(pLHS->GetRunCommand(), pRHS->GetRunCommand());
      break;
    case CItemData::AUTOTYPE:
      iResult = CompareNoCase(pLHS->GetAutoType(), pRHS->GetAutoType());
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
    case CItemData::XTIME:
      pLHS->GetXTime(t1);
      pRHS->GetXTime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case CItemData::XTIME_INT:
      pLHS->GetXTimeInt(xint1);
      pRHS->GetXTimeInt(xint2);
      iResult = (xint1 < xint2) ? -1 : 1;
      break;
    case CItemData::RMTIME:
      pLHS->GetRMTime(t1);
      pRHS->GetRMTime(t2);
      iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case CItemData::POLICY:
      iResult = CompareNoCase(pLHS->GetPWPolicy(), pRHS->GetPWPolicy());
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

void DboxMain::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(DboxMain)
  DDX_Control(pDX, IDC_ITEMLIST, m_ctlItemList);
  DDX_Control(pDX, IDC_ITEMTREE, m_ctlItemTree);
  DDX_Control(pDX, IDC_STATIC_DRAGGROUP, m_DDGroup);
  DDX_Control(pDX, IDC_STATIC_DRAGTITLE, m_DDTitle);
  DDX_Control(pDX, IDC_STATIC_DRAGUSER, m_DDUser);
  DDX_Control(pDX, IDC_STATIC_DRAGPASSWORD, m_DDPassword);
  DDX_Control(pDX, IDC_STATIC_DRAGNOTES, m_DDNotes);
  DDX_Control(pDX, IDC_STATIC_DRAGURL, m_DDURL);
  DDX_Control(pDX, IDC_STATIC_DRAGEMAIL, m_DDemail);
  //}}AFX_DATA_MAP
}

void DboxMain::UpdateToolBarROStatus(const bool bIsRO)
{
  if (m_toolbarsSetup == TRUE) {
    BOOL State = bIsRO ? FALSE : TRUE;
    BOOL SaveState = (!bIsRO && (m_core.IsChanged() || m_core.HaveDBPrefsChanged())) ? TRUE : FALSE;
    CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();
    mainTBCtrl.EnableButton(ID_MENUITEM_ADD, State);
    mainTBCtrl.EnableButton(ID_MENUITEM_DELETEENTRY, State);
    mainTBCtrl.EnableButton(ID_MENUITEM_SAVE, SaveState);
  }
}

void DboxMain::UpdateToolBarForSelectedItem(const CItemData *pci)
{
  // Following test required since this can be called on exit, with a pci
  // from ItemData that's already been deleted. Ugh.
  const CItemData *pci_entry(pci);
  if (m_core.GetNumEntries() != 0) {
    BOOL State = (pci_entry == NULL) ? FALSE : TRUE;
    int IDs[] = {ID_MENUITEM_COPYPASSWORD, ID_MENUITEM_COPYUSERNAME,
                 ID_MENUITEM_COPYNOTESFLD, ID_MENUITEM_AUTOTYPE, 
                 ID_MENUITEM_RUNCOMMAND, ID_MENUITEM_EDIT,
                 ID_MENUITEM_PASSWORDSUBSET};

    CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();
    for (int i = 0; i < _countof(IDs); i++) {
      mainTBCtrl.EnableButton(IDs[i], State);
    }

    mainTBCtrl.EnableButton(ID_MENUITEM_UNDO, m_core.AnyToUndo() ? TRUE : FALSE);
    mainTBCtrl.EnableButton(ID_MENUITEM_REDO, m_core.AnyToRedo() ? TRUE : FALSE);

    if (pci_entry != NULL && pci_entry->IsShortcut()) {
      pci_entry = GetBaseEntry(pci_entry);
    }

    if (pci_entry == NULL || pci_entry->IsUserEmpty()) {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYUSERNAME, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYUSERNAME, TRUE);
    }

    if (pci_entry == NULL || pci_entry->IsNotesEmpty()) {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYNOTESFLD, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYNOTESFLD, TRUE);
    }

    if (pci_entry == NULL || pci_entry->IsRunCommandEmpty()) {
      mainTBCtrl.EnableButton(ID_MENUITEM_RUNCOMMAND, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_RUNCOMMAND, TRUE);
    }

    if (pci_entry == NULL || 
        (pci_entry->IsEmailEmpty() && !pci_entry->IsURLEmail())) {
      mainTBCtrl.EnableButton(ID_MENUITEM_SENDEMAIL, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_SENDEMAIL, TRUE);
    }

    if (pci_entry == NULL || pci_entry->IsURLEmpty() || pci_entry->IsURLEmail()) {
      mainTBCtrl.EnableButton(ID_MENUITEM_BROWSEURL, FALSE);
      mainTBCtrl.EnableButton(ID_MENUITEM_BROWSEURLPLUS, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_BROWSEURL, TRUE);
      mainTBCtrl.EnableButton(ID_MENUITEM_BROWSEURLPLUS, TRUE);
    }

    bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
    if (bDragBarState) {
      // Note: Title & Password are mandatory
      if (pci_entry == NULL) {
        m_DDGroup.SetStaticState(m_core.GetNumEntries() != 0);
        m_DDTitle.SetStaticState(false);
        m_DDPassword.SetStaticState(false);
        m_DDUser.SetStaticState(false);
        m_DDNotes.SetStaticState(false);
        m_DDURL.SetStaticState(false);
        m_DDemail.SetStaticState(false);
      } else {
        m_DDGroup.SetStaticState(!pci_entry->IsGroupEmpty());
        m_DDTitle.SetStaticState(true);
        m_DDPassword.SetStaticState(true);
        m_DDUser.SetStaticState(!pci_entry->IsUserEmpty());
        m_DDNotes.SetStaticState(!pci_entry->IsNotesEmpty());
        m_DDURL.SetStaticState(!pci_entry->IsURLEmpty());
        m_DDemail.SetStaticState(!pci_entry->IsEmailEmpty());
      }
    }
  }
}

void DboxMain::setupBars()
{
#if !defined(POCKET_PC)
  // This code is copied from the DLGCBR32 example that comes with MFC

  // Add the status bar
  if (m_statusBar.Create(this)) {
    // Set up DoubleClickAction text
    const int dca = int(PWSprefs::GetInstance()->
      GetPref(PWSprefs::DoubleClickAction));
    switch (dca) {
      case PWSprefs::DoubleClickAutoType:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATAUTOTYPE;
        break;
      case PWSprefs::DoubleClickBrowse:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATBROWSE;
        break;
      case PWSprefs::DoubleClickCopyNotes:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYNOTES;
        break;
      case PWSprefs::DoubleClickCopyPassword:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYPASSWORD;
        break;
      case PWSprefs::DoubleClickCopyUsername:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYUSERNAME;
        break;
      case PWSprefs::DoubleClickViewEdit:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATVIEWEDIT;
        break;
      case PWSprefs::DoubleClickCopyPasswordMinimize:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYPASSWORDMIN;
        break;
      case PWSprefs::DoubleClickBrowsePlus:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATBROWSEPLUS;
        break;
      case PWSprefs::DoubleClickRun:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATRUN;
        break;
      case PWSprefs::DoubleClickSendEmail:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATSENDEMAIL;
        break;
      default:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOMPANY;
    }

    statustext[CPWStatusBar::SB_CLIPBOARDACTION] = IDS_BLANK;
    // Set up Configuration source indicator (debug only)
#if defined(_DEBUG) || defined(DEBUG)
    statustext[CPWStatusBar::SB_CONFIG] = PWSprefs::GetInstance()->GetConfigIndicator();
#endif /* DEBUG */
    // Set up the rest - all but one empty as pane now re-sized according to contents
    statustext[CPWStatusBar::SB_MODIFIED] = IDS_BLANK;
    statustext[CPWStatusBar::SB_NUM_ENT] = IDS_BLANK;
    statustext[CPWStatusBar::SB_FILTER] = IDS_BLANK;
    statustext[CPWStatusBar::SB_READONLY] = IDS_READ_ONLY;

    // And show
    m_statusBar.SetIndicators(statustext, CPWStatusBar::SB_TOTAL);

    UINT uiID, uiStyle;
    int cxWidth;
    m_statusBar.GetPaneInfo(CPWStatusBar::SB_FILTER, uiID,
                            uiStyle, cxWidth);
    int iBMWidth = m_statusBar.GetBitmapWidth();
    m_statusBar.SetPaneInfo(CPWStatusBar::SB_FILTER, uiID,
                            uiStyle | SBT_OWNERDRAW, iBMWidth);

    // Make a sunken or recessed border around the first pane
    m_statusBar.SetPaneInfo(CPWStatusBar::SB_DBLCLICK, 
                            m_statusBar.GetItemID(CPWStatusBar::SB_DBLCLICK), 
                            SBPS_STRETCH, NULL);
  }

  CDC* pDC = this->GetDC();
  int NumBits = (pDC ? pDC->GetDeviceCaps(12 /*BITSPIXEL*/) : 32);
  m_MainToolBar.Init(NumBits);
  m_FindToolBar.Init(NumBits, this, PWS_MSG_TOOLBAR_FIND,
                     &m_SaveAdvValues[CAdvancedDlg::ADV_FIND]);
  ReleaseDC(pDC);

  // Add the Main ToolBar.
  if (!m_MainToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
                              WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE |
                              CBRS_TOP | CBRS_SIZE_DYNAMIC,
                              CRect(0, 0, 0, 0), AFX_IDW_RESIZE_BAR + 1)) {
    pws_os::Trace(L"Failed to create Main toolbar\n");
    return;      // fail to create
  }
  DWORD dwStyle = m_MainToolBar.GetBarStyle();
  dwStyle = dwStyle | CBRS_BORDER_BOTTOM | CBRS_BORDER_TOP |
                      CBRS_BORDER_LEFT   | CBRS_BORDER_RIGHT |
                      CBRS_TOOLTIPS | CBRS_FLYBY;
  m_MainToolBar.SetBarStyle(dwStyle);
  m_MainToolBar.SetWindowText(L"Standard");

  // Add the Find ToolBar.
  if (!m_FindToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
                              WS_CHILD | WS_VISIBLE |
                              CBRS_BOTTOM | CBRS_SIZE_DYNAMIC,
                              CRect(0, 0, 0, 0), AFX_IDW_RESIZE_BAR + 2)) {
    pws_os::Trace(L"Failed to create Find toolbar\n");
    return;      // fail to create
  }
  dwStyle = m_FindToolBar.GetBarStyle();
  dwStyle = dwStyle | CBRS_BORDER_BOTTOM | CBRS_BORDER_TOP |
                      CBRS_BORDER_LEFT   | CBRS_BORDER_RIGHT |
                      CBRS_TOOLTIPS | CBRS_FLYBY;
  m_FindToolBar.SetBarStyle(dwStyle);
  m_FindToolBar.SetWindowText(L"Find");


  // Set dragbar & toolbar according to graphic capabilities, overridable by user choice.
  if (NumBits < 16 || !PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar))  {
    SetToolbar(ID_MENUITEM_OLD_TOOLBAR, true);
  } else {
    SetToolbar(ID_MENUITEM_NEW_TOOLBAR, true);
  }

  m_FindToolBar.ShowFindToolBar(false);

  // Set flag - we're done
  m_toolbarsSetup = TRUE;
  m_MainToolBar.ShowWindow(PWSprefs::GetInstance()->GetPref(PWSprefs::ShowToolbar) ?
                           SW_SHOW : SW_HIDE);
  UpdateToolBarROStatus(m_core.IsReadOnly());
  m_menuManager.SetImageList(&m_MainToolBar);
  m_menuManager.SetMapping(&m_MainToolBar);

  m_DDGroup.EnableWindow(TRUE);
  m_DDGroup.ShowWindow(SW_SHOW);
  m_DDTitle.EnableWindow(TRUE);
  m_DDTitle.ShowWindow(SW_SHOW);
  m_DDUser.EnableWindow(TRUE);
  m_DDUser.ShowWindow(SW_SHOW);
  m_DDPassword.EnableWindow(TRUE);
  m_DDPassword.ShowWindow(SW_SHOW);
  m_DDNotes.EnableWindow(TRUE);
  m_DDNotes.ShowWindow(SW_SHOW);
  m_DDURL.EnableWindow(TRUE);
  m_DDURL.ShowWindow(SW_SHOW);
  m_DDemail.EnableWindow(TRUE);
  m_DDemail.ShowWindow(SW_SHOW);
#endif
}

void DboxMain::UpdateListItem(const int lindex, const int type, const StringX &newText)
{
  int iSubItem = m_nColumnIndexByType[type];

  // Ignore if this column is not being displayed
  if (iSubItem < 0)
    return;

  BOOL brc = m_ctlItemList.SetItemText(lindex, iSubItem, newText.c_str());
  ASSERT(brc == TRUE);
  if (m_iTypeSortColumn == type) { // resort if necessary
    m_bSortAscending = PWSprefs::GetInstance()->GetPref(PWSprefs::SortAscending);
    m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
    FixListIndexes();
  }
}

void DboxMain::UpdateTreeItem(const HTREEITEM hItem, const StringX &newText)
{
  CRect rect;
  m_ctlItemTree.SetItemText(hItem, newText.c_str());

  m_ctlItemTree.GetItemRect(hItem, &rect, FALSE);
  m_ctlItemTree.InvalidateRect(&rect);
}

// Find in m_pwlist entry with same title and user name as the i'th entry in m_ctlItemList
ItemListIter DboxMain::Find(int i)
{
  CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(i);
  ASSERT(pci != NULL);
  const StringX curGroup = pci->GetGroup();
  const StringX curTitle = pci->GetTitle();
  const StringX curUser = pci->GetUser();
  return Find(curGroup, curTitle, curUser);
}

/*
* Finds all entries in m_pwlist that contain str in any text
* field, returns their sorted indices in m_listctrl via indices, which is
* assumed to be allocated by caller to DboxMain::GetNumEntries() ints.
* FindAll returns the number of entries that matched.
*/

size_t DboxMain::FindAll(const CString &str, BOOL CaseSensitive,
                         vector<int> &indices)
{
  CItemData::FieldBits bsFields;
  bsFields.set();  // Default search is all text fields!

  return FindAll(str, CaseSensitive, indices, bsFields, BST_UNCHECKED, 
                 L"", 0, 0);
}

size_t DboxMain::FindAll(const CString &str, BOOL CaseSensitive,
                         vector<int> &indices,
                         const CItemData::FieldBits &bsFields, const int subgroup_set, 
                         const CString &subgroup_name, const int subgroup_object,
                         const int subgroup_function)
{
  ASSERT(!str.IsEmpty());
  ASSERT(indices.empty());

  StringX curGroup, curTitle, curUser, curNotes, curPassword, curURL, curAT, curXInt;
  StringX curEmail, curRunCommand, listTitle, saveTitle;
  bool bFoundit;
  CString searchstr(str); // Since str is const, and we might need to MakeLower
  size_t retval = 0;

  if (!CaseSensitive)
    searchstr.MakeLower();

  int ititle(-1);  // Must be there as it is mandatory!
  for (int ic = 0; ic < m_nColumns; ic++) {
    if (m_nColumnTypeByIndex[ic] == CItemData::TITLE) {
      ititle = ic;
      break;
    }
  }

  ItemListConstIter listPos, listEnd;

  OrderedItemList orderedItemList;
  OrderedItemList::const_iterator olistPos, olistEnd;
  if (m_IsListView) {
    listPos = m_core.GetEntryIter();
    listEnd = m_core.GetEntryEndIter();
  } else {
    MakeOrderedItemList(orderedItemList);
    olistPos = orderedItemList.begin();
    olistEnd = orderedItemList.end();
  }

  while (m_IsListView ? (listPos != listEnd) : (olistPos != olistEnd)) {
    const CItemData &curitem = m_IsListView ? listPos->second : *olistPos;
    if (subgroup_set == BST_CHECKED &&
        !curitem.Matches(std::wstring(subgroup_name),
                         subgroup_object, subgroup_function))
      goto nextentry;

    bFoundit = false;
    saveTitle = curTitle = curitem.GetTitle(); // saveTitle keeps orig case
    curGroup = curitem.GetGroup();
    curUser =  curitem.GetUser();
    curPassword = curitem.GetPassword();
    curNotes = curitem.GetNotes();
    curURL = curitem.GetURL();
    curEmail = curitem.GetEmail();
    curRunCommand = curitem.GetRunCommand();
    curAT = curitem.GetAutoType();
    curXInt = curitem.GetXTimeInt();

    if (!CaseSensitive) {
      ToLower(curGroup);
      ToLower(curTitle);
      ToLower(curUser);
      ToLower(curPassword);
      ToLower(curNotes);
      ToLower(curURL);
      ToLower(curEmail);
      ToLower(curRunCommand);
      ToLower(curAT);
    }

    // do loop to easily break out as soon as a match is found
    // saves more checking if entry already selected
    do {
      if (bsFields.test(CItemData::GROUP) && ::wcsstr(curGroup.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::TITLE) && ::wcsstr(curTitle.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::USER) && ::wcsstr(curUser.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::PASSWORD) && ::wcsstr(curPassword.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::NOTES) && ::wcsstr(curNotes.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::URL) && ::wcsstr(curURL.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::EMAIL) && ::wcsstr(curEmail.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::RUNCMD) && ::wcsstr(curRunCommand.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::AUTOTYPE) && ::wcsstr(curAT.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::PWHIST)) {
        size_t pwh_max, err_num;
        PWHistList pwhistlist;
        CreatePWHistoryList(curitem.GetPWHistory(), pwh_max, err_num,
                            pwhistlist, TMC_XML);
        PWHistList::iterator iter;
        for (iter = pwhistlist.begin(); iter != pwhistlist.end();
             iter++) {
          PWHistEntry pwshe = *iter;
          if (!CaseSensitive)
            ToLower(pwshe.password);
          if (::wcsstr(pwshe.password.c_str(), searchstr)) {
            bFoundit = true;
            break;  // break out of for loop
          }
        }
        pwhistlist.clear();
        if (bFoundit)
          break;  // break out of do loop
      }
      if (bsFields.test(CItemData::XTIME_INT) && ::wcsstr(curXInt.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
    } while(FALSE);  // only do it once!

    if (bFoundit) {
      // Find index in displayed list
      DisplayInfo *pdi = (DisplayInfo *)curitem.GetDisplayInfo();
      ASSERT(pdi != NULL);
      int li = pdi->list_index;
      ASSERT(m_ctlItemList.GetItemText(li, ititle) == saveTitle.c_str());
      // add to indices, bump retval
      indices.push_back(li);
    } // match found in m_pwlist

nextentry:
    if (m_IsListView)
      listPos++;
    else
      olistPos++;
  } // while

  retval = indices.size();
  // Sort indices if in List View
  if (m_IsListView && retval > 1)
    sort(indices.begin(), indices.end());

  if (!m_IsListView)
    orderedItemList.clear();

  return retval;
}

//Checks and sees if everything works and something is selected
BOOL DboxMain::SelItemOk()
{
  CItemData *pci = getSelectedItem();
  return (pci == NULL) ? FALSE : TRUE;
}

BOOL DboxMain::SelectEntry(const int i, BOOL MakeVisible)
{
  BOOL retval;
  ASSERT(i >= 0);
  if (m_ctlItemList.GetItemCount() == 0)
    return false;

  if (m_ctlItemList.IsWindowVisible()) {
    retval = m_ctlItemList.SetItemState(i,
                                        LVIS_FOCUSED | LVIS_SELECTED,
                                        LVIS_FOCUSED | LVIS_SELECTED);
    if (MakeVisible) {
      m_ctlItemList.EnsureVisible(i, FALSE);
    }
    m_ctlItemList.Invalidate();
  } else { //Tree view active
    CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(i);
    ASSERT(pci != NULL);
    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
    ASSERT(pdi != NULL);
    ASSERT(pdi->list_index == i);

    // Was there anything selected before?
    HTREEITEM hti = m_ctlItemTree.GetSelectedItem();
    // NULL means nothing was selected.
    if (hti != NULL) {
      // Time to remove the old "fake selection" (a.k.a. drop-hilite)
      // Make sure to undo "MakeVisible" on the previous selection.
      m_ctlItemTree.SetItemState(hti, 0, TVIS_DROPHILITED);
    }

    retval = m_ctlItemTree.SelectItem(pdi->tree_item);
    if (MakeVisible) {
      // Following needed to show selection when Find dbox has focus. Ugh.
      m_ctlItemTree.SetItemState(pdi->tree_item,
                                 TVIS_DROPHILITED | TVIS_SELECTED,
                                 TVIS_DROPHILITED | TVIS_SELECTED);
    }
    m_ctlItemTree.Invalidate();
  }
  return retval;
}

void DboxMain::SelectFirstEntry()
{
  if (m_core.GetNumEntries() > 0) {
    // Ensure an entry is selected after open
    CItemData *pci(NULL);
    if (m_ctlItemList.IsWindowVisible()) {
      m_ctlItemList.SetItemState(0,
                                 LVIS_FOCUSED | LVIS_SELECTED,
                                 LVIS_FOCUSED | LVIS_SELECTED);
      m_ctlItemList.EnsureVisible(0, FALSE);
      pci = (CItemData *)m_ctlItemList.GetItemData(0);
    } else {
      HTREEITEM hitem = m_ctlItemTree.GetFirstVisibleItem();
      if (hitem != NULL) {
        m_ctlItemTree.SelectItem(hitem);
        pci = (CItemData *)m_ctlItemTree.GetItemData(hitem);
      }
    }
    UpdateToolBarForSelectedItem(pci);

    SetDCAText(pci);
  }
}

BOOL DboxMain::SelectFindEntry(const int i, BOOL MakeVisible)
{
  BOOL retval;
  if (m_ctlItemList.GetItemCount() == 0)
    return FALSE;

  CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(i);
  ASSERT(pci != NULL);
  if (m_ctlItemList.IsWindowVisible()) {
    retval = m_ctlItemList.SetItemState(i,
                                        LVIS_FOCUSED | LVIS_SELECTED,
                                        LVIS_FOCUSED | LVIS_SELECTED);
    m_LastFoundListItem = i;
    if (MakeVisible) {
      m_ctlItemList.EnsureVisible(i, FALSE);
    }
    m_ctlItemList.Invalidate();
  } else { //Tree view active
    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
    ASSERT(pdi != NULL);
    ASSERT(pdi->list_index == i);

    UnFindItem();

    retval = m_ctlItemTree.SelectItem(pdi->tree_item);
    if (MakeVisible) {
      m_ctlItemTree.SetItemState(pdi->tree_item, TVIS_BOLD, TVIS_BOLD);
      m_LastFoundTreeItem = pdi->tree_item;
      m_bBoldItem = true;
    }
    m_ctlItemTree.Invalidate();
  }
  UpdateToolBarForSelectedItem(pci);
  return retval;
}

// Updates m_ctlItemList and m_ctlItemTree from m_pwlist
// updates of windows suspended until all data is in.
void DboxMain::RefreshViews(const int iView)
{
  if (!m_bInitDone)
    return;

#if defined(POCKET_PC)
  HCURSOR waitCursor = app.LoadStandardCursor(IDC_WAIT);
#endif

  if (m_core.GetNumEntries() == 0) {
    if (iView & iListOnly) {
      m_ctlItemList.SetRedraw(TRUE); 
      m_ctlItemList.Invalidate();
    }
    if (iView & iTreeOnly) {
      m_ctlItemTree.SetRedraw(TRUE);
      m_ctlItemTree.Invalidate();
    }
    return;
  }

  m_bNumPassedFiltering = 0;

  // Get current selected items and save ptr to the entries (unchanged over 
  // refresh of Tree/List
  SaveDisplayBeforeMinimize();

  // can't use LockWindowUpdate 'cause only one window at a time can be locked
  if (iView & iListOnly) {
    m_ctlItemList.SetRedraw(FALSE);
    m_ctlItemList.DeleteAllItems();
  }
  if (iView & iTreeOnly) {
    m_ctlItemTree.SetRedraw(FALSE);
    m_mapGroupToTreeItem.clear();
    m_ctlItemTree.DeleteAllItems();
  }
  m_bBoldItem = false;

  if (m_core.GetNumEntries() != 0) {
    ItemListIter listPos;
#if defined(POCKET_PC)
    SetCursor(waitCursor);
#endif
    for (listPos = m_core.GetEntryIter(); listPos != m_core.GetEntryEndIter();
         listPos++) {
      CItemData &pci = m_core.GetEntry(listPos);
      DisplayInfo *pdi = (DisplayInfo *)pci.GetDisplayInfo();
      if (pdi != NULL)
        pdi->list_index = -1; // easier, but less efficient, to delete pdi
      InsertItemIntoGUITreeList(pci, -1, false, iView);
    }

    m_ctlItemTree.SortTree(TVI_ROOT);
    SortListView();

#if defined(POCKET_PC)
    SetCursor(NULL);
#endif
  } // we have entries

  if (m_bImageInLV) {
    m_ctlItemList.SetColumnWidth(0, LVSCW_AUTOSIZE);
  }

  // re-enable and force redraw!
  if (iView & iListOnly) {
    m_ctlItemList.SetRedraw(TRUE); 
    m_ctlItemList.Invalidate();
  }
  if (iView & iTreeOnly) {
    m_ctlItemTree.SetRedraw(TRUE);
    m_ctlItemTree.Invalidate();
  }

  RestoreDisplayAfterMinimize();

  if (m_bFilterActive)
    UpdateStatusBar();
}

static void Shower(CWnd *pWnd)
{
  pWnd->ShowWindow(SW_SHOW);
}

void DboxMain::RestoreWindows()
{
  ShowWindow(SW_RESTORE);

  // Restore saved DB preferences that may not have been saved in the database
  // over the minimize/restore event.
  // Can't use the fact that the string is empty, as that is a valid state!
  // Use arbitrary value "#Empty#" to indicate nothing here.
  if (m_savedDBprefs != EMPTYSAVEDDBPREFS) {
    PWSprefs::GetInstance()->Load(m_savedDBprefs);
    if (m_core.HaveHeaderPreferencesChanged(m_savedDBprefs))
      m_core.SetDBPrefsChanged(true);
    m_savedDBprefs = EMPTYSAVEDDBPREFS;
  }

  RefreshViews();

  BringWindowToTop();
  CPWDialog::GetDialogTracker()->Apply(Shower);

  RestoreDisplayAfterMinimize();
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

void DboxMain::OnMove(int x, int y)
{
  CDialog::OnMove(x, y);
  // turns out that minimizing calls this
  // with x = y = -32000. Oh joy.
  if (m_bInitDone && IsWindowVisible() == TRUE &&
      x >= 0 && y >= 0) {
    WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
    GetWindowPlacement(&wp);
    PWSprefs::GetInstance()->SetPrefRect(wp.rcNormalPosition.top,
                                         wp.rcNormalPosition.bottom,
                                         wp.rcNormalPosition.left,
                                         wp.rcNormalPosition.right);
  }
}

void DboxMain::OnSize(UINT nType, int cx, int cy) 
{
  // Note that onsize runs before InitDialog (Gee, I love MFC)
  //  Also, OnSize is called AFTER the function has been peformed.
  //  To verify IF the function should be done at all, it must be checked in OnSysCommand.
  CDialog::OnSize(nType, cx, cy);

  // If m_bInitDone not true, then dialog has not yet been completely initialised
  if (!m_bInitDone) 
    return;

  if (nType != SIZE_MINIMIZED) {
    // Position the control bars - don't bother if just been minimized
    CRect rect, dragrect;
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
    bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
    if (bDragBarState) {
      const int i = GetSystemMetrics(SM_CYBORDER);
      m_DDGroup.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      const int j = rect.top + i;
      m_DDGroup.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

      m_DDTitle.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDTitle.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

      m_DDUser.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDUser.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

      m_DDPassword.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDPassword.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

      m_DDNotes.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDNotes.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

      m_DDURL.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDURL.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

      m_DDemail.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDemail.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      rect.top += dragrect.Height() + 2 * i;
    }
    m_ctlItemList.MoveWindow(&rect, TRUE);
    m_ctlItemTree.MoveWindow(&rect, TRUE);
  }

  PWSprefs *prefs = PWSprefs::GetInstance();

  // {kjp} Only SIZE_RESTORED is supported on Pocket PC.
#if !defined(POCKET_PC)
  switch (nType) {
    case SIZE_MINIMIZED:
      pws_os::Trace(L"OnSize:SIZE_MINIMIZED\n");

      // Called when minimize button select on main dialog control box
      // or the system menu or by right clicking in the Taskbar
      // AFTER THE WINDOW HAS BEEN MINIMIZED!!!

      // Save DB preferences that may not have been saved in the database
      // over the minimize/restore event.
      m_savedDBprefs = prefs->Store();

      // Suspend notification of changes
      m_core.SuspendOnDBNotification();

      // PWSprefs::DatabaseClear == Locked
      if (prefs->GetPref(PWSprefs::DatabaseClear)) {
        if (!LockDataBase()) {
          // Failed to save - abort minimize and clearing of data
          ShowWindow(SW_SHOW);
          return;
        }
      }

      m_ctlItemList.DeleteAllItems();
      m_mapGroupToTreeItem.clear();
      m_ctlItemTree.DeleteAllItems();
      m_bBoldItem = false;
      m_LastFoundTreeItem = NULL;
      m_LastFoundListItem = -1;

      if (prefs->GetPref(PWSprefs::ClearClipboardOnMinimize))
        OnClearClipboard();

      if (prefs->GetPref(PWSprefs::UseSystemTray)) {      
        app.SetMenuDefaultItem(ID_MENUITEM_RESTORE);
        ShowWindow(SW_HIDE);

        // User can have 'stealth' mode where, as long as a hot-key is defined,
        // they can hide the System Tray icon when PasswordSafe is minimized
        if (prefs->GetPref(PWSprefs::HideSystemTray) && 
            prefs->GetPref(PWSprefs::HotKeyEnabled) &&
            prefs->GetPref(PWSprefs::HotKey) > 0)
          app.HideIcon();
        else if (app.IsIconVisible() == FALSE)
          app.ShowIcon();
      }
      break;
    case SIZE_MAXIMIZED:
    case SIZE_RESTORED:
      if (!m_bSizing) { // here if actually restored
#endif
        if (nType == SIZE_MAXIMIZED)
          pws_os::Trace(L"OnSize:SIZE_MAXIMIZED\n");
        else
          pws_os::Trace(L"OnSize:SIZE_RESTORED\n");

        app.SetMenuDefaultItem(ID_MENUITEM_MINIMIZE);
        if (!RestoreWindowsData(false))
          return;

        m_bIsRestoring = true; // Stop 'sort of list view' hiding FindToolBar
        m_ctlItemTree.SetRestoreMode(true);
        RefreshViews();
        m_ctlItemTree.SetRestoreMode(false);
        m_bIsRestoring = false;

        // Restore saved DB preferences that may not have been saved in the database
        // over the minimize/restore event.
        // Can't use the fact that the string is empty, as that is a valid state!
        // Use arbitrary value "#Empty#" to indicate nothing here.
        if (m_savedDBprefs != EMPTYSAVEDDBPREFS) {
          prefs->Load(m_savedDBprefs);
          if (m_core.HaveHeaderPreferencesChanged(m_savedDBprefs))
            m_core.SetDBPrefsChanged(true);
          m_savedDBprefs = EMPTYSAVEDDBPREFS;
        }

        CPWDialog::GetDialogTracker()->Apply(Shower);

        RestoreDisplayAfterMinimize();

        if (prefs->GetPref(PWSprefs::UseSystemTray) && app.IsIconVisible() == FALSE) {      
          app.ShowIcon();
        }

        // Resume notification of changes
        m_core.ResumeOnDBNotification();
        if (m_FindToolBar.IsVisible())
          SetFindToolBar(true);
#if !defined(POCKET_PC)
      } else { // m_bSizing == true: here if size changed
        WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(&wp);
        PWSprefs::GetInstance()->SetPrefRect(wp.rcNormalPosition.top,
                                             wp.rcNormalPosition.bottom,
                                             wp.rcNormalPosition.left,
                                             wp.rcNormalPosition.right);

        // Make sure Find toolbar is above Status bar
        if (m_FindToolBar.IsVisible())
          SetToolBarPositions();
      }
      // Set timer for user-defined idle lockout, if selected (DB preference)
      KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
        ResetIdleLockCounter();
        SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
      }
      break;
    case SIZE_MAXHIDE:
      pws_os::Trace(L"OnSize:SIZE_MAXHIDE\n");
      break;
    case SIZE_MAXSHOW:
      pws_os::Trace(L"OnSize:SIZE_MAXSHOW\n");
      break;
  } // nType switch statement
#endif
  m_bSizing = false;
}

void DboxMain::OnMinimize()
{
  // Called when the System Tray Minimize menu option is used
  if (m_bStartHiddenAndMinimized)
    m_bStartHiddenAndMinimized = false;

  SaveDisplayBeforeMinimize();

  // Let OnSize handle this
  ShowWindow(SW_MINIMIZE);
}

void DboxMain::OnRestore()
{
  m_ctlItemTree.SetRestoreMode(true);

  // Called when the System Tray Restore menu option is used
  RestoreWindowsData(true);

  // No need for next statement as it is done via RestoreWindowsData(true)
  // calling RestoreWindows which issues ShowWindow(SW_RESTORE) which does it
  // in OnSize - convoluted logic - needs a re-write but oh so carefully!
  //RestoreDisplayAfterMinimize();

  m_ctlItemTree.SetRestoreMode(false);
}

void DboxMain::OnListItemSelected(NMHDR *pNMHDR, LRESULT *pLResult)
{
  OnItemSelected(pNMHDR, pLResult);
}

void DboxMain::OnTreeItemSelected(NMHDR *pNMHDR, LRESULT *pLResult)
{
  OnItemSelected(pNMHDR, pLResult);
}

void DboxMain::OnItemSelected(NMHDR *pNMHDR, LRESULT *pLResult)
{
  *pLResult = 0L;
  CItemData *pci(NULL);

  if (!m_IsListView) {
    // TreeView

    // Seems that under Vista with Windows Common Controls V6, it is ignoring
    // the single click on the button (+/-) of a node and only processing the 
    // double click, which generates a copy of whatever the user selected
    // for a double click (except that it invalid for a node!) and then does
    // the expand/collapse as appropriate.
    // This codes attempts to fix this.
    HTREEITEM hItem(NULL);

    UnFindItem();
    switch (pNMHDR->code) {
      case NM_CLICK:
      {
        // Mouseclick - Need to find the item clicked via HitTest
        TVHITTESTINFO htinfo = {0};
        CPoint local = ::GetMessagePos();
        m_ctlItemTree.ScreenToClient(&local);
        htinfo.pt = local;
        m_ctlItemTree.HitTest(&htinfo);
        hItem = htinfo.hItem;

        // Ignore any clicks not on an item (group or entry)
        if (hItem == NULL ||
            htinfo.flags & (TVHT_NOWHERE | TVHT_ONITEMRIGHT | 
                            TVHT_ABOVE | TVHT_BELOW | 
                            TVHT_TORIGHT | TVHT_TOLEFT))
            return;

        // If a group
        if (!m_ctlItemTree.IsLeaf(hItem)) {
          // If on indent or button
          if (htinfo.flags & (TVHT_ONITEMINDENT | TVHT_ONITEMBUTTON)) {
            m_ctlItemTree.Expand(htinfo.hItem, TVE_TOGGLE);
            *pLResult = 1L; // We have toggled the group
            return;
          }
        }
        break;
      }
      case TVN_SELCHANGED:
        // Keyboard - We are given the new selected entry
        hItem = ((NMTREEVIEW *)pNMHDR)->itemNew.hItem;
        break;
      default:
        // No idea how we got here!
        return;
    }    

    // Check it was on an item
    if (hItem != NULL && m_ctlItemTree.IsLeaf(hItem)) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
    }

    HTREEITEM hti = m_ctlItemTree.GetDropHilightItem();
    if (hti != NULL)
      m_ctlItemTree.SetItemState(hti, 0, TVIS_DROPHILITED);
  } else {
    // ListView

    int iItem(-1);
    switch (pNMHDR->code) {
      case NM_CLICK:
      {
        LPNMITEMACTIVATE pLVItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
        iItem = pLVItemActivate->iItem;
        break;
      }
      case LVN_KEYDOWN:
      {
        LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
        iItem = m_ctlItemList.GetNextItem(-1, LVNI_SELECTED);
        int nCount = m_ctlItemList.GetItemCount();
        if (pLVKeyDown->wVKey == VK_DOWN)
          iItem = (iItem + 1) % nCount;
        if (pLVKeyDown->wVKey == VK_UP)
          iItem = (iItem - 1 + nCount) % nCount;
        break;
      }
      default:
        // No idea how we got here!
        return;
    }
    if (iItem != -1) {
      // -1 if nothing selected, e.g., empty list
      pci = (CItemData *)m_ctlItemList.GetItemData(iItem);
    }
  }

  UpdateToolBarForSelectedItem(pci);
  SetDCAText(pci);

  m_LastFoundTreeItem = NULL;
  m_LastFoundListItem = -1;
}

void DboxMain::OnKeydownItemlist(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

  // TRUE = we have processed the key stroke - don't call anyone else
  *pResult = TRUE;

  switch (pLVKeyDown->wVKey) {
    case VK_DELETE:
      OnDelete();
      return;
    case VK_INSERT:
      OnAdd();
      return;
    case VK_ADD:
      if ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) {
        SetHeaderInfo();
        return;
      }
      break;
    default:
      break;    
  }

  // FALSE = call next in line to process event
  *pResult = FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// NOTE!
// itemData must be the actual item in the item list.  if the item is removed
// from the list, it must be removed from the display as well and vice versa.
// A pointer is associated with the item in the display that is used for
// sorting.
// The exception is deleted items (database not saved). They exist in memory
// but are not displayed unless the user specifically wants them via a filter.

// {kjp} We could use itemData.GetNotes(CString&) to reduce the number of
// {kjp} temporary objects created and copied.
//
int DboxMain::InsertItemIntoGUITreeList(CItemData &ci, int iIndex, 
                                const bool bSort, const int iView)
{
  DisplayInfo *pdi = (DisplayInfo *)ci.GetDisplayInfo();
  if (pdi != NULL && pdi->list_index != -1) {
    // true iff item already displayed
    return iIndex;
  }

  int iResult = iIndex;
  if (iResult < 0) {
    iResult = m_ctlItemList.GetItemCount();
  }

  if (pdi == NULL) {
    pdi = new DisplayInfo;
    ci.SetDisplayInfo(pdi);
  }

  if (iView & iListOnly)
    pdi->list_index = -1;
  if (iView & iTreeOnly)
    pdi->tree_item = NULL;

  if (m_bFilterActive) {
    if (!PassesFiltering(ci, m_currentfilter))
      return -1;
    m_bNumPassedFiltering++;
  }

  int nImage = GetEntryImage(ci);
  StringX group = ci.GetGroup();
  StringX title = ci.GetTitle();
  StringX username = ci.GetUser();
  // get only the first line for display
  StringX strNotes = ci.GetNotes();
  StringX::size_type iEOL = strNotes.find(L'\r');
  if (iEOL != StringX::npos) {
    StringX strTemp = strNotes.substr(0, iEOL);
    strNotes = strTemp;
  }
  StringX cs_fielddata;

  if (iView & iListOnly) {
    // Insert the first column data
    if (m_bImageInLV)
      cs_fielddata = L"";
    else
      cs_fielddata = ci.GetFieldValue((CItemData::FieldType)m_nColumnTypeByIndex[0]);

    iResult = m_ctlItemList.InsertItem(iResult, cs_fielddata.c_str());

    if (iResult < 0) {
      // TODO: issue error here...
      return iResult;
    }

    pdi->list_index = iResult;
    if (m_bImageInLV)
      SetEntryImage(iResult, nImage);
  }

  if (iView & iTreeOnly) {
    HTREEITEM ti;
    StringX treeDispString = (LPCWSTR)m_ctlItemTree.MakeTreeDisplayString(ci);
    // get path, create if necessary, add title as last node
    bool bAlreadyExists;
    ti = m_ctlItemTree.AddGroup(ci.GetGroup().c_str(), bAlreadyExists);
    if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree)) {
      ti = m_ctlItemTree.InsertItem(treeDispString.c_str(), ti, TVI_SORT);
      m_ctlItemTree.SetItemData(ti, (DWORD_PTR)&ci);
    } else {
      ti = m_ctlItemTree.InsertItem(treeDispString.c_str(), ti, TVI_LAST);
      m_ctlItemTree.SetItemData(ti, (DWORD_PTR)&ci);
      if (bSort)
        m_ctlItemTree.SortTree(m_ctlItemTree.GetParentItem(ti));
    }

    SetEntryImage(ti, nImage);

    ASSERT(ti != NULL);
    pdi->tree_item = ti;
  }

  if (iView & iListOnly) {
    // Set the data in the rest of the columns
    // First get the 1st line of the Notes field
    StringX sxnotes, line1(L"");
    sxnotes = ci.GetNotes();
    if (!sxnotes.empty()) {
      StringX::size_type end;
      const StringX delim = L"\r\n";
      end = sxnotes.find(delim, 0);
      line1 = sxnotes.substr(0, 
                      (end == StringX::npos) ? StringX::npos : end);

      // If more than one line, add '[>>>]' to end of this line
      // Note CHeaderStrl adds the normal ellipsis '...' (without square
      // brackets) if the text doesn't fit in the cell.  Use this to show
      // more lines rather than more text in the first line.
      if (end != StringX::npos)
        line1 += L"[>>>]";
    }

    for (int i = 1; i < m_nColumns; i++) {
      if ((CItemData::FieldType)m_nColumnTypeByIndex[i] == CItemData::NOTES)
        cs_fielddata = line1;
      else
        cs_fielddata = ci.GetFieldValue((CItemData::FieldType)m_nColumnTypeByIndex[i]);

      m_ctlItemList.SetItemText(iResult, i, cs_fielddata.c_str());
    }

    m_ctlItemList.SetItemData(iResult, (DWORD_PTR)&ci);
  }
  return iResult;
}

CItemData *DboxMain::getSelectedItem()
{
  CItemData *retval = NULL;
  if (m_ctlItemList.IsWindowVisible()) { // list view
    POSITION p = m_ctlItemList.GetFirstSelectedItemPosition();
    if (p) {
      int i = m_ctlItemList.GetNextSelectedItem(p);
      retval = (CItemData *)m_ctlItemList.GetItemData(i);
      ASSERT(retval != NULL);
      DisplayInfo *pdi = (DisplayInfo *)retval->GetDisplayInfo();
      ASSERT(pdi != NULL && pdi->list_index == i);
    }
  } else { // tree view; go from HTREEITEM to index
    HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
    if (ti != NULL) {
      retval = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (retval != NULL) {
        // leaf: do some sanity tests
        DisplayInfo *pdi = (DisplayInfo *)retval->GetDisplayInfo();
        ASSERT(pdi != NULL);
        if (pdi->tree_item != ti) {
          pws_os::Trace(L"DboxMain::getSelectedItem: fixing pdi->tree_item!\n");
          pdi->tree_item = ti;
        }
      }
    } // ti != NULL
  } // tree view
  return retval;
}

void DboxMain::ClearData(const bool clearMRE)
{
  m_core.ClearData();  // Clears DB & DB Preferences changed flags

  if (clearMRE)
    m_RUEList.ClearEntries();

  UpdateSystemTray(m_bOpen ? LOCKED : CLOSED);

  //Because GetText returns a copy, we cannot do anything about the names
  if (m_bInitDone) {
    // For long lists, this is painful, so we disable updates
    m_ctlItemList.LockWindowUpdate();
    m_ctlItemList.DeleteAllItems();
    m_ctlItemList.UnlockWindowUpdate();
    m_ctlItemTree.LockWindowUpdate();
    m_mapGroupToTreeItem.clear();
    m_ctlItemTree.DeleteAllItems();
    m_ctlItemTree.UnlockWindowUpdate();
    m_bBoldItem = false;
  }
  m_bDBNeedsReading = true;
}

void DboxMain::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  // Get column index to CItemData value
  int iIndex = pNMListView->iSubItem;
  int iTypeSortColumn = m_nColumnTypeByIndex[iIndex];

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  if (m_iTypeSortColumn == iTypeSortColumn) {
    m_bSortAscending = !m_bSortAscending;
    PWSprefs *prefs = PWSprefs::GetInstance();
    prefs->SetPref(PWSprefs::SortAscending, m_bSortAscending);
    if (!m_core.GetCurFile().empty() &&
        m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
      if (!m_core.IsReadOnly()) {
        const StringX prefString(prefs->Store());
        SetChanged(m_core.HaveHeaderPreferencesChanged(prefString) ? DBPrefs : ClearDBPrefs);
      }
      ChangeOkUpdate();
    }
  } else {
    // Turn off all previous sort arrrows
    // Note: not sure where, as user may have played with the columns!
    for (int i = 0; i < m_LVHdrCtrl.GetItemCount(); i++) {
      m_LVHdrCtrl.GetItem(i, &hdi);
      if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        m_LVHdrCtrl.SetItem(i, &hdi);
      }
    }

    m_iTypeSortColumn = iTypeSortColumn;
    m_bSortAscending = true;
  }
  SortListView();
  OnHideFindToolBar();

  *pResult = TRUE;
}

void DboxMain::SortListView()
{
  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  m_bSortAscending = PWSprefs::GetInstance()->GetPref(PWSprefs::SortAscending);
  m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
  FixListIndexes();

  const int iIndex = m_nColumnIndexByType[m_iTypeSortColumn];
  m_LVHdrCtrl.GetItem(iIndex, &hdi);
  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  m_LVHdrCtrl.SetItem(iIndex, &hdi);

  if (!m_bIsRestoring && m_FindToolBar.IsVisible())
    OnHideFindToolBar();
}

void DboxMain::OnHeaderRClick(NMHDR* /* pNMHDR */, LRESULT *pResult)
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
    MENUINFO minfo;
    SecureZeroMemory(&minfo, sizeof(minfo));
    minfo.cbSize = sizeof(minfo);
    minfo.fMask = MIM_MENUDATA;
    minfo.dwMenuData = IDR_POPCOLUMNS;
    menu.SetMenuInfo(&minfo);
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);
    if (m_pCC != NULL)
      pPopup->CheckMenuItem(ID_MENUITEM_COLUMNPICKER,
                            m_pCC->IsWindowVisible() ? MF_CHECKED : MF_UNCHECKED);
    else
      pPopup->CheckMenuItem(ID_MENUITEM_COLUMNPICKER, MF_UNCHECKED);

    pPopup->TrackPopupMenu(dwTrackPopupFlags, ptMousePos.x, ptMousePos.y, this);
  }
  *pResult = TRUE;
}

void DboxMain::OnHeaderBeginDrag(NMHDR* pNMHDR, LRESULT *pResult)
{
  // Called for HDN_BEGINDRAG which changes the column order when CC not visible
  // Stop drag of first column (image)

  NMHEADER *phdn = (NMHEADER *) pNMHDR;

  *pResult = (m_bImageInLV && phdn->iItem == 0) ? TRUE : FALSE;
}

void DboxMain::OnHeaderEndDrag(NMHDR* pNMHDR, LRESULT *pResult)
{
  // Called for HDN_ENDDRAG which changes the column order when CC not visible
  // Unfortunately the changes are only really done when this call returns,
  // hence the PostMessage to get the information later

  // Get control after operation is really complete
  NMHEADER *phdn = (NMHEADER *) pNMHDR;

  // Stop drag of first column (image)
  if (m_bImageInLV && 
      (phdn->iItem == 0 || 
       (((phdn->pitem->mask & HDI_ORDER) == HDI_ORDER) && 
        phdn->pitem->iOrder == 0))) {
    *pResult = TRUE;
    return;
  }

  // Otherwise allow
  PostMessage(PWS_MSG_HDR_DRAG_COMPLETE);
  *pResult = FALSE;
}

void DboxMain::OnHeaderNotify(NMHDR* pNMHDR, LRESULT *pResult)
{
  NMHEADER *phdn = (NMHEADER *) pNMHDR;
  *pResult = FALSE;

  if (m_nColumnWidthByIndex == NULL || phdn->pitem == NULL)
    return;

  UINT mask = phdn->pitem->mask;
  if ((mask & HDI_WIDTH) != HDI_WIDTH)
    return;

  // column width changed
  switch (phdn->hdr.code) {
    case HDN_ENDTRACK:
    case HDN_ITEMCHANGED:
      m_nColumnWidthByIndex[phdn->iItem] = phdn->pitem->cxy;
      break;
    default:
      break;
  }
}

void DboxMain::OnToggleView() 
{
  if (m_IsListView)
    OnTreeView();
  else
    OnListView();
}

void DboxMain::OnListView() 
{
  SetListView();
  if (m_FindToolBar.IsVisible())
    OnHideFindToolBar();
}

void DboxMain::OnTreeView() 
{
  SetTreeView();
  if (m_FindToolBar.IsVisible())
    OnHideFindToolBar();
}

void DboxMain::SetListView()
{
  UnFindItem();
  m_ctlItemTree.ShowWindow(SW_HIDE);
  m_ctlItemList.ShowWindow(SW_SHOW);
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView, L"list");
  m_ctlItemList.SetFocus();
  m_IsListView = true;
  // Some items may change on change of view
  UpdateMenuAndToolBar(m_bOpen);
}

void DboxMain::SetTreeView()
{
  UnFindItem();
  m_ctlItemList.ShowWindow(SW_HIDE);
  m_ctlItemTree.ShowWindow(SW_SHOW);
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView, L"tree");
  m_ctlItemTree.SetFocus();
  m_IsListView = false;
  // Some items may change on change of view
  UpdateMenuAndToolBar(m_bOpen);
}

void DboxMain::OnShowHideToolbar() 
{
  bool bState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowToolbar);
  
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowToolbar, !bState);
  m_MainToolBar.ShowWindow(bState ? SW_HIDE : SW_SHOW);
  SetToolBarPositions();
  UpdateToolBarROStatus(m_core.IsReadOnly());
}

void DboxMain::OnShowHideDragbar() 
{
  bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);

  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowDragbar, !bDragBarState);
  SetToolBarPositions();
}

void DboxMain::OnOldToolbar() 
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, false);
  SetToolbar(ID_MENUITEM_OLD_TOOLBAR);
  UpdateToolBarROStatus(m_core.IsReadOnly());
}

void DboxMain::OnNewToolbar() 
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, true);
  SetToolbar(ID_MENUITEM_NEW_TOOLBAR);
  UpdateToolBarROStatus(m_core.IsReadOnly());
}

void DboxMain::SetToolbar(const int menuItem, bool bInit)
{
  // Toolbar
  m_toolbarMode = menuItem;

  if (bInit) {
    // Dragbar
    if (menuItem == ID_MENUITEM_NEW_TOOLBAR) {
      m_DDGroup.Init(IDB_DRAGGROUP_NEW, IDB_DRAGGROUPX_NEW);
      m_DDTitle.Init(IDB_DRAGTITLE_NEW, IDB_DRAGTITLEX_NEW);
      m_DDUser.Init(IDB_DRAGUSER_NEW, IDB_DRAGUSERX_NEW);
      m_DDPassword.Init(IDB_DRAGPASSWORD_NEW, IDB_DRAGPASSWORDX_NEW);
      m_DDNotes.Init(IDB_DRAGNOTES_NEW, IDB_DRAGNOTESX_NEW);
      m_DDURL.Init(IDB_DRAGURL_NEW, IDB_DRAGURLX_NEW);
      m_DDemail.Init(IDB_DRAGEMAIL_NEW, IDB_DRAGEMAILX_NEW);
    } else if (menuItem == ID_MENUITEM_OLD_TOOLBAR) {
      m_DDGroup.Init(IDB_DRAGGROUP_CLASSIC, IDB_DRAGGROUPX_CLASSIC);
      m_DDTitle.Init(IDB_DRAGTITLE_CLASSIC, IDB_DRAGTITLEX_CLASSIC);
      m_DDUser.Init(IDB_DRAGUSER_CLASSIC, IDB_DRAGUSERX_CLASSIC);
      m_DDPassword.Init(IDB_DRAGPASSWORD_CLASSIC, IDB_DRAGPASSWORDX_CLASSIC);
      m_DDNotes.Init(IDB_DRAGNOTES_CLASSIC, IDB_DRAGNOTESX_CLASSIC);
      m_DDURL.Init(IDB_DRAGURL_CLASSIC, IDB_DRAGURLX_CLASSIC);
      m_DDemail.Init(IDB_DRAGEMAIL_CLASSIC, IDB_DRAGEMAILX_CLASSIC);
    } else {
      ASSERT(0);
    }
    m_MainToolBar.LoadDefaultToolBar(m_toolbarMode);
    m_FindToolBar.LoadDefaultToolBar(m_toolbarMode);
    CString csButtonNames = PWSprefs::GetInstance()->
      GetPref(PWSprefs::MainToolBarButtons).c_str();
    m_MainToolBar.CustomizeButtons(csButtonNames);
  } else { // !bInit - changing bitmaps
    m_MainToolBar.ChangeImages(m_toolbarMode);
    m_FindToolBar.ChangeImages(m_toolbarMode);
    if (menuItem == ID_MENUITEM_NEW_TOOLBAR) {
      m_DDGroup.ReInit(IDB_DRAGGROUP_NEW, IDB_DRAGGROUPX_NEW);
      m_DDTitle.ReInit(IDB_DRAGTITLE_NEW, IDB_DRAGTITLEX_NEW);
      m_DDUser.ReInit(IDB_DRAGUSER_NEW, IDB_DRAGUSERX_NEW);
      m_DDPassword.ReInit(IDB_DRAGPASSWORD_NEW, IDB_DRAGPASSWORDX_NEW);
      m_DDNotes.ReInit(IDB_DRAGNOTES_NEW, IDB_DRAGNOTESX_NEW);
      m_DDURL.ReInit(IDB_DRAGURL_NEW, IDB_DRAGURLX_NEW);
      m_DDemail.ReInit(IDB_DRAGEMAIL_NEW, IDB_DRAGEMAILX_NEW);
    } else if (menuItem == ID_MENUITEM_OLD_TOOLBAR) {
      m_DDGroup.ReInit(IDB_DRAGGROUP_CLASSIC, IDB_DRAGGROUPX_CLASSIC);
      m_DDTitle.ReInit(IDB_DRAGTITLE_CLASSIC, IDB_DRAGTITLEX_CLASSIC);
      m_DDUser.ReInit(IDB_DRAGUSER_CLASSIC, IDB_DRAGUSERX_CLASSIC);
      m_DDPassword.ReInit(IDB_DRAGPASSWORD_CLASSIC, IDB_DRAGPASSWORDX_CLASSIC);
      m_DDNotes.ReInit(IDB_DRAGNOTES_CLASSIC, IDB_DRAGNOTESX_CLASSIC);
      m_DDURL.ReInit(IDB_DRAGURL_CLASSIC, IDB_DRAGURLX_CLASSIC);
      m_DDemail.ReInit(IDB_DRAGEMAIL_CLASSIC, IDB_DRAGEMAILX_CLASSIC);
    } else {
      ASSERT(0);
    }
    m_DDGroup.Invalidate(); m_DDTitle.Invalidate(); m_DDUser.Invalidate();
    m_DDPassword.Invalidate(); m_DDNotes.Invalidate(); m_DDURL.Invalidate();
    m_DDemail.Invalidate();
  }
  m_menuManager.SetImageList(&m_MainToolBar);

  m_MainToolBar.Invalidate();
  m_FindToolBar.Invalidate();

  SetToolBarPositions();
}

void DboxMain::OnExpandAll()
{
  m_ctlItemTree.OnExpandAll();
}

void DboxMain::OnCollapseAll()
{
  m_ctlItemTree.OnCollapseAll();
}

static void Hider(CWnd *pWnd)
{
  pWnd->ShowWindow(SW_HIDE);
}

void DboxMain::OnTimer(UINT_PTR nIDEvent)
{
  if ((nIDEvent == TIMER_LOCKONWTSLOCK && IsWorkstationLocked()) ||
      (nIDEvent == TIMER_LOCKDBONIDLETIMEOUT &&
       DecrementAndTestIdleLockCounter())) {
    // OK, so we need to lock. If we're not using a system tray,
    // just minimize. If we are, then we need to hide (which
    // also requires children be hidden explicitly)
    pws_os::Trace(L"Locking due to Timer lock countdown or ws lock\n");
    m_vGroupDisplayState = GetGroupDisplayState();
    if (m_bOpen && app.GetSystemTrayState() == UNLOCKED && !IsIconic())
      SaveDisplayBeforeMinimize();

    if (!LockDataBase())
      return;

    // Save any database preference chnages
    PWSprefs *prefs = PWSprefs::GetInstance();
    m_savedDBprefs = prefs->Store();
    bool usingsystray = prefs->GetPref(PWSprefs::UseSystemTray);
    if (!usingsystray) {
      ShowWindow(SW_MINIMIZE);
    } else {
      CPWDialog::GetDialogTracker()->Apply(Hider);
      ShowWindow(SW_HIDE);
      app.SetMenuDefaultItem(ID_MENUITEM_RESTORE);
    }
    if (nIDEvent == TIMER_LOCKONWTSLOCK)
      KillTimer(TIMER_LOCKONWTSLOCK);
  } else {
    pws_os::Trace(L"Timer lock kicked in (countdown=%u), not locking. Timer ID=%d\n",
          m_IdleLockCountDown, nIDEvent);
  }
}

LRESULT DboxMain::OnSessionChange(WPARAM wParam, LPARAM )
{
  // Windows XP and later only
  // Handle Lock/Unlock, Fast User Switching and Remote access.
  // Won't be called if the registration failed (i.e. < Windows XP
  // or the "Windows Terminal Server" service wasn't active at startup).

  pws_os::Trace(L"OnSessionChange. wParam = %d\n", wParam);
  PWSprefs *prefs = PWSprefs::GetInstance();

  switch (wParam) {
    case WTS_CONSOLE_DISCONNECT:
    case WTS_REMOTE_DISCONNECT:
    case WTS_SESSION_LOCK:
      if (m_bOpen && app.GetSystemTrayState() == UNLOCKED) {
        m_bWSLocked = true;

        if (!IsIconic())
          SaveDisplayBeforeMinimize();

        if (prefs->GetPref(PWSprefs::LockOnWindowLock) &&
            LockDataBase()) {
          bool usingsystray = prefs->GetPref(PWSprefs::UseSystemTray);
          if (!usingsystray) {
            ShowWindow(SW_MINIMIZE);
          } else {
            CPWDialog::GetDialogTracker()->Apply(Hider);
            ShowWindow(SW_HIDE);
            app.SetMenuDefaultItem(ID_MENUITEM_RESTORE);
          }
        }
      }
      break;
    case WTS_CONSOLE_CONNECT:
    case WTS_REMOTE_CONNECT:
    case WTS_SESSION_UNLOCK:
    case WTS_SESSION_LOGON:
      m_bWSLocked = false;
      break;
    case WTS_SESSION_LOGOFF:
      // This does NOT get called as OnQueryEndSession/OnEndSession
      // handle this event - but just in case!
      SavePreferencesOnExit();
      SaveDatabaseOnExit(ST_WTSLOGOFFEXIT);
      CleanUpAndExit(false);
      break;
    case WTS_SESSION_REMOTE_CONTROL:
    default:
      break;
  }
  return 0L;
}

bool DboxMain::LockDataBase()
{
  /**
   * Since we clear the data, any unchanged changes will be lost,
   * so we force a save if database is modified, and fail
   * to lock if the save fails (unless db is r-o).
   *
   * returns false iff save was required AND failed.
   */

  // Now try and save changes
  if (m_core.IsChanged() ||  m_bTSUpdated) {
    if (Save() != PWScore::SUCCESS) {
      // If we don't warn the user, data may be lost!
      CGeneralMsgBox gmb;
      CString cs_text(MAKEINTRESOURCE(IDS_COULDNOTSAVE)), 
      cs_title(MAKEINTRESOURCE(IDS_SAVEERROR));
      gmb.MessageBox(cs_text, cs_title, MB_ICONSTOP);
      return false;
    }
  }

  // If there's a pending dialog box prompting for a
  // password, we need to kill it, since we will prompt
  // for the existing dbase's password upon restore.
  // Avoid lots of edge cases this way.
  CancelPendingPasswordDialog();
  ClearData(false);

  // Because LockDatabase actually doen't minimize the Window, the OnSize
  // routine is not called to clear the clipboard - so do it here
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ClearClipboardOnMinimize)) {
    ClearClipboardData();
  }
  return true;
}

// This function determines if the workstation is locked.
bool DboxMain::IsWorkstationLocked() const
{
  bool bResult = false;
  if (m_bWTSRegistered)
    bResult = m_bWSLocked;
  else {
    // Rather not use this as may have impact with multiple desktops
    // but if registering for session change messages failed ...
    HDESK hDesktop = OpenDesktop(L"default", 0, false, DESKTOP_SWITCHDESKTOP);
    if (hDesktop != 0) {
      // SwitchDesktop fails if hDesktop invisible, screensaver or winlogin.
      bResult = !SwitchDesktop(hDesktop);
      CloseDesktop(hDesktop);
    }
  }
  if (bResult)
    pws_os::Trace(L"IsWorkstationLocked() returning true");
  return bResult;
}

void DboxMain::OnChangeTreeFont() 
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  CFont *pOldFontTree;
  pOldFontTree = m_ctlItemTree.GetFont();

  // Make sure we know what is inside the font.
  LOGFONT lf;
  pOldFontTree->GetLogFont(&lf);

  // Present Tree/List view font and possibly change it
  // Allow user to apply changes to font
  StringX cs_TreeListSampleText = prefs->GetPref(PWSprefs::TreeListSampleText);

  CPWFontDialog fontdlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT, NULL, NULL, TLFONT);

  fontdlg.m_sampletext = cs_TreeListSampleText.c_str();

  if (fontdlg.DoModal() == IDOK) {
    m_pFontTree->DeleteObject();
    m_pFontTree->CreateFontIndirect(&lf);

    // Transfer the fonts to the tree and list windows
    m_ctlItemTree.SetUpFont(m_pFontTree);
    m_ctlItemList.SetUpFont(m_pFontTree);
    m_LVHdrCtrl.SetFont(m_pFontTree);

    // Recalculate header widths
    CalcHeaderWidths();
    // Reset column widths
    AutoResizeColumns();

    // Check if default
    CString csfn(lf.lfFaceName), csdfltfn(dfltTreeListFont.lfFaceName);
    if (lf.lfHeight == dfltTreeListFont.lfHeight && 
        lf.lfWeight == dfltTreeListFont.lfWeight &&
        lf.lfItalic == dfltTreeListFont.lfItalic && 
        csfn == csdfltfn) {
      // Delete config Tree/List font
      prefs->ResetPref(PWSprefs::TreeFont);
    } else { // Save user's choice of Tree/List font
      CString treefont_str;
      treefont_str.Format(L"%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s",
                          lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
                          lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut,
                          lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision,
                          lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
      prefs->SetPref(PWSprefs::TreeFont, LPCWSTR(treefont_str));
    }
    // Save user's sample text
    prefs->SetPref(PWSprefs::TreeListSampleText,
                   LPCWSTR(fontdlg.m_sampletext));
  } // DoModal
}

void DboxMain::OnChangePswdFont() 
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  LOGFONT lf;
  // Get Password font in case the user wants to change this.
  GetPasswordFont(&lf);

  // Present Password font and possibly change it
  // Allow user to apply changes to font
  StringX cs_PswdSampleText = prefs->GetPref(PWSprefs::PswdSampleText);

  CPWFontDialog fontdlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT, NULL, NULL, PWFONT);

  fontdlg.m_sampletext = cs_PswdSampleText.c_str();

  if (fontdlg.DoModal() == IDOK) {
    // Transfer the new font to the passwords
    SetPasswordFont(&lf);

    LOGFONT dfltfont;
    GetDefaultPasswordFont(dfltfont);

    // Check if default
    CString csfn(lf.lfFaceName), csdfltfn(dfltfont.lfFaceName);
    if (lf.lfHeight == dfltfont.lfHeight && 
        lf.lfWeight == dfltfont.lfWeight &&
        lf.lfItalic == dfltfont.lfItalic && 
        csfn == csdfltfn) {
      // Delete config password font
      prefs->ResetPref(PWSprefs::PasswordFont);
    } else { // Save user's choice of password font
      CString pswdfont_str;
      pswdfont_str.Format(L"%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s",
                          lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
                          lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut,
                          lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision,
                          lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
      prefs->SetPref(PWSprefs::PasswordFont, LPCWSTR(pswdfont_str));
    }
    // Save user's sample text
    prefs->SetPref(PWSprefs::PswdSampleText, LPCWSTR(fontdlg.m_sampletext));
  } // DoModal
}

void DboxMain::OnChangeVKFont() 
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  LOGFONT lf = {-16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0,
                L""};

  // Get VKeyboard font in case the user wants to change this.
  StringX cs_VKeyboardFont = prefs->GetPref(PWSprefs::VKeyboardFontName);
  if (cs_VKeyboardFont.length() != 0 && 
      cs_VKeyboardFont.length() <= LF_FACESIZE) {
    memcpy_s(lf.lfFaceName, LF_FACESIZE * sizeof(wchar_t),
             cs_VKeyboardFont.c_str(), cs_VKeyboardFont.length() * sizeof(wchar_t));
  }

  // Present VKeyboard font and possibly change it
  // Allow user to apply changes to font
  StringX cs_VKSampleText = prefs->GetPref(PWSprefs::VKSampleText);

  CPWFontDialog fontdlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | 
                             CF_LIMITSIZE | CF_NOSCRIPTSEL,
                             NULL, NULL, VKFONT);

  fontdlg.m_cf.nSizeMin = 12;
  fontdlg.m_cf.nSizeMax = 12;

  fontdlg.m_sampletext = cs_VKSampleText.c_str();

  INT_PTR rc = fontdlg.DoModal();
  if (rc == IDOK) {
    // Check if default
    CString csfn(lf.lfFaceName);
    if (csfn.IsEmpty()) {
      // Delete config VKeyboard font face name
      prefs->ResetPref(PWSprefs::VKeyboardFontName);
    } else {
      // Save user's choice of VKeyboard font face name
      // Remove leading @ (OpenType) if present
      if (csfn.Left(1) == L"@")
        csfn = csfn.Mid(1);
      if (csfn == CString(CVKeyBoardDlg::ARIALUMS))
        prefs->ResetPref(PWSprefs::VKeyboardFontName);
      else
        prefs->SetPref(PWSprefs::VKeyboardFontName, LPCWSTR(csfn));
    }

    // Save user's sample text
    prefs->SetPref(PWSprefs::VKSampleText, LPCWSTR(fontdlg.m_sampletext));
  } else
  if (fontdlg.m_bReset) {
    // Check if user reset it to none.
    prefs->ResetPref(PWSprefs::VKeyboardFontName);
  }
}

void DboxMain::UpdateSystemTray(const STATE s)
{
  switch (s) {
    case LOCKED:
      app.SetSystemTrayState(ThisMfcApp::LOCKED);
      if (!m_core.GetCurFile().empty()) {
        CString ttt(L"[");
        ttt += m_core.GetCurFile().c_str();
        ttt += L"]";
        app.SetTooltipText(ttt);
      }
      break;
    case UNLOCKED:
      app.SetSystemTrayState(ThisMfcApp::UNLOCKED);
      if (!m_core.GetCurFile().empty())
        app.SetTooltipText(m_core.GetCurFile().c_str());
      break;
    case CLOSED:
      app.SetSystemTrayState(ThisMfcApp::CLOSED);
        break;
    default:
    ASSERT(0);
  }
}

BOOL DboxMain::LaunchBrowser(const CString &csURL, const StringX &sxAutotype,
                             const std::vector<size_t> &vactionverboffsets,
                             const bool &bDoAutotype)
{
  CString theURL(csURL);

  // csURL should be a well-formed URL as defined in RFC3986 with
  // the exceptions listed below.
  // 
  // The default behaviour of this function is to pass the URL to the Windows shell.
  // This will invoke a the default browser for http: and https: schema, the default
  // mailer for mailto: schema, etc.
  //
  // The exceptions to the csURL syntax are as follows:
  // 0. csURL is "sanitized" - newlines, tabs & carriage returns are removed.
  // 1. If csURL doesn't contain "://" OR "mailto:" or [ssh], then we prepend
  //    "http://" to it. This is to allow users to enter "amazon.com" and get the
  //    behaviour the expect.
  // 2. If csURL contains [alt], then the rest of the text is passed to the "alternate"
  //    browser defined in the preferences, along with the command-line parameters.
  //    For example, if AltBrowser is "CoolBrowser.exe" and AltBrowserCmdLineParms
  //    is "-x", then the csURL "[alt] supersite.com" will cause the following to
  //    be invoked:
  //          CoolBrowser.exe -x http://supersite.com
  // 3. If csURL contains {alt} or [ssh], then the behaviour is the same as in (2),
  //    except that "http://" is NOT prepended to the rest of the text. This allows
  //    one to specify an ssh client (such as Putty) as the alternate browser,
  //    and user@machine as the URL.  However, this really should be actioned via
  //    the new Run Command!

  theURL.Remove(L'\r');
  theURL.Remove(L'\n');
  theURL.Remove(L'\t');

  bool isMailto = (theURL.Find(L"mailto:") != -1);
  UINT errID = isMailto ? IDS_CANTEMAIL : IDS_CANTBROWSE;

  int altReplacements = theURL.Replace(L"[alt]", L"");
  int alt2Replacements = (theURL.Replace(L"[ssh]", L"") +
                          theURL.Replace(L"{alt}", L""));
  int autotypeReplacements = theURL.Replace(L"[autotype]", L"");
  int no_autotype = theURL.Replace(L"[xa]", L"");

  if (alt2Replacements <= 0 && !isMailto && theURL.Find(L"://") == -1)
    theURL = L"http://" + theURL;

  StringX sxAltBrowser(PWSprefs::GetInstance()->
                       GetPref(PWSprefs::AltBrowser));
  bool useAltBrowser = ((altReplacements > 0 || alt2Replacements > 0) &&
                        !sxAltBrowser.empty());

  StringX sxFile, sxParameters(L"");
  if (!useAltBrowser) {
    sxFile = theURL;
  } else { // alternate browser specified, invoke w/optional args
    sxFile = sxAltBrowser;
    StringX sxCmdLineParms(PWSprefs::GetInstance()->
                           GetPref(PWSprefs::AltBrowserCmdLineParms));

    if (!sxCmdLineParms.empty())
      sxParameters = sxCmdLineParms + StringX(L" ") + StringX(theURL);
    else
      sxParameters = StringX(theURL);
  }

  TrimLeft(sxFile);
  // Obey user's No Autotype flag [xa]
  if (no_autotype > 0) {
    m_bDoAutoType = false;
    m_AutoType.clear();
    m_vactionverboffsets.clear();
  } else {
    // Either do it because they pressed the right menu/shortcut
    // or they had specified Do Auotype flag [autotype]
    m_bDoAutoType = bDoAutotype || autotypeReplacements > 0;
    m_AutoType = m_bDoAutoType ? sxAutotype : L"";
    if (m_bDoAutoType)
      m_vactionverboffsets = vactionverboffsets;
  }
  bool rc = m_runner.issuecmd(sxFile, sxParameters, !m_AutoType.empty());

  if (!rc) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(errID, MB_ICONSTOP);
  }
  return rc ? TRUE : FALSE;
}

BOOL DboxMain::SendEmail(const CString &cs_Email)
{
  /*
   * Format is the standard 'mailto:' rules as per RFC 2368.
   * 'mailto:' is prefixed the the string passed to this routine.
   *
   * sAddress[sHeaders]
   *
   * sAddress
   *  One or more valid email addresses separated by a semicolon. 
   *  You must use Internet-safe characters. Use %20 for the space character.
   *
   * sHeaders
   *  Optional. One or more name-value pairs. The first pair should be 
   *  prefixed by a "?" and any additional pairs should be prefixed by a "&".
   *
   *  The name can be one of the following strings:
   *    subject
   *       Text to appear in the subject line of the message.
   *    body
   *       Text to appear in the body of the message.
   *    CC
   *       Addresses to be included in the "cc" (carbon copy) section of the 
   *       message.
   *    BCC
   *       Addresses to be included in the "bcc" (blind carbon copy) section
   *       of the message.
   *
   * Example:
   *   user@example.com?subject=Message Title&body=Message Content"
   */

  StringX sx_Email(L"mailto:"), sxParameters(L"");
  sx_Email += cs_Email;
  Trim(sx_Email);
  bool rc = m_runner.issuecmd(sx_Email, sxParameters, false);

  if (!rc) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTEMAIL, MB_ICONSTOP);
  }
  return rc ? TRUE : FALSE;
}

void DboxMain::SetColumns()
{
  // User hasn't yet saved the columns he/she wants and so gets our order!
  // Or - user has reset the columns (popup menu from right click on Header)
  CString cs_header;
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;

  PWSprefs *prefs = PWSprefs::GetInstance();
  int ipwd = prefs->GetPref(PWSprefs::ShowPasswordInTree) ? 1 : 0;

  CRect rect;
  m_ctlItemList.GetClientRect(&rect);
  int i1stWidth = prefs->GetPref(PWSprefs::Column1Width,
                                 (rect.Width() / 3 + rect.Width() % 3));
  int i2ndWidth = prefs->GetPref(PWSprefs::Column2Width,
                                 rect.Width() / 3);
  int i3rdWidth = prefs->GetPref(PWSprefs::Column3Width,
                                 rect.Width() / 3);

  cs_header = GetHeaderText(CItemData::TITLE);
  m_ctlItemList.InsertColumn(0, cs_header);
  hdi.lParam = CItemData::TITLE;
  m_LVHdrCtrl.SetItem(0, &hdi);
  m_ctlItemList.SetColumnWidth(0, i1stWidth);

  cs_header = GetHeaderText(CItemData::USER);
  m_ctlItemList.InsertColumn(1, cs_header);
  hdi.lParam = CItemData::USER;
  m_LVHdrCtrl.SetItem(1, &hdi);
  m_ctlItemList.SetColumnWidth(1, i2ndWidth);

  cs_header = GetHeaderText(CItemData::NOTES);
  m_ctlItemList.InsertColumn(2, cs_header);
  hdi.lParam = CItemData::NOTES;
  m_LVHdrCtrl.SetItem(2, &hdi);
  m_ctlItemList.SetColumnWidth(2, i3rdWidth);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPasswordInTree)) {
    cs_header = GetHeaderText(CItemData::PASSWORD);
    m_ctlItemList.InsertColumn(3, cs_header);
    hdi.lParam = CItemData::PASSWORD;
    m_LVHdrCtrl.SetItem(3, &hdi);
    m_ctlItemList.SetColumnWidth(3,
                                 PWSprefs::GetInstance()->GetPref(PWSprefs::Column4Width,
                                 rect.Width() / 4));
  }

  int ioff = 3;
  CItemData::FieldType defCols[] = {CItemData::URL, CItemData::EMAIL,
                                    CItemData::RUNCMD, CItemData::CTIME,
                                    CItemData::PMTIME, CItemData::ATIME,
                                    CItemData::XTIME, CItemData::RMTIME,
                                    CItemData::POLICY,
  };
  for (int i = 0; i < sizeof(defCols)/sizeof(defCols[0]); i++) {
    cs_header = GetHeaderText(defCols[i]);
    m_ctlItemList.InsertColumn(ipwd + ioff, cs_header);
    hdi.lParam = defCols[i];
    m_LVHdrCtrl.SetItem(ipwd + ioff, &hdi);
    ioff++;
  }

  m_ctlItemList.SetRedraw(FALSE);

  for (int i = ipwd + 3; i < (ipwd + ioff); i++) {
    m_ctlItemList.SetColumnWidth(i, m_iDateTimeFieldWidth);
  }

  SetHeaderInfo();
}

void DboxMain::SetColumns(const CString cs_ListColumns)
{
  //  User has saved the columns he/she wants and now we are putting them back

  CString cs_header;
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;

  vector<int> vi_columns;
  vector<int>::const_iterator vi_IterColumns;
  const wchar_t pSep[] = L",";
  wchar_t *pTemp;

  // Duplicate as strtok modifies the string
  pTemp = _wcsdup((LPCWSTR)cs_ListColumns);

#if (_MSC_VER >= 1400)
  // Capture columns shown:
  wchar_t *next_token;
  wchar_t *token = wcstok_s(pTemp, pSep, &next_token);
  while(token) {
    vi_columns.push_back(_wtoi(token));
    token = wcstok_s(NULL, pSep, &next_token);
  }
#else
  // Capture columns shown:
  wchar_t *token = _wcstok(pTemp, pSep);
  while(token) {
    vi_columns.push_back(_wtoi(token));
    token = _wcstok(NULL, pSep);
  }
#endif
  free(pTemp);

  // If present, the images are always first
  int iType= *vi_columns.begin();
  if (iType == CItemData::UUID) {
    m_bImageInLV = true;
    m_ctlItemList.SetImageList(m_pImageList, LVSIL_NORMAL);
    m_ctlItemList.SetImageList(m_pImageList, LVSIL_SMALL);
  }

  int icol(0);
  for (vi_IterColumns = vi_columns.begin();
       vi_IterColumns != vi_columns.end();
       vi_IterColumns++) {
    iType = *vi_IterColumns;
    cs_header = GetHeaderText(iType);
    // Images (if present) must be the first column!
    if (iType == CItemData::UUID && icol != 0)
      continue;

    if (!cs_header.IsEmpty()) {
      m_ctlItemList.InsertColumn(icol, cs_header);
      hdi.lParam = iType;
      m_LVHdrCtrl.SetItem(icol, &hdi);
      icol++;
    }
  }

  SetHeaderInfo();
}

void DboxMain::SetColumnWidths(const CString cs_ListColumnsWidths)
{
  //  User has saved the columns he/she wants and now we are putting them back
  std::vector<int> vi_widths;
  std::vector<int>::const_iterator vi_IterWidths;
  const wchar_t pSep[] = L",";
  wchar_t *pWidths;

  // Duplicate as strtok modifies the string
  pWidths = _wcsdup((LPCWSTR)cs_ListColumnsWidths);

#if (_MSC_VER >= 1400)
  // Capture column widths shown:
  wchar_t *next_token;
  wchar_t *token = wcstok_s(pWidths, pSep, &next_token);
  while(token) {
    vi_widths.push_back(_wtoi(token));
    token = wcstok_s(NULL, pSep, &next_token);
  }
#else
  // Capture columnwidths shown:
  wchar_t *token = _wcstok(pWidths, pSep);
  while(token) {
    vi_widths.push_back(_wtoi(token));
    token = _wcstok(NULL, pSep);
  }
#endif
  free(pWidths);

  int icol = 0, index;

  for (vi_IterWidths = vi_widths.begin();
       vi_IterWidths != vi_widths.end();
       vi_IterWidths++) {
    if (icol == (m_nColumns - 1))
      break;
    int iWidth = *vi_IterWidths;
    m_ctlItemList.SetColumnWidth(icol, iWidth);
    index = m_LVHdrCtrl.OrderToIndex(icol);
    m_nColumnWidthByIndex[index] = iWidth;
    icol++;
  }

  // First column special if the Image
  if (m_bImageInLV) {
    m_ctlItemList.SetColumnWidth(0, LVSCW_AUTOSIZE);
  }
  // Last column special
  index = m_LVHdrCtrl.OrderToIndex(m_nColumns - 1);
  m_ctlItemList.SetColumnWidth(index, LVSCW_AUTOSIZE_USEHEADER);
}

void DboxMain::AddColumn(const int iType, const int iIndex)
{
  // Add new column of type iType after current column index iIndex
  CString cs_header;
  HDITEM hdi;
  int iNewIndex(iIndex);

  //  If iIndex = -1, means drop on the end
  if (iIndex < 0)
    iNewIndex = m_nColumns;

  hdi.mask = HDI_LPARAM | HDI_WIDTH;
  cs_header = GetHeaderText(iType);
  ASSERT(!cs_header.IsEmpty());
  iNewIndex = m_ctlItemList.InsertColumn(iNewIndex, cs_header);
  ASSERT(iNewIndex != -1);
  hdi.lParam = iType;
  hdi.cxy = GetHeaderWidth(iType);
  m_LVHdrCtrl.SetItem(iNewIndex, &hdi);
}

void DboxMain::DeleteColumn(const int iType)
{
  // Delete column
  m_ctlItemList.DeleteColumn(m_nColumnIndexByType[iType]);
}

void DboxMain::SetHeaderInfo()
{
  HDITEM hdi_get;
  // CHeaderCtrl get values
  hdi_get.mask = HDI_LPARAM | HDI_ORDER;

  m_nColumns = m_LVHdrCtrl.GetItemCount();
  ASSERT(m_nColumns > 1);  // Title & User are mandatory!

  // re-initialise array
  for (int i = 0; i < CItemData::LAST; i++)
    m_nColumnIndexByType[i] = 
    m_nColumnIndexByOrder[i] =
    m_nColumnTypeByIndex[i] =
    m_nColumnWidthByIndex[i] = -1;

  m_LVHdrCtrl.GetOrderArray(m_nColumnIndexByOrder, m_nColumns);

  for (int iOrder = 0; iOrder < m_nColumns; iOrder++) {
    const int iIndex = m_nColumnIndexByOrder[iOrder];
    m_ctlItemList.SetColumnWidth(iIndex, LVSCW_AUTOSIZE);
    m_LVHdrCtrl.GetItem(iIndex, &hdi_get);
    ASSERT(iOrder == hdi_get.iOrder);
    m_nColumnIndexByType[hdi_get.lParam] = iIndex;
    m_nColumnTypeByIndex[iIndex] = (int)hdi_get.lParam;
  }

  // Check sort column still there; if not TITLE always is!
  if (m_nColumnIndexByType[m_iTypeSortColumn] == -1)
    m_iTypeSortColumn = CItemData::TITLE;

  SortListView();
  AutoResizeColumns();
}

void DboxMain::OnResetColumns()
{
  // Delete all existing columns
  for (int i = 0; i < m_nColumns; i++) {
    m_ctlItemList.DeleteColumn(0);
  }

  if (m_bImageInLV) {
    m_bImageInLV = false;
    m_ctlItemList.SetImageList(NULL, LVSIL_NORMAL);
    m_ctlItemList.SetImageList(NULL, LVSIL_SMALL);
  }

  // re-initialise array
  for (int itype = 0; itype < CItemData::LAST; itype++)
    m_nColumnIndexByType[itype] = -1;

  // Set default columns
  SetColumns();

  // Reset the column widths
  AutoResizeColumns();

  // Refresh the ListView
  RefreshViews(iListOnly);

  // Reset Column Chooser dialog but only if already created
  if (m_pCC != NULL)
    SetupColumnChooser(false);
}

void DboxMain::AutoResizeColumns()
{
  int iIndex, iType;
  // CHeaderCtrl get values
  for (int iOrder = 0; iOrder < m_nColumns; iOrder++) {
    iIndex = m_nColumnIndexByOrder[iOrder];
    iType = m_nColumnTypeByIndex[iIndex];

    m_ctlItemList.SetColumnWidth(iIndex, LVSCW_AUTOSIZE);
    m_nColumnWidthByIndex[iIndex] = m_ctlItemList.GetColumnWidth(iIndex);

    if (m_nColumnWidthByIndex[iIndex] < m_nColumnHeaderWidthByType[iType]) {
      m_ctlItemList.SetColumnWidth(iIndex, m_nColumnHeaderWidthByType[iType]);
      m_nColumnWidthByIndex[iIndex] = m_nColumnHeaderWidthByType[iType];
    }
  }

  m_ctlItemList.UpdateWindow();

  // First column is special if an image
  if (m_bImageInLV) {
    m_ctlItemList.SetColumnWidth(0, LVSCW_AUTOSIZE);
  }
  // Last column is special
  iIndex = m_nColumnIndexByOrder[m_nColumns - 1];
  m_ctlItemList.SetColumnWidth(iIndex, LVSCW_AUTOSIZE);
  m_ctlItemList.SetColumnWidth(iIndex, LVSCW_AUTOSIZE_USEHEADER);
  m_nColumnWidthByIndex[iIndex] = m_ctlItemList.GetColumnWidth(iIndex);
}

void DboxMain::OnColumnPicker()
{
  SetupColumnChooser(true);
}

void DboxMain::SetupColumnChooser(const bool bShowHide)
{
  if (m_pCC == NULL) {
    m_pCC = new CColumnChooserDlg;
    BOOL ret = m_pCC->Create(IDD_COLUMNCHOOSER, this);
    if (!ret) {   //Create failed.
      m_pCC = NULL;
      return;
    }
    m_pCC->SetLVHdrCtrlPtr(&m_LVHdrCtrl);

    // Set extended style
    DWORD dw_style = m_pCC->m_ccListCtrl.GetExtendedStyle() | LVS_EX_ONECLICKACTIVATE;
    m_pCC->m_ccListCtrl.SetExtendedStyle(dw_style);

    // Make sure it doesn't appear obscure the header
    CRect HDRrect, CCrect;
    m_LVHdrCtrl.GetWindowRect(&HDRrect);
    m_pCC->GetWindowRect(&CCrect);
    // Note (0,0) is the top left of screen
    if (CCrect.top < HDRrect.bottom) {
      int x = CCrect.left;
      int y = HDRrect.bottom + 20;
      m_pCC->SetWindowPos(0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    // Insert column with "dummy" header
    m_pCC->m_ccListCtrl.InsertColumn(0, L"");
    m_pCC->m_ccListCtrl.SetColumnWidth(0, m_iheadermaxwidth);

    // Make it just wide enough to take the text
    CRect rect1, rect2;
    m_pCC->GetWindowRect(&rect1);
    m_pCC->m_ccListCtrl.GetWindowRect(&rect2);
    m_pCC->SetWindowPos(NULL, 0, 0, m_iheadermaxwidth + 18,
                        rect1.Height(), SWP_NOMOVE | SWP_NOZORDER);
    m_pCC->m_ccListCtrl.SetWindowPos(NULL, 0, 0, m_iheadermaxwidth + 6,
                                     rect2.Height(), SWP_NOMOVE | SWP_NOZORDER);
  }

  int i;
  CString cs_header;

  // Clear all current entries
  m_pCC->m_ccListCtrl.DeleteAllItems();

  // and repopulate
  int iItem;
  for (i = CItemData::LAST - 1; i >= 0; i--) {
    // Can't play with Title or User columns
    if (i == CItemData::TITLE || i == CItemData::USER)
      continue;

    if (m_nColumnIndexByType[i] == -1) {
      cs_header = GetHeaderText(i);
      if (!cs_header.IsEmpty()) {
        iItem = m_pCC->m_ccListCtrl.InsertItem(0, cs_header);
        m_pCC->m_ccListCtrl.SetItemData(iItem, (DWORD)i);
      }
    }
  }

  // If called by user right clicking on header, hide it or show it
  if (bShowHide)
    m_pCC->ShowWindow(m_pCC->IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

CString DboxMain::GetHeaderText(int iType) const
{
  CString cs_header;
  switch (iType) {
    case CItemData::UUID:
      cs_header.LoadString(IDS_ICON);
      break;
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
    case CItemData::URL:
      cs_header.LoadString(IDS_URL);
      break;
    case CItemData::AUTOTYPE:
      cs_header.LoadString(IDS_AUTOTYPE);
      break;
    case CItemData::EMAIL:
      cs_header.LoadString(IDS_EMAIL);
      break;
    case CItemData::RUNCMD:
      cs_header.LoadString(IDS_RUNCOMMAND);
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
    case CItemData::XTIME:
      cs_header.LoadString(IDS_PASSWORDEXPIRYDATE);
      break;
    case CItemData::XTIME_INT:
      cs_header.LoadString(IDS_PASSWORDEXPIRYDATEINT);
      break;
    case CItemData::RMTIME:
      cs_header.LoadString(IDS_LASTMODIFIED);
      break;
    case CItemData::POLICY:        
      cs_header.LoadString(IDS_PWPOLICY);
      break;
    default:
      cs_header.Empty();
  }
  return cs_header;
}

int DboxMain::GetHeaderWidth(int iType) const
{
  int nWidth(0);

  switch (iType) {
    case CItemData::UUID:
    case CItemData::GROUP:
    case CItemData::TITLE:
    case CItemData::USER:
    case CItemData::PASSWORD:
    case CItemData::NOTES:
    case CItemData::URL:
    case CItemData::EMAIL:
    case CItemData::RUNCMD:
    case CItemData::POLICY:
    case CItemData::XTIME_INT:
      nWidth = m_nColumnHeaderWidthByType[iType];
      break;
    case CItemData::CTIME:        
    case CItemData::PMTIME:
    case CItemData::ATIME:
    case CItemData::XTIME:
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
  wchar_t time_str[80], datetime_str[80];
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
  wchar_t szBuf[80];
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBuf, 80));
  GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, szBuf, datetime_str, 80);
  szBuf[0] = L' ';  // Put a blank between date and time
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, &szBuf[1], 79));
  GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, szBuf, time_str, 80);
#if (_MSC_VER >= 1400)
  wcscat_s(datetime_str, 80, time_str);
#else
  wcscat(datetime_str, 80, time_str);
#endif

  m_iDateTimeFieldWidth = m_ctlItemList.GetStringWidth(datetime_str) + 6;

  m_iheadermaxwidth = -1;
  CString cs_header;

  for (int iType = 0; iType < CItemData::LAST; iType++) {
    cs_header = GetHeaderText(iType);
    if (!cs_header.IsEmpty())
      m_nColumnHeaderWidthByType[iType] = m_ctlItemList.GetStringWidth(cs_header) + 20;
    else
      m_nColumnHeaderWidthByType[iType] = -4;
    m_iheadermaxwidth = max(m_iheadermaxwidth, m_nColumnHeaderWidthByType[iType]);
  } // for
}

void DboxMain::UnFindItem()
{
  // Entries found are made bold - remove it here.
  if (m_bBoldItem) {
    m_ctlItemTree.SetItemState(m_LastFoundTreeItem, 0, TVIS_BOLD);
    m_bBoldItem = false;
  }
}

static bool GetDriveAndDirectory(const StringX &cs_infile, CString &cs_drive,
                                 CString &cs_directory)
{
  std::wstring applicationpath = pws_os::getexecdir();
  std::wstring inpath = cs_infile.c_str();
  std::wstring appdrive, appdir;
  std::wstring drive, dir, file, ext;

  pws_os::splitpath(applicationpath, appdrive, appdir, file, ext);
  if (!pws_os::splitpath(inpath, drive, dir, file, ext)) {
    pws_os::IssueError(L"View Report: Error finding path to database");
    return false;
  }

  if (drive.empty())
    drive = appdrive;
  if (dir.empty())
    dir = appdir;
  cs_drive = drive.c_str();
  cs_directory = dir.c_str();
  return true;
}

void DboxMain::OnViewReports()
{
  CString cs_filename, cs_path, csAction;
  CString cs_directory, cs_drive;

  if (!GetDriveAndDirectory(m_core.GetCurFile(), cs_drive, cs_directory))
    return;

  CGeneralMsgBox gmb;
  CString cs_msg(MAKEINTRESOURCE(IDS_SELECTREPORT));
  gmb.SetMsg(cs_msg);

  struct _stat statbuf;
  bool bReportExists(false);

  int Reports[] = {
    IDS_RPTCOMPARE, IDS_RPTFIND, IDS_RPTIMPORTTEXT, IDS_RPTIMPORTXML,
    IDS_RPTMERGE, IDS_RPTSYNCH, IDS_RPTVALIDATE,
  };

  for (int i = 0; i < sizeof(Reports)/sizeof(Reports[0]); i++) {
    csAction.LoadString(Reports[i]);
    cs_filename.Format(IDSC_REPORTFILENAME, cs_drive, cs_directory, csAction);
    if (::_tstat(cs_filename, &statbuf) == 0) {
      gmb.AddButton(Reports[i], csAction);
      bReportExists = true;
    }
  }

  if (!bReportExists) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_NOREPORTSEXIST);
    return;
  }

  gmb.AddButton(IDCANCEL, IDS_CANCEL, TRUE, TRUE);
  gmb.SetStandardIcon(MB_ICONQUESTION);

  INT_PTR rc = gmb.DoModal();
  UINT uistring(0);
  switch (rc) {
    case IDS_RPTCOMPARE:
    case IDS_RPTFIND:
    case IDS_RPTIMPORTTEXT:
    case IDS_RPTIMPORTXML:
    case IDS_RPTMERGE:
    case IDS_RPTSYNCH:
    case IDS_RPTVALIDATE:
      uistring = rc;
      break;
    default:
      return;
  }
  csAction.LoadString(uistring);
  cs_filename.Format(IDSC_REPORTFILENAME, cs_drive, cs_directory, csAction);

  ViewReport(cs_filename);
  return;
}

void DboxMain::OnViewReports(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_REPORT_COMPARE) &&
    (nID <= ID_MENUITEM_REPORT_VALIDATE || nID == ID_MENUITEM_REPORT_SYNCHRONIZE));

  CString cs_filename, cs_path, csAction;
  CString cs_drive, cs_directory;

  if (!GetDriveAndDirectory(m_core.GetCurFile(), cs_drive, cs_directory))
    return;

  UINT uistring(0);
  switch (nID) {
    case ID_MENUITEM_REPORT_COMPARE:
      uistring = IDS_RPTCOMPARE;
      break;
    case ID_MENUITEM_REPORT_FIND:
      uistring = IDS_RPTFIND;
      break;
    case ID_MENUITEM_REPORT_IMPORTTEXT:
      uistring = IDS_RPTIMPORTTEXT;
      break;
    case ID_MENUITEM_REPORT_IMPORTXML:
      uistring = IDS_RPTIMPORTXML;
      break;
    case ID_MENUITEM_REPORT_MERGE:
      uistring = IDS_RPTMERGE;
      break;
    case ID_MENUITEM_REPORT_SYNCHRONIZE:
      uistring = IDS_RPTSYNCH;
      break;
    case ID_MENUITEM_REPORT_VALIDATE:
      uistring = IDS_RPTVALIDATE;
      break;
    default:
      ASSERT(0);
  }
  csAction.LoadString(uistring);
  cs_filename.Format(IDSC_REPORTFILENAME, cs_drive, cs_directory, csAction);

  ViewReport(cs_filename);
}

void DboxMain::ViewReport(CReport &rpt)
{
  CViewReport vr_dlg(this, &rpt);

  vr_dlg.DoModal();
}

void DboxMain::ViewReport(const CString &cs_ReportFileName)
{
  CString cs_drive, cs_directory;

  if (!GetDriveAndDirectory(LPCWSTR(cs_ReportFileName), cs_drive, cs_directory))
    return;

  CString cs_path = cs_drive + cs_directory;
  wchar_t szExecName[MAX_PATH + 1];

  // Find out the users default editor for "txt" files
  DWORD dwSize(MAX_PATH);
  HRESULT stat = ::AssocQueryString(0, ASSOCSTR_EXECUTABLE, L".txt", L"Open",
                                    szExecName, &dwSize);
  if (int(stat) != S_OK) {  
#ifdef _DEBUG
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(L"oops");
#endif
    return;
  }

  // Create an Edit process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  DWORD dwCreationFlags(0);
  dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;

  CString cs_CommandLine;

  // Make the command line = "<program>" "file" 
  cs_CommandLine.Format(L"\"%s\" \"%s\"", szExecName, cs_ReportFileName);
  int ilen = cs_CommandLine.GetLength();
  LPWSTR pszCommandLine = cs_CommandLine.GetBuffer(ilen);

  if (!CreateProcess(NULL, pszCommandLine, NULL, NULL, FALSE, dwCreationFlags, 
                     NULL, cs_path, &si, &pi)) {
    pws_os::Trace(L"CreateProcess failed (%d).\n", GetLastError());
  }

  // Close process and thread handles. 
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  cs_CommandLine.ReleaseBuffer();
}

int DboxMain::OnUpdateViewReports(const int nID)
{
  StringX cs_Database(m_core.GetCurFile());

  if (cs_Database.empty()) {
    return FALSE;
  }

  CString cs_filename, csAction;
  CString cs_drive, cs_directory;

  if (!GetDriveAndDirectory(cs_Database, cs_drive, cs_directory))
    return FALSE;

  UINT uistring(0);
  switch (nID) {
    case ID_MENUITEM_REPORT_COMPARE:
      uistring = IDS_RPTCOMPARE;
      break;
    case ID_MENUITEM_REPORT_FIND:
      uistring = IDS_RPTFIND;
      break;
    case ID_MENUITEM_REPORT_IMPORTTEXT:
      uistring = IDS_RPTIMPORTTEXT;
      break;
    case ID_MENUITEM_REPORT_IMPORTXML:
      uistring = IDS_RPTIMPORTXML;
      break;
    case ID_MENUITEM_REPORT_MERGE:
      uistring = IDS_RPTMERGE;
      break;
    case ID_MENUITEM_REPORT_SYNCHRONIZE:
      uistring = IDS_RPTSYNCH;
      break;
    case ID_MENUITEM_REPORT_VALIDATE:
      uistring = IDS_RPTVALIDATE;
      break;
    default:
      pws_os::Trace(L"ID=%d\n", nID);
      ASSERT(0);
  }

  csAction.LoadString(uistring);
  cs_filename.Format(IDSC_REPORTFILENAME, cs_drive, cs_directory, csAction);

  struct _stat statbuf;

  // Only allow selection if file exists!
  int status = ::_tstat(cs_filename, &statbuf);
  return (status != 0) ? FALSE : TRUE;
}

void DboxMain::OnRefreshWindow()
{
  // Useful for users if they are using a filter and have edited an entry
  // so it no longer passes
  RefreshViews();
}

void DboxMain::OnCustomizeToolbar()
{
  CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();
  mainTBCtrl.Customize();

  StringX cs_temp = LPCWSTR(m_MainToolBar.GetButtonString());
  PWSprefs::GetInstance()->SetPref(PWSprefs::MainToolBarButtons, cs_temp);

  CItemData *pci = getSelectedItem();
  UpdateToolBarForSelectedItem(pci);
}

void DboxMain::OnHideFindToolBar()
{
  SetFindToolBar(false);

  // Select the last found item on closing the FindToolbar (either by pressing
  // close or via Esc key if not used to minimize application).
  if (m_ctlItemList.IsWindowVisible() && m_LastFoundListItem != -1) {
    m_ctlItemList.SetFocus();
    m_ctlItemList.SetItemState(m_LastFoundListItem,
                               LVIS_FOCUSED | LVIS_SELECTED,
                               LVIS_FOCUSED | LVIS_SELECTED);
  } else
  if (m_ctlItemTree.IsWindowVisible() && m_LastFoundTreeItem != NULL) {
    m_ctlItemTree.SetFocus();
    m_ctlItemTree.Select(m_LastFoundTreeItem, TVGN_CARET);
  }
}

void DboxMain::SetFindToolBar(bool bShow)
{
  if (m_FindToolBar.GetSafeHwnd() == NULL)
    return;

  if (bShow)
    m_core.ResumeOnDBNotification();

  m_FindToolBar.ShowFindToolBar(bShow);
  SetToolBarPositions();
}

void DboxMain::SetToolBarPositions()
{
  if (m_FindToolBar.GetSafeHwnd() == NULL)
    return;

  CRect rect, dragrect;
  RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
  RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
  bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
  CDDStatic *DDs[] = { &m_DDGroup, &m_DDTitle, &m_DDUser,
                       &m_DDPassword, &m_DDNotes, &m_DDURL, &m_DDemail, };
  if (bDragBarState) {
    // Get the image states just incase another entry selected
    // since last shown
    CItemData *entry = GetLastSelected();
    if (entry == NULL) {
      m_DDGroup.SetStaticState(m_core.GetNumEntries() != 0);
      m_DDTitle.SetStaticState(false);
      m_DDPassword.SetStaticState(false);
      m_DDUser.SetStaticState(false);
      m_DDNotes.SetStaticState(false);
      m_DDURL.SetStaticState(false);
      m_DDemail.SetStaticState(false);
    } else {
      m_DDGroup.SetStaticState(!entry->IsGroupEmpty());
      m_DDTitle.SetStaticState(true);
      m_DDPassword.SetStaticState(true);
      m_DDUser.SetStaticState(!entry->IsUserEmpty());
      m_DDNotes.SetStaticState(!entry->IsNotesEmpty());
      m_DDURL.SetStaticState(!entry->IsURLEmpty());
      m_DDemail.SetStaticState(!entry->IsEmailEmpty());
    }

    const int i = GetSystemMetrics(SM_CYBORDER);

    for (int j = 0; j < sizeof(DDs)/sizeof(DDs[0]); j++) {
      DDs[j]->ShowWindow(SW_SHOW);
      DDs[j]->EnableWindow(TRUE);
      DDs[j]->GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      DDs[j]->SetWindowPos(NULL, dragrect.left, rect.top + i, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    rect.top += dragrect.Height() + 2 * i;
  } else { // !bDragBarState
    for (int j = 0; j < sizeof(DDs)/sizeof(DDs[0]); j++) {
      DDs[j]->ShowWindow(SW_HIDE);
      DDs[j]->EnableWindow(FALSE);
    }
  }
  m_ctlItemList.MoveWindow(&rect, TRUE);
  m_ctlItemTree.MoveWindow(&rect, TRUE);

  if (m_FindToolBar.IsVisible()) {
    // Is visible.  Try to get FindToolBar "above" the StatusBar!
    ASSERT(m_FindToolBar.GetParent() == m_statusBar.GetParent());

    CRect ftb_rect, stb_rect;
    m_FindToolBar.GetWindowRect(&ftb_rect);
    m_statusBar.GetWindowRect(&stb_rect);

    if (ftb_rect.top > stb_rect.top) {
      // FindToolBar is "below" the StatusBar
      ScreenToClient(&ftb_rect);
      ScreenToClient(&stb_rect);
      // Move FindToolBar up by the height of the Statusbar
      m_FindToolBar.MoveWindow(ftb_rect.left, ftb_rect.top - stb_rect.Height(),
        ftb_rect.Width(), ftb_rect.Height());
      // Move Statusbar down by the height of the FindToolBar
      m_statusBar.MoveWindow(stb_rect.left, stb_rect.top + ftb_rect.Height(),
        stb_rect.Width(), stb_rect.Height());
      m_FindToolBar.Invalidate();
      m_statusBar.Invalidate();
    }
  }
}

void DboxMain::OnToolBarClearFind()
{
  m_FindToolBar.ClearFind();
}

void DboxMain::OnToolBarFindCase()
{
  m_FindToolBar.ToggleToolBarFindCase();
}

void DboxMain::OnToolBarFindAdvanced()
{
  m_FindToolBar.ShowFindAdvanced();
}

void DboxMain::OnToolBarFindReport()
{
  std::vector<int> *pindices;
  CString csFindString;

  pindices = m_FindToolBar.GetSearchResults();
  m_FindToolBar.GetSearchText(csFindString);
  if (pindices == NULL || csFindString.IsEmpty())
    return;

  CString buffer, cs_temp;
  CReport rpt;
  cs_temp.LoadString(IDS_RPTFIND);
  rpt.StartReport(cs_temp, m_core.GetCurFile().c_str());

  CItemData::FieldBits bsFFields;
  BOOL bFAdvanced;
  CString Fsubgroup_name;
  int Fsubgroup_set, Fsubgroup_object, Fsubgroup_function;

  m_FindToolBar.GetSearchInfo(bFAdvanced, bsFFields, Fsubgroup_name, 
                              Fsubgroup_set, Fsubgroup_object, Fsubgroup_function);

  // tell the user we're done & provide short Compare report
  if (bFAdvanced == FALSE) {
    cs_temp.LoadString(IDS_NONE);
    buffer.Format(IDS_ADVANCEDOPTIONS, cs_temp);
    rpt.WriteLine((LPCWSTR)buffer);
    rpt.WriteLine();
  } else {
    if (Fsubgroup_set == BST_UNCHECKED) {
      cs_temp.LoadString(IDS_NONE);
    } else {
      CString cs_Object, cs_case, cs_text;
      UINT uistring(0);

      switch(Fsubgroup_object) {
        case CItemData::GROUP:
          uistring = IDS_GROUP;
          break;
        case CItemData::TITLE:
          uistring = IDS_TITLE;
          break;
        case CItemData::USER:
          uistring = IDS_USERNAME;
          break;
        case CItemData::GROUPTITLE:
          uistring = IDS_GROUPTITLE;
          break;
        case CItemData::URL:
          uistring = IDS_URL;
          break;
        case CItemData::EMAIL:
          uistring = IDS_EMAIL;
          break;
        case CItemData::RUNCMD:
          uistring = IDS_RUNCOMMAND;
          break;
        case CItemData::NOTES:
          uistring = IDS_NOTES;
          break;
        default:
          ASSERT(0);
      }
      cs_Object.LoadString(uistring);

      cs_case.LoadString(Fsubgroup_function > 0 ? 
                         IDS_ADVCASE_INSENSITIVE : IDS_ADVCASE_SENSITIVE);

      switch (Fsubgroup_function) {
        case -PWSMatch::MR_EQUALS:
        case  PWSMatch::MR_EQUALS:
          uistring = IDSC_EQUALS;
          break;
        case -PWSMatch::MR_NOTEQUAL:
        case  PWSMatch::MR_NOTEQUAL:
          uistring = IDSC_DOESNOTEQUAL;
          break;
        case -PWSMatch::MR_BEGINS:
        case  PWSMatch::MR_BEGINS:
          uistring = IDSC_BEGINSWITH;
          break;
        case -PWSMatch::MR_NOTBEGIN:
        case  PWSMatch::MR_NOTBEGIN:
          uistring = IDSC_DOESNOTBEGINSWITH;
          break;
        case -PWSMatch::MR_ENDS:
        case  PWSMatch::MR_ENDS:
          uistring = IDSC_ENDSWITH;
          break;
        case -PWSMatch::MR_NOTEND:
        case  PWSMatch::MR_NOTEND:
          uistring = IDSC_DOESNOTENDWITH;
          break;
        case -PWSMatch::MR_CONTAINS:
        case  PWSMatch::MR_CONTAINS:
          uistring = IDSC_CONTAINS;
          break;
        case -PWSMatch::MR_NOTCONTAIN:
        case  PWSMatch::MR_NOTCONTAIN:
          uistring = IDSC_DOESNOTCONTAIN;
          break;
        default:
          ASSERT(0);
      }
      cs_text.LoadString(uistring);
      cs_temp.Format(IDS_ADVANCEDSUBSET, cs_Object, cs_text, Fsubgroup_name,
                     cs_case);
    }
    buffer.Format(IDS_ADVANCEDOPTIONS, cs_temp);
    rpt.WriteLine((LPCWSTR)buffer);
    rpt.WriteLine();

    cs_temp.LoadString(IDS_RPTFIND);
    buffer.Format(IDS_ADVANCEDFIELDS, cs_temp);
    rpt.WriteLine((LPCWSTR)buffer);

    buffer = L"\t";
    // Non-time fields
    if (bsFFields.test(CItemData::GROUP))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_FINDGROUP));
    if (bsFFields.test(CItemData::TITLE))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_FINDTITLE));
    if (bsFFields.test(CItemData::USER))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_FINDUSER));
    if (bsFFields.test(CItemData::PASSWORD))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPPASSWORD));
    if (bsFFields.test(CItemData::NOTES))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPNOTES));
    if (bsFFields.test(CItemData::URL))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPURL));
    if (bsFFields.test(CItemData::EMAIL))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPEMAIL));
    if (bsFFields.test(CItemData::RUNCMD))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPRUNCOMMAND));
    if (bsFFields.test(CItemData::AUTOTYPE))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPAUTOTYPE));
    if (bsFFields.test(CItemData::PWHIST))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_COMPPWHISTORY));
    rpt.WriteLine((LPCWSTR)buffer);
    rpt.WriteLine();
  }

  if (pindices->size() == 0) {
    buffer.Format(IDS_SEARCHRESULTS1, csFindString);
    rpt.WriteLine((LPCWSTR)buffer);
  } else {
    buffer.Format(IDS_SEARCHRESULTS2, csFindString);
    rpt.WriteLine((LPCWSTR)buffer);
    int i, index;
    for (i = 0; i < (int)pindices->size(); i++) {
      index = pindices->at(i);
      CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(index);
      buffer.Format(IDS_COMPARESTATS, pci->GetGroup().c_str(),
                    pci->GetTitle().c_str(), pci->GetUser().c_str());
      rpt.WriteLine((LPCWSTR)buffer, false);
    }
  }
  rpt.WriteLine();
  rpt.EndReport();

  CGeneralMsgBox gmb;
  gmb.SetTitle(IDS_RPTFIND);
  gmb.SetMsg(IDS_REPORTCREATED);
  gmb.SetStandardIcon(MB_ICONINFORMATION);
  gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
  gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);
  INT_PTR msg_rc = gmb.DoModal();
  if (msg_rc == IDS_VIEWREPORT)
    ViewReport(rpt);

  m_FindToolBar.SetStatus(cs_temp);
}

int DboxMain::GetEntryImage(const CItemData &ci)
{
  int entrytype = ci.GetEntryType();
  if (entrytype == CItemData::ET_ALIAS) {
    return CPWTreeCtrl::ALIAS;
  }
  if (entrytype == CItemData::ET_SHORTCUT) {
    return CPWTreeCtrl::SHORTCUT;
  }

  int nImage;
  switch (entrytype) {
    case CItemData::ET_NORMAL:
      nImage = CPWTreeCtrl::NORMAL;
      break;
    case CItemData::ET_ALIASBASE:
      nImage = CPWTreeCtrl::ALIASBASE;
      break;
    case CItemData::ET_SHORTCUTBASE:
      nImage = CPWTreeCtrl::SHORTCUTBASE;
      break;
    default:
      nImage = CPWTreeCtrl::NORMAL;
  }

  time_t tXTime;
  ci.GetXTime(tXTime);
  if ((long)tXTime > 0L && (long)tXTime <= 3650L) {
    time_t tCPMTime;
    ci.GetPMTime(tCPMTime);
    if ((long)tCPMTime == 0L)
      ci.GetCTime(tCPMTime);
    tXTime = (time_t)((long)tCPMTime + (long)tXTime * 86400);
  }

  if (tXTime != 0) {
    time_t now, warnexptime((time_t)0);
    time(&now);
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn)) {
      int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
      struct tm st;
#if (_MSC_VER >= 1400)
      errno_t err;
      err = localtime_s(&st, &now);  // secure version
      ASSERT(err == 0);
#else
      st = *localtime(&now);
      ASSERT(st != NULL); // null means invalid time
#endif
      st.tm_mday += idays;
      warnexptime = mktime(&st);

      if (warnexptime == (time_t)-1)
        warnexptime = (time_t)0;
    }
    if (tXTime <= now) {
      nImage += 2;  // Expired
    } else if (tXTime < warnexptime) {
      nImage += 1;  // Warn nearly expired
    }
  }
  return nImage;
}

void DboxMain::SetEntryImage(const int &index, const int nImage, const bool bOneEntry)
{
  if (!m_bImageInLV)
    return;

  m_ctlItemList.SetItem(index, 0, LVIF_IMAGE, NULL, nImage, 0, 0, 0);

  if (bOneEntry) {
    CRect rect;
    m_ctlItemList.GetItemRect(index, &rect, LVIR_ICON);
    m_ctlItemList.InvalidateRect(&rect);
  }
}

void DboxMain::SetEntryImage(HTREEITEM &ti, const int nImage, const bool bOneEntry)
{
  m_ctlItemTree.SetItemImage(ti, nImage, nImage);

  if (bOneEntry) {
    CRect rect;
    m_ctlItemTree.GetItemRect(ti, &rect, FALSE);
    m_ctlItemTree.InvalidateRect(&rect);
  }
}

void DboxMain::UpdateEntryImages(const CItemData &ci)
{
  DisplayInfo *pdi = (DisplayInfo *)ci.GetDisplayInfo();
  if (ci.GetStatus() != CItemData::ES_DELETED) {
    int nImage = GetEntryImage(ci);
    SetEntryImage(pdi->list_index, nImage, true);
    SetEntryImage(pdi->tree_item, nImage, true);
  } else { // deleted item, remove from display
    m_ctlItemList.DeleteItem(pdi->list_index);
    m_ctlItemTree.DeleteItem(pdi->tree_item);
  }
}


void DboxMain::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
  if (lpdis == NULL || lpdis->CtlType != ODT_MENU) {
    // not for an ownerdrawn menu
    CDialog::OnDrawItem(nIDCtl, lpdis);
    return;
  }

  if (lpdis->rcItem.left != 2) {
    lpdis->rcItem.left -= (lpdis->rcItem.left - 2);
    lpdis->rcItem.right -= 60;
    if (lpdis->itemState & ODS_SELECTED) {
      lpdis->rcItem.left++;
      lpdis->rcItem.right++;
    }
  }

  CRUEItemData *pmd;
  pmd = (CRUEItemData *)lpdis->itemData;
  if (!pmd || !pmd->IsRUEID() || pmd->nImage < 0)
    return;

  HICON hIcon = GetEntryIcon(pmd->nImage);
  if (hIcon) {
    ICONINFO iconinfo;
    ::GetIconInfo(hIcon, &iconinfo);

    BITMAP bitmap;
    ::GetObject(iconinfo.hbmColor, sizeof(bitmap), &bitmap);

    ::DeleteObject(iconinfo.hbmColor);
    ::DeleteObject(iconinfo.hbmMask);

    ::DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, hIcon,
                 bitmap.bmWidth, bitmap.bmHeight, 
                 0, NULL, DI_IMAGE /*NORMAL*/);

    ::DeleteObject(hIcon);
  }
}

void DboxMain::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
  if (lpmis == NULL || lpmis->CtlType != ODT_MENU) {
    // not for an ownerdrawn menu
    CDialog::OnMeasureItem(nIDCtl, lpmis);
    return;
  }

  lpmis->itemWidth = 16;
  lpmis->itemHeight = 16;

  CRUEItemData *pmd;
  pmd = (CRUEItemData *)lpmis->itemData;
  if (!pmd || !pmd->IsRUEID() || pmd->nImage < 0)
    return;

  HICON hIcon = GetEntryIcon(pmd->nImage);
  if (hIcon) {
    ICONINFO iconinfo;
    ::GetIconInfo(hIcon, &iconinfo);

    BITMAP bitmap;
    ::GetObject(iconinfo.hbmColor, sizeof(bitmap), &bitmap);

    ::DeleteObject(iconinfo.hbmColor);
    ::DeleteObject(iconinfo.hbmMask);

    lpmis->itemWidth = bitmap.bmWidth;
    lpmis->itemHeight = bitmap.bmHeight;
  }
}

HICON DboxMain::GetEntryIcon(const int nImage) const
{
  int nID;
  switch (nImage) {
    case CPWTreeCtrl::NORMAL:
      nID = IDI_NORMAL;
      break;
    case CPWTreeCtrl::WARNEXPIRED_NORMAL:
      nID = IDI_NORMAL_WARNEXPIRED;
      break;
    case CPWTreeCtrl::EXPIRED_NORMAL:
      nID = IDI_NORMAL_EXPIRED;
      break;
    case CPWTreeCtrl::ALIASBASE:
      nID = IDI_ABASE;
      break;
    case CPWTreeCtrl::WARNEXPIRED_ALIASBASE:
      nID = IDI_ABASE_WARNEXPIRED;
      break;
    case CPWTreeCtrl::EXPIRED_ALIASBASE:
      nID = IDI_ABASE_EXPIRED;
      break;
    case CPWTreeCtrl::ALIAS:
      nID = IDI_ALIAS;
      break;
    case CPWTreeCtrl::SHORTCUTBASE:
      nID = IDI_SBASE;
      break;
    case CPWTreeCtrl::WARNEXPIRED_SHORTCUTBASE:
      nID = IDI_SBASE_WARNEXPIRED;
      break;
    case CPWTreeCtrl::EXPIRED_SHORTCUTBASE:
      nID = IDI_SBASE_EXPIRED;
      break;
    case CPWTreeCtrl::SHORTCUT:
      nID = IDI_SHORTCUT;
      break;
    default:
      nID = IDI_NORMAL;
  }
  HICON hIcon = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(nID), 
                                   IMAGE_ICON, 0, 0, 
                                   LR_LOADMAP3DCOLORS | LR_SHARED);
  return hIcon;
}

bool DboxMain::SetNotesWindow(const CPoint point, const bool bVisible)
{
  const CItemData *pci(NULL);
  CPoint target(point);
  StringX cs_notes(L"");
  UINT nFlags;
  HTREEITEM hItem(NULL);
  int nItem(-1);

  if (m_pNotesDisplay == NULL)
    return false;

  if (!bVisible) {
    m_pNotesDisplay->SetWindowText(cs_notes.c_str());
    m_pNotesDisplay->ShowWindow(SW_HIDE);
    return false;
  }

  if (m_ctlItemTree.IsWindowVisible()) {
    m_ctlItemTree.ClientToScreen(&target);
    hItem = m_ctlItemTree.HitTest(point, &nFlags);
    if (hItem != NULL &&
        (nFlags & (TVHT_ONITEM | TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT))) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
    }
  } else {
    m_ctlItemList.ClientToScreen(&target);
    nItem = m_ctlItemList.HitTest(point, &nFlags);
    if (nItem >= 0) {
      pci = (CItemData *)m_ctlItemList.GetItemData(nItem);
    }
  }
  target.y += ::GetSystemMetrics(SM_CYCURSOR); // height of cursor

  if (pci != NULL) {
    if (pci->IsShortcut())
      pci = GetBaseEntry(pci);
    cs_notes = pci->GetNotes();
  }

  if (!cs_notes.empty()) {
    Replace(cs_notes, StringX(L"\r\n"), StringX(L"\n"));
    Remove(cs_notes, L'\r');

    if (cs_notes.length() > 256)
      cs_notes = cs_notes.substr(0, 250) + L"[...]";
  }

  // move window
  CString cs_oldnotes;
  m_pNotesDisplay->GetWindowText(cs_oldnotes);
  if (LPCWSTR(cs_oldnotes) != cs_notes)
    m_pNotesDisplay->SetWindowText(cs_notes.c_str());

  m_pNotesDisplay->SetWindowPos(NULL, target.x, target.y, 0, 0,
                                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  m_pNotesDisplay->ShowWindow(!cs_notes.empty() ? SW_SHOWNA : SW_HIDE);

  return !cs_notes.empty();
}

CItemData *DboxMain::GetLastSelected()
{
  CItemData *retval(NULL);
  if (m_core.GetNumEntries() == 0)
    return retval;

  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hSelected = m_ctlItemTree.GetSelectedItem();
    if (hSelected != NULL)
      retval = (CItemData *)m_ctlItemTree.GetItemData(hSelected);
  } else {
    POSITION pSelected = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pSelected != NULL)
      retval = (CItemData *)m_ctlItemList.GetItemData((int)pSelected - 1);
  }
  return retval;
}

StringX DboxMain::GetGroupName()
{
  HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
  StringX s(L"");
  if (hi != NULL)
    s = m_ctlItemTree.GetItemText(hi);

  if ((GetKeyState(VK_CONTROL) & 0x8000) == 0) {
    while ((hi = m_ctlItemTree.GetParentItem(hi)) != NULL) {
      s = StringX(m_ctlItemTree.GetItemText(hi)) + StringX(L".") + s;
    }
  }
  return s;
}

void DboxMain::UpdateGroupNamesInMap(const StringX sxOldPath, const StringX sxNewPath)
{
  // When a group node is renamed, need to update the group to HTREEITEM map
  // We need to build a new map, as we can't erase&add while iterating.
  std::map<StringX, HTREEITEM> new_map;

  size_t len = sxOldPath.length();
  std::map<StringX, HTREEITEM>::iterator iter;

  for (iter = m_mapGroupToTreeItem.begin(); 
       iter != m_mapGroupToTreeItem.end(); iter++) {
    // Only change if path to root is the same
    if (iter->first.length() == len) {
      if (wcsncmp(sxOldPath.c_str(), iter->first.c_str(), len) == 0) {
        HTREEITEM ti = iter->second;
        new_map.insert(make_pair(sxNewPath, ti));
        continue;
      }
    } else if (iter->first.length() > len) {
      // Need to add group seperator to ensure not affecting another group
      StringX path = sxOldPath + StringX(GROUP_SEP2);
      if (wcsncmp(path.c_str(), iter->first.c_str(), len + 1) == 0) {
        HTREEITEM ti = iter->second;
        StringX sxNewGroup = sxNewPath + iter->first.substr(len);
        new_map.insert(make_pair(sxNewGroup, ti));
        continue;
      }
    }
    new_map.insert(*iter);
  } // for
  ASSERT(new_map.size() == m_mapGroupToTreeItem.size());
  m_mapGroupToTreeItem = new_map;
}

void DboxMain::OnShowUnsavedEntries()
{
  m_bUnsavedDisplayed = !m_bUnsavedDisplayed;

  // Check if it needs setting up the first time
  if (m_showunsavedfilter.num_Mactive < 3) {
    CString cs_temp(MAKEINTRESOURCE(IDS_NONSAVEDCHANGES));
    m_showunsavedfilter.fname = cs_temp;

    st_FilterRow fr;

    fr.bFilterComplete = true;
    fr.ftype = FT_ENTRYSTATUS;
    fr.mtype = PWSMatch::MT_ENTRYSTATUS;
    fr.rule = PWSMatch::MR_IS;
    fr.ltype = LC_OR;

    fr.estatus = CItemData::ES_ADDED;
    m_showunsavedfilter.vMfldata.push_back(fr);
    fr.estatus = CItemData::ES_MODIFIED;
    m_showunsavedfilter.vMfldata.push_back(fr);
    m_showunsavedfilter.num_Mactive = m_showunsavedfilter.vMfldata.size();
  }

  m_bFilterActive = !m_bFilterActive;
  if (m_bFilterActive)
    m_currentfilter = m_showunsavedfilter;
  else
    m_currentfilter.Empty();

  ApplyFilters();

  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER,
    (m_bUnsavedDisplayed || !m_bFilterActive) ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_EDITFILTER,
    m_bUnsavedDisplayed ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_MANAGEFILTERS,
    m_bUnsavedDisplayed ? FALSE : TRUE);
}

void DboxMain::UpdateToolBarDoUndo()
{
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_UNDO,
      (m_core.AnyToUndo()) ? TRUE : FALSE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_REDO,
      (m_core.AnyToRedo()) ? TRUE : FALSE);
}

void DboxMain::AddToGUI(CItemData &ci)
{
  uuid_array_t uuid;
  ci.GetUUID(uuid);
  int newpos = InsertItemIntoGUITreeList(m_core.GetEntry(m_core.Find(uuid)));

  if (newpos >= 0) {
    SelectEntry(newpos);
    FixListIndexes();
  }

  RefreshViews();
}

void DboxMain::RemoveFromGUI(CItemData &ci, bool bUpdateGUI)
{
  // RemoveFromGUI should always occur BEFORE the entry is deleted!
  uuid_array_t entry_uuid;
  ci.GetUUID(entry_uuid);
  ItemListIter iter = m_core.Find(entry_uuid);
  if (iter == End()) {
    ASSERT(0);
    return;
  }

  CItemData *pci2 = &iter->second;
  DisplayInfo *pdi2 = (DisplayInfo *)pci2->GetDisplayInfo();
  DisplayInfo *pdi = (DisplayInfo *)ci.GetDisplayInfo();

  if (pdi != NULL) {
    ASSERT(pdi2->list_index == pdi->list_index &&
           pdi2->tree_item == pdi->tree_item);
    if (bUpdateGUI) {
      HTREEITEM hItem = m_ctlItemTree.GetNextItem(pdi->tree_item,
                             TVGN_PREVIOUSVISIBLE);
      m_ctlItemTree.SelectItem(hItem);
    }

    m_ctlItemList.DeleteItem(pdi->list_index);
    m_ctlItemTree.DeleteWithParents(pdi->tree_item);

    FixListIndexes(); // sucks, as make m deletions an NxM operation
    if (bUpdateGUI) { // Make controls redraw
      m_ctlItemList.Invalidate();
      m_ctlItemTree.Invalidate();
    }
  }
}

void DboxMain::RefreshEntryPasswordInGUI(CItemData &ci)
{
  // For when Entry's password + PW history has been updated
  DisplayInfo *pdi = (DisplayInfo *)ci.GetDisplayInfo();

  UpdateListItem(pdi->list_index, CItemData::PWHIST, ci.GetPWHistory());
  RefreshEntryFieldInGUI(ci, CItemData::PASSWORD);
}

void DboxMain::RefreshEntryFieldInGUI(CItemData &ci, CItemData::FieldType ft)
{
  // For when any field is updated
  DisplayInfo *pdi = (DisplayInfo *)ci.GetDisplayInfo();

  UpdateListItem(pdi->list_index, ft, ci.GetFieldValue(ft));

  if (ft == CItemData::GROUP) {
    RefreshViews();
  } else {
    PWSprefs *prefs = PWSprefs::GetInstance();
    bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
    bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);
    if (ft == CItemData::START || ft == CItemData::TITLE || 
        (ft == CItemData::USER && bShowUsernameInTree) ||
        (ft == CItemData::PASSWORD && bShowPasswordInTree)) {
      StringX treeDispString = ci.GetTitle();

      if (bShowUsernameInTree)
        treeDispString += L" [" + ci.GetUser() + L"]";
      if (bShowPasswordInTree)
        treeDispString += L" {" + ci.GetPassword() + L"}";

      UpdateTreeItem(pdi->tree_item, treeDispString);
    }
  }
}

void DboxMain::RebuildGUI(const int iView)
{
  RefreshViews(iView);
}

void DboxMain::SaveDisplayBeforeMinimize()
{
  if (!m_bOpen || app.GetSystemTrayState() != UNLOCKED || IsIconic())
    return;

  if (m_core.GetNumEntries() == 0)
    return;

  if (m_ctlItemList.GetItemCount() == 0 || m_ctlItemTree.GetCount() == 0)
    return;

  pws_os::Trace(L"SaveDisplayBeforeMinimize\n");

  // Save expand/collapse status of groups
  m_vGroupDisplayState = GetGroupDisplayState();

  CItemData *pci(NULL);

  // Note: User can have different entries selected/visible in Tree & List Views
  memset(m_LUUIDSelectedAtMinimize, 0, sizeof(uuid_array_t));
  memset(m_TUUIDSelectedAtMinimize, 0, sizeof(uuid_array_t));
  memset(m_LUUIDVisibleAtMinimize, 0, sizeof(uuid_array_t));
  memset(m_TUUIDVisibleAtMinimize, 0, sizeof(uuid_array_t));

  m_sxSelectedGroup.clear();
  m_sxVisibleGroup.clear();

  // List view
  // Get selected entry in CListCtrl
  POSITION p = m_ctlItemList.GetFirstSelectedItemPosition();
  if (p) {
    int i = m_ctlItemList.GetNextSelectedItem(p);
    pci = (CItemData *)m_ctlItemList.GetItemData(i);
    ASSERT(pci != NULL);  // No groups in List View
    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
    ASSERT(pdi != NULL && pdi->list_index == i);
    pci->GetUUID(m_LUUIDSelectedAtMinimize);
  } // p != 0

  // Get first entry visible in CListCtrl
  int i = m_ctlItemList.GetTopIndex();
  if (i >= 0) {
    pci = (CItemData *)m_ctlItemList.GetItemData(i);
    ASSERT(pci != NULL);  // No groups in List View
    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
    ASSERT(pdi != NULL && pdi->list_index == i);
    pci->GetUUID(m_LUUIDVisibleAtMinimize);
  } // i >= 0

  // Tree view
  // Get selected entry in CTreeCtrl
  HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
  if (ti != NULL) {
    pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
    if (pci != NULL) {
      // Entry: do some sanity tests
      DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
      ASSERT(pdi != NULL);
      if (pdi->tree_item != ti) {
        pws_os::Trace(L"DboxMain::GetSelectedItems: fixing pdi->tree_item!\n");
        pdi->tree_item = ti;
      }
      pci->GetUUID(m_TUUIDSelectedAtMinimize);
    } else {
      // Group: save entry text
      m_sxSelectedGroup = m_ctlItemTree.GetGroup(ti);
    }
  } // ti != NULL

  // Get first entry visible in CTreeCtrl
  ti = m_ctlItemTree.GetFirstVisibleItem();
  if (ti != NULL) {
    pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
    if (pci != NULL) {
      // Entry: do some sanity tests
      DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
      ASSERT(pdi != NULL);
      if (pdi->tree_item != ti) {
        pws_os::Trace(L"DboxMain::GetSelectedItems: fixing pdi->tree_item!\n");
        pdi->tree_item = ti;
      }
      pci->GetUUID(m_TUUIDVisibleAtMinimize);
    } else {
      // Group: save entry text
      m_sxVisibleGroup = m_ctlItemTree.GetGroup(ti);
    }
  } // ti != NULL
}

void DboxMain::RestoreDisplayAfterMinimize()
{
  pws_os::Trace(L"RestoreDisplayAfterMinimize\n");

  if (m_core.GetNumEntries() == 0)
    return;

  if (m_ctlItemList.GetItemCount() == 0 || m_ctlItemTree.GetCount() == 0)
    return;

  // Restore expand/collapse status of groups
  m_bIsRestoring = true;
  m_ctlItemTree.SetRestoreMode(true);
  if (!m_vGroupDisplayState.empty()) {
    SetGroupDisplayState(m_vGroupDisplayState);
    m_vGroupDisplayState.clear();
  }
  m_ctlItemTree.SetRestoreMode(false);
  m_bIsRestoring = false;

  HTREEITEM htvis(NULL), htsel(NULL);
  CItemData *pci(NULL);

  // Process Tree - Selected
  if (memcmp(m_TUUIDSelectedAtMinimize, PWScore::NULL_UUID, sizeof(uuid_array_t)) != 0) {
    // Entry selected
    ItemListIter iter = Find(m_TUUIDSelectedAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = ((DisplayInfo *)(iter->second.GetDisplayInfo()));
      ASSERT(pdi != NULL);
      if (pdi != NULL) {
        htsel = pdi->tree_item;
        pci = &iter->second;
        //memset(m_TUUIDSelectedAtMinimize, 0, sizeof(uuid_array_t));
      }
    }
  } else {
    // Group selected
    if (!m_sxSelectedGroup.empty()) {
      // Find corresponding tree item
      std::map<StringX, HTREEITEM>::iterator iter;
      iter = m_mapGroupToTreeItem.find(m_sxSelectedGroup);
      if (iter != m_mapGroupToTreeItem.end()) {
        htsel = iter->second;
        //m_sxSelectedGroup.clear();
      }
    }
  }

  if (htsel != NULL) {
    m_ctlItemTree.Select(htsel, TVGN_CARET);
    RECT rect;
    m_ctlItemTree.GetItemRect(htsel, &rect, FALSE);
    m_ctlItemTree.InvalidateRect(&rect, TRUE);
    if (m_ctlItemTree.IsWindowVisible())
      UpdateToolBarForSelectedItem(pci);
  }

  // Process Tree - Visible
  if (memcmp(m_TUUIDVisibleAtMinimize, PWScore::NULL_UUID, sizeof(uuid_array_t)) != 0) {
    // Entry topmost visible
    ItemListIter iter = Find(m_TUUIDVisibleAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = ((DisplayInfo *)(iter->second.GetDisplayInfo()));
      ASSERT(pdi != NULL);
      if (pdi != NULL) {
        htvis = pdi->tree_item;
        //memset(m_TUUIDVisibleAtMinimize, 0, sizeof(uuid_array_t));
      }
    }
  } else {
    // Group topmost visible
    if (!m_sxVisibleGroup.empty()) {
      // Find corresponding tree item
      std::map<StringX, HTREEITEM>::iterator iter;
      iter = m_mapGroupToTreeItem.find(m_sxVisibleGroup);
      if (iter != m_mapGroupToTreeItem.end()) {
        htvis = iter->second;
        //m_sxVisibleGroup.clear();
      }
    }
  }

  // Just in case MFC actually selected the first visible entry
  if (htvis != NULL) {
    m_ctlItemTree.Select(htvis, TVGN_FIRSTVISIBLE);
    //m_ctlItemTree.SetItemState(htvis, 0, TVIS_SELECTED | TVIS_DROPHILITED);
    RECT rect;
    m_ctlItemTree.GetItemRect(htvis, &rect, FALSE);
    m_ctlItemTree.InvalidateRect(&rect, TRUE);
  }

  // Process List - selected
  if (memcmp(m_LUUIDSelectedAtMinimize, PWScore::NULL_UUID, sizeof(uuid_array_t)) != 0) {
    ItemListIter iter = Find(m_LUUIDSelectedAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = ((DisplayInfo *)(iter->second.GetDisplayInfo()));
      ASSERT(pdi != NULL);
      if (pdi != NULL) {
        // Select the Entry
        m_ctlItemList.SetItemState(pdi->list_index,
                                   LVIS_FOCUSED | LVIS_SELECTED,
                                   LVIS_FOCUSED | LVIS_SELECTED);
        m_ctlItemList.Update(pdi->list_index);
        //memset(m_LUUIDSelectedAtMinimize, 0, sizeof(uuid_array_t));
        if (m_ctlItemList.IsWindowVisible())
          UpdateToolBarForSelectedItem(&iter->second);
      }
    }
  }

  // Process List - visible
  if (memcmp(m_LUUIDVisibleAtMinimize, PWScore::NULL_UUID, sizeof(uuid_array_t)) != 0) {
    ItemListIter iter = Find(m_LUUIDVisibleAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = ((DisplayInfo *)(iter->second.GetDisplayInfo()));
      ASSERT(pdi != NULL);
      if (pdi != NULL) {
        // There is CListCtrl::GetTopIndex but No CListCtrl::SetTopIndex - Grrrrr!
        CRect indexRect, topRect;
        m_ctlItemList.EnsureVisible(pdi->list_index, FALSE);
        int icurtop = m_ctlItemList.GetTopIndex();
        m_ctlItemList.GetItemRect(pdi->list_index, indexRect, LVIR_BOUNDS);
        m_ctlItemList.GetItemRect(icurtop, topRect, LVIR_BOUNDS);
        m_ctlItemList.Scroll(CSize(0, (topRect.top - indexRect.top) * indexRect.Height()));

        // Just in case MFC actually selected the first visible entry
        m_ctlItemList.SetItemState(pdi->list_index, 0, LVIS_FOCUSED | LVIS_SELECTED);
        m_ctlItemList.Update(pdi->list_index);
        //memset(m_LUUIDVisibleAtMinimize, 0, sizeof(uuid_array_t));
      }
    }
  }
}

void DboxMain::SaveGroupDisplayState()
{
  vector <bool> v = GetGroupDisplayState(); // update it
  m_core.SetDisplayStatus(v); // store it
}

void DboxMain::RestoreGroupDisplayState()
{
  const vector<bool> &displaystatus = m_core.GetDisplayStatus();    

  if (!displaystatus.empty())
    SetGroupDisplayState(displaystatus);
}

vector<bool> DboxMain::GetGroupDisplayState()
{
  HTREEITEM hItem = NULL;
  vector<bool> v;

  if (m_ctlItemTree.GetSafeHwnd() == NULL)
    return v;

  while (NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem))) {
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      bool state = (m_ctlItemTree.GetItemState(hItem, TVIS_EXPANDED)
                    & TVIS_EXPANDED) != 0;
      v.push_back(state);
    }
  }
  return v;
}

void DboxMain::SetGroupDisplayState(const vector<bool> &displaystatus)
{
  // We need to copy displaystatus since Expand may cause
  // SaveGroupDisplayState to be called, updating it

  // Could be called from OnSize before anything set up!
  // Check Tree is valid first
  if (m_ctlItemTree.GetSafeHwnd() == NULL || displaystatus.empty())
    return;

  const vector<bool> dstatus(displaystatus);
  const size_t num = dstatus.size();
  if (num == 0)
    return;

  HTREEITEM hItem = NULL;
  size_t i(0);
  while (NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem))) {
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      m_ctlItemTree.Expand(hItem, dstatus[i] ? TVE_EXPAND : TVE_COLLAPSE);
      i++;
      if (i == num)
        break;
    }
  }
}

void DboxMain::SaveGUIStatus()
{
  st_SaveGUIInfo SaveGUIInfo;
  CItemData *pci_list(NULL), *pci_tree(NULL);

  // Note: we try and keep the same entry selected when the users
  // switches between List & Tree views.
  // But we can't if the user has selected a Group in the Tree view

  // Note: Must do this using the entry's UUID as the POSITION & 
  // HTREEITEM values may be different when we come to use it.
  POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
  if (pos != NULL) {
    pci_list = (CItemData *)m_ctlItemList.GetItemData((int)pos - 1);
    if (pci_list != NULL) {
      pci_list->GetUUID(SaveGUIInfo.lSelected);
      SaveGUIInfo.blSelectedValid = true;
    }
  }

  HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
  if (hi != NULL) {
    pci_tree = (CItemData *)m_ctlItemTree.GetItemData(hi);
    if (pci_tree != pci_list) {
      if (pci_tree != NULL) {
        pci_tree->GetUUID(SaveGUIInfo.tSelected);
        SaveGUIInfo.btSelectedValid = true;
      } else {
        StringX s(L"");
        if (hi != NULL)
          s = m_ctlItemTree.GetItemText(hi);

        while ((hi = m_ctlItemTree.GetParentItem(hi)) != NULL) {
          s = StringX(m_ctlItemTree.GetItemText(hi)) + StringX(L".") + s;
        }
        SaveGUIInfo.sxGroupName = s;
      }
    }
  }
  SaveGUIInfo.vGroupDisplayState = GetGroupDisplayState();

  m_stkSaveGUIInfo.push(SaveGUIInfo);
}

void DboxMain::RestoreGUIStatus()
{
  if (m_stkSaveGUIInfo.empty())
    return; // better safe than sorry...

  st_SaveGUIInfo &SaveGUIInfo = m_stkSaveGUIInfo.top();

  ItemListIter iter;
  DisplayInfo *pdi;
  if (SaveGUIInfo.blSelectedValid) {
    iter = Find(SaveGUIInfo.lSelected);
    pdi = (DisplayInfo *)iter->second.GetDisplayInfo();
    m_ctlItemList.SetItemState(pdi->list_index, LVIS_SELECTED, LVIS_SELECTED);
    m_ctlItemTree.SelectItem(pdi->tree_item);
  }

  if (SaveGUIInfo.btSelectedValid) {
    iter = Find(SaveGUIInfo.tSelected);
    pdi = (DisplayInfo *)iter->second.GetDisplayInfo();
    m_ctlItemTree.SelectItem(pdi->tree_item);
  }

  if (SaveGUIInfo.btGroupValid) {
    std::map<StringX, HTREEITEM>::iterator iter;
    iter = m_mapGroupToTreeItem.find(SaveGUIInfo.sxGroupName);
    if (iter != m_mapGroupToTreeItem.end()) {
      m_ctlItemTree.SelectItem(iter->second);
    }
  }

  SetGroupDisplayState(SaveGUIInfo.vGroupDisplayState);

  m_stkSaveGUIInfo.pop();
}
