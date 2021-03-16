/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file MenuViewHandlers.cpp
 *  This file contains implementations of PasswordSafeFrame
 *  member functions corresponding to actions under the 'View'
 *  menubar menu.
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "core/core.h"
#include "core/PWSprefs.h"

#include "DragBarCtrl.h"
#include "GridCtrl.h"
#include "PasswordSafeFrame.h"
#include "PasswordSafeSearch.h"
#include "TreeCtrl.h"
#include "ViewReportDlg.h"

void PasswordSafeFrame::OnChangeToolbarType(wxCommandEvent& evt)
{
  //This assumes the menu item is checked before it comes here
  if (GetMenuBar()->IsChecked(evt.GetId())) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, evt.GetId() == ID_TOOLBAR_NEW);
    RefreshToolbarButtons();
    DragBarCtrl* dragbar = GetDragBar();
    wxCHECK_RET(dragbar, wxT("Could not find dragbar"));
    dragbar->RefreshButtons();
    wxCHECK_RET(m_search, wxT("Search object not created as expected"));
    m_search->RefreshButtons();
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_LIST_VIEW
 */

void PasswordSafeFrame::OnListViewClick(wxCommandEvent& WXUNUSED(evt))
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView, _T("list"));

  // Unregister the active view at core to not get notifications anymore
  m_core.UnregisterObserver(m_tree);

  ShowTree(false);
  ShowGrid(true);
  SetViewType(ViewType::GRID);

  // Register view at core as new observer for notifications
  m_core.RegisterObserver(m_grid);
  
  UpdateTreeSortMenu();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_TREE_VIEW
 */

void PasswordSafeFrame::OnTreeViewClick(wxCommandEvent& WXUNUSED(evt))
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView, _T("tree"));

  // Unregister the active view at core to not get notifications anymore
  m_core.UnregisterObserver(m_grid);

  ShowGrid(false);
  ShowTree(true);
  SetViewType(ViewType::TREE);

  // Register view at core as new observer for notifications
  m_core.RegisterObserver(m_tree);
  
  UpdateTreeSortMenu();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SORT_TREE_BY_GROUP
 */

void PasswordSafeFrame::OnSortByGroupClick(wxCommandEvent& WXUNUSED(evt))
{
  const TreeSortType oldSortType = m_currentSort;
  
  PWSprefs::GetInstance()->SetPref(PWSprefs::TreeSort, _T("group"));
  SetTreeSortType(TreeSortType::GROUP);
  UpdateTreeSortMenu();
  m_tree->SetSortingGroup();
  m_tree->SetShowGroup(false);
  if (oldSortType != m_currentSort) {
    ShowTree(IsTreeView());
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SORT_TREE_BY_NAME
 */

void PasswordSafeFrame::OnSortByNameClick(wxCommandEvent& WXUNUSED(evt))
{
  const TreeSortType oldSortType = m_currentSort;
  
  PWSprefs::GetInstance()->SetPref(PWSprefs::TreeSort, _T("name"));
  SetTreeSortType(TreeSortType::NAME);
  UpdateTreeSortMenu();
  m_tree->SetSortingName();
  m_tree->SetShowGroup(true);
  if (oldSortType != m_currentSort) {
    ShowTree(IsTreeView());
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SORT_TREE_BY_DATE
 */

void PasswordSafeFrame::OnSortByDateClick(wxCommandEvent& WXUNUSED(evt))
{
  const TreeSortType oldSortType = m_currentSort;
  
  PWSprefs::GetInstance()->SetPref(PWSprefs::TreeSort, _T("date"));
  SetTreeSortType(TreeSortType::DATE);
  UpdateTreeSortMenu();
  m_tree->SetSortingDate();
  m_tree->SetShowGroup(true);
  if (oldSortType != m_currentSort) {
    ShowTree(IsTreeView());
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SHOW_EMPTY_GROUP_IN_FILTER
 */

void PasswordSafeFrame::OnShowGroupInFilterClick(wxCommandEvent& WXUNUSED(evt))
{
  m_bShowEmptyGroupsInFilter = !m_bShowEmptyGroupsInFilter; // Toggle value
  GetMenuBar()->Check(ID_SHOW_EMPTY_GROUP_IN_FILTER, m_bShowEmptyGroupsInFilter);
  if(IsTreeView())
    ShowTree();
}
void PasswordSafeFrame::OnExpandAll(wxCommandEvent& WXUNUSED(evt))
{
  wxASSERT(IsTreeView());

  if (!m_tree->IsEmpty()) {
    m_tree->ExpandAll();
  }
}

void PasswordSafeFrame::OnCollapseAll(wxCommandEvent& WXUNUSED(evt))
{
  wxASSERT(IsTreeView());

  if (m_tree->IsEmpty()) {
    return;
  }

  //we cannot just call wxTreeCtrl::CollapseAll(), since it tries to
  //collapse the invisible root item also, and thus ASSERTs
  wxTreeItemIdValue cookie;
  for ( wxTreeItemId root = m_tree->GetRootItem(), idCurr = m_tree->GetFirstChild(root, cookie);
        idCurr.IsOk();
        idCurr = m_tree->GetNextChild(root, cookie) )
  {
      m_tree->CollapseAllChildren(idCurr);
  }
}

void PasswordSafeFrame::OnChangeTreeFont(wxCommandEvent& WXUNUSED(evt))
{
  ChangeFontPreference(PWSprefs::TreeFont);
}

void PasswordSafeFrame::OnChangeAddEditFont(wxCommandEvent& WXUNUSED(evt))
{
  ChangeFontPreference(PWSprefs::AddEditFont);
}

void PasswordSafeFrame::OnChangePasswordFont(wxCommandEvent& WXUNUSED(evt))
{
  ChangeFontPreference(PWSprefs::PasswordFont);
}

void PasswordSafeFrame::OnChangeNotesFont(wxCommandEvent& WXUNUSED(evt))
{
  ChangeFontPreference(PWSprefs::NotesFont);
}

void PasswordSafeFrame::OnChangeVirtualKeyboardFont(wxCommandEvent& WXUNUSED(evt))
{
  ChangeFontPreference(PWSprefs::VKeyboardFontName);
}

void PasswordSafeFrame::RunShowReport(int iAction)
{
  CReport rpt;
  rpt.StartReport(iAction, m_core.GetCurFile().c_str(), false);
  if(rpt.ReadFromDisk()) {
    ViewReportDlg dlg(this, &rpt, true);
    dlg.ShowModal();
  }
  else {
    wxString tcAction = CReport::ReportNames.find(iAction)->second;
    wxMessageBox(_(tcAction) + _(L" file \'") + rpt.GetFileName() + _(L"' not readable"), _("View Report"), wxOK|wxICON_ERROR);
  }
}

void PasswordSafeFrame::OnShowReportSynchronize(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTSYNCH);
}

void PasswordSafeFrame::OnShowReportCompare(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTCOMPARE);
}

void PasswordSafeFrame::OnShowReportMerge(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTMERGE);
}

void PasswordSafeFrame::OnShowReportImportText(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTIMPORTTEXT);
}

void PasswordSafeFrame::OnShowReportImportXML(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTIMPORTXML);
}

void PasswordSafeFrame::OnShowReportImportKeePassV1_TXT(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTIMPORTKPV1TXT);
}

void PasswordSafeFrame::OnShowReportImportKeePassV1_CSV(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTIMPORTKPV1CSV);
}

void PasswordSafeFrame::OnShowReportExportText(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTEXPORTTEXT);
}

void PasswordSafeFrame::OnShowReportExportXML(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTEXPORTXML);
}

void PasswordSafeFrame::OnShowReportExportDB(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTEXPORTDB);
}

void PasswordSafeFrame::OnShowReportFind(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTFIND);
}

void PasswordSafeFrame::OnShowReportValidate(wxCommandEvent& WXUNUSED(evt))
{
  RunShowReport(IDSC_RPTVALIDATE);
}

void PasswordSafeFrame::OnShowHideToolBar(wxCommandEvent& evt)
{
  GetToolBar()->Show(evt.IsChecked());
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowToolbar, evt.IsChecked());
  DoLayout();
  SendSizeEvent();
}

void PasswordSafeFrame::OnShowHideDragBar(wxCommandEvent& evt)
{
  DragBarCtrl* dragbar = GetDragBar();
  wxCHECK_RET(dragbar, wxT("Could not find dragbar"));

  dragbar->Show(evt.IsChecked());
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowDragbar, evt.IsChecked());
  DoLayout();
}

//-----------------------------------------------------------------
// Filters related
// (Starting with predefined filters)
//-----------------------------------------------------------------
void PasswordSafeFrame::OnShowAllExpiryClick( wxCommandEvent& event )
{
  if (!(m_CurrentPredefinedFilter == NONE || m_CurrentPredefinedFilter == EXPIRY))
    return; // should be disabled - we support only one predefined at a time

  bool showExpiry = event.IsChecked();
  m_CurrentPredefinedFilter = showExpiry ? EXPIRY : NONE;
  m_bFilterActive = showExpiry;
  if (showExpiry) {
    CurrentFilter() = m_FilterManager.GetExpireFilter();
    // Entries with Expiry date iterates on entries only
    m_bShowEmptyGroupsInFilter = false;
  } else {
    CurrentFilter().Empty();
    // Set back to default value at end of filter
    m_bShowEmptyGroupsInFilter = false;
  }
  GetMenuBar()->Check(ID_SHOW_EMPTY_GROUP_IN_FILTER, m_bShowEmptyGroupsInFilter);
  GetMenuBar()->Refresh();
  ApplyFilters();
}

void PasswordSafeFrame::OnShowUnsavedEntriesClick( wxCommandEvent& event )
{
  if (!(m_CurrentPredefinedFilter == NONE || m_CurrentPredefinedFilter == UNSAVED))
    return; // should be disabled - we support only one predefined at a time

  bool showUnsaved = event.IsChecked();
  m_CurrentPredefinedFilter = showUnsaved ? UNSAVED : NONE;

  m_bFilterActive = showUnsaved;
  if (showUnsaved) {
    CurrentFilter() = m_FilterManager.GetUnsavedFilter();
    // Unsaved Entries might include groups, set show groups by default
    m_bShowEmptyGroupsInFilter = false;
  } else {
    CurrentFilter().Empty();
    // Set back to default value at end of filter
    m_bShowEmptyGroupsInFilter = false;
  }
  GetMenuBar()->Check(ID_SHOW_EMPTY_GROUP_IN_FILTER, m_bShowEmptyGroupsInFilter);
  GetMenuBar()->Refresh();
  ApplyFilters();
}

void PasswordSafeFrame::OnShowLastFindClick( wxCommandEvent& event )
{
  if (!(m_CurrentPredefinedFilter == NONE || m_CurrentPredefinedFilter == LASTFIND))
    return; // should be disabled - we support only one predefined at a time

  bool showLastFind = event.IsChecked();
  m_CurrentPredefinedFilter = showLastFind ? LASTFIND : NONE;
  m_FilterManager.SetFindFilter(showLastFind);

  m_bFilterActive = showLastFind;
  if (showLastFind) {
    CurrentFilter() = m_FilterManager.GetFoundFilter();
    // Last Find iterates on entries only
    m_bShowEmptyGroupsInFilter = false;
  } else {
    CurrentFilter().Empty();
    // Set back to default value at end of filter
    m_bShowEmptyGroupsInFilter = false;
  }
  GetMenuBar()->Check(ID_SHOW_EMPTY_GROUP_IN_FILTER, m_bShowEmptyGroupsInFilter);
  GetMenuBar()->Refresh();
  ApplyFilters();
}

void PasswordSafeFrame::ApplyFilters()
{
  m_FilterManager.CreateGroups();
  m_tree->SetFilterState(m_bFilterActive);
  m_grid->SetFilterState(m_bFilterActive);
  RefreshViews();
}

void PasswordSafeFrame::OnEditFilter(wxCommandEvent& )
{

}

void PasswordSafeFrame::OnApplyFilter(wxCommandEvent& )
{

}

void PasswordSafeFrame::OnManageFilters(wxCommandEvent& )
{

}
