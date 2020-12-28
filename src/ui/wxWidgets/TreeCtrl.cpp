/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file TreeCtrl.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin includes
#include <wx/imaglist.h>
#include <wx/tokenzr.h>
////@end includes

#include "core/PWSprefs.h"
#include "core/Command.h"

#include "PasswordSafeFrame.h"
#include "PWSafeApp.h"
#include "TreeCtrl.h"

#include <utility> // for make_pair
#include <vector>

////@begin XPM images
////@end XPM images
#include "graphics/abase_exp.xpm"
#include "graphics/abase_warn.xpm"
#include "graphics/abase.xpm"
#include "graphics/alias.xpm"
#include "graphics/node.xpm"
#include "graphics/normal_exp.xpm"
#include "graphics/normal_warn.xpm"
#include "graphics/normal.xpm"
#include "graphics/sbase_exp.xpm"
#include "graphics/sbase_warn.xpm"
#include "graphics/sbase.xpm"
#include "graphics/shortcut.xpm"
#if wxVERSION_NUMBER >= 3103
// Same in dark
#include "graphics/abase_exp_dark.xpm"
#include "graphics/abase_warn_dark.xpm"
#include "graphics/abase_dark.xpm"
#include "graphics/alias_dark.xpm"
#include "graphics/node_dark.xpm"
#include "graphics/normal_exp_dark.xpm"
#include "graphics/normal_warn_dark.xpm"
#include "graphics/normal_dark.xpm"
#include "graphics/sbase_exp_dark.xpm"
#include "graphics/sbase_warn_dark.xpm"
#include "graphics/sbase_dark.xpm"
#include "graphics/shortcut_dark.xpm"
#endif

using pws_os::CUUID;

/*!
 * TreeCtrl type definition
 */

IMPLEMENT_CLASS( TreeCtrl, wxTreeCtrl )

// Image Indices - these match the order images are added
//                 in TreeCtrl::CreateControls()
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
 * TreeCtrl event table definition
 */

BEGIN_EVENT_TABLE( TreeCtrl, wxTreeCtrl )

////@begin TreeCtrl event table entries
  EVT_TREE_SEL_CHANGED( ID_TREECTRL, TreeCtrl::OnTreectrlSelChanged )
  EVT_TREE_ITEM_ACTIVATED( ID_TREECTRL, TreeCtrl::OnTreectrlItemActivated )
  EVT_TREE_ITEM_MENU( ID_TREECTRL, TreeCtrl::OnContextMenu )
  EVT_TREE_ITEM_GETTOOLTIP( ID_TREECTRL, TreeCtrl::OnGetToolTip )
  EVT_MENU( ID_ADDGROUP, TreeCtrl::OnAddGroup )
  EVT_MENU( ID_RENAME, TreeCtrl::OnRenameGroup )
  EVT_TREE_END_LABEL_EDIT( ID_TREECTRL, TreeCtrl::OnEndLabelEdit )
  EVT_TREE_END_LABEL_EDIT( ID_TREECTRL_1, TreeCtrl::OnEndLabelEdit )
  EVT_TREE_KEY_DOWN( ID_TREECTRL, TreeCtrl::OnKeyDown )
  EVT_TREE_BEGIN_DRAG(ID_TREECTRL, TreeCtrl::OnBeginDrag )
  EVT_TREE_END_DRAG(ID_TREECTRL, TreeCtrl::OnEndDrag )
  EVT_MOTION( TreeCtrl::OnMouseMove )
  /*
    In Linux environments context menus appear on Right-Down mouse click.
    Which mouse click type (Right-Down/Right-Up) is the right one on a platform
    is considered by 'EVT_CONTEXT_MENU(TreeCtrl::OnContextMenu)' which doesn't
    work for wxTreeCtrl prior wx version 3.1.1. See also the following URL.
    https://github.com/wxWidgets/wxWidgets/commit/caea08e6b2b9e4843e84c61abc879880a08634b0
  */
#if wxCHECK_VERSION(3, 1, 1)
  EVT_CONTEXT_MENU(TreeCtrl::OnContextMenu)
#else
#ifdef __WINDOWS__
  EVT_RIGHT_UP(TreeCtrl::OnMouseRightClick)
#else
  EVT_RIGHT_DOWN(TreeCtrl::OnMouseRightClick)
#endif
#endif // wxCHECK_VERSION(3, 1, 1)

  EVT_LEFT_DOWN(TreeCtrl::OnMouseLeftClick)
////@end TreeCtrl event table entries
END_EVENT_TABLE()

const wchar_t GROUP_SEP = L'.';
#define GROUP_SEL_STR wxT(".")

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
 * TreeCtrl constructors
 */

TreeCtrl::TreeCtrl(PWScore &core) : m_core(core)
{
  Init();
}

TreeCtrl::TreeCtrl(wxWindow* parent, PWScore &core,
                         wxWindowID id, const wxPoint& pos,
                         const wxSize& size, long style) : m_core(core)
{
  Init();
  Create(parent, id, pos, size, style);
}

/*!
 * TreeCtrl creator
 */

bool TreeCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin TreeCtrl creation
  m_style = style;
  wxTreeCtrl::Create(parent, id, pos, size, style);
  CreateControls();
////@end TreeCtrl creation
  return true;
}

/*!
 * TreeCtrl destructor
 */

TreeCtrl::~TreeCtrl()
{
////@begin TreeCtrl destruction
if(m_drag_image) wxDELETE(m_drag_image);
////@end TreeCtrl destruction
}

/*!
 * Member initialisation
 */

void TreeCtrl::Init()
{
////@begin TreeCtrl member initialisation
  m_sort = TreeSortType::GROUP;
  m_show_group = false;
  m_drag_item = nullptr;
  m_style = 0L;
  m_lower_scroll_limit = m_upper_scroll_limit = 0;
  m_scroll_timer.setOwner(this);
  m_collapse_timer.setOwner(this);
  m_drag_image = nullptr;
  m_had_been_out = false;
////@end TreeCtrl member initialisation
}

/*!
 * Control creation for TreeCtrl
 */

void TreeCtrl::CreateControls()
{
////@begin TreeCtrl content construction
////@end TreeCtrl content construction
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
#if wxVERSION_NUMBER >= 3103
  const char **xpmDarkList[] = {
    abase_exp_dark_xpm,    // 1
    abase_warn_dark_xpm,   // 2
    abase_dark_xpm,        // 3
    alias_dark_xpm,        // 4
    node_dark_xpm,         // 5
    normal_exp_dark_xpm,   // 6
    normal_warn_dark_xpm,  // 7
    normal_dark_xpm,       // 8
    sbase_exp_dark_xpm,    // 9
    sbase_warn_dark_xpm,   // 10
    sbase_dark_xpm,        // 11
    shortcut_dark_xpm,     // 12
  };
#endif
  const int Nimages = sizeof(xpmList)/sizeof(xpmList[0]);
#if wxVERSION_NUMBER >= 3103
  const bool bIsDark = wxSystemSettings::GetAppearance().IsUsingDarkBackground();
  wxASSERT(Nimages == (sizeof(xpmDarkList)/sizeof(xpmDarkList[0])));
#endif
  
  auto *iList = new wxImageList(13, 13, true, Nimages);
  for (int i = 0; i < Nimages; i++) {
#if wxVERSION_NUMBER >= 3103
    iList->Add(wxBitmap(bIsDark ? xpmDarkList[i] : xpmList[i]));
#else
    iList->Add(wxBitmap(xpmList[i]));
#endif
  }
  AssignImageList(iList);
}

/**
 * Implements Observer::UpdateGUI(UpdateGUICommand::GUI_Action, const pws_os::CUUID&, CItemData::FieldType)
 */
void TreeCtrl::UpdateGUI(UpdateGUICommand::GUI_Action ga, const pws_os::CUUID &entry_uuid, CItemData::FieldType ft)
{
  CItemData *item = nullptr;

  ItemListIter itemIterator = m_core.Find(entry_uuid);

  if (itemIterator != m_core.GetEntryEndIter()) {
    item = &itemIterator->second;
  }
  else if (ga == UpdateGUICommand::GUI_ADD_ENTRY ||
           ga == UpdateGUICommand::GUI_REFRESH_ENTRYFIELD ||
           ga == UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD) {
    pws_os::Trace(wxT("TreeCtrl - Couldn't find uuid %ls"), StringX(CUUID(entry_uuid)).c_str());
    return;
  }

  switch (ga) {
    case UpdateGUICommand::GUI_UPDATE_STATUSBAR:
      // Handled by PasswordSafeFrame
      break;
    case UpdateGUICommand::GUI_ADD_ENTRY:
      ASSERT(item != nullptr);
      AddItem(*item);
      break;
    case UpdateGUICommand::GUI_DELETE_ENTRY:
      Remove(entry_uuid);
      AddRootItem();
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRYFIELD:
    case UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD:
      ASSERT(item != nullptr);
      UpdateItemField(*item, ft);
      break;
    case UpdateGUICommand::GUI_REDO_IMPORT:
    case UpdateGUICommand::GUI_UNDO_IMPORT:
    case UpdateGUICommand::GUI_REDO_MERGESYNC:
    case UpdateGUICommand::GUI_UNDO_MERGESYNC:
      // Handled by PasswordSafeFrame
      break;
    case UpdateGUICommand::GUI_REFRESH_TREE:
      // Handled by PasswordSafeFrame
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRY:
      ASSERT(item != nullptr);
      UpdateItem(*item);
      break;
    case UpdateGUICommand::GUI_REFRESH_GROUPS:
    case UpdateGUICommand::GUI_REFRESH_BOTHVIEWS:
      // TODO: ???
      break;
    case UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED:
      // Handled also by PasswordSafeFrame
      PreferencesChanged();
      break;
    case UpdateGUICommand::GUI_PWH_CHANGED_IN_DB:
      // TODO: ???
      break;
    default:
      wxFAIL_MSG(wxT("TreeCtrl - Unsupported GUI action received."));
      break;
  }
}

/**
 * Implements Observer::GUIRefreshEntry(const CItemData&, bool)
 */
void TreeCtrl::GUIRefreshEntry(const CItemData &item, bool WXUNUSED(bAllowFail))
{
  if (item.GetStatus() == CItemData::ES_DELETED) {
    uuid_array_t uuid;
    item.GetUUID(uuid);
    Remove(uuid);
  }
  else {
    UpdateItem(item);
  }
}

/**
 * Provides the information wether a group tree item is selected.
 * It considers the root item as no group via 'ItemIsGroup'.
 */
bool TreeCtrl::IsGroupSelected() const
{
  return GetSelection().IsOk() && ItemIsGroup(GetSelection());
}

/**
 * Provides the indication whether any editable items exists in
 * the tree view. Editable items are items from the database,
 * hence the tree's root item is excluded.
 */
bool TreeCtrl::HasItems() const
{
  return (GetCount() > 0);
}

/**
 * Provides the indication whether any in the tree visible item
 * is selected. This excludes the tree's root item.
 */
bool TreeCtrl::HasSelection() const
{
  return GetSelection().IsOk() && (GetSelection() != GetRootItem());
}

/**
 * Provides the information whether a tree item is a group.
 * The tree's root item, as a not editable element, is considered
 * as no group.
 */
bool TreeCtrl::ItemIsGroup(const wxTreeItemId& item) const
{
  int image = GetItemImage(item);
  return image == NODE_II && GetRootItem() != item;
}

/**
 * Provides the information whether a tree item is a group or root.
 */
bool TreeCtrl::ItemIsGroupOrRoot(const wxTreeItemId& item) const
{
  int image = GetItemImage(item);
  return image == NODE_II || GetRootItem() == item;
}

// XXX taken from Windows TreeCtrl.cpp
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

bool TreeCtrl::ExistsInTree(wxTreeItemId node,
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

wxTreeItemId TreeCtrl::AddGroup(const StringX &group)
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

wxString TreeCtrl::ItemDisplayString(const CItemData &item) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  const wxString title = item.GetTitle().c_str();

  // Title is a mandatory field - no need to worry if empty
  wxString disp = title;
  
  if (IsShowGroup()) {
    const wxString group = item.GetGroup().c_str();
    disp += wxT(" <") + group + wxT(">");
  }

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

wxString TreeCtrl::GetPath(const wxTreeItemId &node) const
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
      retval += GROUP_SEL_STR;
  }
  return retval;
}

wxString TreeCtrl::GetItemGroup(const wxTreeItemId& item) const
{
  if (!item.IsOk() || item == GetRootItem())
    return wxEmptyString;
  else if (ItemIsGroup(item)) {
    const wxString path = GetPath(item);
    const wxString name = GetItemText(item);
    if (path.IsEmpty())//parent is root
      return name; //group under root
    else
      return path + GROUP_SEL_STR + name; //sub-group of some (non-root) group
  }
  else
    return GetPath(item);
}

void TreeCtrl::UpdateItem(const CItemData &item)
{
  const wxTreeItemId node = Find(item);
  if (node.IsOk()) {
    const wxString oldGroup = GetPath(node);
    const wxString newGroup = GroupNameOfItem(item).c_str();
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

/**
 * Just update the item's text, don't move it into its sorted position.
 */
void TreeCtrl::UpdateItemField(const CItemData &item, CItemData::FieldType ft)
{
  PWSprefs* prefs = PWSprefs::GetInstance();
  if (ft == CItemData::GROUP || (IsSortingName() && (ft == CItemData::TITLE || ft == CItemData::NAME)) || (IsSortingDate() && (ft == CItemData::CTIME || ft == CItemData::PMTIME || ft == CItemData::ATIME || ft == CItemData::XTIME || ft == CItemData::RMTIME))) {
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

StringX TreeCtrl::GroupNameOfItem(const CItemData &item)
{
  StringX group, title;
  time_t gt, t;
  
  if (IsSortingGroup()) {
    group = item.GetGroup();
  } else if (IsSortingName()) {
    title = item.GetTitle();
    if (title.empty()) {
      title = item.GetName();
    }
    if (! title.empty()) {
      group = title.substr(0, 1);
    } else {
      group = L"?";
    }
    ToUpper(group);
    if(! iswalnum(group[0]) && (group[0] <= 127)) {
      group = L"#";
    }
  } else if (IsSortingDate()) {
    if(item.GetCTime(t)) {
      gt = t;
    }
    else {
      gt = 0;
    }
    if(item.GetRMTime(t) && (t > gt)) {
      gt = t;
    }
    if(item.GetPMTime(t) && (t > gt)) {
      gt = t;
    }
    if(item.GetATime(t) && (t > gt)) {
      gt = t;
    }
    
    if (gt == 0) {
      group = L"Undefind Time Value";
    }
    else {
      TCHAR datetime_str[80];
      struct tm *st;
      struct tm st_s;
      errno_t err;
      err = localtime_s(&st_s, &gt);
      if (err) {
        group = L"Conversion Error on Time Value";
      }
      else {
        st = &st_s;
        _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
                  _T("%Y.%m.%d"), st);
        group = datetime_str;
      }
    }
  } else {
    wxASSERT(false);
    group = L"Error";
  }
  
  return group;
}

void TreeCtrl::AddItem(const CItemData &item)
{
  wxTreeItemData *data = new PWTreeItemData(item);
  wxTreeItemId gnode = AddGroup(GroupNameOfItem(item));
  const wxString disp = ItemDisplayString(item);
  wxTreeItemId titem = AppendItem(gnode, disp, -1, -1, data);
  SetItemImage(titem, item);
  SortChildrenRecursively(gnode);
  uuid_array_t uuid;
  item.GetUUID(uuid);
  m_item_map.insert(std::make_pair(CUUID(uuid), titem));
}

/**
 * Adds the root element to the tree if there is none.
 *
 * @note At least one tree element is needed to make the context menu work,
 * otherwise it won't show. TreeCtrl::AddGroup checks if a root item is
 * needed or an existing one is available.
 */
void TreeCtrl::AddRootItem()
{
  if (IsEmpty()) {
    AddRoot("..."); // For drag and drop use a string
    wxTreeCtrl::SetItemImage(GetRootItem(), NODE_II);
  }
}

void TreeCtrl::Clear()
{
  DeleteAllItems();
  AddRootItem();
  m_item_map.clear();
}

CItemData *TreeCtrl::GetItem(const wxTreeItemId &id) const
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

int TreeCtrl::OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
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

void TreeCtrl::SortChildrenRecursively(const wxTreeItemId& item)
{
  if (!ItemIsGroupOrRoot(item) || GetChildrenCount(item) <= 0)
    return;

  SortChildren(item);

  wxTreeItemIdValue cookie;
  for( wxTreeItemId childId = GetFirstChild(item, cookie); childId.IsOk(); childId = GetNextChild(item, cookie)) {
    if (ItemIsGroup(childId) && GetChildrenCount(childId) > 0) { //logically redundant, but probably more efficient
      SortChildrenRecursively(childId);
    }
  }
}

wxTreeItemId TreeCtrl::Find(const CUUID &uuid) const
{
  wxTreeItemId fail;
  auto iter = m_item_map.find(uuid);
  if (iter != m_item_map.end())
    return iter->second;
  else
    return fail;
}

wxTreeItemId TreeCtrl::Find(const CItemData &item) const
{
  uuid_array_t uuid;
  item.GetUUID(uuid);
  return Find(uuid);
}

wxTreeItemId TreeCtrl::Find(const wxString &path, wxTreeItemId subtree) const
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

bool TreeCtrl::Remove(const CUUID &uuid)
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

void TreeCtrl::SetItemImage(const wxTreeItemId &node,
                               const CItemData &item)
{
  // TODO: modify to display warning and expired states
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

void TreeCtrl::OnTreectrlItemActivated( wxTreeEvent& evt )
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

void TreeCtrl::SelectItem(const CUUID & uuid)
{
  uuid_array_t uuid_array;
  uuid.GetARep(uuid_array);
  wxTreeItemId id = Find(uuid_array);
  if (id.IsOk()) {
    wxTreeItemId parent = GetItemParent(id);
    if(parent.IsOk() && (parent != GetRootItem()) && ! IsExpanded(parent))
      Expand(parent);
    ::wxYield();
    EnsureVisible(id);
    ::wxYield();
    
    wxTreeCtrl::SelectItem(id);
  }
}

void TreeCtrl::OnGetToolTip( wxTreeEvent& evt )
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

void TreeCtrl::PreferencesChanged()
{
  ;
}

void EditTreeLabel(wxTreeCtrl* tree, const wxTreeItemId& id)
{
  if (!id) return;
  wxTextCtrl* edit = tree->EditLabel(id);
  if (edit) {
    wxTextValidator val(wxFILTER_EXCLUDE_CHAR_LIST);
    const wxChar* dot = GROUP_SEL_STR;
    val.SetExcludes(wxArrayString(1, &dot));
    edit->SetValidator(val);
    edit->SelectAll();
  }
}

void TreeCtrl::OnAddGroup(wxCommandEvent& WXUNUSED(evt))
{
  wxCHECK_RET(IsShown(), wxT("Group can only be added while in tree view"));
  wxTreeItemId parentId = GetSelection();
  wxString newItemPath = (!parentId || !parentId.IsOk() || parentId == GetRootItem() || !ItemIsGroup(parentId))? wxString(_("New Group")): GetItemGroup(parentId) + GROUP_SEL_STR + _("New Group");
  if(Find(newItemPath, GetRootItem())) {
    wxMessageBox(_("\"") + _("New Group") + _("\" ") + _("name exists"), _("Double group name"), wxOK|wxICON_ERROR);
    return;
  }
  wxTreeItemId newItem = AddGroup(tostringx(newItemPath));
  wxCHECK_RET(newItem.IsOk(), _("Could not add empty group item to tree"));
  // mark it as a new group that is still under construction.  wxWidgets would delete it
  SetItemData(newItem, new PWTreeItemData(true));
  ::wxYield();
  EnsureVisible(newItem);
  ::wxYield();
  EditTreeLabel(this, newItem);
}

void TreeCtrl::OnRenameGroup(wxCommandEvent& WXUNUSED(evt))
{
  wxTreeItemId sel = GetSelection();
  if (sel.IsOk()) {
    wxCHECK_RET(ItemIsGroup(sel), _("Renaming of non-Group items is not implemented"));
    SetItemData(sel, new PWTreeItemData(GetItemGroup(sel)));
    EditTreeLabel(this, sel);
  }
}

void TreeCtrl::OnEndLabelEdit( wxTreeEvent& evt )
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
        wxTreeItemId item = evt.GetItem();
        if(item.IsOk() && ItemIsGroup(item)) {
          wxTreeItemIdValue cookie;
          wxTreeItemId ti = GetFirstChild(GetItemParent(item), cookie);

          while (ti) {
            const wxString itemText = GetItemText(ti);
            if ((itemText == label.c_str()) && (ti != item)) {
              evt.Veto();
              wxMessageBox(_("Same group name exists"), _("Double group name"), wxOK|wxICON_ERROR);
              EditTreeLabel(this, item);
              return;
            }
            ti = GetNextSibling(ti);
          }
        }
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

void TreeCtrl::OnKeyDown(wxTreeEvent& evt)
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

TreeScrollTimer::TreeScrollTimer()
{
  m_owner = NULL;
}

void TreeScrollTimer::Notify()
{
  wxASSERT(m_owner);
  m_owner->CheckScrollList();
}

TreeCollapseTimer::TreeCollapseTimer()
{
    m_owner = NULL;
}

void TreeCollapseTimer::Notify()
{
  wxASSERT(m_owner);
  m_owner->CheckCollapseEntry();
}

void TreeCtrl::CheckScrollList()
{
  if(m_isDragging) {
    int width, height;
    wxPoint mousePt = ScreenToClient(wxGetMousePosition());  // Find where the mouse is in relation to the (exited) pane
    
    GetSize(&width, &height);  // Store window dimensions
    
    if((mousePt.x < 0) || (mousePt.x >= width)) { // Not in same area
      resetScrolling();
      return;
    }
    
    int ThumbVert = GetScrollThumb (wxVERTICAL);
    int PosVert = GetScrollPos (wxVERTICAL);
    int RangeVert = GetScrollRange (wxVERTICAL);
    wxEventType commandType = wxEVT_NULL;
    
    if(mousePt.y <= 0) { // We're above the pane
      StopAutoScrolling();
      if(PosVert > 0) {
        commandType = wxEVT_SCROLLWIN_PAGEUP;
      }
    }
    else if(mousePt.y > height) {  // We're below the pane
      StopAutoScrolling();
      if((PosVert + ThumbVert) < RangeVert) {
        commandType = wxEVT_SCROLLWIN_PAGEDOWN;
      }
    }
    else if(mousePt.y <= m_upper_scroll_limit) {
      if(PosVert > 0) {
        commandType = wxEVT_SCROLLWIN_LINEUP;
      }
    }
    else if(mousePt.y >= m_lower_scroll_limit) {
      if((PosVert + ThumbVert) < RangeVert) {
        commandType = wxEVT_SCROLLWIN_LINEDOWN;
      }
    }
    
    if(commandType != wxEVT_NULL) {
      wxScrollWinEvent scrollEvent( commandType, GetId(), 0);
      GetEventHandler()->ProcessEvent(scrollEvent);
      m_scroll_timer.Start();
    }
    else {
      resetScrolling();
    }
  }
}

void TreeCtrl::CheckCollapseEntry()
{
  if(! m_last_mice_item_in_drag_and_drop.IsOk()) return;
  
  int width, height;
  wxPoint mousePt = ScreenToClient(wxGetMousePosition());  // Find where the mouse is in relation to the (exited) pane
  
  GetSize(&width, &height);  // Store window dimensions
  
  if((mousePt.x < 0) || (mousePt.x >= width) || (mousePt.y < 0) || (mousePt.y >= height)) { // Not in same area
    m_last_mice_item_in_drag_and_drop = nullptr;
    return;
  }
  
  int flags = wxTREE_HITTEST_ONITEM;
  wxTreeItemId currentItem = DoTreeHitTest(mousePt, flags);
  bool itemSame = (currentItem == m_last_mice_item_in_drag_and_drop);
  
  if(itemSame) {
    if(currentItem.IsOk() && ItemHasChildren(currentItem) && (currentItem != GetRootItem())) {
      if(IsExpanded(currentItem)) {
        Collapse(currentItem);
        wxTreeItemId selectedItem = GetSelection();
        if(selectedItem.IsOk()) UnselectAll();
      }
      else { // Is Collapsed
        Expand(currentItem);
        //scroll the last child of this node into visibility
        EnsureVisible(GetLastChild(currentItem));
        //but if that scrolled the parent out of the view, bring it back
        EnsureVisible(currentItem);
      }
    }
  }
  else {
    m_last_mice_item_in_drag_and_drop = nullptr;
  }
}

void TreeCtrl::OnMouseMove(wxMouseEvent& event)
{
  if(m_isDragging && m_drag_item.IsOk())
  {
    int width, height;
    wxPoint pt = ScreenToClient(wxGetMousePosition());
  
    GetSize(&width,  &height);
    
    if((pt.x >= 0) && (pt.x < width)) { // When inside of the windows area
      int flags = wxTREE_HITTEST_ONITEM;
      wxTreeItemId currentItem = DoTreeHitTest(pt, flags);
      bool itemChanged = (currentItem != m_last_mice_item_in_drag_and_drop);
      int ThumbVert = GetScrollThumb (wxVERTICAL);
      int PosVert = GetScrollPos (wxVERTICAL);
      int RangeVert = GetScrollRange (wxVERTICAL);
        
      if((PosVert > 0) && (pt.y <= m_upper_scroll_limit)) {
        if(! m_scroll_timer.IsRunning()) {
          m_scroll_timer.Start();
        }
      } else if(((PosVert + ThumbVert) < RangeVert) && (pt.y >= m_lower_scroll_limit)){
        if(! m_scroll_timer.IsRunning()) {
          m_scroll_timer.Start();
        }
      }
      else if(m_scroll_timer.IsRunning()) {
        resetScrolling();
      }
        
      if(currentItem.IsOk()) {
        wxString label = GetItemText(m_drag_item);
        wxTreeItemId si;
        // Set Cursor depending from kind of entry
        if(ItemIsGroupOrRoot(currentItem) && ! IsDescendant(currentItem, m_drag_item) && (::wxGetKeyState(WXK_SHIFT) || ! ExistsInTree(currentItem, tostringx(label), si))) {
          SetCursor(wxNullCursor);
        }
        else {
          SetCursor(wxCURSOR_NO_ENTRY);
        }
        // Check Collapse or Expand when taying for longer time over group entry
        if(m_last_mice_item_in_drag_and_drop == nullptr) {
          if(ItemHasChildren(currentItem)) {
            m_last_mice_item_in_drag_and_drop = currentItem;
            m_collapse_timer.Start();
          }
        } else if(itemChanged) {
          if(m_collapse_timer.IsRunning())
            m_collapse_timer.Stop();
          if(ItemHasChildren(currentItem)) {
            m_last_mice_item_in_drag_and_drop = currentItem;
            m_collapse_timer.Start();
          }
        }
        else { // Still the same, wait for timer expiry
          // Do nothing
        }
      }
      else {
        if(m_last_mice_item_in_drag_and_drop)
          SetCursor(wxNullCursor);
        if(m_collapse_timer.IsRunning())
          m_collapse_timer.Stop();
        m_last_mice_item_in_drag_and_drop = nullptr;
      }
    }
    else {
      if(m_drag_image) {
        m_drag_image->Hide();
      }
      if(m_last_mice_item_in_drag_and_drop || m_scroll_timer.IsRunning()) { // left or right from tree window, reset scrolling
        resetScrolling();
        resetDragItems();
      }
    }
    
    if((pt.x >= 0) && (pt.x < width) && (pt.y >= 0) && (pt.y < height)) {
      StopAutoScrolling();
      if(m_drag_image) {
        m_drag_image->Hide();
        m_drag_image->Move(pt);
        m_drag_image->Show();
      }
      if(m_had_been_out) {
        SetCursor(wxNullCursor);  // TO BE FIXED: This cursor is not visible because macOS means the cursor is not on the current windows area
        m_had_been_out = false;
      }
    }
    else
      m_had_been_out = true;
  }
  
  wxTreeCtrl::OnMouse(event);
}

void TreeCtrl::OnBeginDrag(wxTreeEvent& evt)
{
  if (m_core.IsReadOnly() || !IsSortingGroup()) {
    evt.Veto();
    return;
  }
  
  wxTreeItemId item = GetSelection();

  if (item.IsOk() && (GetRootItem() != item)) {
    m_drag_item = item;
    m_had_been_out = false;
    SetWindowStyle(m_style & ~wxTR_HIDE_ROOT);
    evt.Allow();
    markDragItem(item);
    Refresh();
    resetDragItems(true);
    m_drag_image = new wxDragImage(*this, item);
    if(m_drag_image) {
      wxPoint mousePt = ScreenToClient(wxGetMousePosition());
      wxRect rect;
      GetBoundingRect(item, rect, true);
      if(! m_drag_image->BeginDrag(mousePt - rect.GetLeftTop(), this, true)) {
        pws_os::Trace(L"BeginDrag failed");
        wxDELETE(m_drag_image);
      }
      else {
        ReleaseMouse(); // Must be released because BeginDrag() will Capture Mouse, but the code following event BeginDragging() will do as well
        m_drag_image->Show();
      }
    }
    return;
  }

  evt.Skip();
}

void TreeCtrl::resetScrolling()
{
  StopAutoScrolling();
  if(m_scroll_timer.IsRunning()) m_scroll_timer.Stop();
}

void TreeCtrl::resetDragItems(bool initSize)
{
  if(m_scroll_timer.IsRunning()) m_scroll_timer.Stop();
  if(m_collapse_timer.IsRunning()) m_collapse_timer.Stop();
  m_last_mice_item_in_drag_and_drop = nullptr;
  if(initSize) {
    int width, height;
    GetSize(&width,  &height);
    m_lower_scroll_limit = height - GetCharHeight();
    m_upper_scroll_limit = GetCharHeight();
  }
}

void TreeCtrl::markDragItem(const wxTreeItemId itemSrc, bool markIt)
{
  if(markIt) {
    m_drag_text_colour = GetItemTextColour(itemSrc);
    m_drag_background_colour = GetItemBackgroundColour(itemSrc);
    
    SetItemTextColour(itemSrc, *wxWHITE);
    SetItemBackgroundColour(itemSrc, *wxBLUE);
  }
  else {
    SetItemTextColour(itemSrc, m_drag_text_colour);
    SetItemBackgroundColour(itemSrc, m_drag_background_colour);
  }
}

void TreeCtrl::OnEndDrag(wxTreeEvent& evt)
{
  bool makeCopy = ::wxGetKeyState(WXK_CONTROL);
  bool doOverride = ::wxGetKeyState(WXK_SHIFT);
  int width, height;
  wxPoint mousePt = ScreenToClient(wxGetMousePosition());  // Find where the mouse is in relation to the (exited) pane
  GetSize(&width, &height);  // Store window dimensions
  int flags = wxTREE_HITTEST_ONITEM;
  wxTreeItemId currentItem = DoTreeHitTest(mousePt, flags); // Check current before removing root from screen
  resetDragItems();
  // Restore Hiden Root
  SetWindowStyle(m_style);
  markDragItem(m_drag_item, false);
  if(m_drag_image) {
    m_drag_image->Hide();
    CaptureMouse(); // EndDrag() will release the Mouse, but before call the mouse had been released from event handler
    m_drag_image->EndDrag();
    wxDELETE(m_drag_image);
  }
  Refresh();
  
  wxTreeItemId selectedItem = GetSelection();
  if(selectedItem.IsOk()) UnselectAll();
  
  if((mousePt.x < 0) || (mousePt.x >= width) || (mousePt.y < 0) || (mousePt.y >= height)) { // Not in same area
    evt.Skip();
    m_drag_item = nullptr;
    return;
  }
  
  if (m_drag_item != nullptr) { // Drag and drop started

    wxTreeItemId itemDst = evt.GetItem();
    
    if(itemDst.IsOk() && (currentItem != itemDst)) { // Mice is not over destination item, skip dragging
      evt.Skip();
      m_drag_item = nullptr;
      return;
    }

    if(itemDst && itemDst.IsOk() &&
       (itemDst != m_drag_item) &&
       (itemDst != GetItemParent(m_drag_item)) &&
       ((GetRootItem() != itemDst) || (GetRootItem() != GetItemParent(m_drag_item)))) { // Do not Drag and Drop on its own
      if(GetRootItem() != itemDst) {
        if(! ItemIsGroup(itemDst)) {                   // Only group may be destination
          wxMessageBox(_("Destination is not a group"), _("Drag and Drop failed"), wxOK|wxICON_ERROR);
          evt.Skip();
          m_drag_item = nullptr;
          return;
        }
        else if(IsDescendant(itemDst, m_drag_item)) {  // Do not drag and drop into the moved tree
          wxMessageBox(_("Destination cannot be inside source tree"), _("Drag and Drop failed"), wxOK|wxICON_ERROR);
          evt.Skip();
          m_drag_item = nullptr;
          return;
        }
      }

      if(! ItemIsGroup(m_drag_item)) {
        // It's only one item to handle
        CItemData *dataSrc = TreeCtrl::GetItem(m_drag_item);
        wxASSERT(dataSrc);
        StringX sxNewPath = tostringx((GetRootItem() != itemDst) ? GetItemGroup(itemDst) : "");
        CItemData modifiedItem = CreateNewItemAsCopy(dataSrc, sxNewPath, ! doOverride, makeCopy);

        if(makeCopy) {
          if (dataSrc->IsDependent()) {
            m_core.Execute(
              AddEntryCommand::Create(&m_core, modifiedItem, dataSrc->GetBaseUUID())
            );
          } else { // not alias or shortcut
            m_core.Execute(
              AddEntryCommand::Create(&m_core, modifiedItem)
            );
          }
        }
        else {
          time_t t;
          time(&t);
          modifiedItem.SetRMTime(t);
          
          // Move is same as rename
          m_core.Execute(
            EditEntryCommand::Create(&m_core, *dataSrc, modifiedItem)
          );
        }
        
        SelectItem(modifiedItem.GetUUID());
      } else {
        // For some reason, Command objects can't handle const references
        StringX sxOldPath = tostringx(GetItemGroup(m_drag_item));
        wxString label = GetItemText(m_drag_item);
        StringX sxNewPath;
        
        if(GetRootItem() != itemDst) {
          sxNewPath = tostringx(GetItemGroup(itemDst) + GROUP_SEL_STR + label);
        }
        else {
          sxNewPath = tostringx(label);
        }
        if(! doOverride) {
          wxTreeItemId si;
          if(ExistsInTree(itemDst, tostringx(label), si)) {
            evt.Veto();
            wxMessageBox(_("Duplicate group name (use Shift to overrule)"), _("Double group name"), wxOK|wxICON_ERROR);
            m_drag_item = nullptr;
            return;
          }
        }

        if(makeCopy) {
          // On control key pressed do copy the tree
          CreateCommandCopyGroup(m_drag_item, sxNewPath, sxOldPath, ! doOverride);
        }
        else {
          // Without key board pressed to move, same as rename
          CreateCommandRenamingGroup(sxNewPath, sxOldPath);
        }
        
        wxTreeItemId newItem = Find(towxstring(sxNewPath), GetRootItem());
        if (newItem.IsOk())
          wxTreeCtrl::SelectItem(newItem);
      }
    }
    else {
      if(m_drag_item.IsOk())
        wxTreeCtrl::SelectItem(m_drag_item);
    }
  }

  evt.Skip();
  m_drag_item = nullptr;
}

bool TreeCtrl::IsDescendant(const wxTreeItemId itemDst, const wxTreeItemId itemSrc)
{
  wxTreeItemId itemParent, itemCurrent = itemDst;

  if(itemDst == GetRootItem()) return false;
  if(itemDst == itemSrc) return true;
  
  do {
    itemParent = GetItemParent(itemCurrent);
    if(itemParent == itemSrc) return true;
    itemCurrent = itemParent;
  } while (itemCurrent && (itemCurrent != GetRootItem()));
  
  return false;
}

/*!
 * wxEVT_TREE_ITEM_MENU event handler for ID_TREECTRL
 */

void TreeCtrl::OnContextMenu( wxTreeEvent& evt )
{
  dynamic_cast<PasswordSafeFrame*>(GetParent())->OnContextMenu(GetItem(evt.GetItem()));
}

#if wxCHECK_VERSION(3, 1, 1)
void TreeCtrl::OnContextMenu(wxContextMenuEvent& event)
#else
void TreeCtrl::OnMouseRightClick(wxMouseEvent& event)
#endif // wxCHECK_VERSION(3, 1, 1)
{
  wxPoint mouseClickPosition = event.GetPosition();
  int positionInfo;

#if wxCHECK_VERSION(3, 1, 1)
  HitTest(wxWindow::ScreenToClient(mouseClickPosition), positionInfo);
#else
  HitTest(mouseClickPosition, positionInfo);
#endif // wxCHECK_VERSION(3, 1, 1)

  if ((positionInfo & wxTREE_HITTEST_NOWHERE) == wxTREE_HITTEST_NOWHERE) {
    auto *parentWindow = dynamic_cast<PasswordSafeFrame*>(GetParent());
    wxASSERT(parentWindow != nullptr);
    Unselect();
    parentWindow->OnContextMenu(nullptr);
  }
  else {
    event.Skip();
  }
}

void TreeCtrl::OnMouseLeftClick(wxMouseEvent& event)
{
  wxPoint mouseClickPosition = event.GetPosition();
  int positionInfo;

  HitTest(mouseClickPosition, positionInfo);

  if ((positionInfo & wxTREE_HITTEST_NOWHERE) == wxTREE_HITTEST_NOWHERE) {
    Unselect();
  }
  else {
    event.Skip();
  }
}

void TreeCtrl::FinishAddingGroup(wxTreeEvent& evt, wxTreeItemId groupItem)
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
      groupName = GetItemText(parent) + GROUP_SEL_STR + groupName;
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

void TreeCtrl::FinishRenamingGroup(wxTreeEvent& evt, wxTreeItemId groupItem, const wxString& oldPath)
{
  wxCHECK_RET(ItemIsGroup(groupItem), wxT("Cannot handle renaming of non-group items"));

  if (evt.IsEditCancelled())
    return;

  // For some reason, Command objects can't handle const references
  StringX sxOldPath = tostringx(oldPath);
  StringX sxNewPath = tostringx(GetItemGroup(groupItem));
  
  CreateCommandRenamingGroup(sxNewPath, sxOldPath);

  // The old treeItem is gone, since it was renamed.  We need to find the new one to select it
  wxTreeItemId newItem = Find(towxstring(sxNewPath), GetRootItem());
  if (newItem.IsOk())
    wxTreeCtrl::SelectItem(newItem);
}

void TreeCtrl::CreateCommandRenamingGroup(StringX sxNewPath, StringX sxOldPath)
{
  // We DON'T need to handle these two as they can only occur while moving items
  //    not removing groups as they become empty
  //    renaming of groups that have only other groups as children

  MultiCommands* pmcmd = MultiCommands::Create(&m_core);
  if (!pmcmd)
    return;

  // This takes care of modifying all the actual items
  pmcmd->Add(RenameGroupCommand::Create(&m_core, sxOldPath, sxNewPath));

  // But we have to do the empty groups ourselves because EG_RENAME is not recursive
  typedef std::vector<StringX> EmptyGroupsArray;
  const EmptyGroupsArray& emptyGroups = m_core.GetEmptyGroups();
  StringX sxOldPathWithDot = sxOldPath + _T('.');
  for(const auto & emptyGroup : emptyGroups)
  {
    if (emptyGroup == sxOldPath || emptyGroup.find(sxOldPathWithDot) == 0) {
      StringX sxOld = emptyGroup;
      StringX sxNew = sxNewPath + emptyGroup.substr(sxOldPath.size());
      pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxOld, sxNew, DBEmptyGroupsCommand::EG_RENAME));
    }
  }

  if (pmcmd->GetSize())
    m_core.Execute(pmcmd);
}

CItemData TreeCtrl::CreateNewItemAsCopy(const CItemData *dataSrc, StringX sxNewPath, bool checkName, bool newEntry)
{
  wxASSERT(dataSrc);
  CItemData modifiedItem(*dataSrc);
  
  modifiedItem.SetGroup(sxNewPath);
  if(newEntry)
    modifiedItem.CreateUUID();
    
  if(! checkName) {
    // Do not add " Copy #" to title when shift key is pressed, as with new location the title is unique (hopefully)
    modifiedItem.SetTitle(dataSrc->GetTitle());
  }
  else {
    // Normal copy search for a unique "Title" at destination place
    ItemListConstIter listpos;
    int i = 0;
    wxString s_copy;
    const StringX ci2_user = dataSrc->GetUser();
    const StringX ci2_title0 = dataSrc->GetTitle();
    StringX ci2_title;
    
    listpos = m_core.Find(sxNewPath, ci2_title0, ci2_user);
    if(listpos == m_core.GetEntryEndIter()) {
      ci2_title = ci2_title0;
    }
    else {
      do {
        s_copy.clear();
        i++;
        s_copy << _(" Copy # ") << i;
        ci2_title = ci2_title0 + tostringx(s_copy);
        listpos = m_core.Find(sxNewPath, ci2_title, ci2_user);
      } while (listpos != m_core.GetEntryEndIter());
    }
    modifiedItem.SetTitle(ci2_title);
  }
  modifiedItem.SetUser(dataSrc->GetUser());
  modifiedItem.SetStatus(CItemData::ES_ADDED);
  if (dataSrc->IsDependent()) {
    modifiedItem.SetPassword(dataSrc->GetPassword());
    if (dataSrc->IsAlias()) {
      modifiedItem.SetAlias();
    } else {
      modifiedItem.SetShortcut();
    }
  } else { // not alias or shortcut
    modifiedItem.SetNormal();
  }
  return modifiedItem;
}

void TreeCtrl::ExtendCommandCopyGroup(MultiCommands* pmCmd, wxTreeItemId itemSrc, StringX sxNewPath, bool checkName)
{
  if (!pmCmd)
    return;
  
  wxASSERT(itemSrc != GetRootItem() && ItemIsGroup(itemSrc));

  wxTreeItemIdValue cookie;
  wxTreeItemId ti = GetFirstChild(itemSrc, cookie);
  
  while (ti) {
    const StringX label = tostringx(GetItemText(ti));
    
    if(ItemIsGroup(ti)) {
      ExtendCommandCopyGroup(pmCmd, ti, sxNewPath + GROUP_SEL_STR + label, checkName);
    }
    else {
      CItemData *dataSrc = TreeCtrl::GetItem(ti);
      wxASSERT(dataSrc);
      CItemData modifiedItem = CreateNewItemAsCopy(dataSrc, sxNewPath, checkName, true);
      
      if (dataSrc->IsDependent()) {
        pmCmd->Add(
          AddEntryCommand::Create(&m_core, modifiedItem, dataSrc->GetBaseUUID())
        );
      } else { // not alias or shortcut
        pmCmd->Add(
          AddEntryCommand::Create(&m_core, modifiedItem)
        );
      }
    }
    ti = GetNextSibling(ti);
  }
}

void TreeCtrl::CreateCommandCopyGroup(wxTreeItemId itemSrc, StringX sxNewPath, StringX sxOldPath, bool checkName)
{
  MultiCommands* pmcmd = MultiCommands::Create(&m_core);
  if (!pmcmd)
    return;
  
  // Copy the selected tree with all entries
  wxASSERT(itemSrc != GetRootItem() && ItemIsGroup(itemSrc));
  ExtendCommandCopyGroup(pmcmd, itemSrc, sxNewPath, checkName);
  
  // But we have to do the empty groups ourselves because EG_ADD is not recursive
  typedef std::vector<StringX> EmptyGroupsArray;
  const EmptyGroupsArray& emptyGroups = m_core.GetEmptyGroups();
  StringX sxOldPathWithDot = sxOldPath + _T('.');
  for(const auto & emptyGroup : emptyGroups)
  {
    if (emptyGroup == sxOldPath || emptyGroup.find(sxOldPathWithDot) == 0) {
      StringX sxNew = sxNewPath + emptyGroup.substr(sxOldPath.size());
      if(checkName) {
        wxTreeItemId item = Find(towxstring(sxNew), GetRootItem());
        if(item.IsOk() && ItemIsGroup(item))
          continue; // Name as group already present at destination, skip this one
      }
      pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxNew, DBEmptyGroupsCommand::EG_ADD));
    }
  }
  
  if (pmcmd->GetSize())
    m_core.Execute(pmcmd);
}

/*!
 * wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
 */

void TreeCtrl::OnTreectrlSelChanged( wxTreeEvent& evt )
{
  CItemData *pci = GetItem(evt.GetItem());

  dynamic_cast<PasswordSafeFrame *>(GetParent())->UpdateSelChanged(pci);
}

static void ColourChildren(TreeCtrl *tree, wxTreeItemId parent, const wxColour &colour)
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

void TreeCtrl::SetFilterState(bool state)
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
void TreeCtrl::SaveGroupDisplayState()
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
void TreeCtrl::RestoreGroupDisplayState()
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
std::vector<bool> TreeCtrl::GetGroupDisplayState()
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
void TreeCtrl::SetGroupDisplayState(const std::vector<bool> &groupstates)
{
  size_t groupIndex = 0;

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
void TreeCtrl::SetGroupDisplayStateAllExpanded()
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
void TreeCtrl::SetGroupDisplayStateAllCollapsed()
{
  auto groupstates = GetGroupDisplayState();

  for (auto &&state : groupstates) {
    state = false;
  }

  SetGroupDisplayState(groupstates);
}

template<typename GroupItemConsumer>
void TreeCtrl::TraverseTree(wxTreeItemId itemId, GroupItemConsumer&& consumer)
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
