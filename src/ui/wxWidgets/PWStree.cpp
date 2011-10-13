/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWSTreeCtrl.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes
#include <utility> // for make_pair
#include <vector>

#include "PWStree.h"
#include "passwordsafeframe.h" // for DispatchDblClickAction()
#include "core/PWSprefs.h"

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

/*!
 * PWSTreeCtrl event table definition
 */

BEGIN_EVENT_TABLE( PWSTreeCtrl, wxTreeCtrl )

////@begin PWSTreeCtrl event table entries
  EVT_TREE_ITEM_ACTIVATED( ID_TREECTRL, PWSTreeCtrl::OnTreectrlItemActivated )
  EVT_TREE_ITEM_MENU( ID_TREECTRL, PWSTreeCtrl::OnContextMenu )
  EVT_CHAR( PWSTreeCtrl::OnChar )
  EVT_CUSTOM(wxEVT_GUI_DB_PREFS_CHANGE, wxID_ANY, PWSTreeCtrl::OnDBGUIPrefsChange)
  EVT_TREE_ITEM_GETTOOLTIP( ID_TREECTRL, PWSTreeCtrl::OnGetToolTip )
////@end PWSTreeCtrl event table entries
END_EVENT_TABLE()


// helper class to match CItemData with wxTreeItemId
class PWTreeItemData : public wxTreeItemData
{
public:
  PWTreeItemData(const CItemData &item)
  {
    item.GetUUID(m_uuid);
  }
  const uuid_array_t &GetUUID() const {return m_uuid;}
private:
  uuid_array_t m_uuid;
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

  wxImageList *iList = new wxImageList(9, 9, true, Nimages);
  for (int i = 0; i < Nimages; i++)
    iList->Add(wxBitmap(xpmList[i]));
  AssignImageList(iList);
}

// XXX taken from Windows PWSTreeCtrl.cpp
// XXX move to core
static StringX GetPathElem(StringX &path)
{
  // Get first path element and chop it off, i.e., if
  // path = "a.b.c.d"
  // will return "a" and path will be "b.c.d"
  // (assuming GROUP_SEP is '.')
  const TCHAR GROUP_SEP = _T('.');

  StringX retval;
  StringX::size_type N = path.find(GROUP_SEP);
  if (N == StringX::npos) {
    retval = path;
    path = _T("");
  } else {
    retval = path.substr(0, N);
    path = path.substr(N + 1);
  }
  return retval;
}

bool PWSTreeCtrl::ExistsInTree(wxTreeItemId node,
                               const StringX &s, wxTreeItemId &si)
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
  wxString disp = title;

  if (prefs->GetPref(PWSprefs::ShowUsernameInTree)) {
    const wxString user = item.GetUser().c_str();
    if (!user.empty())
      disp += _T(" [") + user + _("]");
  }

  if (prefs->GetPref(PWSprefs::ShowPasswordInTree)) {
    const wxString passwd = item.GetPassword().c_str();
    if (!passwd.empty())
      disp += _T(" {") + passwd + _("}");
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
      retval += _(".");
  }
  return retval;
}

wxString PWSTreeCtrl::GetItemGroup(const wxTreeItemId& item) const
{
  if (!item.IsOk() || item == GetRootItem())
    return wxEmptyString;
  else if (ItemHasChildren(item)) {
    const wxString path = GetPath(item);
    const wxString name = GetItemText(item);
    if (path.IsEmpty())//parent is root
      return name; //group under root
    else
      return path + _(".") + name; //sub-group of some (non-root) group
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
    wxRect rc;
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
    return NULL;

  PWTreeItemData *itemData = dynamic_cast<PWTreeItemData *>(GetItemData(id));
  // return if a group is selected
  if (itemData == NULL)
    return NULL;

  ItemListIter itemiter = m_core.Find(itemData->GetUUID());
  if (itemiter == m_core.GetEntryEndIter())
    return NULL;
  return &itemiter->second;

}

//overriden from base for case-insensitive sort
int PWSTreeCtrl::OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
{
  const bool groupsFirst = PWSprefs::GetInstance()->GetPref(PWSprefs::ExplorerTypeTree),
             item1isGroup = ItemHasChildren(item1),
             item2isGroup = ItemHasChildren(item2);

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
  SortChildren(item);
  
  wxTreeItemIdValue cookie;
  for( wxTreeItemId childId = GetFirstChild(item, cookie); childId.IsOk(); childId = GetNextChild(item, cookie)) {
    if (ItemHasChildren(childId)) {
      SortChildrenRecursively(childId);
    }
  }
}

wxTreeItemId PWSTreeCtrl::Find(const CUUID &uuid) const
{
  wxTreeItemId fail;
  UUIDTIMapT::const_iterator iter = m_item_map.find(uuid);
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
  if (ItemHasChildren(item) && GetChildrenCount(item) > 0){
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
    if (ci != NULL)
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
    if (ci != NULL) {
      const wxString note = ci->GetNotes().c_str();
      evt.SetToolTip(note);
    }
  }
}


/*!
 * wxEVT_CHAR event handler for ID_TREECTRL
 */

void PWSTreeCtrl::OnChar( wxKeyEvent& evt )
{
  if (evt.GetKeyCode() == WXK_ESCAPE &&
      PWSprefs::GetInstance()->GetPref(PWSprefs::EscExits)) {
    GetParent()->Close();
  }
  evt.Skip();
}

void PWSTreeCtrl::OnDBGUIPrefsChange(wxEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  PasswordSafeFrame *pwsframe = dynamic_cast<PasswordSafeFrame *>(GetParent());
  wxASSERT(pwsframe != NULL);
  if (pwsframe->IsTreeView())
    pwsframe->RefreshViews();
}
