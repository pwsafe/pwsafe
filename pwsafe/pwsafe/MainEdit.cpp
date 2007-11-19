/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "KeySend.h"
#include "ClearQuestionDlg.h"

#include <vector>
#include <algorithm>

#include  <Winable.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Add an item
void
DboxMain::OnAdd()
{
  CAddDlg dlg_add(this);

  if (m_core.GetUseDefUser()) {
    dlg_add.m_username = m_core.GetDefUsername();
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
  dlg_add.m_group = m_TreeViewGroup;
  m_TreeViewGroup = _T(""); // for next time
  app.DisableAccelerator();
  INT_PTR rc = dlg_add.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    PWSprefs *prefs = PWSprefs::GetInstance();
    //Check if they wish to set a default username
    if (!m_core.GetUseDefUser()
        && (prefs->GetPref(PWSprefs::QuerySetDef))
        && (!dlg_add.m_username.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_message.Format(IDS_SETUSERNAME, (const CString&)dlg_add.m_username);
      INT_PTR rc2 = defDlg.DoModal();
      if (rc2 == IDOK) {
        prefs->SetPref(PWSprefs::UseDefaultUser, true);
        prefs->SetPref(PWSprefs::DefaultUsername,
                       dlg_add.m_username);
        m_core.SetUseDefUser(true);
        m_core.SetDefUsername(dlg_add.m_username);
        RefreshList();
      }
    }

    //Finish Check (Does that make any geographical sense?)
    CItemData temp;
    CMyString user;
    time_t t;

    if (dlg_add.m_username.IsEmpty() && m_core.GetUseDefUser())
      user = m_core.GetDefUsername();
    else
      user = dlg_add.m_username;
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
      m_core.AddAliasEntry(dlg_add.m_base_uuid, alias_uuid);
      temp.SetPassword(CMyString(_T("[Alias]")));
      temp.SetAlias();
      ItemListIter iter = m_core.Find(dlg_add.m_base_uuid);
      if (iter != End()) {
        const CItemData &cibase = iter->second;
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        HTREEITEM bti = di->tree_item;
        SetEntryImage(cibase, bti, true);
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
    if (temp.IsAlias())
      temp.SetLTime((time_t)0);
    else
      temp.SetLTime(dlg_add.m_tttLTime);
    if (dlg_add.m_SavePWHistory == TRUE) {
      TCHAR buffer[6];
#if _MSC_VER >= 1400
      _stprintf_s(buffer, 6, _T("1%02x00"), dlg_add.m_MaxPWHistory);
#else
      _stprintf(buffer, _T("1%02x00"), dlg_add.m_MaxPWHistory);
#endif
      temp.SetPWHistory(buffer);
    }

    AddEntry(temp);

    if (m_core.GetNumEntries() == 1) {
      // For some reason, when adding the first entry, it is not visible!
      m_ctlItemTree.SetRedraw(TRUE);
    }
    m_ctlItemList.SetFocus();
    if (prefs->GetPref(PWSprefs::SaveImmediately))
      Save();

    ChangeOkUpdate();
    uuid_array_t uuid;
    temp.GetUUID(uuid);
    m_RUEList.AddRUEntry(uuid);
  }
}

int
DboxMain::AddEntry(const CItemData &cinew)
{
  // This routine is used by Add and also Drag & Drop

  m_core.AddEntry(cinew);

  // AddEntry copies the entry, and we want to work with the inserted copy
  // Which we'll find by uuid
  uuid_array_t uuid;
  cinew.GetUUID(uuid);
  int newpos = insertItem(m_core.GetEntry(m_core.Find(uuid)));
  SelectEntry(newpos);
  FixListIndexes();
  return newpos;
}

//Add a group (tree view only)
void
DboxMain::OnAddGroup()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  if (m_ctlItemTree.IsWindowVisible()) {
    // This can be reached by right clicking over an existing group node
    // or by clicking over "whitespace".
    // If the former, add a child node to the current one
    // If the latter, add to root.
    CMyString cmys_text(MAKEINTRESOURCE(IDS_NEWGROUP));
    if (m_TreeViewGroup.IsEmpty())
      m_TreeViewGroup = cmys_text;
    else
      m_TreeViewGroup += _T(".") + cmys_text;
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
  }
}

void
DboxMain::Delete(bool inRecursion)
{
  CItemData *ci = getSelectedItem();

  if (ci != NULL) {
    uuid_array_t uuid;
    ci->GetUUID(uuid);

    UUIDList aliaslist;
    int num_aliases;

    m_core.GetAllAliasEntries(uuid, aliaslist);
    num_aliases = aliaslist.size();
    if (num_aliases > 0) {
      CMyString csAliases;
      SortAliasEntries(aliaslist, csAliases);

      CString cs_msg;
      const CString cs_title(MAKEINTRESOURCE(IDS_DELETEBASET));
      cs_msg.Format(IDS_DELETEBASE, aliaslist.size(),
                    aliaslist.size() == 1 ? _T("") : _T("es"), csAliases);
      if (MessageBox(cs_msg, cs_title, MB_ICONQUESTION | MB_YESNO) == IDNO) {
        aliaslist.clear();
        return;
      }
    }

    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    int curSel = di->list_index;
    // Find next in treeview, not always curSel after deletion
    HTREEITEM curTree_item = di->tree_item;
    HTREEITEM nextTree_item = m_ctlItemTree.GetNextItem(curTree_item,
                                                        TVGN_NEXT);
    // Must Find before delete from m_ctlItemList:
    ItemListIter listindex = m_core.Find(uuid);
    ASSERT(listindex !=  m_core.GetEntryEndIter());

    UnFindItem();
    m_ctlItemList.DeleteItem(curSel);
    m_ctlItemTree.DeleteWithParents(curTree_item);
    delete di;

    if (ci->NumberUnknownFields() > 0)
      m_core.DecrementNumRecordsWithUnknownFields();

    uuid_array_t base_uuid;
    if (ci->IsAlias()) {
      // I'm an alias entry
      // Get corresponding base uuid
      m_core.GetBaseUUID(uuid, base_uuid);
      // Delete from both map and multimap
      m_core.RemoveAliasEntry(base_uuid, uuid);

      // Does my base now become a normal entry?
      if (m_core.NumAliases(base_uuid) == 0) {
        ItemListIter iter = m_core.Find(base_uuid);
        CItemData &cibase = iter->second;
        cibase.SetNormal();
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        HTREEITEM bti = di->tree_item;
        SetEntryImage(cibase, bti, true);
      }
    }

    if (num_aliases > 0) {
      // I'm a base entry
      m_core.ResetAllAliasPasswords(uuid);
      m_core.RemoveAllAliasEntries(uuid);

      // Now make all my aliases Normal
      ItemListIter iter;
      UUIDListIter UUIDiter;
      for (UUIDiter = aliaslist.begin(); UUIDiter != aliaslist.end(); UUIDiter++) {
        uuid_array_t auuid;
        UUIDiter->GetUUID(auuid);
        iter = m_core.Find(auuid);
        CItemData &cialias = iter->second;
        DisplayInfo *di = (DisplayInfo *)cialias.GetDisplayInfo();
        HTREEITEM ati = di->tree_item;
        SetEntryImage(cialias, ati, true);
      }
      aliaslist.clear();
    }

    m_core.RemoveEntryAt(listindex);
    FixListIndexes();
    if (m_ctlItemList.IsWindowVisible()) {
      if (m_core.GetNumEntries() > 0) {
        SelectEntry(curSel < (int)m_core.GetNumEntries() ?
                    curSel : (int)(m_core.GetNumEntries() - 1));
      }
      m_ctlItemList.SetFocus();
    } else {// tree view visible
      if (!inRecursion && nextTree_item != NULL) {
        m_ctlItemTree.SelectItem(nextTree_item);
      }
      m_ctlItemTree.SetFocus();
    }
    ChangeOkUpdate();
    m_RUEList.DeleteRUEntry(uuid);
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

void
DboxMain::OnRename()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
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
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
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

bool
DboxMain::EditItem(CItemData *ci, PWScore *pcore)
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

    ci->GetUUID(original_uuid);  // Edit doesn't change this!
    if (ci->IsBase()) {
      // Base entry
      UUIDList aliaslist;
      CMyString csAliases(_T(""));

      m_core.GetAllAliasEntries(original_uuid, aliaslist);
      int num_aliases = aliaslist.size();
      if (num_aliases > 0) {
        SortAliasEntries(aliaslist, csAliases);
      }

      dlg_edit.m_numaliases = num_aliases;
      dlg_edit.m_aliases = csAliases;
      dlg_edit.m_original_entrytype = CItemData::Base;
      aliaslist.clear();
    }

    if (ci->IsAlias()) {
      // Alias entry
      // Get corresponding base uuid
      m_core.GetBaseUUID(original_uuid, original_base_uuid);

      ItemListIter iter = m_core.Find(original_base_uuid);
      if (iter != End()) {
        const CItemData &cibase = iter->second;
        dlg_edit.m_base = _T("[") +
                 cibase.GetGroup() + _T(":") +
                 cibase.GetTitle() + _T(":") +
                 cibase.GetUser()  + _T("]");
        dlg_edit.m_original_entrytype = CItemData::Alias;
      }
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
      CMyString newPassword = editedItem.GetPassword();
      memcpy(new_base_uuid, dlg_edit.m_base_uuid, sizeof(uuid_array_t));

      ItemListIter iter;
      if (dlg_edit.m_original_entrytype == CItemData::Normal &&
          ci->GetPassword() != newPassword) {
        // Original was a 'normal' entry and the password has changed
        if (dlg_edit.m_ibasedata > 0) {
          // Now an alias
          pcore->AddAliasEntry(new_base_uuid, original_uuid);
          editedItem.SetPassword(CMyString(_T("[Alias]")));
          editedItem.SetAlias();
        } else {
          // Still 'normal'
          editedItem.SetPassword(newPassword);
          editedItem.SetNormal();
        }
      }

      if (dlg_edit.m_original_entrytype == CItemData::Alias) {
        // Original was an alias - delete it from multimap
        // RemoveAliasEntry also resets base to normal if the last alias is delete
        pcore->RemoveAliasEntry(original_base_uuid, original_uuid);
        if (newPassword == dlg_edit.m_base) {
          // Password (i.e. base) unchanged - put it back
          pcore->AddAliasEntry(original_base_uuid, original_uuid);
        } else {
          // Password changed so might be an alias of another entry!
          // Could also be the same entry i.e. [:t:] == [t] !
          if (dlg_edit.m_ibasedata > 0) {
            // Still an alias
            pcore->AddAliasEntry(new_base_uuid, original_uuid);
            editedItem.SetPassword(CMyString(_T("[Alias]")));
            editedItem.SetAlias();
          } else {
            // No longer an alias
            editedItem.SetPassword(newPassword);
            editedItem.SetNormal();
          }
        }
      }

      if (dlg_edit.m_original_entrytype == CItemData::Base &&
          ci->GetPassword() != newPassword) {
        // Original was a base but might now be an alias of another entry!
        if (dlg_edit.m_ibasedata > 0) {
          // Now an alias
          // Make this one an alias
          pcore->AddAliasEntry(new_base_uuid, original_uuid);
          editedItem.SetPassword(CMyString(_T("[Alias]")));
          editedItem.SetAlias();
          // Move old aliases across
          pcore->MoveAliases(original_uuid, new_base_uuid);
        } else {
          // Still a base entry but with a new password
          editedItem.SetPassword(newPassword);
          editedItem.SetBase();
        }
      }

      // Reset all images!
      // First the edited entry
      iter = m_core.Find(original_uuid);
      if (iter != End()) {
        const CItemData &cibase = iter->second;
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        HTREEITEM bti = di->tree_item;
        SetEntryImage(cibase, bti, true);
      }
      // Next the original base entry
      iter = m_core.Find(original_base_uuid);
      if (iter != End()) {
        const CItemData &cibase = iter->second;
        DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
        HTREEITEM bti = di->tree_item;
        SetEntryImage(cibase, bti, true);
      }
      // Last the new base entry (only if different to the one we have done!
      if (::memcmp(new_base_uuid, original_base_uuid, sizeof(uuid_array_t)) != 0) {
        iter = m_core.Find(new_base_uuid);
        if (iter != End()) {
          const CItemData &cibase = iter->second;
          DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
          HTREEITEM bti = di->tree_item;
          SetEntryImage(cibase, bti, true);
        }
      }

      if (editedItem.IsAlias())
        editedItem.SetLTime((time_t)0);

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
      rc = SelectEntry(ndi->list_index);
      if (rc == LB_ERR) {
        SelectEntry(m_ctlItemList.GetItemCount() - 1);
      }
      ChangeOkUpdate();
      return true;
    } // rc == IDOK
    return false;
}

// Duplicate selected entry but make title unique
void
DboxMain::OnDuplicateEntry()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

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
    ItemListConstIter listpos;
    int i = 0;
    CString s_copy;
    do {
      i++;
      s_copy.Format(IDS_COPYNUMBER, i);
      ci2_title = ci2_title0 + CMyString(s_copy);
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
    ci->GetCTime(t);
    if ((long) t != 0)
    	ci2.SetCTime(t);
    ci->GetATime(t);
    if ((long) t != 0)
    	ci2.SetATime(t);
    ci->GetLTime(t);
    if ((long) t != 0)
    	ci2.SetLTime(t);
    ci->GetPMTime(t);
    if ((long) t != 0)
    	ci2.SetPMTime(t);
    ci->GetRMTime(t);
    if ((long) t != 0)
    	ci2.SetRMTime(t);
    CMyString tmp = ci->GetPWHistory();
    if (tmp.GetLength() >= 5)
    	    ci2.SetPWHistory(tmp);

    uuid_array_t base_uuid, original_alias_uuid, new_alias_uuid;
    if (ci->IsAlias()) {
      ci->GetUUID(original_alias_uuid);
      m_core.GetBaseUUID(original_alias_uuid, base_uuid);
      ci2.SetAlias();
      ci2.GetUUID(new_alias_uuid);
      m_core.AddAliasEntry(base_uuid, new_alias_uuid);

      ItemListIter iter;
      iter = m_core.Find(base_uuid);
      if (iter != m_core.GetEntryEndIter()) {
        CMyString cs_tmp;
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
    if (rc == LB_ERR) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();
	m_RUEList.AddRUEntry(uuid);
  }
}

void
DboxMain::OnCopyPassword()
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

  uuid_array_t base_uuid, alias_uuid;
  if (ci->IsAlias()) {
    // This is an alias
    ci->GetUUID(alias_uuid);
    m_core.GetBaseUUID(alias_uuid, base_uuid);
    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      SetClipboardData(iter->second.GetPassword());
    }
  } else
  SetClipboardData(ci->GetPassword());

  UpdateAccessTime(ci);
  uuid_array_t RUEuuid;
  ci->GetUUID(RUEuuid);
  m_RUEList.AddRUEntry(RUEuuid);
}

void
DboxMain::OnCopyUsername()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);
  const CMyString username = ci->GetUser();

  if (!username.IsEmpty()) {
    SetClipboardData(username);
    UpdateAccessTime(ci);
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::OnCopyNotes()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  const CMyString notes = ci->GetNotes();
  const CMyString url = ci->GetURL();
  const CMyString autotype = ci->GetAutoType();
  CMyString clipboard_data;
  CString cs_text;

  clipboard_data = notes;
  if (!url.IsEmpty()) {
    cs_text.LoadString(IDS_COPYURL);
    clipboard_data += CMyString(cs_text);
    clipboard_data += url;
  }
  if (!autotype.IsEmpty()) {
    cs_text.LoadString(IDS_COPYAUTOTYPE);
    clipboard_data += CMyString(cs_text);
    clipboard_data += autotype;
  }
  if (!clipboard_data.IsEmpty()) {
    SetClipboardData(clipboard_data);
    UpdateAccessTime(ci);
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::OnCopyURL()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);
  const CMyString cs_URL = ci->GetURL();

  if (!cs_URL.IsEmpty()) {
    SetClipboardData(cs_URL);
    UpdateAccessTime(ci);
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::OnFind()
{
  // Note that this "toggles" the Find Tool Bar so that the user can use Ctrl+F
  // to show it and then hide it.
  SetFindToolBar(!m_FindToolBar.IsVisible());
}

void
DboxMain::OnClearClipboard()
{
   ClearClipboardData();
}

void
DboxMain::OnAutoType()
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
	uuid_array_t RUEuuid;
	ci->GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);
    UpdateAccessTime(ci);
    // All code using ci must be before this AutoType since the
	// latter may trash *ci if lock-on-minimize
	AutoType(*ci);
  }
}

const CString DboxMain::DEFAULT_AUTOTYPE = _T("\\u\\t\\p\\n");

void
DboxMain::AutoType(const CItemData &ci)
{
  CMyString AutoCmd = ci.GetAutoType();
  const CMyString user(ci.GetUser());
  CMyString pwd;

  uuid_array_t base_uuid, alias_uuid;
  if (ci.IsAlias()) {
    // This is an alias
    ci.GetUUID(alias_uuid);
    m_core.GetBaseUUID(alias_uuid, base_uuid);
    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      pwd = iter->second.GetPassword();
    }
  } else {
    pwd = ci.GetPassword();
  }

  // If empty, try the database default
  if (AutoCmd.IsEmpty()) {
    AutoCmd = PWSprefs::GetInstance()->
      GetPref(PWSprefs::DefaultAutotypeString);

    // If still empty, take this default
    if (AutoCmd.IsEmpty()) {
      // checking for user and password for default settings
      if (!pwd.IsEmpty()){
        if (!user.IsEmpty())
          AutoCmd = CMyString(DEFAULT_AUTOTYPE);
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

  CMyString tmp;
  TCHAR curChar;
  const int N = AutoCmd.GetLength();
  ks.ResetKeyboardState();

  ::BlockInput(true);

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

  for(int n = 0; n < N; n++){
    curChar = AutoCmd[n];
    if(curChar == TCHAR('\\')) {
      n++;
      if(n < N)
        curChar=AutoCmd[n];
      switch(curChar){
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
      case TCHAR('d'): {
        // Delay is going to change - send what we have with old delay
        ks.SendString(tmp);
        // start collecting new delay
        tmp = _T("");
        int newdelay = 0;
        int gNumIts = 0;

        for(n++; n < N && (gNumIts < 3); ++gNumIts, n++)
          if(isdigit(AutoCmd[n])){
            newdelay *= 10;
            newdelay += (AutoCmd[n] - TCHAR('0'));
          } else
            break; // for loop
        n--;
        ks.SetAndDelay(newdelay);

        break; // case
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

  ::BlockInput(false);

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

void
DboxMain::AddEntries(CDDObList &in_oblist, const CMyString &DropGroup)
{
  CItemData tempitem;
  UUIDList possible_aliases;
  CMyString Group, Title, User;
  POSITION pos;
  TCHAR *dot;

  for (pos = in_oblist.GetHeadPosition(); pos != NULL; in_oblist.GetNext(pos)) {
    CDDObject *pDDObject = (CDDObject *)in_oblist.GetAt(pos);
    tempitem.Clear();
    pDDObject->ToItem(tempitem);

    if (in_oblist.m_bDragNode) {
      dot = (!DropGroup.IsEmpty() && !tempitem.GetGroup().IsEmpty()) ? _T(".") : _T("");
      Group = DropGroup + dot + tempitem.GetGroup();
    } else {
      Group = DropGroup;
    }

    User = tempitem.GetUser();
    Title = GetUniqueTitle(Group, tempitem.GetTitle(), User, IDS_DRAGNUMBER);

    uuid_array_t entry_uuid;
    tempitem.GetUUID(entry_uuid);
    if (m_core.Find(entry_uuid) != End())
      tempitem.CreateUUID();

    tempitem.SetGroup(Group);
    tempitem.SetTitle(Title);

    uuid_array_t base_uuid, alias_uuid;
    CMyString cs_tmp = tempitem.GetPassword();
    CMyString csPwdGroup, csPwdTitle, csPwdUser;
    bool bBase_was_Alias(false), bMultiple(false);
    int ialias = m_core.GetBaseEntry(cs_tmp, base_uuid, bBase_was_Alias, bMultiple,
                              csPwdGroup, csPwdTitle, csPwdUser);
    if (ialias > 0) {
      // Password in alias format AND base entry exists
      ItemListIter iter = m_core.Find(base_uuid);
      ASSERT(iter != End());
      if (bBase_was_Alias) {
        // This base is in fact an alias. GetBaseEntry already found 'proper base'
        // So dropped entry will point to the 'proper base' and tell the user.
        CString cs_msg;
        cs_msg.Format(IDS_DDBASEISALIAS, Group, Title, User);
        AfxMessageBox(cs_msg, MB_OK);
      }
      tempitem.GetUUID(alias_uuid);
      m_core.AddAliasEntry(base_uuid, alias_uuid);
      tempitem.SetPassword(CMyString(_T("[Alias]")));
      tempitem.SetAlias();
    } else
    if (ialias == 0) {
      // Password NOT in alias format
      tempitem.SetNormal();
    } else
    if (ialias < 0) {
      // Password in alias format AND base entry does not exist or multiple possible
      // base entries exit.
      // Note: As more entries are added, what was "not exist" may become "no unique exists"
      // or "multiple exist".  Let the code that processes the possible aliases after all
      // have been added sort this out.
      tempitem.GetUUID(alias_uuid);
      possible_aliases.push_back(alias_uuid);
    }

    AddEntry(tempitem);

  } // iteration over in_oblist

  // Now try to add aliases we couldn't add in previous processing
  m_core.AddAliasesViaPassword(possible_aliases, NULL);

  if (PWSprefs::GetInstance()->
      GetPref(PWSprefs::SaveImmediately)) {
    Save();
    ChangeOkUpdate();
  }

  FixListIndexes();
  RefreshList();
}

// Return whether first [g:t:u] is greater than the second [g:t:u]
// used in std::sort in SortAliasEntries below.
bool GTUCompare(CMyString elem1, CMyString elem2)
{
  CMyString g1, t1, u1, g2, t2, u2, tmp1, tmp2;

  g1 = elem1.SpanExcluding(_T(":"));
  g2 = elem2.SpanExcluding(_T(":"));
  if (g1 != g2)
    return g1.Compare(g2) < 0;

  tmp1 = elem1.Mid(g1.GetLength() + 1);
  tmp2 = elem2.Mid(g2.GetLength() + 1);
  t1 = tmp1.SpanExcluding(_T(":"));
  t2 = tmp2.SpanExcluding(_T(":"));
  if (t1 != t2)
    return t1.Compare(t2) < 0;

  tmp1 = tmp1.Mid(t1.GetLength() + 1);
  tmp2 = tmp2.Mid(t2.GetLength() + 1);
  u1 = tmp1.SpanExcluding(_T(":"));
  u2 = tmp2.SpanExcluding(_T(":"));
  return u1.Compare(u2) < 0;
}

void DboxMain::SortAliasEntries(UUIDList &aliaslist, CMyString &csAliases)
{
  std::vector<CMyString> sorted_aliases;
  std::vector<CMyString>::iterator sa_iter;

  ItemListIter iter;
  UUIDListIter aiter;
  CMyString cs_alias;

  for (aiter = aliaslist.begin(); aiter != aliaslist.end(); aiter++) {
    uuid_array_t alias_uuid;
    aiter->GetUUID(alias_uuid);
    iter = m_core.Find(alias_uuid);
    if (iter != m_core.GetEntryEndIter()) {
      cs_alias = iter->second.GetGroup() + _T(":") +
                 iter->second.GetTitle() + _T(":") +
                 iter->second.GetUser();
      sorted_aliases.push_back(cs_alias);
    }
  }

  std::sort(sorted_aliases.begin(), sorted_aliases.end(), GTUCompare);
  csAliases.Empty();

  for (sa_iter = sorted_aliases.begin(); sa_iter != sorted_aliases.end(); sa_iter++) {
    csAliases += _T("\t[") +  *sa_iter + _T("]\r\n");
  }
}

LRESULT
DboxMain::OnToolBarFindMessage(WPARAM /* wParam */, LPARAM /* lParam */)
{
  // Called when user types into the Find search edit control on the Find Toolbar
  // and presses enter.
  OnToolBarFind();
  return 0L;
}

void DboxMain::OnToolBarFind()
{
  // Called when the user presses the Find button on the Find Toolbar
  m_FindToolBar.Find();
}

bool
DboxMain::CheckNewPassword(const CMyString &group, const CMyString &title,
                           const CMyString &user, const CMyString &password,
                           const bool bIsEdit, uuid_array_t &base_uuid, int &ibasedata)
{
  // Called from Add and Edit dialog
  CMyString csPwdGroup, csPwdTitle, csPwdUser;
  bool bBase_was_Alias(false), bMultiple(false);
  ibasedata = m_core.GetBaseEntry(password, base_uuid, bBase_was_Alias, bMultiple,
                                  csPwdGroup, csPwdTitle, csPwdUser);

  if (bIsEdit && (csPwdGroup == group && csPwdTitle == title && csPwdUser == user)) {
    // In Edit, check user isn't changing entry to point to itself (circular/self reference)
    // Can't happen during Add as already checked entry does not exist so if accepted the
    // password would be treated as an unusal "normal" password
    AfxMessageBox(IDS_ALIASCANTREFERTOITSELF);
    return false;
  }

  // ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

  // "bBase_was_Alias" is set if the user specified a base entry that is an alias.  The real base is returned
  // "bMultiple" is set if no "unique" base entry could be found and is only valid if n = -1 or -2.

  if (ibasedata < 0) {
    CString cs_msg;
    const CString cs_msgA(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDA));
    const CString cs_msgZ(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDZ));
    int rc(IDNO);
    switch (ibasedata) {
      case -1: // [x]
        if (bMultiple)
          cs_msg.Format(IDS_ALIASNOTFOUND0A, csPwdTitle);  // multiple entries exist with title=x
        else
          cs_msg.Format(IDS_ALIASNOTFOUND0B, csPwdTitle);  // no entry exists with title=x
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -2: // [g,t], [t:u]
        // In this case the 2 fields from the password are in Group & Title
        if (bMultiple)
          cs_msg.Format(IDS_ALIASNOTFOUND1A, csPwdGroup, csPwdTitle, csPwdGroup, csPwdTitle);
        else
          cs_msg.Format(IDS_ALIASNOTFOUND1B, csPwdGroup, csPwdTitle, csPwdGroup, csPwdTitle);
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -3: // [x:y:z], [x:y:], [:y:z], [:y:] (title cannot be empty)
        {
        const bool bGE = csPwdGroup.IsEmpty() == TRUE;
        const bool bTE = csPwdTitle.IsEmpty() == TRUE;
        const bool bUE = csPwdUser.IsEmpty() == TRUE;
        if (bTE) {
          // Title is mandatory for all entries!
          AfxMessageBox(IDS_BASEHASNOTITLE, MB_OK);
          rc = IDNO;
          break;
        } else if (!bGE && !bUE)  // [x:y:z]
          cs_msg.Format(IDS_ALIASNOTFOUND2A, csPwdGroup, csPwdTitle, csPwdUser);
        else if (!bGE && bUE)     // [x:y:]
          cs_msg.Format(IDS_ALIASNOTFOUND2B, csPwdGroup, csPwdTitle);
        else if (bGE && !bUE)     // [:y:z]
          cs_msg.Format(IDS_ALIASNOTFOUND2C, csPwdTitle, csPwdUser);
        else if (bGE && bUE)      // [:y:]
          cs_msg.Format(IDS_ALIASNOTFOUND0B, csPwdTitle);
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        }
        break;
      default:
        // Never happens
        ASSERT(0);
    }
    if (rc == IDNO) {
      return false;
    }
  }

  if (ibasedata > 0 && bBase_was_Alias) {
    CString cs_msg;
    cs_msg.Format(IDS_BASEISALIAS, csPwdGroup, csPwdTitle, csPwdUser);
    if (AfxMessageBox(cs_msg, MB_YESNO | MB_DEFBUTTON2) == IDNO) {
      return false;
    }
  }

  // All OK
  return true;
}
