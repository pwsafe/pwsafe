/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "ColumnChooserDlg.h"
#include "GeneralMsgBox.h"
#include "FontsDialog.h"
#include "Fonts.h"
#include "InfoDisplay.h"
#include "ViewReport.h"
#include "ExpPWListDlg.h"
#include "MenuShortcuts.h"
#include "HKModifiers.h"

#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "core/pwsprefs.h"
#include "core/core.h"
#include "core/PWHistory.h"
#include "core/StringXStream.h"

#include "os/Debug.h"
#include "os/dir.h"
#include "os/env.h"
#include "os/run.h"
#include "os/logit.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include "commctrl.h"
#include <shlwapi.h>
#include <vector>
#include <algorithm>
#include <sys/stat.h>

using namespace std;
using pws_os::CUUID;

extern const wchar_t GROUP_SEP;
extern const wchar_t *GROUP_SEP2;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DboxMain::DatabaseModified(bool bChanged)
{
  PWS_LOGIT_ARGS("bChanged=%s", bChanged ? L"true" : L"false");

  // Callback from PWScore if the database has been changed
  // (entries, preferences, header information,
  //  filters or password policies stored in the database)

  // First if the password list has been changed, invalidate
  // the indices vector in Find
  m_FindToolBar.InvalidateSearch();

  BOOL bFindBarShown = m_FindToolBar.IsWindowVisible();
  if (bFindBarShown)
    SetFindToolBar(false);

  // Save Immediately if user requested it
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately)) {
    if (SaveImmediately() == PWScore::SUCCESS)
      bChanged = false;
  }

  // Update menu/toolbar according to change state
  ChangeOkUpdate();

  if (bFindBarShown && !m_FindToolBar.IsWindowVisible())
    SetFindToolBar(true);

  // This is to prevent Windows (Vista & later) from shutting down
  // if the database has been modified (including preferences
  // stored in the DB)
  static bool bCurrentState(false);

  // Don't do anything if status unchanged or not at least Vista
  if (!pws_os::IsWindowsVistaOrGreater() ||
    m_core.IsReadOnly() || bChanged == bCurrentState)
    return;

  bCurrentState = bChanged;

  BlockLogoffShutdown(bChanged);
}

void DboxMain::BlockLogoffShutdown(const bool bChanged)
{
  // Only supported on Vista and later
  if (!pws_os::IsWindowsVistaOrGreater())
    return;

  if (bChanged) {
    if (m_pfcnShutdownBlockReasonCreate != NULL) {
      CString cs_stopreason;
      cs_stopreason.Format(IDS_STOPREASON, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
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
                         const CUUID &entry_uuid, CItemData::FieldType ft)
{
  // Callback from PWScore if GUI needs updating
  // Note: For some values of 'ga', 'entry_uuid' & ft are invalid and not used.
  CItemData *pci(NULL);

  ItemListIter pos = Find(entry_uuid);
  if (pos != End()) {
    pci = &pos->second;
  }

  switch (ga) {
    case UpdateGUICommand::GUI_UPDATE_STATUSBAR:
      if (app.GetBaseThreadID() == AfxGetThread()->m_nThreadID) {
        // Can't do UI from a worker thread - so check if it is the main thread!
        UpdateToolBarDoUndo();
        UpdateStatusBar();
      }
      break;
    case UpdateGUICommand::GUI_ADD_ENTRY:
      ASSERT(pci != NULL);
      AddToGUI(*pci);
      break;
    case UpdateGUICommand::GUI_DELETE_ENTRY:
      ASSERT(pci != NULL);
      RemoveFromGUI(*pci);
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRYFIELD:
      // Only used when group, title, username or password changed via in place
      // Edit in TreeView
      ASSERT(pci != NULL);
      RefreshEntryFieldInGUI(*pci, ft);
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD:
      ASSERT(pci != NULL);
      RefreshEntryPasswordInGUI(*pci);
      break;
    case UpdateGUICommand::GUI_REDO_IMPORT:
    case UpdateGUICommand::GUI_UNDO_IMPORT:
    case UpdateGUICommand::GUI_REDO_MERGESYNC:
    case UpdateGUICommand::GUI_UNDO_MERGESYNC:
      // During these processes, many entries may be added/removed
      // To stop the UI going nuts, updates to the UI are suspended until
      // the action is complete - when these calls are then sent
      RebuildGUI(BOTHVIEWS);
      break;
    case UpdateGUICommand::GUI_PWH_CHANGED_IN_DB:
      // During this process, many entries may have been edited (marked modified)
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::HighlightChanges))
        RebuildGUI(BOTHVIEWS);
      break;
    case UpdateGUICommand::GUI_REFRESH_TREE:
      // Rebuild only the tree view
      RebuildGUI(TREEONLY);
      break;
    case UpdateGUICommand::GUI_REFRESH_BOTHVIEWS:
      // Rebuild the entire tree & list views
      RebuildGUI(BOTHVIEWS);
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRY:
      // Refresh one entry ListView row and in the tree if the Title/Username/Password
      // has changed and visible in the tree when entry has been edited
      ASSERT(pci != NULL);
      UpdateEntryInGUI(*pci);
      break;
    case UpdateGUICommand::GUI_REFRESH_GROUPS:
      // Processed in the other overload of UpdateGUI
      ASSERT(0);
      break;
    case UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED:
      // Change any impact on the application due to a database preference change
      // Currently - only Idle Timeout values and potentially whether the
      // user/password is shown in the Tree view
      KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
      ResetIdleLockCounter();
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
        SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
      }
      RebuildGUI(BOTHVIEWS);
      break;
    default:
      break;
  }
}

void DboxMain::UpdateGUI(UpdateGUICommand::GUI_Action ga,
                         const std::vector<StringX> &vGroups)
{
  if (ga != UpdateGUICommand::GUI_REFRESH_GROUPS) {
    // Processed in the other overload of UpdateGUI
    ASSERT(0);
    return;
  }

  // Update these groups in the Tree display - saves doing a complete refresh!
  UpdateGroupsInGUI(vGroups);
}

void DboxMain::UpdateGUIDisplay()
{
  ChangeOkUpdate();
  RefreshViews();

  // May need to update menu/toolbar if original database was empty
  UpdateMenuAndToolBar(m_bOpen);
}

void DboxMain::GUIRefreshEntry(const CItemData &ci, bool bAllowFail)
{
  UpdateEntryImages(ci, bAllowFail);
}

void DboxMain::UpdateWizard(const std::wstring &s)
{
  if (m_pWZWnd != NULL)
    m_pWZWnd->SetWindowText(s.c_str());
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
  CItemData* pLHS_PCI = (CItemData *)lParam1;
  CItemData* pRHS_PCI = (CItemData *)lParam2;
  StringX group1, group2;
  time_t t1, t2;
  int xint1, xint2;

  int iResult(0);
  switch (nTypeSortColumn) {
    case CItemData::UUID:  // Image
      if (pLHS_PCI->GetEntryType() != pRHS_PCI->GetEntryType())
        iResult = (pLHS_PCI->GetEntryType() < pRHS_PCI->GetEntryType()) ? -1 : 1;
      break;
    case CItemData::GROUP:
      group1 = pLHS_PCI->GetGroup();
      group2 = pRHS_PCI->GetGroup();
      if (group1.empty())  // root?
        group1 = L"\xff";
      if (group2.empty())  // root?
        group2 = L"\xff";
      iResult = CompareNoCase(group1, group2);
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS_PCI->GetTitle(), pRHS_PCI->GetTitle());
        if (iResult == 0) {
          iResult = CompareNoCase(pLHS_PCI->GetUser(), pRHS_PCI->GetUser());
        }
      }
      break;
    case CItemData::TITLE:
      iResult = CompareNoCase(pLHS_PCI->GetTitle(), pRHS_PCI->GetTitle());
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS_PCI->GetUser(), pRHS_PCI->GetUser());
      }
      break;
    case CItemData::USER:
      iResult = CompareNoCase(pLHS_PCI->GetUser(), pRHS_PCI->GetUser());
      if (iResult == 0) {
        iResult = CompareNoCase(pLHS_PCI->GetTitle(), pRHS_PCI->GetTitle());
      }
      break;
    case CItemData::NOTES:
      iResult = CompareNoCase(pLHS_PCI->GetNotes(), pRHS_PCI->GetNotes());
      break;
    case CItemData::PASSWORD:
      iResult = CompareNoCase(pLHS_PCI->GetPassword(), pRHS_PCI->GetPassword());
      break;
    case CItemData::URL:
      iResult = CompareNoCase(pLHS_PCI->GetURL(), pRHS_PCI->GetURL());
      break;
    case CItemData::EMAIL:
      iResult = CompareNoCase(pLHS_PCI->GetEmail(), pRHS_PCI->GetEmail());
      break;
    case CItemData::SYMBOLS:
      iResult = CompareNoCase(pLHS_PCI->GetSymbols(), pRHS_PCI->GetSymbols());
      break;
    case CItemData::RUNCMD:
      iResult = CompareNoCase(pLHS_PCI->GetRunCommand(), pRHS_PCI->GetRunCommand());
      break;
    case CItemData::AUTOTYPE:
      iResult = CompareNoCase(pLHS_PCI->GetAutoType(), pRHS_PCI->GetAutoType());
      break;
    case CItemData::CTIME:
      pLHS_PCI->GetCTime(t1);
      pRHS_PCI->GetCTime(t2);
      if (t1 != t2)
        iResult = ((long)t1 < (long)t2) ? -1 : 1;
      break;
    case CItemData::PMTIME:
      pLHS_PCI->GetPMTime(t1);
      if (t1 == 0)
        pLHS_PCI->GetCTime(t1);
      pRHS_PCI->GetPMTime(t2);
      if (t2 == 0)
        pRHS_PCI->GetCTime(t2);
      if (t1 != t2)
        iResult = ((long)t1 < (long)t2) ? -1 : 1;
      break;
    case CItemData::ATIME:
      pLHS_PCI->GetATime(t1);
      pRHS_PCI->GetATime(t2);
      if (t1 != t2)
        iResult = ((long)t1 < (long)t2) ? -1 : 1;
      break;
    case CItemData::XTIME:
      pLHS_PCI->GetXTime(t1);
      pRHS_PCI->GetXTime(t2);
      if (t1 != t2)
        iResult = ((long)t1 < (long)t2) ? -1 : 1;
      break;
    case CItemData::XTIME_INT:
      pLHS_PCI->GetXTimeInt(xint1);
      pRHS_PCI->GetXTimeInt(xint2);
      if (xint1 != xint2)
        iResult = (xint1 < xint2) ? -1 : 1;
      break;
    case CItemData::RMTIME:
      pLHS_PCI->GetRMTime(t1);
      if (t1 == 0)
        pLHS_PCI->GetCTime(t1);
      pRHS_PCI->GetRMTime(t2);
      if (t2 == 0)
        pRHS_PCI->GetCTime(t2);
      if (t1 != t2)
        iResult = ((long)t1 < (long)t2) ? -1 : 1;
      break;
    case CItemData::POLICY:
      iResult = CompareNoCase(pLHS_PCI->GetPWPolicy(), pRHS_PCI->GetPWPolicy());
      break;
    case CItemData::POLICYNAME:
      iResult = CompareCase(pLHS_PCI->GetPolicyName(), pRHS_PCI->GetPolicyName());
      break;
    case CItemData::PROTECTED:
      if (pLHS_PCI->IsProtected() != pRHS_PCI->IsProtected())
        iResult = pLHS_PCI->IsProtected() ? 1 : -1;
      break;
    case CItemData::KBSHORTCUT:
      pLHS_PCI->GetKBShortcut(xint1);
      pRHS_PCI->GetKBShortcut(xint2);
      if (xint1 != xint2)
        iResult = (xint1 < xint2) ? -1 : 1;
      break;
    case CItemData::ATTREF:
      if (pLHS_PCI->HasAttRef() != pRHS_PCI->HasAttRef())
        iResult = pLHS_PCI->HasAttRef() ? 1 : -1;
      break;
    default:
      ASSERT(FALSE);
  }
  if (!self->m_bSortAscending && iResult != 0) {
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
  DDX_Control(pDX, IDC_STATIC_DRAGAUTO, m_DDAutotype);
  //}}AFX_DATA_MAP
}

void DboxMain::UpdateToolBarROStatus(const bool bIsRO)
{
  if (m_toolbarsSetup == TRUE) {
    BOOL State = bIsRO ? FALSE : TRUE;
    BOOL SaveState = (!bIsRO && m_core.HasDBChanged()) ? TRUE : FALSE;
    CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();

    // Allow or not Save
    mainTBCtrl.EnableButton(ID_MENUITEM_SAVE, SaveState);

    // Disable the following Toolbar buttons if R-O
    int IDs[] = { ID_MENUITEM_ADD , ID_MENUITEM_ADDGROUP,
      ID_MENUITEM_DELETEENTRY,
      ID_MENUITEM_IMPORT_PLAINTEXT, ID_MENUITEM_IMPORT_XML,
      ID_MENUITEM_MERGE, ID_MENUITEM_SYNCHRONIZE,
      ID_MENUITEM_UNDO, ID_MENUITEM_REDO };

    // Update state of Toolbar buttons
    for (int i = 0; i < sizeof(IDs) / sizeof(int); i++) {
      mainTBCtrl.EnableButton(IDs[i], State);
    }

    if (bIsRO) {
      // Edit should become View
      int nIndex = m_MainToolBar.CommandToIndex(ID_MENUITEM_EDITENTRY);
      if (nIndex != -1) {
        mainTBCtrl.SetCmdID(nIndex, ID_MENUITEM_VIEWENTRY);
      }
    } else {
      // View should become Edit
      int nIndex = m_MainToolBar.CommandToIndex(ID_MENUITEM_VIEWENTRY);
      if (nIndex != -1) {
        mainTBCtrl.SetCmdID(nIndex, ID_MENUITEM_EDITENTRY);
      }
    }
  }
}

void DboxMain::UpdateToolBarForSelectedItem(const CItemData *pci)
{
  const int IDs[] = {ID_MENUITEM_COPYPASSWORD, ID_MENUITEM_COPYUSERNAME,
                     ID_MENUITEM_COPYNOTESFLD, ID_MENUITEM_AUTOTYPE, 
                     ID_MENUITEM_RUNCOMMAND,   ID_MENUITEM_EDITENTRY,
                     ID_MENUITEM_PASSWORDSUBSET};

  // Following test required since this can be called on exit, with a pci
  // from ItemData that's already been deleted. Ugh.
  if (m_core.GetNumEntries() != 0) {
    const CItemData *pci_entry(pci), *pbci(NULL);
    BOOL State = (pci_entry == NULL) ? FALSE : TRUE; // Group vs Entry

    CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();
    for (int i = 0; i < _countof(IDs); i++) {
      mainTBCtrl.EnableButton(IDs[i], State);
    }

    mainTBCtrl.EnableButton(ID_MENUITEM_UNDO, m_core.AnyToUndo() ? TRUE : FALSE);
    mainTBCtrl.EnableButton(ID_MENUITEM_REDO, m_core.AnyToRedo() ? TRUE : FALSE);

    if (pci_entry != NULL) {
      if (m_core.IsReadOnly() || pci_entry->IsProtected()) {
        // Edit should become View
        int nIndex = m_MainToolBar.CommandToIndex(ID_MENUITEM_EDITENTRY);
        if (nIndex != -1) {
          mainTBCtrl.SetCmdID(nIndex, ID_MENUITEM_VIEWENTRY);
        }

        // But enabled
        mainTBCtrl.EnableButton(ID_MENUITEM_VIEWENTRY, TRUE);
      } else {
        // View should become Edit
        int nIndex = m_MainToolBar.CommandToIndex(ID_MENUITEM_VIEWENTRY);
        if (nIndex != -1) {
          mainTBCtrl.SetCmdID(nIndex, ID_MENUITEM_EDITENTRY);
        }

        // But enabled
        mainTBCtrl.EnableButton(ID_MENUITEM_EDITENTRY, TRUE);
      }
    } else {
      // Group selected - Edit should become View
      int nIndex = m_MainToolBar.CommandToIndex(ID_MENUITEM_EDITENTRY);
      if (nIndex != -1) {
        mainTBCtrl.SetCmdID(nIndex, ID_MENUITEM_VIEWENTRY);
      }

      // But disabled
      mainTBCtrl.EnableButton(ID_MENUITEM_VIEWENTRY, FALSE);
    }

    if (pci_entry != NULL && pci_entry->IsDependent()) {
      pbci = GetBaseEntry(pci_entry);
    }

    if (pci_entry == NULL || pci_entry->IsFieldValueEmpty(CItemData::USER, pbci)) {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYUSERNAME, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYUSERNAME, TRUE);
    }

    if (pci_entry == NULL || pci_entry->IsFieldValueEmpty(CItemData::NOTES, pbci)) {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYNOTESFLD, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_COPYNOTESFLD, TRUE);
    }

    if (pci_entry == NULL || pci_entry->IsFieldValueEmpty(CItemData::RUNCMD, pbci)) {
      mainTBCtrl.EnableButton(ID_MENUITEM_RUNCOMMAND, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_RUNCOMMAND, TRUE);
    }

    if (pci_entry == NULL || 
        (pci_entry->IsFieldValueEmpty(CItemData::EMAIL, pbci) && !pci_entry->IsURLEmail(pbci))) {
      mainTBCtrl.EnableButton(ID_MENUITEM_SENDEMAIL, FALSE);
    } else {
      mainTBCtrl.EnableButton(ID_MENUITEM_SENDEMAIL, TRUE);
    }

    if (pci_entry == NULL || pci_entry->IsFieldValueEmpty(CItemData::URL, pbci) ||
        pci_entry->IsURLEmail(pbci)) {
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
        m_DDAutotype.SetStaticState(false);
      } else {
        m_DDGroup.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::GROUP, pbci));
        m_DDTitle.SetStaticState(true);
        m_DDPassword.SetStaticState(true);
        m_DDUser.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::USER, pbci));
        m_DDNotes.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::NOTES, pbci));
        m_DDURL.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::URL, pbci));
        m_DDemail.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::EMAIL, pbci));
        m_DDAutotype.SetStaticState(true);
      }
    }
  } else {
    // NO entries - could be because last one was deleted
    CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();
    for (int i = 0; i < _countof(IDs); i++) {
      mainTBCtrl.EnableButton(IDs[i], FALSE);
    }

    mainTBCtrl.EnableButton(ID_MENUITEM_UNDO, m_core.AnyToUndo() ? TRUE : FALSE);
    mainTBCtrl.EnableButton(ID_MENUITEM_REDO, m_core.AnyToRedo() ? TRUE : FALSE);

    m_DDGroup.SetStaticState(false);
    m_DDTitle.SetStaticState(false);
    m_DDPassword.SetStaticState(false);
    m_DDUser.SetStaticState(false);
    m_DDNotes.SetStaticState(false);
    m_DDURL.SetStaticState(false);
    m_DDemail.SetStaticState(false);
    m_DDAutotype.SetStaticState(false);
  }
}

void DboxMain::setupBars()
{
  // This code is copied from the DLGCBR32 example that comes with MFC

  // Add the status bar
  if (m_StatusBar.Create(this)) {
    // Set up DoubleClickAction text - remove Shift+DCA
    const int dca = int(PWSprefs::GetInstance()->GetPref(PWSprefs::DoubleClickAction));
    switch (dca) {
      case PWSprefs::DoubleClickAutoType:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATAUTOTYPE;
        break;
      case PWSprefs::DoubleClickBrowse:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATBROWSE;
        break;
      case PWSprefs::DoubleClickCopyNotes:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATCOPYNOTES;
        break;
      case PWSprefs::DoubleClickCopyPassword:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATCOPYPASSWORD;
        break;
      case PWSprefs::DoubleClickCopyUsername:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATCOPYUSERNAME;
        break;
      case PWSprefs::DoubleClickViewEdit:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATVIEWEDIT;
        break;
      case PWSprefs::DoubleClickCopyPasswordMinimize:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATCOPYPASSWORDMIN;
        break;
      case PWSprefs::DoubleClickBrowsePlus:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATBROWSEPLUS;
        break;
      case PWSprefs::DoubleClickRun:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATRUN;
        break;
      case PWSprefs::DoubleClickSendEmail:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATSENDEMAIL;
        break;
      default:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDSC_STATCOMPANY;
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
    m_StatusBar.SetIndicators(statustext, CPWStatusBar::SB_TOTAL);

    UINT uiID, uiStyle;
    int cxWidth;
    m_StatusBar.GetPaneInfo(CPWStatusBar::SB_FILTER, uiID,
                            uiStyle, cxWidth);
    int iBMWidth = m_StatusBar.GetBitmapWidth();
    m_StatusBar.SetPaneInfo(CPWStatusBar::SB_FILTER, uiID,
                            uiStyle | SBT_OWNERDRAW, iBMWidth);

    // Make a sunken or recessed border around the first pane
    m_StatusBar.SetPaneInfo(CPWStatusBar::SB_DBLCLICK, 
                            m_StatusBar.GetItemID(CPWStatusBar::SB_DBLCLICK), 
                            SBPS_STRETCH, NULL);
  }

  CDC *pDC = this->GetDC();
  int NumBits = (pDC ? pDC->GetDeviceCaps(12 /*BITSPIXEL*/) : 32);
  m_MainToolBar.Init(NumBits);
  m_FindToolBar.Init(NumBits, PWS_MSG_TOOLBAR_FIND,
                     &m_SaveAdvValues[CAdvancedDlg::FIND]);
  ReleaseDC(pDC);

  // Add the Main ToolBar.
  if (!m_MainToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
                              WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE |
                              CBRS_TOP | CBRS_SIZE_DYNAMIC,
                              CRect(0, 0, 0, 0), AFX_IDW_CONTROLBAR_LAST)) {
    pws_os::Trace(L"Failed to create Main toolbar\n");
    return;      // fail to create
  }

  DWORD dwStyle = m_MainToolBar.GetBarStyle();
  dwStyle = dwStyle | CBRS_BORDER_BOTTOM | CBRS_BORDER_TOP   |
                      CBRS_BORDER_LEFT   | CBRS_BORDER_RIGHT |
                      CBRS_TOOLTIPS      | CBRS_FLYBY;
  m_MainToolBar.SetBarStyle(dwStyle);
  m_MainToolBar.SetWindowText(L"Standard");

  // Add the Find ToolBar.
  if (!m_FindToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
                              WS_CHILD    | WS_VISIBLE |
                              CBRS_BOTTOM | CBRS_SIZE_DYNAMIC,
                              CRect(0, 0, 0, 0), AFX_IDW_CONTROLBAR_LAST - 1)) {
    pws_os::Trace(L"Failed to create Find toolbar\n");
    return;      // fail to create
  }

  dwStyle = m_FindToolBar.GetBarStyle();
  dwStyle = dwStyle | CBRS_BORDER_BOTTOM | CBRS_BORDER_TOP |
                      CBRS_BORDER_LEFT   | CBRS_BORDER_RIGHT |
                      CBRS_TOOLTIPS      | CBRS_FLYBY;
  m_FindToolBar.SetBarStyle(dwStyle);
  m_FindToolBar.SetWindowText(L"Find");

  // Set Toolbar according to graphic capabilities, overridable by user choice.
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

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar)) {
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
    m_DDAutotype.EnableWindow(TRUE);
    m_DDAutotype.ShowWindow(SW_SHOW);
  }
}

void DboxMain::UpdateListItemField(const int lindex, const int type, const StringX &newText)
{
  int iSubItem = m_nColumnIndexByType[type];

  // Ignore if this column is not being displayed
  if (iSubItem < 0)
    return;

  VERIFY(m_ctlItemList.SetItemText(lindex, iSubItem, newText.c_str()));
  if (m_iTypeSortColumn == type) { // resort if necessary
    m_bSortAscending = PWSprefs::GetInstance()->GetPref(PWSprefs::SortAscending);
    m_ctlItemList.SortItems(CompareFunc, (LPARAM)this);
    FixListIndexes();
  }
}

void DboxMain::UpdateTreeItem(const HTREEITEM hItem, const CItemData &ci)
{
  CSecString csCurrentString = m_ctlItemTree.GetItemText(hItem);
  CSecString csNewString = m_ctlItemTree.MakeTreeDisplayString(ci);

  pws_os::Trace(L"UpdateTreeItem %08x - length: %d\n", hItem, csCurrentString.GetLength());
  if (csCurrentString != csNewString) {
    m_ctlItemTree.SetItemText(hItem, csNewString);
  }

  // Update view (e.g. no longer highlighted if not changed)
  CRect rect;
  m_ctlItemTree.GetItemRect(hItem, &rect, FALSE);
  m_ctlItemTree.InvalidateRect(&rect);
}

void DboxMain::UpdateEntryInGUI(CItemData &ci)
{
  DisplayInfo *pdi = GetEntryGUIInfo(ci);
  ASSERT(pdi != nullptr);
  if (pdi == nullptr) // BR1435 - not root cause, but at least don't crash...
    return;

  const int iIndex = pdi->list_index;
  const HTREEITEM hItem =  pdi->tree_item;

  const int nImage = GetEntryImage(ci);
  StringX sx_fielddata(L""), sx_oldfielddata;

  // Deal with Tree View
  UpdateTreeItem(hItem, ci);

  // Deal with List View
  bool bSortedFieldChanged(false);
  int iSortColumn = m_nColumnIndexByType[m_iTypeSortColumn];

  // Change the first column data - it is empty (as already set)
  if (!m_bImageInLV) {
    sx_fielddata = GetListViewItemText(ci, 0);
  }

  sx_oldfielddata = m_ctlItemList.GetItemText(iIndex, 0);
  if (sx_oldfielddata != sx_fielddata) {
    m_ctlItemList.SetItemText(iIndex, 0, sx_fielddata.c_str());
    if (iSortColumn == 0)
      bSortedFieldChanged = true;
  }

  if (m_bImageInLV)
    SetEntryImage(iIndex, nImage);

  // Change the data in the rest of the columns
  // First get the 1st line of the Notes field
  StringX sxNotes, line1(L"");
  sxNotes = ci.GetNotes();
  if (!sxNotes.empty()) {
    StringX::size_type end;
    const StringX delim = L"\r\n";
    end = sxNotes.find(delim, 0);
    line1 = sxNotes.substr(0, 
                    (end == StringX::npos) ? StringX::npos : end);

    // If more than one line, add '[>>>]' to end of this line
    // Note CHeaderStrl adds the normal ellipsis '...' (without square
    // brackets) if the text doesn't fit in the cell.  Use this to show
    // more lines rather than more text in the first line.
    if (end != StringX::npos)
      line1 += L"[>>>]";
  }

  for (int i = 1; i < m_nColumns; i++) {
    sx_fielddata = GetListViewItemText(ci, i);
    sx_oldfielddata = m_ctlItemList.GetItemText(iIndex, i);
    if (sx_oldfielddata != sx_fielddata) {
      m_ctlItemList.SetItemText(iIndex, i, sx_fielddata.c_str());
      if (iSortColumn == i)
        bSortedFieldChanged = true;
    }
  }

  // Unfortunately can't just update this one entry as it may have moved
  // if the field corresponding to the sort column has changed.
  // If sorted column field unchanged, just update this one entry.
  // If sorted column field changed, need to refresh the whole List view
  if (!bSortedFieldChanged) {
    m_ctlItemList.Update(iIndex);
  }  else {
    // Unselect current entry
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    while (pos) {
      int i = m_ctlItemList.GetNextSelectedItem(pos);
      m_ctlItemList.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
    }

    // Would like to just sort the list but this seems to leave both the old and the
    // new list entry visible on Undo until Refresh(F5) performed.  So refresh list
    RefreshViews(LISTONLY);

    // The iIndex might have changed as the edit could have changed the position
    // in the list depending on what entry field has changed
    for (int iItem = 0; iItem < m_ctlItemList.GetItemCount(); iItem++) {
      CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(iItem);
      if (ci.GetUUID() == pci->GetUUID()) {
        // Now reselect it and make visible
        m_ctlItemList.SetItemState(iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
        m_ctlItemList.EnsureVisible(iItem, false);
        break;
      }
    }
  }
}

void DboxMain::UpdateGroupsInGUI(const std::vector<StringX> &vGroups)
{
  // Update these group in the Tree display - saves doing a complete refresh!
  std::map<StringX, HTREEITEM>::iterator it;

  for (auto iter_group = vGroups.begin(); iter_group != vGroups.end(); iter_group++) {
    it = m_mapGroupToTreeItem.find(*iter_group);
    if (it != m_mapGroupToTreeItem.end()) {
      CRect rect;
      m_ctlItemTree.GetItemRect(it->second, &rect, FALSE);
      m_ctlItemTree.InvalidateRect(&rect);
    }
  }
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
                         vector<int> &vIndices, vector<pws_os::CUUID> &vFoundUUIDs)
{
  CItemData::FieldBits bsFields;
  bsFields.set();  // Default search is all text fields!

  CItemAtt::AttFieldBits bsAttFields;
  bsAttFields.reset();  // Default DON'T search attachment filename

  return FindAll(str, CaseSensitive, vIndices, vFoundUUIDs, bsFields, bsAttFields,
                 false, L"", 0, 0);
}

size_t DboxMain::FindAll(const CString &str, BOOL CaseSensitive,
                         vector<int> &vIndices,
                         vector<pws_os::CUUID> &vFoundUUIDs,
                         const CItemData::FieldBits &bsFields,
                         const CItemAtt::AttFieldBits &bsAttFields, 
                         const bool &subgroup_bset, const std::wstring &subgroup_name,
                         const int subgroup_object, const int subgroup_function)
{
  ASSERT(!str.IsEmpty());
  ASSERT(vIndices.empty());
  ASSERT(vFoundUUIDs.empty());

  StringX curGroup, curTitle, curUser, curNotes, curPassword, curURL, curAT, curXInt;
  StringX curEmail, curSymbols, curPolicyName, curRunCommand, listTitle, saveTitle;
  StringX curKBS;
  StringX curFN, curFP, curMT; // Attachment fields: FileName, FilePath, MediaType
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

  OrderedItemList OIL;
  OrderedItemList::const_iterator olistPos, olistEnd;
  if (m_IsListView) {
    listPos = m_core.GetEntryIter();
    listEnd = m_core.GetEntryEndIter();
  } else {
    MakeOrderedItemList(OIL);
    olistPos = OIL.begin();
    olistEnd = OIL.end();
  }

  while (m_IsListView ? (listPos != listEnd) : (olistPos != olistEnd)) {
    const CItemData &curitem = m_IsListView ? listPos->second : *olistPos;
    if (subgroup_bset &&
      !curitem.Matches(std::wstring(subgroup_name),
      subgroup_object, subgroup_function))
      goto nextentry;

    bFoundit = false;
    saveTitle = curTitle = curitem.GetTitle(); // saveTitle keeps orig case
    curGroup = curitem.GetGroup();
    curUser = curitem.GetUser();
    curPassword = curitem.GetPassword();
    curNotes = curitem.GetNotes();
    curURL = curitem.GetURL();
    curEmail = curitem.GetEmail();
    curSymbols = curitem.GetSymbols();
    curPolicyName = curitem.GetPolicyName();
    curRunCommand = curitem.GetRunCommand();
    curKBS = curitem.GetKBShortcut();
    curAT = curitem.GetAutoType();
    curXInt = curitem.GetXTimeInt();

    // Don't bother getting the attachment if not searching its fields
    if (bsAttFields.count() != 0) {
      if (curitem.HasAttRef()) {
        pws_os::CUUID attuuid = curitem.GetAttUUID();
        const CItemAtt &att = m_core.GetAtt(attuuid);
        curFN = att.GetFileName();
        curFP = att.GetFilePath();
        curMT = att.GetMediaType();
      } else {
        curFN = curFP = curMT = L"";
      }
    }

    if (!CaseSensitive) {
      ToLower(curGroup);
      ToLower(curTitle);
      ToLower(curUser);
      ToLower(curPassword);
      ToLower(curNotes);
      ToLower(curURL);
      ToLower(curEmail);
      // ToLower(curSymbols); - not needed as contains only symbols
      ToLower(curPolicyName);
      ToLower(curRunCommand);
      ToLower(curAT);
      ToLower(curFN);
      ToLower(curFP);
      ToLower(curMT);
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
      if (bsFields.test(CItemData::SYMBOLS) && ::wcsstr(curSymbols.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::RUNCMD) && ::wcsstr(curRunCommand.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::POLICYNAME) && ::wcsstr(curPolicyName.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::AUTOTYPE) && ::wcsstr(curAT.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::KBSHORTCUT) && ::wcsstr(curKBS.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsFields.test(CItemData::PWHIST)) {
        size_t pwh_max, err_num;
        PWHistList pwhistlist;
        CreatePWHistoryList(curitem.GetPWHistory(), pwh_max, err_num,
                            pwhistlist, PWSUtil::TMC_XML);
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
      if (bsAttFields.test(CItemAtt::FILENAME - CItemAtt::START) && ::wcsstr(curFN.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsAttFields.test(CItemAtt::FILEPATH - CItemAtt::START) && ::wcsstr(curFP.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
      if (bsAttFields.test(CItemAtt::MEDIATYPE - CItemAtt::START) && ::wcsstr(curMT.c_str(), searchstr)) {
        bFoundit = true;
        break;
      }
    } while(FALSE);  // only do it once!

    if (bFoundit) {
      // Find index in displayed list
      DisplayInfo *pdi = GetEntryGUIInfo(curitem);
      int li = pdi->list_index;
      ASSERT(m_ctlItemList.GetItemText(li, ititle) == saveTitle.c_str());
      // add to indices, bump retval
      vIndices.push_back(li);
      // Add into FoundUUID list
      vFoundUUIDs.push_back(curitem.GetUUID());
    } // match found in m_pwlist

nextentry:
    if (m_IsListView)
      listPos++;
    else
      olistPos++;
  } // while

  retval = vIndices.size();
  // Sort indices if in List View
  if (m_IsListView)
    sort(vIndices.begin(), vIndices.end());
  else
    OIL.clear();

  // If none found, reset found items
  if (retval == 0) {
    m_LastFoundTreeItem = NULL;
    m_LastFoundListItem = -1;

    // Update toolbar
    UpdateToolBarForSelectedItem(NULL);
  }

  return retval;
}

void DboxMain::SetFilterFindEntries(std::vector<pws_os::CUUID> *pvFoundUUIDs)
{
  // If the "Show entries from last Find" is active, we should not change this
  // as it will override the results - say if the user maximizes the PWS window
  // and the view is Refreshed using these entries rather than the original list
  // used for the filter.
  if (!(m_bFilterActive && m_bFindFilterDisplayed))
    m_FilterManager.SetFilterFindEntries(pvFoundUUIDs);
}

//Checks and sees if everything works and something is selected
BOOL DboxMain::SelItemOk()
{
  CItemData *pci = getSelectedItem();
  return (pci == NULL) ? FALSE : TRUE;
}

BOOL DboxMain::SelectEntry(const int i, BOOL MakeVisible)
{
  BOOL retval_tree, retval_list;
  ASSERT(i >= 0);

  if (m_ctlItemList.GetItemCount() == 0)
    return false;

  // Do both Tree & List views just in case user changes view
  retval_list = m_ctlItemList.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

  CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(i);
  ASSERT(pci != NULL);
  DisplayInfo *pdi = GetEntryGUIInfo(*pci);
  ASSERT(pdi->list_index == i);

  // Was there anything selected before?
  HTREEITEM hti = m_ctlItemTree.GetSelectedItem();

  // NULL means nothing was selected.
  if (hti != NULL) {
    // Time to remove the old "fake selection" (a.k.a. drop-highlight)
    // Make sure to undo "MakeVisible" on the previous selection.
    m_ctlItemTree.SetItemState(hti, 0, TVIS_DROPHILITED);
  }

  retval_tree = m_ctlItemTree.SelectItem(pdi->tree_item);

  if (MakeVisible) {
    if (m_ctlItemList.IsWindowVisible()) {
      m_ctlItemList.EnsureVisible(i, FALSE);
    } else {
      m_ctlItemTree.EnsureVisible(pdi->tree_item);
    }
  }
  m_ctlItemList.Invalidate();
  m_ctlItemTree.Invalidate();

  return m_ctlItemList.IsWindowVisible() ? retval_list : retval_tree;
}

void DboxMain::SelectFirstEntry()
{
  if (m_core.GetNumEntries() > 0) {
    // Ensure an entry is selected after open
    CItemData *pci(NULL);
    if (m_ctlItemList.IsWindowVisible()) {
      m_ctlItemList.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
      m_ctlItemList.EnsureVisible(0, FALSE);
      pci = (CItemData *)m_ctlItemList.GetItemData(0);
      m_ctlItemList.SetFocus();
    } else {
      HTREEITEM hitem = m_ctlItemTree.GetRootItem();
      if (hitem != NULL) {
        m_ctlItemTree.SelectItem(hitem);
        m_ctlItemTree.EnsureVisible(hitem);
        pci = (CItemData *)m_ctlItemTree.GetItemData(hitem);
        m_ctlItemTree.SetFocus();
      }
    }

    UpdateToolBarForSelectedItem(pci);
    SetDCAText(pci);
  }
}

BOOL DboxMain::SelectFindEntry(const int i, BOOL MakeVisible)
{
  BOOL retval_list, retval_tree;
  if (m_ctlItemList.GetItemCount() == 0)
    return FALSE;

  if (i < 0 || i >= m_ctlItemList.GetItemCount()) {
    ASSERT(0);
    return FALSE;
  }

  CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(i);
  ASSERT(pci != NULL);
  
  UnFindItem();

  // Do both Tree & List views just in case user changes view
  // Unselect all others first
  POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
  while (pos) {
    int iIndex = m_ctlItemList.GetNextSelectedItem(pos);
    m_ctlItemList.SetItemState(iIndex, 0, LVIS_FOCUSED | LVIS_SELECTED);
  }

  // Now select found item
  retval_list = m_ctlItemList.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
  m_LastFoundListItem = i;

  DisplayInfo *pdi = GetEntryGUIInfo(*pci);
  ASSERT(pdi->list_index == i);

  retval_tree = m_ctlItemTree.SelectItem(pdi->tree_item);
  m_ctlItemTree.SetItemState(pdi->tree_item, TVIS_BOLD, TVIS_BOLD);
  m_LastFoundTreeItem = pdi->tree_item;
  m_bBoldItem = true;

  if (MakeVisible) {
    if (m_ctlItemList.IsWindowVisible()) {
      m_ctlItemList.EnsureVisible(i, FALSE);
    } else {
      m_ctlItemTree.EnsureVisible(pdi->tree_item);
    }
  }
  m_ctlItemList.Invalidate();
  m_ctlItemTree.Invalidate();

  UpdateToolBarForSelectedItem(pci);
  return m_ctlItemList.IsWindowVisible() ? retval_list : retval_tree;
}

// Updates m_ctlItemList and m_ctlItemTree from m_pwlist
// updates of windows suspended until all data is in.
void DboxMain::RefreshViews(const ViewType iView)
{
  PWS_LOGIT_ARGS("iView=%d", iView);

  if (iView == NONE)
    return;

  // Remember if we need a refresh later
  // Luckily ViewType values are bitwise compaptible i.e.
  //   LISTONLY = 0x01, TREEONLY = 0x10, BOTHVIEWS = 0x11
  // Pity no |= operator for non-scoped enum
  m_iNeedRefresh = (ViewType)((int)m_iNeedRefresh | (int)iView);

  // Ignore if not yet initialised or we have suspended GUI updates
  if (!m_bInitDone || m_bSuspendGUIUpdates)
    return;

  m_bNumPassedFiltering = 0;
  m_bInRefresh = true;

  // Need to save last found item information
  pws_os::CUUID entry_uuid, tree_find_entry_uuid, list_find_entry_uuid;
  StringX sxGroupPath;

  // Save selected/highlighted entry
  GetSelectedItems(entry_uuid, tree_find_entry_uuid, list_find_entry_uuid, sxGroupPath);

  // can't use LockWindowUpdate 'cause only one window at a time can be locked
  if (iView & LISTONLY) {
    m_ctlItemList.SetRedraw(FALSE);
    m_ctlItemList.DeleteAllItems();
  }

  if (iView & TREEONLY) {
    m_ctlItemTree.SetRedraw(FALSE);
    m_mapGroupToTreeItem.clear();
    m_mapTreeItemToGroup.clear();
    m_ctlItemTree.DeleteAllItems();
  }

  m_bBoldItem = false;

  for (auto listPos = m_core.GetEntryIter(); listPos != m_core.GetEntryEndIter();
       listPos++) {
    CItemData &ci = m_core.GetEntry(listPos);
    DisplayInfo *pdi = GetEntryGUIInfo(ci, true);
    if (pdi != NULL) {
      if (iView & LISTONLY) {
        pdi->list_index = -1;
      }
      if (iView & TREEONLY) {
        pdi->tree_item = 0;
      }
    }

    InsertItemIntoGUITreeList(ci, -1, false, iView);
  }

  // Need to add any empty groups into the view
  if (m_bFilterActive) {
    bool bAlreadyExists;
    // 1. Filter on Unsaved changes
    if (m_bUnsavedDisplayed && m_core.HaveEmptyGroupsChanged()) {
      // We have some unsaved empty groups
      // Only add empty groups not yet saved
      std::vector<StringX> vSavedEmptyGroups = m_core.GetSavedEmptyGroups();
      for (auto emptyGrp : m_core.GetEmptyGroups()) {
        auto it = std::find(vSavedEmptyGroups.begin(), vSavedEmptyGroups.end(), emptyGrp);
        if (it == vSavedEmptyGroups.end()) {
          m_ctlItemTree.AddGroup(emptyGrp.c_str(), bAlreadyExists);
          m_bNumPassedFiltering++;
        }
      }
    }

    // 2. Filter on Find results
    if (m_bFindFilterDisplayed) {
      // Initially only test if empty group contains the search text (case insensitive)
      CString csFindString;
      m_FindToolBar.GetSearchText(csFindString);
      // If the user has cleared the Find search string but not cleared the previous results
      // Don't bother!
      if (!csFindString.IsEmpty()) {
        StringX sxFindString(csFindString);
        ToUpper(sxFindString);

        for (auto &emptyGrp : m_core.GetEmptyGroups()) {
          StringX sxCode = emptyGrp;
          ToUpper(sxCode);
          if (sxCode.find(sxFindString) != StringX::npos) {
            m_ctlItemTree.AddGroup(emptyGrp.c_str(), bAlreadyExists);
            m_bNumPassedFiltering++;
          }
        }
      }
    }

    // 3. Filter on entries with expired passwords - do not add any empty groups
    // Don't add empty groups if m_bExpireDisplayed == true

    // 4. User defined filter
    if (!m_bExpireDisplayed && !m_bUnsavedDisplayed && !m_bFindFilterDisplayed) {
      // Show all empty groups that passes a filter, which must include
      // a test on the group value
      for (auto &emptyGrp : m_core.GetEmptyGroups()) {
        if (m_FilterManager.PassesEmptyGroupFiltering(emptyGrp)) {
          m_ctlItemTree.AddGroup(emptyGrp.c_str(), bAlreadyExists);
          m_bNumPassedFiltering++;
        }
      }
    }
  } else {
    // Show all empty groups as no filter active
    bool bAlreadyExists;
    for (auto &emptyGrp : m_core.GetEmptyGroups()) {
      m_ctlItemTree.AddGroup(emptyGrp.c_str(), bAlreadyExists);
    }
  }  // m_bFilterActive

  m_ctlItemTree.SortTree(TVI_ROOT);
  SortListView();

  if (m_bImageInLV) {
    m_ctlItemList.SetColumnWidth(0, LVSCW_AUTOSIZE);
  }

  // re-enable and force redraw!
  if (iView & LISTONLY) {
    m_ctlItemList.SetRedraw(TRUE); 
    m_ctlItemList.Invalidate();
  }
  if (iView & TREEONLY) {
    m_ctlItemTree.SetRedraw(TRUE);
    m_ctlItemTree.Invalidate();
  }

  // Try and put selection back - positions in lists may have changed
  ReSelectItems(entry_uuid, tree_find_entry_uuid, list_find_entry_uuid, sxGroupPath);

  RestoreGUIStatusEx();

  if (m_bFilterActive)
    UpdateStatusBar();

  m_bInRefresh = false;
}

void DboxMain::RestoreWindows()
{
  PWS_LOGIT;

  ShowWindow(SW_RESTORE);

  // Restore saved DB preferences that may not have been saved in the database
  // over the minimize/restore event.
  // Can't use the fact that the string is empty, as that is a valid state!
  // Use arbitrary value "#Empty#" to indicate nothing here.
  if (m_savedDBprefs != EMPTYSAVEDDBPREFS) {
    if (m_core.HaveHeaderPreferencesChanged(m_savedDBprefs)) {
      PWSprefs::GetInstance()->Load(m_savedDBprefs);
    }
    m_savedDBprefs = EMPTYSAVEDDBPREFS;
  }

  RefreshViews();

  // Restore current horizontal scroll bar position
  m_ctlItemList.Scroll(CSize(m_iListHBarPos, 0));
  m_ctlItemTree.SetScrollPos(SB_HORZ, m_iTreeHBarPos);

  BringWindowToTop();

  CPWDialog::GetDialogTracker()->ShowOpenDialogs();
}

// this tells OnSize that the user is currently
// changing the size of the dialog, and not restoring it
void DboxMain::OnSizing(UINT fwSide, LPRECT pRect)
{
  PWS_LOGIT;

  CDialog::OnSizing(fwSide, pRect);
  m_bSizing = true;
}

void DboxMain::OnMove(int x, int y)
{
  CDialog::OnMove(x, y);
  // turns out that minimizing calls this with x = y = -32000. Oh joy.
  if (m_bInitDone && IsWindowVisible() == TRUE && x >= 0 && y >= 0) {
    CRect rc;
    GetWindowRect(&rc);
    PWSprefs::GetInstance()->SetPrefRect(rc.top, rc.bottom, rc.left, rc.right);
  }
}

void DboxMain::OnSize(UINT nType, int cx, int cy) 
{
  PWS_LOGIT_ARGS("nType=%d", nType);

  // Note that onsize runs before InitDialog (Gee, I love MFC)
  //  Also, OnSize is called AFTER the function has been peformed.
  //  To verify IF the function should be done at all, it must be checked in OnSysCommand.
  CDialog::OnSize(nType, cx, cy);

  // If m_bInitDone not true, then dialog has not yet been completely initialised
  if (!m_bInitDone) 
    return;

  m_bFindBarShown = m_FindToolBar.IsWindowVisible() == TRUE;

  if (nType != SIZE_MINIMIZED) {
    // Position the control bars - don't bother if just been minimized

    // If Find toolbar active - hide it until after we move
    if (m_bFindBarShown)
      SetFindToolBar(false);

    CRect rect, dragrect;
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
    bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
    if (bDragBarState) {
      const int i = GetSystemMetrics(SM_CYBORDER);
      const int j = rect.top + i;
      m_DDGroup.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
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

      m_DDAutotype.GetWindowRect(&dragrect);
      ScreenToClient(&dragrect);
      m_DDAutotype.SetWindowPos(NULL, dragrect.left, j, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      rect.top += dragrect.Height() + 2 * i;
    }
    m_ctlItemList.MoveWindow(&rect, TRUE);
    m_ctlItemTree.MoveWindow(&rect, TRUE);
  }

  PWSprefs *prefs = PWSprefs::GetInstance();
  CRect rc;

  switch (nType) {
    case SIZE_MINIMIZED:
      // Called when minimize button select on main dialog control box
      // or the system menu or by right clicking in the Taskbar
      // AFTER THE WINDOW HAS BEEN MINIMIZED!!!

      // Save DB preferences that may not have been saved in the database
      // over the minimize/restore event.
      m_savedDBprefs = prefs->Store();

      // PWSprefs::DatabaseClear == Lock DB on minimize but
      // only bother if currently unlocked
      if (m_TrayLockedState == UNLOCKED &&
          prefs->GetPref(PWSprefs::DatabaseClear)) {
        if (!LockDataBase()) {
          // Failed to save - abort minimize and clearing of data
          ShowWindow(SW_SHOW);
          return;
        }
      }

      m_ctlItemList.DeleteAllItems();
      m_mapGroupToTreeItem.clear();
      m_mapTreeItemToGroup.clear();
      m_ctlItemTree.DeleteAllItems();
      m_bBoldItem = false;
      m_LastFoundTreeItem = NULL;
      m_LastFoundListItem = -1;

      if (prefs->GetPref(PWSprefs::ClearClipboardOnMinimize))
        OnClearClipboard();

      if (prefs->GetPref(PWSprefs::UseSystemTray)) {      
        ShowWindow(SW_HIDE);

        // User can have 'stealth' mode where, as long as a hot-key is defined,
        // they can hide the System Tray icon when PasswordSafe is minimized
        if (prefs->GetPref(PWSprefs::HideSystemTray) && 
            prefs->GetPref(PWSprefs::HotKeyEnabled) &&
            prefs->GetPref(PWSprefs::HotKey) > 0)
          HideIcon();
        else if (IsIconVisible() == FALSE)
          ShowIcon();
      }
      break;

    case SIZE_MAXIMIZED:
    case SIZE_RESTORED:
      if (!m_bSizing) { // here if actually restored
        if (!RestoreWindowsData(false))
          return;

        m_bIsRestoring = true; // Stop 'sort of list view' hiding FindToolBar
        m_ctlItemTree.SetRestoreMode(true);

        RefreshViews(BOTHVIEWS);

        m_ctlItemTree.SetRestoreMode(false);
        m_bIsRestoring = false;

        // Restore saved DB preferences that may not have been saved in the database
        // over the minimize/restore event.
        // Can't use the fact that the string is empty, as that is a valid state!
        // Use arbitrary value "#Empty#" to indicate nothing here.
        if (m_savedDBprefs != EMPTYSAVEDDBPREFS) {
          if (m_core.HaveHeaderPreferencesChanged(m_savedDBprefs)) {
            prefs->Load(m_savedDBprefs);
          }
          m_savedDBprefs = EMPTYSAVEDDBPREFS;
        }

        CPWDialog::GetDialogTracker()->ShowOpenDialogs();

        // Restore current horizontal scroll bar position
        m_ctlItemList.Scroll(CSize(m_iListHBarPos, 0));
        m_ctlItemTree.SetScrollPos(SB_HORZ, m_iTreeHBarPos);
        RestoreGUIStatusEx();

        if (prefs->GetPref(PWSprefs::UseSystemTray) && IsIconVisible() == FALSE) {      
          ShowIcon();
        }
      }

      GetWindowRect(&rc);
      PWSprefs::GetInstance()->SetPrefRect(rc.top, rc.bottom, rc.left, rc.right);

      // Set timer for user-defined idle lockout, if selected (DB preference)
      KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
        ResetIdleLockCounter();
        SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
      }
      break;
    case SIZE_MAXSHOW:
    case SIZE_MAXHIDE:
      break;
  } // nType switch statement

    // If Find toolbar was active - reshow it
  if (m_bFindBarShown && !m_FindToolBar.IsWindowVisible())
    SetFindToolBar(true);

  m_bSizing = false;
}

void DboxMain::OnMinimize()
{
  // Called when the System Tray Minimize menu option is used

  // Also, in order to save the scroll positions during minimize
  // of the application, this should be the ONLY place that calls
  // "ShowWindow(SW_MINIMIZE);" as this call can't be intercepted
  // The one EXCEPTION is after calls to LockDataBase as we must save
  // the scroll positions before the GUI is cleared.
  PWS_LOGIT;

  // Save current horizontal scroll bar position
  if (m_ctlItemList.GetItemCount() == 0) {
    m_iListHBarPos = m_iTreeHBarPos = 0;
  } else {
    m_iListHBarPos = m_ctlItemList.GetScrollPos(SB_HORZ);
    m_iTreeHBarPos = m_ctlItemTree.GetScrollPos(SB_HORZ);
  }

  // Let OnSize handle this
  ShowWindow(SW_MINIMIZE);
}

void DboxMain::OnRestore()
{
  PWS_LOGIT;

  m_ctlItemTree.SetRestoreMode(true);

  // Called when the System Tray Restore menu option is used
  if (RestoreWindowsData(true)) {
    // Restore current horizontal scroll bar position
    m_ctlItemList.Scroll(CSize(m_iListHBarPos, 0));
    m_ctlItemTree.SetScrollPos(SB_HORZ, m_iTreeHBarPos);

    m_ctlItemTree.SetRestoreMode(false);

  // If Find toolbar was active - reshow it
    if (m_bFindBarShown && !m_FindToolBar.IsWindowVisible())
      SetFindToolBar(true);

    TellUserAboutExpiredPasswords();
  }
}

void DboxMain::OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult, const bool bTreeView)
{
  // Needed as need public function called by CPWTreeCtrl and CPWListCtrl
  if (bTreeView)
    OnTreeItemSelected(pNotifyStruct, pLResult);
  else
    OnListItemSelected(pNotifyStruct, pLResult);
}

void DboxMain::OnListItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // ListView
  *pLResult = 0L;
  CItemData *pci(NULL);

  int iItem(-1);
  switch (pNotifyStruct->code) {
    case NM_CLICK:
    {
      LPNMITEMACTIVATE pLVItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNotifyStruct);
      iItem = pLVItemActivate->iItem;
      break;
    }
    case LVN_KEYDOWN:
    {
      LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNotifyStruct);
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

  UpdateToolBarForSelectedItem(pci);
  SetDCAText(pci);

  m_LastFoundTreeItem = NULL;
  m_LastFoundListItem = -1;
}

void DboxMain::OnTreeItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // TreeView
  *pLResult = 0L;
  CItemData *pci(NULL);

  // Seems that under Vista with Windows Common Controls V6, it is ignoring
  // the single click on the button (+/-) of a node and only processing the 
  // double click, which generates a copy of whatever the user selected
  // for a double click (except that it invalid for a node!) and then does
  // the expand/collapse as appropriate.
  // This codes attempts to fix this.
  HTREEITEM hItem(NULL);

  UnFindItem();
  switch (pNotifyStruct->code) {
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
                            TVHT_ABOVE   | TVHT_BELOW | 
                            TVHT_TORIGHT | TVHT_TOLEFT))
            return;

      // If a group
      if (!m_ctlItemTree.IsLeaf(hItem)) {
        // If on indent or button
        if (htinfo.flags & (TVHT_ONITEMINDENT | TVHT_ONITEMBUTTON)) {
          // The group may be expanded or collapsed.
          const UINT uiState = m_ctlItemTree.GetItemState(hItem, TVIS_EXPANDED | TVIS_SELECTED);
          m_ctlItemTree.Expand(hItem, TVE_TOGGLE);

          // Update display state
          SaveGroupDisplayState();

          // If this group wasn't the currently selected item AND
          // it was expanded and so now collapsed - we might select something else!
          if ((uiState & TVIS_SELECTED) == 0 && (uiState & TVIS_EXPANDED) != 0) {
            // We have just collapsed the group
            // If the currently selected entry is NOT within this group - do nothing
            // If the currently selected entry IS within this group - select the group

            // Unselect TreeView entry and select group being collapsed
            HTREEITEM ti = m_ctlItemTree.GetSelectedItem();

            if (ti != NULL) {
              // Something selected
              CSecString scSelectedItemPath = m_ctlItemTree.GetGroup(ti);
              CSecString scCollapsedGroupPath = m_ctlItemTree.GetGroup(htinfo.hItem);
              const int iSelectLen = scSelectedItemPath.GetLength();
              const int iGroupLenP1 = scCollapsedGroupPath.GetLength() + 1;

              if (scSelectedItemPath == scCollapsedGroupPath ||
                  (iSelectLen > iGroupLenP1 &&
                    scSelectedItemPath.Left(iGroupLenP1) == (scCollapsedGroupPath + L"."))) {
                // Selected item is in this group or subgroup
                m_ctlItemTree.SetItemState(ti, 0, TVIS_SELECTED);
                m_ctlItemTree.SetItemState(htinfo.hItem, TVIS_SELECTED, TVIS_SELECTED);

                // Update Toolbar & Dragbar for a group
                UpdateToolBarForSelectedItem(NULL);
              }
            }
          }

          *pLResult = 1L; // We have toggled the group
          return;
        }
      }
      break;
    }
    case TVN_SELCHANGED:
      // Keyboard - We are given the new selected entry
      hItem = ((NMTREEVIEW *)pNotifyStruct)->itemNew.hItem;
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

  UpdateToolBarForSelectedItem(pci);
  SetDCAText(pci);

  m_LastFoundTreeItem = NULL;
  m_LastFoundListItem = -1;
}

void DboxMain::OnKeydownItemlist(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNotifyStruct);

  // TRUE = we have processed the key stroke - don't call anyone else
  *pLResult = TRUE;

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
  *pLResult = FALSE;
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
                                const bool bSort, const ViewType iView)
{
  DisplayInfo *pdi = GetEntryGUIInfo(ci, true);
  if (pdi != NULL && pdi->list_index != -1 && pdi->tree_item != 0) {
    // true iff item already displayed
    return iIndex;
  }

  DisplayInfo di;

  if (pdi != NULL)
    di = *pdi;

  int iResult = iIndex;
  if (iResult < 0) {
    iResult = m_ctlItemList.GetItemCount();
  }

  if (iView & LISTONLY)
    di.list_index = -1;
  if (iView & TREEONLY)
    di.tree_item = NULL;

  SetEntryGUIInfo(ci, di);

  if (m_bFilterActive) {
    if (!m_FilterManager.PassesFiltering(ci, m_core))
      return -1;

    m_bNumPassedFiltering++;
  }

  int nImage = GetEntryImage(ci);
  StringX sx_fielddata(L"");

  if (iView & LISTONLY) {
    // Insert the first column data (it will be empty if an image is in 1st column)
    if (!m_bImageInLV) {
      sx_fielddata = GetListViewItemText(ci, 0);
    }

    iResult = m_ctlItemList.InsertItem(iResult, sx_fielddata.c_str());

    if (iResult < 0) {
      // TODO: issue error here...
      return iResult;
    }

    di.list_index = iResult;
    if (m_bImageInLV)
      SetEntryImage(iResult, nImage);
  }

  if (iView & TREEONLY) {
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
    di.tree_item = ti;
  }

  if (iView & LISTONLY) {
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
      sx_fielddata = GetListViewItemText(ci, i);
      m_ctlItemList.SetItemText(iResult, i, sx_fielddata.c_str());
    }

    m_ctlItemList.SetItemData(iResult, (DWORD_PTR)&ci);
  }

  if (di.list_index == -1 || di.tree_item == NULL) {
    ASSERT(0);
  }

  SetEntryGUIInfo(ci, di);
  return iResult;
}

CItemData *DboxMain::getSelectedItem()
{
  CItemData *pci = NULL;
  if (m_core.GetNumEntries() == 0)
    return pci;

  if (m_ctlItemList.IsWindowVisible()) { // list view
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos) {
      int i = m_ctlItemList.GetNextSelectedItem(pos);
      pci = (CItemData *)m_ctlItemList.GetItemData(i);
      ASSERT(pci != NULL);
      ASSERT(GetEntryGUIInfo(*pci) != NULL && GetEntryGUIInfo(*pci)->list_index == i);
    }
  } else { // tree view; go from HTREEITEM to index
    HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
    if (ti != NULL) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (pci != NULL) {
        // leaf: do some sanity tests
        DisplayInfo *pdi = GetEntryGUIInfo(*pci);
        if (pdi->tree_item != ti) {
          pws_os::Trace(L"DboxMain::getSelectedItem: fixing pdi->tree_item!\n");
          pdi->tree_item = ti;
        }
      }
    } // ti != NULL
  } // tree view
  return pci;
}

void DboxMain::GetSelectedItems(pws_os::CUUID &entry_uuid,
                                pws_os::CUUID &tree_find_entry_uuid, pws_os::CUUID &list_find_entry_uuid,
                                StringX &sxGroupPath)
{
  entry_uuid = tree_find_entry_uuid = list_find_entry_uuid = pws_os::CUUID::NullUUID();
  sxGroupPath.clear();

  if (m_ctlItemTree.GetCount() == 0) {
    m_LastFoundTreeItem = NULL;
    m_LastFoundListItem = -1;
    return;
  }

  // Find last found entries (no groups)
  if (m_LastFoundTreeItem != NULL) {
    CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(m_LastFoundTreeItem);
    ASSERT(pci != nullptr);
    if (pci == nullptr)
      return;
    tree_find_entry_uuid = pci->GetUUID();
  }

  if (m_LastFoundListItem != -1) {
    CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(m_LastFoundListItem);
    ASSERT(pci != nullptr);
    if (pci == nullptr)
      return;
    list_find_entry_uuid = pci->GetUUID();
  }

  if (m_ctlItemTree.IsWindowVisible()) {
    // Tree view visible
    HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
    if (ti != NULL) {
      CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (pci == NULL) {
        // It is a group
        sxGroupPath = m_mapTreeItemToGroup[ti];
      } else {
        // An entry
        entry_uuid = pci->GetUUID();
      }
    }
  } else {
    // List view visible
    if (m_ctlItemList.GetSelectedCount() == 1) {
      POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
      if (pos != NULL) {
        int nItem = m_ctlItemList.GetNextSelectedItem(pos);
        CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(nItem);
        entry_uuid = pci->GetUUID();
      }
    }
  }
}

void DboxMain::ReSelectItems(pws_os::CUUID entry_uuid,
                             pws_os::CUUID &tree_find_entry_uuid, pws_os::CUUID &list_find_entry_uuid,
                             StringX sxGroupPath)
{
  // Find the entries
  HTREEITEM hItem(NULL);
  int item(-1);
  if (entry_uuid != CUUID::NullUUID()) {
    ItemListIter iter = Find(entry_uuid);
    if (iter != m_core.GetEntryEndIter()) {
      CItemData &ci = GetEntryAt(iter);
      DisplayInfo *pdi = GetEntryGUIInfo(ci);
      hItem = pdi->tree_item;
      item = pdi->list_index;
    }
  } else
   if (!sxGroupPath.empty() &&
       m_mapGroupToTreeItem.find(m_sxSelectedGroup) != m_mapGroupToTreeItem.end()) {
     hItem = m_mapGroupToTreeItem[sxGroupPath];
  }

  // Now select
  if (m_ctlItemTree.IsWindowVisible() && hItem != NULL) {
    m_ctlItemTree.SelectItem(hItem);
  } else if (item != -1) {
    m_ctlItemList.SetItemState(item, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
  }

  // Reset last found entries (no groups)
  if (tree_find_entry_uuid != pws_os::CUUID::NullUUID()) {
    ItemListIter iter = Find(tree_find_entry_uuid);
    if (iter != m_core.GetEntryEndIter()) {
      CItemData &ci = GetEntryAt(iter);
      DisplayInfo *pdi = GetEntryGUIInfo(ci);
      m_LastFoundTreeItem = pdi->tree_item;
    }
  }

  if (list_find_entry_uuid != pws_os::CUUID::NullUUID()) {
    ItemListIter iter = Find(list_find_entry_uuid);
    if (iter != m_core.GetEntryEndIter()) {
      CItemData &ci = GetEntryAt(iter);
      DisplayInfo *pdi = GetEntryGUIInfo(ci);
      m_LastFoundListItem = pdi->list_index;
    }
  }
}

void DboxMain::ClearAppData(const bool bClearMRE)
{
  PWS_LOGIT;

  if (bClearMRE)
    m_RUEList.ClearEntries();

  UpdateSystemTray(m_bOpen ? LOCKED : CLOSED);

  // Because GetText returns a copy, we cannot do anything about the names
  if (m_bInitDone) {
    // For long lists, this is painful, so we disable updates
    m_ctlItemList.LockWindowUpdate();
    m_ctlItemList.DeleteAllItems();
    m_ctlItemList.UnlockWindowUpdate();

    m_ctlItemTree.LockWindowUpdate();
    m_ctlItemTree.DeleteAllItems();
    m_ctlItemTree.UnlockWindowUpdate();

    m_mapGroupToTreeItem.clear();
    m_mapTreeItemToGroup.clear();

    m_MapEntryToGUI.clear();

    m_bBoldItem = false;

    if (bClearMRE) {
      // We are not closing the DB or clearing fields before opening and so
      // leave this information untouched
      // Clear found item references
      m_LastFoundTreeItem = NULL;
      m_LastFoundListItem = -1;

      // Zero entry UUID selected and first visible at minimize and group text
      m_LUUIDSelectedAtMinimize = CUUID::NullUUID();
      m_TUUIDSelectedAtMinimize = CUUID::NullUUID();
      m_LUUIDVisibleAtMinimize = CUUID::NullUUID();
      m_TUUIDVisibleAtMinimize = CUUID::NullUUID();
      m_sxSelectedGroup = L"";
      m_sxVisibleGroup = L"";

      // Clear Find
      m_FindToolBar.ClearFind();

      // Tidy up filters
      m_bFilterActive = m_bUnsavedDisplayed = m_bExpireDisplayed = m_bFindFilterDisplayed = false;
      m_FilterManager.SetFindFilter(false);
      m_FilterManager.SetFilterFindEntries(NULL);
      ClearFilter();
    }
  }
  m_bDBNeedsReading = true;
}

void DboxMain::OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult) 
{
  NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNotifyStruct;

  // Get column index to CItemData value
  int iIndex = pNMListView->iSubItem;
  int iTypeSortColumn = m_nColumnTypeByIndex[iIndex];

  if (m_iTypeSortColumn == iTypeSortColumn) {
    m_bSortAscending = !m_bSortAscending;
    PWSprefs *prefs = PWSprefs::GetInstance();
    prefs->SetPref(PWSprefs::SortAscending, m_bSortAscending);
  } else {
    // Turn off all previous sort arrows
    // Note: not sure where, as user may have played with the columns!
    HDITEM hdi;
    hdi.mask = HDI_FORMAT;
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
  OnHideFindToolbar();

  *pLResult = TRUE;
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
}

void DboxMain::OnHeaderRClick(NMHDR *, LRESULT *pLResult)
{
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;
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
  *pLResult = TRUE;
}

void DboxMain::OnHeaderBeginDrag(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // Called for HDN_BEGINDRAG which changes the column order when CC not visible
  // Stop drag of first column (image)

  NMHEADER *phdn = (NMHEADER *)pNotifyStruct;

  *pLResult = (m_bImageInLV && phdn->iItem == 0) ? TRUE : FALSE;

  SaveColumnWidths();
}

void DboxMain::OnHeaderEndDrag(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // Called for HDN_ENDDRAG which changes the column order when CC not visible
  // Unfortunately the changes are only really done when this call returns,
  // hence the PostMessage to get the information later

  // Get control after operation is really complete
  NMHEADER *phdn = (NMHEADER *)pNotifyStruct;

  // Stop drag of first column (image)
  if (m_bImageInLV && 
      (phdn->iItem == 0 || 
       (((phdn->pitem->mask & HDI_ORDER) == HDI_ORDER) && 
        phdn->pitem->iOrder == 0))) {
    *pLResult = TRUE;
    return;
  }

  // Otherwise allow
  PostMessage(PWS_MSG_HDR_DRAG_COMPLETE);
  *pLResult = FALSE;
}

void DboxMain::OnHeaderNotify(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMHEADER *phdn = (NMHEADER *)pNotifyStruct;
  *pLResult = FALSE;

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
    OnHideFindToolbar();
}

void DboxMain::OnTreeView() 
{
  SetTreeView();
  if (m_FindToolBar.IsVisible())
    OnHideFindToolbar();
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
      m_DDAutotype.Init(IDB_AUTOTYPE_NEW, IDB_DRAGAUTOX_NEW);
    } else if (menuItem == ID_MENUITEM_OLD_TOOLBAR) {
      m_DDGroup.Init(IDB_DRAGGROUP_CLASSIC, IDB_DRAGGROUPX_CLASSIC);
      m_DDTitle.Init(IDB_DRAGTITLE_CLASSIC, IDB_DRAGTITLEX_CLASSIC);
      m_DDUser.Init(IDB_DRAGUSER_CLASSIC, IDB_DRAGUSERX_CLASSIC);
      m_DDPassword.Init(IDB_DRAGPASSWORD_CLASSIC, IDB_DRAGPASSWORDX_CLASSIC);
      m_DDNotes.Init(IDB_DRAGNOTES_CLASSIC, IDB_DRAGNOTESX_CLASSIC);
      m_DDURL.Init(IDB_DRAGURL_CLASSIC, IDB_DRAGURLX_CLASSIC);
      m_DDemail.Init(IDB_DRAGEMAIL_CLASSIC, IDB_DRAGEMAILX_CLASSIC);
      m_DDAutotype.Init(IDB_AUTOTYPE_CLASSIC, IDB_DRAGAUTOX_CLASSIC);
    } else {
      ASSERT(0);
    }
    m_MainToolBar.LoadDefaultToolBar(m_toolbarMode);
    m_FindToolBar.LoadDefaultToolBar(m_toolbarMode);
    CString csButtonNames = PWSprefs::GetInstance()->GetPref(PWSprefs::MainToolBarButtons).c_str();
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
      m_DDAutotype.ReInit(IDB_AUTOTYPE_NEW, IDB_DRAGAUTOX_NEW);
    } else if (menuItem == ID_MENUITEM_OLD_TOOLBAR) {
      m_DDGroup.ReInit(IDB_DRAGGROUP_CLASSIC, IDB_DRAGGROUPX_CLASSIC);
      m_DDTitle.ReInit(IDB_DRAGTITLE_CLASSIC, IDB_DRAGTITLEX_CLASSIC);
      m_DDUser.ReInit(IDB_DRAGUSER_CLASSIC, IDB_DRAGUSERX_CLASSIC);
      m_DDPassword.ReInit(IDB_DRAGPASSWORD_CLASSIC, IDB_DRAGPASSWORDX_CLASSIC);
      m_DDNotes.ReInit(IDB_DRAGNOTES_CLASSIC, IDB_DRAGNOTESX_CLASSIC);
      m_DDURL.ReInit(IDB_DRAGURL_CLASSIC, IDB_DRAGURLX_CLASSIC);
      m_DDemail.ReInit(IDB_DRAGEMAIL_CLASSIC, IDB_DRAGEMAILX_CLASSIC);
      m_DDAutotype.ReInit(IDB_AUTOTYPE_CLASSIC, IDB_DRAGAUTOX_CLASSIC);
    } else {
      ASSERT(0);
    }
    m_DDGroup.Invalidate(); m_DDTitle.Invalidate(); m_DDUser.Invalidate();
    m_DDPassword.Invalidate(); m_DDNotes.Invalidate(); m_DDURL.Invalidate();
    m_DDemail.Invalidate(); m_DDAutotype.Invalidate();
  }
  m_menuManager.SetImageList(&m_MainToolBar);

  m_MainToolBar.Invalidate();
  m_FindToolBar.Invalidate();

  SetToolBarPositions();
}

void DboxMain::OnExpandAll()
{
  m_ctlItemTree.OnExpandAll();

  SaveGroupDisplayState();
}

void DboxMain::OnCollapseAll()
{

  // GetSelected Item
  HTREEITEM ti = m_ctlItemTree.GetSelectedItem();

  m_ctlItemTree.OnCollapseAll();

  SaveGroupDisplayState();

  if (ti == NULL) {
    // Nothing selected - do nothing more
    return;
  }

  CSecString scSelectedItemPath = m_ctlItemTree.GetGroup(ti);

  int iDelimiterPosition = scSelectedItemPath.Find(L".");
  ASSERT(iDelimiterPosition != 0);  // Group name can't start with a delimiter!

  // Entry in root or a top level group - leave selected
  if ((m_ctlItemTree.IsLeaf(ti) && scSelectedItemPath.IsEmpty()) ||
      (!m_ctlItemTree.IsLeaf(ti) && iDelimiterPosition == -1)) {
     return;
  }

  // Unselect old selected item
  m_ctlItemTree.SetItemState(ti, 0, TVIS_SELECTED);

  // Get its root level group
  StringX sxRootGroup = iDelimiterPosition == -1 ? scSelectedItemPath : scSelectedItemPath.Left(iDelimiterPosition);

  // Find it in the Tree
  auto iter = m_mapGroupToTreeItem.find(sxRootGroup);
  ASSERT(iter != m_mapGroupToTreeItem.end());

  // Now selected it
  m_ctlItemTree.SetItemState(iter->second, TVIS_SELECTED, TVIS_SELECTED);

  // Update for group
  UpdateToolBarForSelectedItem(NULL);
}

void DboxMain::OnTimer(UINT_PTR nIDEvent)
{
  if ((nIDEvent == TIMER_LOCKONWTSLOCK && IsWorkstationLocked()) ||
      (nIDEvent == TIMER_LOCKDBONIDLETIMEOUT &&
       DecrementAndTestIdleLockCounter())) {
    // OK, so we need to lock. If we're not using a system tray,
    // just minimize. If we are, then we need to hide (which
    // also requires children be hidden explicitly)
    m_vGroupDisplayState = GetGroupDisplayState();

    if (!LockDataBase())
      return;

    // Save any database preference changes
    PWSprefs *prefs = PWSprefs::GetInstance();
    m_savedDBprefs = prefs->Store();

    // Hide everything
    CPWDialog::GetDialogTracker()->HideOpenDialogs();

    // Now hide/minimize main dialog
    // NOTE: Do not call OnMinimize if minimizing as this will overwrite
    // the scroll bar positions
    if (prefs->GetPref(PWSprefs::UseSystemTray)) {
      ShowWindow(SW_HIDE);
    } else {
      ShowWindow(SW_MINIMIZE);
    }

    if (nIDEvent == TIMER_LOCKONWTSLOCK)
      KillTimer(TIMER_LOCKONWTSLOCK);
  } else if (nIDEvent == TIMER_EXPENT) {
    // once a day, we want to check the expired entries list
    CheckExpireList();
  }
}

LRESULT DboxMain::OnSessionChange(WPARAM wParam, LPARAM )
{
  PWS_LOGIT_ARGS("wParam=%d", wParam);

  // Windows XP and later only
  // Handle Lock/Unlock, Fast User Switching and Remote access.
  // Won't be called if the registration failed (i.e. < Windows XP
  // or the "Windows Terminal Server" service wasn't active at startup).
  PWSprefs *prefs = PWSprefs::GetInstance();

  switch (wParam) {
    case WTS_CONSOLE_DISCONNECT:
    case WTS_REMOTE_DISCONNECT:
    case WTS_SESSION_LOCK:
      if (m_bOpen && m_TrayLockedState == UNLOCKED) {
        m_bWSLocked = true;

        if (prefs->GetPref(PWSprefs::LockOnWindowLock) &&
            LockDataBase()) {
          // Save any database preference changes
          m_savedDBprefs = prefs->Store();

          // Hide everything
          CPWDialog::GetDialogTracker()->HideOpenDialogs();

          // Now hide/minimize main dialog
          if (prefs->GetPref(PWSprefs::UseSystemTray)) {
            ShowWindow(SW_HIDE);
          } else {
            OnMinimize();
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
  PWS_LOGIT;

  // Bug 1149: Check DB open before doing anything
  if (!m_core.IsDbOpen())
    return true;

  /*
   * Since we clear the data, any unchanged changes will be lost,
   * so we force a save if database is modified, and fail
   * to lock if the save fails (unless db is r-o).
   *
   * returns false iff save was required AND failed.
   */

  // Now try and save changes
  if (m_core.HasDBChanged() || m_bEntryTimestampsChanged) {
    if (Save() != PWScore::SUCCESS) {
      // If we don't warn the user, data may be lost!
      CGeneralMsgBox gmb;
      CString cs_text(MAKEINTRESOURCE(IDS_COULDNOTSAVE)), 
              cs_title(MAKEINTRESOURCE(IDS_SAVEERROR));
      gmb.MessageBox(cs_text, cs_title, MB_ICONSTOP);
      return false;
    }
  }

  // Save current horizontal scroll bar position BEFORE
  // everyting is cleared
  if (m_ctlItemList.GetItemCount() == 0) {
    m_iListHBarPos = m_iTreeHBarPos = 0;
  } else {
    m_iListHBarPos = m_ctlItemList.GetScrollPos(SB_HORZ);
    m_iTreeHBarPos = m_ctlItemTree.GetScrollPos(SB_HORZ);
  }

  // If a Find is active, save its status
  pws_os::CUUID entry_uuid;
  m_bFindToolBarVisibleAtLock = m_FindToolBar.IsVisible();
  m_iCurrentItemFound = m_FindToolBar.GetLastSelectedFoundItem(entry_uuid);

  // If there's a pending dialog box prompting for a
  // password, we need to kill it, since we will prompt
  // for the existing dbase's password upon restore.
  // Avoid lots of edge cases this way.
  CancelPendingPasswordDialog();

  // Do NOT call PWScore::ReInit as it will clear commands preventing the
  // user from undoing commands after unlocking DB

  // Reset core and clear all associated data EXCEPT commands & DB state
  m_core.ClearDBData();

  // Clear the application data before locking
  ClearAppData(false);

  // Because LockDatabase actually doen't minimize the Window, the OnSize
  // routine is not called to clear the clipboard - so do it here
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ClearClipboardOnMinimize)) {
    ClearClipboardData();
  }
  
  // If DB is currently in R/W and it was opened in R-O mode, now make it R-O
  // (FR703)
  // Unless there's a child dbox open (BR1192)
  if (!IsDBReadOnly() && m_bDBInitiallyRO && !CPWDialog::GetDialogTracker()->AnyOpenDialogs()) {
    ChangeMode(false);
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

  return bResult;
}

void DboxMain::OnChangeTreeFont()
{
  Fonts *pFonts = Fonts::GetInstance();

  const bool bWasUsingNewProtectSymbol = pFonts->IsSymbolSuported(Fonts::PROTECT, Fonts::TREELIST);
  const bool bWasUsingNewAttachmentSymbol = pFonts->IsSymbolSuported(Fonts::ATTACHMENT, Fonts::TREELIST);

  ChangeFont(CFontsDialog::TREELISTFONT);
  
  // Verify protect and attachment symbols supported
  pFonts->VerifySymbolsSupported();

  // If supported - fine - use it
  bool bNewProtectedSymbolSupported = pFonts->IsSymbolSuported(Fonts::PROTECT, Fonts::TREELIST);

  if (!bNewProtectedSymbolSupported) {
    pws_os::Trace(L"New font does not support the new entry Protected symbol.\n");
  }

  // If supported - fine - use it
  bool bNewAttachmentSymbolSupported = pFonts->IsSymbolSuported(Fonts::ATTACHMENT, Fonts::TREELIST);

  if (!pFonts->IsSymbolSuported(Fonts::ATTACHMENT, Fonts::TREELIST)) {
    pws_os::Trace(L"New font does not support the new entry has Attachment symbol.\n");
  }

  // If we have changed the "protect" or "attachment" symbol, then the Invalidate in the ChangeFont
  // routine is not good enough - we have to change the actual displayed string
  if ((bWasUsingNewProtectSymbol != bNewProtectedSymbolSupported) ||
      (bWasUsingNewAttachmentSymbol != bNewAttachmentSymbolSupported)) {
    RefreshViews();
  }
}

void DboxMain::OnChangeAddEditFont()
{
  ChangeFont(CFontsDialog::ADDEDITFONT);

  Fonts *pFonts = Fonts::GetInstance();

  // Verify protect and attachment symbols supported
  pFonts->VerifySymbolsSupported();

  // Protected entry symbol
  bool bNewProtectedSymbolSupported = pFonts->IsSymbolSuported(Fonts::PROTECT, Fonts::ADDEDIT);

  if (!bNewProtectedSymbolSupported) {
    pws_os::Trace(L"New font does not support the new entry Protected symbol.\n");
  }

  // Entry has an attachment symbol
  bool bNewAttachmentSymbolSupported = pFonts->IsSymbolSuported(Fonts::ATTACHMENT, Fonts::ADDEDIT);

  if (!bNewAttachmentSymbolSupported) {
    pws_os::Trace(L"New font does not support the new entry has Attachment symbol.\n");
  }
}

void DboxMain::OnChangeNotesFont()
{
  ChangeFont(CFontsDialog::NOTESFONT);
  
  UpdateNotesTooltipFont();
}

void DboxMain::OnChangePswdFont()
{
  ChangeFont(CFontsDialog::PASSWORDFONT);
}

void DboxMain::OnChangeVKFont()
{
  ChangeFont(CFontsDialog::VKEYBOARDFONT);
}

void DboxMain::ChangeFont(const CFontsDialog::FontType iType)
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  Fonts *pFonts = Fonts::GetInstance();
  CFont *pOldFont;
  StringX cs_FontName, cs_SampleText;
  LOGFONT lf, dflt_lf;
  PWSprefs::StringPrefs pref_Font(PWSprefs::TreeFont), pref_FontSampleText(PWSprefs::TreeListSampleText);
  PWSprefs::IntPrefs pref_font_point_size(PWSprefs::TreeFontPtSz);
  int iFontSize(0);

  DWORD dwFlags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

  switch (iType) {
    case CFontsDialog::TREELISTFONT:
      pref_Font = PWSprefs::TreeFont;
      pref_font_point_size = PWSprefs::TreeFontPtSz;
      pref_FontSampleText = PWSprefs::TreeListSampleText;
      pOldFont = m_ctlItemTree.GetFont();
      pOldFont->GetLogFont(&lf);
      pFonts->GetDefaultTreeListFont(dflt_lf);
      break;
    case CFontsDialog::ADDEDITFONT:
      pref_Font = PWSprefs::AddEditFont;
      pref_font_point_size = PWSprefs::AddEditFontPtSz;
      pref_FontSampleText = PWSprefs::AddEditSampleText;
      pFonts->GetAddEditFont(&lf);
      pFonts->GetDefaultAddEditFont(dflt_lf);
      break;
    case CFontsDialog::PASSWORDFONT:
      pref_Font = PWSprefs::PasswordFont;
      pref_font_point_size = PWSprefs::PasswordFontPtSz;
      pref_FontSampleText = PWSprefs::PswdSampleText;
      pFonts->GetPasswordFont(&lf);
      pFonts->GetDefaultPasswordFont(dflt_lf);
      break;
    case CFontsDialog::NOTESFONT:
      pref_Font = PWSprefs::NotesFont;
      pref_font_point_size = PWSprefs::NotesFontPtSz;
      pref_FontSampleText = PWSprefs::NotesSampleText;
      pFonts->GetNotesFont(&lf);
      pFonts->GetDefaultNotesFont(dflt_lf);
      break;
    case CFontsDialog::VKEYBOARDFONT:
      // Note Virtual Keyboard font is not kept in Fonts class - so set manually
      pref_Font = PWSprefs::VKeyboardFontName;
      pref_FontSampleText = PWSprefs::VKSampleText;
      dwFlags |= CF_LIMITSIZE | CF_NOSCRIPTSEL;
      dflt_lf = {-16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L""};
      lf = dflt_lf;

      // Get resolution
      HDC hDC = ::GetWindowDC(GetSafeHwnd());
      const int Ypixels = GetDeviceCaps(hDC, LOGPIXELSY);
      ::ReleaseDC(GetSafeHwnd(), hDC);

      // See if user has set a point size
      iFontSize = PWSprefs::GetInstance()->GetPref(PWSprefs::VKFontPtSz);
      if (iFontSize == 0) {
        // Use default
        iFontSize = MulDiv(16, 72, Ypixels) * 10;
        PWSprefs::GetInstance()->SetPref(PWSprefs::VKFontPtSz, iFontSize);
      }

      // Update font point size
      lf.lfHeight = -MulDiv(iFontSize / 10, Ypixels, 72);

      // Get VKeyboard font in case the user wants to change this.
      cs_FontName = prefs->GetPref(PWSprefs::VKeyboardFontName);
      if (cs_FontName.length() != 0 && cs_FontName.length() <= LF_FACESIZE) {
        memcpy_s(lf.lfFaceName, LF_FACESIZE * sizeof(wchar_t),
          cs_FontName.c_str(), cs_FontName.length() * sizeof(wchar_t));
      }
      break;
    // NO "default" statement to generate compiler error if enum missing
  }

  cs_SampleText = prefs->GetPref(pref_FontSampleText);

  CFontsDialog fontdlg(&lf, dwFlags, NULL, NULL, iType);

  if (iType == CFontsDialog::VKEYBOARDFONT) {
    fontdlg.m_cf.nSizeMin = 8;
    fontdlg.m_cf.nSizeMax = 16;
  }

  fontdlg.m_sampletext = cs_SampleText.c_str();

  INT_PTR rc = fontdlg.DoModal();
  if (rc== IDOK) {
    iFontSize = fontdlg.GetSize();
    if (iType == CFontsDialog::VKEYBOARDFONT && fontdlg.m_bReset) {
      // User requested the Virtual Keyboard to be reset now
      // Other fonts are just reset within the Fontdialog without exiting
      prefs->ResetPref(pref_Font);
      prefs->ResetPref(pref_FontSampleText);
      return;
    }

    CString csfn(lf.lfFaceName), csdfltfn(dflt_lf.lfFaceName);
    switch (iType) {
      case CFontsDialog::TREELISTFONT:
        // Set current tree/list font
        pFonts->SetTreeListFont(&lf, iFontSize);

        // Transfer the fonts to the tree and list windows
        m_ctlItemTree.SetUpFont();
        m_ctlItemList.SetUpFont();
        m_LVHdrCtrl.SetFont(pFonts->GetTreeListFont());

        // Recalculate header widths but don't change column widths
        CalcHeaderWidths();

        // Redraw in new font
        m_ctlItemTree.Invalidate();
        m_ctlItemList.Invalidate();
        break;
      case CFontsDialog::ADDEDITFONT:
        // Transfer the new font to the selected Add/Edit fields
        pFonts->SetAddEditFont(&lf, iFontSize);

        // Change the Find Toolbar font
        m_FindToolBar.ChangeFont();
        break;
      case CFontsDialog::PASSWORDFONT:
        // Transfer the new font to the passwords
        pFonts->SetPasswordFont(&lf, iFontSize);

        // Recalculating row height
        m_ctlItemList.UpdateRowHeight(true);
        break;
      case CFontsDialog::NOTESFONT:
        // Transfer the new font to the Notes field
        pFonts->SetNotesFont(&lf, iFontSize);

        // Recalculating row height
        m_ctlItemList.UpdateRowHeight(true);
        break;
      case CFontsDialog::VKEYBOARDFONT:
        // Note Virtual Keyboard font is not kept in Fonts class - so set manually
        prefs->SetPref(PWSprefs::VKFontPtSz, iFontSize);

        if (csfn.IsEmpty()) {
          // Delete config VKeyboard font face name
          prefs->ResetPref(pref_Font);
        } else {
          // Save user's choice of VKeyboard font face name
          // Remove leading @ (OpenType) if present
          if (csfn.Left(1) == L"@")
            csfn = csfn.Mid(1);

          if (csfn == CString(CVKeyBoardDlg::ARIALUMS))
            prefs->ResetPref(pref_Font);
          else
            prefs->SetPref(pref_Font, LPCWSTR(csfn));

          prefs->SetPref(pref_FontSampleText, LPCWSTR(fontdlg.m_sampletext));
        }
        return;
      // NO "default" statement to generate compiler error if enum missing
    }

    // Check if default
    if (lf.lfHeight == dflt_lf.lfHeight &&
        lf.lfWeight == dflt_lf.lfWeight &&
        lf.lfItalic == dflt_lf.lfItalic &&
        csfn == csdfltfn) {
      // Delete config font
      prefs->ResetPref(pref_Font);
    } else { // Save user's choice of font
      CString font_str;
      font_str.Format(L"%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s",
        lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
        lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut,
        lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision,
        lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
      prefs->SetPref(pref_Font, LPCWSTR(font_str));
      prefs->SetPref(pref_font_point_size, iFontSize);
    }

    // Save user's sample text
    prefs->SetPref(pref_FontSampleText, LPCWSTR(fontdlg.m_sampletext));
  }
}

void DboxMain::UpdateSystemTray(const DBSTATE s)
{
  CString csTooltip(L"");
  if (m_core.IsDbOpen()) {
    std::wstring cdrive, cdir, cFilename, cExtn;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, cFilename, cExtn);

    if (m_iDBIndex == 0) {
      switch (s) {
      case LOCKED:
        csTooltip.Format(L"[%s\n%s]", (cdrive + cdir).c_str(),
          (cFilename + cExtn).c_str());
        break;
      case UNLOCKED:
        csTooltip.Format(L"%s\n%s", (cdrive + cdir).c_str(),
          (cFilename + cExtn).c_str());
        break;
      case CLOSED:
        break;
      }
    } else {
      switch (s) {
      case LOCKED:
        csTooltip.Format(L"%2d: [%s\n    %s]", m_iDBIndex, (cdrive + cdir).c_str(),
          (cFilename + cExtn).c_str());
        break;
      case UNLOCKED:
        csTooltip.Format(L"%2d:%s\n    %s", m_iDBIndex, (cdrive + cdir).c_str(),
          (cFilename + cExtn).c_str());
        break;
      case CLOSED:
        break;
      }
    }
  }

  switch (s) {
    case LOCKED:
      SetSystemTrayState(LOCKED);
      if (!csTooltip.IsEmpty()) {
        SetTooltipText(csTooltip);
      }
      break;
    case UNLOCKED:
      SetSystemTrayState(UNLOCKED);
      if (!csTooltip.IsEmpty())
        SetTooltipText(csTooltip);
      break;
    case CLOSED:
      SetSystemTrayState(CLOSED);
        break;
    default:
    ASSERT(0);
  }

  UpdateStatusBar();
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
    m_sxAutoType = L"";
    m_vactionverboffsets.clear();
  } else {
    // Either do it because they pressed the right menu/shortcut
    // or they had specified Do Autotype flag [autotype]
    m_bDoAutoType = bDoAutotype || autotypeReplacements > 0;
    m_sxAutoType = m_bDoAutoType ? sxAutotype : L"";
    if (m_bDoAutoType)
      m_vactionverboffsets = vactionverboffsets;
  }
  bool rc = m_runner.issuecmd(sxFile, sxParameters, !m_sxAutoType.empty());

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

void DboxMain::SetDefaultColumns()
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
                                 (rect.Width() / 3 + rect.Width() % 3), false);
  int i2ndWidth = prefs->GetPref(PWSprefs::Column2Width,
                                 rect.Width() / 3, false);
  int i3rdWidth = prefs->GetPref(PWSprefs::Column3Width,
                                 rect.Width() / 3, false);

  int iWidth, iSortColumn /* Not used here but needed for GetHeaderColumnProperties call */;

  GetHeaderColumnProperties(CItemData::TITLE, cs_header, iWidth, iSortColumn);
  m_ctlItemList.InsertColumn(0, cs_header);
  hdi.lParam = CItemData::TITLE;
  m_LVHdrCtrl.SetItem(0, &hdi);
  m_ctlItemList.SetColumnWidth(0, i1stWidth);

  GetHeaderColumnProperties(CItemData::USER, cs_header, iWidth, iSortColumn);
  m_ctlItemList.InsertColumn(1, cs_header);
  hdi.lParam = CItemData::USER;
  m_LVHdrCtrl.SetItem(1, &hdi);
  m_ctlItemList.SetColumnWidth(1, i2ndWidth);

  GetHeaderColumnProperties(CItemData::NOTES, cs_header, iWidth, iSortColumn);
  m_ctlItemList.InsertColumn(2, cs_header);
  hdi.lParam = CItemData::NOTES;
  m_LVHdrCtrl.SetItem(2, &hdi);
  m_ctlItemList.SetColumnWidth(2, i3rdWidth);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPasswordInTree)) {
    GetHeaderColumnProperties(CItemData::PASSWORD, cs_header, iWidth, iSortColumn);
    m_ctlItemList.InsertColumn(3, cs_header);
    hdi.lParam = CItemData::PASSWORD;
    m_LVHdrCtrl.SetItem(3, &hdi);
    m_ctlItemList.SetColumnWidth(3,
                                 PWSprefs::GetInstance()->GetPref(PWSprefs::Column4Width,
                                 rect.Width() / 4, false));
  }

  int ioff = 3;
  CItemData::FieldType defCols[] = {CItemData::URL, CItemData::EMAIL,
                                    CItemData::RUNCMD, CItemData::CTIME,
                                    CItemData::PMTIME, CItemData::ATIME,
                                    CItemData::XTIME, CItemData::RMTIME,
                                    CItemData::POLICY,
  };

  for (int i = 0; i < sizeof(defCols)/sizeof(defCols[0]); i++) {
    GetHeaderColumnProperties(defCols[i], cs_header, iWidth, iSortColumn);
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

  // Capture columns shown:
  wchar_t *next_token;
  wchar_t *token = wcstok_s(pTemp, pSep, &next_token);
  while(token) {
    vi_columns.push_back(_wtoi(token));
    token = wcstok_s(NULL, pSep, &next_token);
  }
  free(pTemp);

  // If present, the images are always first
  int iType= *vi_columns.begin();
  if (iType == CItemData::UUID) {
    m_bImageInLV = true;
    m_ctlItemList.SetImageList(m_pImageList, LVSIL_NORMAL);
    m_ctlItemList.SetImageList(m_pImageList, LVSIL_SMALL);
  }

  int icol(0);
  int iWidth, iSortColumn /* Not used here but needed for GetHeaderColumnProperties call */;

  for (vi_IterColumns = vi_columns.begin();
       vi_IterColumns != vi_columns.end();
       vi_IterColumns++) {
    iType = *vi_IterColumns;
    GetHeaderColumnProperties(iType, cs_header, iWidth, iSortColumn);
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

  // Capture column widths shown:
  wchar_t *next_token;
  wchar_t *token = wcstok_s(pWidths, pSep, &next_token);
  while(token) {
    vi_widths.push_back(_wtoi(token));
    token = wcstok_s(NULL, pSep, &next_token);
  }
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
  int iWidth;
  int iSortColumn /* Not used here but needed for GetHeaderColumnProperties call */;
  HDITEM hdi;
  int iNewIndex(iIndex);

  //  If iIndex = -1, means drop on the end
  if (iIndex < 0)
    iNewIndex = m_nColumns;

  hdi.mask = HDI_LPARAM | HDI_WIDTH;
  GetHeaderColumnProperties(iType, cs_header, iWidth, iSortColumn);
  ASSERT(!cs_header.IsEmpty());
  iNewIndex = m_ctlItemList.InsertColumn(iNewIndex, cs_header);
  ASSERT(iNewIndex != -1);
  hdi.lParam = iType;
  hdi.cxy = iWidth;
  m_LVHdrCtrl.SetItem(iNewIndex, &hdi);
}

void DboxMain::DeleteColumn(const int iType)
{
  // Delete column
  m_ctlItemList.DeleteColumn(m_nColumnIndexByType[iType]);
}

void DboxMain::SetHeaderInfo(const bool bSetWidths)
{
  HDITEM hdi_get;
  // CHeaderCtrl get values
  hdi_get.mask = HDI_LPARAM | HDI_ORDER;

  m_nColumns = m_LVHdrCtrl.GetItemCount();
  ASSERT(m_nColumns > 1);  // Title & User are mandatory!

  // re-initialise array
  for (int i = 0; i < CItem::LAST_DATA; i++) {
    m_nColumnIndexByType[i] = m_nColumnIndexByOrder[i] =  m_nColumnTypeByIndex[i] = -1;

    // Only reset column width if we are going to set them
    if (bSetWidths)
      m_nColumnWidthByIndex[i] = -1;
  }

  m_LVHdrCtrl.GetOrderArray(m_nColumnIndexByOrder, m_nColumns);

  for (int iOrder = 0; iOrder < m_nColumns; iOrder++) {
    const int iIndex = m_nColumnIndexByOrder[iOrder];
    if (bSetWidths)
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

  if (bSetWidths)
    AutoResizeColumns();
}

void DboxMain::SaveColumnWidths()
{
  // We need to save the current column widths
  // Zero it out first
  for (int iType = 0; iType < CItem::LAST_DATA; iType++) {
    m_nSaveColumnHeaderWidthByType[iType] = 0;
  }

  // Now save current widths according to type
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;

  for (int icol = 0; icol < m_LVHdrCtrl.GetItemCount(); icol++) {
    m_LVHdrCtrl.GetItem(icol, &hdi);
    m_nSaveColumnHeaderWidthByType[hdi.lParam] = m_ctlItemList.GetColumnWidth(icol);
  }
}

void DboxMain::RestoreColumnWidths()
{
  // Now put back the column widths!
  HDITEM hdi;
  hdi.mask = HDI_LPARAM;

  for (int icol = 0; icol < m_LVHdrCtrl.GetItemCount(); icol++) {
    m_LVHdrCtrl.GetItem(icol, &hdi);
    int iWidth = m_nSaveColumnHeaderWidthByType[hdi.lParam];

    // If not there previously (i.e. new column dragged in) use AutoSize
    if (iWidth == 0)
      iWidth = LVSCW_AUTOSIZE_USEHEADER;

    m_ctlItemList.SetColumnWidth(icol, iWidth);
  }
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
  for (int itype = 0; itype < CItem::LAST_DATA; itype++)
    m_nColumnIndexByType[itype] = -1;

  // Set default columns
  SetDefaultColumns();

  // Reset the column widths
  AutoResizeColumns();

  // Refresh the ListView
  RefreshViews(LISTONLY);

  // Reset Column Chooser dialog but only if already created
  if (m_pCC != NULL)
    SetupColumnChooser(false);
}

void DboxMain::AutoResizeColumns()
{
  int iIndex;
  // CHeaderCtrl get values
  for (int iOrder = 0; iOrder < m_nColumns; iOrder++) {
    iIndex = m_nColumnIndexByOrder[iOrder];
    int iType = m_nColumnTypeByIndex[iIndex];

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
  SaveColumnWidths();

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
  int iWidth, iSortColumn /* Not used here but needed for GetHeaderColumnProperties call */;

  for (i = CItem::LAST_DATA - 1; i >= 0; i--) {
    // Can't play with Title or User columns
    if (i == CItemData::TITLE || i == CItemData::USER)
      continue;

    if (m_nColumnIndexByType[i] == -1) {
      GetHeaderColumnProperties(i, cs_header, iWidth, iSortColumn);
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

void DboxMain::GetHeaderColumnProperties(const int &iType, CString &cs_Header, int &iWidth,
  int &iSortColumn)
{
  // ***
  //   REMEMBER TO ADD HERE IF THE FIELD IS GOING TO BE AVAILABLE IN LISTVIEW!!!
  //   It would be nice to use the compiler to tell us if anything is omitted but
  //   CIteData::FieldType enum has internal fields and unused gaps plus it would mean
  //   that arrays would be significantly bigger (CItemData::LAST_DATA [67] vs
  //   CItemData::LAST_FIELD [260].
  //   The one field outside CItemData::LAST_DATA is CItemData::ENTRYTYPE but this
  //   is shown by the image in the CItemData::UUID column if selected
  //   Lastly, "switch" is based on "int" not "CItemData::FieldType" for the compiler
  //   to complain!
  // ***

  cs_Header.Empty();
  iWidth = m_nColumnHeaderWidthByType[iType];
  UINT iID(0);
  switch (iType) {
    case CItemData::UUID:
      iID = IDS_ICON;
      break;
    case CItemData::GROUP:
      iID = IDS_GROUP;
      break;
    case CItemData::TITLE:
      iID = IDS_TITLE;
      break;
    case CItemData::USER:
      iID = IDS_USERNAME;
      break;
    case CItemData::PASSWORD:
      iID = IDS_PASSWORD;
      break;
    case CItemData::URL:
      iID = IDS_URL;
      break;
    case CItemData::AUTOTYPE:
      iID = IDS_AUTOTYPE;
      break;
    case CItemData::EMAIL:
      iID = IDS_EMAIL;
      break;
    case CItemData::SYMBOLS:
      iID = IDS_SYMBOLS;
      break;
    case CItemData::RUNCMD:
      iID = IDS_RUNCOMMAND;
      break;
    case CItemData::NOTES:
      iID = IDS_NOTES;
      break;
    case CItemData::CTIME:        
      iID = IDS_CREATED;
      iWidth = m_iDateTimeFieldWidth;
      break;
    case CItemData::PMTIME:
      iID = IDS_PASSWORDMODIFIED;
      iWidth = m_iDateTimeFieldWidth;
      break;
    case CItemData::ATIME:
      iID = IDS_LASTACCESSED;
      iWidth = m_iDateTimeFieldWidth;
      break;
    case CItemData::XTIME:
      iID = IDS_PASSWORDEXPIRYDATE;
      iWidth = m_iDateTimeFieldWidth;
      break;
    case CItemData::XTIME_INT:
      iID = IDS_PASSWORDEXPIRYDATEINT;
      break;
    case CItemData::RMTIME:
      iID = IDS_LASTMODIFIED;
      iWidth = m_iDateTimeFieldWidth;
      break;
    case CItemData::POLICY:        
      iID = IDS_PWPOLICY;
      break;
    case CItemData::POLICYNAME:        
      iID = IDS_POLICYNAME;
      break;
    case CItemData::PROTECTED:        
      iID = IDS_PROTECTED;
      break;
    case CItemData::KBSHORTCUT:        
      iID = IDS_KBSHORTCUT;
      break;
    case CItemData::ATTREF:
      iID = IDS_ATTREF;
      break;
    case CItemData::PWHIST:  // Not displayed in ListView
      break;
    default:
      // Not found, however as Title is a mandatory column - so can't go wrong!
      iSortColumn = CItemData::TITLE;
      break;
  }

  if (iID != 0)
    cs_Header.LoadString(iID);
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
  wcscat_s(datetime_str, 80, time_str);

  m_iDateTimeFieldWidth = m_ctlItemList.GetStringWidth(datetime_str) + 6;

  m_iheadermaxwidth = -1;
  CString cs_header;
  int iWidth, iSortColumn /* Not used here but needed for GetHeaderColumnProperties call */;

  for (int iType = 0; iType < CItem::LAST_DATA; iType++) {
    GetHeaderColumnProperties(iType, cs_header, iWidth, iSortColumn);
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
    IDS_RPTIMPORTKPV1CSV, IDS_RPTIMPORTKPV1TXT,
    IDS_RPTEXPORTTEXT, IDS_RPTEXPORTXML,
    IDS_RPTMERGE, IDS_RPTSYNCH, IDS_RPTVALIDATE,
  };

  for (int i = 0; i < sizeof(Reports) / sizeof(Reports[0]); i++) {
    csAction.LoadString(Reports[i]);
    cs_filename.Format(IDSC_REPORTFILENAME, static_cast<LPCWSTR>(cs_drive),
                       static_cast<LPCWSTR>(cs_directory),
                       static_cast<LPCWSTR>(csAction));
    if (::_tstat(cs_filename, &statbuf) == 0) {
      gmb.AddButton(Reports[i], csAction);
      bReportExists = true;
    }
  }

  if (!bReportExists) {
    CGeneralMsgBox gmbx;  // Note - this variable is NOT reusable after display!
    gmbx.AfxMessageBox(IDS_NOREPORTSEXIST);
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
    case IDS_RPTIMPORTKPV1CSV:
    case IDS_RPTIMPORTKPV1TXT:
    case IDS_RPTEXPORTTEXT:
    case IDS_RPTEXPORTXML:
    case IDS_RPTMERGE:
    case IDS_RPTSYNCH:
    case IDS_RPTVALIDATE:
      uistring = (UINT)rc;
      break;
    default:
      return;
  }
  csAction.LoadString(uistring);
  cs_filename.Format(IDSC_REPORTFILENAME, static_cast<LPCWSTR>(cs_drive),
                     static_cast<LPCWSTR>(cs_directory),
                     static_cast<LPCWSTR>(csAction));

  ViewReport(cs_filename);
  return;
}

static UINT SetupViewReports(const int nID)
{
  switch (nID) {
  case ID_MENUITEM_REPORT_COMPARE:
    return IDS_RPTCOMPARE;
  case ID_MENUITEM_REPORT_FIND:
    return IDS_RPTFIND;
  case ID_MENUITEM_REPORT_IMPORTTEXT:
    return IDS_RPTIMPORTTEXT;
  case ID_MENUITEM_REPORT_IMPORTXML:
    return IDS_RPTIMPORTXML;
  case ID_MENUITEM_REPORT_IMPORTKP1CSV:
    return IDS_RPTIMPORTKPV1CSV;
  case ID_MENUITEM_REPORT_IMPORTKP1TXT:
    return IDS_RPTIMPORTKPV1TXT;
  case ID_MENUITEM_REPORT_MERGE:
    return IDS_RPTMERGE;
  case ID_MENUITEM_REPORT_SYNCHRONIZE:
    return IDS_RPTSYNCH;
  case ID_MENUITEM_REPORT_EXPORTTEXT:
    return IDS_RPTEXPORTTEXT;
  case ID_MENUITEM_REPORT_EXPORTXML:
    return IDS_RPTEXPORTXML;
  case ID_MENUITEM_REPORT_EXPORTDB:
    return IDS_RPTEXPORTDB;
  case ID_MENUITEM_REPORT_VALIDATE:
    return IDS_RPTVALIDATE;
  default:
    pws_os::Trace(L"ID=%d\n", nID);
    ASSERT(0);
    return 0;
  }
}

void DboxMain::OnViewReportsByID(UINT nID)
{
  CString cs_filename, cs_path, csAction;
  CString cs_drive, cs_directory;

  if (!GetDriveAndDirectory(m_core.GetCurFile(), cs_drive, cs_directory))
    return;

  csAction.LoadString(SetupViewReports(nID));
  cs_filename.Format(IDSC_REPORTFILENAME, static_cast<LPCWSTR>(cs_drive),
                     static_cast<LPCWSTR>(cs_directory),
                     static_cast<LPCWSTR>(csAction));

  ViewReport(cs_filename);
}

void DboxMain::ViewReport(CReport &rpt) const
{
  CViewReport vr_dlg(const_cast<DboxMain*>(this), &rpt);

  vr_dlg.DoModal();
}

void DboxMain::ViewReport(const CString &cs_ReportFileName) const
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
  cs_CommandLine.Format(L"\"%s\" \"%s\"", static_cast<LPCWSTR>(szExecName), 
                        static_cast<LPCWSTR>(cs_ReportFileName));
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

  csAction.LoadString(SetupViewReports(nID));
  cs_filename.Format(IDSC_REPORTFILENAME, static_cast<LPCWSTR>(cs_drive),
                     static_cast<LPCWSTR>(cs_directory),
                     static_cast<LPCWSTR>(csAction));

  struct _stat statbuf;

  // Only allow selection if file exists!
  int status = ::_tstat(cs_filename, &statbuf);
  return (status != 0) ? FALSE : TRUE;
}

void DboxMain::OnRefreshWindow()
{
  PWS_LOGIT;

  pws_os::CUUID entry_uuid, tree_find_entry_uuid, list_find_entry_uuid;
  StringX sxGroupPath;
 
  // Save selected/highlighted entry
  GetSelectedItems(entry_uuid, tree_find_entry_uuid, list_find_entry_uuid, sxGroupPath);

  // Stop Find Toolbar during refresh
  BOOL bFindVisible = m_FindToolBar.IsWindowVisible();
  if (bFindVisible)
    OnHideFindToolbar();

  // Useful for users if they are using a filter and have edited an entry
  // so it no longer passes
  RefreshViews();

  // Try and put selection back - positions in lists may have changed
  ReSelectItems(entry_uuid, tree_find_entry_uuid, list_find_entry_uuid, sxGroupPath);

  // Put Find Toolbar back if originally present
  if (bFindVisible)
    OnShowFindToolbar();
}

void DboxMain::OnCustomizeToolbar()
{
  CToolBarCtrl& mainTBCtrl = m_MainToolBar.GetToolBarCtrl();

  // Before we customise the Toolbar, to prevent multiple "Edit entry" buttons
  // change back View to Edit (if changed)
  int nIndex = m_MainToolBar.CommandToIndex(ID_MENUITEM_VIEWENTRY);
  if (nIndex != -1) {
    mainTBCtrl.SetCmdID(nIndex, ID_MENUITEM_EDITENTRY);
  }

  mainTBCtrl.Customize();

  // Now save user's Toolbar preference
  StringX cs_temp = LPCWSTR(m_MainToolBar.GetButtonString());
  PWSprefs::GetInstance()->SetPref(PWSprefs::MainToolBarButtons, cs_temp);

  // Ensure toolbar buttons reflect current state e.g. no DB open or R-O or R/W status if open
  if (!m_bOpen) {
    UpdateMenuAndToolBar(false);
  } else {
    // Update toolbar per R/W status
    UpdateToolBarROStatus(m_core.IsReadOnly());

    // Update toolbar per selected item
    CItemData *pci = getSelectedItem();
    UpdateToolBarForSelectedItem(pci);
  }
}

void DboxMain::OnShowFindToolbar()
{
  // Show Find Toolbar - only if the find entries filter not active
  if (!(m_bFilterActive && m_bFindFilterDisplayed))
    SetFindToolBar(true);
}

void DboxMain::OnHideFindToolbar()
{
  SetFindToolBar(false);

  // Select the last found item on closing the FindToolbar (either by pressing
  // close or via Esc key if not used to minimize application).
  if (m_ctlItemList.IsWindowVisible() && m_LastFoundListItem != -1) {
    m_ctlItemList.SetFocus();
    m_ctlItemList.SetItemState(m_LastFoundListItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
  } else
  if (m_ctlItemTree.IsWindowVisible() && m_LastFoundTreeItem != NULL) {
    m_ctlItemTree.SetFocus();
    m_ctlItemTree.Select(m_LastFoundTreeItem, TVGN_CARET);
  }

  m_bFindBarShown = false;
}

void DboxMain::SetFindToolBar(bool bShow)
{
  if (m_FindToolBar.GetSafeHwnd() == NULL)
    return;

  SetToolBarPositions();

  if (!m_FindToolBar.IsWindowVisible() && !bShow)
    return;  // Nothing to do if not visible

  m_FindToolBar.ShowFindToolBar(bShow);
  SetToolBarPositions();
}

void DboxMain::SetToolBarPositions()
{
  if (m_FindToolBar.GetSafeHwnd() == NULL)
    return;

  // We mustn't do this if a Wizard dialog is open
  if (m_bWizardActive)
    return;

  CRect rect, dragrect;
  RepositionBars(AFX_IDW_TOOLBAR, AFX_IDW_CONTROLBAR_LAST, 0);
  RepositionBars(AFX_IDW_TOOLBAR, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, &rect);
  bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
  CDDStatic *DDs[] = { &m_DDGroup, &m_DDTitle, &m_DDUser,
                       &m_DDPassword, &m_DDNotes, &m_DDURL, &m_DDemail,
                       &m_DDAutotype};
  if (bDragBarState) {
    // Get the image states just incase another entry selected
    // since last shown
    CItemData *pci_entry = GetLastSelected(), *pbci(NULL);

    if (pci_entry == NULL) {
      m_DDGroup.SetStaticState(m_core.GetNumEntries() != 0);
      m_DDTitle.SetStaticState(false);
      m_DDPassword.SetStaticState(false);
      m_DDUser.SetStaticState(false);
      m_DDNotes.SetStaticState(false);
      m_DDURL.SetStaticState(false);
      m_DDemail.SetStaticState(false);
      m_DDAutotype.SetStaticState(false);
    } else {
      if (pci_entry->IsDependent()) {
        pbci = GetBaseEntry(pci_entry);
      }
      m_DDGroup.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::GROUP, pbci));
      m_DDTitle.SetStaticState(true);
      m_DDPassword.SetStaticState(true);
      m_DDUser.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::USER, pbci));
      m_DDNotes.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::NOTES, pbci));
      m_DDURL.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::URL, pbci));
      m_DDemail.SetStaticState(!pci_entry->IsFieldValueEmpty(CItemData::EMAIL, pbci));
      m_DDAutotype.SetStaticState(true);
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
    ASSERT(m_FindToolBar.GetParent() == m_StatusBar.GetParent());

    CRect ftb_rect, stb_rect;
    m_FindToolBar.GetWindowRect(&ftb_rect);
    m_StatusBar.GetWindowRect(&stb_rect);

    if (ftb_rect.top > stb_rect.top) {
      // FindToolBar is "below" the StatusBar
      ScreenToClient(&ftb_rect);
      ScreenToClient(&stb_rect);
      // Move FindToolBar up by the height of the Statusbar
      m_FindToolBar.MoveWindow(ftb_rect.left, ftb_rect.top - stb_rect.Height(),
                               ftb_rect.Width(), ftb_rect.Height());
      // Move Statusbar down by the height of the FindToolBar
      m_StatusBar.MoveWindow(stb_rect.left, stb_rect.top + ftb_rect.Height(),
                             stb_rect.Width(), stb_rect.Height());
      m_FindToolBar.Invalidate();
      m_StatusBar.Invalidate();
    }
  }

  m_ctlItemList.Invalidate();
  m_ctlItemTree.Invalidate();
}

void DboxMain::OnToolBarClearFind()
{
  m_FindToolBar.ClearFind();
  if (m_bFilterActive && m_bFindFilterDisplayed) {
    OnShowFoundEntries();
  }
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
  CString csFindString;

  m_FindToolBar.GetSearchText(csFindString);
  if (csFindString.IsEmpty())
    return;

  std::vector<int> vIndices = m_FindToolBar.GetSearchResults();
  CString buffer, cs_temp;
  CReport rpt;
  cs_temp.LoadString(IDS_RPTFIND);
  rpt.StartReport(cs_temp, m_core.GetCurFile().c_str());

  CItemData::FieldBits bsFFields;
  CItemAtt::AttFieldBits bsAttFFields;
  bool bFAdvanced;
  std::wstring Fsubgroup_name;
  int Fsubgroup_object, Fsubgroup_function;
  bool Fsubgroup_set;

  m_FindToolBar.GetSearchInfo(bFAdvanced, bsFFields, bsAttFFields, Fsubgroup_name,
                              Fsubgroup_set, Fsubgroup_object, Fsubgroup_function);

  // tell the user we're done & provide short Compare report
  if (!bFAdvanced) {
    cs_temp.LoadString(IDS_NONE);
    buffer.Format(IDS_ADVANCEDOPTIONS, static_cast<LPCWSTR>(cs_temp));
    rpt.WriteLine((LPCWSTR)buffer);
    rpt.WriteLine();
  } else {
    if (!Fsubgroup_set) {
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
        case CItemData::SYMBOLS:
          uistring = IDS_SYMBOLS;
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

      uistring = PWSMatch::GetRule(PWSMatch::MatchRule(abs(Fsubgroup_function)));

      cs_text.LoadString(uistring);
      cs_temp.Format(IDS_ADVANCEDSUBSET, static_cast<LPCWSTR>(cs_Object),
                     static_cast<LPCWSTR>(cs_text),
                     static_cast<LPCWSTR>(Fsubgroup_name.c_str()),
                     static_cast<LPCWSTR>(cs_case));
    }
    buffer.Format(IDS_ADVANCEDOPTIONS, static_cast<LPCWSTR>(cs_temp));
    rpt.WriteLine((LPCWSTR)buffer);
    rpt.WriteLine();

    cs_temp.LoadString(IDS_RPTFIND);
    buffer.Format(IDS_ADVANCEDFIELDS, static_cast<LPCWSTR>(cs_temp));
    rpt.WriteLine((LPCWSTR)buffer);

    buffer = L"\t";
    // Non-time fields
    const struct {int si; int ii;} nonTimeTextIfSet [] = {
      {CItemData::GROUP, IDS_FINDGROUP},
      {CItemData::TITLE, IDS_FINDTITLE},
      {CItemData::USER, IDS_FINDUSER},
      {CItemData::PASSWORD, IDS_COMPPASSWORD},
      {CItemData::NOTES, IDS_COMPNOTES},
      {CItemData::URL, IDS_COMPURL},
      {CItemData::EMAIL, IDS_COMPEMAIL},
      {CItemData::PROTECTED, IDS_COMPPROTECTED},
      {CItemData::SYMBOLS, IDS_COMPSYMBOLS},
      {CItemData::RUNCMD, IDS_COMPRUNCOMMAND},
      {CItemData::AUTOTYPE, IDS_COMPAUTOTYPE},
      {CItemData::PWHIST, IDS_COMPPWHISTORY},
      {CItemData::POLICYNAME, IDS_COMPPOLICYNAME},
      {CItemData::KBSHORTCUT, IDS_COMPKBSHORTCUT},
      {CItemData::ATTREF, IDS_COMPATTREF},
    };

    for (auto &elem : nonTimeTextIfSet) {
      if (bsFFields.test(elem.si))
        buffer += L"\t" + CString(MAKEINTRESOURCE(elem.ii));
    }

    if (bsAttFFields.test(CItemAtt::FILENAME - CItemAtt::START))
      buffer += L"\t" + CString(MAKEINTRESOURCE(IDS_FILENAME));

    rpt.WriteLine((LPCWSTR)buffer);
    rpt.WriteLine();
  }

  if (vIndices.empty()) {
    buffer.Format(IDS_SEARCHRESULTS1, static_cast<LPCWSTR>(csFindString));
    rpt.WriteLine((LPCWSTR)buffer);
  } else {
    buffer.Format(IDS_SEARCHRESULTS2, static_cast<LPCWSTR>(csFindString));
    rpt.WriteLine((LPCWSTR)buffer);

    for (size_t i = 0; i < vIndices.size(); i++) {
      int index = vIndices[i];
      CItemData *pci = (CItemData *)m_ctlItemList.GetItemData(index);
      buffer.Format(IDS_COMPARESTATS, static_cast<LPCWSTR>(pci->GetGroup().c_str()),
                    static_cast<LPCWSTR>(pci->GetTitle().c_str()),
                    static_cast<LPCWSTR>(pci->GetUser().c_str()));
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

int DboxMain::GetEntryImage(const CItemData &ci) const
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

  time_t tttXTime;
  ci.GetXTime(tttXTime);
  if (tttXTime > time_t(0) && tttXTime <= time_t(3650)) {
    time_t tttCPMTime;
    ci.GetPMTime(tttCPMTime);
    if ((long)tttCPMTime == 0L)
      ci.GetCTime(tttCPMTime);
    tttXTime = (time_t)((long)tttCPMTime + (long)tttXTime * 86400);
  }

  if (tttXTime != 0) {
    time_t now, warnexptime((time_t)0);
    time(&now);
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn)) {
      int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
      struct tm st;
      errno_t err;
      err = localtime_s(&st, &now);  // secure version
      ASSERT(err == 0);
      st.tm_mday += idays;
      warnexptime = mktime(&st);

      if (warnexptime == (time_t)-1)
        warnexptime = (time_t)0;
    }
    if (tttXTime <= now) {
      nImage += 2;  // Expired
    } else if (tttXTime < warnexptime) {
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
  int icurrentImage, icurrentSelectedImage;

  m_ctlItemTree.GetItemImage(ti, icurrentImage, icurrentSelectedImage);

  if (icurrentImage != nImage) {
    m_ctlItemTree.SetItemImage(ti, nImage, nImage);

    if (bOneEntry) {
      CRect rect;
      m_ctlItemTree.GetItemRect(ti, &rect, FALSE);
      m_ctlItemTree.InvalidateRect(&rect);
    }
  }
}

void DboxMain::UpdateEntryImages(const CItemData &ci, bool bAllowFail)
{
  DisplayInfo *pdi = GetEntryGUIInfo(ci, bAllowFail);
  if (pdi == NULL)
    return;

  if (ci.GetStatus() != CItemData::ES_DELETED) {
    int nImage = GetEntryImage(ci);
    SetEntryImage(pdi->list_index, nImage, true);
    SetEntryImage(pdi->tree_item, nImage, true);
  } else { // deleted item, remove from display
    m_ctlItemList.DeleteItem(pdi->list_index);
    m_ctlItemTree.DeleteItem(pdi->tree_item);
    VERIFY(m_MapEntryToGUI.erase(ci.GetUUID()) == 1);
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

bool DboxMain::SetNotesWindow(const CPoint ptClient, const bool bVisible)
{
/*
 *  Use of CInfoDisplay to replace MS's broken ToolTips support.
 *  Based on CInfoDisplay class taken from Asynch Explorer by
 *  Joseph M. Newcomer [MVP]; http://www.flounder.com
 *  Additional enhancements to the use of this code have been made to 
 *  allow for delayed showing of the display and for a limited period.
 */

  const CItemData *pci(NULL);
  CPoint ptScreen(ptClient);
  StringX sx_notes(L"");
  UINT nFlags;
  HTREEITEM hItem(NULL);
  int nItem(-1);

  if (m_pNotesDisplay == NULL)
    return false;

  if (!bVisible) {
    m_pNotesDisplay->SetWindowText(sx_notes.c_str());
    m_pNotesDisplay->ShowWindow(SW_HIDE);
    return false;
  }

  if (m_ctlItemTree.IsWindowVisible()) {
    m_ctlItemTree.ClientToScreen(&ptScreen);
    hItem = m_ctlItemTree.HitTest(ptClient, &nFlags);
    if (hItem != NULL &&
        (nFlags & (TVHT_ONITEM | TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT))) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
    }
  } else {
    m_ctlItemList.ClientToScreen(&ptScreen);
    nItem = m_ctlItemList.HitTest(ptClient, &nFlags);
    if (nItem >= 0) {
      pci = (CItemData *)m_ctlItemList.GetItemData(nItem);
    }
  }
  ptScreen.y += ::GetSystemMetrics(SM_CYCURSOR) / 2; // half-height of cursor

  if (pci != NULL) {
    if (pci->IsShortcut())
      pci = GetBaseEntry(pci);
    sx_notes = pci->GetNotes();
  }

  if (!sx_notes.empty()) {
    Replace(sx_notes, StringX(L"\r\n"), StringX(L"\n"));
    Remove(sx_notes, L'\r');

    if (sx_notes.length() > 256)
      sx_notes = sx_notes.substr(0, 250) + L"[...]";
  }

  // move window
  CSecString cs_oldnotes;
  m_pNotesDisplay->GetWindowText(cs_oldnotes);
  if (LPCWSTR(cs_oldnotes) != sx_notes)
    m_pNotesDisplay->SetWindowText(sx_notes.c_str());

  m_pNotesDisplay->SetWindowPos(NULL, ptScreen.x, ptScreen.y, 0, 0,
                                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  m_pNotesDisplay->ShowWindow(!sx_notes.empty() ? SW_SHOWNA : SW_HIDE);

  return !sx_notes.empty();
}

void DboxMain::UpdateNotesTooltipFont()
{
  CFont *pNotes = Fonts::GetInstance()->GetNotesFont();
  m_pNotesDisplay->SendMessage(WM_SETFONT, (WPARAM)pNotes, 1);
}

CItemData *DboxMain::GetLastSelected() const
{
  CItemData *pci(NULL);
  if (m_core.GetNumEntries() == 0)
    return pci;

  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hSelected = m_ctlItemTree.GetSelectedItem();
    if (hSelected != NULL)
      pci = (CItemData *)m_ctlItemTree.GetItemData(hSelected);
  } else {
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos != NULL) {
      int i = m_ctlItemList.GetNextSelectedItem(pos);
      pci = (CItemData *)m_ctlItemList.GetItemData(i);
    }
  }
  return pci;
}

StringX DboxMain::GetGroupName(const bool bFullPath) const
{
  HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
  StringX s(L"");
  if (hi != NULL)
    s = m_ctlItemTree.GetItemText(hi);

  if (bFullPath || (GetKeyState(VK_CONTROL) & 0x8000) == 0) {
    while ((hi = m_ctlItemTree.GetParentItem(hi)) != NULL) {
      s = StringX(m_ctlItemTree.GetItemText(hi)) + StringX(L".") + s;
    }
  }
  return s;
}

void DboxMain::UpdateGroupNamesInMap(const StringX sxOldPath, const StringX sxNewPath)
{
  // When a group node is renamed, need to update the group to HTREEITEM map
  // We need to build a new map, as we can't erase & add while iterating.
  std::map<StringX, HTREEITEM> new_mapGroupToTreeItem;
  std::map<HTREEITEM, StringX> new_mapTreeItemToGroup;

  size_t len = sxOldPath.length();
  std::map<StringX, HTREEITEM>::iterator iter;

  for (iter = m_mapGroupToTreeItem.begin(); 
       iter != m_mapGroupToTreeItem.end(); iter++) {
    // Only change if path to root is the same
    if (iter->first.length() == len) {
      if (wcsncmp(sxOldPath.c_str(), iter->first.c_str(), len) == 0) {
        HTREEITEM ti = iter->second;
        new_mapGroupToTreeItem.insert(make_pair(sxNewPath, ti));
        continue;
      }
    } 
    else if ((iter->first.length() > len+1) && (iter->first[len+1] != GROUP_SEP)) {
      // Need to add group separator and check that next symbol is not a dot
      // to ensure not affecting another group
      // (group name could contain trailing dots, for example abc..def.g)
      // subgroup name will have len > len+1 (old_name + dot + subgroup_name)
      StringX path = sxOldPath + StringX(GROUP_SEP2);
      if (wcsncmp(path.c_str(), iter->first.c_str(), len + 1) == 0) {
        HTREEITEM ti = iter->second;
        StringX sxNewGroup = sxNewPath + iter->first.substr(len);
        new_mapGroupToTreeItem.insert(make_pair(sxNewGroup, ti));
        continue;
      }
    }
    new_mapGroupToTreeItem.insert(*iter);
  } // for

  ASSERT(new_mapGroupToTreeItem.size() == m_mapGroupToTreeItem.size());
  m_mapGroupToTreeItem = new_mapGroupToTreeItem;

  // Now recreate reverse look
  m_mapTreeItemToGroup.clear();
  for (iter = m_mapGroupToTreeItem.begin();
    iter != m_mapGroupToTreeItem.end(); iter++) {
    m_mapTreeItemToGroup.insert(make_pair(iter->second, iter->first));
  }
}

void DboxMain::OnShowUnsavedEntries()
{
  if (m_bUnsavedDisplayed) {
    // If currently active - clear it
    m_bFilterActive = m_bUnsavedDisplayed = false;
    CurrentFilter().Empty();
  } else {
    // If another internal filter currently active - clear it
    m_FilterManager.SetFindFilter(false);
    m_FindToolBar.InvalidateSearch();
    m_bExpireDisplayed = m_bFindFilterDisplayed = false;

    // Now set this filter
    m_bFilterActive = m_bUnsavedDisplayed = true;
    CurrentFilter() = m_FilterManager.GetUnsavedFilter();
  }

  ApplyFilters();

  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER,
    (m_bUnsavedDisplayed || !m_bFilterActive) ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_EDITFILTER,
    m_bUnsavedDisplayed ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_MANAGEFILTERS,
    m_bUnsavedDisplayed ? FALSE : TRUE);
}

void DboxMain::OnShowExpireList()
{
  if (m_bExpireDisplayed) {
    // If currently active - clear it
    m_bFilterActive = m_bExpireDisplayed = false;
    CurrentFilter().Empty();
  } else {
    // If another internal filter currently active - clear it
    m_FilterManager.SetFindFilter(false);
    m_FindToolBar.InvalidateSearch();
    m_bUnsavedDisplayed = m_bFindFilterDisplayed = false;

    // Now set this filter
    m_bFilterActive = m_bExpireDisplayed = true;
    CurrentFilter() = m_FilterManager.GetExpireFilter();
  }

  ApplyFilters();

  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER,
    (m_bExpireDisplayed || !m_bFilterActive) ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_EDITFILTER,
    m_bExpireDisplayed ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_MANAGEFILTERS,
    m_bExpireDisplayed ? FALSE : TRUE);
}

void DboxMain::OnShowFoundEntries() 
{
  if (m_bFindFilterDisplayed) {
    // If currently active - clear it
    m_bFilterActive = m_bFindFilterDisplayed = false;
    m_FilterManager.SetFindFilter(false);
    CurrentFilter().Empty();
  } else {
    // If another internal filter currently active - clear it
    m_bUnsavedDisplayed = m_bExpireDisplayed = false;

    // Hide Find toolbar
    OnHideFindToolbar();

    // Now set this filter
    m_bFilterActive = m_bFindFilterDisplayed = true;
    m_FilterManager.SetFindFilter(true);
    CurrentFilter() = m_FilterManager.GetFoundFilter();
  }

  // If a Find is active, save its status
  pws_os::CUUID entry_uuid;
  m_iCurrentItemFound = m_FindToolBar.GetLastSelectedFoundItem(entry_uuid);

  UnFindItem();

  ApplyFilters();

  // As ApplyFilters will redo the Find resetting the selected item to the first
  // Put it back now
  if (m_iCurrentItemFound != -1) {
    m_FindToolBar.Find(m_iCurrentItemFound);
  }

  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_APPLYFILTER,
    (m_bFindFilterDisplayed || !m_bFilterActive) ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_EDITFILTER,
    m_bFindFilterDisplayed ? FALSE : TRUE);
  m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_MANAGEFILTERS,
    m_bFindFilterDisplayed ? FALSE : TRUE);
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
  int newpos = InsertItemIntoGUITreeList(m_core.GetEntry(m_core.Find(ci.GetUUID())));

  if (newpos >= 0) {
    SelectEntry(newpos);
    FixListIndexes();
  }

  RefreshViews(BOTHVIEWS);
}

void DboxMain::RemoveFromGUI(CItemData &ci)
{
  // RemoveFromGUI should always occur BEFORE the entry is deleted!
  // Note: Also called if a filter is active and an entry is changed and no longer
  // satisfies the filter criteria.
  ItemListIter iter = m_core.Find(ci.GetUUID());
  if (iter == End()) {
    ASSERT(0);
    return;
  }

  DisplayInfo *pdi = GetEntryGUIInfo(ci, true);

  if (pdi != NULL) {
    ASSERT(GetEntryGUIInfo(iter->second)->list_index == pdi->list_index &&
           GetEntryGUIInfo(iter->second)->tree_item == pdi->tree_item);

    HTREEITEM hItem = m_ctlItemTree.GetNextItem(pdi->tree_item,
                            TVGN_PREVIOUSVISIBLE);
    m_ctlItemTree.SelectItem(hItem);


    m_ctlItemList.DeleteItem(pdi->list_index);
    m_ctlItemTree.DeleteWithParents(pdi->tree_item);

    // Remove if saved as last selected item
    if (m_LastFoundTreeItem == pdi->tree_item) {
      m_LastFoundTreeItem = NULL;
    }

    if (m_LastFoundListItem == pdi->list_index) {
      m_LastFoundListItem = -1;
    }

    // Now remove from map
    VERIFY(m_MapEntryToGUI.erase(ci.GetUUID()) == 1);

    FixListIndexes(); // sucks, as make M deletions an NxM operation
    m_ctlItemList.Invalidate();
    m_ctlItemTree.Invalidate();
  }
}

void DboxMain::RefreshEntryPasswordInGUI(CItemData &ci)
{
  // For when Entry's password + PW history has been updated
  DisplayInfo *pdi = GetEntryGUIInfo(ci);

  UpdateListItemField(pdi->list_index, CItemData::PWHIST, ci.GetPWHistory());
  RefreshEntryFieldInGUI(ci, CItemData::PASSWORD);
}

void DboxMain::RefreshEntryFieldInGUI(CItemData &ci, CItemData::FieldType ft)
{
  // For when any field is updated
  DisplayInfo *pdi = GetEntryGUIInfo(ci);

  StringX sx_fielddata;
  time_t t;

  switch (ft) {
    case CItemData::NOTES:
    {
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
      sx_fielddata = line1;
      break;
    }
    case CItemData::CTIME:
      sx_fielddata = ci.GetCTimeL();
      break;
    case CItemData::PMTIME:
      ci.GetPMTime(t);
      sx_fielddata = ((long)t == 0) ? ci.GetCTimeL() : ci.GetPMTimeL();
      break;
    case CItemData::RMTIME:
      ci.GetRMTime(t);
      sx_fielddata = ((long)t == 0) ? ci.GetCTimeL() : ci.GetRMTimeL();
      break;
    case CItemData::POLICY:
    {
      PWPolicy pwp;
      ci.GetPWPolicy(pwp);
      sx_fielddata = pwp.GetDisplayString();
      break;
    }
    default:
      sx_fielddata = ci.GetFieldValue(ft);
  }

  UpdateListItemField(pdi->list_index, ft, sx_fielddata);

  if (ft == CItemData::GROUP || m_bFilterActive) {
    RefreshViews();
  } else {
    PWSprefs *prefs = PWSprefs::GetInstance();
    bool bShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
    bool bShowPasswordInTree = prefs->GetPref(PWSprefs::ShowPasswordInTree);
    if ( ft == CItemData::START || ft == CItemData::TITLE || 
        (ft == CItemData::USER && bShowUsernameInTree) ||
        (ft == CItemData::PASSWORD && bShowPasswordInTree) ||
         ft == CItemData::PROTECTED) {
      UpdateTreeItem(pdi->tree_item, ci);
      if (ft == CItemData::PASSWORD && bShowPasswordInTree) {
        UpdateEntryImages(ci);
      }
    }
  }
}

void DboxMain::RebuildGUI(const ViewType iView)
{
  // Don't redraw if Find filter active as it will be drawn later
  // Hopefully stop multiple refresh of the GUI
  RefreshViews(iView);
}

void DboxMain::SaveGUIStatusEx(const ViewType iView)
{
  PWS_LOGIT_ARGS("iView=%d", iView);

  if (m_bInRefresh || m_bInRestoreWindows)
    return;

  if (!m_bOpen || m_TrayLockedState != UNLOCKED || IsIconic())
    return;

  if (m_core.GetNumEntries() == 0 && m_core.GetEmptyGroups().empty())
    return;

  if ((m_ctlItemList.IsWindowVisible() && m_ctlItemList.GetItemCount() == 0) ||
      (m_ctlItemTree.IsWindowVisible() && m_ctlItemTree.GetCount() == 0))
    return;

  CItemData *pci(NULL);
  POSITION pos;
  HTREEITEM ti;

  // Note: User can have different entries selected/visible in Tree & List Views
  if ((iView & LISTONLY) == LISTONLY && m_ctlItemList.GetItemCount() > 0) {
    m_LUUIDSelectedAtMinimize = CUUID::NullUUID();
    m_LUUIDVisibleAtMinimize = CUUID::NullUUID();

    // List view
    // Get selected entry in CListCtrl
    pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos) {
      int i = m_ctlItemList.GetNextSelectedItem(pos);
      pci = (CItemData *)m_ctlItemList.GetItemData(i);
      ASSERT(pci != NULL);  // No groups in List View
      ASSERT(GetEntryGUIInfo(*pci, true) != NULL && GetEntryGUIInfo(*pci, true)->list_index == i);
      m_LUUIDSelectedAtMinimize = pci->GetUUID();
    } // pos != 0

    // Get first entry visible in CListCtrl
    int i = m_ctlItemList.GetTopIndex();

    // Note GetTopIndex can give an invalid value
    // if the List view has never been visible - check within limits
    if (i >= 0 && i < m_ctlItemList.GetItemCount()) {
      pci = (CItemData *)m_ctlItemList.GetItemData(i);
      ASSERT(pci != NULL);  // No groups in List View
      ASSERT(GetEntryGUIInfo(*pci, true) != NULL && GetEntryGUIInfo(*pci, true)->list_index == i);
      m_LUUIDVisibleAtMinimize = pci->GetUUID();
    } // i >= 0
  }

  if ((iView & TREEONLY) == TREEONLY && m_ctlItemTree.GetCount() > 0) {
    // Save expand/collapse status of groups
    m_vGroupDisplayState = GetGroupDisplayState();

    m_TUUIDSelectedAtMinimize = CUUID::NullUUID();
    m_TUUIDVisibleAtMinimize = CUUID::NullUUID();

    m_sxSelectedGroup = L"";
    m_sxVisibleGroup = L"";

    // Tree view
    // Get selected entry in CTreeCtrl
    ti = m_ctlItemTree.GetSelectedItem();
    if (ti != NULL) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (pci != NULL) {
        // Entry: do some sanity tests
        DisplayInfo *pdi = GetEntryGUIInfo(*pci);
        if (pdi->tree_item != ti) {
          pws_os::Trace(L"DboxMain::SaveGUIStatusEx: fixing pdi->tree_item!\n");
          pdi->tree_item = ti;
        }
        m_TUUIDSelectedAtMinimize = pci->GetUUID();
      } else {
        // Group: save entry text
        m_sxSelectedGroup = m_mapTreeItemToGroup[ti];
      }
    } // ti != NULL

    // Get first entry visible in CTreeCtrl
    ti = m_ctlItemTree.GetFirstVisibleItem();
    if (ti != NULL) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
      if (pci != NULL) {
        // Entry: do some sanity tests
        DisplayInfo *pdi = GetEntryGUIInfo(*pci);
        if (pdi->tree_item != ti) {
          pws_os::Trace(L"DboxMain::SaveGUIStatusEx: fixing pdi->tree_item!\n");
          pdi->tree_item = ti;
        }
        m_TUUIDVisibleAtMinimize = pci->GetUUID();
      } else {
        // Group: save entry text
        m_sxVisibleGroup = m_mapTreeItemToGroup[ti];
      }
    } // ti != NULL
  }
}

void DboxMain::RestoreGUIStatusEx()
{
  PWS_LOGIT;

  if (m_core.GetNumEntries() == 0 && m_core.GetEmptyGroups().empty())
    return;

  if ((m_ctlItemList.IsWindowVisible() && m_ctlItemList.GetItemCount() == 0) || 
      (m_ctlItemTree.IsWindowVisible() && m_ctlItemTree.GetCount() == 0))
    return;

  m_bInRestoreWindows = true;

  // Restore expand/collapse status of groups
  m_bIsRestoring = true;
  m_ctlItemTree.SetRestoreMode(true);
  if (!m_vGroupDisplayState.empty()) {
    SetGroupDisplayState(m_vGroupDisplayState);
  }
  m_ctlItemTree.SetRestoreMode(false);
  m_bIsRestoring = false;

  HTREEITEM htvis(NULL), htsel(NULL);
  CItemData *pci(NULL);

  // Process Tree - Selected
  if (m_TUUIDSelectedAtMinimize != CUUID::NullUUID()) {
    // Entry selected
    ItemListIter iter = Find(m_TUUIDSelectedAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = GetEntryGUIInfo(iter->second);
      if (pdi != NULL) {
        htsel = pdi->tree_item;
        pci = &iter->second;
      }
    }
  } else {
    // Group selected
    if (!m_sxSelectedGroup.empty()) {
      // Find corresponding tree item
      // The issue here is that we might have just performed a group rename
      // (execute, redo or undo) and so the selected group is no longer
      // in the map and can't be selected.
      // QUestion is: how to save this for reselection afterwards or
      // ho to get new group name after rename execute/undo/redo.
      std::map<StringX, HTREEITEM>::iterator iter;
      iter = m_mapGroupToTreeItem.find(m_sxSelectedGroup);
      if (iter != m_mapGroupToTreeItem.end()) {
        htsel = iter->second;
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
  if (m_TUUIDVisibleAtMinimize != CUUID::NullUUID()) {
    // Entry topmost visible
    ItemListIter iter = Find(m_TUUIDVisibleAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = GetEntryGUIInfo(iter->second);
      if (pdi != NULL) {
        htvis = pdi->tree_item;
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
      }
    }
  }

  // Just in case MFC actually selected the first visible entry
  if (htvis != NULL) {
    m_ctlItemTree.Select(htvis, TVGN_FIRSTVISIBLE);
    RECT rect;
    m_ctlItemTree.GetItemRect(htvis, &rect, FALSE);
    m_ctlItemTree.InvalidateRect(&rect, TRUE);
  }

  // Process List - selected
  if (m_LUUIDSelectedAtMinimize != CUUID::NullUUID()) {
    ItemListIter iter = Find(m_LUUIDSelectedAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = GetEntryGUIInfo(iter->second);
      if (pdi != NULL) {
        // Select the Entry
        m_ctlItemList.SetItemState(pdi->list_index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
        m_ctlItemList.Update(pdi->list_index);
        if (m_ctlItemList.IsWindowVisible())
          UpdateToolBarForSelectedItem(&iter->second);
      }
    }
  }

  // Process List - visible
  if (m_LUUIDVisibleAtMinimize != CUUID::NullUUID()) {
    ItemListIter iter = Find(m_LUUIDVisibleAtMinimize);
    if (iter != End()) {
      DisplayInfo *pdi = GetEntryGUIInfo(iter->second);
      if (pdi != NULL) {
        // There is CListCtrl::GetTopIndex but No CListCtrl::SetTopIndex - Grrrrr!
        CRect indexRect, topRect;
        m_ctlItemList.EnsureVisible(pdi->list_index, FALSE);
        int icurtop = m_ctlItemList.GetTopIndex();
        m_ctlItemList.GetItemRect(pdi->list_index, indexRect, LVIR_BOUNDS);
        m_ctlItemList.GetItemRect(icurtop, topRect, LVIR_BOUNDS);
        m_ctlItemList.Scroll(CSize(0, (indexRect.top - topRect.top) * indexRect.Height()));

        // Just in case MFC actually selected the first visible entry
        m_ctlItemList.SetItemState(pdi->list_index, 0, LVIS_FOCUSED | LVIS_SELECTED);
        m_ctlItemList.Update(pdi->list_index);
      }
    }
  }
  m_bInRestoreWindows = false;
}

void DboxMain::SaveGroupDisplayState(const bool bClear)
{
  PWS_LOGIT_ARGS("bClear=%ls", bClear ? L"true" : L"false");

  vector <bool> v;
  if (!bClear)
      v = GetGroupDisplayState(); // update or clear it

  m_core.SetDisplayStatus(v); // store it
}

void DboxMain::RestoreGroupDisplayState()
{
  PWS_LOGIT;

  const vector<bool> &displaystatus = m_core.GetDisplayStatus();    

  if (!displaystatus.empty())
    SetGroupDisplayState(displaystatus);
}

vector<bool> DboxMain::GetGroupDisplayState()
{
  PWS_LOGIT;

  HTREEITEM hItem = NULL;
  vector<bool> v;

  if (m_ctlItemTree.GetSafeHwnd() == NULL)
    return v;

  while (NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem))) {
    StringX sxPath = m_mapTreeItemToGroup[hItem];
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      bool bState = (m_ctlItemTree.GetItemState(hItem, TVIS_EXPANDED) &
                             TVIS_EXPANDED) != 0;
      v.push_back(bState);
    }
  }
  return v;
}

void DboxMain::SetGroupDisplayState(const vector<bool> &displaystatus)
{
  PWS_LOGIT;

  // We need to copy displaystatus since Expand may cause
  // SaveGroupDisplayState to be called, updating it

  // Could be called from OnSize before anything set up!
  // Check Tree is valid first
  if (m_ctlItemTree.GetSafeHwnd() == NULL || displaystatus.empty())
    return;

  HTREEITEM hItem = NULL;
  size_t i(0);
  while (NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem)) && i != displaystatus.size()) {
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      m_ctlItemTree.Expand(hItem, displaystatus[i] ? TVE_EXPAND : TVE_COLLAPSE);
      i++;
    }
  }
}

void DboxMain::SaveGUIStatus()
{
  PWS_LOGIT;

  st_SaveGUIInfo SaveGUIInfo;
  CItemData *pci_list(NULL), *pci_tree(NULL);

  // Note: we try and keep the same entry selected when the users
  // switches between List & Tree views.
  // But we can't if the user has selected a Group in the Tree view

  // Note: Must do this using the entry's UUID as the POSITION & 
  // HTREEITEM values may be different when we come to use it.
  POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
  if (pos != NULL) {
    pci_list = (CItemData *)m_ctlItemList.GetItemData((int)(INT_PTR)pos - 1);
    if (pci_list != NULL) {
      SaveGUIInfo.lSelected = pci_list->GetUUID();
      SaveGUIInfo.blSelectedValid = true;
    }
  }

  if (!m_sxNewPath.empty()) {
    SaveGUIInfo.sxGroupName = m_sxNewPath;  // Rename group only
    SaveGUIInfo.btGroupValid = true;
  } else {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    if (hi != NULL) {
      pci_tree = (CItemData *)m_ctlItemTree.GetItemData(hi);
      if (pci_tree != NULL) {
        SaveGUIInfo.tSelected = pci_tree->GetUUID();
        SaveGUIInfo.btSelectedValid = true;
      } else {
        StringX s;
        s = m_ctlItemTree.GetItemText(hi);

        while ((hi = m_ctlItemTree.GetParentItem(hi)) != NULL) {
          s = StringX(m_ctlItemTree.GetItemText(hi)) + StringX(L".") + s;
        }
        SaveGUIInfo.sxGroupName = s;
        SaveGUIInfo.btGroupValid = true;
      }
    }
  }

  SaveGUIInfo.vGroupDisplayState = GetGroupDisplayState();

  m_stkSaveGUIInfo.push(SaveGUIInfo);
}

void DboxMain::RestoreGUIStatus()
{
  PWS_LOGIT;

  if (m_stkSaveGUIInfo.empty())
    return; // better safe than sorry...

  st_SaveGUIInfo &SaveGUIInfo = m_stkSaveGUIInfo.top();

  if (m_bBoldItem) {
    m_ctlItemTree.SetItemState(m_LastFoundTreeItem, 0, TVIS_BOLD);
    m_bBoldItem = false;
  }

  ItemListIter iter;
  DisplayInfo *pdi;
  if (SaveGUIInfo.blSelectedValid) {
    iter = Find(SaveGUIInfo.lSelected);
    // Check if the previously selected item has been deleted
    if (iter != End()) {
      pdi = GetEntryGUIInfo(iter->second);
      m_ctlItemList.SetItemState(pdi->list_index, LVIS_SELECTED, LVIS_SELECTED);
      m_ctlItemTree.SelectItem(pdi->tree_item);

      if (m_ctlItemList.IsWindowVisible()) {
        UpdateToolBarForSelectedItem(&iter->second);
        SetDCAText(&iter->second);
      }
    }
  }

  if (SaveGUIInfo.btSelectedValid) {
    iter = Find(SaveGUIInfo.tSelected);
    // Check if the previously selected item has been deleted
    if (iter != End()) {
      pdi = GetEntryGUIInfo(iter->second);
      m_ctlItemTree.SelectItem(pdi->tree_item);

      if (m_ctlItemTree.IsWindowVisible()) {
        UpdateToolBarForSelectedItem(&iter->second);
        SetDCAText(&iter->second);
      }
    }
  }

  if (SaveGUIInfo.btGroupValid) {
    std::map<StringX, HTREEITEM>::iterator grouptreeiter;
    grouptreeiter = m_mapGroupToTreeItem.find(SaveGUIInfo.sxGroupName);
    if (grouptreeiter != m_mapGroupToTreeItem.end()) {
      m_ctlItemTree.SelectItem(grouptreeiter->second);

      if (m_ctlItemTree.IsWindowVisible()) {
        UpdateToolBarForSelectedItem(NULL);
        SetDCAText(NULL);
      }
    }
  }

  SetGroupDisplayState(SaveGUIInfo.vGroupDisplayState);

  m_stkSaveGUIInfo.pop();
}

void DboxMain::GetAllGroups(std::vector<std::wstring> &vGroups) const
{
  std::map<StringX, HTREEITEM>::const_iterator iter;

  for (iter = m_mapGroupToTreeItem.begin(); 
       iter != m_mapGroupToTreeItem.end(); iter++) {
    vGroups.push_back(iter->first.c_str());
  }
}

bool DboxMain::LongPPs(CWnd *pWnd)
{
  // Based on current screen height, decide if we want to display.
  // The normal "tall/long" page, or the "wide/short" version (for netbooks)
  // This can be overridden by the DlgOrientation preference.

  int orientation = PWSprefs::GetInstance()->GetPref(PWSprefs::DlgOrientation);
  if (orientation != PWSprefs::AUTO) {
    return (orientation == PWSprefs::TALL); // values are AUTO, TALL, WIDE
  }

  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST), &mi);
  const int YM = abs(mi.rcWork.bottom - mi.rcWork.top);

  // This is the main dialog - not the one we need to ask about!
  CRect rect;
  pWnd->GetWindowRect(&rect);
  const int YD = abs(rect.bottom - rect.top);

  return (YM > YD);
}

bool DboxMain::GetShortCut(const unsigned int &uiMenuItem,
                           unsigned short int &siVirtKey, unsigned char &cModifier)
{
  siVirtKey = 0;
  cModifier = '0';

  MapMenuShortcutsIter iter;

  iter = m_MapMenuShortcuts.find(uiMenuItem);
  if (iter == m_MapMenuShortcuts.end())
    return false;

  if (iter->second.siVirtKey  != iter->second.siDefVirtKey ||
      iter->second.cPWSModifier != iter->second.cDefPWSModifier) {
    siVirtKey = iter->second.siVirtKey;
    cModifier = iter->second.cPWSModifier;
  } else {
    siVirtKey = iter->second.siDefVirtKey;
    cModifier = iter->second.cDefPWSModifier;
  }

  return true;
}

StringX DboxMain::GetListViewItemText(CItemData &ci, const int &icolumn)
{
  StringX sx_fielddata(L"");
  time_t t;
  const CItemData::FieldType ft = (CItemData::FieldType)m_nColumnTypeByIndex[icolumn];
  
  switch (ft) {
    case CItemData::CTIME:
      sx_fielddata = ci.GetCTimeL();
      break;
    case CItemData::PMTIME:
      ci.GetPMTime(t);
      sx_fielddata = ((long)t == 0) ? ci.GetCTimeL() : ci.GetPMTimeL();
      break;
    case CItemData::RMTIME:
      ci.GetRMTime(t);
      sx_fielddata = ((long)t == 0) ? ci.GetCTimeL() : ci.GetRMTimeL();
      break;
    case CItemData::POLICY:
    {
      PWPolicy pwp;
      ci.GetPWPolicy(pwp);
      sx_fielddata = pwp.GetDisplayString();
      break;
    }
    case CItemData::KBSHORTCUT:
    {
      int32 iKBShortcut;
      ci.GetKBShortcut(iKBShortcut);
      if (iKBShortcut != 0) {
        WORD wVirtualKeyCode = iKBShortcut & 0xff;
        WORD wPWSModifiers = iKBShortcut >> 16;

        sx_fielddata = CMenuShortcut::FormatShortcut(wPWSModifiers, wVirtualKeyCode);
      }
      break;
    }
    case CItemData::ATTREF:
    {
      // Get "&Yes" and remove & (May not be leading in non-English languages)
      CString csYes(MAKEINTRESOURCE(IDS_YES));
      csYes.Replace(L"&", L"");
      sx_fielddata = ci.HasAttRef() ? csYes : L"";
      break;
    }
    default:
      sx_fielddata = ci.GetFieldValue(ft);
  }
  return sx_fielddata;
}

bool DboxMain::SetLayered(CWnd *pWnd, const int value)
{
  if (m_pfcnSetLayeredWindowAttributes && m_bOnStartupTransparancyEnabled) {
    HWND hWnd = pWnd->GetSafeHwnd();

    // Set Layered if not already
    LONG lstyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    if ((lstyle & WS_EX_LAYERED) == 0) {
      SetWindowLong(hWnd, GWL_EXSTYLE, lstyle | WS_EX_LAYERED);
    }

    const BYTE bytePercentTransparency = (value != -1) ? (BYTE)value :
      (BYTE)PWSprefs::GetInstance()->GetPref(PWSprefs::WindowTransparency);

    const BYTE byteWindowTransparency = (100 - bytePercentTransparency) * 255 / 100;

    // Set final transparency
    BOOL brc = m_pfcnSetLayeredWindowAttributes(hWnd, 0, byteWindowTransparency, LWA_ALPHA);

    if (brc == 0) {
      pws_os::IssueError(L"SetLayeredWindowAttributes", false);
    }

    return brc != 0;
  }

  // Couldn't do it
  return false;
}
