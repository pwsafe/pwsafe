
/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

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

void CollectExpandedNodes(PWSTreeCtrl* tree, wxTreeItemId root, wxArrayString& expanded)
{
  if ( !tree || tree->GetCount() == 0 )
    return;
  
  wxTreeItemIdValue cookie;
  for( wxTreeItemId id = tree->GetFirstChild(root, cookie); id.IsOk(); id = tree->GetNextChild(root, cookie))
  {
    if (tree->ItemHasChildren(id) && tree->IsExpanded(id)) {
      expanded.Add(tree->GetItemGroup(id));
      CollectExpandedNodes(tree, id, expanded);
    }
  }
}
      
void GUIInfo::SaveTreeViewInfo(PWSTreeCtrl* tree)
{
  //save the first visible item
  wxTreeItemId treeItem = tree->GetFirstVisibleItem();
  if (treeItem.IsOk() && treeItem != tree->GetRootItem()) {
    CItemData* item = tree->GetItem(treeItem);
    if (item) {
      m_treeTop = item->GetUUID();
    }
    else if (tree->ItemIsGroup(treeItem)) {
      m_treeTop = tree->GetItemGroup(treeItem);
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
  CollectExpandedNodes(tree, tree->GetRootItem(), m_expanded);
 
  //save the selected item
  wxTreeItemId selection = tree->GetSelection();
  if (selection.IsOk() && selection != tree->GetRootItem()) {
    if(tree->ItemIsGroup(selection)) {
      m_treeSelection = tree->GetItemGroup(selection);
      const wxString selectionStr = m_treeSelection;
      wxASSERT(!selectionStr.IsEmpty());
    }
    else {
      CItemData* item = tree->GetItem(selection);
      if (item) {
        m_treeSelection = item->GetUUID();
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
  //has the grid been initialized?
  if (grid->GetNumItems() == 0)
    return;

  const int row = grid->YToRow(0);
  if (row != wxNOT_FOUND) {
    CItemData* item = grid->GetItem(row);
    if (item) {
      m_gridTop = item->GetUUID();
    }
    else {
      wxFAIL_MSG(wxString(wxT("Top grid row ")) << row << wxT(" has no CItemData attached"));
      m_gridTop = pws_os::CUUID::NullUUID();
    }
  }
  else {
    m_gridTop = pws_os::CUUID::NullUUID();
  }

  const int selection = grid->GetGridCursorRow();
  if (selection != wxNOT_FOUND) {
    CItemData* item = grid->GetItem(selection);
    if (item) {
      m_gridSelection = item->GetUUID();
    }
    else {
      wxFAIL_MSG(wxString(wxT("Selected grid row ")) << selection << wxT(" has no CItemData attached"));
      m_gridSelection = pws_os::CUUID::NullUUID();
    }
  }
  else {
    m_gridSelection = pws_os::CUUID::NullUUID();
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

template <class TreeFunc>
void RestoreTreeItem(PWSTreeCtrl* tree, const string_or_uuid& val, TreeFunc func)
{
  wxTreeItemId id;
  switch(val.Type()) {
    case string_or_uuid::ItemType::NORMAL:
      id = tree->Find(static_cast<pws_os::CUUID> (val));
      break;
    case string_or_uuid::ItemType::GROUP:
      id = tree->Find(static_cast<wxString>(val), tree->GetRootItem());
      break;
    default:
      break;
  }
  if (id.IsOk())
    func(tree, id);
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

void GUIInfo::RestoreTreeViewInfo(PWSTreeCtrl* tree)
{
  // We do this first to ensure that item selection and scrolling an item to top are not wasted by this
  for (size_t idx = 0; idx < m_expanded.Count(); ++idx) {
    wxTreeItemId group = tree->Find(m_expanded[idx], tree->GetRootItem());
    if (group.IsOk()) {
      tree->Expand(group);
    }
    else {
      // It is possible that the group with single item was deleted when item was moved to another group
      // We need to prevent that from happening.  But for now, it will assert too much
      wxLogDebug( wxString(wxT("Could not find group \"")) << m_expanded[idx] << wxT("\" to expand") );
    }
  }

  // Then restore the "Top" item
  RestoreTreeItem(tree, m_treeTop, BringItemToTop());
  
  // Finally select the previously "selected" item
  RestoreTreeItem(tree, m_treeSelection, SelectItem());
}
