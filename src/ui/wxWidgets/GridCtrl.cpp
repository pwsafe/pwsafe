/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file GridCtrl.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

////@begin includes
////@end includes

#include "GridCtrl.h"
#include "GridTable.h"
#include "PasswordSafeFrame.h" // for DispatchDblClickAction()
#include "PWSafeApp.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

using pws_os::CUUID;

/*!
 * GridCtrl type definition
 */

IMPLEMENT_CLASS( GridCtrl, wxGrid )

/*!
 * GridCtrl event table definition
 */

BEGIN_EVENT_TABLE( GridCtrl, wxGrid )

////@begin GridCtrl event table entries
  EVT_GRID_CELL_RIGHT_CLICK( GridCtrl::OnCellRightClick )
  EVT_GRID_CELL_LEFT_DCLICK( GridCtrl::OnLeftDClick )
  EVT_GRID_SELECT_CELL( GridCtrl::OnSelectCell )
  EVT_CONTEXT_MENU(GridCtrl::OnContextMenu)
////@end GridCtrl event table entries

END_EVENT_TABLE()

/*!
 * GridCtrl constructors
 */

GridCtrl::GridCtrl(PWScore &core) : m_core(core)
{
  Init();
}

GridCtrl::GridCtrl(wxWindow* parent, PWScore &core,
                 wxWindowID id, const wxPoint& pos,
                 const wxSize& size, long style) : m_core(core)
{
  Init();
  Create(parent, id, pos, size, style);

  auto *header = wxGrid::GetGridColHeader();

  if (header) {

    // Handler for double click events on column header separator.
    header->Bind(
      wxEVT_HEADER_SEPARATOR_DCLICK, 
      [=](wxHeaderCtrlEvent& event) {
        wxGrid::AutoSizeColumn(event.GetColumn());
      }
    );

    // Handler for single click events on column header.
    header->Bind(
      wxEVT_HEADER_CLICK, 
      &GridCtrl::OnHeaderClick,
      this
    );
  }

  // Handler for mouse right click events outside of grid, resp. within grid window.
  GetGridWindow()->Bind(
#ifdef __WINDOWS__
    wxEVT_RIGHT_UP,
#else
    wxEVT_RIGHT_DOWN,
#endif
    &GridCtrl::OnMouseRightClick,
    this
  );

  // Handler for mouse left click events outside of grid, resp. within grid window.
  GetGridWindow()->Bind(
    wxEVT_LEFT_DOWN,
    &GridCtrl::OnMouseLeftClick,
    this
  );
}

/*!
 * GridCtrl creator
 */

bool GridCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin GridCtrl creation
  wxGrid::Create(parent, id, pos, size, style);
  CreateControls();
  EnableGridLines(PWSprefs::GetInstance()->GetPref(PWSprefs::ListViewGridLines));
  EnableDragColMove(true);

  //column picker is free if built with wx2.9.1
#if wxCHECK_VERSION(2, 9, 1)
  UseNativeColHeader(true);
#endif
////@end GridCtrl creation

  UpdateSorting();

  return true;
}

/*!
 * GridCtrl destructor
 */

GridCtrl::~GridCtrl()
{
////@begin GridCtrl destruction
////@end GridCtrl destruction
}

/*!
 * Member initialisation
 */

void GridCtrl::Init()
{
////@begin GridCtrl member initialisation
////@end GridCtrl member initialisation
}

/*!
 * Control creation for GridCtrl
 */

void GridCtrl::CreateControls()
{
////@begin GridCtrl content construction
////@end GridCtrl content construction
  CreateGrid(0, GridTable::GetNumHeaderCols(), wxGrid::wxGridSelectRows);
  SetColLabelValue(0, _("Title"));
  SetColLabelValue(1, _("User"));
  SetRowLabelSize(0);
  int w,h;
  GetClientSize(&w, &h);
  int cw = w/2; // 2 = number of columns
  SetColSize(0, cw);
  SetColSize(1, cw);
}

/**
 * Implements Observer::UpdateGUI(UpdateGUICommand::GUI_Action, const pws_os::CUUID&, CItemData::FieldType)
 */
void GridCtrl::UpdateGUI(UpdateGUICommand::GUI_Action ga, const pws_os::CUUID &entry_uuid, CItemData::FieldType ft)
{
  CItemData *item = nullptr;

  ItemListIter itemIterator = m_core.Find(entry_uuid);

  if (itemIterator != m_core.GetEntryEndIter()) {
    item = &itemIterator->second;
  }
  else if (ga == UpdateGUICommand::GUI_ADD_ENTRY ||
           ga == UpdateGUICommand::GUI_REFRESH_ENTRYFIELD ||
           ga == UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD) {
    pws_os::Trace(wxT("GridCtrl - Couldn't find uuid %ls"), StringX(CUUID(entry_uuid)).c_str());
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
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRYFIELD:
    case UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD:
      ASSERT(item != nullptr);
      RefreshItemField(item->GetUUID(), ft);
      break;
    case UpdateGUICommand::GUI_REDO_IMPORT:
    case UpdateGUICommand::GUI_UNDO_IMPORT:
    case UpdateGUICommand::GUI_REDO_MERGESYNC:
    case UpdateGUICommand::GUI_UNDO_MERGESYNC:
      // Handled by PasswordSafeFrame
      break;
    case UpdateGUICommand::GUI_REFRESH_TREE:
      // Not relevant for this view
      break;
    case UpdateGUICommand::GUI_REFRESH_ENTRY:
      ASSERT(item != nullptr);
      RefreshItem(*item);
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
      wxFAIL_MSG(wxT("GridCtrl - Unsupported GUI action received."));
      break;
  }
}

/**
 * Implements Observer::GUIRefreshEntry(const CItemData&, bool)
 */
void GridCtrl::GUIRefreshEntry(const CItemData &item, bool WXUNUSED(bAllowFail))
{
  pws_os::Trace(wxT("GridCtrl::GUIRefreshEntry"));

  if (item.GetStatus() == CItemData::ES_DELETED) {
    uuid_array_t uuid;
    item.GetUUID(uuid);
    Remove(uuid);
  }
  else {
    this->UpdateItem(item);
  }
}

void GridCtrl::OnPasswordListModified()
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

bool GridCtrl::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap GridCtrl::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin GridCtrl bitmap retrieval
  return wxNullBitmap;
////@end GridCtrl bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon GridCtrl::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin GridCtrl icon retrieval
  return wxNullIcon;
////@end GridCtrl icon retrieval
}

void GridCtrl::AddItem(const CItemData &item, int row)
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

void GridCtrl::RefreshItem(const CItemData &item, int row)
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

void GridCtrl::UpdateItem(const CItemData &item)
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

void GridCtrl::RefreshRow(int row)
{
  wxRect rect(CellToRect( row, 0 ));
  rect.x = 0;
  rect.width = GetGridWindow()->GetClientSize().GetWidth();
  int dummy;
  CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
  GetGridWindow()->Refresh( false, &rect );
}

void GridCtrl::RefreshItemRow(const pws_os::CUUID& uuid)
{
  const int row = FindItemRow(uuid);
  if (row != wxNOT_FOUND)
    RefreshRow(row);
}

void GridCtrl::RefreshItemField(const pws_os::CUUID& uuid, CItemData::FieldType ft)
{
  int row = FindItemRow(uuid);
  int col = GridTable::Field2Column(ft);
  if (row != wxNOT_FOUND && col != wxNOT_FOUND && IsVisible(row, col, false)) {
    //last param is false to check if the required cell is even partially visible
    RefreshItemRow(uuid);
  }
}

void GridCtrl::Remove(const CUUID &uuid)
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
      m_row_map[static_cast<int>(newRow) - 1] = m_row_map[static_cast<int>(newRow)];
    }

    //remove the last row, which is now extraneous
    m_row_map.erase(static_cast<int>(m_row_map.size()) - 1);

    //subtract the row values of all entries in uuid map if it is greater
    //than the row we just deleted
    std::for_each(m_uuid_map.begin(), m_uuid_map.end(), [row](UUIDRowMapT::value_type &v){if (v.second > row) v.second -= 1;} );

    //delete the item itself
    m_uuid_map.erase(iter);
  }
}

/*!
 * Returns the number of elements in its own books
 */
size_t GridCtrl::GetNumItems() const
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
 * callback from GridTable.  Removes the item(s) from its own books
 * as well as from PWSCore.  The display will be updated by an event
 * generated by GridTable once this function returns.
 */
void GridCtrl::DeleteItems(int row, size_t numItems)
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
 * callback from GridTable.  Removes all items from its own books
 * as well as from PWSCore.  The items from display grid will be
 * removed by an event generated by GridTable once this function
 * returns
 */
void GridCtrl::DeleteAllItems()
{
  m_uuid_map.clear();
  m_row_map.clear();
}

/*!
 * wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_LISTBOX
 */

void GridCtrl::OnCellRightClick( wxGridEvent& evt )
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
  wxGetApp().GetPasswordSafeFrame()->OnContextMenu(GetItem(evt.GetRow()));
}

/*!
 * wxEVT_GRID_CELL_ITEM_MENU event handler for ID_LISTBOX
 */

void GridCtrl::OnContextMenu( wxContextMenuEvent& evt )
{
  wxPoint pos = evt.GetPosition();
  if ( pos == wxDefaultPosition ) { //sent from keyboard?
    const int row = GetGridCursorRow();
    SelectRow(row);
    wxGetApp().GetPasswordSafeFrame()->OnContextMenu(GetItem(row));
  }
  else { //sent from mouse.  I don't know how to convert the mouse coords to grid's row,column
    evt.Skip();
  }
}

CItemData *GridCtrl::GetItem(int row) const
{
  if (row < 0 || row > const_cast<GridCtrl *>(this)->GetNumberRows())
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

void GridCtrl::OnLeftDClick( wxGridEvent& evt )
{
  CItemData *item = GetItem(evt.GetRow());
  if (item != nullptr)
    wxGetApp().GetPasswordSafeFrame()->DispatchDblClickAction(*item);
}

 void GridCtrl::SelectItem(const CUUID & uuid)
 {
     UUIDRowMapT::const_iterator itr = m_uuid_map.find(uuid);
     if (itr != m_uuid_map.end()) {
         MakeCellVisible(itr->second, 0);
         wxGrid::SelectRow(itr->second);
     }
 }

int  GridCtrl::FindItemRow(const CUUID& uu)
{
     UUIDRowMapT::const_iterator itr = m_uuid_map.find(uu);
     if (itr != m_uuid_map.end()) {
       return itr->second;
     }
     return wxNOT_FOUND;
}

void GridCtrl::SaveSettings() const
{
  auto *table = dynamic_cast<GridTable*>(GetTable());
  if (table)  //may not have been created/assigned
    table->SaveSettings();
}

void GridCtrl::PreferencesChanged()
{
  EnableGridLines(PWSprefs::GetInstance()->GetPref(PWSprefs::ListViewGridLines));
}

void GridCtrl::Clear()
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

void GridCtrl::OnSelectCell( wxGridEvent& evt )
{
  CItemData *pci = GetItem(evt.GetRow());

  wxGetApp().GetPasswordSafeFrame()->UpdateSelChanged(pci);
}

void GridCtrl::SetFilterState(bool state)
{
  const wxColour *colour = state ? wxRED : wxBLACK;
  SetDefaultCellTextColour(*colour);
  ForceRefresh();
}

void GridCtrl::UpdateSorting()
{
  SortByColumn(
    PWSprefs::GetInstance()->GetPref(PWSprefs::SortedColumn),
    PWSprefs::GetInstance()->GetPref(PWSprefs::SortAscending)
  );
}

void GridCtrl::OnHeaderClick(wxHeaderCtrlEvent& event)
{
  SortByColumn(event.GetColumn(), !IsSortOrderAscending());

  if (GetSortingColumn() != wxNOT_FOUND) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::SortedColumn , GetSortingColumn());
    PWSprefs::GetInstance()->SetPref(PWSprefs::SortAscending, IsSortOrderAscending());
  }
}

void GridCtrl::OnMouseRightClick(wxMouseEvent& event)
{
  auto gridCellInfo = HitTest(event.GetPosition());

  /* check whether a grid cell was hit */
  if (!HasGridCell(gridCellInfo)) {
    ClearSelection();
    wxGetApp().GetPasswordSafeFrame()->OnContextMenu(nullptr);
  }
  else {
    event.Skip();
  }
}

void GridCtrl::OnMouseLeftClick(wxMouseEvent& event)
{
  auto gridCellInfo = HitTest(event.GetPosition());

  /* check whether a grid cell was hit */
  if (!HasGridCell(gridCellInfo)) {
    ClearSelection();
  }
  else {
    event.Skip();
  }
}

void GridCtrl::SortByColumn(int column, bool ascending)
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
void GridCtrl::RearrangeItems(ItemsCollection& collection, int column)
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

/**
 * Determines whether a cell or row is below the specified position.
 * 
 * @param point reflects the position to be checked for a grid cell.
 * @return the cell index and row index as tuple.
 *         -1 indicates that no grid cell exists at the given position.
 */
std::tuple<int, int> GridCtrl::HitTest(const wxPoint& point) const
{
  auto gridCellCoordinates = XYToCell(point);

  return std::make_tuple(gridCellCoordinates.GetCol(), gridCellCoordinates.GetRow());
}

/**
 * Determines whether a grid cell exists at given grid coordinates.
 * 
 * @param cellGridCoordinates the column and row index pointing possibly to a grid cell.
 * @return true if a grid cell exists at the given coordinates, otherwise false.
 */
bool GridCtrl::HasGridCell(const std::tuple<int, int>& cellGridCoordinates) const
{
  return !((std::get<0>(cellGridCoordinates) < 0) && (std::get<1>(cellGridCoordinates) < 0));
}
