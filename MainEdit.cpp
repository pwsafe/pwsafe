/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "corelib/pwsprefs.h"
#include "DDSupport.h"

// dialog boxen
#include "DboxMain.h"
#include "AddDlg.h"
#include "ConfirmDeleteDlg.h"
#include "QuerySetDef.h"
#include "EditDlg.h"
#include "EditShortcutDlg.h"
#include "KeySend.h"
#include "ClearQuestionDlg.h"
#include "CreateShortcutDlg.h"
#include "PasswordSubsetDlg.h"

#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include <vector>
#include <algorithm>

/*
 * Make sure we get the right declaration of BlockInput
 * VS2005 - it is in "winable.h"
 * VS2008 - it is in "winuser.h"
 */
 
#if _MSC_VER < 1500
#include <winable.h>
#else
#include <winuser.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Add an item
void DboxMain::OnAdd()
{
  CAddDlg dlg_add(this);

  if (m_core.GetUseDefUser()) {
    dlg_add.m_username = m_core.GetDefUsername();
  }
  // m_TreeViewGroup may be set by OnContextMenu, if not, try to grok it
  if (m_TreeViewGroup.empty()) {
    CItemData *itemData = NULL;
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        itemData = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (itemData != NULL) { // leaf selected
          m_TreeViewGroup = itemData->GetGroup();
        } else { // node selected
          m_TreeViewGroup = m_ctlItemTree.GetGroup(ti);
        }
      }
    } else { // list view
      // XXX TBD - get group name of currently selected list entry
    }
  }
  dlg_add.m_group = m_TreeViewGroup;
  m_TreeViewGroup = _T(""); // for next time
  app.DisableAccelerator();
  INT_PTR rc = dlg_add.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    PWSprefs *prefs = PWSprefs::GetInstance();
    //Check if they wish to set a default username
    if (!m_core.GetUseDefUser() &&
        (prefs->GetPref(PWSprefs::QuerySetDef)) &&
        (!dlg_add.m_username.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_message.Format(IDS_SETUSERNAME, (const CString&)dlg_add.m_username);
      INT_PTR rc2 = defDlg.DoModal();
      if (rc2 == IDOK) {
        prefs->SetPref(PWSprefs::UseDefaultUser, true);
        prefs->SetPref(PWSprefs::DefaultUsername, dlg_add.m_username);
        m_core.SetUseDefUser(true);
        m_core.SetDefUsername(dlg_add.m_username);
      }
    }

    //Finish Check (Does that make any geographical sense?)
    CItemData temp;
    StringX user;
    time_t t;

    if (dlg_add.m_username.IsEmpty() && m_core.GetUseDefUser())
      user = m_core.GetDefUsername();
    else
      user = LPCTSTR(dlg_add.m_username);
    temp.CreateUUID();
    temp.SetGroup(dlg_add.m_group);
    temp.SetTitle(dlg_add.m_title);
    temp.SetUser(user);

    if (dlg_add.m_ibasedata > 0) {
      // Password in alias format AND base entry exists
      // No need to check if base is an alias as already done in
      // call to PWScore::GetBaseEntry
      uuid_array_t alias_uuid;
      temp.GetUUID(alias_uuid);
      m_core.AddDependentEntry(dlg_add.m_base_uuid, alias_uuid, CItemData::ET_ALIAS);
      temp.SetPassword(_T("[Alias]"));
      temp.SetAlias();
      ItemListIter iter = m_core.Find(dlg_add.m_base_uuid);
      if (iter != End()) {
        const CItemData &cibase = iter->second;
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        int nImage = GetEntryImage(cibase);
        SetEntryImage(di->list_index, nImage, true);
        SetEntryImage(di->tree_item, nImage, true);
      }
    } else {
      temp.SetPassword(dlg_add.m_password);
      temp.SetNormal();
    }

    temp.SetNotes(dlg_add.m_notes);
    temp.SetURL(dlg_add.m_URL);
    temp.SetAutoType(dlg_add.m_autotype);
    time(&t);
    temp.SetCTime(t);

    // Set per-item password policy if user selected
    // "Override Policy"
    if (dlg_add.m_OverridePolicy == TRUE)
      temp.SetPWPolicy(dlg_add.m_pwp);

    if (temp.IsAlias()) {
      temp.SetXTime((time_t)0);
      temp.SetPWPolicy(_T(""));
    } else {
      temp.SetXTime(dlg_add.m_tttXTime);
      temp.SetPWPolicy(dlg_add.m_pwp);
    }

    if (dlg_add.m_XTimeInt > 0 && dlg_add.m_XTimeInt <= 3650)
      temp.SetXTimeInt(dlg_add.m_XTimeInt);

    if (dlg_add.m_SavePWHistory == TRUE) {
      temp.SetPWHistory(MakePWHistoryHeader(TRUE, dlg_add.m_MaxPWHistory, 0));
    }

    AddEntry(temp);

    if (m_core.GetNumEntries() == 1) {
      // For some reason, when adding the first entry, it is not visible!
      m_ctlItemTree.SetRedraw(TRUE);
    }
    SortListView();
    m_ctlItemList.SetFocus();
    if (prefs->GetPref(PWSprefs::SaveImmediately))
      Save();

    ChangeOkUpdate();
    uuid_array_t uuid;
    temp.GetUUID(uuid);
    m_RUEList.AddRUEntry(uuid);
    // May need to update menu/toolbar if database was previously empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);
  }
}

//Add a shortcut
void DboxMain::OnCreateShortcut()
{
   // disable in read-only mode or nothing selected
  if (m_core.IsReadOnly() || SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  CCreateShortcutDlg dlg_createshortcut(this, ci->GetGroup(), 
    ci->GetTitle(), ci->GetUser());

  if (m_core.GetUseDefUser()) {
    dlg_createshortcut.m_username = m_core.GetDefUsername();
  }

  app.DisableAccelerator();
  INT_PTR rc = dlg_createshortcut.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    PWSprefs *prefs = PWSprefs::GetInstance();
    //Check if they wish to set a default username
    if (!m_core.GetUseDefUser() &&
        (prefs->GetPref(PWSprefs::QuerySetDef)) &&
        (!dlg_createshortcut.m_username.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_message.Format(IDS_SETUSERNAME, (const CString&)dlg_createshortcut.m_username);
      INT_PTR rc2 = defDlg.DoModal();
      if (rc2 == IDOK) {
        prefs->SetPref(PWSprefs::UseDefaultUser, true);
        prefs->SetPref(PWSprefs::DefaultUsername, dlg_createshortcut.m_username);
        m_core.SetUseDefUser(true);
        m_core.SetDefUsername(dlg_createshortcut.m_username);
      }
    }
    if (dlg_createshortcut.m_username.IsEmpty() && m_core.GetUseDefUser())
      dlg_createshortcut.m_username = m_core.GetDefUsername();

    CreateShortcutEntry(ci, dlg_createshortcut.m_group, 
                        dlg_createshortcut.m_title, 
                        dlg_createshortcut.m_username);
  }
}
void DboxMain::CreateShortcutEntry(CItemData *ci, const StringX &cs_group,
                                   const StringX &cs_title, const StringX &cs_user)
{
  uuid_array_t base_uuid, shortcut_uuid;

  ASSERT(ci != NULL);
  ci->GetUUID(base_uuid);

  //Finish Check (Does that make any geographical sense?)
  CItemData temp;
  time_t t;

  temp.CreateUUID();
  temp.GetUUID(shortcut_uuid);
  temp.SetGroup(cs_group);
  temp.SetTitle(cs_title);
  temp.SetUser(cs_user);

  m_core.AddDependentEntry(base_uuid, shortcut_uuid, CItemData::ET_SHORTCUT);
  temp.SetPassword(_T("[Shortcut]"));
  temp.SetShortcut();
  ItemListIter iter = m_core.Find(base_uuid);
  if (iter != End()) {
    const CItemData &cibase = iter->second;
    DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
    int nImage = GetEntryImage(cibase);
    SetEntryImage(di->list_index, nImage, true);
    SetEntryImage(di->tree_item, nImage, true);
  }

  time(&t);
  temp.SetCTime(t);
  temp.SetXTime((time_t)0);

  AddEntry(temp);

  if (m_core.GetNumEntries() == 1) {
    // For some reason, when adding the first entry, it is not visible!
    m_ctlItemTree.SetRedraw(TRUE);
  }
  m_ctlItemList.SetFocus();

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately))
    Save();

  ChangeOkUpdate();
  m_RUEList.AddRUEntry(shortcut_uuid);
}

int DboxMain::AddEntry(const CItemData &cinew)
{
  // This routine is used by Add and also Drag & Drop

  m_core.AddEntry(cinew);

  // AddEntry copies the entry, and we want to work with the inserted copy
  // Which we'll find by uuid
  uuid_array_t uuid;
  cinew.GetUUID(uuid);
  int newpos = insertItem(m_core.GetEntry(m_core.Find(uuid)));
  if (m_bFilterActive && newpos >= 0) {
    SelectEntry(newpos);
    FixListIndexes();
  }
  return newpos;
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
    if (m_TreeViewGroup.empty())
      m_TreeViewGroup = cmys_text;
    else
      m_TreeViewGroup += _T(".") + cmys_text;
    HTREEITEM newGroup = m_ctlItemTree.AddGroup(m_TreeViewGroup.c_str());
    m_ctlItemTree.SelectItem(newGroup);
    m_TreeViewGroup = _T(""); // for next time
    m_ctlItemTree.EditLabel(newGroup);
  }
}

// Delete key was pressed (in list view or tree view) to delete an entry.
void DboxMain::OnDelete()
{
  if (m_core.GetNumEntries() == 0) // easiest way to avoid asking stupid questions...
    return;

  bool dontaskquestion = PWSprefs::GetInstance()->
                         GetPref(PWSprefs::DeleteQuestion);

  bool dodelete = true;
  int num_children = 0;

  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hStartItem = m_ctlItemTree.GetSelectedItem();
    if (hStartItem != NULL) {
      if (m_ctlItemTree.GetItemData(hStartItem) == NULL) {  // group node
        dontaskquestion = false; // ALWAYS ask if deleting a group
        // Find number of child items
        num_children = 0;
        if (m_ctlItemTree.ItemHasChildren(hStartItem)) {
          num_children = CountChildren(hStartItem);
        }  // if has children
      }
    }
  }

  //Confirm whether to delete the item
  if (!dontaskquestion) {
    CConfirmDeleteDlg deleteDlg(this, num_children);
    INT_PTR rc = deleteDlg.DoModal();
    if (rc == IDCANCEL) {
      dodelete = false;
    }
  }

  if (dodelete) {
    Delete();
    if (m_bFilterActive)
      RefreshViews();
  }
}

void DboxMain::Delete(bool inRecursion)
{
  CItemData *ci = getSelectedItem();

  if (ci != NULL) {
    uuid_array_t entry_uuid;
    ci->GetUUID(entry_uuid);

    UUIDList dependentslist;
    int num_dependents(0);
    CItemData::EntryType entrytype = ci->GetEntryType();

    if (entrytype == CItemData::ET_ALIASBASE)
      m_core.GetAllDependentEntries(entry_uuid, dependentslist, CItemData::ET_ALIAS);
    else 
    if (entrytype == CItemData::ET_SHORTCUTBASE)
      m_core.GetAllDependentEntries(entry_uuid, dependentslist, CItemData::ET_SHORTCUT);

    num_dependents = dependentslist.size();
    if (num_dependents > 0) {
      StringX csDependents;
      SortDependents(dependentslist, csDependents);

      CString cs_msg, cs_type;
      const CString cs_title(MAKEINTRESOURCE(IDS_DELETEBASET));
      if (entrytype == CItemData::ET_ALIASBASE) {
        cs_type.LoadString(num_dependents == 1 ? IDS_ALIAS : IDS_ALIASES);
        cs_msg.Format(IDS_DELETEABASE, dependentslist.size(), cs_type, csDependents);
      } else {
        cs_type.LoadString(num_dependents == 1 ? IDS_SHORTCUT : IDS_SHORTCUTS);
        cs_msg.Format(IDS_DELETESBASE, dependentslist.size(), cs_type, csDependents);
      }

      if (MessageBox(cs_msg, cs_title, MB_ICONQUESTION | MB_YESNO) == IDNO) {
        dependentslist.clear();
        return;
      }
    }

    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    int curSel = di->list_index;
    // Find next in treeview, not always curSel after deletion
    HTREEITEM curTree_item = di->tree_item;
    HTREEITEM nextTree_item = m_ctlItemTree.GetNextItem(curTree_item, TVGN_NEXT);
    // Must Find before delete from m_ctlItemList:
    ItemListIter listindex = m_core.Find(entry_uuid);
    ASSERT(listindex !=  m_core.GetEntryEndIter());

    UnFindItem();
    m_ctlItemList.DeleteItem(curSel);
    m_ctlItemTree.DeleteWithParents(curTree_item);
    delete di;
    FixListIndexes();

    if (ci->NumberUnknownFields() > 0)
      m_core.DecrementNumRecordsWithUnknownFields();

    uuid_array_t base_uuid;
    if (entrytype == CItemData::ET_ALIAS) {
      // I'm an alias entry
      // Get corresponding base uuid
      m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
      // Delete from both map and multimap
      m_core.RemoveDependentEntry(base_uuid, entry_uuid, CItemData::ET_ALIAS);

      // Does my base now become a normal entry?
      if (m_core.NumAliases(base_uuid) == 0) {
        ItemListIter iter = m_core.Find(base_uuid);
        CItemData &cibase = iter->second;
        cibase.SetNormal();
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        int nImage = GetEntryImage(cibase);
        SetEntryImage(di->list_index, nImage, true);
        SetEntryImage(di->tree_item, nImage, true);
      }
    }
    if (entrytype == CItemData::ET_SHORTCUT) {
      // I'm a shortcut entry
      // Get corresponding base uuid
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);
      // Delete from both map and multimap
      m_core.RemoveDependentEntry(base_uuid, entry_uuid, CItemData::ET_SHORTCUT);

      // Does my base now become a normal entry?
      if (m_core.NumShortcuts(base_uuid) == 0) {
        ItemListIter iter = m_core.Find(base_uuid);
        CItemData &cibase = iter->second;
        cibase.SetNormal();
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        int nImage = GetEntryImage(cibase);
        SetEntryImage(di->list_index, nImage, true);
        SetEntryImage(di->tree_item, nImage, true);
      }
    }

    if (num_dependents > 0) {
      // I'm a base entry
      if (entrytype == CItemData::ET_ALIASBASE) {
        m_core.ResetAllAliasPasswords(entry_uuid);
        m_core.RemoveAllDependentEntries(entry_uuid, CItemData::ET_ALIAS);

        // Now make all my aliases Normal
        ItemListIter iter;
        UUIDListIter UUIDiter;
        for (UUIDiter = dependentslist.begin(); UUIDiter != dependentslist.end(); UUIDiter++) {
          uuid_array_t auuid;
          UUIDiter->GetUUID(auuid);
          iter = m_core.Find(auuid);
          CItemData &cialias = iter->second;
          DisplayInfo *di = (DisplayInfo *)cialias.GetDisplayInfo();
          int nImage = GetEntryImage(cialias);
          SetEntryImage(di->list_index, nImage, true);
          SetEntryImage(di->tree_item, nImage, true);
        }
      } else {
        m_core.RemoveAllDependentEntries(entry_uuid, CItemData::ET_SHORTCUT);
        // Now delete all my shortcuts
        ItemListIter iter;
        UUIDListIter UUIDiter;
        for (UUIDiter = dependentslist.begin(); UUIDiter != dependentslist.end(); UUIDiter++) {
          uuid_array_t suuid;
          UUIDiter->GetUUID(suuid);
          iter = m_core.Find(suuid);
          CItemData &cshortcut = iter->second;
          DisplayInfo *di = (DisplayInfo *)cshortcut.GetDisplayInfo();
          m_ctlItemList.DeleteItem(di->list_index);
          m_ctlItemTree.DeleteItem(di->tree_item);
          delete di;
          FixListIndexes();
          m_core.RemoveEntryAt(iter);
        }
      }
      dependentslist.clear();
    }

    m_core.RemoveEntryAt(listindex);
    if (m_ctlItemList.IsWindowVisible()) {
      if (m_core.GetNumEntries() > 0) {
        SelectEntry(curSel < (int)m_core.GetNumEntries() ? curSel : (int)(m_core.GetNumEntries() - 1));
      }
      m_ctlItemList.SetFocus();
    } else {// tree view visible
      if (!inRecursion && nextTree_item != NULL) {
        m_ctlItemTree.SelectItem(nextTree_item);
      }
      m_ctlItemTree.SetFocus();
    }
    ChangeOkUpdate();
    m_RUEList.DeleteRUEntry(entry_uuid);
  } else { // !SelItemOk()
    if (m_ctlItemTree.IsWindowVisible()) {
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) {
        if (!m_ctlItemTree.IsLeaf(ti)) {
          HTREEITEM cti = m_ctlItemTree.GetChildItem(ti);

          m_ctlItemTree.SetRedraw( FALSE );

          while (cti != NULL) {
            m_ctlItemTree.SelectItem(cti);
            Delete(true); // recursion - I'm so lazy!
            cti = m_ctlItemTree.GetChildItem(ti);
          }

          m_ctlItemTree.SetRedraw( TRUE );
          m_ctlItemTree.Invalidate();

          //  delete an empty group.
          HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);
          m_ctlItemTree.DeleteItem(ti);
          m_ctlItemTree.SelectItem(parent);
        }
      }
    }
  }
  m_TreeViewGroup = _T("");
}

void DboxMain::OnRename()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  // Renaming is only allowed while in Tree mode.
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hItem = m_ctlItemTree.GetSelectedItem();
    if (hItem != NULL) {
      m_ctlItemTree.EditLabel(hItem);
      if (m_bFilterActive && m_ctlItemTree.WasLabelEdited())
        RefreshViews();
    }
  }
}

void DboxMain::OnEdit()
{
  // Note that Edit is also used for just viewing - don't want to disable
  // viewing in read-only mode
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    if (ci->IsShortcut())
      EditShortcut(ci);
    else
      EditItem(ci);
  } else {
    // entry item not selected - perhaps here on Enter on tree item?
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
}

bool DboxMain::EditItem(CItemData *ci, PWScore *pcore)
{
  if (pcore == NULL)
    pcore = &m_core;

  // List might be cleared if db locked.
  // Need to take care that we handle a rebuilt list.
  CItemData editedItem(*ci);

  CEditDlg dlg_edit(&editedItem, this);

  if (pcore->GetUseDefUser())
    dlg_edit.m_defusername = pcore->GetDefUsername();

  dlg_edit.m_Edit_IsReadOnly = pcore->IsReadOnly();

  uuid_array_t original_uuid, original_base_uuid, new_base_uuid;
  CItemData::EntryType entrytype = ci->GetEntryType();

  ci->GetUUID(original_uuid);  // Edit doesn't change this!
  if (entrytype == CItemData::ET_ALIASBASE || entrytype == CItemData::ET_SHORTCUTBASE) {
    // Base entry
    UUIDList dependentslist;
    StringX csDependents(_T(""));

    m_core.GetAllDependentEntries(original_uuid, dependentslist, 
      entrytype == CItemData::ET_ALIASBASE ? CItemData::ET_ALIAS : CItemData::ET_SHORTCUT);
    int num_dependents = dependentslist.size();
    if (num_dependents > 0) {
      SortDependents(dependentslist, csDependents);
    }

    dlg_edit.m_num_dependents = num_dependents;
    dlg_edit.m_dependents = csDependents;
    dlg_edit.m_original_entrytype = entrytype;
    dependentslist.clear();
  }

  if (entrytype == CItemData::ET_ALIAS) {
    // Alias entry
    // Get corresponding base uuid
    m_core.GetAliasBaseUUID(original_uuid, original_base_uuid);

    ItemListIter iter = m_core.Find(original_base_uuid);
    if (iter != End()) {
      const CItemData &cibase = iter->second;
      dlg_edit.m_base = _T("[") +
                        cibase.GetGroup() + _T(":") +
                        cibase.GetTitle() + _T(":") +
                        cibase.GetUser()  + _T("]");
      dlg_edit.m_original_entrytype = CItemData::ET_ALIAS;
    }
  } else {
    editedItem.GetPWPolicy(dlg_edit.m_pwp);
  }

  app.DisableAccelerator();
  INT_PTR rc = dlg_edit.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    // Out with the old, in with the new
    ItemListIter listpos = Find(original_uuid);
    ASSERT(listpos != m_core.GetEntryEndIter());
    CItemData oldElem = GetEntryAt(listpos);
    DisplayInfo *di = (DisplayInfo *)oldElem.GetDisplayInfo();
    ASSERT(di != NULL);
    // editedItem's displayinfo will have been deleted if
    // application "locked" (Cleared list)
    DisplayInfo *ndi = new DisplayInfo;
    ndi->list_index = -1; // so that insertItem will set new values
    ndi->tree_item = 0;
    editedItem.SetDisplayInfo(ndi);
    StringX newPassword = editedItem.GetPassword();
    memcpy(new_base_uuid, dlg_edit.m_base_uuid, sizeof(uuid_array_t));

    ItemListIter iter;
    if (dlg_edit.m_original_entrytype == CItemData::ET_NORMAL &&
        ci->GetPassword() != newPassword) {
      // Original was a 'normal' entry and the password has changed
      if (dlg_edit.m_ibasedata > 0) {
        // Now an alias
        pcore->AddDependentEntry(new_base_uuid, original_uuid, CItemData::ET_ALIAS);
        editedItem.SetPassword(_T("[Alias]"));
        editedItem.SetAlias();
      } else {
        // Still 'normal'
        editedItem.SetPassword(newPassword);
        editedItem.SetNormal();
      }
    }

    if (dlg_edit.m_original_entrytype == CItemData::ET_ALIAS) {
      // Original was an alias - delete it from multimap
      // RemoveDependentEntry also resets base to normal if the last alias is delete
      pcore->RemoveDependentEntry(original_base_uuid, original_uuid, CItemData::ET_ALIAS);
      if (newPassword == dlg_edit.m_base) {
        // Password (i.e. base) unchanged - put it back
        pcore->AddDependentEntry(original_base_uuid, original_uuid, CItemData::ET_ALIAS);
      } else {
        // Password changed so might be an alias of another entry!
        // Could also be the same entry i.e. [:t:] == [t] !
        if (dlg_edit.m_ibasedata > 0) {
          // Still an alias
          pcore->AddDependentEntry(new_base_uuid, original_uuid, CItemData::ET_ALIAS);
          editedItem.SetPassword(_T("[Alias]"));
          editedItem.SetAlias();
        } else {
          // No longer an alias
          editedItem.SetPassword(newPassword);
          editedItem.SetNormal();
        }
      }
    }

    if (dlg_edit.m_original_entrytype == CItemData::ET_ALIASBASE &&
        ci->GetPassword() != newPassword) {
      // Original was a base but might now be an alias of another entry!
      if (dlg_edit.m_ibasedata > 0) {
        // Now an alias
        // Make this one an alias
        pcore->AddDependentEntry(new_base_uuid, original_uuid, CItemData::ET_ALIAS);
        editedItem.SetPassword(_T("[Alias]"));
        editedItem.SetAlias();
        // Move old aliases across
        pcore->MoveDependentEntries(original_uuid, new_base_uuid, CItemData::ET_ALIAS);
      } else {
        // Still a base entry but with a new password
        editedItem.SetPassword(newPassword);
        editedItem.SetAliasBase();
      }
    }

    // Reset all images!
    // This entry's image will be set by DboxMain::insertItem

    // Next the original base entry
    iter = m_core.Find(original_base_uuid);
    if (iter != End()) {
      const CItemData &cibase = iter->second;
      DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
      int nImage = GetEntryImage(cibase);
      SetEntryImage(di->list_index, nImage, true);
      SetEntryImage(di->tree_item, nImage, true);
    }

    // Last the new base entry (only if different to the one we have done!
    if (::memcmp(new_base_uuid, original_base_uuid, sizeof(uuid_array_t)) != 0) {
      iter = m_core.Find(new_base_uuid);
      if (iter != End()) {
        const CItemData &cibase = iter->second;
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        int nImage = GetEntryImage(cibase);
        SetEntryImage(di->list_index, nImage, true);
        SetEntryImage(di->tree_item, nImage, true);
      }
    }

    if (editedItem.IsAlias() || editedItem.IsShortcut()) {
      editedItem.SetXTime((time_t)0);
      editedItem.SetPWPolicy(_T(""));
    } else {
      editedItem.SetPWPolicy(dlg_edit.m_pwp);
    }

    pcore->RemoveEntryAt(listpos);
    pcore->AddEntry(editedItem);
    m_ctlItemList.DeleteItem(di->list_index);
    m_ctlItemTree.DeleteWithParents(di->tree_item);

    // AddEntry copies the entry, and we want to work with the inserted copy
    // Which we'll find by uuid
    insertItem(pcore->GetEntry(m_core.Find(original_uuid)));
    FixListIndexes();
    // Now delete old entry's DisplayInfo
    delete di;
    if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::SaveImmediately)) {
        Save();
    }
    if (ndi->list_index >= 0) {
      rc = SelectEntry(ndi->list_index);
      if (rc == 0) {
        SelectEntry(m_ctlItemList.GetItemCount() - 1);
      }
    }
    ChangeOkUpdate();
    // Order may have changed as a result of edit
    m_ctlItemTree.SortTree(TVI_ROOT);
    SortListView();

    return true;
  } // rc == IDOK
  return false;
}

bool DboxMain::EditShortcut(CItemData *ci, PWScore *pcore)
{
  if (pcore == NULL)
    pcore = &m_core;

  // List might be cleared if db locked.
  // Need to take care that we handle a rebuilt list.
  CItemData editedItem(*ci);

  uuid_array_t entry_uuid, base_uuid;
  ci->GetUUID(entry_uuid);  // Edit doesn't change this!

  // Shortcut entry
  // Get corresponding base uuid
  m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

  ItemListIter iter = m_core.Find(base_uuid);
  if (iter == End())
    return false;

  CEditShortcutDlg dlg_editshortcut(&editedItem, this, iter->second.GetGroup(),
                                    iter->second.GetTitle(), iter->second.GetUser());

  if (pcore->GetUseDefUser())
    dlg_editshortcut.m_defusername = pcore->GetDefUsername();
  dlg_editshortcut.m_Edit_IsReadOnly = pcore->IsReadOnly();

  app.DisableAccelerator();
  INT_PTR rc = dlg_editshortcut.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    // Out with the old, in with the new
    // User cannot change a shortcut entry to anything else!
    ItemListIter listpos = Find(entry_uuid);
    ASSERT(listpos != m_core.GetEntryEndIter());
    CItemData oldElem = GetEntryAt(listpos);
    DisplayInfo *di = (DisplayInfo *)oldElem.GetDisplayInfo();
    ASSERT(di != NULL);
    // editedItem's displayinfo will have been deleted if
    // application "locked" (Cleared list)
    DisplayInfo *ndi = new DisplayInfo;
    ndi->list_index = -1; // so that insertItem will set new values
    ndi->tree_item = 0;
    editedItem.SetDisplayInfo(ndi);

    editedItem.SetXTime((time_t)0);

    pcore->RemoveEntryAt(listpos);
    pcore->AddEntry(editedItem);
    m_ctlItemList.DeleteItem(di->list_index);
    m_ctlItemTree.DeleteWithParents(di->tree_item);
    // AddEntry copies the entry, and we want to work with the inserted copy
    // Which we'll find by uuid
    insertItem(pcore->GetEntry(m_core.Find(entry_uuid)));
    FixListIndexes();
    // Now delete old entry's DisplayInfo
    delete di;
    if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::SaveImmediately)) {
        Save();
    }
    rc = SelectEntry(ndi->list_index);
    if (rc == 0) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();
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
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);

    // Get information from current selected entry
    StringX ci2_group = ci->GetGroup();
    StringX ci2_user = ci->GetUser();
    StringX ci2_title0 = ci->GetTitle();
    StringX ci2_title;

    // Find a unique "Title"
    ItemListConstIter listpos;
    int i = 0;
    CString s_copy;
    do {
      i++;
      s_copy.Format(IDS_COPYNUMBER, i);
      ci2_title = ci2_title0 + LPCTSTR(s_copy);
      listpos = m_core.Find(ci2_group, ci2_title, ci2_user);
    } while (listpos != m_core.GetEntryEndIter());

    // Set up new entry
    CItemData ci2;
    ci2.CreateUUID();
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetPassword(ci->GetPassword());
    ci2.SetURL(ci->GetURL());
    ci2.SetAutoType(ci->GetAutoType());
    ci2.SetNotes(ci->GetNotes());
    time_t t;
    int xint;
    ci->GetCTime(t);
    if ((long) t != 0L)
      ci2.SetCTime(t);
    ci->GetATime(t);
    if ((long) t != 0L)
      ci2.SetATime(t);
    ci->GetXTime(t);
    if ((long) t != 0L)
      ci2.SetXTime(t);
    ci->GetXTimeInt(xint);
    if (xint != 0)
      ci2.SetXTimeInt(xint);
    ci->GetPMTime(t);
    if ((long) t != 0L)
      ci2.SetPMTime(t);
    ci->GetRMTime(t);
    if ((long) t != 0L)
      ci2.SetRMTime(t);
    StringX tmp = ci->GetPWHistory();
    if (tmp.length() >= 5)
      ci2.SetPWHistory(tmp);

    uuid_array_t base_uuid, original_entry_uuid, new_entry_uuid;
    CItemData::EntryType entrytype = ci->GetEntryType();
    if (entrytype == CItemData::ET_ALIAS || entrytype == CItemData::ET_SHORTCUT) {
      ci->GetUUID(original_entry_uuid);
      ci2.GetUUID(new_entry_uuid);
      if (entrytype == CItemData::ET_ALIAS) {
        m_core.GetAliasBaseUUID(original_entry_uuid, base_uuid);
        ci2.SetAlias();
        m_core.AddDependentEntry(base_uuid, new_entry_uuid, CItemData::ET_ALIAS);
      } else {
        m_core.GetShortcutBaseUUID(original_entry_uuid, base_uuid);
        ci2.SetShortcut();
        m_core.AddDependentEntry(base_uuid, new_entry_uuid, CItemData::ET_SHORTCUT);
      }

      ItemListIter iter;
      iter = m_core.Find(base_uuid);
      if (iter != m_core.GetEntryEndIter()) {
        StringX cs_tmp;
        cs_tmp = _T("[") +
                 iter->second.GetGroup() + _T(":") +
                 iter->second.GetTitle() + _T(":") +
                 iter->second.GetUser()  + _T("]");
        ci2.SetPassword(cs_tmp);
      }
    }

    // Add it to the end of the list
    m_core.AddEntry(ci2);
    di->list_index = -1; // so that insertItem will set new values
    uuid_array_t uuid;
    ci2.GetUUID(uuid);
    insertItem(m_core.GetEntry(m_core.Find(uuid)));
    FixListIndexes();
    if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::SaveImmediately)) {
        Save();
    }
    int rc = SelectEntry(di->list_index);
    if (rc == 0) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();
    m_RUEList.AddRUEntry(uuid);
  }
}

void DboxMain::OnCopyPassword()
{
  if (!SelItemOk())
    return;

  //Remind the user about clipboard security
  CClearQuestionDlg clearDlg(this);
  if (clearDlg.m_dontaskquestion == FALSE &&
    clearDlg.DoModal() == IDCANCEL)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  CItemData *ci_original(ci);

  uuid_array_t base_uuid, entry_uuid;
  const CItemData::EntryType entrytype = ci->GetEntryType();
  if (entrytype == CItemData::ET_ALIAS || entrytype == CItemData::ET_SHORTCUT) {
    // This is an alias/shortcut
    ci->GetUUID(entry_uuid);
    if (entrytype == CItemData::ET_ALIAS)
      m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
    else
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = &iter->second;
    }
  }

  SetClipboardData(ci->GetPassword());

  UpdateLastClipboardAction(CItemData::PASSWORD);
  UpdateAccessTime(ci_original);
}

void DboxMain::OnCopyPasswordMinimize()
{
  // Do OnCopyPassword, and minimize afterwards.
  if (SelItemOk()) {
    OnCopyPassword();
    ShowWindow(SW_MINIMIZE);
  }
}

void DboxMain::OnDisplayPswdSubset()
{
  if (!SelItemOk())
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  CItemData *ci_original(ci);

  uuid_array_t base_uuid, entry_uuid;
  const CItemData::EntryType entrytype = ci->GetEntryType();
  if (entrytype == CItemData::ET_ALIAS || entrytype == CItemData::ET_SHORTCUT) {
    // This is an alias/shortcut
    ci->GetUUID(entry_uuid);
    if (entrytype == CItemData::ET_ALIAS)
      m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
    else
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = &iter->second;
    }
  }

  CPasswordSubsetDlg DisplaySubsetDlg(this, ci);

  app.DisableAccelerator();
  if (DisplaySubsetDlg.DoModal() != IDCANCEL)
    UpdateAccessTime(ci_original);

  app.EnableAccelerator();
}

void DboxMain::OnCopyUsername()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  CItemData *ci_original(ci);

  if (ci->IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci->GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = &iter->second;
    }
  }

  SetClipboardData(ci->GetUser());
  UpdateLastClipboardAction(CItemData::USER);
  UpdateAccessTime(ci_original);
}

void DboxMain::OnCopyNotes()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  CItemData *ci_original(ci);

  if (ci->IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci->GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = &iter->second;
    }
  }

  const StringX notes = ci->GetNotes();
  const StringX url = ci->GetURL();
  const StringX autotype = ci->GetAutoType();
  StringX clipboard_data;
  CString cs_text;

  clipboard_data = notes;
  if (!url.empty()) {
    if (ci->IsURLEmail())
      cs_text.LoadString(IDS_COPYURL);
    else
      cs_text.LoadString(IDS_COPYEMAIL);
    clipboard_data += LPCTSTR(cs_text);
    clipboard_data += url;
  }

  if (!autotype.empty()) {
    cs_text.LoadString(IDS_COPYAUTOTYPE);
    clipboard_data += LPCTSTR(cs_text);
    clipboard_data += autotype;
  }

  SetClipboardData(clipboard_data);
  UpdateLastClipboardAction(CItemData::NOTES);
  UpdateAccessTime(ci_original);
}

void DboxMain::OnCopyURL()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  CItemData *ci_original(ci);

  if (ci->IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci->GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = &iter->second;
    }
  }

  StringX cs_URL = ci->GetURL();
  StringX::size_type ipos;
  ipos = cs_URL.find(_T("[alt]"));
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, _T(""));
  ipos = cs_URL.find(_T("[ssh]"));
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, _T(""));
  ipos = cs_URL.find(_T("{alt}"));
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, _T(""));
  SetClipboardData(cs_URL);
  UpdateLastClipboardAction(CItemData::URL);
  UpdateAccessTime(ci_original);
}

void DboxMain::UpdateLastClipboardAction(const int iaction)
{
  int imsg(0);
  m_lastclipboardaction = _T("");
  switch (iaction) {
    case -1:
      // Clipboard cleared
      m_lastclipboardaction = _T("");
      break;
    case CItemData::GROUP:
      imsg = IDS_GROUP;
      break;
    case CItemData::TITLE:
      imsg = IDS_TITLE;
      break;
    case CItemData::USER:
      imsg = IDS_USER;
      break;
    case CItemData::PASSWORD:
      imsg = IDS_PSWD;
      break;
    case CItemData::NOTES:
      imsg = IDS_NOTES;
      break;
    case CItemData::URL:
      imsg = IDS_URL;
      break;
    case CItemData::AUTOTYPE:
      imsg = IDS_AUTOTYPE;
      break;
    default:
      ASSERT(0);
      return;
  }

  if (iaction > 0) {
    TCHAR szTimeFormat[80], szTimeString[80];
    VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, 
                           szTimeFormat, 80 /* sizeof(szTimeFormat) / sizeof(TCHAR) */));
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, szTimeFormat,
                  szTimeString, 80 /* sizeof(szTimeString) / sizeof(TCHAR) */);
    m_lastclipboardaction.Format(IDS_CLIPBOARDACTION, CString(MAKEINTRESOURCE(imsg)), 
                                 szTimeString);
  }
  UpdateStatusBar();
}

void DboxMain::OnFind()
{
  // Note that this "toggles" the Find Tool Bar so that the user can use Ctrl+F
  // to show it and then hide it.
  // Note hiding is via OnHideFindToolBar so that we only have place selecting
  // last found item in one place.
  if (m_FindToolBar.IsVisible())
    OnHideFindToolBar();
  else
    SetFindToolBar(true);
}

void DboxMain::OnClearClipboard()
{
  UpdateLastClipboardAction(-1);
  ClearClipboardData();
}

void DboxMain::OnAutoType()
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);

    UpdateAccessTime(ci);

    // All code using ci must be before this AutoType since the
    // latter may trash *ci if lock-on-minimize
    AutoType(*ci);
  }
}

const CString DboxMain::DEFAULT_AUTOTYPE = _T("\\u\\t\\p\\n");

void DboxMain::AutoType(const CItemData &ci)
{
  StringX AutoCmd = ci.GetAutoType();
  StringX user(ci.GetUser());
  StringX pwd;

  uuid_array_t base_uuid, entry_uuid;
  CItemData::EntryType entrytype = ci.GetEntryType();
  if (entrytype == CItemData::ET_ALIAS || entrytype == CItemData::ET_SHORTCUT) {
    // This is an alias
    ci.GetUUID(entry_uuid);
    if (entrytype == CItemData::ET_ALIAS)
      m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
    else
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      pwd = iter->second.GetPassword();
      if (entrytype == CItemData::ET_SHORTCUT) {
        user = iter->second.GetUser();
        AutoCmd = iter->second.GetAutoType();
      }
    }
  } else {
    pwd = ci.GetPassword();
  }

  // If empty, try the database default
  if (AutoCmd.empty()) {
    AutoCmd = PWSprefs::GetInstance()->
              GetPref(PWSprefs::DefaultAutotypeString);

    // If still empty, take this default
    if (AutoCmd.empty()) {
      // checking for user and password for default settings
      if (!pwd.empty()){
        if (!user.empty())
          AutoCmd = DEFAULT_AUTOTYPE;
        else
          AutoCmd = _T("\\p\\n");
      }
    }
  }

  CKeySend ks;
  // Turn off CAPSLOCK
  bool bCapsLock = false;
  if (GetKeyState(VK_CAPITAL)) {
    bCapsLock = true;
    ks.SetCapsLock(false);
  }

  StringX tmp(_T(""));
  TCHAR curChar;
  const int N = AutoCmd.length();
  ks.ResetKeyboardState();

#if _MSC_VER < 1500
  ::BlockInput(true);
#else
  BlockInput(true);
#endif

  // Note that minimizing the window before calling ci.Get*()
  // will cause garbage to be read if "lock on minimize" selected,
  // since that will clear the data [Bugs item #1026630]
  // (this is why we read user & pwd before actual use)

  // Rules are ("Minimize on Autotype" takes precedence):
  // 1. If "MinimizeOnAutotype" - minimize PWS during Autotype but do
  //    not restore it (previous default action - but a pain if locked
  //    in the system tray!)
  // 2. If "Always on Top" - hide PWS during Autotype and then make it
  //    "AlwaysOnTop" again, unless minimized!
  // 3. If not "Always on Top" - hide PWS during Autotype and show
  //    it again once finished - but behind other windows.
  bool bMinOnAuto = PWSprefs::GetInstance()->
                    GetPref(PWSprefs::MinimizeOnAutotype) == TRUE;

  if (bMinOnAuto)
    ShowWindow(SW_MINIMIZE);
  else
    ShowWindow(SW_HIDE);

  Sleep(1000); // Karl Student's suggestion, to ensure focus set correctly on minimize.

  for (int n = 0; n < N; n++){
    curChar = AutoCmd[n];
    if (curChar == TCHAR('\\')) {
      n++;
      if (n < N)
        curChar=AutoCmd[n];

      switch (curChar){
        case TCHAR('\\'):
          tmp += TCHAR('\\');
          break;
        case TCHAR('n'):
        case TCHAR('r'):
          tmp += TCHAR('\r');
          break;
        case TCHAR('t'):
          tmp += TCHAR('\t');
          break;
        case TCHAR('u'):
          tmp += user;
          break;
        case TCHAR('p'):
          tmp += pwd;
          break;
        case TCHAR('d'):
        {
          // Delay is going to change - send what we have with old delay
          ks.SendString(tmp);
          // start collecting new delay
          tmp = _T("");
          int newdelay = 0;
          int gNumIts = 0;

          for (n++; n < N && (gNumIts < 3); ++gNumIts, n++) {
            if (isdigit(AutoCmd[n])) {
              newdelay *= 10;
              newdelay += (AutoCmd[n] - TCHAR('0'));
            } else
              break; // for loop
          }
    
          n--;
          ks.SetAndDelay(newdelay);
          break; // case 'd'
        }
        default:
          tmp += _T("\\") + curChar;
          break;
      }
    } else
      tmp += curChar;
  }
  ks.SendString(tmp);
  // If we turned off CAPSLOCK, put it back
  if (bCapsLock)
    ks.SetCapsLock(true);

  Sleep(100);

#if _MSC_VER < 1500
  ::BlockInput(false);
#else
  BlockInput(false);
#endif

  // If we hid it, now show it
  if (bMinOnAuto)
    return;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
    SetWindowPos(&wndTopMost, 0, 0, 0, 0,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
  } else {
    SetWindowPos(&wndBottom, 0, 0, 0, 0,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
  }
}

StringX DboxMain::GetAutoTypeString(const StringX autocmd, const StringX user, 
                                    const StringX pwd)
{
  // If empty, try the database default
  StringX AutoCmd(autocmd);
  if (AutoCmd.empty()) {
    AutoCmd = PWSprefs::GetInstance()->
              GetPref(PWSprefs::DefaultAutotypeString);

    // If still empty, take this default
    if (AutoCmd.empty()) {
      // checking for user and password for default settings
      if (!pwd.empty()){
        if (!user.empty())
          AutoCmd = DEFAULT_AUTOTYPE;
        else
          AutoCmd = _T("\\p\\n");
      }
    }
  }

  StringX tmp(_T(""));
  TCHAR curChar;
  const int N = AutoCmd.length();

  for (int n = 0; n < N; n++){
    curChar = AutoCmd[n];
    if (curChar == TCHAR('\\')) {
      n++;
      if (n < N)
        curChar = AutoCmd[n];

      switch (curChar){
        case TCHAR('\\'):
          tmp += TCHAR('\\');
          break;
        case TCHAR('n'):
        case TCHAR('r'):
          tmp += TCHAR('\r');
          break;
        case TCHAR('t'):
          tmp += TCHAR('\t');
          break;
        case TCHAR('u'):
          tmp += user;
          break;
        case TCHAR('p'):
          tmp += pwd;
          break;
        case TCHAR('d'):
          // Ignore delay!
          break; // case 'd'
        default:
          tmp += _T("\\") + curChar;
          break;
      }
    } else
      tmp += curChar;
  }
  return tmp;
}

void DboxMain::OnGotoBaseEntry()
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);

    uuid_array_t base_uuid, entry_uuid;
    CItemData::EntryType entrytype = ci->GetEntryType();
    if (entrytype == CItemData::ET_ALIAS || entrytype == CItemData::ET_SHORTCUT) {
      // This is an alias or shortcut
      ci->GetUUID(entry_uuid);
      if (entrytype == CItemData::ET_ALIAS)
        m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
      else
        m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

      ItemListIter iter = m_core.Find(base_uuid);
      if (iter != End()) {
         DisplayInfo *di = (DisplayInfo *)iter->second.GetDisplayInfo();
         SelectEntry(di->list_index);
      } else
        return;

      UpdateAccessTime(ci);
    }
  }
}

void DboxMain::AddEntries(CDDObList &in_oblist, const StringX &DropGroup)
{
  // Add Drop entries
  CItemData tempitem;
  UUIDList possible_aliases, possible_shortcuts;
  StringX Group, Title, User;
  POSITION pos;
  TCHAR *dot;
  uuid_array_t entry_uuid;
  bool bAddToViews;

  for (pos = in_oblist.GetHeadPosition(); pos != NULL; in_oblist.GetNext(pos)) {
    CDDObject *pDDObject = (CDDObject *)in_oblist.GetAt(pos);
#ifdef DEMO
    if (m_core.GetNumEntries() >= MAXDEMO)
      break;
#endif /* DEMO */
    tempitem.Clear();
    // Only set to false if adding a shortcut where the base isn't there (yet)
    bAddToViews = true;
    pDDObject->ToItem(tempitem);

    if (in_oblist.m_bDragNode) {
      dot = (!DropGroup.empty() && !tempitem.GetGroup().empty()) ? _T(".") : _T("");
      Group = DropGroup + dot + tempitem.GetGroup();
    } else {
      Group = DropGroup;
    }

    User = tempitem.GetUser();
    Title = GetUniqueTitle(Group, tempitem.GetTitle(), User, IDS_DRAGNUMBER);

    tempitem.GetUUID(entry_uuid);
    if (m_core.Find(entry_uuid) != End()) {
      // Already in use - get a new one!
      tempitem.CreateUUID();
      tempitem.GetUUID(entry_uuid);
    }

    tempitem.SetGroup(Group);
    tempitem.SetTitle(Title);

    StringX cs_tmp = tempitem.GetPassword();

    GetBaseEntryPL pl;
    pl.InputType = CItemData::ET_NORMAL;

    // Potentially remove outer single square brackets as GetBaseEntry expects only
    // one set of square brackets (processing import and user edit of entries)
    if (cs_tmp.substr(0, 2) == _T("[[") &&
        cs_tmp.substr(cs_tmp.length() - 2) == _T("]]")) {
      cs_tmp = cs_tmp.substr(1, cs_tmp.length() - 2);
      pl.InputType = CItemData::ET_ALIAS;
    }

    // Potentially remove tilde as GetBaseEntry expects only
    // one set of square brackets (processing import and user edit of entries)
    if (cs_tmp.substr(0, 2) == _T("[~") &&
        cs_tmp.substr(cs_tmp.length() - 2) == _T("~]")) {
      cs_tmp = _T("[") + cs_tmp.substr(2, cs_tmp.length() - 4) + _T("]");
      pl.InputType = CItemData::ET_SHORTCUT;
    }

    m_core.GetBaseEntry(cs_tmp, pl);
    if (pl.ibasedata > 0) {
      // Password in alias/shortcut format AND base entry exists
      if (pl.InputType == CItemData::ET_ALIAS) {
        ItemListIter iter = m_core.Find(pl.base_uuid);
        ASSERT(iter != End());
        if (pl.TargetType == CItemData::ET_ALIAS) {
          // This base is in fact an alias. GetBaseEntry already found 'proper base'
          // So dropped entry will point to the 'proper base' and tell the user.
          CString cs_msg;
          cs_msg.Format(IDS_DDBASEISALIAS, Group, Title, User);
          AfxMessageBox(cs_msg, MB_OK);
        } else
        if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
          // Only normal or alias base allowed as target
          CString cs_msg;
          cs_msg.Format(IDS_ABASEINVALID, Group, Title, User);
          AfxMessageBox(cs_msg, MB_OK);
          continue;
        }
        m_core.AddDependentEntry(pl.base_uuid, entry_uuid, CItemData::ET_ALIAS);
        tempitem.SetPassword(_T("[Alias]"));
        tempitem.SetAlias();
      } else
      if (pl.InputType == CItemData::ET_SHORTCUT) {
        ItemListIter iter = m_core.Find(pl.base_uuid);
        ASSERT(iter != End());
        if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_SHORTCUTBASE) {
          // Only normal or shortcut base allowed as target
          CString cs_msg;
          cs_msg.Format(IDS_SBASEINVALID, Group, Title, User);
          AfxMessageBox(cs_msg, MB_OK);
          continue;
        }
        m_core.AddDependentEntry(pl.base_uuid, entry_uuid, CItemData::ET_SHORTCUT);
        tempitem.SetPassword(_T("[Shortcut]"));
        tempitem.SetShortcut();
      }
    } else
    if (pl.ibasedata == 0) {
      // Password NOT in alias/shortcut format
      tempitem.SetNormal();
    } else
    if (pl.ibasedata < 0) {
      // Password in alias/shortcut format AND base entry does not exist or multiple possible
      // base entries exit.
      // Note: As more entries are added, what was "not exist" may become "OK",
      // "no unique exists" or "multiple exist".
      // Let the code that processes the possible aliases after all have been added sort this out.
      if (pl.InputType == CItemData::ET_ALIAS) {
        possible_aliases.push_back(entry_uuid);
      } else
      if (pl.InputType == CItemData::ET_SHORTCUT) {
        possible_shortcuts.push_back(entry_uuid);
        bAddToViews = false;
      }
    }
    if (bAddToViews) {
      // Add to pwlist + Tree + List views
      AddEntry(tempitem);
    } else {
      // ONLY Add to pwlist and NOT to Tree or List views
      // After the call to AddDependentEntries for shortcuts, check if still
      // in password list and, if so, then add to Tree + List views
      m_core.AddEntry(tempitem);
    }
  } // iteration over in_oblist

  // Now try to add aliases/shortcuts we couldn't add in previous processing
  m_core.AddDependentEntries(possible_aliases, NULL, CItemData::ET_ALIAS, 
                             CItemData::PASSWORD);
  m_core.AddDependentEntries(possible_shortcuts, NULL, CItemData::ET_SHORTCUT, 
                             CItemData::PASSWORD);
  possible_aliases.clear();

  // Some shortcuts may have been deleted from the database as base does not exist
  // Tidy up Tree/List
  UUIDListIter paiter;
  ItemListIter iter;
  for (paiter = possible_shortcuts.begin();
       paiter != possible_shortcuts.end(); paiter++) {
    paiter->GetUUID(entry_uuid);
    iter = m_core.Find(entry_uuid);
    if (iter != End()) {
      // Still in pwlist - NOW add to Tree and List views
      insertItem(m_core.GetEntry(iter));
    }
  }
  possible_shortcuts.clear();

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately)) {
    Save();
    ChangeOkUpdate();
  }

  FixListIndexes();
  RefreshViews();
}

// Return whether first [g:t:u] is greater than the second [g:t:u]
// used in std::sort in SortDependents below.
bool GTUCompare(const StringX &elem1, const StringX &elem2)
{
  StringX g1, t1, u1, g2, t2, u2, tmp1, tmp2;

  StringX::size_type i1 = g1.find_first_of(_T(":"));
  g1 = (i1 == StringX::npos) ? elem1 : elem1.substr(0, i1 - 1);
  StringX::size_type i2 = g2.find_first_of(_T(":"));
  g2 = (i2 == StringX::npos) ? elem2 : elem2.substr(0, i2 - 1);
  if (g1 != g2)
    return g1.compare(g2) < 0;

  tmp1 = elem1.substr(g1.length() + 1);
  tmp2 = elem2.substr(g2.length() + 1);
  i1 = tmp1.find_first_of(_T(":"));
  t1 = (i1 == StringX::npos) ? tmp1 : tmp1.substr(0, i1 - 1);
  i2 = tmp2.find_first_of(_T(":"));
  t2 = (i2 == StringX::npos) ? tmp2 : tmp2.substr(0, i2 - 1);
  if (t1 != t2)
    return t1.compare(t2) < 0;

  tmp1 = tmp1.substr(t1.length() + 1);
  tmp2 = tmp2.substr(t2.length() + 1);
  i1 = tmp1.find_first_of(_T(":"));
  u1 = (i1 == StringX::npos) ? tmp1 : tmp1.substr(0, i1 - 1);
  i2 = tmp2.find_first_of(_T(":"));
  u2 = (i2 == StringX::npos) ? tmp2 : tmp2.substr(0, i2 - 1);
  return u1.compare(u2) < 0;
}

void DboxMain::SortDependents(UUIDList &dlist, StringX &csDependents)
{
  std::vector<StringX> sorted_dependents;
  std::vector<StringX>::iterator sd_iter;

  ItemListIter iter;
  UUIDListIter diter;
  StringX cs_dependent;

  for (diter = dlist.begin(); diter != dlist.end(); diter++) {
    uuid_array_t dependent_uuid;
    diter->GetUUID(dependent_uuid);
    iter = m_core.Find(dependent_uuid);
    if (iter != m_core.GetEntryEndIter()) {
      cs_dependent = iter->second.GetGroup() + _T(":") +
                     iter->second.GetTitle() + _T(":") +
                     iter->second.GetUser();
      sorted_dependents.push_back(cs_dependent);
    }
  }

  std::sort(sorted_dependents.begin(), sorted_dependents.end(), GTUCompare);
  csDependents.clear();

  for (sd_iter = sorted_dependents.begin(); sd_iter != sorted_dependents.end(); sd_iter++) {
    csDependents += _T("\t[") +  *sd_iter + _T("]\r\n");
  }
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
  m_FindToolBar.Find();
}

bool DboxMain::CheckNewPassword(const StringX &group, const StringX &title,
                                const StringX &user, const StringX &password,
                                const bool bIsEdit, const CItemData::EntryType &InputType, 
                                uuid_array_t &base_uuid, int &ibasedata, bool &b_msg_issued)
{
  // bmsgissued - whether this routine issued a message
  b_msg_issued = false;

  // Called from Add and Edit entry
  // Returns false if not a special alias or shortcut password
  GetBaseEntryPL pl;
  pl.InputType = InputType;

  bool brc = m_core.GetBaseEntry(password, pl);

  // Copy data back before possibly returning
  ibasedata = pl.ibasedata;
  memcpy(base_uuid, pl.base_uuid, sizeof(uuid_array_t));
  if (!brc)    
    return false;

  // if we ever return 'false', this routine will have issued a message to the user
  b_msg_issued = true;

  if (bIsEdit && 
    (pl.csPwdGroup == group && pl.csPwdTitle == title && pl.csPwdUser == user)) {
    // In Edit, check user isn't changing entry to point to itself (circular/self reference)
    // Can't happen during Add as already checked entry does not exist so if accepted the
    // password would be treated as an unusal "normal" password
    AfxMessageBox(IDS_ALIASCANTREFERTOITSELF, MB_OK);
    return false;
  }

  // ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and is only valid if n = -1 or -2.

  if (pl.ibasedata < 0) {
    if (InputType == CItemData::ET_SHORTCUT) {
      // Target must exist (unlike for aliases where it could be an unusual password)
      if (pl.bMultipleEntriesFound)
        AfxMessageBox(IDS_MULTIPLETARGETSFOUND, MB_OK);
      else
        AfxMessageBox(IDS_TARGETNOTFOUND, MB_OK);
      return false;
    }

    CString cs_msg;
    const CString cs_msgA(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDA));
    const CString cs_msgZ(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDZ));
    int rc(IDNO);
    switch (pl.ibasedata) {
      case -1: // [t] - must be title as this is the only mandatory field
        if (pl.bMultipleEntriesFound)
          cs_msg.Format(IDS_ALIASNOTFOUND0A,
                        pl.csPwdTitle.c_str());  // multiple entries exist with title=x
        else
          cs_msg.Format(IDS_ALIASNOTFOUND0B,
                        pl.csPwdTitle.c_str());  // no entry exists with title=x
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -2: // [g,t], [t:u]
        // In this case the 2 fields from the password are in Group & Title
        if (pl.bMultipleEntriesFound)
          cs_msg.Format(IDS_ALIASNOTFOUND1A, 
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str(),
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str());
        else
          cs_msg.Format(IDS_ALIASNOTFOUND1B, 
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str(),
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str());
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, 
                           MB_YESNO | MB_DEFBUTTON2);
        break;
      case -3: // [g:t:u], [g:t:], [:t:u], [:t:] (title cannot be empty)
      {
        const bool bGE = pl.csPwdGroup.empty();
        const bool bTE = pl.csPwdTitle.empty();
        const bool bUE = pl.csPwdUser.empty();
        if (bTE) {
          // Title is mandatory for all entries!
          AfxMessageBox(IDS_BASEHASNOTITLE, MB_OK);
          rc = IDNO;
          break;
        } else if (!bGE && !bUE)  // [x:y:z]
          cs_msg.Format(IDS_ALIASNOTFOUND2A, 
                        pl.csPwdGroup.c_str(), 
                        pl.csPwdTitle.c_str(), 
                        pl.csPwdUser.c_str());
        else if (!bGE && bUE)     // [x:y:]
          cs_msg.Format(IDS_ALIASNOTFOUND2B, 
                        pl.csPwdGroup.c_str(), 
                        pl.csPwdTitle.c_str());
        else if (bGE && !bUE)     // [:y:z]
          cs_msg.Format(IDS_ALIASNOTFOUND2C, 
                        pl.csPwdTitle.c_str(), 
                        pl.csPwdUser.c_str());
        else if (bGE && bUE)      // [:y:]
          cs_msg.Format(IDS_ALIASNOTFOUND0B, 
                        pl.csPwdTitle.c_str());

        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, 
                           MB_YESNO | MB_DEFBUTTON2);
        break;
      }
      default:
        // Never happens
        ASSERT(0);
    }
    if (rc == IDNO)
      return false;
  }

  if (pl.ibasedata > 0) {
    if (pl.TargetType == CItemData::ET_ALIAS) {
      // If user tried to point to an alias -> change to point to the 'real' base
      CString cs_msg;
      cs_msg.Format(IDS_BASEISALIAS, 
                    pl.csPwdGroup.c_str(),
                    pl.csPwdTitle.c_str(),
                    pl.csPwdUser.c_str());
      if (AfxMessageBox(cs_msg, MB_YESNO | MB_DEFBUTTON2) == IDNO) {
        return false;
      }
    } else {
      if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
        // An alias can only point to a normal entry or an alias base entry
        CString cs_msg;
        cs_msg.Format(IDS_ABASEINVALID, 
                      pl.csPwdGroup.c_str(),
                      pl.csPwdTitle.c_str(), 
                      pl.csPwdUser.c_str());
        AfxMessageBox(cs_msg, MB_OK);
        return false;
      } else {
        return true;
      }
    }
  }

  // All OK
  return true;
}
