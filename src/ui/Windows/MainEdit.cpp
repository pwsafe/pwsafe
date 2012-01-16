/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// file MainEdit.cpp
//
// Edit-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "DDSupport.h"
#include "DboxMain.h"
#include "AddEdit_PropertySheet.h"
#include "ConfirmDeleteDlg.h"
#include "QuerySetDef.h"
#include "EditShortcutDlg.h"
#include "ClearQuestionDlg.h"
#include "CreateShortcutDlg.h"
#include "PasswordSubsetDlg.h"
#include "ExpPWListDlg.h"

#include "core/pwsprefs.h"
#include "core/PWSAuxParse.h"
#include "core/Command.h"
#include "core/core.h"

#include "os/dir.h"
#include "os/run.h"
#include "os/debug.h"

#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include <vector>
#include <algorithm>

using pws_os::CUUID;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Add an item
void DboxMain::OnAdd()
{
  CItemData ci;
  ci.CreateUUID();

  bool bLongPPs = LongPPs();

  CAddEdit_PropertySheet add_entry_psh(IDS_ADDENTRY, this, &m_core, NULL, &ci, bLongPPs,  L""); 

  PWSprefs *prefs = PWSprefs::GetInstance();
  if (prefs->GetPref(PWSprefs::UseDefaultUser)) {
    add_entry_psh.SetUsername(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
  }

  // m_TreeViewGroup may be set by OnContextMenu, if not, try to grok it
  if (m_TreeViewGroup.empty()) {
    CItemData *pci = NULL;
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (pci != NULL) { // leaf selected
          m_TreeViewGroup = pci->GetGroup();
        } else { // node selected
          m_TreeViewGroup = m_ctlItemTree.GetGroup(ti);
        }
      }
    } else { // list view
      // XXX TBD - get group name of currently selected list entry
    }
  }
  add_entry_psh.SetGroup(m_TreeViewGroup);
  m_TreeViewGroup = L""; // for next time

  // Remove Apply button
  add_entry_psh.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  INT_PTR rc = add_entry_psh.DoModal();

  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    bool bSetDefaultUser(false);
    CSecString &sxUsername = add_entry_psh.GetUsername();

    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

    //Check if they wish to set a default username
    if (!prefs->GetPref(PWSprefs::UseDefaultUser) &&
        (prefs->GetPref(PWSprefs::QuerySetDef)) &&
        (!sxUsername.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_message.Format(IDS_SETUSERNAME, (const CString&)sxUsername);

      INT_PTR rc2 = defDlg.DoModal();

      if (rc2 == IDOK) {
        bSetDefaultUser = true;

        // Initialise a copy of the DB preferences
        prefs->SetupCopyPrefs();

        // Update Copy with new values
        prefs->SetPref(PWSprefs::UseDefaultUser, true, true);
        prefs->SetPref(PWSprefs::DefaultUsername, sxUsername, true);

        // Set new DB preferences String value (from Copy)
        StringX sxNewDBPrefsString(prefs->Store(true));

        Command *pcmd1 = UpdateGUICommand::Create(&m_core,
                                                  UpdateGUICommand::WN_UNDO,
                                                  UpdateGUICommand::GUI_REFRESH_TREE);
        pmulticmds->Add(pcmd1);

        Command *pcmd2 = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
        pmulticmds->Add(pcmd2);
      }
    }

    DisplayInfo *pdi = new DisplayInfo;
    ci.SetDisplayInfo(pdi); // DisplayInfo values will be set later

    // Add the entry
    ci.SetStatus(CItemData::ES_ADDED);

    Command *pcmd;
    if (add_entry_psh.GetIBasedata() == 0) {
      pcmd = AddEntryCommand::Create(&m_core, ci);
    } else { // creating an alias
      pcmd = AddEntryCommand::Create(&m_core, ci, add_entry_psh.GetBaseUUID());
    }
    pmulticmds->Add(pcmd);

    if (bSetDefaultUser) {
      Command *pcmd3 = UpdateGUICommand::Create(&m_core,
                                                UpdateGUICommand::WN_EXECUTE_REDO,
                                                UpdateGUICommand::GUI_REFRESH_TREE);
      pmulticmds->Add(pcmd3);
    }

    Execute(pmulticmds);

    // Update Toolbar for this new entry
    m_ctlItemList.SetItemState(pdi->list_index, LVIS_SELECTED, LVIS_SELECTED);
    m_ctlItemTree.SelectItem(pdi->tree_item);
    UpdateToolBarForSelectedItem(&ci);

    if (m_core.GetNumEntries() == 1) {
      // For some reason, when adding the first entry, it is not visible!
      m_ctlItemTree.SetRedraw(TRUE);
    }

    SortListView();
    m_ctlItemList.SetFocus();
    SetChanged(Data);

    ChangeOkUpdate();
    m_RUEList.AddRUEntry(ci.GetUUID());

    // May need to update menu/toolbar if database was previously empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);

  } // rc == OK
}

//Add a shortcut
void DboxMain::OnCreateShortcut()
{
   // disable in read-only mode or nothing selected
  if (m_core.IsReadOnly() || SelItemOk() != TRUE)
    return;

  StringX sxNewDBPrefsString;
  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);

  CCreateShortcutDlg dlg_createshortcut(this, pci->GetGroup(), 
    pci->GetTitle(), pci->GetUser());

  PWSprefs *prefs = PWSprefs::GetInstance();
  if (prefs->GetPref(PWSprefs::UseDefaultUser)) {
    dlg_createshortcut.m_username = prefs->GetPref(PWSprefs::DefaultUsername).c_str();
  }

  INT_PTR rc = dlg_createshortcut.DoModal();

  if (rc == IDOK) {
    //Check if they wish to set a default username
    if (!prefs->GetPref(PWSprefs::UseDefaultUser) &&
        (prefs->GetPref(PWSprefs::QuerySetDef)) &&
        (!dlg_createshortcut.m_username.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_message.Format(IDS_SETUSERNAME, (const CString&)dlg_createshortcut.m_username);
      INT_PTR rc2 = defDlg.DoModal();
      if (rc2 == IDOK) {
        // Initialise a copy of the DB preferences
        prefs->SetupCopyPrefs();
        // Update Copy with new values
        prefs->SetPref(PWSprefs::UseDefaultUser, true, true);
        prefs->SetPref(PWSprefs::DefaultUsername, dlg_createshortcut.m_username, true);
        // Get old DB preferences String value (from current preferences) & 
        // new DB preferences String value (from Copy)
        const StringX sxOldDBPrefsString(prefs->Store());
        sxNewDBPrefsString = prefs->Store(true);
        if (sxOldDBPrefsString == sxNewDBPrefsString) {
          sxNewDBPrefsString.clear();
        }
      }
    }
    if (dlg_createshortcut.m_username.IsEmpty() && 
        prefs->GetPref(PWSprefs::UseDefaultUser))
      dlg_createshortcut.m_username = prefs->GetPref(PWSprefs::DefaultUsername).c_str();

    CreateShortcutEntry(pci, dlg_createshortcut.m_group, 
                        dlg_createshortcut.m_title, 
                        dlg_createshortcut.m_username, sxNewDBPrefsString);
  }
}
void DboxMain::CreateShortcutEntry(CItemData *pci, const StringX &cs_group,
                                   const StringX &cs_title, const StringX &cs_user,
                                   StringX &sxNewDBPrefsString)
{
  ASSERT(pci != NULL);

  CItemData ci_temp;
  ci_temp.CreateUUID();
  ci_temp.SetGroup(cs_group);
  ci_temp.SetTitle(cs_title);
  ci_temp.SetUser(cs_user);
  ci_temp.SetPassword(L"[Shortcut]");
  ci_temp.SetShortcut();

  time_t t;
  time(&t);
  ci_temp.SetCTime(t);
  ci_temp.SetXTime((time_t)0);
  ci_temp.SetStatus(CItemData::ES_ADDED);

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);
  if (!sxNewDBPrefsString.empty()) {
    Command *pcmd1 = UpdateGUICommand::Create(&m_core,
                                              UpdateGUICommand::WN_UNDO,
                                              UpdateGUICommand::GUI_REFRESH_TREE);
    pmulticmds->Add(pcmd1);

    Command *pcmd2 = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
    pmulticmds->Add(pcmd2);
  }

  Command *pcmd = AddEntryCommand::Create(&m_core, ci_temp, pci->GetUUID());
  pmulticmds->Add(pcmd);

  if (!sxNewDBPrefsString.empty()) {
   Command *pcmd3 = UpdateGUICommand::Create(&m_core,
                                              UpdateGUICommand::WN_EXECUTE_REDO,
                                              UpdateGUICommand::GUI_REFRESH_TREE);
    pmulticmds->Add(pcmd3);
  }
  Execute(pmulticmds);

  // Update base item's graphic
  ItemListIter iter = m_core.Find(pci->GetUUID());
  if (iter != End())
    UpdateEntryImages(iter->second);

  m_ctlItemList.SetFocus();
  SetChanged(Data);
  ChangeOkUpdate();
  m_RUEList.AddRUEntry(ci_temp.GetUUID());
}

//Add a group (tree view only)
void DboxMain::OnAddGroup()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  if (m_ctlItemTree.IsWindowVisible()) {
    // This can be reached by right clicking over an existing group node
    // or by clicking over "whitespace".
    // If the former, add a child node to the current one
    // If the latter, add to root.
    CString cmys_text(MAKEINTRESOURCE(IDS_NEWGROUP));
    CItemData *pci = getSelectedItem();
    if (pci != NULL) {
      // On an entry
      m_TreeViewGroup = pci->GetGroup();
    } else {
      // Group is selected but need to know if user used the Edit menu, right-clicked on it
      // or right-clicked on whitespace
      // The first 2 create a group under the selected group, the last creates it under root.
      if (m_bWhitespaceRightClick) {
        m_bWhitespaceRightClick = false;
        m_TreeViewGroup = L"";
      } else  {
        HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
        m_TreeViewGroup = m_ctlItemTree.GetGroup(ti);
      }
    }

    if (m_TreeViewGroup.empty())
      m_TreeViewGroup = cmys_text;
    else
      m_TreeViewGroup += L"." + cmys_text;

    // If group is there - make unique
    StringX s_copy(m_TreeViewGroup);
    bool bAlreadyExists(true);
    HTREEITEM newGroup;

    int i = 1;
    do {
      newGroup = m_ctlItemTree.AddGroup(s_copy.c_str(), bAlreadyExists);
      i++;
      // Format as per Windows Exlorer "New Folder"/"New Folder (n)"
      // where if 'n' present, it starts from 2
      Format(s_copy, L"%s (%d)", m_TreeViewGroup.c_str(), i);
    } while (bAlreadyExists);

    m_ctlItemTree.SelectItem(newGroup);
    m_TreeViewGroup = L""; // for next time

    // Needed by PWTreeCtrl::OnEndLabelEdit in case user doesn't change the group name
    m_bInAddGroup = true;
    m_ctlItemTree.EditLabel(newGroup);
  }
}

struct DupGroupFunctor : public CPWTreeCtrl::TreeItemFunctor {
  DupGroupFunctor(HTREEITEM hBase) : m_hBase(hBase) {}
  virtual void operator()(HTREEITEM hItem) {
    if (hItem == m_hBase) // we;re not interested in top
      return;
    m_items.push_back(hItem);
  }
  std::vector<HTREEITEM> m_items;
private:
  HTREEITEM m_hBase;
};

void DboxMain::OnDuplicateGroup()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  bool bRefresh(true);
  // Get selected group
  HTREEITEM ti = m_ctlItemTree.GetSelectedItem();

  // Verify that it is a group
  ASSERT((CItemData *)m_ctlItemTree.GetItemData(ti) == NULL);

  // Get complete group name
  StringX sxCurrentPath = (StringX)m_ctlItemTree.GetGroup(ti); // e.g., a.b.c
  StringX sxCurrentGroup = (StringX)m_ctlItemTree.GetItemText(ti); // e.g., c
  size_t grplen = sxCurrentPath.length();

  // Make sxNewGroup a unique name for the duplicate, e.g., "a.b.c Copy #1"
  size_t i = 0;
  StringX sxNewPath, sxNewGroup, sxCopy;
  do {
    i++;
    Format(sxCopy, IDS_COPYNUMBER, i);
    sxNewPath = sxCurrentPath + sxCopy;
  } while (m_mapGroupToTreeItem.find(sxNewPath) != m_mapGroupToTreeItem.end());
  sxNewGroup = sxCurrentGroup + sxCopy;

  // Have new group - now copy all entries in current group to new group

  std::vector<bool> bVNodeStates; // new group will have old's expanded nodes
  bool bState = (m_ctlItemTree.GetItemState(ti, TVIS_EXPANDED) &
                 TVIS_EXPANDED) != 0;
  bVNodeStates.push_back(bState);

  // Get all of the children
  if (m_ctlItemTree.ItemHasChildren(ti)) {
    DupGroupFunctor dgf(ti);
    m_ctlItemTree.Iterate(ti, dgf); // collects children into a handy vector
    /*
      Create 2 multi-commands to first add all normal or base entries and then a second
      to add any aliases and shortcuts.
    */
    MultiCommands *pmulti_cmd_base = MultiCommands::Create(&m_core);
    MultiCommands *pmulti_cmd_deps = MultiCommands::Create(&m_core);

    // Note that we need to do this twice - once to get all the normal entries
    // and bases and then the dependents as we need the mapping between the old
    // base UUIDs and their new UUIDs
    std::map<CUUID, CUUID> mapOldToNewBaseUUIDs;
    std::map<CUUID, CUUID>::const_iterator citer;

    // Process normal & base entries
    bool bDependentsExist(false);

    std::vector<HTREEITEM>::iterator iter;
    for (iter = dgf.m_items.begin(); iter != dgf.m_items.end(); iter++) {
      HTREEITEM hNextItem = *iter;
      CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hNextItem);
      if (pci != NULL) { // An entry (leaf)
        if (pci->IsDependent()) {
          bDependentsExist = true;
          continue;
        }
        
        TRACE(L"Copying %s.%s\n", pci->GetGroup().c_str(), pci->GetTitle().c_str());
        // Set up copy
        CItemData ci2(*pci);
        ci2.SetDisplayInfo(NULL);
        ci2.CreateUUID();
        ci2.SetStatus(CItemData::ES_ADDED);
        ci2.SetProtected(false);
        // Set new group
        StringX subPath =  pci->GetGroup();
        ASSERT(subPath.length() >= grplen);
        subPath =  subPath.substr(grplen);
        const StringX sxThisEntryNewGroup = sxNewPath + subPath;
        ci2.SetGroup(sxThisEntryNewGroup);

        if (pci->IsBase()) {
          mapOldToNewBaseUUIDs.insert(std::make_pair(pci->GetUUID(),
                                                     ci2.GetUUID()));
        } // pci->IsBase()

        // Make copy normal to begin with - if it has dependents then when those
        // are added then its entry type will automatically be changed
        ci2.SetNormal();
        Command *pcmd = AddEntryCommand::Create(&m_core, ci2);
        pcmd->SetNoGUINotify();
        pmulti_cmd_base->Add(pcmd);
      } else { // pci == NULL -> This is a node: save its expanded/collapsed state
        bool bState = (m_ctlItemTree.GetItemState(hNextItem, TVIS_EXPANDED) &
                       TVIS_EXPANDED) != 0;
        bVNodeStates.push_back(bState);
      }
    } // for

    // Now process dependents (if any)
    if (bDependentsExist) {
      for (iter = dgf.m_items.begin(); iter != dgf.m_items.end(); iter++) {
        HTREEITEM hNextItem = *iter;
        CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hNextItem);
        if (pci != NULL) {
          // Not a group but ignore if not a dependent
          if (!pci->IsDependent()) {
            continue;
          }
 
          // Set up copy
          CItemData ci2(*pci);
          ci2.SetDisplayInfo(NULL);
          ci2.CreateUUID();
          ci2.SetStatus(CItemData::ES_ADDED);
          ci2.SetProtected(false);
          // Set new group
          StringX subPath =  pci->GetGroup();
          ASSERT(subPath.length() >= grplen);
          subPath =  subPath.substr(grplen);
          const StringX sxThisEntryNewGroup = sxNewPath + subPath;
          ci2.SetGroup(sxThisEntryNewGroup);

          if (pci->IsAlias())
            ci2.SetAlias();
          else
            ci2.SetShortcut();

          Command *pcmd = NULL;
          const CItemData *pbci = GetBaseEntry(pci);
          ASSERT(pbci != NULL);

          StringX sxtmp;
          citer = mapOldToNewBaseUUIDs.find(pbci->GetUUID());
          if (citer != mapOldToNewBaseUUIDs.end()) {
            // Base is in duplicated group - use new values
            StringX subPath2 =  pbci->GetGroup();
            ASSERT(subPath2.length() >= grplen);
            subPath2 =  subPath2.substr(grplen);
            sxtmp = L"[" +
                      sxNewPath + subPath2 + L":" +
                      pbci->GetTitle() + L":" +
                      pbci->GetUser()  +
                    L"]";
            ci2.SetPassword(sxtmp);
            pcmd = AddEntryCommand::Create(&m_core, ci2, citer->second);
          } else {
            // Base not in duplicated group - use old values
            sxtmp = L"[" +
                      pbci->GetGroup() + L":" +
                      pbci->GetTitle() + L":" +
                      pbci->GetUser()  +
                    L"]";
            ci2.SetPassword(sxtmp);
            pcmd = AddEntryCommand::Create(&m_core, ci2, pbci->GetUUID());
          } // where's the base?
          pcmd->SetNoGUINotify();
          pmulti_cmd_deps->Add(pcmd);
        } // pci != NULL
      } // for
    } // bDependentsExist

    Command *pcmd1(NULL), *pcmd2(NULL);

    // We either do only one set of multi-commands, none of them or both of them
    // as one multi-command (normals/bases first followed by dependents)
    const int iDoExec = ((pmulti_cmd_base->GetSize() == 0) ? 0 : 1) +
                        ((pmulti_cmd_deps->GetSize() == 0) ? 0 : 2);
    if (iDoExec != 0) {
      pcmd1 = UpdateGUICommand::Create(&m_core, UpdateGUICommand::WN_UNDO,
                                            UpdateGUICommand::GUI_UNDO_IMPORT);
      pcmd2 = UpdateGUICommand::Create(&m_core, UpdateGUICommand::WN_EXECUTE_REDO,
                                            UpdateGUICommand::GUI_REDO_IMPORT);
    }
    switch (iDoExec) {
      case 0:
        // Do nothing
        bRefresh = false;
        break;
      case 1:
        // Only normal/base entries
        pmulti_cmd_base->Insert(pcmd1);
        pmulti_cmd_base->Add(pcmd2);
        Execute(pmulti_cmd_base);
        break;
      case 2:
        // Only dependents
        pmulti_cmd_deps->Insert(pcmd1);
        pmulti_cmd_deps->Add(pcmd2);
        Execute(pmulti_cmd_deps);
        break;
      case 3:
      {
        // Both - normal entries/bases first then dependents
        MultiCommands *pmulti_cmds = MultiCommands::Create(&m_core);
        pmulti_cmds->Add(pcmd1);
        pmulti_cmds->Add(pmulti_cmd_base);
        pmulti_cmds->Add(pmulti_cmd_deps);
        pmulti_cmds->Add(pcmd2);
        Execute(pmulti_cmds);
        break;
      }
      default:
        ASSERT(0);
    }
  } else { // !m_ctlItemTree.ItemHasChildren(ti)
    // User is duplicating an empty group - just add it
    HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);
    HTREEITEM ng_ti = m_ctlItemTree.InsertItem(sxNewGroup.c_str(), parent, TVI_SORT);
    m_ctlItemTree.SetItemImage(ng_ti, CPWTreeCtrl::NODE, CPWTreeCtrl::NODE);
    m_mapGroupToTreeItem[sxNewPath] = ng_ti;
    bRefresh = false;
  }

  // Must do this to re-populate m_mapGroupToTreeItem
  if (bRefresh) {
    RefreshViews();
    // Set expand/collapse state of new groups to match old
    HTREEITEM hNextItem = m_ctlItemTree.FindItem(sxNewPath.c_str(), TVI_ROOT);
    ASSERT(hNextItem != NULL);

    m_ctlItemTree.Expand(hNextItem, bVNodeStates[0] ? TVE_EXPAND : TVE_COLLAPSE);
    for (i = 1, hNextItem = m_ctlItemTree.GetNextTreeItem(hNextItem);
         hNextItem != NULL && i < bVNodeStates.size();
         hNextItem = m_ctlItemTree.GetNextTreeItem(hNextItem)) {
      if (m_ctlItemTree.ItemHasChildren(hNextItem)) {
        // Is node - set its expanded/collapsed state
        m_ctlItemTree.Expand(hNextItem, bVNodeStates[i] ? TVE_EXPAND : TVE_COLLAPSE);
        i++;
      } // ItemHasChildren
    } // Expand/collapse
  } // bRefresh

  m_ctlItemTree.SelectItem(ti);
}

void DboxMain::OnProtect(UINT nID)
{
  CItemData *pci(NULL);
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hStartItem = m_ctlItemTree.GetSelectedItem();
    if (hStartItem != NULL) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(hStartItem);
    }
  } else {
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos != NULL) {
      pci = (CItemData *)m_ctlItemList.GetItemData((int)pos - 1);
    }
  }
  if (pci != NULL) {
    // Entry
    ASSERT(nID == ID_MENUITEM_PROTECT || nID == ID_MENUITEM_UNPROTECT);
    Command *pcmd = UpdateEntryCommand::Create(&m_core, *pci, 
                                               CItemData::PROTECTED,
                                               nID == ID_MENUITEM_UNPROTECT ? L"0" : L"1");
    Execute(pcmd);

    SetChanged(Data);
  } else {
    // Group
    ASSERT(nID == ID_MENUITEM_PROTECTGROUP || nID == ID_MENUITEM_UNPROTECTGROUP);
    ChangeSubtreeEntriesProtectStatus(nID);
  }
  ChangeOkUpdate();
}

void DboxMain::ChangeSubtreeEntriesProtectStatus(const UINT nID)
{
  // Get selected group
  HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
  const HTREEITEM nextsibling = m_ctlItemTree.GetNextSiblingItem(ti);

  // Verify that it is a group
  ASSERT((CItemData *)m_ctlItemTree.GetItemData(ti) == NULL);

  // Get all of the children
  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

  if (m_ctlItemTree.ItemHasChildren(ti)) {
    HTREEITEM hNextItem;
    hNextItem = m_ctlItemTree.GetNextTreeItem(ti);

    while (hNextItem != NULL && hNextItem != nextsibling) {
      CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hNextItem);
      if (pci != NULL) {
        if (!pci->IsShortcut()) {
          if (pci->IsProtected() && nID == ID_MENUITEM_UNPROTECTGROUP) {
            Command *pcmd = UpdateEntryCommand::Create(&m_core, *pci, 
                                             CItemData::PROTECTED,
                                             L"0");
            pmulticmds->Add(pcmd);
          } else {
            if (nID == ID_MENUITEM_PROTECTGROUP) {
              Command *pcmd = UpdateEntryCommand::Create(&m_core, *pci, 
                                               CItemData::PROTECTED,
                                               L"1");
              pmulticmds->Add(pcmd);
            }
          }
        }
      }
      hNextItem = m_ctlItemTree.GetNextTreeItem(hNextItem);
    }
  }
  if (pmulticmds->GetSize() != 0)
    Execute(pmulticmds);
}

bool DboxMain::GetSubtreeEntriesProtectedStatus(int &numProtected, int &numUnprotected)
{
  int numShortcuts(0);
  numProtected = numUnprotected = 0;

  // Get selected group
  HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
  const HTREEITEM nextsibling = m_ctlItemTree.GetNextSiblingItem(ti);

  // Verify that it is a group
  ASSERT((CItemData *)m_ctlItemTree.GetItemData(ti) == NULL);

  // Get all of the children
  if (m_ctlItemTree.ItemHasChildren(ti)) {
    HTREEITEM hNextItem;
    hNextItem = m_ctlItemTree.GetNextTreeItem(ti);

    while (hNextItem != NULL && hNextItem != nextsibling) {
      CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hNextItem);
      if (pci != NULL) {
        if (pci->IsShortcut()) {
          numShortcuts++;
        } else {
          if (pci->IsProtected())
            numProtected++;
          else
            numUnprotected++;
        }
      }
      hNextItem = m_ctlItemTree.GetNextTreeItem(hNextItem);
    }
  }
  // Nothing to do if sub-tree empty or only contains shortcuts
  return !(numShortcuts >= 0 && (numProtected == 0 && numUnprotected == 0));
}

void DboxMain::OnDelete()
{
  // Check preconditions, possibly prompt user for confirmation, then call Delete()
  // to do the heavy lifting.
  if (m_core.GetNumEntries() == 0) // easiest way to avoid asking stupid questions...
    return;

  bool bAskForDeleteConfirmation = !(PWSprefs::GetInstance()->
                                     GetPref(PWSprefs::DeleteQuestion));
  bool dodelete = true;
  int num_children = 0;

  // Find number of child items, ask for confirmation if > 0
  StringX sxGroup(L""), sxTitle(L""), sxUser(L"");
  CItemData *pci(NULL);
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hStartItem = m_ctlItemTree.GetSelectedItem();
    if (hStartItem != NULL) {
      if (m_ctlItemTree.GetItemData(hStartItem) == NULL) {  // group node
        // ALWAYS ask if deleting a group - unless it is empty!
        num_children = m_ctlItemTree.CountChildren(hStartItem);
        bAskForDeleteConfirmation = num_children != 0;
      } else {
        pci = (CItemData *)m_ctlItemTree.GetItemData(hStartItem);
      }
    }
  } else {
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos != NULL) {
      pci = (CItemData *)m_ctlItemList.GetItemData((int)pos - 1);
    }
  }

  if (pci != NULL) {
    if (pci->IsProtected())
      return;

    sxGroup = pci->GetGroup();
    sxTitle = pci->GetTitle();
    sxUser = pci->GetUser();
  }

  // Confirm whether to delete the item
  if (bAskForDeleteConfirmation) {
    CConfirmDeleteDlg deleteDlg(this, num_children, sxGroup, sxTitle, sxUser);
    INT_PTR rc = deleteDlg.DoModal();
    if (rc == IDCANCEL) {
      dodelete = false;
    }
  }

  if (dodelete) {
    Delete();
    // Only refresh views if an entry or a non-empty group was deleted
    // If we refresh when deleting an empty group, the user will lose all
    // other empty groups
    if (m_bFilterActive || pci != NULL || (pci == NULL && num_children > 0))
      RefreshViews();

    ChangeOkUpdate();
  }
}

void DboxMain::Delete()
{
  // "Top level" element delete:
  // 1. Sets up Command mechanism
  // 2. Calls group or element Delete method, as appropriate
  // 3. Executes, updates UI.

  CItemData *pci = getSelectedItem();
  Command *pcmd = NULL;

  if (pci != NULL)
    pcmd = Delete(pci); // single entry
  else if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
    // Deleting a Group
    HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);
    pcmd = Delete(ti);
    m_ctlItemTree.SelectItem(parent);
    m_TreeViewGroup = L"";
  }

  if (pcmd != NULL) {
    Execute(pcmd);
  }
}

Command *DboxMain::Delete(const CItemData *pci)
{
  // Delete a single item of any type:
  // Normal, base, alias, shortcut...
  ASSERT(pci != NULL);
  pws_os::Trace(L"DboxMain::Delete(%s.%s)\n", pci->GetGroup().c_str(),
        pci->GetTitle().c_str());

  // ConfirmDelete asks for user confirmation
  // when deleting a shortcut or alias base.
  // Otherwise it just return true
  if (m_core.ConfirmDelete(pci))
    return DeleteEntryCommand::Create(&m_core, *pci);
  else
    return NULL;
}

// Functor for find_if to find the group name associated with a HTREEITEM
struct FindGroupFromHTREEITEM {
  FindGroupFromHTREEITEM(HTREEITEM& ti) : m_ti(ti) {}
  bool operator()(std::pair<StringX, HTREEITEM> const & p) const
  {
    return (p.second  == m_ti);
  }

  HTREEITEM m_ti;
};

Command *DboxMain::Delete(HTREEITEM ti)
{
  // Delete a group
  // Create a multicommand, iterate over tree's children,
  // throw everything into multicommand and be done with it.
  // Of course, this may recurse...

  if (ti == NULL)
    return NULL; // bad bottoming out of recursion?

  if (m_ctlItemTree.IsLeaf(ti)) {
    CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
    return Delete(pci); // normal bottom out of recursion
  }

  // Here if we have a bona fida group
  ASSERT(ti != NULL && !m_ctlItemTree.IsLeaf(ti));
  MultiCommands *pmulti_cmd = MultiCommands::Create(&m_core);
  
  HTREEITEM cti = m_ctlItemTree.GetChildItem(ti);

  while (cti != NULL) {
    CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(cti);
    if (pci != NULL)
      pmulti_cmd->Add(Delete(pci));
    else
      pmulti_cmd->Add(Delete(cti)); // subgroup!

    cti = m_ctlItemTree.GetNextItem(cti, TVGN_NEXT);
  }

  return pmulti_cmd;
}

void DboxMain::OnRename()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  // Renaming is only allowed while in Tree mode.
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hItem = m_ctlItemTree.GetSelectedItem();
    if (hItem != NULL) {
      if (m_ctlItemTree.IsLeaf(hItem)) {
        // Do not allow rename of protected entry
        CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
        ASSERT(pci != NULL);
        if (pci->IsProtected())
          return;
      }
      m_bInRename = true;
      m_ctlItemTree.EditLabel(hItem);
      if (m_bFilterActive && m_ctlItemTree.WasLabelEdited())
        RefreshViews();
      m_bInRename = false;
    }
  }
}

void DboxMain::OnEdit()
{
  // Note that Edit is also used for just viewing - don't want to disable
  // viewing in read-only mode
  if (SelItemOk() == TRUE) {
    CItemData *pci = getSelectedItem();
    ASSERT(pci != NULL);

    if (pci->IsShortcut())
      EditShortcut(pci);
    else
      EditItem(pci);
  } else {
    // entry item not selected - perhaps here on Enter on tree item?
    // perhaps not the most elegant solution to improving non-mouse use,
    // but it works. If anyone knows how Enter/Return gets mapped to OnEdit,
    // let me know...
    CItemData *pci_node(NULL);
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        pci_node = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (pci_node == NULL) { // node selected
          m_ctlItemTree.Expand(ti, TVE_TOGGLE);
        }
      }
    }
  }
}

bool DboxMain::EditItem(CItemData *pci, PWScore *pcore)
{
  // Note: In all but one circumstance, pcore == NULL, implying edit of an entry
  // in the current database.
  // The one except is when the user wishes to View an entry from the comparison
  // database via "CompareResultsDlg" (the Compare Database results dialog).
  // Note: In this instance, the comparison database is R-O and hence the user may
  // only View these entries and any database preferences can be obtain from the
  // copy of the header within that instance of PWScore - see below when changing the
  // default username.
  if (pcore == NULL)
    pcore = &m_core;

  CItemData ci_edit(*pci);

  // As pci may be invalidated if database is Locked while in this routine, 
  // we use a clone
  CItemData ci_original(*pci);
  pci = NULL; // Set to NULL - should use ci_original

  const UINT uicaller = pcore->IsReadOnly() ? IDS_VIEWENTRY : IDS_EDITENTRY;
  bool bLongPPs = LongPPs();
  CAddEdit_PropertySheet edit_entry_psh(uicaller, this, pcore,
                                        &ci_original, &ci_edit, 
                                        bLongPPs, pcore->GetCurFile()); 

  // List might be cleared if db locked.
  // Need to take care that we handle a rebuilt list.
  bool bIsDefUserSet;
  StringX sxDefUserValue;
  PWSprefs *prefs = PWSprefs::GetInstance();
  if (pcore == &m_core) {
    // As it is us, get values from actually current 
    bIsDefUserSet = prefs->GetPref(PWSprefs::UseDefaultUser) ? TRUE : FALSE;
    sxDefUserValue = prefs->GetPref(PWSprefs::DefaultUsername);
  } else {
    // Need to get Default User value from this core's preferences stored in its header.
    // Since this core is R-O, the values in the header will not have been changed
    StringX sxDBPreferences(pcore->GetDBPreferences());
    prefs->GetDefaultUserInfo(sxDBPreferences, bIsDefUserSet, sxDefUserValue);
  }
  if (bIsDefUserSet)
    edit_entry_psh.SetDefUsername(sxDefUserValue.c_str());

  // Don't show Apply button if in R-O mode (View)
  if (uicaller == IDS_VIEWENTRY)
    edit_entry_psh.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  INT_PTR rc = edit_entry_psh.DoModal();

  if (rc == IDOK && uicaller == IDS_EDITENTRY && edit_entry_psh.IsEntryModified()) {
    // Process user's changes.
    UpdateEntry(&edit_entry_psh);
    return true;
  } // rc == IDOK
  return false;
}

LRESULT DboxMain::OnApplyEditChanges(WPARAM wParam, LPARAM lParam)
{
  // Called if user does 'Apply' on the Add/Edit property sheet via
  // Windows Message PWS_MSG_EDIT_APPLY
  UNREFERENCED_PARAMETER(lParam);
  CAddEdit_PropertySheet *pentry_psh = (CAddEdit_PropertySheet *)wParam;
  UpdateEntry(pentry_psh);
  return 0L;
}

void DboxMain::UpdateEntry(CAddEdit_PropertySheet *pentry_psh)
{
  // Called by EditItem on return from Edit but
  // also called if user does 'Apply' on the Add/Edit property sheet via
  // Windows Message PWS_MSG_EDIT_APPLY

  PWScore *pcore = pentry_psh->GetCore();
  CItemData *pci_original = pentry_psh->GetOriginalCI();
  CItemData *pci_new = pentry_psh->GetNewCI();

  // Most of the following code handles special cases of alias/shortcut/base
  // But the common case is simply to replace the original entry
  // with a new one having the edited values and the same uuid.
  MultiCommands *pmulticmds = MultiCommands::Create(pcore);

  StringX newPassword = pci_new->GetPassword();

  CUUID original_base_uuid = CUUID::NullUUID();
  CUUID new_base_uuid = pentry_psh->GetBaseUUID();
  CUUID original_uuid = pci_original->GetUUID();

  if (pci_original->IsDependent()) {
    const CItemData *pci_orig_base = m_core.GetBaseEntry(pci_original);
    ASSERT(pci_orig_base != NULL);
    original_base_uuid = pci_orig_base->GetUUID();
  }

  ItemListIter iter;
  if (pentry_psh->GetOriginalEntrytype() == CItemData::ET_NORMAL &&
      pci_original->GetPassword() != newPassword) {
    // Original was a 'normal' entry and the password has changed
    if (pentry_psh->GetIBasedata() > 0) { // Now an alias
      Command *pcmd = AddDependentEntryCommand::Create(pcore, new_base_uuid, 
                                                       original_uuid,
                                                       CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);
      pci_new->SetPassword(L"[Alias]");
      pci_new->SetAlias();
    } else { // Still 'normal'
      pci_new->SetPassword(newPassword);
      pci_new->SetNormal();
    }
  } // Normal entry, password changed

  if (pentry_psh->GetOriginalEntrytype() == CItemData::ET_ALIAS) {
    // Original was an alias - delete it from multimap
    // RemoveDependentEntry also resets base to normal if no more dependents
    Command *pcmd = RemoveDependentEntryCommand::Create(pcore,
                                                        original_base_uuid,
                                                        original_uuid,
                                                        CItemData::ET_ALIAS);
    pmulticmds->Add(pcmd);
    if (newPassword == pentry_psh->GetBase()) {
      // Password (i.e. base) unchanged - put it back
      Command *pcmd = AddDependentEntryCommand::Create(pcore,
                                                       original_base_uuid, 
                                                       original_uuid,
                                                       CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);
    } else { // Password changed
      // Password changed so might be an alias of another entry!
      // Could also be the same entry i.e. [:t:] == [t] !
      if (pentry_psh->GetIBasedata() > 0) { // Still an alias
        Command *pcmd = AddDependentEntryCommand::Create(pcore, new_base_uuid, 
                                                         original_uuid,
                                                         CItemData::ET_ALIAS);
        pmulticmds->Add(pcmd);
        pci_new->SetPassword(L"[Alias]");
        pci_new->SetAlias();
      } else { // No longer an alias
        pci_new->SetPassword(newPassword);
        pci_new->SetNormal();
      }
    } // Password changed
  } // Alias

  if (pentry_psh->GetOriginalEntrytype() == CItemData::ET_ALIASBASE &&
      pci_original->GetPassword() != newPassword) {
    // Original was a base but might now be an alias of another entry!
    if (pentry_psh->GetIBasedata() > 0) {
      // Now an alias
      // Make this one an alias
      Command *pcmd1 = AddDependentEntryCommand::Create(pcore, new_base_uuid, 
                                                        original_uuid,
                                                        CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd1);
      pci_new->SetPassword(L"[Alias]");
      pci_new->SetAlias();
      // Move old aliases across
      Command *pcmd2 = MoveDependentEntriesCommand::Create(pcore,
                                                           original_uuid, 
                                                           new_base_uuid,
                                                           CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd2);
    } else { // Still a base entry but with a new password
      pci_new->SetPassword(newPassword);
      pci_new->SetAliasBase();
    }
  } // AliasBase with password changed

  // Update old base...
  iter = pcore->Find(original_base_uuid);
  if (iter != End())
    UpdateEntryImages(iter->second);

  // ... and the new base entry (only if different from the old one)
  if (CUUID(new_base_uuid) != CUUID(original_base_uuid)) {
    iter = pcore->Find(new_base_uuid);
    if (iter != End())
      UpdateEntryImages(iter->second);
  }

  if (pci_new->IsDependent()) {
    pci_new->SetXTime((time_t)0);
    pci_new->SetPWPolicy(L"");
  }

  pci_new->SetStatus(CItemData::ES_MODIFIED);

  Command *pcmd = EditEntryCommand::Create(pcore, *(pci_original), 
                                                  *(pci_new));
  pmulticmds->Add(pcmd);

  Execute(pmulticmds, pcore);

  SetChanged(Data);

  ChangeOkUpdate();
  // Order may have changed as a result of edit
  m_ctlItemTree.SortTree(TVI_ROOT);
  SortListView();

  short sh_odca, sh_ndca;
  pci_original->GetDCA(sh_odca);
  pci_new->GetDCA(sh_ndca);
  if (sh_odca != sh_ndca)
    SetDCAText(pci_new);

  UpdateToolBarForSelectedItem(pci_new);

  // Password may have been updated and so not expired
  UpdateEntryImages(*pci_new);

  // Update display if no longer passes filter criteria
  if (m_bFilterActive && !PassesFiltering(*pci_new, m_currentfilter)) {
      RefreshViews();
      return;
  }

  // Reselect entry, where-ever it may be
  iter = m_core.Find(original_uuid);
  if (iter != End()) {
    DisplayInfo *pdi = (DisplayInfo *)iter->second.GetDisplayInfo();
    SelectEntry(pdi->list_index);
  }
}

bool DboxMain::EditShortcut(CItemData *pci, PWScore *pcore)
{
  if (pcore == NULL)
    pcore = &m_core;

  // List might be cleared if db locked.
  // Need to take care that we handle a rebuilt list.
  CItemData ci_edit(*pci);
  // As pci may be invalidated if database is Locked while in this routine, 
  // we use a clone
  CItemData ci_original(*pci);
  pci = NULL; // Set to NULL - should use ci_original

  const CItemData *pbci = GetBaseEntry(&ci_original);

  CEditShortcutDlg dlg_editshortcut(&ci_edit, this, pbci->GetGroup(),
                                    pbci->GetTitle(), pbci->GetUser());

  // Need to get Default User value from this core's preferences stored in its header, 
  // which are not necessarily in our PWSprefs::GetInstance !
  bool bIsDefUserSet;
  StringX sxDefUserValue, sxDBPreferences;
  sxDBPreferences = pcore->GetDBPreferences();
  PWSprefs::GetInstance()->GetDefaultUserInfo(sxDBPreferences, bIsDefUserSet,
                                              sxDefUserValue);
  if (bIsDefUserSet)
    dlg_editshortcut.m_defusername = sxDefUserValue.c_str();

  dlg_editshortcut.m_Edit_IsReadOnly = pcore->IsReadOnly();

  INT_PTR rc = dlg_editshortcut.DoModal();

  if (rc == IDOK && dlg_editshortcut.IsEntryModified()) {
    // Out with the old, in with the new
    // User cannot change a shortcut entry to anything else!
    ItemListIter listpos = Find(ci_original.GetUUID());
    ASSERT(listpos != pcore->GetEntryEndIter());
    CItemData oldElem = GetEntryAt(listpos);
    ci_edit.SetXTime((time_t)0);
    ci_edit.SetStatus(CItemData::ES_MODIFIED);

    Command *pcmd = EditEntryCommand::Create(pcore, ci_original, ci_edit);
    Execute(pcmd, pcore);
    SetChanged(Data);

    // DisplayInfo's copied and changed, get up-to-date version
    DisplayInfo *pdi = static_cast<DisplayInfo *>
         (pcore->GetEntry(pcore->Find(ci_original.GetUUID())).GetDisplayInfo());
    rc = SelectEntry(pdi->list_index);

    if (rc == 0) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();

    UpdateToolBarForSelectedItem(&ci_edit);
    return true;
  } // rc == IDOK
  return false;
}

// Duplicate selected entry but make title unique
void DboxMain::OnDuplicateEntry()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  if (SelItemOk() == TRUE) {
    CItemData *pci = getSelectedItem();
    ASSERT(pci != NULL);
    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
    ASSERT(pdi != NULL);

    // Get information from current selected entry
    const StringX ci2_group = pci->GetGroup();
    const StringX ci2_user = pci->GetUser();
    const StringX ci2_title0 = pci->GetTitle();
    StringX ci2_title;

    // Find a unique "Title"
    ItemListConstIter listpos;
    int i = 0;
    StringX sxCopy;
    do {
      i++;
      Format(sxCopy, IDS_COPYNUMBER, i);
      ci2_title = ci2_title0 + sxCopy;
      listpos = m_core.Find(ci2_group, ci2_title, ci2_user);
    } while (listpos != m_core.GetEntryEndIter());

    // Set up new entry
    CItemData ci2(*pci);
    ci2.SetDisplayInfo(NULL);
    ci2.CreateUUID();
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetStatus(CItemData::ES_ADDED);
    ci2.SetProtected(false);

    Command *pcmd = NULL;
    if (pci->IsDependent()) {
      if (pci->IsAlias()) {
        ci2.SetAlias();
      } else {
        ci2.SetShortcut();
      }

      const CItemData *pbci = GetBaseEntry(pci);
      if (pbci != NULL) {
        StringX sxtmp;
        sxtmp = L"[" +
                  pbci->GetGroup() + L":" +
                  pbci->GetTitle() + L":" +
                  pbci->GetUser()  +
                L"]";
        ci2.SetPassword(sxtmp);
        pcmd = AddEntryCommand::Create(&m_core, ci2, pbci->GetUUID());
      }
    } else { // not alias or shortcut
      ci2.SetNormal();
      pcmd = AddEntryCommand::Create(&m_core, ci2);
    }

    Execute(pcmd);

    pdi->list_index = -1; // so that InsertItemIntoGUITreeList will set new values

    ItemListIter iter = m_core.Find(ci2.GetUUID());
    ASSERT(iter != m_core.GetEntryEndIter());

    InsertItemIntoGUITreeList(m_core.GetEntry(iter));
    FixListIndexes();
    SetChanged(Data);

    int rc = SelectEntry(pdi->list_index);
    if (rc == 0) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();
    m_RUEList.AddRUEntry(ci2.GetUUID());
  }
}

void DboxMain::OnDisplayPswdSubset()
{
  if (!SelItemOk())
    return;

  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);


  if (pci->IsDependent()) {
    pci = GetBaseEntry(pci);
    ASSERT(pci != NULL);
  }

  const CUUID uuid = pci->GetUUID();

  CPasswordSubsetDlg DisplaySubsetDlg(this, pci->GetPassword());

  if (DisplaySubsetDlg.DoModal() != IDCANCEL) {
    // get pci again, in case PasswordSafe was locked and pci is invalidated
    ItemListIter iter = Find(uuid);
    if (iter != End()) { // can happen if dbox didn't minimize
      UpdateAccessTime(&iter->second);
    }
  }
}

void DboxMain::OnCopyPassword()
{
  CopyDataToClipBoard(CItemData::PASSWORD);
}

void DboxMain::OnCopyPasswordMinimize()
{
  // Do OnCopyPassword and minimize afterwards.
   CopyDataToClipBoard(CItemData::PASSWORD, true);
}

void DboxMain::OnCopyUsername()
{
  CopyDataToClipBoard(CItemData::USER);
}

void DboxMain::OnCopyNotes()
{
  CopyDataToClipBoard(CItemData::NOTES);
}

void DboxMain::OnCopyURL()
{
  CopyDataToClipBoard(CItemData::URL);
}

void DboxMain::OnCopyEmail()
{
  CopyDataToClipBoard(CItemData::EMAIL);
}

void DboxMain::OnCopyRunCommand()
{
  const MSG *pMSG = GetCurrentMessage();
  // Expand Run Command by default
  // Disable expansion if via a menu item (not accelerator) and Ctrl key down
  bool bDoNotExpand = (HIWORD(pMSG->wParam) == 0 && pMSG->lParam == 0) &&
                      (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  CopyDataToClipBoard(CItemData::RUNCMD, bDoNotExpand);
}

void DboxMain::CopyDataToClipBoard(const CItemData::FieldType ft, const bool bSpecial)
{
  // Boolean 'bSpecial' flag is CItemData::FieldType 'ft' dependent
  // For example:
  //   For "CItemData::PASSWORD": "bSpecial" == true means "Minimize after copy"
  //   For "CItemData::RUNCMD":   "bSpecial" == true means "Do NOT expand the Run command"
  if (SelItemOk() != TRUE)
    return;

  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);

  CItemData *pci_original(pci);

  if (pci->IsShortcut() ||
      (pci->IsAlias() && ft == CItemData::PASSWORD)) {
    CItemData *pbci = GetBaseEntry(pci);
    ASSERT(pbci != NULL);
    pci = pbci;
  }

  StringX sxData;

  switch (ft) {
    case CItemData::PASSWORD:
    {
      //Remind the user about clipboard security
      CClearQuestionDlg clearDlg(this);
      if (clearDlg.m_dontaskquestion == FALSE &&
          clearDlg.DoModal() == IDCANCEL)
        return;
      sxData = pci->GetPassword();
      if (bSpecial) {
        ShowWindow(SW_MINIMIZE);
      }
      break;
    }
    case CItemData::USER:
      sxData = pci->GetUser();
      break;
    case CItemData::NOTES:
      sxData = pci->GetNotes();
      break;
    case CItemData::URL:
    {
      StringX::size_type ipos;
      sxData = pci->GetURL();
      ipos = sxData.find(L"[alt]");
      if (ipos != StringX::npos)
        sxData.replace(ipos, 5, L"");
      ipos = sxData.find(L"[ssh]");
      if (ipos != StringX::npos)
        sxData.replace(ipos, 5, L"");
      ipos = sxData.find(L"{alt}");
      if (ipos != StringX::npos)
        sxData.replace(ipos, 5, L"");
      ipos = sxData.find(L"[autotype]");
      if (ipos != StringX::npos)
        sxData.replace(ipos, 10, L"");
      ipos = sxData.find(L"[xa]");
      if (ipos != StringX::npos)
        sxData.replace(ipos, 4, L"");
      break;
    }
    case CItemData::RUNCMD:
      sxData = pci->GetRunCommand();
      if (!bSpecial) {
        // Expand Run Command
        std::wstring errmsg;
        size_t st_column;
        bool bURLSpecial;
        sxData = PWSAuxParse::GetExpandedString(sxData,
                                                 m_core.GetCurFile(),
                                                 pci,
                                                 m_bDoAutoType,
                                                 m_AutoType,
                                                 errmsg, st_column, bURLSpecial);
        if (errmsg.length() > 0) {
          CGeneralMsgBox gmb;
          CString cs_title(MAKEINTRESOURCE(IDS_RUNCOMMAND_ERROR));
          CString cs_errmsg;
          cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
          gmb.MessageBox(cs_errmsg, cs_title, MB_ICONERROR);
        }
      }
      break;
    case CItemData::EMAIL:
      sxData = pci->GetEmail();
      break;
    default:
      ASSERT(0);
  }

  SetClipboardData(sxData);
  UpdateLastClipboardAction(ft);
  UpdateAccessTime(pci_original);
}

void DboxMain::UpdateLastClipboardAction(const int iaction)
{
  int imsg(0);
  m_lastclipboardaction = L"";
  switch (iaction) {
    case -1:
      // Clipboard cleared
      m_lastclipboardaction = L"";
      break;
    case CItemData::GROUP:
      imsg = IDS_GROUPCOPIED;
      break;
    case CItemData::TITLE:
      imsg = IDS_TITLECOPIED;
      break;
    case CItemData::USER:
      imsg = IDS_USERCOPIED;
      break;
    case CItemData::PASSWORD:
      imsg = IDS_PSWDCOPIED;
      break;
    case CItemData::NOTES:
      imsg = IDS_NOTESCOPIED;
      break;
    case CItemData::URL:
      imsg = IDS_URLCOPIED;
      break;
    case CItemData::AUTOTYPE:
      imsg = IDS_AUTOTYPECOPIED;
      break;
    case CItemData::RUNCMD:
      imsg = IDS_RUNCMDCOPIED;
      break;
    case CItemData::EMAIL:
      imsg = IDS_EMAILCOPIED;
      break;
    default:
      ASSERT(0);
      return;
  }

  if (iaction > 0) {
    wchar_t szTimeFormat[80], szTimeString[80];
    VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, 
                           szTimeFormat, 80 /* sizeof(szTimeFormat) / sizeof(wchar_t) */));
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, szTimeFormat,
                  szTimeString, 80 /* sizeof(szTimeString) / sizeof(wchar_t) */);
    m_lastclipboardaction.LoadString(imsg);
    m_lastclipboardaction += szTimeString;
  }

  m_ilastaction = iaction;
  UpdateStatusBar();
}

void DboxMain::OnShowFindToolbar()
{
  // Show Find Toolbar
  SetFindToolBar(true);
}

void DboxMain::OnClearClipboard()
{
  UpdateLastClipboardAction(-1);
  ClearClipboardData();
}

// Generate a random password.
// The generated password will be copied to the clipboard. Doing
// this leaves a problem where the user can generate a random password, have
// the password copied to the clipboard and then change the password. This could
// be avoided by putting the password into the clipboard when the entry is saved
// but that would be annoying when generating a new entry.

void DboxMain::MakeRandomPassword(StringX &password, PWPolicy &pwp, stringT st_symbols,
                                  bool bIssueMsg)
{
  password = pwp.MakeRandomPassword(st_symbols);
  SetClipboardData(password);
  UpdateLastClipboardAction(CItemData::PASSWORD);

  if (bIssueMsg) {
    CGeneralMsgBox gmb;
    CString cs_title, cs_msg;
    cs_title.LoadString(IDS_PASSWORDGENERATED1);
    cs_msg.Format(IDS_PASSWORDGENERATED2, password.c_str());
    gmb.MessageBox(cs_msg, cs_title, MB_OK);
  }
}

void DboxMain::PerformAutoType()
{
  OnAutoType();
}

void DboxMain::OnAutoType()
{
  CItemData *pci(NULL);
  if (m_ctlItemTree.IsWindowVisible() && m_LastFoundTreeItem != NULL) {
    pci = (CItemData *)m_ctlItemTree.GetItemData(m_LastFoundTreeItem);
    pws_os::Trace(L"OnAutoType: Using Tree found item\n");
  } else
  if (m_ctlItemList.IsWindowVisible() && m_LastFoundListItem >= 0) {
    pci = (CItemData *)m_ctlItemList.GetItemData(m_LastFoundListItem);
    pws_os::Trace(L"OnAutoType: Using List found item\n");
  } else {
    pci = getSelectedItem();
    pws_os::Trace(L"OnAutoType: Using Selected item\n");
  }

  if (pci == NULL)
    return;

  UpdateAccessTime(pci);

  // All code using ci must be before this AutoType since the
  // latter may trash *pci if lock-on-minimize
  AutoType(*pci);
}

void DboxMain::AutoType(const CItemData &ci)
{
  // Called from OnAutoType, OnTrayAutoType and OnDragAutoType

  // Rules are ("Minimize on Autotype" takes precedence):
  // 1. If "MinimizeOnAutotype" - minimize PWS during Autotype but do
  //    not restore it (previous default action - but a pain if locked
  //    in the system tray!)
  // 2. If "Always on Top" - hide PWS during Autotype and then make it
  //    "AlwaysOnTop" again, unless minimized!
  // 3. If not "Always on Top" - hide PWS during Autotype and show
  //    it again once finished - but behind other windows.
  // NOTE: If "Lock on Minimize" is set and "MinimizeOnAutotype" then
  //       the window will be locked once minimized.
  bool bMinOnAuto = PWSprefs::GetInstance()->
                    GetPref(PWSprefs::MinimizeOnAutotype);

  // Use CItemData ci before we potentially minimize the Window, since if
  // the user also specifies 'Lock on Minimize', it will become invalid.
  std::vector<size_t> vactionverboffsets;
  const StringX sxautotype = PWSAuxParse::GetAutoTypeString(ci, m_core,
                                                            vactionverboffsets);

  if (bMinOnAuto) {
    // Need to save display status for when we return from minimize
    m_vGroupDisplayState = GetGroupDisplayState();
    ShowWindow(SW_MINIMIZE);
  } else {
    ShowWindow(SW_HIDE);
  }

  DoAutoType(sxautotype, vactionverboffsets);

  // If we minimized it, exit. If we only hid it, now show it
  if (bMinOnAuto)
    return;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
    SetWindowPos(&wndTopMost, 0, 0, 0, 0,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
  } else {
    ShowWindow(SW_SHOW);
  }
}

void DboxMain::DoAutoType(const StringX &sx_autotype,
                          const std::vector<size_t> &vactionverboffsets)
{
  PWSAuxParse::SendAutoTypeString(sx_autotype, vactionverboffsets);
}

void DboxMain::OnGotoBaseEntry()
{
  if (SelItemOk() == TRUE) {
    CItemData *pci = getSelectedItem();
    ASSERT(pci != NULL);

    const CItemData *pbci = GetBaseEntry(pci);
    if (pbci != NULL) {
      DisplayInfo *pdi = (DisplayInfo *)pbci->GetDisplayInfo();
      SelectEntry(pdi->list_index);
      UpdateAccessTime(pci);
    }
  }
}

void DboxMain::OnEditBaseEntry()
{
  // Only for Shortcuts & Aliases
  if (SelItemOk() == TRUE) {
    CItemData *pci = getSelectedItem();
    ASSERT(pci != NULL);

    CItemData *pbci = GetBaseEntry(pci);
    if (pbci != NULL) {
       DisplayInfo *pdi = (DisplayInfo *)pbci->GetDisplayInfo();
       SelectEntry(pdi->list_index);
       EditItem(pbci);
       // pbci may be invalid upon return!
       UpdateAccessTime(GetBaseEntry(pci));
    }
  }
}

void DboxMain::OnRunCommand()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);

  CItemData *pci_original(pci);
  StringX sx_pswd;

  if (pci->IsDependent()) {
    CItemData *pbci = GetBaseEntry(pci);
    ASSERT(pbci != NULL);
    sx_pswd = pbci->GetPassword();
    if (pci->IsShortcut())
      pci = pbci;
  } else
    sx_pswd = pci->GetPassword();

  StringX sx_RunCommand, sx_Expanded_ES;
  sx_RunCommand = pci->GetRunCommand();
  if (sx_RunCommand.empty())
    return;

  std::wstring errmsg;
  StringX::size_type st_column;
  bool bURLSpecial;
  sx_Expanded_ES = PWSAuxParse::GetExpandedString(sx_RunCommand, 
                       m_core.GetCurFile(), pci, 
                       m_bDoAutoType, m_AutoType, 
                       errmsg, st_column, bURLSpecial);
  if (!errmsg.empty()) {
    CGeneralMsgBox gmb;
    CString cs_title, cs_errmsg;
    cs_title.LoadString(IDS_RUNCOMMAND_ERROR);
    cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
    gmb.MessageBox(cs_errmsg, cs_title, MB_OK | MB_ICONQUESTION);
    return;
  }

  m_AutoType = PWSAuxParse::GetAutoTypeString(m_AutoType, pci->GetGroup(), 
                                 pci->GetTitle(), pci->GetUser(), 
                                 sx_pswd, pci->GetNotes(),
                                 m_vactionverboffsets);
  SetClipboardData(pci->GetPassword());
  UpdateLastClipboardAction(CItemData::PASSWORD);
  UpdateAccessTime(pci_original);

  // Now honour presence of [alt], {alt} or [ssh] in the url if present
  // in the RunCommand field.  Note: they are all treated the same (unlike
  // in 'Browse to'.
  StringX sxAltBrowser(PWSprefs::GetInstance()->
                         GetPref(PWSprefs::AltBrowser));

  if (bURLSpecial && !sxAltBrowser.empty()) {
    StringX sxCmdLineParms(PWSprefs::GetInstance()->
                           GetPref(PWSprefs::AltBrowserCmdLineParms));

    if (sxAltBrowser[0] != L'\'' && sxAltBrowser[0] != L'"')
      sxAltBrowser = L"\"" + sxAltBrowser + L"\"";
    if (!sxCmdLineParms.empty())
      sx_Expanded_ES = sxAltBrowser + StringX(L" ") + 
                       sxCmdLineParms + StringX(L" ") + sx_Expanded_ES;
    else
      sx_Expanded_ES = sxAltBrowser + StringX(L" ") + sx_Expanded_ES;
  }

  bool rc = m_runner.runcmd(sx_Expanded_ES, !m_AutoType.empty());
  if (!rc) {
    m_bDoAutoType = false;
    m_AutoType.clear();
    return;
  }
}

void DboxMain::AddDDEntries(CDDObList &in_oblist, const StringX &DropGroup)
{
  // Add Drop entries
  CItemData ci_temp;
  UUIDVector Possible_Aliases, Possible_Shortcuts;
  std::vector<StringX> vAddedPolicyNames;
  StringX sxEntriesWithNewNamedPolicies;
  std::map<StringX, StringX> mapRenamedPolicies;
  StringX sxgroup, sxtitle, sxuser;
  POSITION pos;
  wchar_t *dot;
  bool bAddToViews;

  const StringX sxDD_DateTime = PWSUtil::GetTimeStamp(true).c_str();

  // Initialize set
  GTUSet setGTU;
  m_core.InitialiseGTU(setGTU);

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

  for (pos = in_oblist.GetHeadPosition(); pos != NULL; in_oblist.GetNext(pos)) {
    CDDObject *pDDObject = (CDDObject *)in_oblist.GetAt(pos);
#ifdef DEMO
    if (m_core.GetNumEntries() >= MAXDEMO)
      break;
#endif /* DEMO */

    bool bChangedPolicy(false);
    ci_temp.Clear();
    // Only set to false if adding a shortcut where the base isn't there (yet)
    bAddToViews = true;
    pDDObject->ToItem(ci_temp);

    StringX sxPolicyName = ci_temp.GetPolicyName();
    if (!sxPolicyName.empty()) {
      // D&D put the entry's name here and the details in the entry
      // which we now have to add to this core and remove from the entry

      // Get the source database PWPolicy & symbols for this name
      st_PSWDPolicy st_pp;
      ci_temp.GetPWPolicy(st_pp.pwp);
      st_pp.symbols = ci_temp.GetSymbols();

      // Get the same info if the policy is in the target database
      st_PSWDPolicy currentDB_st_pp;
      bool bNPWInCurrentDB = GetPolicyFromName(sxPolicyName, currentDB_st_pp);
      if (bNPWInCurrentDB) {
        // It exists in target database
        if (st_pp != currentDB_st_pp) {
          // They are not the same - make this policy unique
          m_core.MakePolicyUnique(mapRenamedPolicies, sxPolicyName, sxDD_DateTime, 
                                  IDS_DRAGPOLICY);
          ci_temp.SetPolicyName(sxPolicyName);
          bChangedPolicy = true;
        }
      }

      if (!bNPWInCurrentDB || bChangedPolicy) {
        // Not in target database or has different settings -
        // Add it if we haven't already
        if (std::find(vAddedPolicyNames.begin(), vAddedPolicyNames.end(), sxPolicyName) ==
                      vAddedPolicyNames.end()) {
          // Doesn't already exist and we haven't already added it - add
          Command *pcmd = DBPolicyNamesCommand::Create(&m_core, sxPolicyName, st_pp);
          pmulticmds->Add(pcmd);
          vAddedPolicyNames.push_back(sxPolicyName);
        }
        // No longer need these values
        ci_temp.SetPWPolicy(L"");
        ci_temp.SetSymbols(L"");
      }
    }

    if (in_oblist.m_bDragNode) {
      dot = (!DropGroup.empty() && !ci_temp.GetGroup().empty()) ? L"." : L"";
      sxgroup = DropGroup + dot + ci_temp.GetGroup();
    } else {
      sxgroup = DropGroup;
    }

    sxuser = ci_temp.GetUser();
    StringX sxnewtitle(ci_temp.GetTitle());
    m_core.MakeEntryUnique(setGTU, sxgroup, sxnewtitle, sxuser, IDS_DRAGNUMBER);

    if (m_core.Find(ci_temp.GetUUID()) != End()) {
      // Already in use - get a new one!
      ci_temp.CreateUUID();
    }

    if (bChangedPolicy) {
      StringX sxChanged = L"\r\n\xab" + sxgroup + L"\xbb " +
	                        L"\xab" + sxnewtitle + L"\xbb " +
	                        L"\xab" + sxuser + L"\xbb";
      sxEntriesWithNewNamedPolicies += sxChanged;
    }

    ci_temp.SetGroup(sxgroup);
    ci_temp.SetTitle(sxnewtitle);

    StringX cs_tmp = ci_temp.GetPassword();

    BaseEntryParms pl;
    pl.InputType = CItemData::ET_NORMAL;

    // Potentially remove outer single square brackets as ParseBaseEntryPWD expects only
    // one set of square brackets (processing import and user edit of entries)
    if (cs_tmp.substr(0, 2) == L"[[" &&
        cs_tmp.substr(cs_tmp.length() - 2) == L"]]") {
      cs_tmp = cs_tmp.substr(1, cs_tmp.length() - 2);
      pl.InputType = CItemData::ET_ALIAS;
    }

    // Potentially remove tilde as ParseBaseEntryPWD expects only
    // one set of square brackets (processing import and user edit of entries)
    if (cs_tmp.substr(0, 2) == L"[~" &&
        cs_tmp.substr(cs_tmp.length() - 2) == L"~]") {
      cs_tmp = L"[" + cs_tmp.substr(2, cs_tmp.length() - 4) + L"]";
      pl.InputType = CItemData::ET_SHORTCUT;
    }

    m_core.ParseBaseEntryPWD(cs_tmp, pl);
    if (pl.ibasedata > 0) {
      CGeneralMsgBox gmb;
      // Password in alias/shortcut format AND base entry exists
      if (pl.InputType == CItemData::ET_ALIAS) {
        ItemListIter iter = m_core.Find(pl.base_uuid);
        ASSERT(iter != End());
        if (pl.TargetType == CItemData::ET_ALIAS) {
          // This base is in fact an alias. ParseBaseEntryPWD already found 'proper base'
          // So dropped entry will point to the 'proper base' and tell the user.
          CString cs_msg;
          cs_msg.Format(IDS_DDBASEISALIAS, sxgroup, sxtitle, sxuser);
          gmb.AfxMessageBox(cs_msg, NULL, MB_OK);
        } else
        if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
          // Only normal or alias base allowed as target
          CString cs_msg;
          cs_msg.Format(IDS_ABASEINVALID, sxgroup, sxtitle, sxuser);
          gmb.AfxMessageBox(cs_msg, NULL, MB_OK);
          continue;
        }
        Command *pcmd = AddDependentEntryCommand::Create(&m_core, pl.base_uuid,
                                                         ci_temp.GetUUID(),
                                                         CItemData::ET_ALIAS);
        pmulticmds->Add(pcmd);
        ci_temp.SetPassword(L"[Alias]");
        ci_temp.SetAlias();
      } else
      if (pl.InputType == CItemData::ET_SHORTCUT) {
        ItemListIter iter = m_core.Find(pl.base_uuid);
        ASSERT(iter != End());
        if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_SHORTCUTBASE) {
          // Only normal or shortcut base allowed as target
          CString cs_msg;
          cs_msg.Format(IDS_SBASEINVALID, sxgroup, sxtitle, sxuser);
          gmb.AfxMessageBox(cs_msg, NULL, MB_OK);
          continue;
        }
        Command *pcmd = AddDependentEntryCommand::Create(&m_core,
                                                         pl.base_uuid,
                                                         ci_temp.GetUUID(),
                                                         CItemData::ET_SHORTCUT);
        pmulticmds->Add(pcmd);
        ci_temp.SetPassword(L"[Shortcut]");
        ci_temp.SetShortcut();
      }
    } else
    if (pl.ibasedata == 0) {
      // Password NOT in alias/shortcut format
      ci_temp.SetNormal();
    } else
    if (pl.ibasedata < 0) {
      // Password in alias/shortcut format AND base entry does not exist or multiple possible
      // base entries exit.
      // Note: As more entries are added, what was "not exist" may become "OK",
      // "no unique exists" or "multiple exist".
      // Let the code that processes the possible aliases after all have been added sort this out.
      if (pl.InputType == CItemData::ET_ALIAS) {
        Possible_Aliases.push_back(ci_temp.GetUUID());
      } else
      if (pl.InputType == CItemData::ET_SHORTCUT) {
        Possible_Shortcuts.push_back(ci_temp.GetUUID());
        bAddToViews = false;
      }
    }
    ci_temp.SetStatus(CItemData::ES_ADDED);
    // Add to pwlist
    Command *pcmd = AddEntryCommand::Create(&m_core, ci_temp);
    if (!bAddToViews) {
      // ONLY Add to pwlist and NOT to Tree or List views
      // After the call to AddDependentEntries for shortcuts, check if still
      // in password list and, if so, then add to Tree + List views
      pcmd->SetNoGUINotify();
    }
    pmulticmds->Add(pcmd);
  } // iteration over in_oblist

  // Now try to add aliases/shortcuts we couldn't add in previous processing
  Command *pcmdA = AddDependentEntriesCommand::Create(&m_core,
                                                      Possible_Aliases, NULL,
                                                      CItemData::ET_ALIAS,
                                                      CItemData::PASSWORD);
  pmulticmds->Add(pcmdA);
  Command *pcmdS = AddDependentEntriesCommand::Create(&m_core,
                                                      Possible_Shortcuts, NULL, 
                                                      CItemData::ET_SHORTCUT,
                                                      CItemData::PASSWORD);
  pmulticmds->Add(pcmdS);
  Execute(pmulticmds);

  // Some shortcuts may have been deleted from the database as base does not exist
  // Tidy up Tree/List
  UUIDVectorIter paiter;
  ItemListIter iter;
  for (paiter = Possible_Shortcuts.begin();
       paiter != Possible_Shortcuts.end(); paiter++) {
    iter = m_core.Find(*paiter);
    if (iter != End()) {
      // Still in pwlist - NOW add to Tree and List views
      InsertItemIntoGUITreeList(m_core.GetEntry(iter));
    }
  }

  // Clear set
  setGTU.clear();

  SetChanged(Data);
  FixListIndexes();
  RefreshViews();

  if (!sxEntriesWithNewNamedPolicies.empty()) {
    // A number of entries had a similar named password policy but with
    // different settings to those in this database.
    // Tell user
    CGeneralMsgBox gmb;
    CString cs_title, cs_msg;
    cs_title.LoadString(IDS_CHANGED_POLICIES);
    cs_msg.Format(IDSC_ENTRIES_POLICIES, sxDD_DateTime.c_str(),
                  sxEntriesWithNewNamedPolicies.c_str());
    gmb.MessageBox(cs_msg, cs_title, MB_OK);
  }
}

LRESULT DboxMain::OnDragAutoType(WPARAM wParam, LPARAM /* lParam */)
{
  const CItemData *pci = reinterpret_cast<const CItemData *>(wParam);

  AutoType(*pci);
  return 0L;
}

LRESULT DboxMain::OnToolBarFindMessage(WPARAM /* wParam */, LPARAM /* lParam */)
{
  // Called when user types into the Find search edit control on the Find Toolbar
  // and presses enter.
  OnToolBarFind();
  return 0L;
}

void DboxMain::OnToolBarFind()
{
  // Called when the user presses the Find button on the Find Toolbar or
  // when they press enter when the search string has focus or
  // when they press F3
  if (GetKeyState(VK_SHIFT) & 0x8000) {
    // User clicked on Toolbar button with Shift key down
    OnToolBarFindUp();
    return;
  }
  m_FindToolBar.SetSearchDirection(FIND_DOWN);
  m_FindToolBar.Find();
}

void DboxMain::OnToolBarFindUp()
{
  // Called when the user presses the Find button on the Find Toolbar or
  // when they press enter when the search string has focus or
  // when they press Shift+F3
  m_FindToolBar.SetSearchDirection(FIND_UP);
  m_FindToolBar.Find();
}
