
/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file guiinfo.cpp
* 
*/

#include <wx/wxprec.h>

#include "./guiinfo.h"
#include "./passwordsafeframe.h"
#include "./PWStree.h"
#include "./PWSgrid.h"
#include <functional>


void ClearUUID(uuid_array_t uu)
{
  memset(uu, 0, sizeof(uu));
}


GUIInfo::GUIInfo() : m_viewType(UNKNOWN)
{
}

void GUIInfo::Save(PasswordSafeFrame* frame)
{
  m_viewType = (frame->m_currentView == PasswordSafeFrame::GRID? GRIDVIEW : TREEVIEW);
  m_position = frame->GetRect();

  SaveTreeViewInfo(frame->m_tree);
  SaveGridViewInfo(frame->m_grid);
}

void GUIInfo::Restore(PasswordSafeFrame* frame)
{
  RestoreTreeViewInfo(frame->m_tree);
  RestoreGridViewInfo(frame->m_grid);

  frame->ShowGrid(m_viewType == GRIDVIEW);
  frame->ShowTree(m_viewType == TREEVIEW);
}

void GUIInfo::SaveTreeViewInfo(PWSTreeCtrl* tree)
{
  //save the first visible item
  wxTreeItemId treeItem = tree->GetFirstVisibleItem();
  if (treeItem.IsOk()) {
    CItemData* item = tree->GetItem(treeItem);
    if (item) {
      item->GetUUID(m_treeTop);
    }
    else if (tree->ItemHasChildren(treeItem)) {
      m_treeTop = tree->GetItemText(treeItem);
    }
    else {
      m_treeTop.Clear();
      wxFAIL_MSG(wxString(wxT("Tree item \'")) << tree->GetItemText(treeItem) << wxT("\' found with no children and no CItemData"));
    }
  }
  
  m_expanded.Empty();

  //find out all the expanded groups in a depth-first manner
  wxTreeItemIdValue dummy;
  for ( wxTreeItemId id = tree->GetFirstChild(tree->GetRootItem(), dummy); id.IsOk(); ) {
    if (tree->ItemHasChildren(id) && tree->IsExpanded(id)) {
      m_expanded.Add(tree->GetItemText(id));
      id = tree->GetFirstChild(id, dummy);
    }
    else {
      wxTreeItemId parent = tree->GetItemParent(id);
      for (id = tree->GetNextSibling(id); !id.IsOk() && parent != tree->GetRootItem(); 
                 id = tree->GetNextSibling(parent), parent = tree->GetItemParent(parent))
      {
      }
    }
  }

  //save the selected item
  wxTreeItemId selection = tree->GetSelection();
  if (tree->HasChildren(selection)) {
    m_treeSelection = tree->GetItemText(selection);
    wxASSERT(!wxString(m_treeSelection).IsEmpty());
  }
  else {
    CItemData* item = tree->GetItem(selection);
    if (item) {
      item->GetUUID(m_treeSelection);
    }
    else {
      m_treeSelection.Clear();
      wxFAIL_MSG(wxString(wxT("tree item \'")) << tree->GetItemText(selection) << wxT("\' found with no CItemData attached"));
    }
  }
}

void GUIInfo::SaveGridViewInfo(PWSGrid* grid)
{
  int row = grid->YToRow(0);
  CItemData* item = grid->GetItem(row);
  if (item) {
    item->GetUUID(m_gridTop);
  }
  else {
    wxFAIL_MSG(wxString(wxT("Top grid row ")) << row << wxT(" has no CItemData attached"));
    ClearUUID(m_gridTop);
  }

  int selection = grid->GetGridCursorRow();
  item = grid->GetItem(selection);
  if (item) {
    item->GetUUID(m_gridSelection);
  }
  else {
    wxFAIL_MSG(wxString(wxT("Selected grid row ")) << selection << wxT(" has no CItemData attached"));
    ClearUUID(m_gridSelection);
  }
}

void GUIInfo::RestoreGridViewInfo(PWSGrid* grid)
{
  const int top = grid->FindItemRow(m_gridTop);
  if (top != wxNOT_FOUND)
    grid->MakeCellVisible(top, 0);

  const int selection = grid->FindItemRow(m_gridSelection);
  if (selection != wxNOT_FOUND) {
#if wxCHECK_VERSION(2, 9, 0)
    grid->GoToCell(selection, 0);
#else
    grid->MakeCellVisible(selection, 0);
    grid->SetGridCursor(selection, 0);
#endif
  }
}

template <class TreeFunc, class Predicate>
void VisitGroupItems(PWSTreeCtrl* tree, const Predicate& pred, TreeFunc func, bool visitAll)
{
  wxTreeItemIdValue dummy;
  for ( wxTreeItemId id = tree->GetFirstChild(tree->GetRootItem(), dummy); id.IsOk(); ) {
    if (tree->ItemHasChildren(id)) {
      if (pred(tree->GetItemText(id))) {
        func(tree, id); 
        if (!visitAll)
          break;
      }
      id = tree->GetFirstChild(id, dummy);
    }
    else {
      wxTreeItemId parent = tree->GetItemParent(id);
      for (id = tree->GetNextSibling(id); !id.IsOk() && parent != tree->GetRootItem(); 
                 id = tree->GetNextSibling(parent), parent = tree->GetItemParent(parent))
      {
      }
    }
  }
}

template <class TreeFunc>
void RestoreTreeItem(PWSTreeCtrl* tree, const string_or_uuid& val, TreeFunc func)
{
  if (val.Type() == string_or_uuid::ITEM_NORMAL) {
    wxTreeItemId id = tree->Find(val);
    if (id.IsOk())
      func(tree, id); 
  }
  else {
    VisitGroupItems(tree, std::bind2nd(std::equal_to<wxString>(), val), func, false);
  }
}

struct SelectItem {
  void operator()(PWSTreeCtrl* tree, const wxTreeItemId& id) { tree->wxTreeCtrl::SelectItem(id, true); }
};

struct BringItemToTop {
  //best I can do right now to bring it to top
  void operator()(PWSTreeCtrl* tree, const wxTreeItemId& id) { tree->ScrollTo(id); }
};

struct FindInArray : public std::binary_function<wxString, wxArrayString, bool>{
  bool operator() (const wxString& val, const wxArrayString& as) const {
    return as.Index(val) != wxNOT_FOUND;
  }
};

struct ExpandItem {
  void operator()( PWSTreeCtrl* tree, const wxTreeItemId& id) { tree->Expand(id); }
};

void GUIInfo::RestoreTreeViewInfo(PWSTreeCtrl* tree)
{
  RestoreTreeItem(tree, m_treeTop, BringItemToTop());
  RestoreTreeItem(tree, m_treeSelection, SelectItem());
  VisitGroupItems(tree, std::bind2nd(FindInArray(), m_expanded), ExpandItem(), true);
}

