/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwsgrid.cpp
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
////@end includes

#include <wx/memory.h>

#include "PWSgrid.h"
#include "passwordsafeframe.h" // for DispatchDblClickAction()
#include "./PWSgridtable.h"

#include <algorithm>
#include <utility> // for make_pair

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

using pws_os::CUUID;

/*!
 * PWSGrid type definition
 */

IMPLEMENT_CLASS( PWSGrid, wxGrid )

/*!
 * PWSGrid event table definition
 */

BEGIN_EVENT_TABLE( PWSGrid, wxGrid )

////@begin PWSGrid event table entries
  EVT_GRID_CELL_RIGHT_CLICK( PWSGrid::OnCellRightClick )
  EVT_GRID_CELL_LEFT_DCLICK( PWSGrid::OnLeftDClick )
  EVT_GRID_SELECT_CELL( PWSGrid::OnSelectCell )
  EVT_CONTEXT_MENU(PWSGrid::OnContextMenu)
  EVT_CUSTOM(wxEVT_GUI_DB_PREFS_CHANGE, wxID_ANY, PWSGrid::OnDBGUIPrefsChange)
////@end PWSGrid event table entries

END_EVENT_TABLE()

/*!
 * PWSGrid constructors
 */

PWSGrid::PWSGrid(PWScore &core) : m_core(core)
{
  Init();
}

PWSGrid::PWSGrid(wxWindow* parent, PWScore &core,
                 wxWindowID id, const wxPoint& pos,
                 const wxSize& size, long style) : m_core(core)
{
  Init();
  Create(parent, id, pos, size, style);
  
  auto *header = wxGrid::GetGridColHeader();
  
  if (header) {
    
    // Handler for double click events on column header separator
    header->Bind(
      wxEVT_HEADER_SEPARATOR_DCLICK, 
      [=](wxHeaderCtrlEvent& event) {
        wxGrid::AutoSizeColumn(event.GetColumn());
      }
    );
    
    // Handler for single click events on column header
    header->Bind(
      wxEVT_HEADER_CLICK, 
      &PWSGrid::OnHeaderClick,
      this
    );
  }
}

/*!
 * PWSGrid creator
 */

bool PWSGrid::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin PWSGrid creation
  wxGrid::Create(parent, id, pos, size, style);
  CreateControls();
  EnableGridLines(PWSprefs::GetInstance()->GetPref(PWSprefs::ListViewGridLines));
  EnableDragColMove(true);

  //column picker is free if built with wx2.9.1
#if wxCHECK_VERSION(2, 9, 1)
  UseNativeColHeader(true);
#endif
////@end PWSGrid creation
  
  UpdateSorting();
  
  return true;
}

/*!
 * PWSGrid destructor
 */

PWSGrid::~PWSGrid()
{
////@begin PWSGrid destruction
////@end PWSGrid destruction
}

/*!
 * Member initialisation
 */

void PWSGrid::Init()
{
////@begin PWSGrid member initialisation
////@end PWSGrid member initialisation
}

/*!
 * Control creation for PWSGrid
 */

void PWSGrid::CreateControls()
{
////@begin PWSGrid content construction
////@end PWSGrid content construction
  CreateGrid(0, PWSGridTable::GetNumHeaderCols(), wxGrid::wxGridSelectRows);
  SetColLabelValue(0, _("Title"));
  SetColLabelValue(1, _("User"));
  SetRowLabelSize(0);
  int w,h;
  GetClientSize(&w, &h);
  int cw = w/2; // 2 = number of columns
  SetColSize(0, cw);
  SetColSize(1, cw);
}

void PWSGrid::OnPasswordListModified()
{
  m_row_map.clear();
  m_uuid_map.clear();

  ItemListConstIter iter;
  int row = 0;
  for (iter = m_core.GetEntryIter();
       iter != m_core.GetEntryEndIter();
       iter++) {
    AddItem(iter->second, row);
    row++;
  }
}

/*!
 * Should we show tooltips?
 */

bool PWSGrid::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap PWSGrid::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin PWSGrid bitmap retrieval
  return wxNullBitmap;
////@end PWSGrid bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon PWSGrid::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin PWSGrid icon retrieval
  return wxNullIcon;
////@end PWSGrid icon retrieval
}

void PWSGrid::AddItem(const CItemData &item, int row)
{
  int nRows = GetNumberRows();
  if (row == -1)
    row = nRows;
  uuid_array_t uuid;
  item.GetUUID(uuid);
  m_row_map.insert(std::make_pair(row, CUUID(uuid)));
  m_uuid_map.insert(std::make_pair(CUUID(uuid), row));
  InsertRows(row);
}

void PWSGrid::RefreshItem(const CItemData &item, int row)
{
  int nRows = GetNumberRows();
  if (row == -1)
    row = nRows;
  uuid_array_t uuid;
  item.GetUUID(uuid);
  m_row_map.insert(std::make_pair(row, CUUID(uuid)));
  m_uuid_map.insert(std::make_pair(CUUID(uuid), row));
  RefreshRow(row);
}

void PWSGrid::UpdateItem(const CItemData &item)
{
  uuid_array_t uuid;
  item.GetUUID(uuid);
  auto iter = m_uuid_map.find(CUUID(uuid));
  if (iter != m_uuid_map.end()) {
    int row = iter->second;
    DeleteRows(row);
    InsertRows(row);
  }
}

void PWSGrid::RefreshRow(int row)
{
  wxRect rect(CellToRect( row, 0 ));
  rect.x = 0;
  rect.width = GetGridWindow()->GetClientSize().GetWidth();
  int dummy;
  CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
  GetGridWindow()->Refresh( false, &rect );
}

void PWSGrid::RefreshItemRow(const pws_os::CUUID& uuid)
{
  const int row = FindItemRow(uuid);
  if (row != wxNOT_FOUND)
    RefreshRow(row);
}

void PWSGrid::RefreshItemField(const pws_os::CUUID& uuid, CItemData::FieldType ft)
{
  int row = FindItemRow(uuid);
  int col = PWSGridTable::Field2Column(ft);
  if (row != wxNOT_FOUND && col != wxNOT_FOUND && IsVisible(row, col, false)) {
    //last param is false to check if the required cell is even partially visible
    RefreshItemRow(uuid);
  }
}

struct moveup : public std::binary_function<UUIDRowMapT::value_type, int, void> {
  void operator()(UUIDRowMapT::value_type& v, int rowDeleted) const {
    if (v.second > rowDeleted)
      v.second = v.second - 1;
  }
};

void PWSGrid::Remove(const CUUID &uuid)
{
  auto iter = m_uuid_map.find(uuid);
  if (iter != m_uuid_map.end()) {
    const int row = iter->second;

    //The UI element must be removed first, since the entry in m_core is deleted after
    //this callback returns, and the deletion process might check for consistency
    //in number of entries in various UI maps and m_core
    DeleteRows(row);

    //move all the rows up by 1
    for (size_t newRow = row + 1; newRow < m_row_map.size(); ++newRow) {
      m_row_map[reinterpret_cast<int &>(newRow) - 1] = m_row_map[reinterpret_cast<int &>(newRow)];
    }

    //remove the last row, which is now extraneous
    m_row_map.erase(static_cast<int>(m_row_map.size()) - 1);

    //subtract the row values of all entries in uuid map if it is greater
    //than the row we just deleted
    std::for_each(m_uuid_map.begin(), m_uuid_map.end(), std::bind2nd(moveup(), row));

    //delete the item itself
    m_uuid_map.erase(iter);
  }
}

/*!
 * Returns the number of elements in its own books
 */
size_t PWSGrid::GetNumItems() const
{
  //These vars help with debugging.  Declared volatile to keep the compiler
  //from optimizing them away
  volatile size_t uuid_map_size = m_uuid_map.size();
  volatile size_t row_map_size = m_row_map.size();
  volatile size_t numEntries = m_core.GetNumEntries();

  //asserts below are commented out since it is unavoidable that the grid
  //receives a paint event between the time an entry is erased from
  //its maps and the corresponding row is erased from the grid UI
  if(!m_uuid_map.empty() && uuid_map_size != numEntries) {
    //wxFAIL_MSG(wxString() << wxT("uuid map has ") << uuid_map_size
    //                      << wxT(" entries, but core has ") << numEntries);
  }

  if (!m_row_map.empty() && row_map_size != numEntries) {
    //wxFAIL_MSG(wxString() << wxT("row map has ") << row_map_size
    //                      << wxT(" entries, but core has ") << numEntries);
  }
  //this is the most meaningful, to keep wxGrid from asking us
  //about rows that we don't have
  return row_map_size;
}

/*!
 * callback from PWSGridTable.  Removes the item(s) from its own books
 * as well as from PWSCore.  The display will be updated by an event
 * generated by PWSGridTable once this function returns.
 */
void PWSGrid::DeleteItems(int row, size_t numItems)
{
  for (size_t N = 0; N < numItems; ++N) {
    auto iter = m_row_map.find(row);
    if (iter != m_row_map.end()) {
      auto iter_uuid = m_uuid_map.find(iter->second);
      m_row_map.erase(iter);
      if (iter_uuid != m_uuid_map.end()) {
        uuid_array_t uuid;
        iter_uuid->first.GetARep(uuid);
        m_uuid_map.erase(iter_uuid);
        auto citer = m_core.Find(uuid);
        if (citer != m_core.GetEntryEndIter()){
          m_core.SuspendOnDBNotification();
          m_core.Execute(DeleteEntryCommand::Create(&m_core,
                                                    m_core.GetEntry(citer)));
          m_core.ResumeOnDBNotification();
        }
      }
    }
  }
  if (m_core.HasDBChanged())
    OnPasswordListModified();
}

/*!
 * callback from PWSGridTable.  Removes all items from its own books
 * as well as from PWSCore.  The items from display grid will be
 * removed by an event generated by PWSGridTable once this function
 * returns
 */
void PWSGrid::DeleteAllItems()
{
  m_uuid_map.clear();
  m_row_map.clear();
}

/*!
 * wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_LISTBOX
 */

void PWSGrid::OnCellRightClick( wxGridEvent& evt )
{
  // We need this function because wxGrid doesn't convert unprocessed
  // right-mouse-down events to contextmenu events, so we need to do
  // handle it ourselves
  //
  // This is what we should ideally do, but in the ContextMenu handler below,
  // I can't convert the mouse position to logical(x,y) to grid's row, column.
  // The row is always 1 more than where I click
  //
  // wxContextMenuEvent cme(wxEVT_CONTEXT_MENU, GetId(), event.GetPosition());
  // cme.SetEventObject(event.GetEventObject());
  // ProcessEvent(cme);
  //
  SetGridCursor(evt.GetRow(), evt.GetCol());
  SelectRow(evt.GetRow());
  dynamic_cast<PasswordSafeFrame *>(GetParent())->OnContextMenu(GetItem(evt.GetRow()));
}

/*!
 * wxEVT_GRID_CELL_ITEM_MENU event handler for ID_LISTBOX
 */

void PWSGrid::OnContextMenu( wxContextMenuEvent& evt )
{
  wxPoint pos = evt.GetPosition();
  if ( pos == wxDefaultPosition ) { //sent from keyboard?
    const int row = GetGridCursorRow();
    SelectRow(row);
    dynamic_cast<PasswordSafeFrame *>(GetParent())->OnContextMenu(GetItem(row));
  }
  else { //sent from mouse.  I don't know how to convert the mouse coords to grid's row,column
    evt.Skip();
  }
}

CItemData *PWSGrid::GetItem(int row) const
{
  if (row < 0 || row > const_cast<PWSGrid *>(this)->GetNumberRows())
    return nullptr;
  auto iter = m_row_map.find(row);
  if (iter != m_row_map.end()) {
    uuid_array_t uuid;
    iter->second.GetARep(uuid);
    auto itemiter = m_core.Find(uuid);
    if (itemiter == m_core.GetEntryEndIter())
      return nullptr;
    return &itemiter->second;
  }
  return nullptr;
}

/*!
 * wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_LISTBOX
 */

void PWSGrid::OnLeftDClick( wxGridEvent& evt )
{
  CItemData *item = GetItem(evt.GetRow());
  if (item != nullptr)
    dynamic_cast<PasswordSafeFrame *>(GetParent())->
      DispatchDblClickAction(*item);
}

 void PWSGrid::SelectItem(const CUUID & uuid)
 {
     UUIDRowMapT::const_iterator itr = m_uuid_map.find(uuid);
     if (itr != m_uuid_map.end()) {
         MakeCellVisible(itr->second, 0);
         wxGrid::SelectRow(itr->second);
     }
 }

int  PWSGrid::FindItemRow(const CUUID& uu)
{
     UUIDRowMapT::const_iterator itr = m_uuid_map.find(uu);
     if (itr != m_uuid_map.end()) {
       return itr->second;
     }
     return wxNOT_FOUND;
}

void PWSGrid::SaveSettings() const
{
  auto *table = dynamic_cast<PWSGridTable*>(GetTable());
  if (table)  //may not have been created/assigned
    table->SaveSettings();
}

void PWSGrid::OnDBGUIPrefsChange(wxEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  EnableGridLines(PWSprefs::GetInstance()->GetPref(PWSprefs::ListViewGridLines));
}

void PWSGrid::Clear()
{
  if (GetNumberRows() > 0) {
    BeginBatch();
    DeleteRows(0, GetNumberRows());
    EndBatch();
  }
  DeleteAllItems();
}

/*!
 * wxEVT_GRID_SELECT_CELL event handler for ID_LISTBOX
 */

void PWSGrid::OnSelectCell( wxGridEvent& evt )
{
  CItemData *pci = GetItem(evt.GetRow());

  dynamic_cast<PasswordSafeFrame *>(GetParent())->UpdateSelChanged(pci);
}

void PWSGrid::SetFilterState(bool state)
{
  const wxColour *colour = state ? wxRED : wxBLACK;
  SetDefaultCellTextColour(*colour);
  ForceRefresh();
}

void PWSGrid::UpdateSorting()
{
  SortByColumn(
    PWSprefs::GetInstance()->GetPref(PWSprefs::SortedColumn),
    PWSprefs::GetInstance()->GetPref(PWSprefs::SortAscending)
  );
}

void PWSGrid::OnHeaderClick(wxHeaderCtrlEvent& event)
{
  SortByColumn(event.GetColumn(), !IsSortOrderAscending());
  
  if (GetSortingColumn() != wxNOT_FOUND) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::SortedColumn , GetSortingColumn());
    PWSprefs::GetInstance()->SetPref(PWSprefs::SortAscending, IsSortOrderAscending());
  }
}

void PWSGrid::SortByColumn(int column, bool ascending)
{
  UnsetSortingColumn();
  
  SetSortingColumn(column, ascending);
  
  if (ascending) {
    AscendingSortedMultimap collection;
    
    RearrangeItems<AscendingSortedMultimap> (collection, column);
  }
  else {
    DescendingSortedMultimap collection;
    
    RearrangeItems<DescendingSortedMultimap> (collection, column);
  }
}

template<typename ItemsCollection>
void PWSGrid::RearrangeItems(ItemsCollection& collection, int column)
{
  int row = 0;
  
  for (row = 0; row < GetNumberRows(); row++) {
    collection.insert(std::pair<wxString, const CItemData*>(GetCellValue(row, column), GetItem(row)));
  }
  
  m_row_map.clear();
  m_uuid_map.clear();
  
  row = 0;
  
  for (auto& item : collection) {
    RefreshItem(*item.second, row++);
  }
}
