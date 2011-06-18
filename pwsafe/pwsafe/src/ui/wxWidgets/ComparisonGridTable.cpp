/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "../../core/PWScore.h"
#include "./wxutils.h"

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

///////////////////////////////////////////////////////////
// ComparisonGridTable
// base class for the other two types of grid table
ComparisonGridTable::ComparisonGridTable(SelectionCriteria* criteria): m_criteria(criteria),
                      m_colFields(new ColumnData[criteria->GetNumSelectedFields()-1])
{
  //this is the order in which we want to display the comparison grids
  const ColumnData columns[] = {{CItemData::PASSWORD, &CItemData::IsPasswordSet},
                                {CItemData::URL,      &CItemData::IsURLSet},
                                {CItemData::AUTOTYPE, &CItemData::IsAutoTypeSet},
                                {CItemData::PWHIST,   &CItemData::IsPasswordHistorySet},
                                {CItemData::RUNCMD,   &CItemData::IsRunCommandSet},
                                {CItemData::EMAIL,    &CItemData::IsEmailSet},
                                {CItemData::NOTES,    &CItemData::IsNotesSet}};

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

void ComparisonGridTable::AutoSizeField(CItemData::FieldType ft)
{
  int col = FieldToColumn(ft);
  if (col > 0 && col < GetNumberCols())
    GetView()->AutoSizeColumn(col);
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
//    wxLogDebug(wxT("UniSafeCompareGridTable::IsEmptyCell returning %s for %d, %d"), ToStr(retval), row, col);
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
//  wxLogDebug(wxT("UniSafeCompareGridTable::GetValue returning %s for %d, %d"), ToStr(retval), row, col);
  return retval;
}

wxGridCellAttr* UniSafeCompareGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind /*kind*/)
{
  //wxLogDebug(wxT("UniSafeCompareGridTable::GetAttr called for %d, %d"), row, col);
  m_gridAttr->IncRef();
  return m_gridAttr;
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
//    wxLogDebug(wxT("MultiSafeCompareGridTable::IsEmptyCell returning %s for %d, %d"), ToStr(retval), row, col);
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
//  wxLogDebug(wxT("MultiSafeCompareGridTable::GetValue returning %s for %d, %d"), ToStr(retval), row, col);
  return retval;
}

wxGridCellAttr* MultiSafeCompareGridTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind /*kind*/)
{
  //wxLogDebug(wxT("MultiSafeCompareGridTable::GetAttr called for %d, %d"), row, col);
  wxGridCellAttr* attr = ( row%2 == 0? m_currentAttr: m_comparisonAttr );
  attr->IncRef();
  return attr;
}

wxString MultiSafeCompareGridTable::GetRowLabelValue(int row)
{
  //donot show row labels in odd number rows in conflict grid
  if (row%2 == 1)
    return wxEmptyString;
  return wxGridTableBase::GetRowLabelValue(row/2);
}

//////////////////////////////////////////////////////////////////
//ComparisonGrid
ComparisonGrid::ComparisonGrid(wxWindow* parent, wxWindowID id): wxGrid(parent, id)
{
  int horiz, vert;
  GetRowLabelAlignment(&horiz, &vert);
  SetRowLabelAlignment(horiz, wxALIGN_BOTTOM);
}

wxPen ComparisonGrid::GetRowGridLinePen(int row)
{
  return row%2 == 0? wxPen(CurrentBackgroundColor, 1, wxSOLID): *wxBLACK_PEN;
}

