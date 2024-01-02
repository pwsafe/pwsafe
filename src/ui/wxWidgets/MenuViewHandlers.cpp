/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include "core/StringX.h"

#include "DragBarCtrl.h"
#include "GridCtrl.h"
#include "PasswordSafeFrame.h"
#include "PasswordSafeSearch.h"
#include "TreeCtrl.h"
#include "ViewReportDlg.h"
#include "core/PWSFilters.h"
#include "SetFiltersDlg.h"
#include "ManageFiltersDlg.h"

void PasswordSafeFrame::OnChangeToolbarType(wxCommandEvent& evt)
{
  //This assumes the menu item is checked before it comes here
  if (GetMenuBar()->IsChecked(evt.GetId())) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, evt.GetId() == ID_TOOLBAR_NEW);
    UpdateMainToolbarBitmaps();

    auto dragbar = GetDragBar();
    wxCHECK_RET(dragbar, wxT("Could not find dragbar"));
    dragbar->UpdateBitmaps();

    auto searchbar = GetSearchBar();
    wxCHECK_RET(searchbar, wxT("Search object not created as expected"));
    searchbar->UpdateBitmaps();

    DoLayout();
    SendSizeEvent();
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
    ShowModalAndGetResult<ViewReportDlg>(this, &rpt, true);
  }
  else {
    wxString tcAction = CReport::ReportNames.find(iAction)->second;
    wxMessageBox(_(tcAction) + _(L" file \'") + rpt.GetFileName() + _(L"' not readable"), _("View Report"), wxOK|wxICON_ERROR);
  }
}

void PasswordSafeFrame::OnShowReportSynchronize(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTSYNCH);
}

void PasswordSafeFrame::OnShowReportCompare(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTCOMPARE);
}

void PasswordSafeFrame::OnShowReportMerge(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTMERGE);
}

void PasswordSafeFrame::OnShowReportImportText(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTIMPORTTEXT);
}

void PasswordSafeFrame::OnShowReportImportXML(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTIMPORTXML);
}

void PasswordSafeFrame::OnShowReportImportKeePassV1_TXT(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTIMPORTKPV1TXT);
}

void PasswordSafeFrame::OnShowReportImportKeePassV1_CSV(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTIMPORTKPV1CSV);
}

void PasswordSafeFrame::OnShowReportExportText(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTEXPORTTEXT);
}

void PasswordSafeFrame::OnShowReportExportXML(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTEXPORTXML);
}

void PasswordSafeFrame::OnShowReportExportDB(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTEXPORTDB);
}

void PasswordSafeFrame::OnShowReportFind(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTFIND);
}

void PasswordSafeFrame::OnShowReportValidate(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::RunShowReport, IDSC_RPTVALIDATE);
}

void PasswordSafeFrame::OnShowHideToolBar(wxCommandEvent& evt)
{
  GetMainToolbarPane().Show(evt.IsChecked());
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowToolbar, evt.IsChecked());
  m_AuiManager.Update();
  DoLayout();
  SendSizeEvent();
  SetFocus();
}

void PasswordSafeFrame::OnShowHideDragBar(wxCommandEvent& evt)
{
  GetDragBarPane().Show(evt.IsChecked());
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowDragbar, evt.IsChecked());
  m_AuiManager.Update();
  DoLayout();
  SendSizeEvent();
  SetFocus();
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
  // Update and setting of filter state is not needed here, as update is done at the end of RefreshView()
  RefreshViews();
}

void PasswordSafeFrame::OnEditFilter(wxCommandEvent& )
{
  CallAfter(&PasswordSafeFrame::DoEditFilter);
}

void PasswordSafeFrame::DoEditFilter()
{
  st_filters filters(CurrentFilter());
  bool bCanHaveAttachments = m_core.GetNumAtts() > 0;
  const std::set<StringX> sMediaTypes = m_core.GetAllMediaTypes();
  bool bAppliedCalled = false;
  stringT oldName = CurrentFilter().fname;
  st_Filterkey fk;
  bool bDoEdit = true;
  
  while(bDoEdit) {
    bDoEdit = false; // In formal case we run only one time in the loop; but when removal of double entry is requested we avoid goto

    int rc = ShowModalAndGetResult<SetFiltersDlg>(this, &filters, &CurrentFilter(), &bAppliedCalled, DFTYPE_MAIN, FPOOL_SESSION, bCanHaveAttachments, &sMediaTypes);;
  
    if (rc == wxID_OK || (rc == wxID_CANCEL && bAppliedCalled && !IsCloseInProgress())) {
      // User can apply the filter in SetFiltersDlg and then press Cancel button
      // and afterwards process changes, update only on OK button and continue with actualized version
      if(rc == wxID_OK) {
        CurrentFilter().Empty();
        CurrentFilter() = filters;
      }
      
      // Update pre-defined filter, if present
      wxMenuBar* menuBar = GetMenuBar();
      if(CurrentFilter() == m_FilterManager.GetExpireFilter()) {
        m_CurrentPredefinedFilter = EXPIRY;
        menuBar->Check(ID_SHOW_ALL_EXPIRY, true);
        menuBar->Check(ID_SHOWHIDE_UNSAVED, false);
        menuBar->Check(ID_SHOW_LAST_FIND_RESULTS, false);
      } else if(CurrentFilter() == m_FilterManager.GetUnsavedFilter()) {
        m_CurrentPredefinedFilter = UNSAVED;
        menuBar->Check(ID_SHOW_ALL_EXPIRY, false);
        menuBar->Check(ID_SHOWHIDE_UNSAVED, true);
        menuBar->Check(ID_SHOW_LAST_FIND_RESULTS, false);
      } else if(CurrentFilter() == m_FilterManager.GetFoundFilter()) {
        m_CurrentPredefinedFilter = LASTFIND;
        menuBar->Check(ID_SHOW_ALL_EXPIRY, false);
        menuBar->Check(ID_SHOWHIDE_UNSAVED, false);
        menuBar->Check(ID_SHOW_LAST_FIND_RESULTS, true);
      } else {
        m_CurrentPredefinedFilter = NONE;
        menuBar->Check(ID_SHOW_ALL_EXPIRY, false);
        menuBar->Check(ID_SHOWHIDE_UNSAVED, false);
        menuBar->Check(ID_SHOW_LAST_FIND_RESULTS, false);
      }
    
      // Update filter in Filters map
      fk.fpool = FPOOL_SESSION;
      fk.cs_filtername = CurrentFilter().fname;
    
      PWSFilters::iterator mf_iter = m_MapAllFilters.find(fk);
      // When entry already in map and had been new or name changed
      if(mf_iter != m_MapAllFilters.end() && (oldName.empty() || (oldName != CurrentFilter().fname) || (m_currentfilterpool != FPOOL_SESSION))) {
        wxMessageDialog dialog(this, _("This filter already exists"), _("Do you wish to replace it?"), wxYES_NO | wxICON_EXCLAMATION);
        if(dialog.ShowModal() == wxID_NO) {
          bDoEdit = true; // Repeat editing to allow name change
          continue;
        }
      }
    }
    // Erase entry and add again with actual content
    m_MapAllFilters.erase(fk);
    m_MapAllFilters.insert(PWSFilters::Pair(fk, CurrentFilter()));

    m_currentfilterpool = fk.fpool;
    m_selectedfiltername = fk.cs_filtername.c_str();
    
    // If filters currently active - update and re-apply
    if (m_bFilterActive) {
      m_bFilterActive = CurrentFilter().IsActive();
      ApplyFilters();
    }
  }
}

void PasswordSafeFrame::OnApplyFilter(wxCommandEvent& event)
{
  wxMenuBar* menuBar = GetMenuBar();
  wxString par = event.GetString(); // From Edit/New filter dialog a string is set
  
  // Toggle filter Apply / Clear, when not called from Edit/New filter -> while perform apply always
  if(m_bFilterActive && par.IsEmpty()) {
    m_bFilterActive = false;
    if(m_CurrentPredefinedFilter == EXPIRY) {
      menuBar->Check(ID_SHOW_ALL_EXPIRY, false);
      CurrentFilter().Empty();
    } else if (m_CurrentPredefinedFilter == UNSAVED) {
      menuBar->Check(ID_SHOWHIDE_UNSAVED, false);
      CurrentFilter().Empty();
    } else if (m_CurrentPredefinedFilter == LASTFIND) {
      menuBar->Check(ID_SHOW_LAST_FIND_RESULTS, false);
      CurrentFilter().Empty();
    }
    m_CurrentPredefinedFilter = NONE;
    m_ApplyClearFilter->SetItemLabel(_("&Apply current"));
  }
  else {
    m_bFilterActive = CurrentFilter().IsActive();
    m_ApplyClearFilter->SetItemLabel(_("&Clear current"));
    if(par.CompareTo(pwManageFiltersTable::getSourcePoolLabel(FPOOL_DATABASE).c_str()) == 0) {
      m_currentfilterpool = FPOOL_DATABASE;
    }
    else if(par.CompareTo(pwManageFiltersTable::getSourcePoolLabel(FPOOL_AUTOLOAD).c_str()) == 0) {
      m_currentfilterpool = FPOOL_AUTOLOAD;
    }
    else if(par.CompareTo(pwManageFiltersTable::getSourcePoolLabel(FPOOL_IMPORTED).c_str()) == 0) {
      m_currentfilterpool = FPOOL_IMPORTED;
    }
    else if(par.CompareTo(pwManageFiltersTable::getSourcePoolLabel(FPOOL_SESSION).c_str()) == 0) {
      m_currentfilterpool = FPOOL_SESSION;
    }
    m_selectedfiltername = CurrentFilter().fname;
  }
  // Update menu
  menuBar->Refresh();
  // Update shown items
  ApplyFilters();
  // If Apply called from Edit/New Filter or Manage Filter expand tree view
  if(! par.IsEmpty()) {
    if(IsTreeView() && !m_tree->IsEmpty()) {
      m_tree->ExpandAll();
    }
  }
}

// functor for Copy subset of map entries back to the database
struct CopyDBFilters {
  CopyDBFilters(PWSFilters &core_mapFilters) :
  m_CoreMapFilters(core_mapFilters)
  {}

  // operator
  void operator()(std::pair<const st_Filterkey, st_filters> p)
  {
    m_CoreMapFilters.insert(PWSFilters::Pair(p.first, p.second));
  }

private:
  PWSFilters &m_CoreMapFilters;
};

void PasswordSafeFrame::OnManageFilters(wxCommandEvent& )
{
  CallAfter(&PasswordSafeFrame::DoManageFilters);
}

void PasswordSafeFrame::DoManageFilters()
{
  st_Filterkey fkl, fku;
  PWSFilters::iterator mf_iter, mf_lower_iter, mf_upper_iter;
  
  // Search range for data base filters
  fkl.fpool = FPOOL_DATABASE;
  fkl.cs_filtername = L"";
  fku.fpool = (FilterPool)((int)FPOOL_DATABASE + 1);
  fku.cs_filtername = L"";
  // Find & delete DB filters only
  if(!m_MapAllFilters.empty()) {
    mf_lower_iter = m_MapAllFilters.lower_bound(fkl);
    // Check that there are some first!
    if (mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      mf_upper_iter = m_MapAllFilters.upper_bound(fku);
      // Delete existing database filters (if any)
      m_MapAllFilters.erase(mf_lower_iter, mf_upper_iter);
    }
  }

  // Get current core filters
  PWSFilters core_filters = m_core.GetDBFilters();
  const PWSFilters original_core_filters = m_core.GetDBFilters();

  // Now add any existing database filters
  for(mf_iter = core_filters.begin();
      mf_iter != core_filters.end(); mf_iter++) {
    m_MapAllFilters.insert(PWSFilters::Pair(mf_iter->first, mf_iter->second));
  }

  bool bCanHaveAttachments = m_core.GetNumAtts() > 0;
  const std::set<StringX> sMediaTypes = m_core.GetAllMediaTypes();

  int rc = ShowModalAndGetResult<ManageFiltersDlg>(this, &m_core, m_MapAllFilters, &CurrentFilter(), &m_currentfilterpool, &m_selectedfiltername, &m_bFilterActive, bCanHaveAttachments, &sMediaTypes, m_core.IsReadOnly());;
  
  // No change in DB filter when return ID_CANCEL
  if(rc == wxID_CANCEL)
    return;
  
  // Clear core filters ready to replace with new ones
  core_filters.clear();

  // Get DB filters populated via CManageFiltersDlg
  if(!m_MapAllFilters.empty()) {
    mf_lower_iter = m_MapAllFilters.lower_bound(fkl);

    // Check that there are some first!
    if(mf_lower_iter->first.fpool == FPOOL_DATABASE) {
      // Now find upper bound of database filters
      mf_upper_iter = m_MapAllFilters.upper_bound(fku);

      // Copy database filters (if any) to the core
      CopyDBFilters copy_db_filters(core_filters);
      for_each(mf_lower_iter, mf_upper_iter, copy_db_filters);
    }
  }
   
  // However, we need to check as user may have edited the filter more than once
  // and reverted any changes!
  if(core_filters != original_core_filters) {
    // Now update DB filters in core
    Command *pcmd = DBFiltersCommand::Create(&m_core, core_filters);

    // Do it
    Execute(pcmd);
  }
}
