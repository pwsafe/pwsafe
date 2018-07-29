/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "CompareWithSelectDlg.h"
#include "ShowCompareDlg.h"
#include "ViewAttachmentDlg.h"

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

extern const wchar_t *GROUP_SEP2;

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
  
  PWSprefs *prefs = PWSprefs::GetInstance();

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
          m_TreeViewGroup = m_mapTreeItemToGroup[ti];
        }
      }
    } else { // list view
      // XXX TBD - get group name of currently selected list entry
    }
  }

  CAddEdit_PropertySheet *pAddEntryPSH(NULL);
  
  // Try Tall version
  pAddEntryPSH = new CAddEdit_PropertySheet(IDS_ADDENTRY, this, &m_core, NULL, &ci, true,  L"");

  if (prefs->GetPref(PWSprefs::UseDefaultUser)) {
    pAddEntryPSH->SetUsername(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
  }

  pAddEntryPSH->SetGroup(m_TreeViewGroup);

  // Remove Apply button
  pAddEntryPSH->m_psh.dwFlags |= PSH_NOAPPLYNOW;

  INT_PTR rc = pAddEntryPSH->DoModal();

  if (rc < 0) {
    // Try again with Wide version
    delete pAddEntryPSH;
    pAddEntryPSH = new CAddEdit_PropertySheet(IDS_ADDENTRY, this, &m_core, NULL, &ci, false,  L"");

    if (prefs->GetPref(PWSprefs::UseDefaultUser)) {
      pAddEntryPSH->SetUsername(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
    }

    pAddEntryPSH->SetGroup(m_TreeViewGroup);

    // Remove Apply button
    pAddEntryPSH->m_psh.dwFlags |= PSH_NOAPPLYNOW;
  
    rc = pAddEntryPSH->DoModal(); 
  }

  m_TreeViewGroup = L""; // for next time

  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    CSecString &sxUsername = pAddEntryPSH->GetUsername();

    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

    //Check if they wish to set a default username
    if (!prefs->GetPref(PWSprefs::UseDefaultUser) &&
        (prefs->GetPref(PWSprefs::QuerySetDef)) &&
        (!sxUsername.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_defaultusername.Format(IDS_SETUSERNAME,
            static_cast<LPCWSTR>((const CString&)sxUsername));

      INT_PTR rc2 = defDlg.DoModal();

      if (rc2 == IDOK) {
        // Initialise a copy of the DB preferences
        prefs->SetupCopyPrefs();

        // Update Copy with new values
        prefs->SetPref(PWSprefs::UseDefaultUser, true, true);
        prefs->SetPref(PWSprefs::DefaultUsername, sxUsername, true);

        // Set new DB preferences String value (from Copy)
        StringX sxNewDBPrefsString(prefs->Store(true));

        Command *pcmd = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
        pmulticmds->Add(pcmd);
      }
    }

    // Save to find it again
    const pws_os::CUUID newentry_uuid = ci.GetUUID();

    // Add the entry
    ci.SetStatus(CItemData::ES_ADDED); 
    StringX sxGroup = ci.GetGroup();
    if (m_core.IsEmptyGroup(sxGroup)) {
      // It was an empty group - better delete it
      pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, sxGroup,
                      DBEmptyGroupsCommand::EG_DELETE));
    }

    pws_os::CUUID baseUUID(pws_os::CUUID::NullUUID());
    if (pAddEntryPSH->GetIBasedata() != 0)  // creating an alias
      baseUUID = pAddEntryPSH->GetBaseUUID();

    pmulticmds->Add(AddEntryCommand::Create(&m_core, ci, baseUUID,
                                            pAddEntryPSH->GetAtt()));

    // Do it
    Execute(pmulticmds);

    if (m_core.GetNumEntries() == 1) {
      // For some reason, when adding the first entry, it is not visible!
      m_ctlItemTree.SetRedraw(TRUE);
    }

    SortListView();
    m_ctlItemList.SetFocus();

    // Find the new entry again as DisplayInfo now updated
    ItemListIter iter = m_core.Find(newentry_uuid);
    UpdateToolBarForSelectedItem(&iter->second);

    // Now select item
    DisplayInfo *pdi = GetEntryGUIInfo(iter->second);
    ASSERT(pdi->list_index != -1);
    SelectEntry(pdi->list_index);

    m_RUEList.AddRUEntry(newentry_uuid);

    // May need to update menu/toolbar if database was previously empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);

    if (m_ctlItemTree.IsWindowVisible())
      m_ctlItemTree.SetFocus();
    else
      m_ctlItemList.SetFocus();

    ChangeOkUpdate();
  } // rc == OK
  
  // Delete Add Property Sheet
  delete pAddEntryPSH;
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

  INT_PTR rc = dlg_createshortcut.DoModal();

  if (rc == IDOK) {
    CreateShortcutEntry(pci, dlg_createshortcut.m_group,
                        dlg_createshortcut.m_title,
                        dlg_createshortcut.m_username, sxNewDBPrefsString);

    // Ensure selected item looks selected as focus may have been lost
    if (m_ctlItemTree.IsWindowVisible())
      m_ctlItemTree.SetFocus();
    else
      m_ctlItemList.SetFocus();
  }
}

void DboxMain::CreateShortcutEntry(CItemData *pci, const StringX &sx_group,
                                   const StringX &sx_title, const StringX &sx_user,
                                   StringX &sxNewDBPrefsString)
{
  ASSERT(pci != NULL);

  CItemData ci_temp;
  ci_temp.SetGroup(sx_group);
  ci_temp.SetTitle(sx_title);
  ci_temp.SetUser(sx_user);
  ci_temp.SetPassword(L"[Shortcut]");

  // call before setting to shortcut so that it can be moved to correct place
  ci_temp.CreateUUID();
  ci_temp.SetShortcut();

  time_t t;
  time(&t);
  ci_temp.SetCTime(t);
  ci_temp.SetXTime((time_t)0);
  ci_temp.SetStatus(CItemData::ES_ADDED);

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);
  if (!sxNewDBPrefsString.empty()) {
    Command *pcmd = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
    pmulticmds->Add(pcmd);
  }

  if (IsEmptyGroup(sx_group)) {
    Command *pcmd = DBEmptyGroupsCommand::Create(&m_core, sx_group,
                                                 DBEmptyGroupsCommand::EG_DELETE);
    pmulticmds->Add(pcmd);
  }

  Command *pcmd = AddEntryCommand::Create(&m_core, ci_temp, pci->GetUUID());
  pmulticmds->Add(pcmd);

  // Do it
  Execute(pmulticmds);

  // Update base item's graphic
  ItemListIter iter = m_core.Find(pci->GetUUID());
  if (iter != End())
    UpdateEntryImages(iter->second);

  m_ctlItemList.SetFocus();

  m_RUEList.AddRUEntry(ci_temp.GetUUID());

  ChangeOkUpdate();
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
        m_TreeViewGroup = m_mapTreeItemToGroup[ti];
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
      // Format as per Windows Explorer "New Folder"/"New Folder (n)"
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
    if (hItem == m_hBase) // we're not interested in top
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

  bool bRefresh(true), bState, bChanged(false);
  // Get selected group
  HTREEITEM ti = m_ctlItemTree.GetSelectedItem();

  // Verify that it is a group
  ASSERT((CItemData *)m_ctlItemTree.GetItemData(ti) == NULL);

  // Get complete group name
  StringX sxCurrentPath = m_mapTreeItemToGroup[ti]; // e.g., a.b.c
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
  bState = (m_ctlItemTree.GetItemState(ti, TVIS_EXPANDED) & TVIS_EXPANDED) != 0;
  bVNodeStates.push_back(bState);

  // Get all of the children
  if (m_ctlItemTree.ItemHasChildren(ti)) {
    DupGroupFunctor dgf(ti);
    m_ctlItemTree.Iterate(ti, dgf); // collects children into a handy vector

    //  Create 3 multi-commands to first add all normal or base entries and then a second
    //  to add any aliases and shortcuts and finally to add any sub-empty groups
    MultiCommands *pmulti_cmd_base = MultiCommands::Create(&m_core);
    MultiCommands *pmulti_cmd_deps = MultiCommands::Create(&m_core);
    MultiCommands *pmulti_cmd_egrps = MultiCommands::Create(&m_core);

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

        // Set up copy
        CItemData ci2(*pci);
        ci2.CreateUUID();
        ci2.SetStatus(CItemData::ES_ADDED);
        ci2.SetProtected(false);

        // Set new group
        StringX subPath =  pci->GetGroup();
        ASSERT(subPath.length() >= grplen);
        subPath =  subPath.substr(grplen);
        const StringX sxThisEntryNewGroup = sxNewPath + subPath;
        ci2.SetGroup(sxThisEntryNewGroup);

        // Remove any keyboard shortcut otherwise it will be doubly assigned (not allowed!)
        ci2.SetKBShortcut(0);

        if (pci->IsBase()) {
          mapOldToNewBaseUUIDs.insert(std::make_pair(pci->GetUUID(),
                                                     ci2.GetUUID()));
        } // pci->IsBase()

        // Make copy normal to begin with - if it has dependents then when those
        // are added then its entry type will automatically be changed
        ci2.SetNormal();

        // Set duplication times as per FR819
        ci2.SetDuplicateTimes(*pci);

        Command *pcmd = AddEntryCommand::Create(&m_core, ci2);
        pcmd->SetNoGUINotify();
        pmulti_cmd_base->Add(pcmd);
      } else { // pci == NULL -> This is a node: save its expanded/collapsed state
        bState = (m_ctlItemTree.GetItemState(hNextItem, TVIS_EXPANDED) & TVIS_EXPANDED) != 0;
        bVNodeStates.push_back(bState);

        // Get group name & check if empty
        StringX subPath = m_mapTreeItemToGroup[hNextItem]; // e.g., a.b.c
        if (IsEmptyGroup(subPath)) {
          ASSERT(subPath.length() >= grplen);
          subPath = subPath.substr(grplen);
          StringX sxThisEntryNewGroup = sxNewPath + subPath;

          // Get command to add empty group
          Command *pcmd = DBEmptyGroupsCommand::Create(&m_core, sxThisEntryNewGroup,
            DBEmptyGroupsCommand::EG_ADD);
          pcmd->SetNoGUINotify();
          pmulti_cmd_egrps->Add(pcmd);
        }
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
          ci2.CreateUUID();
          ci2.SetStatus(CItemData::ES_ADDED);
          ci2.SetProtected(false);

          // Remove any keyboard shortcut otherwise it will be doubly assigned (not allowed!)
          ci2.SetKBShortcut(0);

          // Set new group
          StringX subPath =  pci->GetGroup();
          ASSERT(subPath.length() >= grplen);
          subPath =  subPath.substr(grplen);
          const StringX sxThisEntryNewGroup = sxNewPath + subPath;
          ci2.SetGroup(sxThisEntryNewGroup);

          if (pci->IsAlias()) {
            ci2.SetAlias();
          } else {
            ci2.SetShortcut();
          }

          // Set duplication times as per FR819
          ci2.SetDuplicateTimes(*pci);

          Command *pcmd = NULL;
          const CItemData *pbci = GetBaseEntry(pci);
          ASSERT(pbci != NULL);

          StringX sxtmp;
          citer = mapOldToNewBaseUUIDs.find(pbci->GetUUID());
          CUUID baseUUID;
          if (citer != mapOldToNewBaseUUIDs.end()) {
            // Base is in duplicated group - use base's copy
            baseUUID = citer->second;
          } else {
            // Base not in duplicated group - use old base
            baseUUID = pbci->GetUUID();
          } // where's the base?

          pcmd = AddEntryCommand::Create(&m_core, ci2, baseUUID);
          pcmd->SetNoGUINotify();
          pmulti_cmd_deps->Add(pcmd);
        } // pci != NULL
      } // for
    } // bDependentsExist

    // Define the Undo & Redo commands that would bracket any actions performed
    Command *pcmd_undo(NULL), *pcmd_redo(NULL);

    // We either do only one set of multi-commands, none of them or both of them
    // as one multi-command (normals/bases first followed by dependents) and possibly
    // add empty groups
    const int iDoExec = (pmulti_cmd_base->IsEmpty() ? 0 : 1) +
                        (pmulti_cmd_deps->IsEmpty() ? 0 : 2);
    
    if (iDoExec != 0 || !pmulti_cmd_egrps->IsEmpty()) {
      pcmd_undo = UpdateGUICommand::Create(&m_core, UpdateGUICommand::WN_UNDO,
                                            UpdateGUICommand::GUI_UNDO_IMPORT);
      pcmd_redo = UpdateGUICommand::Create(&m_core, UpdateGUICommand::WN_EXECUTE_REDO,
                                            UpdateGUICommand::GUI_REDO_IMPORT);
    }
   
    switch (iDoExec) {
      case 0:
        // Do nothing unless there are empty groups
        if (!pmulti_cmd_egrps->IsEmpty()) {
          pmulti_cmd_egrps->Insert(pcmd_undo);
          pmulti_cmd_egrps->Add(pcmd_redo);
          Execute(pmulti_cmd_egrps);
          bChanged = true;
        } else
          bRefresh = false;
        break;
      case 1:
        // Only normal/base entries
        pmulti_cmd_base->Insert(pcmd_undo);
        if (!pmulti_cmd_egrps->IsEmpty())
          pmulti_cmd_base->Add(pmulti_cmd_egrps);
        pmulti_cmd_base->Add(pcmd_redo);
        Execute(pmulti_cmd_base);
        bChanged = true;
        break;
      case 2:
        // Only dependents
        pmulti_cmd_deps->Insert(pcmd_undo);
        if (!pmulti_cmd_egrps->IsEmpty())
          pmulti_cmd_deps->Add(pmulti_cmd_egrps);
        pmulti_cmd_deps->Add(pcmd_redo);
        Execute(pmulti_cmd_deps);
        bChanged = true;
        break;
      case 3:
      {
        // Both - normal entries/bases first then dependents
        MultiCommands *pmulti_cmds = MultiCommands::Create(&m_core);
        pmulti_cmds->Add(pcmd_undo);
        pmulti_cmds->Add(pmulti_cmd_base);
        pmulti_cmds->Add(pmulti_cmd_deps);

        if (!pmulti_cmd_egrps->IsEmpty()) {
          pmulti_cmds->Add(pmulti_cmd_egrps);
        }

        pmulti_cmds->Add(pcmd_redo);
        Execute(pmulti_cmds);
        bChanged = true;
        break;
      }
      default:
        ASSERT(0);
    }

    // If we didn't populate the multi-commands, delete them
    if (pmulti_cmd_base->IsEmpty())
      delete pmulti_cmd_base;
    if (pmulti_cmd_deps->IsEmpty())
      delete pmulti_cmd_deps;
    if (pmulti_cmd_egrps->IsEmpty())
      delete pmulti_cmd_egrps;
  } else { // !m_ctlItemTree.ItemHasChildren(ti)
    // User is duplicating an empty group - just add it
    HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);
    HTREEITEM ng_ti = m_ctlItemTree.InsertItem(sxNewPath.c_str(), parent, TVI_SORT);

    if (IsEmptyGroup(sxCurrentPath))
      m_ctlItemTree.SetItemImage(ng_ti, CPWTreeCtrl::EMPTY_GROUP, CPWTreeCtrl::EMPTY_GROUP);
    else
      m_ctlItemTree.SetItemImage(ng_ti, CPWTreeCtrl::GROUP, CPWTreeCtrl::GROUP);

    m_mapGroupToTreeItem[sxNewPath] = ng_ti;
    m_mapTreeItemToGroup[ng_ti] = sxNewPath;

    MultiCommands *pmulti_cmds = MultiCommands::Create(&m_core);
    Command *pcmd(NULL), *pcmd_undo(NULL), *pcmd_redo(NULL);

    // Now get commands to allow Undo/Redo
    pcmd_undo = UpdateGUICommand::Create(&m_core, UpdateGUICommand::WN_UNDO,
      UpdateGUICommand::GUI_UNDO_IMPORT);
    pcmd_redo = UpdateGUICommand::Create(&m_core, UpdateGUICommand::WN_EXECUTE_REDO,
      UpdateGUICommand::GUI_REDO_IMPORT);

    // Get command to add empty group
    pcmd = DBEmptyGroupsCommand::Create(&m_core, sxNewPath,
      DBEmptyGroupsCommand::EG_ADD);
    pmulti_cmds->Add(pcmd_undo);
    pmulti_cmds->Add(pcmd);
    pmulti_cmds->Add(pcmd_redo);
    Execute(pmulti_cmds);
    bChanged = true;
    bRefresh = true;
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

  if (bChanged) {
    ChangeOkUpdate();
  }
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
      pci = (CItemData *)m_ctlItemList.GetItemData((int)(INT_PTR)pos - 1);
    }
  }
  if (pci != NULL) {
    // Entry
    ASSERT(nID == ID_MENUITEM_PROTECT || nID == ID_MENUITEM_UNPROTECT);
    Command *pcmd = UpdateEntryCommand::Create(&m_core, *pci,
                                               CItemData::PROTECTED,
                                               nID == ID_MENUITEM_UNPROTECT ? L"0" : L"1");
    Execute(pcmd);

    ChangeOkUpdate();
  } else {
    // Group
    ASSERT(nID == ID_MENUITEM_PROTECTGROUP || nID == ID_MENUITEM_UNPROTECTGROUP);
    ChangeSubtreeEntriesProtectStatus(nID);
  }
}

void DboxMain::OnCompareEntries()
{
  CItemData *pci(NULL), *pci_other(NULL);

  // Not yet supported in Tree view unless command line flag present.
  if (!m_IsListView && !m_bCompareEntries)
    return;

  if (m_IsListView) {
    // Only called if 2 entries selected
    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    int nItem = m_ctlItemList.GetNextSelectedItem(pos);
    pci = (CItemData *)m_ctlItemList.GetItemData(nItem);
    nItem = m_ctlItemList.GetNextSelectedItem(pos);
    pci_other = (CItemData *)m_ctlItemList.GetItemData(nItem);
  } else {
    // Not yet supported in Tree view - get user to select other item
    HTREEITEM hStartItem = m_ctlItemTree.GetSelectedItem();
    if (hStartItem != NULL) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(hStartItem);
    }

    if (pci != NULL) {
      // Entry - selected - shouldn't be called when group is selected
      // Now get the other entry
      CString csProtect = Fonts::GetInstance()->GetProtectedSymbol().c_str();
      CString csAttachment = Fonts::GetInstance()->GetAttachmentSymbol().c_str();

      CCompareWithSelectDlg dlg(this, pci, &m_core, csProtect, csAttachment);

      if (dlg.DoModal() == IDOK) {
        // Get UUID of the entry
        CUUID otherUUID = dlg.GetUUID();
        if (otherUUID == CUUID::NullUUID())
          return;

        ItemListIter pos = Find(otherUUID);
        if (pos == End())
          return;
 
        pci_other = &pos->second;
      } else
        return;
    }
  }

  if (pci != NULL && pci_other != NULL) {
    // Entries - selected - shouldn't be called when group is selected
    CShowCompareDlg showdlg(pci, pci_other, this, false);
    showdlg.DoModal();
  }
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

  if (!pmulticmds->IsEmpty()) {
    Execute(pmulticmds);
    ChangeOkUpdate();
  } else { // IsEmpty
    delete pmulticmds;
  }
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
  if (m_core.GetNumEntries() == 0 &&
      m_core.GetEmptyGroups().empty()) // easiest way to avoid asking stupid questions...
    return;

  bool bAskForDeleteConfirmation = !(PWSprefs::GetInstance()->
                                     GetPref(PWSprefs::DeleteQuestion));
  bool dodelete = true;
  int num_children = 0;
  m_sxOriginalGroup = L""; // Needed if deleting a groups due to Delete recusrsion

  // Entry to be selected after deletion
  bool bSelectNext(false), bSelectPrev(false);
  pws_os::CUUID nextUUID(pws_os::CUUID::NullUUID()), prevUUID(pws_os::CUUID::NullUUID());
  StringX sxNextGroup(L""), sxPrevGroup(L"");

  // Find number of child items, ask for confirmation if > 0
  StringX sxGroup(L""), sxTitle(L""), sxUser(L"");
  CItemData *pci(NULL);
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hStartItem = m_ctlItemTree.GetSelectedItem();

    // Check that it wasn't the bottom entry in the tree (can't select next)
    HTREEITEM hNextSelectedItem = m_ctlItemTree.GetNextItem(hStartItem, TVGN_NEXTVISIBLE);
    if (hNextSelectedItem != NULL) {
      bSelectNext = true;
      CItemData *pci_next = (CItemData *)m_ctlItemTree.GetItemData(hNextSelectedItem);
      if (pci_next == NULL) {
        // It is a group - save full path
        sxNextGroup = m_ctlItemTree.GetGroup(hNextSelectedItem);
      } else {
        nextUUID = pci_next->GetUUID();
      }
    }

    // Check that it wasn't the top entry in the tree (can't select previous)
    HTREEITEM hPrevSelectedItem = m_ctlItemTree.GetNextItem(hStartItem, TVGN_PREVIOUSVISIBLE);
    if (hPrevSelectedItem != NULL) {
      bSelectPrev = true;
      CItemData * pci_prev = (CItemData *)m_ctlItemTree.GetItemData(hPrevSelectedItem);
      if (pci_prev == NULL) {
        // It is a group - save full path
        sxPrevGroup = m_ctlItemTree.GetGroup(hPrevSelectedItem);
      } else {
        prevUUID = pci_prev->GetUUID();
      }
    }

    if (hStartItem != NULL) {
      if (m_ctlItemTree.GetItemData(hStartItem) == NULL) {  // group node
        // ALWAYS ask if deleting a group - unless it is empty or
        // only contains empty groups!
        m_sxOriginalGroup = m_mapTreeItemToGroup[hStartItem];
        num_children = m_ctlItemTree.CountLeafChildren(hStartItem);
        bAskForDeleteConfirmation = num_children != 0;

        // If the user just wants to delete an empty group or
        // a group only containing empty groups - delete!
        if (num_children == 0) {
          MultiCommands *pmcmd = MultiCommands::Create(&m_core);
          const StringX sxPath2 = m_sxOriginalGroup + L".";
          const int len = (int)sxPath2.length();
          std::vector<StringX> vEmptyGroups = m_core.GetEmptyGroups();
          for (size_t i = 0; i < vEmptyGroups.size(); i++) {
            if (vEmptyGroups[i] == m_sxOriginalGroup || vEmptyGroups[i].substr(0, len) == sxPath2) {
              pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, vEmptyGroups[i],
                DBEmptyGroupsCommand::EG_DELETE));
            }
          }

          // Was this empty group the only child of its parent?
          // If so - make it empty too.
          HTREEITEM parent = m_ctlItemTree.GetParentItem(hStartItem);
          num_children = m_ctlItemTree.CountChildren(parent);
          if (num_children == 1 && parent != NULL && parent != TVI_ROOT) {
            StringX sxPath= m_mapTreeItemToGroup[parent];
            pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxPath,
              DBEmptyGroupsCommand::EG_ADD));
          }

          // Now do it
          Execute(pmcmd);

          // Clear current group as just deleted
          m_TreeViewGroup = L"";

          ChangeOkUpdate();
          goto Select_Next_Prev;
        }
      } else {
        pci = (CItemData *)m_ctlItemTree.GetItemData(hStartItem);
      }
    }
  } else {
    // Ignore if more than one selected - List view only
    if (m_ctlItemList.GetSelectedCount() > 1)
      return;

    POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
    if (pos != NULL) {
      const int nItem = m_ctlItemList.GetNextSelectedItem(pos);
      pci = (CItemData *)m_ctlItemList.GetItemData(nItem);

      // Check that it wasn't the bottom entry in the list (can't select next)
      if (nItem < (m_ctlItemList.GetItemCount() - 1)) {
        bSelectNext = true;
        CItemData *pci_next = (CItemData *)m_ctlItemList.GetItemData(nItem + 1);
        nextUUID = pci_next->GetUUID();
      }

      // Check that it wasn't the top entry in the list (can't select previous)
      if (nItem > 0) {
        bSelectPrev = true;
        CItemData *pci_prev = (CItemData *)m_ctlItemList.GetItemData(nItem - 1);
        prevUUID = pci_prev->GetUUID();
      }
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
    // Here we delete an entry or a group containing entries
    MultiCommands *pmcmd = MultiCommands::Create(&m_core);
    Delete(pmcmd);

    // If we were originally deleting a group with entries, we now need to delete
    // resulting empty groups (after all sub-entries deleted) and the original group
    if (!m_sxOriginalGroup.empty()) {
      const StringX sxPath2 = m_sxOriginalGroup + L".";
      const int len = (int)sxPath2.length();
      std::vector<StringX> vEmptyGroups = m_core.GetEmptyGroups();
      for (size_t i = 0; i < vEmptyGroups.size(); i++) {
        if (vEmptyGroups[i] == m_sxOriginalGroup || vEmptyGroups[i].substr(0, len) == sxPath2) {
          pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, vEmptyGroups[i],
            DBEmptyGroupsCommand::EG_DELETE));
        }
      }
    }

    // Now do it
    if (!pmcmd->IsEmpty()) {
      Execute(pmcmd);
    } else {
      delete pmcmd;
    }

    // Only refresh views if an entry or a non-empty group was deleted
    // If we refresh when deleting an empty group, the user will lose all
    // other empty groups
    if (m_bFilterActive || pci != NULL || (pci == NULL && num_children > 0))
      RefreshViews();

    ChangeOkUpdate();
  }

Select_Next_Prev:
  // Now reselect the next/previous item
  CItemData *pci_select(NULL);
  DisplayInfo *pdi;
  HTREEITEM hItem(NULL);
  int item(-1);
  bool bFoundSelected(false);

  if (bSelectNext) {
    // Have to be careful if deleting a group as the next item might be a member
    // of that group and so no longer there!
    if (nextUUID != pws_os::CUUID::NullUUID()) {
      ItemListIter iter = Find(nextUUID);
      if (iter != End()) {
        pdi = GetEntryGUIInfo(iter->second);
        hItem = pdi->tree_item;
        item = pdi->list_index;
        pci_select = &iter->second;
        bFoundSelected = true;
      }
    } else {
      if (!sxNextGroup.empty() &&
          m_mapGroupToTreeItem.find(sxNextGroup) != m_mapGroupToTreeItem.end()) {
        hItem = m_mapGroupToTreeItem[sxNextGroup];
        bFoundSelected = true;
      }
    }
  }   
  if (bSelectPrev && !bFoundSelected) {
    if (prevUUID != pws_os::CUUID::NullUUID()) {
      ItemListIter iter = Find(prevUUID);
      if (iter != End()) {
        pdi = GetEntryGUIInfo(iter->second);
        hItem = pdi->tree_item;
        item = pdi->list_index;
        pci_select = &iter->second;
        bFoundSelected = true;
      }
    } else {
      if (!sxPrevGroup.empty() &&
          m_mapGroupToTreeItem.find(sxPrevGroup) != m_mapGroupToTreeItem.end()) {
        hItem = m_mapGroupToTreeItem[sxPrevGroup];
        bFoundSelected = true;
      }
    }
  }

  // Now select the deleted item's (group's) next or previous entry
  if (bFoundSelected) {
    if (m_ctlItemTree.IsWindowVisible() && hItem != NULL) {
      m_ctlItemTree.SelectItem(hItem);
    } else if (item != -1) {
      m_ctlItemList.SetItemState(item, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }
  }

  // Update Toolbar & Dragbar
  UpdateToolBarForSelectedItem(pci_select);
}

void DboxMain::Delete(MultiCommands *pmcmd)
{
  // "Top level" element delete:
  // 1. Sets up Command mechanism
  // 2. Calls group or element Delete method, as appropriate
  // 3. Executes, updates UI.

  CItemData *pci = getSelectedItem();

  if (pci != NULL) {
    // Easy option - just one entry
    DisplayInfo *pdi = GetEntryGUIInfo(*pci);

    bool bLastEntry = (m_ctlItemTree.GetNextSiblingItem(pdi->tree_item) == NULL) &&
                      (m_ctlItemTree.GetPrevSiblingItem(pdi->tree_item) == NULL);

    // Now delete single entry
    Command *pcmd = Delete(pci);

    // pcmd can by NULL if user says No to confirm delete of a base entry!
    if (pcmd != NULL) {
      pmcmd->Add(pcmd);

      // Check if last entry in group and if so - add group to empty groups
      if (bLastEntry) {
        StringX sxGroup = pci->GetGroup();
        if (!sxGroup.empty()) {
          // Only add group if not root
          pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxGroup,
            DBEmptyGroupsCommand::EG_ADD));
        }
      }
    }
  } else
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
    // Deleting a Group
    HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);
    // Only interested in children of the parent - not grand-children etc.
    const int numchildren = m_ctlItemTree.CountChildren(parent, false);

    // We need to collect bases and dependents separately, deleting the latter first
    // so that an undo will add bases first.
    std::vector<Command *> vbases, vdeps, vemptygrps;

    // Generate commands
    // Note - as deleting a group - don't make this top level group empty
    Delete(ti, vbases, vdeps, vemptygrps, true);

    // Delete normal and dependent entries
    std::for_each(vdeps.begin(), vdeps.end(),
                  [&] (Command *cmd) {pmcmd->Add(cmd);});

    // Delete alias & shortcut bases
    std::for_each(vbases.begin(), vbases.end(),
                  [&] (Command *cmd) {pmcmd->Add(cmd);});

    // This will either delete the empty groups or convert non-empty groups
    // to empty ones once all entries have been deleted
    std::for_each(vemptygrps.begin(), vemptygrps.end(),
      [&](Command *cmd) {pmcmd->Add(cmd);});

    m_ctlItemTree.SelectItem(parent);

    // If this was the last group - make parent empty.
    if (numchildren == 1) {
      const StringX sxPath = m_ctlItemTree.GetGroup(parent);
      pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxPath,
        DBEmptyGroupsCommand::EG_ADD));
    }
    m_TreeViewGroup = L"";
  }
}

Command *DboxMain::Delete(const CItemData *pci)
{
  // Delete a single item of any type:
  // Normal, base, alias, shortcut...
  // ONLY called when deleting a group and all its entries
  ASSERT(pci != NULL);

  // ConfirmDelete asks for user confirmation
  // when deleting a shortcut or alias base.
  // Otherwise it just returns true
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

void DboxMain::Delete(HTREEITEM ti,
                      std::vector<Command *> &vbases,
                      std::vector<Command *> &vdeps,
                      std::vector<Command *> &vemptygrps,
                      bool bExcludeTopGroup)
{
  // Delete a group
  // Create a multicommand, iterate over tree's children,
  // throw everything into multicommand and be done with it.
  // Of course, this may recurse...

  if (ti == NULL)
    return; // bad bottoming out of recursion?

  if (m_ctlItemTree.IsLeaf(ti)) {
    // normal bottom out of recursion
    CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
    if (pci->IsBase() || pci->IsNormal())
      vbases.push_back(Delete(pci));
    else
      vdeps.push_back(Delete(pci));
  }

  // Here if we have a bona fide group
  ASSERT(ti != NULL && !m_ctlItemTree.IsLeaf(ti));
  
  StringX sxPath = m_mapTreeItemToGroup[ti];
  // Check if an Empty Group
  if (m_ctlItemTree.ItemHasChildren(ti) != 0 && !bExcludeTopGroup &&
    m_sxOriginalGroup == sxPath) {
    // Non-empty group - convert to empty if user want to keep it
    vemptygrps.push_back(DBEmptyGroupsCommand::Create(&m_core, sxPath,
      DBEmptyGroupsCommand::EG_ADD));
  }

  HTREEITEM cti = m_ctlItemTree.GetChildItem(ti);

  while (cti != NULL) {
    CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(cti);
    if (pci != NULL) {
      if (pci->IsBase())
        vbases.push_back(Delete(pci));
      else
        vdeps.push_back(Delete(pci));
    } else
      Delete(cti, vbases, vdeps, vemptygrps); // subgroup!

    cti = m_ctlItemTree.GetNextItem(cti, TVGN_NEXT);
  }
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
      if (m_ctlItemTree.WasLabelEdited())
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
    try {
      if (pci->IsShortcut()) {
        if (!m_bViaDCA) {
          EditShortcut(pci);
        } else {
          // Save entry UUID in case DB locked
          pws_os::CUUID entryuuid = pci->GetUUID();
          EditItem(GetBaseEntry(pci));

          // If DB was locked then the pci will not be valid and the number of entries will be zero
          // (otherwise shouldn't be since we were just editing one) - so use entry's UUID
          UpdateAccessTime(entryuuid);
        }
      }  else {
        // Save entry UUID in case DB locked
        pws_os::CUUID entryuuid = pci->GetUUID();
        EditItem(pci);

        // If DB was locked then the pci will not be valid and the number of entries will be zero
        // (otherwise shouldn't be since we were just editing one) - so use entry's UUID
        UpdateAccessTime(entryuuid);
      }
    } catch (CString &err) {
      CGeneralMsgBox gmb;
      gmb.MessageBox(err, NULL, MB_OK | MB_ICONERROR);
    }
  } else {
    // entry item not selected - perhaps here on Enter on tree item?
    // perhaps not the most elegant solution to improving non-mouse use,
    // but it works. If anyone knows how Enter/Return gets mapped to OnEdit,
    // let me know...
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        CItemData *pci_node = (CItemData *)m_ctlItemTree.GetItemData(ti);
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
  // The one exception is when the user wishes to View an entry from the comparison
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

  pci = NULL; // Set to NULL - use ci_original

  const UINT uicaller = pcore->IsReadOnly() ? IDS_VIEWENTRY : IDS_EDITENTRY;

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
  
  CAddEdit_PropertySheet *pEditEntryPSH(NULL);
  
  // Try Tall version
  pEditEntryPSH = new CAddEdit_PropertySheet(uicaller, this, pcore,
                                             &ci_original, &ci_edit,
                                             true, pcore->GetCurFile());

  if (bIsDefUserSet)
    pEditEntryPSH->SetDefUsername(sxDefUserValue.c_str());

  // Don't show Apply button if in R-O mode (View)
  if (uicaller == IDS_VIEWENTRY)
    pEditEntryPSH->m_psh.dwFlags |= PSH_NOAPPLYNOW;

  INT_PTR rc = pEditEntryPSH->DoModal();

  if (rc < 0) {
    // Try again with Wide version
    delete pEditEntryPSH;
    pEditEntryPSH = new CAddEdit_PropertySheet(uicaller, this, pcore,
                                               &ci_original, &ci_edit,
                                               false, pcore->GetCurFile());

    if (bIsDefUserSet)
      pEditEntryPSH->SetDefUsername(sxDefUserValue.c_str());

    // Don't show Apply button if in R-O mode (View)
    if (uicaller == IDS_VIEWENTRY)
      pEditEntryPSH->m_psh.dwFlags |= PSH_NOAPPLYNOW;
  
    rc = pEditEntryPSH->DoModal(); 
  }

  bool brc(false);
  if (rc == IDOK && uicaller == IDS_EDITENTRY && pEditEntryPSH->IsEntryModified()) {
    // Process user's changes.
    UpdateEntry(pEditEntryPSH);
    brc = true;
  } // rc == IDOK
  
  // Delete Edit Entry Property Sheet
  delete pEditEntryPSH;
  return brc;
}

LRESULT DboxMain::OnApplyEditChanges(WPARAM wParam, LPARAM )
{
  // Called if user does 'Apply' on the Add/Edit property sheet via
  // Windows Message PWS_MSG_EDIT_APPLY
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
  const CItemData *pci_original = pentry_psh->GetOriginalCI();
  CItemData ci_new(*pentry_psh->GetNewCI());

  // Most of the following code handles special cases of alias/shortcut/base
  // But the common case is simply to replace the original entry
  // with a new one having the edited values and the same uuid.
  MultiCommands *pmulticmds = MultiCommands::Create(pcore);
  Command *pcmd(NULL);

  // Determine if last entry in this group just in case the user changes the group
  DisplayInfo *pdi = GetEntryGUIInfo(*pci_original);
  bool bLastEntry = (m_ctlItemTree.GetNextSiblingItem(pdi->tree_item) == NULL) &&
                    (m_ctlItemTree.GetPrevSiblingItem(pdi->tree_item) == NULL);

  StringX newPassword = ci_new.GetPassword();

  CUUID original_base_uuid = CUUID::NullUUID();
  CUUID new_base_uuid = pentry_psh->GetBaseUUID();
  CUUID original_uuid = pci_original->GetUUID();

  StringX sxPWH = pci_original->GetPWHistory();
  bool bTemporaryChangeOfPWH(false), bAliasBecomingNormal(false);

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
      pcmd = AddDependentEntryCommand::Create(pcore, new_base_uuid,
                                                     original_uuid,
                                                     CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);

      // We can allow saving of the current password in the ALias PWH this time
      ci_new.SetPassword(L"[Alias]");
      ci_new.SetAlias();
      ci_new.SetBaseUUID(new_base_uuid);
    } else { // Still 'normal'
      ci_new.SetPassword(newPassword);
      ci_new.SetNormal();
    }
  } // Normal entry, password changed

  if (pentry_psh->GetOriginalEntrytype() == CItemData::ET_ALIAS) {
    // Original was an alias
    // Only remove alias from multimap if base has changed (either to another
    // base or none [alias now a normal entry].
    if (new_base_uuid != original_base_uuid) {
      // Its base has been changed or removed to become a normal entry again
      // RemoveDependentEntry also resets base to normal if no more dependents
      pcmd = RemoveDependentEntryCommand::Create(pcore, original_base_uuid,
                                                        original_uuid,
                                                        CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);

      // Password changed so might be an alias of another entry!
      if (pentry_psh->GetIBasedata() > 0) { // Still an alias
        pcmd = AddDependentEntryCommand::Create(pcore, new_base_uuid,
                                                       original_uuid,
                                                       CItemData::ET_ALIAS);
        pmulticmds->Add(pcmd);

        ci_new.SetAlias();
        ci_new.SetBaseUUID(new_base_uuid);
      } else { // No longer an alias
        // Temporarily disable password history so it doesn't have the special
        // password of [Alias] saved into it on reverting to normal
        bAliasBecomingNormal = true;
        if (!sxPWH.empty() && sxPWH.substr(0, 1) == L"1") {
          bTemporaryChangeOfPWH = true;
          sxPWH[0] = L'0';
          ci_new.SetPWHistory(sxPWH);
        }

        // Change password
        ci_new.SetPassword(newPassword);
        // Now set as a normal entry - this will also clear old base_uuid
        ci_new.SetNormal();
      } // Password changed
    } // base uuids changed
  } // Alias

  if (pentry_psh->GetOriginalEntrytype() == CItemData::ET_ALIASBASE &&
      pci_original->GetPassword() != newPassword) {
    // Original was a base but might now be an alias of another entry!
    if (pentry_psh->GetIBasedata() > 0) {
      // Now an alias
      // Make this one an alias
      pcmd = AddDependentEntryCommand::Create(pcore, new_base_uuid,
                                                     original_uuid,
                                                     CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);

      ci_new.SetAlias();
      ci_new.SetBaseUUID(new_base_uuid);

      // Move old aliases across in core multimap
      pcmd = MoveDependentEntriesCommand::Create(pcore, original_uuid,
                                                        new_base_uuid,
                                                        CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);

      // Now actually move the aliases
      const CUUID old_base_uuid = pci_original->GetUUID();
      UUIDVector tlist;
      m_core.GetAllDependentEntries(old_base_uuid, tlist, CItemData::ET_ALIAS);
      for (size_t idep = 0; idep < tlist.size(); idep++) {
        ItemListIter alias_iter = m_core.Find(tlist[idep]);
        CItemData ci_oldalias(alias_iter->second);
        CItemData ci_newalias(ci_oldalias);
        ci_newalias.SetBaseUUID(new_base_uuid);
        pcmd = EditEntryCommand::Create(pcore, ci_oldalias, ci_newalias);
        pmulticmds->Add(pcmd);
      }
    } else { // Still a base entry but with a new password
      ci_new.SetPassword(newPassword);
    }
  } // AliasBase with password changed

  if (pentry_psh->GetOriginalEntrytype() == CItemData::ET_SHORTCUTBASE &&
      pci_original->GetPassword() != newPassword) {
    // Original was a shortcut base and can only become a normal entry
    if (pentry_psh->GetIBasedata() > 0) {
      // Now an alias
      // Make this one an alias
      pcmd = AddDependentEntryCommand::Create(pcore, new_base_uuid,
                                                     original_uuid,
                                                     CItemData::ET_ALIAS);
      pmulticmds->Add(pcmd);

      ci_new.SetPassword(L"[Alias]");
      ci_new.SetAlias();
      ci_new.SetBaseUUID(new_base_uuid);

      // Delete shortcuts
      const CUUID old_base_uuid = pci_original->GetUUID();
      UUIDVector tlist;
      m_core.GetAllDependentEntries(old_base_uuid, tlist, CItemData::ET_SHORTCUT);
      for (size_t idep = 0; idep < tlist.size(); idep++) {
        ItemListIter shortcut_iter = m_core.Find(tlist[idep]);
        pcmd = DeleteEntryCommand::Create(&m_core, shortcut_iter->second);
        pmulticmds->Add(pcmd);
      }
    } else { // Still a base entry but with a new password
      ci_new.SetPassword(newPassword);
      ci_new.SetShortcutBase();
    }
  } // ShortcutBase with password changed

  // Update old base...
  iter = pcore->Find(original_base_uuid);
  if (iter != End())
    UpdateEntryImages(iter->second);

  // ... and the new base entry (only if different from the old one)
  if (new_base_uuid != pws_os::CUUID::NullUUID() && new_base_uuid != original_base_uuid) {
    iter = pcore->Find(new_base_uuid);
    if (iter != End())
      UpdateEntryImages(iter->second);
  }

  if (ci_new.IsDependent()) {
    ci_new.SetXTime((time_t)0);
    ci_new.SetPWPolicy(L"");
  }

  ci_new.SetStatus(CItemData::ES_MODIFIED);

  // Now actually do the edit!
  pcmd = EditEntryCommand::Create(pcore, *(pci_original), ci_new);
  pmulticmds->Add(pcmd);

  DisplayInfo *pdiold = GetEntryGUIInfo(*pci_original);
  ASSERT(pdiold != NULL);

  const HTREEITEM hItemOld = pdiold->tree_item;
  pws_os::Trace(L"UpdateEntry old %08x\n", hItemOld);

  const HTREEITEM hItemNew = pdiold->tree_item;

  ASSERT(GetEntryGUIInfo(ci_new) != nullptr);

  pws_os::Trace(L"UpdateEntry new %08x\n", hItemNew);

  // Restore PWH as it was before it became an alias if we had changed it
  if (bAliasBecomingNormal) {
    if (bTemporaryChangeOfPWH) {
      sxPWH[0] = L'1';
    }
    pcmd = UpdateEntryCommand::Create(pcore, ci_new,
                                      CItemData::PWHIST,
                                      sxPWH);
    pmulticmds->Add(pcmd);
  }

  const StringX &sxNewGroup = ci_new.GetGroup();
  if (m_core.IsEmptyGroup(sxNewGroup)) {
    // It was an empty group - better delete it
    pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, sxNewGroup,
      DBEmptyGroupsCommand::EG_DELETE));
  }

  // Check if group changed and last entry in group and, if so,
  // add original group to empty groups
  if (bLastEntry && pci_original->GetGroup() != sxNewGroup) {
    pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, pci_original->GetGroup(),
      DBEmptyGroupsCommand::EG_ADD));

    // If new group currently empty - it isn't now
    if (IsEmptyGroup(sxNewGroup)) {
      pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, sxNewGroup,
        DBEmptyGroupsCommand::EG_DELETE));
    }
  }

  // Do it
  Execute(pmulticmds, pcore);

  ChangeOkUpdate();

  // Order may have changed as a result of edit
  // Check if we need to though!
  if (m_ctlItemTree.MakeTreeDisplayString(*pci_original) != 
      m_ctlItemTree.MakeTreeDisplayString(ci_new)) {
    m_ctlItemTree.SortTree(TVI_ROOT);
  }

  SortListView();

  short sh_odca, sh_ndca;
  pci_original->GetDCA(sh_odca);
  ci_new.GetDCA(sh_ndca);
  if (sh_odca != sh_ndca)
    SetDCAText(&ci_new);

  UpdateToolBarForSelectedItem(&ci_new);

  // Password may have been updated and so not expired
  UpdateEntryImages(ci_new);

  // Update display if no longer passes filter criteria
  if (m_bFilterActive &&
      !m_FilterManager.PassesFiltering(ci_new, m_core)) {
      RefreshViews();
      return;
  }

  // Reselect entry, where-ever it may be
  iter = m_core.Find(original_uuid);
  if (iter != End()) {
    DisplayInfo *pnew_di = GetEntryGUIInfo(iter->second);
    SelectEntry(pnew_di->list_index);
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

  pci = NULL; // Set to NULL - use ci_original

  pws_os::CUUID entryuuid = ci_original.GetUUID();

  // Determine if last entry in this group just in case the user changes the group
  DisplayInfo *pdi = GetEntryGUIInfo(ci_original);
  bool bLastEntry = (m_ctlItemTree.GetNextSiblingItem(pdi->tree_item) == NULL) &&
                    (m_ctlItemTree.GetPrevSiblingItem(pdi->tree_item) == NULL);

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

    MultiCommands *pmulticmds = MultiCommands::Create(pcore);

    Command *pcmd_undo = UpdateGUICommand::Create(&m_core,
                                                          UpdateGUICommand::WN_UNDO,
                                                          UpdateGUICommand::GUI_REFRESH_ENTRY,
                                                          entryuuid);
    pmulticmds->Add(pcmd_undo);

    pmulticmds->Add(EditEntryCommand::Create(pcore, ci_original, ci_edit));

    // Check if group changed and last entry in group and, if so,
    // add original group to empty groups
    const StringX &sxNewGroup = ci_edit.GetGroup();
    if (bLastEntry && ci_original.GetGroup() != sxNewGroup) {
      pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, ci_original.GetGroup(),
        DBEmptyGroupsCommand::EG_ADD));

      // If new group currently empty - it isn't now
      if (IsEmptyGroup(sxNewGroup)) {
        pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, sxNewGroup,
          DBEmptyGroupsCommand::EG_DELETE));
      }
    }

    Command *pcmd_redo = UpdateGUICommand::Create(&m_core,
                                                          UpdateGUICommand::WN_REDO,
                                                          UpdateGUICommand::GUI_REFRESH_ENTRY,
                                                          entryuuid);

    pmulticmds->Add(pcmd_redo);

    // Do it
    Execute(pmulticmds, pcore);

    // DisplayInfo's copied and changed, get up-to-date version
    CItemData &cidp = pcore->GetEntry(pcore->Find(ci_original.GetUUID()));
    DisplayInfo *pnew_di = GetEntryGUIInfo(cidp);
    rc = SelectEntry(pnew_di->list_index);

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
    DisplayInfo *pdi = GetEntryGUIInfo(*pci);

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
    ci2.CreateUUID();
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetStatus(CItemData::ES_ADDED);
    ci2.SetProtected(false);

    // Remove any keyboard shortcut otherwise it will be doubly assigned (not allowed!)
    ci2.SetKBShortcut(0);

    pws_os::CUUID baseUUID = pws_os::CUUID::NullUUID();

    if (pci->IsDependent()) {
      const CItemData *pbci = GetBaseEntry(pci);
      ASSERT(pbci != NULL);
      baseUUID = pci->GetBaseUUID();
      const StringX sxtmp = L"[" +
        pbci->GetGroup() + L":" +
        pbci->GetTitle() + L":" +
        pbci->GetUser()  +
        L"]";
      ci2.SetPassword(sxtmp);
    } else if (pci->IsBase()) {
      ci2.SetNormal();
    }

    // Set duplication times as per FR819
    ci2.SetDuplicateTimes(*pci);

    Execute(AddEntryCommand::Create(&m_core, ci2, baseUUID));

    // so that InsertItemIntoGUITreeList will set new values
    pdi->list_index = -1;
    pdi->tree_item = 0;

    ItemListIter iter = m_core.Find(ci2.GetUUID());
    ASSERT(iter != m_core.GetEntryEndIter());

    InsertItemIntoGUITreeList(m_core.GetEntry(iter));
    FixListIndexes();

    m_RUEList.AddRUEntry(ci2.GetUUID());

    ChangeOkUpdate();
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

  if (DisplaySubsetDlg.DoModal() != IDCANCEL)
    UpdateAccessTime(uuid);
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
  // bSpecial's meaning depends on ft:
  //
  //   For "CItemData::PASSWORD": "bSpecial" == true means "Minimize after copy"
  //   For "CItemData::RUNCMD":   "bSpecial" == true means "Do NOT expand the Run command"
  if (SelItemOk() != TRUE)
    return;

  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);

  CItemData *pbci(NULL);
  const pws_os::CUUID uuid = pci->GetUUID();

  if (pci->IsDependent()) {
    pbci = GetBaseEntry(pci);
    ASSERT(pbci != NULL);
  }

  StringX sxData = pci->GetEffectiveFieldValue(ft, pbci);

  switch (ft) {
    case CItemData::PASSWORD:
    {
      //Remind the user about clipboard security
      CClearQuestionDlg clearDlg(this);
      if (clearDlg.AskQuestion() && clearDlg.DoModal() == IDCANCEL)
        return;
      if (bSpecial) {
        OnMinimize();
      }
      break;
    }
    case CItemData::USER:
      break;
    case CItemData::NOTES:
      break;
    case CItemData::URL:
    {
      StringX::size_type ipos;
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
      if (!bSpecial) {
        // Expand Run Command
        std::wstring errmsg;
        size_t st_column;
        bool bURLSpecial;

        sxData = PWSAuxParse::GetExpandedString(sxData,
                                                 m_core.GetCurFile(),
                                                 pci, pbci,
                                                 m_bDoAutoType,
                                                 m_sxAutoType,
                                                 errmsg, st_column, bURLSpecial);
        if (!errmsg.empty()) {
          CGeneralMsgBox gmb;
          CString cs_title(MAKEINTRESOURCE(IDS_RUNCOMMAND_ERROR));
          CString cs_errmsg;
          cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
          gmb.MessageBox(cs_errmsg, cs_title, MB_ICONERROR);
        }
      }
      break;
    case CItemData::EMAIL:
      break;
    default:
      ASSERT(0);
  }

  SetClipboardData(sxData);
  UpdateLastClipboardAction(ft);
  UpdateAccessTime(uuid);
}

void DboxMain::UpdateLastClipboardAction(const int iaction)
{
  // Note use of CItemData::RESERVED for indicating in the
  // Status bar that an old password has been copied
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
    case CItemData::PWHIST:
      imsg = IDS_PWHISTORYCOPIED;
      break;
    case CItemData::RESERVED:
      imsg = IDS_OLDPSWDCOPIED;
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

void DboxMain::OnClearClipboard()
{
  UpdateLastClipboardAction(-1);
  ClearClipboardData();
}

void DboxMain::MakeRandomPassword(StringX &password, PWPolicy &pwp, bool bIssueMsg)
{
  /**
   * Until 3.37.1 (inclusive) this used to copy the generate password to the clipboard
   * BR1289 points out that this is better left to user's discretion, hence
   * removed SetClipboardData/UpdateLastClipboardAction
   */
  password = pwp.MakeRandomPassword();

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

  CItemData *pci_selected = getSelectedItem();

  if (m_ctlItemTree.IsWindowVisible() && m_LastFoundTreeItem != NULL) {
    pci = (CItemData *)m_ctlItemTree.GetItemData(m_LastFoundTreeItem);
  } else
  if (m_ctlItemList.IsWindowVisible() && m_LastFoundListItem >= 0) {
    pci = (CItemData *)m_ctlItemList.GetItemData(m_LastFoundListItem);
  }

  /*
    BR1432 - If the user has selected an entry, which is not the previous result of
    a Find request, then use the selected item.

    Otherwise, use the last found item (if present)
  */
  if (pci_selected != NULL && pci != pci_selected) {
    pci = pci_selected;
  }

  if (pci == NULL)
    return;

  UpdateAccessTime(pci->GetUUID());

  // All code using ci must be before this AutoType since the
  // *pci may be trashed if lock-on-minimize
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

    // Now minimise
    OnMinimize();
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

    if (m_bFilterActive) {
      // Although the menu entry may have been removed if the base entry
      // is not in the view, this won't stop the accelerator key working
      // as TranslateAccelerator only ignores a menu entry if disabled but
      // doesn't check that it has been removed.

      // If a filter is active, then might not be able to go to
      // entry's base entry as not in Tree or List view
      pws_os::CUUID uuidBase = pci->GetBaseUUID();
      auto iter = m_MapEntryToGUI.find(uuidBase);
      ASSERT(iter != m_MapEntryToGUI.end());
      if (iter->second.list_index == -1)
        return;
    }

    const CItemData *pbci = GetBaseEntry(pci);
    if (pbci != NULL) {
      DisplayInfo *pdi = GetEntryGUIInfo(*pbci);
      SelectEntry(pdi->list_index);
      UpdateAccessTime(pci->GetUUID());
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
      const pws_os::CUUID uuid = pbci->GetUUID();
      DisplayInfo *pdi = GetEntryGUIInfo(*pbci);
      SelectEntry(pdi->list_index);
      EditItem(pbci);
      UpdateAccessTime(uuid);
    }
  }
}

void DboxMain::OnViewAttachment()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);

  if (!pci->HasAttRef())
    return;

  ASSERT(m_core.HasAtt(pci->GetAttUUID()));
  CItemAtt att = m_core.GetAtt(pci->GetAttUUID());

  // Shouldn't be here if no content
  if (!att.HasContent())
    return;

  // Get media type before we find we can't load it
  CString csMediaType = att.GetMediaType().c_str();

  if (csMediaType.Left(5) != L"image") {
    CGeneralMsgBox gmb;
    CString csMessage(MAKEINTRESOURCE(IDS_NOPREVIEW_AVAILABLE));
    CString csTitle(MAKEINTRESOURCE(IDS_VIEWATTACHMENT));
    gmb.MessageBox(csMessage, csTitle, MB_OK);
    return;
  }

  CViewAttachmentDlg viewdlg(this, &att);

  viewdlg.DoModal();
}

void DboxMain::OnRunCommand()
{
  if (SelItemOk() != TRUE)
    return;

  const CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL);
  if (pci == NULL)
    return;

  const CItemData *pbci = pci->IsDependent() ? m_core.GetBaseEntry(pci) : nullptr;
  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd;

  if (!PWSAuxParse::GetEffectiveValues(pci, pbci, sx_group, sx_title, sx_user,
                                       sx_pswd, sx_lastpswd,
                                       sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd))
    return;

  StringX sx_Expanded_ES;
  if (sx_runcmd.empty())
    return;

  std::wstring errmsg;
  StringX::size_type st_column;
  bool bURLSpecial;
  sx_Expanded_ES = PWSAuxParse::GetExpandedString(sx_runcmd,
                                                   m_core.GetCurFile(), pci, pbci,
                                                   m_bDoAutoType, m_sxAutoType,
                                                   errmsg, st_column, bURLSpecial);

  if (!errmsg.empty()) {
    CGeneralMsgBox gmb;
    CString cs_title, cs_errmsg;
    cs_title.LoadString(IDS_RUNCOMMAND_ERROR);
    cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
    gmb.MessageBox(cs_errmsg, cs_title, MB_OK | MB_ICONQUESTION);
    return;
  }

  pws_os::CUUID uuid = pci->GetUUID();

  // if no autotype value in run command's $a(value), start with item's (bug #1078)
  if (m_sxAutoType.empty())
    m_sxAutoType = pci->GetAutoType();

  m_sxAutoType = PWSAuxParse::GetAutoTypeString(m_sxAutoType,
                                                sx_group, sx_title, sx_user,
                                                sx_pswd, sx_lastpswd,
                                                sx_notes, sx_url, sx_email,
                                                m_vactionverboffsets);
  UpdateAccessTime(uuid);

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

  bool rc = m_runner.runcmd(sx_Expanded_ES, !m_sxAutoType.empty());
  if (!rc) {
    m_bDoAutoType = false;
    m_sxAutoType = L"";
    return;
  }
}

void DboxMain::AddDDEntries(CDDObList &in_oblist, const StringX &DropGroup,
                            const std::vector<StringX> &vsxEmptyGroups)
{
  // Add Drop entries
  CItemData ci_temp;
  UUIDVector Possible_Aliases, Possible_Shortcuts;
  std::vector<StringX> vAddedPolicyNames;
  StringX sxEntriesWithNewNamedPolicies;
  std::map<StringX, StringX> mapRenamedPolicies;
  StringX sxgroup, sxtitle, sxuser;
  POSITION pos;

  const StringX sxDD_DateTime = PWSUtil::GetTimeStamp(true).c_str();

  // Initialize set
  GTUSet setGTU;
  m_core.InitialiseGTU(setGTU);

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

  pmulticmds->Add(UpdateGUICommand::Create(&m_core,
     UpdateGUICommand::WN_UNDO, UpdateGUICommand::GUI_REFRESH_BOTHVIEWS));

  for (pos = in_oblist.GetHeadPosition(); pos != NULL; in_oblist.GetNext(pos)) {
    CDDObject *pDDObject = (CDDObject *)in_oblist.GetAt(pos);

    bool bChangedPolicy(false);
    ci_temp.Clear();
    // Only set to false if adding a shortcut where the base isn't there (yet)
    bool bAddToViews = true;
    pDDObject->ToItem(ci_temp);
    ASSERT(ci_temp.GetBaseUUID() != CUUID::NullUUID());

    StringX sxPolicyName = ci_temp.GetPolicyName();
    if (!sxPolicyName.empty()) {
      // D&D put the entry's name here and the details in the entry
      // which we now have to add to this core and remove from the entry

      // Get the source database PWPolicy & symbols for this name
      PWPolicy st_pp;
      ci_temp.GetPWPolicy(st_pp);
      st_pp.symbols = ci_temp.GetSymbols();

      // Get the same info if the policy is in the target database
      PWPolicy currentDB_st_pp;
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
      wchar_t *dot = (!DropGroup.empty() && !ci_temp.GetGroup().empty()) ? L"." : L"";
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

    // If we have BASEUUID, life is simple
    if (ci_temp.GetBaseUUID() != CUUID::NullUUID()) {
      pl.ibasedata = 1;
      pl.base_uuid = ci_temp.GetBaseUUID();
      pl.TargetType = CItemData::ET_NORMAL;
    } else {
      m_core.ParseBaseEntryPWD(cs_tmp, pl);
    }
    if (pl.ibasedata > 0) {
      // Add to pwlist
      pmulticmds->Add(AddEntryCommand::Create(&m_core,
                                              ci_temp, ci_temp.GetBaseUUID())); // need to do this as well as AddDep...
      CGeneralMsgBox gmb;
      CString cs_msg;
      // Password in alias/shortcut format AND base entry exists
      if (pl.InputType == CItemData::ET_ALIAS) {
        ItemListIter iter = m_core.Find(pl.base_uuid);
        ASSERT(iter != End());
        if (pl.TargetType == CItemData::ET_ALIAS) {
          // This base is in fact an alias. ParseBaseEntryPWD already found 'proper base'
          // So dropped entry will point to the 'proper base' and tell the user.
          cs_msg.Format(IDS_DDBASEISALIAS, static_cast<LPCWSTR>(sxgroup.c_str()),
                        static_cast<LPCWSTR>(sxtitle.c_str()),
                        static_cast<LPCWSTR>(sxuser.c_str()));
          gmb.AfxMessageBox(cs_msg, NULL, MB_OK);
        } else if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
          // Only normal or alias base allowed as target
          cs_msg.Format(IDS_ABASEINVALID, static_cast<LPCWSTR>(sxgroup.c_str()),
                        static_cast<LPCWSTR>(sxtitle.c_str()),
                        static_cast<LPCWSTR>(sxuser.c_str()));
          gmb.AfxMessageBox(cs_msg, NULL, MB_OK);
          continue;
        }
        Command *pcmd = AddDependentEntryCommand::Create(&m_core, pl.base_uuid,
                                                         ci_temp.GetUUID(),
                                                         CItemData::ET_ALIAS);
        pmulticmds->Add(pcmd);

        ci_temp.SetPassword(L"[Alias]");
        ci_temp.SetAlias();
      } else if (pl.InputType == CItemData::ET_SHORTCUT) {
        ItemListIter iter = m_core.Find(pl.base_uuid);
        ASSERT(iter != End());
        if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_SHORTCUTBASE) {
          // Only normal or shortcut base allowed as target
          cs_msg.Format(IDS_SBASEINVALID, static_cast<LPCWSTR>(sxgroup.c_str()),
                        static_cast<LPCWSTR>(sxtitle.c_str()),
                        static_cast<LPCWSTR>(sxuser.c_str()));
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
    } else if (pl.ibasedata == 0) {
      // Password NOT in alias/shortcut format
      ci_temp.SetNormal();
    } else if (pl.ibasedata < 0) {
      // Password in alias/shortcut format AND base entry does not exist or multiple possible
      // base entries exit.
      // Note: As more entries are added, what was "not exist" may become "OK",
      // "no unique exists" or "multiple exist".
      // Let the code that processes the possible aliases after all have been added sort this out.
      if (pl.InputType == CItemData::ET_ALIAS) {
        Possible_Aliases.push_back(ci_temp.GetUUID());
      } else if (pl.InputType == CItemData::ET_SHORTCUT) {
        Possible_Shortcuts.push_back(ci_temp.GetUUID());
        bAddToViews = false;
      }
    }
    ci_temp.SetStatus(CItemData::ES_ADDED);
    
    // Need to check that entry keyboard shortcut not already in use!
    int32 iKBShortcut;
    ci_temp.GetKBShortcut(iKBShortcut);
    
    if (iKBShortcut != 0 && 
      m_core.GetKBShortcut(iKBShortcut) != CUUID::NullUUID()) {
      // Remove it but no mechanism to tell user!
      ci_temp.SetKBShortcut(0);
    }

    if (!ci_temp.IsDependent()) { // Dependents handled later
      // Add to pwlist
      Command *pcmd = AddEntryCommand::Create(&m_core, ci_temp);

      if (!bAddToViews) {
        // ONLY Add to pwlist and NOT to Tree or List views
        // After the call to AddDependentEntries for shortcuts, check if still
        // in password list and, if so, then add to Tree + List views
        pcmd->SetNoGUINotify();
      }
      pmulticmds->Add(pcmd);
    }
  } // iteration over in_oblist

  // Now try to add aliases/shortcuts we couldn't add in previous processing
  if (!Possible_Aliases.empty()) {
    Command *pcmdA = AddDependentEntriesCommand::Create(&m_core,
                                                        Possible_Aliases, NULL,
                                                        CItemData::ET_ALIAS,
                                                        CItemData::PASSWORD);
    pmulticmds->Add(pcmdA);
  }

  if (!Possible_Shortcuts.empty()) {
    Command *pcmdS = AddDependentEntriesCommand::Create(&m_core,
                                                        Possible_Shortcuts, NULL,
                                                        CItemData::ET_SHORTCUT,
                                                        CItemData::PASSWORD);
    pmulticmds->Add(pcmdS);
  }

  // Now add Empty Groups
  for (size_t i = 0; i < vsxEmptyGroups.size(); i++) {
    pmulticmds->Add(DBEmptyGroupsCommand::Create(&m_core, vsxEmptyGroups[i],
      DBEmptyGroupsCommand::EG_ADD));
  }

  pmulticmds->Add(UpdateGUICommand::Create(&m_core,
    UpdateGUICommand::WN_EXECUTE_REDO, UpdateGUICommand::GUI_REFRESH_BOTHVIEWS));

  if (pmulticmds->GetSize() > 2) {
    // Since we added some commands apart from the first & last WM_UNDO/WM_REDO,
    // check if original drop group was empty - if so, it won't be now
    // We need to insert it after the first command (WN_UNDO, GUI_REFRESH_BOTHVIEWS)
    if (IsEmptyGroup(DropGroup)) {
      pmulticmds->Insert(DBEmptyGroupsCommand::Create(&m_core, DropGroup,
        DBEmptyGroupsCommand::EG_DELETE), 1);
    }
  } else {
    // We didn't add any "useful" commands - so why continue?
    delete pmulticmds;
    return;
  }

  // Do it!
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
