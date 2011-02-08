
/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file guiinfo.cpp
* 
*/

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "./guiinfo.h"
#include "./passwordsafeframe.h"
#include "./PWStree.h"
#include "./PWSgrid.h"
#include <functional>

void ClearUUID(uuid_array_t uu)
{
  memset(uu, 0, sizeof(uu));
}


void GUIInfo::Save(PasswordSafeFrame* frame)
{
  SaveTreeViewInfo(frame->m_tree);
  SaveGridViewInfo(frame->m_grid);
}

void GUIInfo::Restore(PasswordSafeFrame* frame)
{
  RestoreTreeViewInfo(frame->m_tree);
  RestoreGridViewInfo(frame->m_grid);
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
  else {
    m_treeTop.Clear();
  }
  
  m_expanded.Empty();

  //find out all the expanded groups in a depth-first manner
  wxTreeItemIdValue dummy;
  const wxTreeItemId root = tree->GetRootItem();
  if (root.IsOk()) {
    for ( wxTreeItemId id = tree->GetFirstChild(root, dummy); id.IsOk(); ) {
      if (tree->ItemHasChildren(id) && tree->IsExpanded(id)) {
        m_expanded.Add(tree->GetItemText(id));
        id = tree->GetFirstChild(id, dummy);
      }
      else {
        wxTreeItemId parent = tree->GetItemParent(id);
        for (id = tree->GetNextSibling(id); !id.IsOk() && parent != root; 
                   id = tree->GetNextSibling(parent), parent = tree->GetItemParent(parent))
        {
        }
      }
    }
  }

  //save the selected item
  wxTreeItemId selection = tree->GetSelection();
  if (selection.IsOk() && selection != tree->GetRootItem()) {
    if(tree->HasChildren(selection)) {
      m_treeSelection = tree->GetItemText(selection);
      const wxString selectionStr = m_treeSelection;
      wxASSERT(!selectionStr.IsEmpty());
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
  else {
    m_treeSelection.Clear();
  }
}

void GUIInfo::SaveGridViewInfo(PWSGrid* grid)
{
  const int row = grid->YToRow(0);
  if (row != wxNOT_FOUND) {
    CItemData* item = grid->GetItem(row);
    if (item) {
      item->GetUUID(m_gridTop);
    }
    else {
      wxFAIL_MSG(wxString(wxT("Top grid row ")) << row << wxT(" has no CItemData attached"));
      ClearUUID(m_gridTop);
    }
  }
  else {
    ClearUUID(m_gridTop);
  }

  const int selection = grid->GetGridCursorRow();
  if (selection != wxNOT_FOUND) {
    CItemData* item = grid->GetItem(selection);
    if (item) {
      item->GetUUID(m_gridSelection);
    }
    else {
      wxFAIL_MSG(wxString(wxT("Selected grid row ")) << selection << wxT(" has no CItemData attached"));
      ClearUUID(m_gridSelection);
    }
  }
  else {
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
    grid->SelectRow(selection);
  }
}

template <class TreeFunc, class Predicate>
void VisitGroupItems(PWSTreeCtrl* tree, const Predicate& pred, TreeFunc func, bool visitAll)
{
  wxTreeItemIdValue dummy;
  const wxTreeItemId root = tree->GetRootItem();
  if (root.IsOk()) {
    for ( wxTreeItemId id = tree->GetFirstChild(root, dummy); id.IsOk(); ) {
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
        for (id = tree->GetNextSibling(id); !id.IsOk() && parent != root; 
                 id = tree->GetNextSibling(parent), parent = tree->GetItemParent(parent))
        {
        }
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
    const wxString valStr = val;
    VisitGroupItems(tree, std::bind2nd(std::equal_to<wxString>(), valStr), func, false);
  }
}

struct SelectItem {
  void operator()(PWSTreeCtrl* tree, const wxTreeItemId& id) { 
      tree->wxTreeCtrl::SelectItem(id, true); 
  }
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
  void operator()( PWSTreeCtrl* tree, const wxTreeItemId& id) { 
    tree->Expand(id); 
  }
};

void GUIInfo::RestoreTreeViewInfo(PWSTreeCtrl* tree)
{
  if (m_treeTop.Type() != string_or_uuid::ITEM_NONE)
    RestoreTreeItem(tree, m_treeTop, BringItemToTop());
  if (m_treeSelection.Type() != string_or_uuid::ITEM_NONE)
    RestoreTreeItem(tree, m_treeSelection, SelectItem());
  VisitGroupItems(tree, std::bind2nd(FindInArray(), m_expanded), ExpandItem(), true);
}

