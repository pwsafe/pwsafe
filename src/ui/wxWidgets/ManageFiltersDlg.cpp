/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManageFiltersDlg.cpp
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
#include <wx/gbsizer.h>
#include <wx/grid.h>
#include <wx/filename.h>

#include "core/PWSprefs.h"
#include "core/PWScore.h"
#include "core/StringX.h"
#include "core/PWSprefs.h"

#include "wxUtilities.h"
#include "PasswordSafeFrame.h"
#include "PWFiltersEditor.h"
#include "PWFiltersGrid.h"
#include "ManageFiltersTable.h"
#include "ManageFiltersDlg.h"
#include "SetFiltersDlg.h"
#include "wxMessages.h"
////@end includes

////@begin XPM images
////@end XPM images

/*!
 * ManageFiltersGrid type definition
 */

IMPLEMENT_CLASS( ManageFiltersGrid, wxGrid )

/*!
 * ManageFiltersGrid event table definition
 */
DEFINE_EVENT_TYPE(EVT_SELECT_MANAGE_GRID_ROW)

BEGIN_EVENT_TABLE( ManageFiltersGrid, wxGrid )

////@begin ManageFiltersGrid event table entries
#if wxCHECK_VERSION(3, 1, 5)
  EVT_GRID_RANGE_SELECTED(ManageFiltersGrid::OnGridRangeSelect)
#else
  EVT_GRID_RANGE_SELECT(ManageFiltersGrid::OnGridRangeSelect)
#endif
  EVT_COMMAND(wxID_ANY, EVT_SELECT_MANAGE_GRID_ROW, ManageFiltersGrid::OnAutoSelectGridRow)
  EVT_CHAR_HOOK(ManageFiltersGrid::OnChar)
////@end ManageFiltersGrid event table entries

END_EVENT_TABLE()

ManageFiltersGrid::ManageFiltersGrid(wxWindow* parent, pwSortedManageFilters *data, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : m_pMapFilterData(data)
{
  Init();
  Create(parent, id, pos, size, style);
#ifdef PW_FILTERS_GIRD_USE_NATIVE_HEADER
  //column picker is free if built with wx2.9.1
#if wxCHECK_VERSION(2, 9, 1)
  UseNativeColHeader(true);
#endif
#endif
}

void ManageFiltersGrid::BindEvents()
{
#ifdef PW_FILTERS_GIRD_USE_NATIVE_HEADER
#if wxCHECK_VERSION(2, 9, 1)
  wxHeaderCtrl *header = wxGrid::GetGridColHeader();
  if (header) {
    // Handler for double click events on column header separator.
    header->Bind(wxEVT_HEADER_SEPARATOR_DCLICK,
      [=](wxHeaderCtrlEvent& event) {
        wxGrid::AutoSizeColumn(event.GetColumn());
      }
    );
    // Handler for single click events on column header.
    header->Bind(wxEVT_HEADER_CLICK, &ManageFiltersGrid::OnHeaderClick, this);
  }
#endif
#endif
}

void ManageFiltersGrid::OnHeaderClick(wxHeaderCtrlEvent& event)
{
  bool bSort = true;
  if(m_pMapFilterData->GetSortingColumn() != event.GetColumn())
    bSort = true; // Start with ascending when column change
  else
    bSort = !m_pMapFilterData->GetAscending();
  SortGrid(event.GetColumn(), bSort);
}

void ManageFiltersGrid::SortGrid(int column, bool ascending)
{
  if(m_pMapFilterData->IsFilterSelected()) {
    ClearSelection();   // No specific clear of row, due to error in wxWidgets 3.1.4
  }
  m_pMapFilterData->SortByColumn(column, ascending);
  SetSortingColumn(m_pMapFilterData->GetSortingColumn(), m_pMapFilterData->GetAscending());
  if(m_pMapFilterData->IsFilterSelected()) {
    SelectRow(m_pMapFilterData->GetSelectedFilterIdx());
    if(! IsVisible(m_pMapFilterData->GetSelectedFilterIdx(), m_pMapFilterData->GetSortingColumn()))
      MakeCellVisible(m_pMapFilterData->GetSelectedFilterIdx(), m_pMapFilterData->GetSortingColumn());
  }
  ForceRefresh();
}

void ManageFiltersGrid::SortRefresh()
{
  SortGrid(m_pMapFilterData->GetSortingColumn(), m_pMapFilterData->GetAscending());
}


void ManageFiltersGrid::OnGridRangeSelect(wxGridRangeSelectEvent& evt)
{
  //select grids asynchronously, or else we get in an infinite loop of selections & their notifications
  if (evt.Selecting() && m_pMapFilterData->IsFilterSelected() && evt.GetTopRow() != evt.GetBottomRow() && m_pMapFilterData->GetSelectedFilterIdx() != evt.GetTopRow()) { // More than one row selected
    wxCommandEvent cmdEvent(EVT_SELECT_MANAGE_GRID_ROW);
    cmdEvent.SetEventObject(evt.GetEventObject());
    cmdEvent.SetInt(m_pMapFilterData->GetSelectedFilterIdx());
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
}


void ManageFiltersGrid::OnAutoSelectGridRow(wxCommandEvent& evt)
{
  ClearSelection();
  SelectRow(evt.GetInt());
}


void ManageFiltersGrid::OnChar(wxKeyEvent& evt)
{
  int nKey = evt.GetKeyCode();
  
  switch(nKey) {
    case WXK_LEFT:
    case WXK_RIGHT:
    case WXK_UP:
    case WXK_DOWN:
      if(m_pMapFilterData->IsFilterSelected()) {
        int newRow = -1;
        if(nKey == WXK_LEFT || nKey == WXK_UP) {
          if(m_pMapFilterData->GetSelectedFilterIdx() > 0)
            newRow = m_pMapFilterData->GetSelectedFilterIdx() - 1;
        }
        else if (nKey == WXK_RIGHT || nKey == WXK_DOWN) {
          if(m_pMapFilterData->GetSelectedFilterIdx() < (static_cast<int>(m_pMapFilterData->size()) - 1))
            newRow = m_pMapFilterData->GetSelectedFilterIdx() + 1;
        }
        if(newRow != -1) {
          // Simulate click on first field to select the demanded row
          wxRect rect = CellToRect(newRow, 0);
          wxRect devRect = BlockToDeviceRect(XYToCell(rect.GetTopLeft()), XYToCell(rect.GetTopRight()));

          wxGridEvent event(GetParent()->GetId(), wxEVT_GRID_CELL_LEFT_CLICK, this, newRow, 0, devRect.GetX()+(devRect.GetWidth()/ 2), devRect.GetY()+(devRect.GetHeight()/2));
          
          GetParent()->GetEventHandler()->AddPendingEvent(event);
        }
      }
      break;
    default:
      evt.Skip();
  }
}


/*!
 * ManageFiltersDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ManageFiltersDlg, wxDialog )

/*!
 * ManageFiltersDlg event table definition
 */

BEGIN_EVENT_TABLE( ManageFiltersDlg, wxDialog )

////@begin ManageFiltersDlg event table entries
  EVT_GRID_CELL_LEFT_CLICK( ManageFiltersDlg::OnCellLeftClick )

  EVT_BUTTON( wxID_NEW, ManageFiltersDlg::OnNewClick )
  EVT_BUTTON( ID_EDIT, ManageFiltersDlg::OnEditClick )
  EVT_BUTTON( wxID_COPY, ManageFiltersDlg::OnCopyClick )
  EVT_BUTTON( wxID_DELETE, ManageFiltersDlg::OnDeleteClick )
  EVT_BUTTON( ID_IMPORT, ManageFiltersDlg::OnImportClick )
  EVT_BUTTON( ID_EXPORT, ManageFiltersDlg::OnExportClick )
  EVT_BUTTON( wxID_HELP, ManageFiltersDlg::OnHelpClick )
  EVT_BUTTON( wxID_CLOSE, ManageFiltersDlg::OnCloseClick )

  EVT_SIZE( ManageFiltersDlg::OnSize )
////@end ManageFiltersDlg event table entries

END_EVENT_TABLE()


/*!
 * ManageFiltersDlg constructors
 */

ManageFiltersDlg::ManageFiltersDlg(wxWindow *parent, PWScore *core,
                                   PWSFilters &MapFilters,
                                   st_filters *currentFilters,
                                   FilterPool *activefilterpool,
                                   stringT *activefiltername,
                                   bool *bFilterActive,
                                   const bool bCanHaveAttachments,
                                   const std::set<StringX> *psMediaTypes,
                                   bool readOnly,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style ) :
                                            m_pMapAllFilters(&MapFilters),
                                            m_pCurrentFilters(currentFilters),
                                            m_bCanHaveAttachments(bCanHaveAttachments),
                                            m_psMediaTypes(psMediaTypes),
#if ! defined(__WXMAC__) || (wxVERSION_NUMBER >= 3104)
                                            m_bReadOnly(readOnly),
#else
                                            m_bReadOnly(false), // Copy to DB crash in 3.0.5 - positive tested with 3.1.4
#endif
                                            m_core(core),
                                            m_pActiveFilterPool(activefilterpool),
                                            m_pActiveFilterName(activefiltername),
                                            m_pbFilterActive(bFilterActive)
{
  wxASSERT(!parent || parent->IsTopLevel());
////@begin ManageFiltersDlg creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end ManageFiltersDlg creation
}


ManageFiltersDlg* ManageFiltersDlg::Create(wxWindow *parent, PWScore *core,
                                   PWSFilters &MapFilters,
                                   st_filters *currentFilters,
                                   FilterPool *activefilterpool,
                                   stringT *activefiltername,
                                   bool *bFilterActive,
                                   const bool bCanHaveAttachments,
                                   const std::set<StringX> *psMediaTypes,
                                   bool readOnly,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style )
{
  return new ManageFiltersDlg(parent, core, MapFilters, currentFilters,
                              activefilterpool, activefiltername,
                              bFilterActive, bCanHaveAttachments,
                              psMediaTypes, readOnly,
                              id, caption, pos, size, style);
}

/*!
 * Control creation for ManageFiltersDlg
 */

void ManageFiltersDlg::CreateControls()
{    
////@begin ManageFiltersDlg content construction
  ManageFiltersDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Available Filters:") + _T(" "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_LEFT|wxALL, 5);

  wxGridBagSizer *basicGridSizer = new wxGridBagSizer();
  itemBoxSizer2->Add(basicGridSizer, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP|wxALL, 0);
  
  m_MapFiltersGrid = new ManageFiltersGrid( itemDialog1, &m_MapFilterData, ID_FILTERSGRID, wxDefaultPosition,
                        wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL|wxEXPAND );
  
  m_MapFiltersGrid->SetTable(new pwManageFiltersTable(m_MapFilterData.Data()), true, wxGrid::wxGridSelectRows); // true => auto-delete
  
  wxFont font(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::TreeFont)));
  if (font.IsOk())
    m_MapFiltersGrid->SetDefaultCellFont(font);

  font = m_MapFiltersGrid->GetDefaultCellFont();
  if(font.IsOk()) {
    wxSize size = font.GetPixelSize();
    m_FontHeight = size.GetHeight();
    // Update Label font to have similar size
    m_MapFiltersGrid->SetLabelFont(font.Italic());
  }
  
  m_MapFiltersGrid->SetDefaultColSize(50, true);
  m_MapFiltersGrid->SetDefaultRowSize(m_FontHeight + 10);
  m_MapFiltersGrid->SetColLabelSize(25);
  m_MapFiltersGrid->SetRowLabelSize(0); // Do not show row numbers, we want to manage by ower own
  m_MapFiltersGrid->DisableDragRowSize();
  m_MapFiltersGrid->SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_MapFiltersGrid->SetSelectionForeground(*wxRED);
  
  // Set Column sorting mark
  m_MapFiltersGrid->SetSortingColumn(m_MapFilterData.GetSortingColumn(), m_MapFilterData.GetAscending());
  
  m_MapFiltersGrid->AutoSize();
  
  SetGridColLeftAligned(MFLC_FILTER_NAME);
  SetManageGridColFormat(MFLC_INUSE, new pwFiltersActiveRenderer(m_FontHeight));
  SetManageGridColFormat(MFLC_COPYTODATABASE, new pwFiltersActiveRenderer(m_FontHeight));
  SetManageGridColFormat(MFLC_EXPORT, new pwFiltersActiveRenderer(m_FontHeight));
 
  // Set size to a minimum
  wxClientDC dc(m_MapFiltersGrid->GetGridWindow());
  dc.SetFont(m_MapFiltersGrid->GetDefaultCellFont());
  wxSize size = dc.GetTextExtent(wxString(_("Filter 1")));
  size.SetWidth(size.GetWidth() * 2);
  if(m_bReadOnly) {
    // When readonly, do not copy to data base, do not show column. Add the space of the column to filters name.
    size.SetWidth(size.GetWidth() + m_MapFiltersGrid->GetColSize(MFLC_COPYTODATABASE));
    m_MapFiltersGrid->SetColSize(MFLC_COPYTODATABASE, 0);
  }
  else {
    ExtendColSize(MFLC_COPYTODATABASE);
  }
  if(size.GetWidth() > m_MapFiltersGrid->GetColSize(MFLC_FILTER_NAME))
    m_MapFiltersGrid->SetColSize(MFLC_FILTER_NAME, size.GetWidth() + 6);
  
  ExtendColSize(MFLC_INUSE);
  ExtendColSize(MFLC_EXPORT);
  
  size = ExtendColSize(MFLC_FILTER_SOURCE, 6);
  for(int fp = FPOOL_DATABASE; fp < FPOOL_LAST; ++fp) {
    size.IncTo(dc.GetTextExtent(pwManageFiltersTable::GetColLabelString(static_cast<FilterPool>(fp))));
  }
  if(size.GetWidth() > m_MapFiltersGrid->GetColSize(MFLC_FILTER_SOURCE))
    m_MapFiltersGrid->SetColSize(MFLC_FILTER_SOURCE, size.GetWidth() + 6);
  
  m_MapFiltersGrid->EnableEditing(false);
  
  // Determine minimal size of the Grid
  int width = m_MapFiltersGrid->GetRowLabelSize();
  for(int i = 0; i < MFLC_NUM_COLUMNS; i++) {
    width += m_MapFiltersGrid->GetColSize(i);
  }
  wxSize minSize(width, (m_FontHeight + 10) * FLT_DEFAULT_NUM_ROWS);
  m_MapFiltersGrid->SetMinClientSize(minSize);
  // At the end bind dynmic events
  m_MapFiltersGrid->BindEvents();
  
  basicGridSizer->Add(m_MapFiltersGrid, wxGBPosition(/*row:*/ 0, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 5, /*columnspan:*/ 1), wxALIGN_LEFT|wxALIGN_TOP|wxALL|wxEXPAND, 5);

  wxButton* itemButton6 = new wxButton( itemDialog1, wxID_NEW, _("&New"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton6, wxGBPosition(/*row:*/ 0, /*column:*/ 1), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton7 = new wxButton( itemDialog1, ID_EDIT, _("&Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton7, wxGBPosition(/*row:*/ 0, /*column:*/ 2), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton( itemDialog1, wxID_COPY, _("&Copy"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton8, wxGBPosition(/*row:*/ 1, /*column:*/ 1), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, wxID_DELETE, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton9, wxGBPosition(/*row:*/ 1, /*column:*/ 2), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, ID_IMPORT, _("&Import"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton10, wxGBPosition(/*row:*/ 2, /*column:*/ 1), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_EXPORT, _("E&xport"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton11, wxGBPosition(/*row:*/ 2, /*column:*/ 2), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton12, wxGBPosition(/*row:*/ 3, /*column:*/ 1), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(itemButton13, wxGBPosition(/*row:*/ 3, /*column:*/ 2), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_SelectedFilterText = new wxStaticText( itemDialog1, wxID_STATIC, _("Selected Filter Details:") + _T(" "), wxDefaultPosition, wxDefaultSize, 0 );
  basicGridSizer->Add(m_SelectedFilterText, wxGBPosition(/*row:*/ 5, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3),  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_filterGrid = new pwFiltersGrid(itemDialog1, ID_GRID,
                                   &m_SelectedFilter, DFTYPE_MAIN, FPOOL_LAST,
                                   m_bCanHaveAttachments, m_psMediaTypes, false, // Is read only
                                   wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL|wxEXPAND );
  basicGridSizer->Add(m_filterGrid, wxGBPosition(/*row:*/ 6, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3),  wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);
  
  // Only in case an entry is selected or copy/export flag set the buttons are enabled
  FindWindow(ID_EDIT)->Disable();
  FindWindow(wxID_COPY)->Disable();
  FindWindow(wxID_DELETE)->Disable();
  FindWindow(ID_EXPORT)->Disable();
  
  Layout();
  windowSize = GetSize(); // Fetch initial size
////@end ManageFiltersDlg content construction
}

wxSize ManageFiltersDlg::ExtendColSize(int col, int extend)
{
  wxClientDC dc(m_MapFiltersGrid->GetGridColLabelWindow());
#ifdef PW_FILTERS_GIRD_USE_NATIVE_HEADER
  dc.SetFont(m_MapFiltersGrid->GetDefaultCellFont());
#else
  dc.SetFont(m_MapFiltersGrid->GetLabelFont());
#endif
  wxSize size = dc.GetTextExtent(pwManageFiltersTable::GetColLabelString(col));
  m_MapFiltersGrid->SetColSize(col, size.GetWidth() + extend); // Extend for sorting
  return size;
}


/*!
 * Init the dialog after creation ManageFiltersDlg
 */

void ManageFiltersDlg::InitDialog()
{
  // Fill data from filter liste
  UpdateFilterList(false);
}


/*!
 * SetManageGridColFormat is setting column renderer and attribute. At the end set the minimum column width
 */

void ManageFiltersDlg::SetManageGridColFormat(int col, wxGridCellRenderer *renderer)
{
  ASSERT(renderer);
  ASSERT(m_MapFiltersGrid);
  wxGridCellAttr *attr = m_MapFiltersGrid->GetTable()->GetAttr(-1, col, wxGridCellAttr::Col);
  if(!attr)
      attr = new wxGridCellAttr;
  attr->SetRenderer(renderer);
  attr->SetReadOnly();
  attr->SetFont(m_MapFiltersGrid->GetDefaultCellFont());
  attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  
  m_MapFiltersGrid->SetColAttr(col, attr);
  
  // Determine best Coll width
  wxClientDC dc(m_MapFiltersGrid->GetGridColLabelWindow());
#ifdef PW_FILTERS_GIRD_USE_NATIVE_HEADER
  dc.SetFont(m_MapFiltersGrid->GetDefaultCellFont());
#else
  dc.SetFont(m_MapFiltersGrid->GetLabelFont());
#endif
  wxSize size = dc.GetTextExtent(m_MapFiltersGrid->GetColLabelValue(col)); // Size of header
  wxClientDC dc_cell(m_MapFiltersGrid->GetGridWindow());
  dc_cell.SetFont(m_MapFiltersGrid->GetDefaultCellFont());
  size.IncTo(renderer->GetBestSize(*m_MapFiltersGrid, *attr, dc_cell, 0, col)); // Size of entries
  
  m_MapFiltersGrid->SetColSize(col, size.GetWidth() + 6); // 6 is same as in SetColSize() for header extend with -1
}


/*!
 * SetGridColLeftAligned is setting column attribute. left aligned.
 */

void ManageFiltersDlg::SetGridColLeftAligned(int col)
{
  wxGridCellAttr *attr = m_MapFiltersGrid->GetTable()->GetAttr(-1, col, wxGridCellAttr::Col);
  if(!attr)
      attr = new wxGridCellAttr;

  attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
  m_MapFiltersGrid->SetColAttr(col, attr);
}


/*!
 * UpdateFilterList is filling the filter list with coontent of actual filter map.
 * Maintaine active and selected filter. Number of export and copy marked entries is reset.
 */

void ManageFiltersDlg::UpdateFilterList(bool bRefreshGrid /* = true */)
{
  m_MapFiltersGrid->GetTable()->Clear();
  m_MapFilterData.clear();

  PWSFilters::iterator mf_iter;
  int i = 0;
  m_num_to_copy = 0;
  m_num_to_export = 0;
  
  // Insert all known filters into sorted filter list
  for (mf_iter = m_pMapAllFilters->begin();
         mf_iter != m_pMapAllFilters->end();
         mf_iter++, i++) {
    struct st_FilterItemData data;
    bool bSelected = false;
    
    data.flt_key = mf_iter->first;
    data.flt_flags = 0;
    
    if(*m_pbFilterActive &&
       mf_iter->first.fpool == *m_pActiveFilterPool &&
       mf_iter->first.cs_filtername.c_str() == *m_pActiveFilterName) {
      data.flt_flags |= MFLT_INUSE;
    }
    if(mf_iter->first.fpool == m_SelectedFilterPool &&
       mf_iter->first.cs_filtername.c_str() == m_SelectedFilterName) {
      bSelected = true;
    }
    
    // Insert new entry into private list of sorted entries
    int idx = m_MapFilterData.insert(data);
    // Add row at the screen
    m_MapFiltersGrid->GetTable()->InsertRows(idx, 1);
    
    if(data.flt_flags & MFLT_INUSE)
      m_MapFilterData.SetActiveFilterIdx(idx);
    if(bSelected)
      m_MapFilterData.SetSelectedFilterIdx(idx);
    
    // Adapt width of filter name
    wxClientDC dc(m_MapFiltersGrid->GetGridWindow());
    dc.SetFont(m_MapFiltersGrid->GetDefaultCellFont());
    wxSize size = dc.GetTextExtent(wxString(mf_iter->first.cs_filtername));
    if(size.GetWidth() > m_MapFiltersGrid->GetColSize(MFLC_FILTER_NAME))
      m_MapFiltersGrid->SetColSize(MFLC_FILTER_NAME, size.GetWidth() + 6);
  }
  // When active filter is no longer in list, remove active filter
  if(*m_pbFilterActive && m_MapFilterData.IsNoFilterActive()) {
    ClearActiveFilter();
  }
  else if(m_MapFilterData.IsFilterActive()) {
    // Update filter with actual content
    st_Filterkey fk;
    fk.fpool = *m_pActiveFilterPool;
    fk.cs_filtername = *m_pActiveFilterName;
    mf_iter = m_pMapAllFilters->find(fk);
    if(mf_iter != m_pMapAllFilters->end()) {
      // When active, update filter with actual content
      UpdateActiveFilterContent(fk, mf_iter->second);
    }
    else {
      wxASSERT(false);
    }
  }
  
  // On single entry, select this one as defauzlt
  if(m_pMapAllFilters->size() == 1 && m_MapFilterData.IsNoFilterSelected()) {
    m_MapFilterData.SetSelectedFilterIdx(0);
    m_SelectedFilterPool = m_MapFilterData.GetSelectedPool();
    m_SelectedFilterName = m_MapFilterData.GetSelectedFilterName();
  }
  // Depending on selected entries enable or disable button
  if(m_MapFilterData.IsNoFilterSelected()) {
    m_SelectedFilterPool = FPOOL_LAST;
    m_SelectedFilterName = L"";
    
    FindWindow(ID_EDIT)->Disable();
    FindWindow(wxID_DELETE)->Disable();
  }
  else {
    SelectEntry(m_MapFilterData.GetSelectedFilterIdx());
  }
  
  FindWindow(wxID_COPY)->Disable();
  FindWindow(ID_EXPORT)->Disable();
  
  if(bRefreshGrid)
    m_MapFiltersGrid->ForceRefresh();
  
  ShowSelectedFilter();
}


/*!
 * ShowSelectedFilter is upating lower grid with content of the selected filter and update label with label name
 */

void ManageFiltersDlg::ShowSelectedFilter()
{
  m_filterGrid->ClearFilter();
  if(m_MapFilterData.IsFilterSelected()) {
    st_Filterkey fk = m_MapFilterData.GetSelectedKey();
    
    PWSFilters::iterator mf_iter = m_pMapAllFilters->find(fk);
    if(mf_iter != m_pMapAllFilters->end()) {
      m_SelectedFilter = mf_iter->second;
    }
    wxString label = _("Selected Filter Details:") + _T(" ") + fk.cs_filtername + _T(" ");    
    wxClientDC dc(this);
    dc.SetFont(GetFont());
    wxSize size = dc.GetTextExtent(label), labelSize = m_SelectedFilterText->GetSize();
    if(size.GetWidth() > labelSize.GetWidth()) {
      labelSize.IncTo(size);
      m_SelectedFilterText->SetSize(size);
    }
    m_SelectedFilterText->SetLabel(label);
    
    m_filterGrid->RefreshFilter();
  }
  else {
    m_SelectedFilterText->SetLabel(_("Selected Filter Details:") + _T(" "));
    m_SelectedFilter.Empty();
  }
  m_SelectedFilterText->Refresh();
  m_filterGrid->ForceRefresh();
}

/*!
 * Should we show tooltips?
 */

bool ManageFiltersDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ManageFiltersDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin ManageFiltersDlg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end ManageFiltersDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ManageFiltersDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin ManageFiltersDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end ManageFiltersDlg icon retrieval
}


/*!
 * wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERSGRID
 */

void ManageFiltersDlg::OnCellLeftClick( wxGridEvent& event )
{
  int row = event.GetRow(), col = event.GetCol();
  
  if(row >= 0 && row < static_cast<int>(m_MapFilterData.size())) { // Check read write and range of row
    if(m_MapFilterData.GetSelectedFilterIdx() != row) { // On change of selected row, mark the row first time, second click will active function
      SelectEntry(row);
      ShowSelectedFilter();
    }
    else {  // When row is already selected one, check on column to perform related action
      if(col == MFLC_INUSE) {
        // Apply filter or clear
        if(m_MapFilterData.IsFlagSetAt(row, MFLT_INUSE)) {
          m_MapFilterData.ClearFlagAt(row, MFLT_INUSE);
          ClearSelectedActiveFilter();
        }
        else {
          m_MapFilterData.SetFlagAt(row, MFLT_INUSE);
          SetSelectedActiveFilter();
        }
        RefreshFiltersCell(row, MFLC_INUSE);
        // If column is sorting refresh order
        if(m_MapFilterData.GetSortingColumn() == MFLC_INUSE) {
          m_MapFiltersGrid->SortRefresh();
        }
      }
      else if(col == MFLC_COPYTODATABASE) {
        if(m_MapFilterData.GetPoolAt(row) != FPOOL_DATABASE) { // Do not toggle DB entries status
          // Toggle copy to DB status
          if(m_MapFilterData.IsFlagSetAt(row, MFLT_REQUEST_COPY_TO_DB)) {
            m_MapFilterData.ClearFlagAt(row, MFLT_REQUEST_COPY_TO_DB);
            m_num_to_copy--;
          }
          else {
            m_MapFilterData.SetFlagAt(row, MFLT_REQUEST_COPY_TO_DB);
            m_num_to_copy++;
          }
          FindWindow(wxID_COPY)->Enable(m_num_to_copy > 0);
          RefreshFiltersCell(row, MFLC_COPYTODATABASE);
          // If column is sorting refresh order
          if(m_MapFilterData.GetSortingColumn() == MFLC_COPYTODATABASE) {
            m_MapFiltersGrid->SortRefresh();
          }
        }
      }
      else if(col == MFLC_EXPORT) {
        // Toggle export status
        if(m_MapFilterData.IsFlagSetAt(row, MFLT_REQUEST_EXPORT)) {
          m_MapFilterData.ClearFlagAt(row, MFLT_REQUEST_EXPORT);
          m_num_to_export--;
        }
        else {
          m_MapFilterData.SetFlagAt(row, MFLT_REQUEST_EXPORT);
          m_num_to_export++;
        }
        FindWindow(ID_EXPORT)->Enable(m_num_to_export > 0);
        RefreshFiltersCell(row, MFLC_EXPORT);
        // If column is sorting refresh order
        if(m_MapFilterData.GetSortingColumn() == MFLC_EXPORT) {
          m_MapFiltersGrid->SortRefresh();
        }
      }
    }
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
 */

void ManageFiltersDlg::OnNewClick(wxCommandEvent&)
{
  CallAfter(&ManageFiltersDlg::DoNewClick);
}

void ManageFiltersDlg::DoNewClick()
{
  st_filters filters; // New filter is empty
  bool bActiveFilter = m_MapFilterData.IsFilterActive();
  bool bDoEdit = true;
  bool bAppliedCalled = false;
  
  while(bDoEdit) {
    bDoEdit = false; // In formal case we run only one time in the loop; but when removal of double entry is requested we avoid goto

    int rc = ShowModalAndGetResult<SetFiltersDlg>(this, &filters, m_pCurrentFilters, &bAppliedCalled, DFTYPE_MAIN, FPOOL_SESSION, m_bCanHaveAttachments, m_psMediaTypes);
    
    if(bActiveFilter && ! *m_pbFilterActive) {
      // On filter active before and now no filter active take back usage flag
      if(m_MapFilterData.IsFilterActive()) {
        m_MapFilterData.ClearFlagAt(m_MapFilterData.GetActiveFilterIdx(), MFLT_INUSE);
        RefreshFiltersCell(m_MapFilterData.GetActiveFilterIdx(), MFLC_INUSE);
        m_MapFilterData.SetActiveFilterInactive();
      }
      bActiveFilter = false;
    }
  
    if(rc == wxID_OK || (rc == wxID_CANCEL && bAppliedCalled && !IsCloseInProgress())) {
      if(rc != wxID_OK) {
        // Overtake last applied filter
        filters = *m_pCurrentFilters;
      }
      
      st_Filterkey fk;
      fk.fpool = FPOOL_SESSION;
      fk.cs_filtername = filters.fname;
      int idx = -1;
    
      PWSFilters::iterator mf_iter = m_pMapAllFilters->find(fk);
      if(mf_iter != m_pMapAllFilters->end()) {
        wxMessageDialog dialog(this, _("This filter already exists"), _("Do you wish to replace it?"), wxYES_NO | wxICON_EXCLAMATION);
        if(dialog.ShowModal() == wxID_NO) {
          bDoEdit = true; // Repeat editing to allow name change
          continue;
        }
        // Replace content
        mf_iter->second = filters;
        
        // Set actual one as selected entry
        idx = FindIndex(fk);
        SelectEntry(idx);
        
        // Force actual one as applied. as content could be changed
        if(bAppliedCalled) {
          // Update Flags and index, may be changed after apply pressed
          MarkAppliedFilter(); // Selected entry is already called
          *m_pActiveFilterPool = fk.fpool;
          *m_pActiveFilterName = fk.cs_filtername;
        }
        
        // When active, update filter with new content
        UpdateActiveFilterContent(fk, filters);
      }
      else {
        idx = InsertEntry(fk, filters);
        SelectEntry(idx);
        m_MapFiltersGrid->ForceRefresh();
        // Update with actual content, when applied had been called
        if(bAppliedCalled && rc == wxID_OK) {
          SetSelectedActiveFilter();
        }
      }
      ShowSelectedFilter();
      // When Applied had been done in New/Edit Dialog the actual filter will be marked as in use
      if(bAppliedCalled) {
        MarkAppliedFilter();
      }
    }
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT
 */

void ManageFiltersDlg::OnEditClick(wxCommandEvent&)
{
  CallAfter(&ManageFiltersDlg::DoEditClick);
}

void ManageFiltersDlg::DoEditClick()
{
  st_Filterkey fk;
  st_filters filters; // New filter is empty
  PWSFilters::iterator mf_iter_entry;
  
  fk = m_MapFilterData.GetSelectedKey();
  mf_iter_entry = m_pMapAllFilters->find(fk);
  if(mf_iter_entry != m_pMapAllFilters->end()) {
    filters = mf_iter_entry->second;
  }
  else {
    wxASSERT(false);
    return;
  }
  wxASSERT(fk.fpool == m_SelectedFilterPool && fk.cs_filtername == m_SelectedFilterName);
  
  bool bActiveFilter = (m_MapFilterData.IsFilterActive() && (m_MapFilterData.GetActiveFilterIdx() == m_MapFilterData.GetSelectedFilterIdx()));
  bool bDoEdit = true;
  bool bAppliedCalled = false;
  
  while(bDoEdit) { // Loop to avoid goto
    bDoEdit = false;  // In formal case we run only one time in the loop; but when removal of double entry is requested we avoid goto

    int rc = ShowModalAndGetResult<SetFiltersDlg>(this, &filters, m_pCurrentFilters, &bAppliedCalled, DFTYPE_MAIN, fk.fpool, m_bCanHaveAttachments, m_psMediaTypes);
    
    if(bActiveFilter && ! *m_pbFilterActive) {
      // On filter active before and now no filter active take back usage flag
      if(m_MapFilterData.IsFilterActive()) {
        m_MapFilterData.ClearFlagAt(m_MapFilterData.GetActiveFilterIdx(), MFLT_INUSE);
        RefreshFiltersCell(m_MapFilterData.GetActiveFilterIdx(), MFLC_INUSE);
        m_MapFilterData.SetActiveFilterInactive();
      }
      bActiveFilter = false;
    }
  
    if (rc == wxID_OK || (rc == wxID_CANCEL && bAppliedCalled && !IsCloseInProgress())) {
      if(rc != wxID_OK) {
        // Overtake last applied filter
        filters = *m_pCurrentFilters;
      }
    
      st_Filterkey fk_new;
      fk_new.fpool = fk.fpool;
      fk_new.cs_filtername = filters.fname;
      
      // To avoid idea of user that an imported filter will be (automatically) updated, the type is changed to session
      if(fk_new.fpool == FPOOL_AUTOLOAD ||  fk_new.fpool == FPOOL_IMPORTED)
        fk_new.fpool = FPOOL_SESSION;
    
      PWSFilters::iterator mf_iter_new = m_pMapAllFilters->find(fk_new);
      if(mf_iter_new != m_pMapAllFilters->end() && mf_iter_new != mf_iter_entry) {
        wxMessageDialog dialog(this, _("This filter already exists"), _("Do you wish to replace it?"), wxYES_NO | wxICON_EXCLAMATION);
        if(dialog.ShowModal() == wxID_NO) {
          bDoEdit = true; // Repeat editing to allow name change
          continue;
        }
        // Remove the old entry
        DeleteSelectedFilter();
        // Update Interator after delete;
        mf_iter_new = m_pMapAllFilters->find(fk_new);
        wxASSERT(mf_iter_new != m_pMapAllFilters->end());
        mf_iter_new->second = filters;
        // Select new entry
        int idx_new = FindIndex(fk_new);
        SelectEntry(idx_new);
        wxASSERT(m_MapFilterData.GetSelectedFilterIdx() >= 0);
        if(bActiveFilter || bAppliedCalled) {
          SetSelectedActiveFilter();
        }
        else if(idx_new == m_MapFilterData.GetActiveFilterIdx()) {
          // When active, update filter with new content
          UpdateActiveFilterContent(fk_new, filters);
        }
      }
      else {
        // Update of filter name or pool
        if(fk_new.fpool != fk.fpool || filters.fname != fk.cs_filtername) {
          // Remove the entry
          DeleteSelectedFilter();
          // And add again with new data
          int idx_new = InsertEntry(fk_new, filters);
          SelectEntry(idx_new);
          m_MapFiltersGrid->ForceRefresh();
          // Correct active filter idx when needed
          if(bActiveFilter) {
            m_MapFilterData.SetActiveFilterIdx(idx_new);
          }
          // Update the name field
          RefreshFiltersCell(idx_new, MFLC_FILTER_NAME);
        }
        else {
          ASSERT(mf_iter_entry != m_pMapAllFilters->end());
          // Update the existing entry
          mf_iter_entry->second = filters;
        }
        
        // Update when filter had been active
        if (bActiveFilter || bAppliedCalled) {
          SetSelectedActiveFilter();
        }
      }
      
      if(fk.fpool == FPOOL_DATABASE)
        m_bDBFiltersChanged = true;
      
      // When Applied had been done in New/Edit Dialog the actual filter will be marked as in use
      if(bAppliedCalled) {
        MarkAppliedFilter();
      }
      
      m_MapFiltersGrid->ForceRefresh();
      ShowSelectedFilter();
    }
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_COPY
 */

void ManageFiltersDlg::OnCopyClick( wxCommandEvent& event )
{
  size_t i, numFilters = m_MapFilterData.size();
  bool bCopied = false;
  
  for(i = 0; i < numFilters; ++i) {
    // Copy all marked entries
    if(m_MapFilterData.IsFlagSetAt(i, MFLT_REQUEST_COPY_TO_DB)) {
      st_Filterkey fk = m_MapFilterData.GetKeyAt(i);
      PWSFilters::iterator mf_iter;
      
      if(fk.fpool == FPOOL_DATABASE) {
        // Do not copy over itself
        continue;
      }

      mf_iter = m_pMapAllFilters->find(fk);
      if (mf_iter == m_pMapAllFilters->end()) {
        // Skip not found entry
        wxASSERT(false);
        continue;
      }
      
      m_MapFilterData.ClearFlagAt(i, MFLT_REQUEST_COPY_TO_DB);
      m_num_to_copy--;

      PWSFilters::iterator mf_citer;
      st_Filterkey flt_keydb;
      flt_keydb.fpool = FPOOL_DATABASE;
      flt_keydb.cs_filtername = fk.cs_filtername;
      mf_citer = m_pMapAllFilters->find(flt_keydb);
      // Check on entry alread exists
      if (mf_citer != m_pMapAllFilters->end()) {
        wxMessageDialog dialog(this, _("This filter already exists"), _("Do you wish to replace it?"), wxYES_NO | wxICON_EXCLAMATION);
        if(dialog.ShowModal() == wxID_NO) {
          // Skip this one on users answer
          continue;
        }
        // User agrees to replace - replace content
        mf_citer->second = mf_iter->second;
        // Update Filter, when changed
        UpdateActiveFilterContent(flt_keydb, mf_citer->second);
        // If selected filter, show updated content
        if(m_SelectedFilterPool == flt_keydb.fpool &&
           m_SelectedFilterName == flt_keydb.cs_filtername) {
          ShowSelectedFilter();
        }
      }
      else {
        (void) InsertEntry(flt_keydb, mf_iter->second);
        numFilters = m_MapFilterData.size();
        --i; // If entry is inserted before current one, we can miss one
      }

      bCopied = true;
    }
  }
  
  if(bCopied) {
    m_bDBFiltersChanged = true;
    // Update the button and view
    FindWindow(wxID_COPY)->Enable(m_num_to_copy > 0);
    m_MapFiltersGrid->ForceRefresh(); // To update the copy flags
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
 */

void ManageFiltersDlg::OnDeleteClick( wxCommandEvent& WXUNUSED(event) )
{
  DeleteSelectedFilter();
  
  m_MapFiltersGrid->ForceRefresh();
  ShowSelectedFilter();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT
 */

void ManageFiltersDlg::OnImportClick( wxCommandEvent& event )
{
  wxAsker asker;
  
  // Build default file name
  wxFileName TxtFileName(towxstring(PWSUtil::GetNewFileName(stringx2std(m_core->GetCurFile()) + wxT(".") + wxT("filters"), wxT("xml"))));
  // Ask for file name
  wxFileDialog fd(this, _("Choose an XML File to Import"), TxtFileName.GetPath(),
                  TxtFileName.GetFullName(), _("XML files (*.xml)|*.xml"),
                  wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);

  if (fd.ShowModal() == wxID_OK) {
    wxString filename = fd.GetPath();
    stringT strErrors = L"";
    
    int rc = m_pMapAllFilters->ImportFilterXMLFile(FPOOL_IMPORTED, L"", tostdstring(filename), L"", strErrors, &asker, nullptr);
    
    if (rc != PWScore::SUCCESS) {
      wxString reason = _("Error");
      stringT cs_error;
      
      if(rc == PWScore::XML_FAILED_VALIDATION) {
        Format(cs_error, _("File %ls was not imported (failed validation against XML Schema)").c_str(), tostdstring(filename).c_str());
      }
      else if(rc == PWScore::XML_FAILED_IMPORT) {
        Format(cs_error, _("File %ls was not imported (error during import)").c_str(), tostdstring(filename).c_str());
      }
      else {
        Format(cs_error, _("File %ls was not imported").c_str(), tostdstring(filename).c_str());
      }
      wxMessageBox(towxstring(strErrors), towxstring(cs_error), wxOK | wxICON_ERROR, this);
      pws_os::Trace(L"Error while parsing filters from file: %ls.\n\tErrors: %ls\n", tostdstring(filename).c_str(), strErrors.c_str());
    }
    else {
      if (!strErrors.empty()) {
        stringT cs_error;
        Format(cs_error, _("File %ls was imported with error").c_str(), tostdstring(filename).c_str());
        wxMessageBox(towxstring(strErrors), towxstring(cs_error), wxOK | wxICON_ERROR, this);
      }
      else {
        wxMessageBox(filename, _("Filters successfully imported"), wxOK | wxICON_INFORMATION, this);
      }
      // Update the list of all filters on screen
      UpdateFilterList(true);
    }
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT
 */

void ManageFiltersDlg::OnExportClick( wxCommandEvent& event )
{
  PWSFilters Filters;
  size_t i, numFilters = m_MapFilterData.size();
  
  for(i = 0; i < numFilters; ++i) {
    // Export all marked filters
    if(m_MapFilterData.IsFlagSetAt(i, MFLT_REQUEST_EXPORT)) {
      st_Filterkey fk = m_MapFilterData.GetKeyAt(i);
      PWSFilters::iterator mf_iter;
      
      mf_iter = m_pMapAllFilters->find(fk);
      if (mf_iter == m_pMapAllFilters->end()) {
        // Skip not found entry
        wxASSERT(false);
        continue;
      }
      Filters.insert(PWSFilters::Pair(fk, mf_iter->second));
      
      m_MapFilterData.ClearFlagAt(i, MFLT_REQUEST_EXPORT);
      m_num_to_export--;
    }
  }
  // When filter copied into collection do the export
  if (!Filters.empty()) {
    // Build default file name
    wxFileName TxtFileName(towxstring(PWSUtil::GetNewFileName(stringx2std(m_core->GetCurFile()), tostdstring(_("filter")) + wxT(".xml"))));
    // Ask for file name
    wxFileDialog fd(this, _("Name the XML file"), TxtFileName.GetPath(),
                    TxtFileName.GetFullName(), _("XML files (*.xml)|*.xml|All files (*.*; *)|*.*;*"),
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (fd.ShowModal() == wxID_OK) {
      PWSfileHeader hdr = m_core->GetHeader();
      StringX newfile = tostringx(fd.GetPath());
      StringX currentfile = m_core->GetCurFile();
      
      int rc = Filters.WriteFilterXMLFile(newfile, hdr, currentfile);
      // Show result
      if (rc != PWScore::SUCCESS) {
        PasswordSafeFrame::DisplayFileWriteError(rc, newfile);
      }
      else {
        wxMessageBox(towxstring(newfile), _("Filters successfully exported"), wxOK | wxICON_INFORMATION, this);
      }
    }
    Filters.clear();
  }
  FindWindow(ID_EXPORT)->Enable(m_num_to_export > 0);
  m_MapFiltersGrid->ForceRefresh(); // To update the export flags
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void ManageFiltersDlg::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in ManageFiltersDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in ManageFiltersDlg.
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void ManageFiltersDlg::OnCloseClick( wxCommandEvent& event )
{
  // Save DB filter when flag is set and imediate storage configured -> return ID_OK or ID_CANCEL
  EndModal(m_bDBFiltersChanged ? wxID_OK : wxID_CANCEL);
}


/**
 * Event handler (EVT_SIZE) that will be called when the window has been resized.
 *
 * @param event holds information about size change events.
 * @see <a href="http://docs.wxwidgets.org/3.0/classwx_size_event.html">wxSizeEvent Class Reference</a>
 */

void ManageFiltersDlg::OnSize(wxSizeEvent &event)
{
  if(windowSize == wxSize(0, 0) || windowSize == SYMBOL_MANAGEFILTERS_SIZE) {
    // At startup we must fetch the actual size
    windowSize = event.GetSize();
  }
  else if(windowSize != event.GetSize()) {
    // On change recalculate new size of the grid
    wxSize mapFiltersSize = m_MapFiltersGrid->GetSize();
    wxSize gridFilterSize = m_filterGrid->GetSize();
    
    // Use half the size for the filter map the the filter grid
    int widthDiff = event.GetSize().GetWidth() - windowSize.GetWidth();
    int heightDiff = event.GetSize().GetHeight() - windowSize.GetHeight();
    int mapHeightDiff = heightDiff / 2;
    int filterHeightDiff = heightDiff - mapHeightDiff;
    
    mapFiltersSize.SetWidth(mapFiltersSize.GetWidth() + widthDiff);
    mapFiltersSize.SetHeight(mapFiltersSize.GetHeight() + mapHeightDiff);
    gridFilterSize.SetWidth(gridFilterSize.GetWidth() + widthDiff);
    gridFilterSize.SetHeight(gridFilterSize.GetHeight() + filterHeightDiff);
    
    m_MapFiltersGrid->SetMinSize(mapFiltersSize);
    m_filterGrid->SetMinSize(gridFilterSize);
    
    windowSize = event.GetSize();
    
    Layout();
  }
  event.Skip();
}


/*!
 * InsertEntry: Insert Entry into all related maps and screen
 */

int ManageFiltersDlg::InsertEntry(const st_Filterkey &fk, const st_filters &filters)
{
  m_pMapAllFilters->insert(PWSFilters::Pair(fk, filters));
  
  struct st_FilterItemData entry;
  entry.flt_key = fk;
  entry.flt_flags = 0;
  int idx = m_MapFilterData.insert(entry);
  if(idx == 0 && m_MapFilterData.size() == 1)
    m_MapFiltersGrid->GetTable()->AppendRows(1);
  else
    m_MapFiltersGrid->GetTable()->InsertRows(idx, 1);
  return idx;
}


/*!
 * DeleteSelectedFilter: Remove the currently selected filter
 */

void ManageFiltersDlg::DeleteSelectedFilter()
{
  st_Filterkey fk;
  PWSFilters::iterator mf_iter_entry;
  
  if(m_MapFilterData.IsFilterSelected()) {
    fk = m_MapFilterData.GetSelectedKey();
    wxASSERT(fk.fpool == m_SelectedFilterPool && fk.cs_filtername == m_SelectedFilterName);
    size_t idx = m_MapFilterData.GetSelectedFilterIdx();
    
    mf_iter_entry = m_pMapAllFilters->find(fk);
    if(mf_iter_entry != m_pMapAllFilters->end()) {
      // Correct flags count for copy and export
      if(m_MapFilterData.IsFlagSetAt(idx, MFLT_REQUEST_COPY_TO_DB)) {
        m_num_to_copy--;
        if(m_num_to_copy <= 0)
          FindWindow(wxID_COPY)->Disable();
      }
      if(m_MapFilterData.IsFlagSetAt(idx, MFLT_REQUEST_EXPORT)) {
        m_num_to_export--;
        if(m_num_to_export <= 0)
          FindWindow(ID_EXPORT)->Disable();
      }
      
      // Remove from all places
      m_pMapAllFilters->erase(mf_iter_entry);
      m_MapFilterData.remove(); // Remove actual selected index and restore index
      m_MapFiltersGrid->GetTable()->DeleteRows(idx, 1);
      
      // Update when filter had been active
      if (*m_pbFilterActive &&
          *m_pActiveFilterPool == fk.fpool &&
          *m_pActiveFilterName == fk.cs_filtername) {
        ClearActiveFilter();
      }
      
      if(! SelectIfSingleEntry()) { // When one entry left only, select this one,
        // Otherwise mark as not selected
        m_MapFilterData.SetSelectedFilterInactive();
        m_SelectedFilterPool = FPOOL_LAST;
        m_SelectedFilterName = L"";
        FindWindow(ID_EDIT)->Disable();
        FindWindow(wxID_DELETE)->Disable();
      }
    }
    else {
      wxASSERT(false);
    }
  }
}


/*!
 * SetSelectedActiveFilter: Set the actual selected filter as applied filter
 */

void ManageFiltersDlg::SetSelectedActiveFilter()
{
  st_Filterkey fk;
  PWSFilters::iterator mf_iter;

  MarkAppliedFilter(); // Mark filter as in use
  *m_pActiveFilterPool = m_SelectedFilterPool;
  *m_pActiveFilterName = m_SelectedFilterName;
  
  fk.fpool = m_SelectedFilterPool;
  fk.cs_filtername = m_SelectedFilterName;
  // Search for the filters content
  mf_iter = m_pMapAllFilters->find(fk);
  if(mf_iter != m_pMapAllFilters->end()) {
    m_pCurrentFilters->Empty();
    *m_pCurrentFilters = mf_iter->second;
    
    // Send event on menu Apply filter to show new applied filter while dialog is open
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_APPLYFILTER);
    event.SetString(pwManageFiltersTable::getSourcePoolLabel(m_SelectedFilterPool));
    GetParent()->GetEventHandler()->ProcessEvent(event);
  }
  else {
    wxASSERT(false);
  }
}


/*!
 * ClearSelectedActiveFilter: Deselect/Clear the actual selected filter as applied filter
 */

void ManageFiltersDlg::ClearSelectedActiveFilter()
{
  if(m_MapFilterData.IsFilterActive()) {
    m_MapFilterData.ClearFlagAt(m_MapFilterData.GetActiveFilterIdx(), MFLT_INUSE);
    RefreshFiltersCell(m_MapFilterData.GetActiveFilterIdx(), MFLC_INUSE);
    m_MapFilterData.SetActiveFilterInactive();
  }
  
  // Update active filter content and remove applied
  ClearActiveFilter();
  *m_pbFilterActive = false;
  *m_pActiveFilterPool = FPOOL_LAST;
  *m_pActiveFilterName = L"";
}


/*!
 * RefreshFiltersRow: Referesh filter row on screen
 */

void ManageFiltersDlg::RefreshFiltersRow(int row)
{
  wxRect rect(m_MapFiltersGrid->CellToRect(row, 0));
  rect.x = 0;
  rect.width = m_MapFiltersGrid->GetGridWindow()->GetClientSize().GetWidth();
  int dummy;
  m_MapFiltersGrid->CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
  m_MapFiltersGrid->GetGridWindow()->Refresh(true, &rect);
}


/*!
 * RefreshFiltersCell: Referesh filter cell on screen
 */

void ManageFiltersDlg::RefreshFiltersCell(int row, int col)
{
  wxRect rect(m_MapFiltersGrid->CellToRect(row, col));
  m_MapFiltersGrid->CalcScrolledPosition(rect.x, rect.y, &rect.x, &rect.y);
  m_MapFiltersGrid->GetGridWindow()->Refresh(true, &rect);
}


/*!
 * MarkAppliedFilter: Mark actual selected filter as in use
 */

void ManageFiltersDlg::MarkAppliedFilter()
{
  // On previous active filter remove in use flag
  if(m_MapFilterData.IsFilterActive() &&  m_MapFilterData.GetActiveFilterIdx() != m_MapFilterData.GetSelectedFilterIdx()) {
    m_MapFilterData.ClearFlagAt(m_MapFilterData.GetActiveFilterIdx(), MFLT_INUSE);
    RefreshFiltersCell(m_MapFilterData.GetActiveFilterIdx(), MFLC_INUSE);
  }
  // Set filter as active and set in use flag
  m_MapFilterData.SetActiveFilterIdx(m_MapFilterData.GetSelectedFilterIdx());
  m_MapFilterData.SetFlagAt(m_MapFilterData.GetActiveFilterIdx(), MFLT_INUSE);
  RefreshFiltersCell(m_MapFilterData.GetActiveFilterIdx(), MFLC_INUSE);
}


/*!
 * SelectEntry: Select the given index (equal to row)
 */

void ManageFiltersDlg::SelectEntry(int idx)
{
  if(idx >= 0 && idx < static_cast<int>(m_MapFilterData.size())) {
    // On previous selected filter remove selection
    if(idx != m_MapFilterData.GetSelectedFilterIdx() && m_MapFilterData.IsFilterSelected()) {
      m_MapFiltersGrid->ClearSelection();   // No specific clear of row, due to error in wxWidgets 3.1.4
      RefreshFiltersRow(m_MapFilterData.GetSelectedFilterIdx());
    }
    // Mark as selected
    m_MapFilterData.SetSelectedFilterIdx(idx);
    m_SelectedFilterPool = m_MapFilterData.GetSelectedPool();
    m_SelectedFilterName = m_MapFilterData.GetSelectedFilterName();
    m_MapFiltersGrid->SelectRow(idx);
    // Set selected entry to be visible on screen
    if(! m_MapFiltersGrid->IsVisible(m_MapFilterData.GetSelectedFilterIdx(), m_MapFilterData.GetSortingColumn()))
      m_MapFiltersGrid->MakeCellVisible(m_MapFilterData.GetSelectedFilterIdx(), m_MapFilterData.GetSortingColumn());
    RefreshFiltersRow(idx);
    // Enable edit and delete when entry is selected
    FindWindow(ID_EDIT)->Enable();
    FindWindow(wxID_DELETE)->Enable();
  }
  else {
    wxASSERT(false);
  }
}


/*!
 * SelectIfSingleEntry: Select the left entry automatically when only one entry is present
 */

bool ManageFiltersDlg::SelectIfSingleEntry()
{
  if(m_MapFilterData.size() == 1) {
    SelectEntry(0);
    return true;
  }
  return false;
}


/*!
 * UpdateActiveFilterContent: Update the applied filters content when the filter key is equal to the given one
 */

void ManageFiltersDlg::UpdateActiveFilterContent(const st_Filterkey &fk, const st_filters &filters)
{
  // Update when filter had been active
  if (*m_pbFilterActive &&
      *m_pActiveFilterPool == fk.fpool &&
      *m_pActiveFilterName == fk.cs_filtername) {
    m_pCurrentFilters->Empty();
    *m_pCurrentFilters = filters;
    
    // Send event on menu Apply filter to show new applied filter content while dialog is open
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_APPLYFILTER);
    event.SetString(pwManageFiltersTable::getSourcePoolLabel(fk.fpool));
    GetParent()->GetEventHandler()->ProcessEvent(event);
  }
}


/*!
 * ClearActiveFilter: Clear filter applied, when active
 */

void ManageFiltersDlg::ClearActiveFilter()
{
  if(*m_pbFilterActive) {
    *m_pActiveFilterPool = FPOOL_LAST;
    *m_pActiveFilterName = L"";
    m_MapFilterData.SetActiveFilterInactive();
    m_pCurrentFilters->Empty();
    
    // Send event on menu Apply filter to remove applied filter while dialog is open
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_APPLYFILTER);
    GetParent()->GetEventHandler()->ProcessEvent(event);
  }
}
