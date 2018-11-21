/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWSTreeCtrl.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

////@begin includes
#include <wx/imaglist.h>
#include <wx/tokenzr.h>
////@end includes

#include "PWStree.h"
#include "passwordsafeframe.h"
#include "core/PWSprefs.h"
#include "../../core/Command.h"

#include <utility> // for make_pair
#include <vector>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images
#include "./graphics/abase_exp.xpm"
#include "./graphics/abase_warn.xpm"
#include "./graphics/abase.xpm"
#include "./graphics/alias.xpm"
#include "./graphics/node.xpm"
#include "./graphics/normal_exp.xpm"
#include "./graphics/normal_warn.xpm"
#include "./graphics/normal.xpm"
#include "./graphics/sbase_exp.xpm"
#include "./graphics/sbase_warn.xpm"
#include "./graphics/sbase.xpm"
#include "./graphics/shortcut.xpm"

using pws_os::CUUID;

/*!
 * PWSTreeCtrl type definition
 */

IMPLEMENT_CLASS( PWSTreeCtrl, wxTreeCtrl )

// Image Indices - these match the order images are added
//                 in PWSTreeCtrl::CreateControls()
enum {
  ABASE_EXP_II,    // 0
  ABASE_WARN_II,   // 1
  ABASE_II,        // 2
  ALIAS_II,        // 3
  NODE_II,         // 4
  NORMAL_EXP_II,   // 5
  NORMAL_WARN_II,  // 6
  NORMAL_II,       // 7
  SBASE_EXP_II,    // 8
  SBASE_WARN_II,   // 9
  SBASE_II,        // 10
  SHORTCUT_II,     // 11
};

// We use/need this ID to re-post to AddPendingEvent the wxTreeEvent from END_LABEL notification.
// We need to re-post as we cannot touch the wxTreeCtrl at all in the actual notification. So we have
// to let that stack unwind.
enum {ID_TREECTRL_1 = ID_TREECTRL + 1 };

/*!
 * PWSTreeCtrl event table definition
 */

BEGIN_EVENT_TABLE( PWSTreeCtrl, wxTreeCtrl )

////@begin PWSTreeCtrl event table entries
  EVT_TREE_SEL_CHANGED( ID_TREECTRL, PWSTreeCtrl::OnTreectrlSelChanged )
  EVT_TREE_ITEM_ACTIVATED( ID_TREECTRL, PWSTreeCtrl::OnTreectrlItemActivated )
  EVT_TREE_ITEM_MENU( ID_TREECTRL, PWSTreeCtrl::OnContextMenu )
  EVT_CUSTOM(wxEVT_GUI_DB_PREFS_CHANGE, wxID_ANY, PWSTreeCtrl::OnDBGUIPrefsChange)
  EVT_TREE_ITEM_GETTOOLTIP( ID_TREECTRL, PWSTreeCtrl::OnGetToolTip )
  EVT_MENU( ID_ADDGROUP, PWSTreeCtrl::OnAddGroup )
  EVT_MENU( ID_RENAME, PWSTreeCtrl::OnRenameGroup )
  EVT_TREE_END_LABEL_EDIT( ID_TREECTRL, PWSTreeCtrl::OnEndLabelEdit )
  EVT_TREE_END_LABEL_EDIT( ID_TREECTRL_1, PWSTreeCtrl::OnEndLabelEdit )
  EVT_TREE_KEY_DOWN( ID_TREECTRL, PWSTreeCtrl::OnKeyDown )
////@end PWSTreeCtrl event table entries
END_EVENT_TABLE()

const wchar_t GROUP_SEP = L'.';

// helper class to match CItemData with wxTreeItemId
class PWTreeItemData : public wxTreeItemData
{
public:
  PWTreeItemData(): m_state(ItemState::UNMODIFIED)
  { 
    pws_os::CUUID::NullUUID().GetARep(m_uuid);
  }
  
  PWTreeItemData(bool): m_state(ItemState::ADDED)
  { 
    pws_os::CUUID::NullUUID().GetARep(m_uuid);
  }
  
  PWTreeItemData(const wxString& oldPath): m_state(ItemState::EDITED), m_oldPath(oldPath)
  { 
    pws_os::CUUID::NullUUID().GetARep(m_uuid);
  }
  
  PWTreeItemData(const CItemData &item): m_state(ItemState::UNMODIFIED)
  {
    item.GetUUID(m_uuid);
  }
  
  const uuid_array_t &GetUUID() const { return m_uuid; }
  bool BeingAdded() const { return m_state == ItemState::ADDED; }
  bool BeingEdited() const { return m_state == ItemState::EDITED; }
  wxString GetOldPath() const { return m_oldPath; }
  
private:
  enum class ItemState { UNMODIFIED, ADDED, EDITED };

  uuid_array_t m_uuid;
  ItemState    m_state;
  wxString     m_oldPath;
};

/*!
 * PWSTreeCtrl constructors
 */

PWSTreeCtrl::PWSTreeCtrl(PWScore &core) : m_core(core)
{
  Init();
}

PWSTreeCtrl::PWSTreeCtrl(wxWindow* parent, PWScore &core,
                         wxWindowID id, const wxPoint& pos,
                         const wxSize& size, long style) : m_core(core)
{
  Init();
  Create(parent, id, pos, size, style);
}

/*!
 * PWSTreeCtrl creator
 */

bool PWSTreeCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin PWSTreeCtrl creation
  wxTreeCtrl::Create(parent, id, pos, size, style);
  CreateControls();
////@end PWSTreeCtrl creation
  return true;
}

/*!
 * PWSTreeCtrl destructor
 */

PWSTreeCtrl::~PWSTreeCtrl()
{
////@begin PWSTreeCtrl destruction
////@end PWSTreeCtrl destruction
}

/*!
 * Member initialisation
 */

void PWSTreeCtrl::Init()
{
////@begin PWSTreeCtrl member initialisation
////@end PWSTreeCtrl member initialisation
}

/*!
 * Control creation for PWSTreeCtrl
 */

void PWSTreeCtrl::CreateControls()
{
////@begin PWSTreeCtrl content construction
////@end PWSTreeCtrl content construction
  const char **xpmList[] = {
    abase_exp_xpm,    // 0
    abase_warn_xpm,   // 1
    abase_xpm,        // 2
    alias_xpm,        // 3
    node_xpm,         // 4
    normal_exp_xpm,   // 5
    normal_warn_xpm,  // 6
    normal_xpm,       // 7
    sbase_exp_xpm,    // 8
    sbase_warn_xpm,   // 9
    sbase_xpm,        // 10
    shortcut_xpm,     // 11
  };
  const int Nimages = sizeof(xpmList)/sizeof(xpmList[0]);

  auto *iList = new wxImageList(13, 13, true, Nimages);
  for (int i = 0; i < Nimages; i++)
    iList->Add(wxBitmap(xpmList[i]));
  AssignImageList(iList);
}

bool PWSTreeCtrl::ItemIsGroup(const wxTreeItemId& item) const
{
  int image = GetItemImage(item);
  return image == NODE_II || GetRootItem() == item;
}

// XXX taken from Windows PWSTreeCtrl.cpp
// XXX move to core
static StringX GetPathElem(StringX &sxPath)
{
  // Get first path element and chop it off, i.e., if
  // path = "a.b.c.d"
  // will return "a" and path will be "b.c.d"
  // path = "a..b.c.d"
  // will return "a." and path will be "b.c.d"
   // (assuming GROUP_SEP is '.')

  StringX sxElement;
  size_t dotPos = sxPath.find_first_of(GROUP_SEP);
  size_t len=sxPath.length();
  if (dotPos == StringX::npos){
    sxElement = sxPath;
    sxPath = wxEmptyString;
  } else {
    while ((dotPos < len) && (sxPath[dotPos] == GROUP_SEP)) {// look for consecutive dots
      dotPos++;
    }
    if (dotPos < len) {
      sxElement = sxPath.substr(0, dotPos-1);
      sxPath = sxPath.substr(dotPos);
    }
    else { // trailing dots
      sxElement = sxPath;
      sxPath = wxEmptyString;
    }
  }
  return sxElement;
}

bool PWSTreeCtrl::ExistsInTree(wxTreeItemId node,
                               const StringX &s, wxTreeItemId &si) const
{
  // returns true iff s is a direct descendant of node
  wxTreeItemIdValue cookie;
  wxTreeItemId ti = GetFirstChild(node, cookie);

  while (ti) {
    const wxString itemText = GetItemText(ti);
    if (itemText == s.c_str()) {
      si = ti;
      return true;
    }
    ti = GetNextSibling(ti);
  }
  return false;
}

wxTreeItemId PWSTreeCtrl::AddGroup(const StringX &group)
{
  wxTreeItemId ti = GetRootItem();
  if (!ti.IsOk())
    ti=AddRoot(wxString());

  // Add a group at the end of path
  wxTreeItemId si;
  if (!group.empty()) {
    StringX path = group;
    StringX s;
    do {
      s = GetPathElem(path);
      if (!ExistsInTree(ti, s, si)) {
        ti = AppendItem(ti, s.c_str());
        wxTreeCtrl::SetItemImage(ti, NODE_II);
      } else
        ti = si;
    } while (!path.empty());
  }
  return ti;
}

wxString PWSTreeCtrl::ItemDisplayString(const CItemData &item) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  const wxString title = item.GetTitle().c_str();

  // Title is a mandatory field - no need to worry if empty
  wxString disp = title;

  if (prefs->GetPref(PWSprefs::ShowUsernameInTree)) {
    const wxString user = item.GetUser().c_str();
    // User is NOT a mandatory field - but show not present by empty brackets i.e. []
    // if user wants it displayed
    disp += wxT(" [") + user + wxT("]");
  }

  if (prefs->GetPref(PWSprefs::ShowPasswordInTree)) {
    const wxString passwd = item.GetPassword().c_str();
    // Password is a mandatory field - no need to worry if empty
    disp += wxT(" {") + passwd + wxT("}");
  }

  if (item.IsProtected()) { 
    disp += wxT(" #");
#ifdef NOTYET
    wxUniChar padlock(0x1f512);
    disp +=padlock;
#endif
  }

  return disp;
}

wxString PWSTreeCtrl::GetPath(const wxTreeItemId &node) const
{
  wxString retval;
  std::vector<wxString> v;
  const wxTreeItemId root = GetRootItem();
  wxTreeItemId parent = GetItemParent(node);

  while (parent != root) {
    v.push_back(GetItemText(parent));
    parent = GetItemParent(parent);
  }
  std::vector<wxString>::reverse_iterator iter;
  for(iter = v.rbegin(); iter != v.rend(); iter++) {
    retval += *iter;
    if ((iter + 1) != v.rend())
      retval += wxT(".");
  }
  return retval;
}

wxString PWSTreeCtrl::GetItemGroup(const wxTreeItemId& item) const
{
  if (!item.IsOk() || item == GetRootItem())
    return wxEmptyString;
  else if (ItemIsGroup(item)) {
    const wxString path = GetPath(item);
    const wxString name = GetItemText(item);
    if (path.IsEmpty())//parent is root
      return name; //group under root
    else
      return path + wxT(".") + name; //sub-group of some (non-root) group
  }
  else
    return GetPath(item);
}

void PWSTreeCtrl::UpdateItem(const CItemData &item)
{
  const wxTreeItemId node = Find(item);
  if (node.IsOk()) {
    const wxString oldGroup = GetPath(node);
    const wxString newGroup = item.GetGroup().c_str();
    if (oldGroup == newGroup) {
      const wxString disp = ItemDisplayString(item);
      SetItemText(node, disp);
      SetItemImage(node, item);
    } else { // uh-oh - group's changed
      uuid_array_t uuid;
      item.GetUUID(uuid);
      // remove old item
      m_item_map.erase(CUUID(uuid));
      Delete(node);
      // add new group
      AddItem(item);
    }
    Update();
  }
}

//Just update the item's text, don't move it into its sorted position
void PWSTreeCtrl::UpdateItemField(const CItemData &item, CItemData::FieldType ft)
{
  PWSprefs* prefs = PWSprefs::GetInstance();
  if (ft == CItemData::GROUP) {
    //remove & add again
    UpdateItem(item);
  }
  //these are the only items ever shown in the tree
  else if (ft == CItemData::TITLE || ft == CItemData::START ||
       (ft == CItemData::USER && prefs->GetPref(PWSprefs::ShowUsernameInTree)) ||
       (ft == CItemData::PASSWORD && prefs->GetPref(PWSprefs::ShowPasswordInTree))) {
    wxTreeItemId ti = Find(item);
    if (ti.IsOk()) {
      SetItemText(ti, ItemDisplayString(item));
    }
  }
}

void PWSTreeCtrl::AddItem(const CItemData &item)
{
  wxTreeItemData *data = new PWTreeItemData(item);
  wxTreeItemId gnode = AddGroup(item.GetGroup());
  const wxString disp = ItemDisplayString(item);
  wxTreeItemId titem = AppendItem(gnode, disp, -1, -1, data);
  SetItemImage(titem, item);
  SortChildrenRecursively(gnode);
  uuid_array_t uuid;
  item.GetUUID(uuid);
  m_item_map.insert(std::make_pair(CUUID(uuid), titem));
}

CItemData *PWSTreeCtrl::GetItem(const wxTreeItemId &id) const
{
  if (!id.IsOk())
    return nullptr;

  auto *itemData = dynamic_cast<PWTreeItemData *>(GetItemData(id));
  // return if a group is selected
  if (itemData == nullptr)
    return nullptr;

  auto itemiter = m_core.Find(itemData->GetUUID());
  if (itemiter == m_core.GetEntryEndIter())
    return nullptr;
  return &itemiter->second;
}

//overridden from base for case-insensitive sort
int PWSTreeCtrl::OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
{
  const bool groupsFirst = PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree),
             item1isGroup = ItemIsGroup(item1),
             item2isGroup = ItemIsGroup(item2);

  if (groupsFirst) {
    if (item1isGroup && !item2isGroup)
      return -1;
    else if (item2isGroup && !item1isGroup)
      return 1;
  }

  const wxString text1 = GetItemText(item1);
  const wxString text2 = GetItemText(item2);
  return text1.CmpNoCase(text2);
}

void PWSTreeCtrl::SortChildrenRecursively(const wxTreeItemId& item)
{
  if (!ItemIsGroup(item) || GetChildrenCount(item) <= 0)
    return;

  SortChildren(item);

  wxTreeItemIdValue cookie;
  for( wxTreeItemId childId = GetFirstChild(item, cookie); childId.IsOk(); childId = GetNextChild(item, cookie)) {
    if (ItemIsGroup(childId) && GetChildrenCount(childId) > 0) { //logically redundant, but probably more efficient
      SortChildrenRecursively(childId);
    }
  }
}

wxTreeItemId PWSTreeCtrl::Find(const CUUID &uuid) const
{
  wxTreeItemId fail;
  auto iter = m_item_map.find(uuid);
  if (iter != m_item_map.end())
    return iter->second;
  else
    return fail;
}

wxTreeItemId PWSTreeCtrl::Find(const CItemData &item) const
{
  uuid_array_t uuid;
  item.GetUUID(uuid);
  return Find(uuid);
}

wxTreeItemId PWSTreeCtrl::Find(const wxString &path, wxTreeItemId subtree) const
{
  wxArrayString elems(::wxStringTokenize(path, wxT('.')));
  for( size_t idx = 0; idx < elems.Count(); ++idx) {
    wxTreeItemId next;
    if (ExistsInTree(subtree, tostringx(elems[idx]), next))
      subtree = next;
    else
      return wxTreeItemId();
  }
  return subtree;
}

bool PWSTreeCtrl::Remove(const CUUID &uuid)
{
  wxTreeItemId id = Find(uuid);
  if (id.IsOk()) {
    m_item_map.erase(uuid);
    // if item's the only leaf of  group, delete parent
    // group as well. repeat up the tree...
    wxTreeItemId parentId = GetItemParent(id);
    Delete(id);
    while (parentId != GetRootItem()) {
      wxTreeItemId grandparentId = GetItemParent(parentId);
      if (GetChildrenCount(parentId) == 0) {
        Delete(parentId);
        parentId = grandparentId;
      } else
        break;
    } // while
    Refresh();
    Update();
    return true;
  } else {
    return false;
  }
}

void PWSTreeCtrl::SetItemImage(const wxTreeItemId &node,
                               const CItemData &item)
{
  // XXX TBD: modify to display warning and expired states
  int i = NORMAL_II;
  switch (item.GetEntryType()) {
  case CItemData::ET_NORMAL:       i = NORMAL_II;   break;
  case CItemData::ET_ALIASBASE:    i = ABASE_II;    break;
  case CItemData::ET_ALIAS:        i = ALIAS_II;    break;
  case CItemData::ET_SHORTCUTBASE: i = SBASE_II;    break;
  case CItemData::ET_SHORTCUT:     i = SHORTCUT_II; break;
  case CItemData::ET_INVALID:      ASSERT(0); break;
  default: ASSERT(0);
  }
  wxTreeCtrl::SetItemImage(node, i);
}

/*!
 * wxEVT_COMMAND_TREE_ITEM_ACTIVATED event handler for ID_TREECTRL
 */

void PWSTreeCtrl::OnTreectrlItemActivated( wxTreeEvent& evt )
{
  const wxTreeItemId item = evt.GetItem();
  if (ItemIsGroup(item) && GetChildrenCount(item) > 0){
    if (IsExpanded(item))
      Collapse(item);
    else {
      Expand(item);
      //scroll the last child of this node into visibility
      EnsureVisible(GetLastChild(item));
      //but if that scrolled the parent out of the view, bring it back
      EnsureVisible(item);
    }
  }
  else {
    CItemData *ci = GetItem(item);
    if (ci != nullptr)
      dynamic_cast<PasswordSafeFrame *>(GetParent())->
        DispatchDblClickAction(*ci);
  }
}

/*!
 * wxEVT_TREE_ITEM_MENU event handler for ID_TREECTRL
 */

void PWSTreeCtrl::OnContextMenu( wxTreeEvent& evt )
{
  dynamic_cast<PasswordSafeFrame*>(GetParent())->OnContextMenu(GetItem(evt.GetItem()));
}

void PWSTreeCtrl::SelectItem(const CUUID & uuid)
{
  uuid_array_t uuid_array;
  uuid.GetARep(uuid_array);
  wxTreeItemId id = Find(uuid_array);
  if (id.IsOk())
      wxTreeCtrl::SelectItem(id);
}

void PWSTreeCtrl::OnGetToolTip( wxTreeEvent& evt )
{ // Added manually
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesAsTooltipsInViews)) {
    wxTreeItemId id = evt.GetItem();
    const CItemData *ci = GetItem(id);
    if (ci != nullptr) {
      const wxString note = ci->GetNotes().c_str();
      evt.SetToolTip(note);
    }
  }
}

void PWSTreeCtrl::OnDBGUIPrefsChange(wxEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  auto *pwsframe = dynamic_cast<PasswordSafeFrame *>(GetParent());
  wxASSERT(pwsframe != nullptr);
  if (pwsframe->IsTreeView())
    pwsframe->RefreshViews();
}

void EditTreeLabel(wxTreeCtrl* tree, const wxTreeItemId& id)
{
  if (!id) return;
  wxTextCtrl* edit = tree->EditLabel(id);
  if (edit) {
    wxTextValidator val(wxFILTER_EXCLUDE_CHAR_LIST);
    const wxChar* dot = wxT(".");
    val.SetExcludes(wxArrayString(1, &dot));
    edit->SetValidator(val);
    edit->SelectAll();
  }
}
void PWSTreeCtrl::OnAddGroup(wxCommandEvent& /*evt*/)
{
  wxCHECK_RET(IsShown(), wxT("Group can only be added while in tree view"));
  wxTreeItemId parentId = GetSelection();
  wxString newItemPath = (!parentId || parentId == GetRootItem() || !ItemIsGroup(parentId))? wxString(_("New Group")): GetItemGroup(parentId) + wxT(".") + _("New Group");
  wxTreeItemId newItem = AddGroup(tostringx(newItemPath));
  wxCHECK_RET(newItem.IsOk(), _("Could not add empty group item to tree"));
  // mark it as a new group that is still under construction.  wxWidgets would delete it
  SetItemData(newItem, new PWTreeItemData(true));
  ::wxYield();
  EnsureVisible(newItem);
  ::wxYield();
  EditTreeLabel(this, newItem);
}

void PWSTreeCtrl::OnRenameGroup(wxCommandEvent& /* evt */)
{
  wxTreeItemId sel = GetSelection();
  if (sel.IsOk()) {
    wxCHECK_RET(ItemIsGroup(sel), _("Renaming of non-Group items is not implemented"));
    SetItemData(sel, new PWTreeItemData(GetItemGroup(sel)));
    EditTreeLabel(this, sel);
  }
}

void PWSTreeCtrl::OnEndLabelEdit( wxTreeEvent& evt )
{
  const wxString &label =evt.GetLabel();

  if (label.empty()) {
    // empty entry or group names are a non-no...
    evt.Veto();
    return;
  }

  switch (evt.GetId()) {
    case ID_TREECTRL:
    {
      if (label.Find(wxT('.')) == wxNOT_FOUND) {
      // Not safe to modify the tree ctrl in any way.  Wait for the stack to unwind.
      wxTreeEvent newEvt(evt);
      newEvt.SetId(ID_TREECTRL_1);
      AddPendingEvent(newEvt);
      }
      else {
        evt.Veto();
        wxMessageBox(_("Dots are not allowed in group names"), _("Invalid Character"), wxOK|wxICON_ERROR);
      }
      break;
    }
    case ID_TREECTRL_1:
    {
      wxTreeItemId groupItem = evt.GetItem();
      if (groupItem.IsOk()) {
        auto *data = dynamic_cast<PWTreeItemData *>(GetItemData(groupItem));
        if (data && data->BeingAdded()) {
          // A new group being added
          FinishAddingGroup(evt, groupItem);
        }
        else if (data && data->BeingEdited()) {
          // An existing group being renamed
          FinishRenamingGroup(evt, groupItem, data->GetOldPath());
        }
      }
      break;
    }
    default:
      wxFAIL_MSG(wxString::Format(wxT("End Label Edit handler received an unexpected identifier: %d"), evt.GetId()));
      break;
  }
}

void PWSTreeCtrl::OnKeyDown(wxTreeEvent& evt)
{
  if (evt.GetKeyCode() == WXK_LEFT) {
    
    wxTreeItemId item = GetSelection();
    
    if (item.IsOk() && ItemIsGroup(item) && IsExpanded(item)) {
      Collapse(item);
      return;
    }
  }

  evt.Skip();
}

void PWSTreeCtrl::FinishAddingGroup(wxTreeEvent& evt, wxTreeItemId groupItem)
{
  if (evt.IsEditCancelled()) {
    // New item, not yet in db.  So we could just remove from the tree
    Delete(groupItem);
  }
  else {
    // Can't call GetItemGroup here since the item doesn't actually have the new label
    wxString groupName = evt.GetLabel();
    for (wxTreeItemId parent = GetItemParent(groupItem);
                      parent != GetRootItem(); parent = GetItemParent(parent)) {
      groupName = GetItemText(parent) + wxT(".") + groupName;
    }
    StringX sxGroup = tostringx(groupName);

    // The new item we just added above will get removed once the update GUI callback from core happens.
    DBEmptyGroupsCommand* cmd = DBEmptyGroupsCommand::Create(&m_core,
                                                             sxGroup,
                                                             DBEmptyGroupsCommand::EG_ADD);
    if (cmd)
      m_core.Execute(cmd);

    // evt.GetItem() is not valid anymore.  A new item has been inserted instead.
    // We can select it using the full path we computed earlier
    wxTreeItemId newItem = Find(groupName, GetRootItem());
    if (newItem.IsOk())
      wxTreeCtrl::SelectItem(newItem);
  }
}

void PWSTreeCtrl::FinishRenamingGroup(wxTreeEvent& evt, wxTreeItemId groupItem, const wxString& oldPath)
{
  wxCHECK_RET(ItemIsGroup(groupItem), wxT("Cannot handle renaming of non-group items"));

  if (evt.IsEditCancelled())
    return;

  // We DON'T need to handle these two as they can only occur while moving items
  //    not removing groups as they become empty
  //    renaming of groups that have only other groups as children

  MultiCommands* pmcmd = MultiCommands::Create(&m_core);
  if (!pmcmd)
    return;

  // For some reason, Command objects can't handle const references
  StringX sxOldPath = tostringx(oldPath);
  StringX sxNewPath = tostringx(GetItemGroup(groupItem));

  // This takes care of modifying all the actual items
  pmcmd->Add(RenameGroupCommand::Create(&m_core, sxOldPath, sxNewPath));

  // But we have to do the empty groups ourselves because EG_RENAME is not recursive
  typedef std::vector<StringX> EmptyGroupsArray;
  const EmptyGroupsArray& emptyGroups = m_core.GetEmptyGroups();
  StringX sxOldPathWithDot = sxOldPath + _T('.');
  for( EmptyGroupsArray::const_iterator itr = emptyGroups.begin(); itr != emptyGroups.end(); ++itr)
  {
    if (*itr == sxOldPath || itr->find(sxOldPathWithDot) == 0) {
      StringX sxOld = *itr;
      StringX sxNew = sxNewPath + itr->substr(sxOldPath.size());
      pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxOld, sxNew, DBEmptyGroupsCommand::EG_RENAME));
    }
  }

  if (pmcmd->GetSize())
    m_core.Execute(pmcmd);

  // The old treeItem is gone, since it was renamed.  We need to find the new one to select it
  wxTreeItemId newItem = Find(towxstring(sxNewPath), GetRootItem());
  if (newItem.IsOk())
    wxTreeCtrl::SelectItem(newItem);
}

/*!
 * wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
 */

void PWSTreeCtrl::OnTreectrlSelChanged( wxTreeEvent& evt )
{
  CItemData *pci = GetItem(evt.GetItem());

  dynamic_cast<PasswordSafeFrame *>(GetParent())->UpdateSelChanged(pci);
}

static void ColourChildren(PWSTreeCtrl *tree, wxTreeItemId parent, const wxColour &colour)
{
  wxTreeItemIdValue cookie;
  wxTreeItemId child = tree->GetFirstChild(parent, cookie);

  while (child) {
    tree->SetItemTextColour(child, colour);
    child = tree->GetNextChild(parent, cookie);
    if (child && tree->ItemHasChildren(child))
      ColourChildren(tree, child, colour);
  }
}

void PWSTreeCtrl::SetFilterState(bool state)
{
  const wxColour *colour = state ? wxRED : wxBLACK;
  // iterate over all items, no way to do this en-mass
  wxTreeItemId root = GetRootItem();
  if (root)
    ColourChildren(this, root, *colour);
}

/**
 * Saves the state of all groups that have child items in tree view.
 */
void PWSTreeCtrl::SaveGroupDisplayState()
{
  auto groupstates = GetGroupDisplayState();
  
  if (!groupstates.empty()) {
    m_core.SetDisplayStatus(groupstates);
  }
}

/**
 * Restores the state of each individual group in the tree.
 * If the amount of groups in the tree view differs from the 
 * amount of stored group states nothing will be done.
 */
void PWSTreeCtrl::RestoreGroupDisplayState()
{
  auto currentstates = GetGroupDisplayState();
  auto groupstates   = m_core.GetDisplayStatus();
  
  if (currentstates.size() != groupstates.size()) {
    return;
  }
  
  if (!groupstates.empty()) {
    SetGroupDisplayState(groupstates);
  }
}
 
/**
 * Collects the state of all group related items.
 * 
 * A group can be in state expanded or collapsed. The state for an expanded group item 
 * is represented as <i>true</i>, whereas a collapsed group is reflected via <i>false</i>.
 * 
 * @note The booleans in the vector from first to last index represent the groups as
 *       they appear in the tree from top to bottom.
 * @return a vector of booleans representing the state of each group.
 */
std::vector<bool> PWSTreeCtrl::GetGroupDisplayState()
{
  std::vector<bool> groupstates;
  
  TraverseTree(
    GetRootItem(), 
    [&]
    (wxTreeItemId itemId) -> void { 
      groupstates.push_back(IsExpanded(itemId));
    }
  );
  
  return groupstates;
}

/**
 * Sets the state of each individual group to be visually expanded or collapsed.
 * 
 * A boolean of <i>true</i> in the vector will lead to an expanded group, whereas 
 * a boolean of <i>false</i> will trigger collapsing of a group.
 * 
 * @note The booleans in the vector from first to last index represent the groups as
 *       they appear in the tree from top to bottom.
 * @param groupstates a vector of booleans representing the state of each group.
 */
void PWSTreeCtrl::SetGroupDisplayState(const std::vector<bool> &groupstates)
{
  int groupIndex = 0;
  
  TraverseTree(
    GetRootItem(), 
    [&]
    (wxTreeItemId itemId) -> void { 
      if (groupIndex < groupstates.size())
        groupstates[groupIndex++] ? Expand(itemId) : Collapse(itemId);
    }
  );
}

/**
 * Sets the state for each individual group item to be visually expanded.
 */
void PWSTreeCtrl::SetGroupDisplayStateAllExpanded()
{
  auto groupstates = GetGroupDisplayState();
  
  for (auto &&state : groupstates) {
    state = true;
  }
  
  SetGroupDisplayState(groupstates);
}

/**
 * Sets the state for each individual group item to be visually collapsed.
 */
void PWSTreeCtrl::SetGroupDisplayStateAllCollapsed()
{
  auto groupstates = GetGroupDisplayState();
  
  for (auto &&state : groupstates) {
    state = false;
  }
  
  SetGroupDisplayState(groupstates);
}

template<typename GroupItemConsumer>
void PWSTreeCtrl::TraverseTree(wxTreeItemId itemId, GroupItemConsumer&& consumer)
{
  wxTreeItemIdValue cookie;
  
  if (itemId.IsOk()) {
    
    if (ItemHasChildren(itemId)) {
      
      // The root item is not one of the visible tree items.
      // It is neither an group item nor an password related item.
      // Hence, the root item shouldn't be processed, but the 
      // traversal throught the tree should continue.
      if (itemId.GetID() != GetRootItem().GetID()) {
        consumer(itemId);
      }
      
      TraverseTree(GetFirstChild(itemId, cookie), consumer);
    }
    
    TraverseTree(GetNextSibling(itemId), consumer);
  }
}
