/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ComparisonGridTable.cpp
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

#include "./ComparisonGridTable.h"

#include "./AdvancedSelectionDlg.h"
#include "./SelectionCriteria.h"
#include "../../core/PWScore.h"
#include "./wxutils.h"
#include <algorithm>
#include <functional>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

class ComparisonGridCellAttr: public wxGridCellAttr
{
  ComparisonGridCellAttr();
  DECLARE_NO_COPY_CLASS(ComparisonGridCellAttr)

public:
  ComparisonGridCellAttr(ComparisonGridTable* table): m_table(table)
  {}

private:
  ComparisonGridTable* m_table;
};

struct st_CompareData_match : public std::binary_function<st_CompareData, pws_os::CUUID, bool>
{
  result_type operator()(const first_argument_type& arg1, const second_argument_type& arg2) const
  {
    return arg1.uuid0 == arg2 || arg1.uuid1 == arg2;
  }
};

///////////////////////////////////////////////////////////
// ComparisonGridTable
// base class for the other two types of grid table
ComparisonGridTable::ComparisonGridTable(SelectionCriteria* criteria): m_criteria(criteria),
                      m_colFields(new ColumnData[criteria->GetNumSelectedFields()-1])
{
  //this is the order in which we want to display the comparison grids
  const ColumnData columns[] = {{CItemData::PASSWORD,     &CItemData::IsPasswordSet},
                                {CItemData::URL,          &CItemData::IsURLSet},
                                {CItemData::AUTOTYPE,     &CItemData::IsAutoTypeSet},
                                {CItemData::PWHIST,       &CItemData::IsPasswordHistorySet},
                                {CItemData::RUNCMD,       &CItemData::IsRunCommandSet},
                                {CItemData::EMAIL,        &CItemData::IsEmailSet},
                                {CItemData::NOTES,        &CItemData::IsNotesSet},
                                {CItemData::POLICY,       &CItemData::IsPasswordPolicySet},
                                {CItemData::POLICYNAME,   &CItemData::IsPolicyNameSet},
                                {CItemData::SYMBOLS,      &CItemData::IsSymbolsSet},
                                {CItemData::DCA,          &CItemData::IsDCASet},
                                {CItemData::SHIFTDCA,     &CItemData::IsShiftDCASet},
                                {CItemData::PROTECTED,    &CItemData::IsProtectionSet},
                                {CItemData::CTIME,        &CItemData::IsCreationTimeSet},
                                {CItemData::ATIME,        &CItemData::IsLastAccessTimeSet},
                                {CItemData::XTIME,        &CItemData::IsExpiryDateSet},
                                {CItemData::XTIME_INT,    &CItemData::IsPasswordExpiryIntervalSet},
                                {CItemData::PMTIME,       &CItemData::IsModificationTimeSet},
                                {CItemData::RMTIME,       &CItemData::IsRecordModificationTimeSet},
                              };

  const size_t ncols = size_t(GetNumberCols());
  for(size_t col = 0, idx = 0; (idx < WXSIZEOF(columns)) && (col < ncols); idx++) {
    if (m_criteria->IsFieldSelected(columns[idx].ft))
      m_colFields[col++] = columns[idx];
  }
}

ComparisonGridTable::~ComparisonGridTable()
{
  delete [] m_colFields;
  m_criteria = 0;
}

int ComparisonGridTable::GetNumberCols()
{
  //GetSelectedFields() will return at-least G+T+U
  //We club T & U and display only Group + title[user] + fields
  //Hence return one less
  return m_criteria->GetNumSelectedFields() - 1; 
}

void ComparisonGridTable::SetValue(int /*row*/, int /*col*/, const wxString& /*value*/)
{
}

wxString ComparisonGridTable::GetColLabelValue(int col)
{
  switch(col) {
    case 0:
      return towxstring(CItemData::FieldName(CItemData::GROUP));
    case 1:
    {
      wxString label;
      label << CItemData::FieldName(CItemData::TITLE) << wxT('[') 
                        << CItemData::FieldName(CItemData::USER) << wxT(']');
      return label;
    }
    default:
      return towxstring(CItemData::FieldName(m_colFields[col-2].ft));
  }
}

int ComparisonGridTable::FieldToColumn(CItemData::FieldType ft)
{
  switch(ft) {
    case CItemData::GROUP:
      return 0;
    case CItemData::USER:
    case CItemData::TITLE:
      return 1;
    default:
      for( int col = 0; col < GetNumberCols(); ++col)
        if (m_colFields[col].ft == ft)
          return col + 2;
      break;
  }
  return -1;
}

CItemData::FieldType ComparisonGridTable::ColumnToField(int col)
{
  switch(col) {
    case 0:
      return CItemData::GROUP;
    case 1:
      return CItemData::USER;
    default:
      if (col > 0 && col < GetNumberCols())
        return m_colFields[col-2].ft;
      break;
  }
  return CItem::LAST_DATA;
}

void ComparisonGridTable::AutoSizeField(CItemData::FieldType ft)
{
  int col = FieldToColumn(ft);
  if (col > 0 && col < GetNumberCols())
    GetView()->AutoSizeColumn(col);
}

void ComparisonGridTable::RefreshRow(int row) const
{
  wxRect rect( GetView()->CellToRect( row, 0 ) );
  rect.x = 0;
  rect.width = GetView()->GetGridWindow()->GetClientSize().GetWidth();
  int dummy;
  GetView()->CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
  GetView()->GetGridWindow()->Refresh( false, &rect );
}

///////////////////////////////////////////////////////////////
//UniSafeCompareGridTable
//
UniSafeCompareGridTable::UniSafeCompareGridTable(SelectionCriteria* criteria, 
                                                 CompareData* data,
                                                 PWScore* core,
                                                 uuid_ptr pu,
                                                 const wxColour& bgColour): ComparisonGridTable(criteria),
                                                                            m_compData(data),
                                                                            m_core(core),
                                                                            m_gridAttr(new wxGridCellAttr),
                                                                            m_uuidptr(pu)
{
  m_gridAttr->SetBackgroundColour(bgColour);
  m_gridAttr->SetOverflow(false);
}

UniSafeCompareGridTable::~UniSafeCompareGridTable()
{
  m_gridAttr->DecRef();
}

int UniSafeCompareGridTable::GetNumberRows()
{
  const size_t N = m_compData->size();
  assert(N <= size_t(std::numeric_limits<int>::max()));
  return int(N);
}

bool UniSafeCompareGridTable::IsEmptyCell(int row, int col)
{
  if (row < 0 || size_t(row) >= m_compData->size())
    return true;
  if (col < 0 || col > GetNumberCols())
    return true;

  if (col == 1)
    return false; //title is always present

  const st_CompareData& cd = m_compData->at(row);

  ItemListConstIter itr = m_core->Find(cd.*m_uuidptr);
  if (itr != m_core->GetEntryEndIter()) {
    const CItemData& item = itr->second;
    AvailableFunction available = col == 0? &CItemData::IsGroupSet: m_colFields[col-2].available;
    const bool empty = !(item.*available)();
//    wxLogDebug(wxT("UniSafeCompareGridTable::IsEmptyCell returning %ls for %d, %d"), ToStr(retval), row, col);
    return empty;
  }
  return true;
}

wxString UniSafeCompareGridTable::GetValue(int row, int col)
{
  wxString retval = wxEmptyString;
  if (size_t(row) < m_compData->size() && col < GetNumberCols()) {
    switch(col) {
      case 0:
        retval = towxstring(m_compData->at(row).group);
        break;
      case 1:
      {
        retval << m_compData->at(row).title << wxT('[') <<  m_compData->at(row).user << wxT(']');
        break;
      }
      default:
      {
        const st_CompareData& cd = m_compData->at(row);
        ItemListConstIter itr = m_core->Find(cd.*m_uuidptr);
        if ((itr != m_core->GetEntryEndIter()) && (itr->second.*m_colFields[col-2].available)())
          retval = towxstring(itr->second.GetFieldValue(m_colFields[col-2].ft));
        break;
      }
    }
  }
//  wxLogDebug(wxT("UniSafeCompareGridTable::GetValue returning %ls for %d, %d"), ToStr(retval), row, col);
  return retval;
}

wxGridCellAttr* UniSafeCompareGridTable::GetAttr(int /*row*/, int /*col*/, wxGridCellAttr::wxAttrKind /*kind*/)
{
  //wxLogDebug(wxT("UniSafeCompareGridTable::GetAttr called for %d, %d"), row, col);
  m_gridAttr->IncRef();
  return m_gridAttr;
}

int UniSafeCompareGridTable::GetItemRow(const pws_os::CUUID& uuid) const
{
  CompareData::iterator itr = std::find_if(m_compData->begin(),
                                            m_compData->end(),
                                            std::bind2nd(st_CompareData_match(), uuid));
  if (itr != m_compData->end())
    return std::distance(m_compData->begin(), itr);
  else
    return wxNOT_FOUND;
}

pws_os::CUUID UniSafeCompareGridTable::GetSelectedItemId(bool readOnly)
{
  wxArrayInt selection = GetView()->GetSelectedRows();
  wxCHECK_MSG(!selection.IsEmpty(), pws_os::CUUID::NullUUID(), wxT("Trying to retrieve selected item id when nothing is selected"));
  if (readOnly)
    return m_compData->at(selection[0]).uuid1;
  else
    return m_compData->at(selection[0]).uuid0;
}

bool UniSafeCompareGridTable::DeleteRows(size_t pos, size_t numRows)
{
  size_t curNumRows = m_compData->size();

  if (pos > curNumRows) {
    wxFAIL_MSG( wxString::Format(
                 wxT("Called UniSafeCompareGridTable::DeleteRows(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu rows"),
                 static_cast<unsigned int>(pos),
                 static_cast<unsigned int>(numRows),
                 static_cast<unsigned int>(curNumRows)
                 ));
    return false;
  }

  if (numRows > curNumRows - pos)
    numRows = curNumRows - pos;

  CompareData::iterator from = m_compData->begin(), to = m_compData->begin();
  std::advance(from, pos);
  std::advance(to, pos + numRows);
  m_compData->erase(from, to);

  if (GetView()) {
    //This will actually remove the item from grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           reinterpret_cast<int &>(pos),
                           reinterpret_cast<int &>(numRows));
    GetView()->ProcessTableMessage(msg);
  }

  return true;  
}

bool UniSafeCompareGridTable::AppendRows(size_t numRows/*=1*/)
{
  if (GetView()) {
    wxCHECK_MSG(m_compData->size() == GetView()->GetNumberRows() + numRows, 
                false,
                wxT("Items must be added to UnisafeComparisonGridTable's data before adding rows"));
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                           reinterpret_cast<int &>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  return true;
}

///////////////////////////////////////////////////////////////////
//MultiSafeCompareGridTable
MultiSafeCompareGridTable::MultiSafeCompareGridTable(SelectionCriteria* criteria,
                                                     CompareData* data,
                                                     PWScore* current,
                                                     PWScore* other)
                                                           :ComparisonGridTable(criteria),
                                                            m_compData(data),
                                                            m_currentCore(current),
                                                            m_otherCore(other),
                                                            m_currentAttr(new wxGridCellAttr),
                                                            m_comparisonAttr(new wxGridCellAttr)
{
  m_currentAttr->SetBackgroundColour(CurrentBackgroundColor);
  m_comparisonAttr->SetBackgroundColour(ComparisonBackgroundColor);

  m_currentAttr->SetOverflow(false);
  m_comparisonAttr->SetOverflow(false);

  m_currentAttr->SetAlignment(wxALIGN_LEFT, wxALIGN_BOTTOM);
  m_comparisonAttr->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
}

MultiSafeCompareGridTable::~MultiSafeCompareGridTable()
{
  m_currentAttr->DecRef();
  m_comparisonAttr->DecRef();
}

int MultiSafeCompareGridTable::GetNumberRows()
{
  const size_t N = m_compData->size()*2;
  assert(m_compData->size() <= size_t(std::numeric_limits<int>::max())/2);
  return int(N);
}

PWScore* MultiSafeCompareGridTable::GetRowCore(int row)
{
  return (row%2 == 0)? m_currentCore: m_otherCore;
}

bool MultiSafeCompareGridTable::IsEmptyCell(int row, int col)
{
  PWScore* core = GetRowCore(row);

  row = row/2;

  if (row < 0 || size_t(row) > m_compData->size())
    return true;
  if (col < 0 || col > GetNumberCols())
    return true;

  if (col == 1)
    return false; //title is always present

  const st_CompareData& cd = m_compData->at(row);

  ItemListConstIter itr = core->Find(core == m_currentCore? cd.uuid0: cd.uuid1);
  if (itr != core->GetEntryEndIter()) {
    const CItemData& item = itr->second;
    AvailableFunction available = col == 0? &CItemData::IsGroupSet: m_colFields[col-2].available;
    bool empty = !(item.*available)();
//    wxLogDebug(wxT("MultiSafeCompareGridTable::IsEmptyCell returning %ls for %d, %d"), ToStr(retval), row, col);
    return empty;
  }

  return true;
}

wxString MultiSafeCompareGridTable::GetValue(int row, int col)
{
  wxString retval = wxEmptyString;

  const int origRow = row;

  row = row/2;

  if (size_t(row) < m_compData->size() && col < GetNumberCols()) {
    switch(col) {
      case 0:
        retval = towxstring(m_compData->at(row).group);
        break;
      case 1:
      {
        retval << m_compData->at(row).title << wxT('[') <<  m_compData->at(row).user << wxT(']');
        break;
      }
      default:
      {
        PWScore* core = GetRowCore(origRow);
        const st_CompareData& cd = m_compData->at(row);
        ItemListConstIter itr = core->Find(core == m_currentCore? cd.uuid0: cd.uuid1);
        if ((itr != core->GetEntryEndIter()) && (itr->second.*m_colFields[col-2].available)())
          retval = towxstring(itr->second.GetFieldValue(m_colFields[col-2].ft));
        break;
      }
    }
  }
//  wxLogDebug(wxT("MultiSafeCompareGridTable::GetValue returning %ls for %d, %d"), ToStr(retval), row, col);
  return retval;
}

wxGridCellAttr* MultiSafeCompareGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind /*kind*/)
{
  //wxLogDebug(wxT("MultiSafeCompareGridTable::GetAttr called for %d, %d"), row, col);
  wxGridCellAttr* attr = ( row%2 == 0? m_currentAttr: m_comparisonAttr );
  int idx = row/2;
  if (m_compData->at(idx).bsDiffs.test(ColumnToField(col))) {
    wxGridCellAttr* diffAttr = attr->Clone();
    diffAttr->SetTextColour(*wxRED);
    return diffAttr;
  }
  else {
    attr->IncRef();
    return attr;
  }
}

wxString MultiSafeCompareGridTable::GetRowLabelValue(int row)
{
  //do not show row labels in odd number rows in conflict grid
  if (row%2 == 1)
    return wxEmptyString;
  return wxGridTableBase::GetRowLabelValue(row/2);
}

int MultiSafeCompareGridTable::GetItemRow(const pws_os::CUUID& uuid) const
{
  CompareData::iterator itr = std::find_if(m_compData->begin(),
                                            m_compData->end(),
                                            std::bind2nd(st_CompareData_match(), uuid));
  if (itr != m_compData->end())
    return std::distance(m_compData->begin(), itr)*2 + (itr->uuid0 == uuid? 0: 1);
  else
    return wxNOT_FOUND;
}

pws_os::CUUID MultiSafeCompareGridTable::GetSelectedItemId(bool readOnly)
{
  wxArrayInt selection = GetView()->GetSelectedRows();
  wxCHECK_MSG(!selection.IsEmpty(), pws_os::CUUID::NullUUID(), wxT("Trying to retrieve selected item id when nothing is selected"));
  if (readOnly)
    return m_compData->at(selection[0]/2).uuid1;
  else
    return m_compData->at(selection[0]/2).uuid0;
}

bool MultiSafeCompareGridTable::DeleteRows(size_t pos, size_t numRows)
{
  //this is what gets deleted in the vector
  size_t datapos = pos/2;
  size_t datarows = numRows/2 + numRows %2;

  size_t curNumRows = m_compData->size();

  if (datapos > curNumRows) {
    wxFAIL_MSG( wxString::Format(
                 wxT("Called MultiSafeCompareGridTable::DeleteRows(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu rows"),
                 static_cast<unsigned int>(pos),
                 static_cast<unsigned int>(numRows),
                 static_cast<unsigned int>(curNumRows)
                 ));
    return false;
  }

  if (datarows > curNumRows - datapos)
    datarows = curNumRows - datapos;

  CompareData::iterator from = m_compData->begin(), to = m_compData->begin();
  std::advance(from, datapos);
  std::advance(to, datapos + datarows);
  m_compData->erase(from, to);

  if (GetView()) {
    //make sure an even number of rows are deleted
    numRows = numRows + numRows%2;
    //This will actually remove the item from grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           reinterpret_cast<int &>(pos),
                           reinterpret_cast<int &>(numRows));
    GetView()->ProcessTableMessage(msg);
  }

  return true;  
}

//////////////////////////////////////////////////////////////////
//ComparisonGrid

DEFINE_EVENT_TYPE(EVT_SELECT_GRID_ROW)

BEGIN_EVENT_TABLE( ComparisonGrid, wxGrid )
  EVT_GRID_RANGE_SELECT(ComparisonGrid::OnGridRangeSelect)
  EVT_COMMAND(wxID_ANY, EVT_SELECT_GRID_ROW, ComparisonGrid::OnAutoSelectGridRow)
END_EVENT_TABLE()

ComparisonGrid::ComparisonGrid(wxWindow* parent, wxWindowID id): wxGrid(parent, id)
{
  int horiz, vert;
  GetRowLabelAlignment(&horiz, &vert);
  SetRowLabelAlignment(horiz, wxALIGN_BOTTOM);
}

bool ComparisonGrid::IsRowSelected(int row) const
{
  //this works only if we are in wxGridSelectRows mode
  return IsInSelection(row, 0);
}

//remove the grid line between every other row to make two rows appear as one
wxPen ComparisonGrid::GetRowGridLinePen(int row)
{
  if (row%2 == 0) {
    if (IsRowSelected(row)) {
      if (wxWindow::FindFocus() == GetGridWindow()) {
        return wxPen(GetSelectionBackground());
      }
      else {
        //just like it is hard-coded in wxGridCellRenderer::Draw()
        return wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
      }
    }
    else {
      return wxPen(GetCellBackgroundColour(row,0));
    }
  }
  return wxGrid::GetRowGridLinePen(row);
}

void ComparisonGrid::OnGridRangeSelect(wxGridRangeSelectEvent& evt)
{
  //select grids asynchronously, or else we get in an infinite loop of selections & their notifications
  if (evt.GetTopRow()%2 != 0) {
    wxCommandEvent cmdEvent(EVT_SELECT_GRID_ROW);
    cmdEvent.SetEventObject(evt.GetEventObject());
    cmdEvent.SetInt(evt.GetTopRow()-1);
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
  if (evt.GetBottomRow()%2 == 0) {
    wxCommandEvent cmdEvent(EVT_SELECT_GRID_ROW);
    cmdEvent.SetEventObject(evt.GetEventObject());
    cmdEvent.SetInt(evt.GetBottomRow()+1);
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
}

void ComparisonGrid::OnAutoSelectGridRow(wxCommandEvent& evt)
{
  SelectRow(evt.GetInt(), true);
}
