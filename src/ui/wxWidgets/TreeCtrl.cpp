/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/wfstream.h>
////@end includes

#include "core/core.h"
#include "core/UTF8Conv.h"

#include "core/PWSprefs.h"
#include "core/Command.h"

#include "core/Util.h"

#include "PasswordSafeFrame.h"
#include "PWSafeApp.h"
#include "TreeCtrl.h"
#include "DnDPWSafeObject.h"
#include "DnDSupport.h"
#include "DnDDropTarget.h"

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
#include "graphics/empty_node.xpm"
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
#include "graphics/empty_node_dark.xpm"
#endif

using pws_os::CUUID;
class Command;

/*!
 * TreeCtrl type definition
 */

IMPLEMENT_CLASS( TreeCtrl, wxTreeCtrl )

// Image Indices - these match the order images are added
//                 in TreeCtrl::CreateControls()
// Warning: Do not change ordering on ..._EXP_II, ..._WARN_II and base entry for ABASE, NORMAL and SBASE
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
  EMPTY_NODE_II,   // 12
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
  EVT_TREE_BEGIN_LABEL_EDIT( ID_TREECTRL, TreeCtrl::OnStartLabelEdit )
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

TreeCtrl::TreeCtrl(PWScore &core) : TreeCtrlBase(core),
                                    m_collapse_timer(this, &TreeCtrl::CheckCollapseEntry, TreeCtrlTimer::DELAY_COLLAPSE),
                                    m_scroll_timer(this, &TreeCtrl::CheckScrollList, TreeCtrlTimer::DELAY_SCROLLING)
{
  Init();
}

TreeCtrl::TreeCtrl(wxWindow* parent, PWScore &core,
                         wxWindowID id, const wxPoint& pos,
                         const wxSize& size, long style) : TreeCtrlBase(core),
                                                           m_collapse_timer(this, &TreeCtrl::CheckCollapseEntry, TreeCtrlTimer::DELAY_COLLAPSE),
                                                           m_scroll_timer(this, &TreeCtrl::CheckScrollList, TreeCtrlTimer::DELAY_SCROLLING)
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
#if wxUSE_DRAG_AND_DROP
  if(PWSprefs::GetInstance()->GetPref(PWSprefs::MultipleInstances))
    SetDropTarget(new DndPWSafeDropTarget(this));
#endif
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
void TreeCtrlBase::Init()
{
////@begin TreeCtrl member initialisation
  m_sort = TreeSortType::GROUP;
  m_show_group = false;
}

void TreeCtrl::Init()
{
  TreeCtrlBase::Init();
////@begin TreeCtrl member initialisation
  m_drag_item = nullptr;
  m_style = 0L;
  m_lower_scroll_limit = m_upper_scroll_limit = 0;
  m_drag_image = nullptr;
  m_had_been_out = false;
  m_bFilterActive = false;
  m_last_dnd_item = nullptr;
  m_run_dnd = false;
////@end TreeCtrl member initialisation
}

/*!
 * Control creation for TreeCtrl
 */

void TreeCtrlBase::CreateControls()
{
////@begin TreeCtrl content construction
////@end TreeCtrl content construction
  static const char* const* const xpmList[] = {
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
    empty_node_xpm,   // 12
  };
#if wxVERSION_NUMBER >= 3103
  static const char* const* const xpmDarkList[] = {
    abase_exp_dark_xpm,    // 0
    abase_warn_dark_xpm,   // 1
    abase_dark_xpm,        // 2
    alias_dark_xpm,        // 3
    node_dark_xpm,         // 4
    normal_exp_dark_xpm,   // 5
    normal_warn_dark_xpm,  // 6
    normal_dark_xpm,       // 7
    sbase_exp_dark_xpm,    // 8
    sbase_warn_dark_xpm,   // 9
    sbase_dark_xpm,        // 10
    shortcut_dark_xpm,     // 11
    empty_node_dark_xpm,   // 12
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
      if(HasItems())
        SortChildrenRecursively(GetRootItem());
      break;
    case UpdateGUICommand::GUI_ADD_ENTRY:
      ASSERT(item != nullptr);
      if(!m_bFilterActive) {
        AddItem(*item);
      }
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
bool TreeCtrlBase::IsGroupSelected() const
{
  return GetSelection().IsOk() && ItemIsGroup(GetSelection());
}

/**
 * Provides the indication whether any editable items exists in
 * the tree view. Editable items are items from the database,
 * hence the tree's root item is excluded.
 */
bool TreeCtrlBase::HasItems() const
{
  return (GetCount() > 0);
}

/**
 * Provides the indication whether any in the tree visible item
 * is selected. This excludes the tree's root item.
 */
bool TreeCtrlBase::HasSelection() const
{
  return GetSelection().IsOk() && (GetSelection() != GetRootItem());
}

/**
 * Provides the information whether a tree item is a group.
 * The tree's root item, as a not editable element, is considered
 * as no group.
 */
bool TreeCtrlBase::ItemIsGroup(const wxTreeItemId& item) const
{
  int image = GetItemImage(item);
  return ((image == NODE_II) || (image == EMPTY_NODE_II)) && (GetRootItem() != item);
}

/**
 * Provides the information whether a tree item is a group or root.
 */
bool TreeCtrlBase::ItemIsGroupOrRoot(const wxTreeItemId& item) const
{
  int image = GetItemImage(item);
  return (image == NODE_II) || (image == EMPTY_NODE_II) || (GetRootItem() == item);
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

bool TreeCtrlBase::ExistsInTree(wxTreeItemId node,
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

wxTreeItemId TreeCtrlBase::AddGroup(const StringX &group)
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
        if(GetRootItem() != ti) {
          // Correct icon of parent, if needed
          wxTreeItemId parent = GetItemParent(ti);
          setNodeAsNotEmpty(parent);
        }
      } else
        ti = si;
    } while (!path.empty());
  }
  // A new group will be empty, so set Icon to empty one
  setNodeAsEmptyIfNeeded(ti);
  return ti;
}

wxString TreeCtrlBase::ItemDisplayString(const CItemData &item) const
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

wxString TreeCtrlBase::GetPath(const wxTreeItemId &node) const
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

StringX TreeCtrlBase::GroupNameOfItem(const CItemData &item)
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

void TreeCtrlBase::AddItem(const CItemData &item)
{
  wxTreeItemData *data = new PWTreeItemData(item);
  wxTreeItemId gnode = AddGroup(GroupNameOfItem(item));
  const wxString disp = ItemDisplayString(item);
  wxTreeItemId titem = AppendItem(gnode, disp, -1, -1, data);
  setNodeAsNotEmpty(gnode); // Could be empty before
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
void TreeCtrlBase::AddRootItem()
{
  if (IsEmpty()) {
    AddRoot("..."); // For drag and drop use a string
    wxTreeCtrl::SetItemImage(GetRootItem(), NODE_II);
  }
}

void TreeCtrlBase::Clear()
{
  DeleteAllItems();
  AddRootItem();
  m_item_map.clear();
}

CItemData *TreeCtrlBase::GetItem(const wxTreeItemId &id) const
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

void TreeCtrlBase::SortChildrenRecursively(const wxTreeItemId& item)
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

wxTreeItemId TreeCtrlBase::Find(const CUUID &uuid) const
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
  return TreeCtrlBase::Find(uuid);
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
  wxTreeItemId id = TreeCtrlBase::Find(uuid);
  if (id.IsOk()) {
    m_item_map.erase(uuid);
    // if item's the only leaf of  group, delete parent
    // group as well. repeat up the tree...
    wxTreeItemId parentId = GetItemParent(id);
    Delete(id);
    // Correct Icon of parent if needed
    setNodeAsEmptyIfNeeded(parentId);
    Refresh();
    Update();
    return true;
  } else {
    return false;
  }
}

void TreeCtrlBase::SetItemImage(const wxTreeItemId &node,
                               const CItemData &item)
{
  int offset = 0;
  CItemData::EntryType entrytype = item.GetEntryType();
  
  if((entrytype == CItemData::ET_NORMAL) || (entrytype == CItemData::ET_ALIASBASE) || (entrytype == CItemData::ET_SHORTCUTBASE)) {
    time_t tttXTime(static_cast<time_t>(0));
    item.GetXTime(tttXTime); // Fetch eXpiry time (or period, range from 1 to 3650 day))
    if (tttXTime > static_cast<time_t>(0) && tttXTime <= static_cast<time_t>(3650)) {
      // When period stored, calculate time by taking last password modification time (PM)
      // or, if PM is not present, use Creation time (C).
      time_t tttCPMTime(static_cast<time_t>(0));
      item.GetPMTime(tttCPMTime);
      if (tttCPMTime == static_cast<time_t>(0))
        item.GetCTime(tttCPMTime);
      // Add days of period to last Creation/Password modifiction time (stored in days, 86400 is seconds of day)
      tttXTime = static_cast<time_t>((long)tttCPMTime + (long)tttXTime * 86400L);
    }
    
    if (tttXTime != static_cast<time_t>(0)) {
      time_t now, warnexptime(static_cast<time_t>(0));
      time(&now);
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarn)) {
        int idays = PWSprefs::GetInstance()->GetPref(PWSprefs::PreExpiryWarnDays);
        struct tm st;
        errno_t err;
        err = localtime_s(&st, &now);  // secure version
        ASSERT(err == 0);
        st.tm_mday += idays;
        warnexptime = mktime(&st);

        if (warnexptime == static_cast<time_t>(-1))
          warnexptime = static_cast<time_t>(0);
      }
      // When expired or nearly expired use different icon, which is located in list before normal entry
      if (tttXTime <= now) {
        offset = -2;  // Expired
      } else if (tttXTime < warnexptime) {
        offset = -1;  // Warn nearly expired
      }
    }
  }
  
  int i = NORMAL_II;
  switch (entrytype) {
  case CItemData::ET_NORMAL:       i = NORMAL_II + offset; break;
  case CItemData::ET_ALIASBASE:    i = ABASE_II + offset;  break;
  case CItemData::ET_ALIAS:        i = ALIAS_II;           break;
  case CItemData::ET_SHORTCUTBASE: i = SBASE_II + offset;  break;
  case CItemData::ET_SHORTCUT:     i = SHORTCUT_II;        break;
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
      wxGetApp().GetPasswordSafeFrame()->DispatchDblClickAction(*ci);
  }
}

void TreeCtrlBase::SelectItem(const CUUID & uuid)
{
  uuid_array_t uuid_array;
  uuid.GetARep(uuid_array);
  wxTreeItemId id = Find(uuid_array);
  if (id.IsOk()) {
    wxTreeItemId parent = GetItemParent(id);
    if(parent.IsOk() && (parent != GetRootItem()) && ! IsExpanded(parent))
      Expand(parent);
    ::wxSafeYield();
    EnsureVisible(id);
    ::wxSafeYield();
    
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

void TreeCtrl::EditTreeLabel(wxTreeCtrl* tree, const wxTreeItemId& id)
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
    wxMessageBox(_("\"") + _("New Group") + _("\" ") + _("name exists"), _("Duplicate group name"), wxOK|wxICON_ERROR);
    return;
  }
  wxTreeItemId newItem = AddGroup(tostringx(newItemPath));
  wxCHECK_RET(newItem.IsOk(), _("Could not add empty group item to tree"));
  // mark it as a new group that is still under construction.  wxWidgets would delete it
  SetItemData(newItem, new PWTreeItemData(true));
  ::wxSafeYield();
  EnsureVisible(newItem);
  ::wxSafeYield();
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

void TreeCtrl::OnStartLabelEdit( wxTreeEvent& evt )
{
  wxTreeItemId item = evt.GetItem();
  if(IsReadOnly() || !item.IsOk() || !ItemIsGroup(item)) {
    // In case of read only, item not ok or item is no group editing in the tree looks not promissing
    evt.Veto();
    return;
  }
  // If old path not yet set, fill out now before editing
  auto *data = dynamic_cast<PWTreeItemData *>(GetItemData(item));
  if (!data) {
    SetItemData(item, new PWTreeItemData(GetItemGroup(item)));
  }
}

void TreeCtrl::OnEndLabelEdit( wxTreeEvent& evt )
{
  const wxString &label = (evt.IsEditCancelled() ? GetItemText(evt.GetItem()) : evt.GetLabel());

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
              wxMessageBox(_("Group names on the same level must be unique."), _("Duplicate group name"), wxOK|wxICON_ERROR);
              /*
                Wrapped in a 'CallAfter' so the veto and current event can be finalized.
                A call to 'EditTreeLabel' will trigger a new event for editing the label
                and will open a new text entry field, whereas the previously open seems
                still to exists. Hence, without a delay the new event seems to be in
                conflict with the current one.
              */
              CallAfter(&TreeCtrl::EditTreeLabel, this, item);
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
          if(evt.IsEditCancelled()) {
            evt.SetLabel(GetItemText(evt.GetItem())); // On cancel label is empty
            evt.SetEditCanceled(false); // Set to false, as changing no name will be handled the same as cancel
          }
          // A new group being added
          FinishAddingGroup(evt, groupItem);
        }
        else if (data && data->BeingEdited()) {
          // An existing group being renamed
          FinishRenamingGroup(evt, groupItem, data->GetOldPath());
        }
        else {
          // Oops, rename not possible, as starting group name unknown
          wxFAIL_MSG(wxString::Format(wxT("ID_TREECTRL_1 no old path known")));
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

void TreeCtrlTimer::Notify() {
  wxASSERT(m_owner);
  (m_owner->*m_callback)();
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
      if(PosVert < RangeVert) { // Normally ThumVert should be added to PosVert = GetScrollThumb (wxVERTICAL), but we like to have more space at the end of the list
        commandType = wxEVT_SCROLLWIN_PAGEDOWN;
      }
    }
    else if(mousePt.y <= m_upper_scroll_limit) {
      if(PosVert > 0) {
        commandType = wxEVT_SCROLLWIN_LINEUP;
      }
    }
    else if(mousePt.y >= m_lower_scroll_limit) {
      if(PosVert < RangeVert) { // Normally ThumVert should be added to PosVert = GetScrollThumb (wxVERTICAL), but we like to have more space at the end of the list
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
  bool bSetCursor = false;
  wxCursor newCursor = wxNullCursor;
  
  if(m_isDragging && m_drag_item.IsOk())
  {
    int width, height;
    wxPoint pt = ScreenToClient(wxGetMousePosition());
  
    GetSize(&width,  &height);
    
    if((pt.x >= 0) && (pt.x < width)) { // When inside of the windows area
      int flags = wxTREE_HITTEST_ONITEM | wxTREE_HITTEST_BELOW | wxTREE_HITTEST_NOWHERE;
      wxTreeItemId currentItem = DoTreeHitTest(pt, flags);
      bool itemChanged = (currentItem != m_last_mice_item_in_drag_and_drop);
      int PosVert = GetScrollPos (wxVERTICAL);
      int RangeVert = GetScrollRange (wxVERTICAL);
        
      if((PosVert > 0) && (pt.y <= m_upper_scroll_limit)) {
        if(! m_scroll_timer.IsRunning()) {
          m_scroll_timer.Start();
        }
      } else if((PosVert < RangeVert) && (pt.y >= m_lower_scroll_limit)){ // Normally ThumVert should be added to PosVert = GetScrollThumb (wxVERTICAL), but we like to have more space at the end of the list
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
        if(ItemIsGroup(m_drag_item) && (IsDescendant(currentItem, m_drag_item) || (! ::wxGetKeyState(WXK_SHIFT) && ExistsInTree(currentItem, tostringx(label), si)))) {
          bSetCursor = true; // Is already set to wxNullCursor, the default one, so change to wxCURSOR_NO_ENTRY
          newCursor = wxCURSOR_NO_ENTRY;
        }
        else {
          bSetCursor = true; // Is already set to wxNullCursor, the default one
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
        if(m_last_mice_item_in_drag_and_drop) {
          bSetCursor = true; // Is already set to wxNullCursor, the default one
        }
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
        bSetCursor = true; // Is already set to wxNullCursor, the default one
        // TO BE FIXED: This cursor is not visible because macOS means the cursor is not on the current windows area
        m_had_been_out = false;
      }
    }
    else
      m_had_been_out = true;
  }
  
  wxTreeCtrl::OnMouse(event);
  
  // Call SetCurosr after wxTreeCtrl::OnMouse to overrule the setting of the library, which is working fine as long as we are not leaving the window in macOS, where the size change cursor is shown from now on.
  if(bSetCursor) {
    SetCursor(newCursor);
  }
}

void TreeCtrl::OnBeginDrag(wxTreeEvent& evt)
{
  if (m_core.IsReadOnly() || !IsSortingGroup()) {
    evt.Veto();
    return;
  }
  
  wxTreeItemId item = GetSelection();
  
  if(m_bFilterActive && ItemIsGroupOrRoot(item)) { // On filter active we do not know if all items beneath group shall be moved/copied
    evt.Veto();
    return;
  }

  if (item.IsOk() && (GetRootItem() != item)) {
    m_drag_item = item;
    m_had_been_out = false;
    if(PWSprefs::GetInstance()->GetPref(PWSprefs::DragAndDropShowRoot)) // Show root for hard core user, setting in XML config only
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
  int flags = wxTREE_HITTEST_ONITEM | wxTREE_HITTEST_BELOW | wxTREE_HITTEST_NOWHERE;
  wxTreeItemId currentItem = DoTreeHitTest(mousePt, flags); // Check current before removing root from screen
  resetDragItems();
  if(PWSprefs::GetInstance()->GetPref(PWSprefs::DragAndDropShowRoot)) // Restore Hiden Root
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
    auto parentOfDragItem = GetItemParent(m_drag_item);
    auto sxSrcGroupName = tostringx(GetItemText(parentOfDragItem));
    StringX sxDstGroupName;
    bool dragItemLeavesEmptyGroup = (ItemIsGroup(parentOfDragItem) && GetChildrenCount(parentOfDragItem) == 1);
    bool isDestinationEmptyGroup = false;
    
    if(! currentItem.IsOk() && (flags & (wxTREE_HITTEST_BELOW|wxTREE_HITTEST_NOWHERE))) {
      itemDst = GetRootItem();
    }
    else if(itemDst.IsOk() && !(flags & wxTREE_HITTEST_ONITEM) && (currentItem != itemDst)) { // Mice is not over destination item, skip dragging
      evt.Skip();
      m_drag_item = nullptr;
      return;
    }

    if(itemDst && itemDst.IsOk() &&
       (itemDst != m_drag_item) &&
       (itemDst != GetItemParent(m_drag_item)) &&
       ((GetRootItem() != itemDst) || (GetRootItem() != GetItemParent(m_drag_item)))) { // Do not Drag and Drop on its own
      if(GetRootItem() != itemDst) {
        if(! ItemIsGroup(itemDst)) {              // On no group, use parent as destination
          itemDst = GetItemParent(itemDst);
        }
        else {                                    // On group, the group needs to be removed, if it is an empty group
          sxDstGroupName = tostringx(GetItemText(itemDst));
          isDestinationEmptyGroup = m_core.IsEmptyGroup(sxDstGroupName);
        }
        if(IsDescendant(itemDst, m_drag_item)) {  // Do not drag and drop into the moved tree
          wxMessageBox(_("Destination cannot be inside source tree"), _("Drag and Drop failed"), wxOK|wxICON_ERROR);
          evt.Skip();
          m_drag_item = nullptr;
          return;
        }
      }

      auto *commands = MultiCommands::Create(&m_core);

      // If the drag'd item leaves an empty group in the tree
      // the group needs to be created as such in the database.
      if (dragItemLeavesEmptyGroup) {
        commands->Add(
          DBEmptyGroupsCommand::Create(&m_core, sxSrcGroupName, DBEmptyGroupsCommand::EG_ADD)
        );
      }
      // If the destination was an empty group in the tree it is not empty now
      // and the empty group can be removed from the database.
      if (isDestinationEmptyGroup) {
        commands->Add(
          DBEmptyGroupsCommand::Create(&m_core, sxDstGroupName, DBEmptyGroupsCommand::EG_DELETE)
        );
      }

      if(! ItemIsGroup(m_drag_item)) {
        // It's only one item to handle
        CItemData *dataSrc = TreeCtrl::GetItem(m_drag_item);
        wxASSERT(dataSrc);
        StringX sxNewPath = tostringx((GetRootItem() != itemDst) ? GetItemGroup(itemDst) : "");
        CItemData modifiedItem = CreateNewItemAsCopy(dataSrc, sxNewPath, ! doOverride, makeCopy);

        if(makeCopy) {
          if (dataSrc->IsDependent()) {
            commands->Add(
              AddEntryCommand::Create(&m_core, modifiedItem, dataSrc->GetBaseUUID())
            );
          } else { // not alias or shortcut
            commands->Add(
              AddEntryCommand::Create(&m_core, modifiedItem)
            );
          }
        }
        else {
          time_t t;
          time(&t);
          modifiedItem.SetRMTime(t);
          
          // Move is same as rename
          commands->Add(
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
            wxMessageBox(_("Duplicate group name (use Shift to overrule)"), _("Duplicate group name"), wxOK|wxICON_ERROR);
            m_drag_item = nullptr;
            return;
          }
        }

        if(makeCopy) {
          // On control key pressed do copy the tree
          commands->Add(
            CreateCommandCopyGroup(m_drag_item, sxNewPath, sxOldPath, ! doOverride)
          );
        }
        else {
          // Without key board pressed to move, same as rename
          commands->Add(
            CreateCommandRenamingGroup(sxNewPath, sxOldPath)
          );
        }
        
        wxTreeItemId newItem = Find(towxstring(sxNewPath), GetRootItem());
        if (newItem.IsOk())
          wxTreeCtrl::SelectItem(newItem);
      }
      setNodeAsNotEmpty(itemDst);

      // If there are commands execute them
      // otherwise delete the MultiCommand.
      if (commands->GetSize() > 0) {
        commands->Add(
          UpdateGUICommand::Create(
            &m_core,
            UpdateGUICommand::ExecuteFn::WN_ALL,
            UpdateGUICommand::GUI_Action::GUI_REFRESH_TREE
          )
        );
        m_core.Execute(commands);
      }
      else {
        delete commands;
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
  wxGetApp().GetPasswordSafeFrame()->OnContextMenu(GetItem(evt.GetItem()));
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
    auto *parentWindow = wxGetApp().GetPasswordSafeFrame();
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

    auto *commands = MultiCommands::Create(&m_core);
    // The new item we just added above will get removed once the update GUI callback from core happens.
    commands->Add(
      DBEmptyGroupsCommand::Create(&m_core, sxGroup, DBEmptyGroupsCommand::EG_ADD)
    );

    auto parent = GetItemParent(groupItem);
    auto sxParentGroupName = tostringx(GetItemGroup(parent));
    if (parent != GetRootItem() && m_core.IsEmptyGroup(sxParentGroupName)) {
      commands->Add(
        DBEmptyGroupsCommand::Create(&m_core, sxParentGroupName, DBEmptyGroupsCommand::EG_DELETE)
      );
    }

    if (commands) {
      m_core.Execute(commands);
    }

    // evt.GetItem() is not valid anymore.  A new item has been inserted instead.
    // We can select it using the full path we computed earlier
    wxTreeItemId newItem = Find(groupName, GetRootItem());
    if (newItem.IsOk()) {
      wxTreeCtrl::SelectItem(newItem);
      // correct group icon of parent if needed
      wxTreeItemId parent = GetItemParent(newItem);
      setNodeAsNotEmpty(parent);
    }
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
  
  ExecuteMultiCommands(
    CreateCommandRenamingGroup(sxNewPath, sxOldPath)
  );

  // The old treeItem is gone, since it was renamed.  We need to find the new one to select it
  wxTreeItemId newItem = Find(towxstring(sxNewPath), GetRootItem());
  if (newItem.IsOk())
    wxTreeCtrl::SelectItem(newItem);
}

MultiCommands* TreeCtrl::CreateCommandRenamingGroup(StringX sxNewPath, StringX sxOldPath)
{
  // We DON'T need to handle these two as they can only occur while moving items
  //    not removing groups as they become empty
  //    renaming of groups that have only other groups as children

  MultiCommands* pmcmd = MultiCommands::Create(&m_core);
  if (!pmcmd)
    return pmcmd;

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

  return pmcmd;
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
    if(newEntry)
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

MultiCommands* TreeCtrl::CreateCommandCopyGroup(wxTreeItemId itemSrc, StringX sxNewPath, StringX sxOldPath, bool checkName)
{
  MultiCommands* pmcmd = MultiCommands::Create(&m_core);
  if (!pmcmd)
    return pmcmd;
  
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
  
  return pmcmd;
}

void TreeCtrl::ExecuteMultiCommands(MultiCommands* commands)
{
  if (commands && commands->GetSize() > 0) {
    m_core.Execute(commands);
  }
}

/*!
 * wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
 */

void TreeCtrl::OnTreectrlSelChanged( wxTreeEvent& evt )
{
  CItemData *pci = GetItem(evt.GetItem());

  wxGetApp().GetPasswordSafeFrame()->UpdateSelChanged(pci);
}

static void ColourChildren(TreeCtrl *tree, wxTreeItemId parent, const wxColour &colour)
{
  wxTreeItemIdValue cookie;
  wxTreeItemId child = tree->GetFirstChild(parent, cookie);

  while (child) {
    tree->SetItemTextColour(child, colour);
    if (tree->ItemHasChildren(child))
      ColourChildren(tree, child, colour);
    child = tree->GetNextChild(parent, cookie);
  }
}

void TreeCtrl::SetFilterState(bool state)
{
  const wxColour *colour = state ? wxRED : wxBLACK;
  // iterate over all items, no way to do this en-mass
  wxTreeItemId root = GetRootItem();
  if (root)
    ColourChildren(this, root, *colour);
  SetFilterActive(state);
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

void TreeCtrl::OnDrag(wxAuiToolBarEvent& event)
{
#if wxUSE_DRAG_AND_DROP
  if(m_last_dnd_item == nullptr) {
    event.Skip();
    return;
  }
  
  pws_os::Trace(L"TreeCtrl::OnDrag Start");
  wxMemoryBuffer buffer;
  wxString fileName = L"";
  CollectDnDData(buffer, fileName);
  if(! buffer.GetDataLen()) {
    pws_os::Trace(L"TreeCtrl::OnDrag End without buffer content");
    event.Skip();
    return;
  }
  
  pws_os::Trace(L"TreeCtrl::OnDrag Size %ld", static_cast<long>(buffer.GetDataLen()));
  
  DnDPWSafeObject dragData(&buffer);
  
  wxDropSource source(dragData, this);
  m_run_dnd = true; // mark running drag and drop to be aware on local dropping
  
  switch(source.DoDragDrop(true))
  {
    default:
    case wxDragError:
      pws_os::Trace(L"TreeCtrl::OnDrag Error");
      break;
      
    case wxDragNone:
      pws_os::Trace(L"TreeCtrl::OnDrag None (nothing happend)");
      break;
      
    case wxDragCopy:
      pws_os::Trace(L"TreeCtrl::OnDrag Copy");
      break;
      
    case wxDragMove:
      pws_os::Trace(L"TreeCtrl::OnDrag Move");
#if !defined(__WXMAC__)
      // mac OS darg and drop only allows Move, as control/command is not handled
      // Perform copy as default action for mac OS
      if(! ::wxGetKeyState(WXK_CONTROL) && ! m_core.IsReadOnly()) {
        class Command *doit;
        doit = wxGetApp().GetPasswordSafeFrame()->Delete(m_last_dnd_item);
        if (doit != nullptr)
          m_core.Execute(doit);
      }
#endif
      break;
      
    case wxDragCancel:
      pws_os::Trace(L"TreeCtrl::OnDrag Cancel");
      break;
  }
  
  if(! fileName.IsEmpty() && wxFileExists(fileName)) {
    wxRemoveFile(fileName);
  }
  
  m_run_dnd = false;
#else
  event.Skip();
#endif
  m_last_dnd_item = nullptr;
}

void TreeCtrl::CollectDnDData(wxMemoryBuffer &outDDmem, wxString &fileName)
{
  std::vector<StringX> vEmptyGroups;
  wxTreeItemId parent = GetItemParent(m_last_dnd_item);
  StringX DragPathParent = tostringx((GetRootItem() != m_last_dnd_item) ? GetItemGroup(parent) : "");
  DnDObList dnd_oblist(DragPathParent.length());
  // With shift key pressed on drag left full group, else wise remove path to group
  dnd_oblist.SetDragNode(::wxGetKeyState(WXK_SHIFT) ? false : true);
  
  if((m_last_dnd_item == nullptr) || (GetRootItem() == m_last_dnd_item))
    return;
  
  // Collect all items to drag in dnd_oblist and vEmptyGroups
  if(!ItemIsGroup(m_last_dnd_item)) {
    // It's only one item to handle
    CItemData *pci = TreeCtrl::GetItem(m_last_dnd_item);
    wxASSERT(pci);
    GetEntryData(dnd_oblist, pci);
  }
  else {
    StringX DragPath = tostringx(GetItemGroup(m_last_dnd_item));
    
    if(GetChildrenCount(m_last_dnd_item) == 0) {
      // Don't bother looking for children, it is only one empty group
      if(dnd_oblist.CutGroupPath() && (dnd_oblist.DragPathParentLen() > 0) && (DragPath.length() > dnd_oblist.DragPathParentLen())) {
        vEmptyGroups.push_back(DragPath.substr(dnd_oblist.DragPathParentLen() + 1));
      }
      else {
        vEmptyGroups.push_back(DragPath);
      }
    }
    else {
      // Handle all items selected
      GetGroupEntriesData(dnd_oblist, m_last_dnd_item);
      // Handle all empty groups in addition
      typedef std::vector<StringX> EmptyGroupsArray;
      const EmptyGroupsArray& emptyGroups = m_core.GetEmptyGroups();
      StringX sxDragPathWithDot = DragPath + _T('.');
      for(const auto & emptyGroup : emptyGroups)
      {
        if(emptyGroup == DragPath || emptyGroup.find(sxDragPathWithDot) == 0) {
          if(dnd_oblist.CutGroupPath() && (dnd_oblist.DragPathParentLen() > 0) && (emptyGroup.length() > dnd_oblist.DragPathParentLen())) {
            vEmptyGroups.push_back(emptyGroup.substr(dnd_oblist.DragPathParentLen() + 1));
          }
          else {
            vEmptyGroups.push_back(emptyGroup);
          }
        }
      }
    }
  }
  
  // Store entries in buffer, at least write header
  dnd_oblist.DnDSerialize(outDDmem);
  
  // Now process empty groups
  if(!vEmptyGroups.empty()) {
    // Add special field to ensure we recognise the extra data correctly
    // when dropping
    outDDmem.AppendData("egrp", 4);

    size_t nemptygroups = vEmptyGroups.size();
    outDDmem.AppendData((void *)&nemptygroups, sizeof(size_t));
    for(size_t i = 0; i < nemptygroups; i++) {
      CUTF8Conv conv;
      const unsigned char *utf8;
      size_t utf8Len;

      if(conv.ToUTF8(vEmptyGroups[i].c_str(), utf8, utf8Len)) {
        outDDmem.AppendData((void *)&utf8Len, sizeof(size_t));
        outDDmem.AppendData(utf8, utf8Len);
      } else {
        wxASSERT(false);
      }
    }
  }
  
  int limit = PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::DNDMaxMemSize);
  bool ferror = false;
  wxFile *tmpfile = nullptr;
  wxString name;
  // On huge data we encounter problems with drag and drop, data buffer has correct size, but content is zero only.
  // For flexible adaptation in pwsafe.cfg the parameter "DNDMaximumMemorySize" (DNDMaxMemSize) might be set to one of the below values.
  // On limit is > 0 we do write to a file is data already achived demanded limit or attachement is given
  // On limit is == 0 we do write to a file if attachment is present (only)
  // On limit is == -1 we never write to a file
  if(((limit > 0) && (outDDmem.GetDataLen() >= static_cast<size_t>(limit))) || ((limit != -1) && dnd_oblist.HasAttachments())) {
    // Create temporary file and write into it
    name = wxFileName::CreateTempFileName(L"", tmpfile = new wxFile());
    if(!name.IsEmpty() && tmpfile != nullptr) {
      if(tmpfile->Write(outDDmem.GetData(), outDDmem.GetDataLen()) == outDDmem.GetDataLen()) {
        if(dnd_oblist.HasAttachments()) {
          if(tmpfile->Write("atta", 4) != 4)
            ferror = true;
          else if(! dnd_oblist.DnDSerializeAttachments(m_core, tmpfile))
            ferror = true;
        }
      }
      else {
        ferror = true;
      }
      tmpfile->Close();
    }
    else {
      ferror = true;
    }
    if(ferror) {
      if(! name.IsEmpty() && wxFileExists(name))
        wxRemoveFile(name);
      if(tmpfile != nullptr)
        delete tmpfile;
      // On error just try to use the buffer, also for attachment
      tmpfile = nullptr;
    }
    else {
      // Mark as file is 0x00 00 00 00 00 <length as size_t> <UTF-8 coded file name with given length>
      outDDmem.Clear();
      int nCount = 0;
      outDDmem.AppendData((void *)&nCount, sizeof(int));
      std::string utf8name = toutf8(tostdstring(name));
      size_t length = utf8name.length();
      outDDmem.AppendData((void *)&length, sizeof(size_t));
      outDDmem.AppendData(&(*utf8name.begin()), length);
      // Store name to remove after drag and drop
      fileName = name;
    }
  }
  
  // Store Attachments according to list build up when storing the entries
  if((tmpfile == nullptr) && dnd_oblist.HasAttachments()) {
    // Add special field to ensure we recognise the extra data correctly
    // when dropping
    outDDmem.AppendData("atta", 4);
    // Write attachement data separate from base entries
    dnd_oblist.DnDSerializeAttachments(m_core, outDDmem);
  }
  
  if(tmpfile != nullptr)
    delete tmpfile;
}

void TreeCtrl::GetGroupEntriesData(DnDObList &dnd_oblist, wxTreeItemId item)
{
  if(! ItemIsGroup(item)) {
    CItemData *pci = TreeCtrl::GetItem(item);
    wxASSERT(pci != NULL);
    GetEntryData(dnd_oblist, pci);
  } else {
    wxTreeItemIdValue cookie;
    wxTreeItemId ti = GetFirstChild(item, cookie);

    while(ti) {
      GetGroupEntriesData(dnd_oblist, ti);
      ti = GetNextSibling(ti);
    }
  }
}

void TreeCtrl::GetEntryData(DnDObList &dnd_oblist, CItemData *pci)
{
  wxASSERT(pci != NULL);
  DnDObject *pDnDObject = new DnDObject;
  wxASSERT(pDnDObject);

  if(dnd_oblist.CutGroupPath() && (dnd_oblist.DragPathParentLen() > 0)) {
    CItemData ci2(*pci); // we need a copy since to modify the group
    const StringX cs_Group = pci->GetGroup();
    ci2.SetGroup(cs_Group.length() > dnd_oblist.DragPathParentLen() ? cs_Group.substr(dnd_oblist.DragPathParentLen() + 1) : L"");
    pDnDObject->FromItem(ci2);
  } else {
    pDnDObject->FromItem(*pci);
  }

  if(pci->IsDependent()) {
    // I'm an alias or shortcut; pass on ptr to my base item
    // to retrieve its group/title/user
    CItemData *pbci = m_core.GetBaseEntry(pci);
    wxASSERT(pbci != NULL);
    pDnDObject->SetBaseItem(pbci);
  }

  dnd_oblist.AddTail(pDnDObject);
}

bool TreeCtrl::ProcessDnDData(StringX &sxDropPath, wxMemoryBuffer *inDDmem)
{
  wxASSERT(inDDmem);
  wxMemoryInputStream memStream(inDDmem->GetData(), inDDmem->GetDataLen());
  DnDObList dnd_oblist;
  wxFileInputStream *fileStream = nullptr;
  wxString fileName;
  wxInputStream *stream = &memStream;
  
  // Check on drop is stored in file
  if(inDDmem->GetDataLen() > (sizeof(int)+sizeof(size_t))) {
    int nCnt;
    memStream.Read(&nCnt, sizeof(int));
    
    pws_os::Trace(L"TreeCtrl::ProcessDnDData Size %ld Count %d", static_cast<long>(inDDmem->GetDataLen()), nCnt);
    
    if(memStream.LastRead() == sizeof(int) && (nCnt == 0)) {
      size_t len;
      memStream.Read(&len, sizeof(size_t));
      if(len > 0) {
        unsigned char *utf8 = nullptr;
        utf8 = new unsigned char[len+1];
        wxASSERT(utf8);
        // Read file name from buffer, stored as UTF-8
        memset(utf8, 0, len+1);
        stream->Read(utf8, len);
        if(stream->LastRead() == len) {
          CUTF8Conv conv;
          StringX sxdata;
          // Convert to wxString
          conv.FromUTF8(utf8, len, sxdata);
          fileName = towxstring(sxdata);
          
          fileStream = new wxFileInputStream(fileName);
          if(fileStream != nullptr && fileStream->IsOk()) {
            stream = fileStream;
          }
          else if(fileStream != nullptr) {
            delete fileStream;
            fileStream = nullptr;
            fileName.Clear();
          }
        }
        delete[] utf8;
      }
    }
    if(fileStream == nullptr) {
      memStream.SeekI(0); // Set back to start
    }
  }
  
  // Get all the entries
  dnd_oblist.DnDUnSerialize(*stream);
  // Now check if empty group list is appended to the item data
  // Empty groups have a dummy header of 'egrp' to check it is ours
  // Note: No EOF indication in CFile and hence CMemfile only to check
  // we have read in is what we wanted!
  std::vector<StringX> vsxEmptyGroups;
  StringX sxDropGroup(L"");
  if(!sxDropPath.empty()) // Add DOT at end of drop destination if not root
    sxDropGroup = sxDropPath + GROUP_SEP;

  char chdr[5] = { 0 };
  stream->Read(chdr, 4);
  if((stream->LastRead() == 4) && (strncmp(chdr, "egrp", 4) == 0)) {
    // It is ours - now process the empty groups being dropped
    size_t nemptygroups, utf8Len, buffer_size = 0;
    unsigned char *utf8 = nullptr;
    
    stream->Read((void *)&nemptygroups, sizeof(nemptygroups));
    if(stream->LastRead() == sizeof(size_t)) {
      for(size_t i = 0; i < nemptygroups; i++) {
        StringX sxEmptyGroup;
        CUTF8Conv conv;

        stream->Read((void *)&utf8Len, sizeof(size_t));
        if(stream->LastRead() != sizeof(utf8Len)) {
          delete[] utf8;
          return false;
        }
        if(utf8Len > buffer_size) {
          delete[] utf8;
          buffer_size = utf8Len * 2;
          utf8 = new unsigned char[buffer_size+1];
          wxASSERT(utf8);
        }

        // Clear buffer
        memset(utf8, 0, buffer_size);
        stream->Read(utf8, utf8Len);
        if(stream->LastRead() != utf8Len) {
          delete[] utf8;
          return false;
        }

        conv.FromUTF8(utf8, utf8Len, sxEmptyGroup);
        vsxEmptyGroups.push_back(sxEmptyGroup);
      }
    }
    else // Buffer utf8 still not allocated
      return false;
    delete[] utf8;
    
    // Read again to double check on attachement
    stream->Read(chdr, 4);
  }
  
  // Check on attachments following, in case local data base is V4
  if((stream->LastRead() == 4) && (strncmp(chdr, "atta", 4) == 0)) {
    if(m_core.GetReadFileVersion() == PWSfile::V40) {
      // Get all the attachments
      dnd_oblist.DnDUnSerializeAttachments(*stream);
    }
    else {
      wxMessageBox(_("Attachments not overtaken due to data base version"), _("Drag and Drop"), wxOK|wxICON_WARNING);
    }
  }
  
  if(fileStream != nullptr)
    delete fileStream;
  if(! fileName.IsEmpty() && wxFileExists(fileName))
    wxRemoveFile(fileName);
  
  if(!dnd_oblist.IsEmpty() || !vsxEmptyGroups.empty()) {
    
    MultiCommands* pmcmd = MultiCommands::Create(&m_core);
    if(!pmcmd)
      return false;

    pmcmd->Add(UpdateGUICommand::Create(&m_core,
       UpdateGUICommand::WN_UNDO, UpdateGUICommand::GUI_REFRESH_BOTHVIEWS));
    
    // Copy the selected tree with all entries
    AddDnDEntries(pmcmd, dnd_oblist, sxDropPath);
    
    // But we have to do the empty groups ourselves because EG_ADD is not recursive
    for(const auto & emptyGroup : vsxEmptyGroups)
    {
        pmcmd->Add(DBEmptyGroupsCommand::Create(&m_core, sxDropGroup + emptyGroup, DBEmptyGroupsCommand::EG_ADD));
    }
    
    pmcmd->Add(UpdateGUICommand::Create(&m_core,
      UpdateGUICommand::WN_EXECUTE_REDO, UpdateGUICommand::GUI_REFRESH_BOTHVIEWS));
    
    if(pmcmd->GetSize() > 2) {
      
      // Since we added some commands apart from the first & last WM_UNDO/WM_REDO,
      // check if original drop group was empty - if so, it won't be now
      // We need to insert it after the first command (WN_UNDO, GUI_REFRESH_BOTHVIEWS)
      if(!sxDropPath.empty() && m_core.IsEmptyGroup(sxDropPath)) {
        pmcmd->Insert(DBEmptyGroupsCommand::Create(&m_core, sxDropPath, DBEmptyGroupsCommand::EG_DELETE), 1);
      }
      
      // Execute the event
      m_core.Execute(pmcmd);
    }
    
    // FixListIndexes();
    wxGetApp().GetPasswordSafeFrame()->RefreshViews();
    
    return true;
  }
  
  return false;
}

void TreeCtrl::AddDnDEntries(MultiCommands *pmCmd, DnDObList &dnd_oblist, StringX &sxDropPath) // DropPath is not including trailing dot
{
  // Add Drop entries
  CItemData ci_temp;
  UUIDVector Possible_Aliases, Possible_Shortcuts;
  std::vector<StringX> vAddedPolicyNames;
  StringX sxEntriesWithNewNamedPolicies;
  std::map<StringX, StringX> mapRenamedPolicies;
  StringX sxgroup, sxtitle, sxuser;
  DnDIterator pos;
  StringX sxEntriesWithBaseEntryMissing;
  StringX sxDropGroup(L"");
  if(!sxDropPath.empty()) // Add DOT at end of drop destination if not root
    sxDropGroup = sxDropPath + GROUP_SEP;

  const StringX sxDD_DateTime = PWSUtil::GetTimeStamp(true).c_str();

  // Initialize set
  GTUSet setGTU;
  m_core.InitialiseGTU(setGTU);
  
  for (pos = dnd_oblist.ObjectsBegin(); pos != dnd_oblist.ObjectsEnd(); ++pos) {
    DnDObject *pDnDObject = *pos;
    wxASSERT(pDnDObject);
    pws_os::CUUID uuid = pDnDObject->GetUUID();
    if (m_core.Find(uuid) != m_core.GetEntryEndIter()) {
      // UUID already in use - get a new one!
      pDnDObject->CreateUUID();
      if(pDnDObject->GetBaseUUID() == CUUID::NullUUID()) { // When not alias or shortcut
        // Update base UUID or attachment
        pws_os::CUUID new_uuid = pDnDObject->GetUUID();
        dnd_oblist.UpdateBaseUUIDinDnDEntries(uuid, new_uuid);
      }
    }
  }
  
  for (CItemAttIterator iter = dnd_oblist.AttachmentsBegin(); iter != dnd_oblist.AttachmentsEnd(); ++iter) {
    CItemAtt *pAttaObject = *iter;
    wxASSERT(pAttaObject);
    pws_os::CUUID uuid = pAttaObject->GetUUID();
    if (m_core.HasAtt(uuid)) {
      // UUID already in use - get a new one!
      pAttaObject->CreateUUID();
      // Update base UUID
      pws_os::CUUID new_uuid = pAttaObject->GetUUID();
    }
  }

  for (pos = dnd_oblist.ObjectsBegin(); pos != dnd_oblist.ObjectsEnd(); ++pos) {
    DnDObject *pDnDObject = *pos;
    wxASSERT(pDnDObject);

    bool bChangedPolicy(false);
    ci_temp.Clear();
    // Only set to false if adding a shortcut where the base isn't there (yet)
    bool bAddToViews = true;
    pDnDObject->ToItem(ci_temp);

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
      bool bNPWInCurrentDB = m_core.GetPolicyFromName(sxPolicyName, currentDB_st_pp);
      if (bNPWInCurrentDB) {
        // It exists in target database
        if (st_pp != currentDB_st_pp) {
          // They are not the same - make this policy unique
          m_core.MakePolicyUnique(mapRenamedPolicies, sxPolicyName, sxDD_DateTime, IDSC_DRAGPOLICY);
          ci_temp.SetPolicyName(sxPolicyName);
          bChangedPolicy = true;
        }
      }
      
      if (!bNPWInCurrentDB || bChangedPolicy) {
        // Not in target database or has different settings -
        // Add it if we haven't already
        if (std::find(vAddedPolicyNames.begin(), vAddedPolicyNames.end(), sxPolicyName) == vAddedPolicyNames.end()) {
          // Doesn't already exist and we haven't already added it - add
          pmCmd->Add(DBPolicyNamesCommand::Create(&m_core, sxPolicyName, st_pp));
          vAddedPolicyNames.push_back(sxPolicyName);
        }
        // No longer need these values
        ci_temp.SetPWPolicy(L"");
        ci_temp.SetSymbols(L"");
      }
    }

    // Using shift will force to the root, but the path is left as it is
    StringX oldSXgroup = ci_temp.GetGroup();
    if(oldSXgroup.empty()) {
      sxgroup = sxDropPath;
    }
    else {
      sxgroup = sxDropGroup + oldSXgroup;
    }

    sxuser = ci_temp.GetUser();
    StringX sxnewtitle(ci_temp.GetTitle());
    m_core.MakeEntryUnique(setGTU, sxgroup, sxnewtitle, sxuser, IDSC_DRAGNUMBER);

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
    if (cs_tmp.substr(0, 2) == L"[[" && cs_tmp.substr(cs_tmp.length() - 2) == L"]]") {
      cs_tmp = cs_tmp.substr(1, cs_tmp.length() - 2);
      pl.InputType = CItemData::ET_ALIAS;
    }
    
    // Potentially remove tilde as ParseBaseEntryPWD expects only
    // one set of square brackets (processing import and user edit of entries)
    if (cs_tmp.substr(0, 2) == L"[~" && cs_tmp.substr(cs_tmp.length() - 2) == L"~]") {
      cs_tmp = L"[" + cs_tmp.substr(2, cs_tmp.length() - 4) + L"]";
      pl.InputType = CItemData::ET_SHORTCUT;
    }

    // If we have BASEUUID, life is simple
    if (ci_temp.GetBaseUUID() != CUUID::NullUUID()) {
      pl.base_uuid = ci_temp.GetBaseUUID();
      if((m_core.Find(pl.base_uuid) != m_core.GetEntryEndIter()) || dnd_oblist.CanFind(pl.base_uuid)) {
        // If object already exist or will be added
        pl.ibasedata = 1;
        pl.TargetType = CItemData::ET_NORMAL;
      }
      else {
        // Overtake, but change to normal item
        StringX sxChanged = L"\r\n\xab" + sxgroup + L"\xbb " +
                            L"\xab" + sxtitle + L"\xbb " +
                            L"\xab" + sxuser + L"\xbb";
        sxEntriesWithBaseEntryMissing += sxChanged;

        pl.ibasedata = 0;
        pl.base_uuid = CUUID::NullUUID();
      }
    } else {
        m_core.ParseBaseEntryPWD(cs_tmp, pl);
    }
    
    if (pl.ibasedata > 0) {
      // Add to pwlist
      pmCmd->Add(AddEntryCommand::Create(&m_core,
                                         ci_temp, ci_temp.GetBaseUUID(),
                                         dnd_oblist.AttachmentOfItem(&ci_temp))); // need to do this as well as AddDep...

      // Password in alias/shortcut format AND base entry exists
      if(pl.InputType == CItemData::ET_ALIAS) {
        if(pl.TargetType == CItemData::ET_ALIAS) {
          // This base is in fact an alias. ParseBaseEntryPWD already found 'proper base'
          // So dropped entry will point to the 'proper base' and tell the user.
          wxString cs_msg = wxString::Format(_("Dropped entry '%s:%s:%s' password points to an entry that is also an alias.\n\nThe dropped entry now references that entry's base entry."),
                            sxgroup.c_str(),
                            sxtitle.c_str(),
                            sxuser.c_str());
          wxMessageBox(cs_msg, _("Drag and Drop failed"), wxOK);
        } else if(pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
          // Only normal or alias base allowed as target
          wxString cs_msg = wxString::Format(_("This alias's target [%s:%s:%s] is not a normal entry."),
                            sxgroup.c_str(),
                            sxtitle.c_str(),
                            sxuser.c_str());
          wxMessageBox(cs_msg, _("Drag and Drop failed"), wxOK);
          continue;
        }
        
        pmCmd->Add(AddDependentEntryCommand::Create(&m_core, pl.base_uuid,
                                                    ci_temp.GetUUID(),
                                                    CItemData::ET_ALIAS));

        ci_temp.SetPassword(L"[Alias]");
        ci_temp.SetAlias();
      } else if(pl.InputType == CItemData::ET_SHORTCUT) {
        if(pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_SHORTCUTBASE) {
          // Only normal or shortcut base allowed as target
          wxString cs_msg = wxString::Format(_("This shortcut's target [%s:%s:%s] is not a normal entry."),
                            sxgroup.c_str(),
                            sxtitle.c_str(),
                            sxuser.c_str());
          wxMessageBox(cs_msg, _("Drag and Drop failed"), wxOK);
          continue;
        }

        pmCmd->Add(AddDependentEntryCommand::Create(&m_core,
                                                    pl.base_uuid,
                                                    ci_temp.GetUUID(),
                                                    CItemData::ET_SHORTCUT));

        ci_temp.SetPassword(L"[Shortcut]");
        ci_temp.SetShortcut();
      }
    } else if(pl.ibasedata == 0) {
      // Password NOT in alias/shortcut format
      ci_temp.SetNormal();
    } else if(pl.ibasedata < 0) {
      // Password in alias/shortcut format AND base entry does not exist or multiple possible
      // base entries exit.
      // Note: As more entries are added, what was "not exist" may become "OK",
      // "no unique exists" or "multiple exist".
      // Let the code that processes the possible aliases after all have been added sort this out.
      if(pl.InputType == CItemData::ET_ALIAS) {
         Possible_Aliases.push_back(ci_temp.GetUUID());
      } else if(pl.InputType == CItemData::ET_SHORTCUT) {
         Possible_Shortcuts.push_back(ci_temp.GetUUID());
         bAddToViews = false;
      }
    }
    ci_temp.SetStatus(CItemData::ES_ADDED);

    // Need to check that entry keyboard shortcut not already in use!
    int32 iKBShortcut;
    ci_temp.GetKBShortcut(iKBShortcut);
       
    if(iKBShortcut != 0 && m_core.GetKBShortcut(iKBShortcut) != CUUID::NullUUID()) {
      // Remove it but no mechanism to tell user!
      ci_temp.SetKBShortcut(0);
    }
    
    if(!ci_temp.IsDependent()) { // Dependents handled later
      // Add to pwlist
      AddEntryCommand *pcmd = AddEntryCommand::Create(&m_core, ci_temp, pws_os::CUUID::NullUUID(),
                                                      dnd_oblist.AttachmentOfItem(&ci_temp));

      if(!bAddToViews) {
        // ONLY Add to pwlist and NOT to Tree or List views
        // After the call to AddDependentEntries for shortcuts, check if still
        // in password list and, if so, then add to Tree + List views
        pcmd->SetNoGUINotify();
      }
      pmCmd->Add(pcmd);
    }
  } // iteration over in_oblist

  // Now try to add aliases/shortcuts we couldn't add in previous processing
  if(!Possible_Aliases.empty()) {
    AddDependentEntriesCommand *pcmdA = AddDependentEntriesCommand::Create(&m_core,
                                                        Possible_Aliases, NULL,
                                                        CItemData::ET_ALIAS,
                                                        CItemData::PASSWORD);
    pmCmd->Add(pcmdA);
  }

  if(!Possible_Shortcuts.empty()) {
    AddDependentEntriesCommand *pcmdS = AddDependentEntriesCommand::Create(&m_core,
                                                        Possible_Shortcuts, NULL,
                                                        CItemData::ET_SHORTCUT,
                                                        CItemData::PASSWORD);
    pmCmd->Add(pcmdS);
  }
  
  // Clear set
  setGTU.clear();

  if(!sxEntriesWithNewNamedPolicies.empty()) {
    // A number of entries had a similar named password policy but with
    // different settings to those in this database.
    // Tell user
    wxString cs_msg;
    cs_msg = wxString::Format(_("The following entries had a policy name that already existed in this database but with different settings.\nA new named policy has been created with the current date/time (%ls) appended:%ls"),
                  sxDD_DateTime.c_str(),
                  sxEntriesWithNewNamedPolicies.c_str());
    wxMessageBox(cs_msg, _("Entry Password Policy Changes"), wxOK);
  }
  
  if(!sxEntriesWithBaseEntryMissing.empty()) {
    // A number of alias/shortcut had been dropped, but baseitem is missing. Conversion to normal entry, but no real content is present
    // Tell user
    wxString cs_msg;
    cs_msg = wxString::Format(_("The following alias or shortcut entries did not match to a base entry and had been converted to normal entries:%ls"),
                        sxEntriesWithBaseEntryMissing.c_str());
    wxMessageBox(cs_msg, _("Entry Type Changed"), wxOK);
  }
}

wxDragResult TreeCtrl::OnDrop(wxCoord x, wxCoord y, wxMemoryBuffer *inDDmem)
{
  if(! inDDmem) {
    pws_os::Trace(L"TreeCtrl::OnDrop return 'wxDragNone' on missing memory");
    return wxDragNone;
  }

  wxDragResult result = wxDragNone;
  wxPoint mousePt(x, y);
  int flags = wxTREE_HITTEST_ONITEM | wxTREE_HITTEST_BELOW | wxTREE_HITTEST_NOWHERE;
  wxTreeItemId itemDst = DoTreeHitTest(mousePt, flags);
  bool reportCopy = ::wxGetKeyState(WXK_CONTROL);
  bool placeRoot = ::wxGetKeyState(WXK_SHIFT);
  StringX sxDropPath;
  
  if(!placeRoot && (flags & wxTREE_HITTEST_ONITEM) && itemDst.IsOk()) {
    if(! ItemIsGroup(itemDst)) {              // On no group, use parent as destination
      itemDst = GetItemParent(itemDst);
    }
    if(itemDst == GetRootItem()) {
      sxDropPath = L""; // Place into root
    }
    else {
      sxDropPath = tostringx(GetItemGroup(itemDst));
    }
      
  }
  else {
    itemDst = GetRootItem();
    sxDropPath = L""; // Place into root
  }
  
  if(m_run_dnd && IsDescendant(itemDst, m_last_dnd_item)) {
    wxMessageBox(_("Destination cannot be inside source tree"), _("Drag and Drop failed"), wxOK|wxICON_ERROR);
    pws_os::Trace(L"TreeCtrl::OnDrop return 'wxDragNone' on recursive dropping");
    return wxDragNone;
  }
  
  pws_os::Trace(L"TreeCtrl::OnDrop to path '%ls'", sxDropPath.c_str());
  
  if (ProcessDnDData(sxDropPath, inDDmem)) {
    pws_os::Trace(L"TreeCtrl::OnDrop return '%s'", reportCopy ? "wxDragCopy" : "wxDragMove");
    result = reportCopy ? wxDragCopy : wxDragMove;

    setNodeAsNotEmpty(itemDst);
  }
  else {
    pws_os::Trace(L"TreeCtrl::OnDrop return 'wxDragNone'");
  }

  delete inDDmem;
  return result;
}

void TreeCtrlBase::setNodeAsNotEmpty(const wxTreeItemId item)
{
  if(GetRootItem() != item && GetItemImage(item) == EMPTY_NODE_II)
    wxTreeCtrl::SetItemImage(item, NODE_II); // Could be empty before
}

void TreeCtrlBase::setNodeAsEmptyIfNeeded(const wxTreeItemId item)
{
  if(GetRootItem() != item && GetChildrenCount(item) == 0) {
    wxTreeCtrl::SetItemImage(item, EMPTY_NODE_II); // Empty Group shall show the empty node icon
  }
}
