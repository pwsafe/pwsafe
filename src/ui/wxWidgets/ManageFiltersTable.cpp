/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManageFiltersTable.cpp
* 
*/

////@begin includes

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "wx/string.h"
#include "wx/grid.h"

#include "ManageFiltersTable.h"
#include "PWFiltersEditor.h"

////@end includes

////@begin XPM images
////@end XPM images

/*!
 * pwManageFiltersTable type definition
 */

IMPLEMENT_CLASS(pwManageFiltersTable, wxGridTableBase)

/*!
 * pwManageFiltersTable constructor
 */

pwManageFiltersTable::pwManageFiltersTable(std::vector<struct st_FilterItemData> &data) : m_pdata(&data)
{
}


/*!
 * pwManageFiltersTable destructor
 */

pwManageFiltersTable::~pwManageFiltersTable()
{
}


/*!
 * wxGridTableBase override implementations
 * --------------------------------------------------
 */

/*!
 * Get number of rows
 */

int pwManageFiltersTable::GetNumberRows()
{
  return static_cast<int>(m_pdata->size());
}


/*!
 * Get number of columns
 */

int pwManageFiltersTable::GetNumberCols()
{
  return MFLC_NUM_COLUMNS;
}


/*!
 * Is cell empty - no cell is left empty
 */

bool pwManageFiltersTable::IsEmptyCell(int WXUNUSED(row), int WXUNUSED(col))
{
  return false;
}

/*!
 * Column header label value returned on demanded column
 */

wxString pwManageFiltersTable::GetColLabelValue(int col)
{
  return GetColLabelString(col);
}

/*!
 * Cell type name is returned for each ccolumn
 */

wxString pwManageFiltersTable::GetTypeName(int WXUNUSED(row), int col)
{
  switch(col) {
    case MFLC_FILTER_NAME:
    case MFLC_FILTER_SOURCE:
    case MFLC_COPYTODATABASE:
      return wxGRID_VALUE_STRING;
    case MFLC_INUSE:
    case MFLC_EXPORT:
      return wxGRID_VALUE_BOOL;
    default:
      wxASSERT(false);
      break;
  }
  return wxEmptyString;
}

/*!
 * CanGetValueAs returns if column value can be fetched by special type (or string)
 */

bool pwManageFiltersTable::CanGetValueAs(int WXUNUSED(row), int col, const wxString& typeName)
{
  if((typeName.CompareTo(wxGRID_VALUE_BOOL) == 0) && ((col == MFLC_INUSE) ||
                                                      (col == MFLC_EXPORT)))
    return true;
  if((typeName.CompareTo(wxGRID_VALUE_STRING) == 0) && ((col == MFLC_FILTER_NAME) ||
                                                        (col == MFLC_FILTER_SOURCE) ||
                                                        (col == MFLC_COPYTODATABASE)))
    return true;
  return false;
}

/*!
 * CanSetValueAs returns if column value can be set by special type (or string)
 */

bool pwManageFiltersTable::CanSetValueAs(int WXUNUSED(row), int col, const wxString& typeName)
{
  if((typeName.CompareTo(wxGRID_VALUE_BOOL) == 0) && ((col == MFLC_INUSE) ||
                                                      (col == MFLC_COPYTODATABASE) ||
                                                      (col == MFLC_EXPORT)))
    return true;
  if((typeName.CompareTo(wxGRID_VALUE_STRING) == 0) && ((col == MFLC_FILTER_NAME) ||
                                                        (col == MFLC_FILTER_SOURCE)))
    return false; // We cannot set a value on such entries
  return false;
}

/*!
 * GetValue returns cell value (as string)
 */

wxString pwManageFiltersTable::GetValue(int row, int col)
{
  wxString result(wxEmptyString);
  
  ASSERT(m_pdata);
  
  switch(col) {
    case MFLC_FILTER_NAME:
      result = wxString(m_pdata->at(row).flt_key.cs_filtername);
      break;
    case MFLC_FILTER_SOURCE:
      result = getSourcePoolLabel(m_pdata->at(row).flt_key.fpool);
      break;
    case MFLC_INUSE:
      result = BoolString(m_pdata->at(row).flt_flags & MFLT_INUSE);
      break;
    case MFLC_COPYTODATABASE:
      result = (m_pdata->at(row).flt_key.fpool == FPOOL_DATABASE) ? pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_UNCHECKED_DISABLED) : BoolString(m_pdata->at(row).flt_flags & MFLT_REQUEST_COPY_TO_DB);
      break;
    case MFLC_EXPORT:
      result = BoolString(m_pdata->at(row).flt_flags & MFLT_REQUEST_EXPORT);
      break;
    default:
      wxASSERT(false);
      break;
  }
  return result;
}

/*!
 * GetValueAsBool returns cell value (as boolean)
 */

bool pwManageFiltersTable::GetValueAsBool(int row, int col)
{
  bool result = false;
  
  ASSERT(m_pdata);
  
  switch(col) {
    case MFLC_INUSE:
      result = m_pdata->at(row).flt_flags & MFLT_INUSE;
      break;
    case MFLC_COPYTODATABASE:
      result = m_pdata->at(row).flt_flags & MFLT_REQUEST_COPY_TO_DB;
      break;
    case MFLC_EXPORT:
      result = m_pdata->at(row).flt_flags & MFLT_REQUEST_EXPORT;
      break;
    default:
      wxASSERT(false);
      break;
  }
  return result;
}

/*!
 * SetValue set cell value (as string)
 */

void pwManageFiltersTable::SetValue(int row, int col, const wxString& value)
{
  switch(col) {
    case MFLC_INUSE:
    case MFLC_COPYTODATABASE:
    case MFLC_EXPORT:
      SetValueAsBool(row, col, ((value.CompareTo(L"0") == 0) || value.empty()) ? false : true);
      break;
    default:
      wxASSERT(false);
      break;
  }
}

/*!
 * SetValueAsBool set cell value (as boolean)
 */

void pwManageFiltersTable::SetValueAsBool(int row, int col, bool value)
{
  ASSERT(m_pdata);
  
  switch(col) {
    case MFLC_INUSE:
      if(value)
        m_pdata->at(row).flt_flags |= MFLT_INUSE;
      else
        m_pdata->at(row).flt_flags &= ~MFLT_INUSE;
      break;
    case MFLC_COPYTODATABASE:
      if(value)
        m_pdata->at(row).flt_flags |= MFLT_REQUEST_COPY_TO_DB;
      else
        m_pdata->at(row).flt_flags &= ~MFLT_REQUEST_COPY_TO_DB;
      break;
    case MFLC_EXPORT:
      if(value)
        m_pdata->at(row).flt_flags |= MFLT_REQUEST_EXPORT;
      else
        m_pdata->at(row).flt_flags &= ~MFLT_REQUEST_EXPORT;
      break;
    default:
      wxASSERT(false);
      break;
  }
}

/*!
 * pwManageFiltersTable override implementations
 * --------------------------------------------------------
 */

/*!
 * Clear content of grid
 */

void pwManageFiltersTable::Clear()
{
  ASSERT(m_pdata);
  if(m_pdata->size())
    DeleteRows(0, m_pdata->size());
}

/*!
 * DeleteRows Remove rows from screen
 */

bool pwManageFiltersTable::DeleteRows(size_t pos, size_t numRows)
{
  if (GetView()) {
    //This will actually remove the item from grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           static_cast<int>(pos),
                           static_cast<int>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
    
  return true;
}

/*!
 * AppendRows Append rows to screen
 */

bool pwManageFiltersTable::AppendRows(size_t numRows/*=1*/)
{
  if (GetView()) {
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                           static_cast<int>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  return true;
}

/*!
 * InsertRows Insert rows on screen
 */

bool pwManageFiltersTable::InsertRows(size_t pos/*=0*/, size_t numRows/*=1*/)
{
  if (GetView()) {
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                           static_cast<int>(pos),
                           static_cast<int>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  return true;
}

/*!
 * pwManageFiltersTable supporting functions
 * --------------------------------------------------
 */

/*!
 * GetColLabelString Column heading label.
 */

wxString pwManageFiltersTable::GetColLabelString(int col)
{
  switch(col) {
    case MFLC_FILTER_NAME:
      return wxString(_("Name"));
    case MFLC_FILTER_SOURCE:
      return wxString(_("Source"));
    case MFLC_INUSE:
      return wxString(_("Apply"));
    case MFLC_COPYTODATABASE:
      return wxString(_("Copy to database"));
    case MFLC_EXPORT:
      return wxString(_("Export"));
    default:
      wxASSERT(false);
      break;
  }
  return wxEmptyString;
}

/*!
 * getSourcePoolLabel Get filter pool label.
 */

wxString pwManageFiltersTable::getSourcePoolLabel(FilterPool pool)
{
  wxString result(wxEmptyString);
  
  switch(pool) {
    case FPOOL_DATABASE:
      result = wxString(_("Database"));
      break;
    case FPOOL_AUTOLOAD:
      result = wxString(_("Autoload"));
      break;
    case FPOOL_IMPORTED:
      result = wxString(_("Imported"));
      break;
    case FPOOL_SESSION:
      result = wxString(_("Session"));
      break;
    default:
      wxASSERT(false);
      result = wxString("?");
  }
  return result;
}

/*!
 * pwSortedManageFilters  functions
 * ----------------------------------------
 */

/*!
 * LessThan Sorting function for filter
 */

bool pwSortedManageFilters::LessThan(struct st_FilterItemData &fd1, struct st_FilterItemData &fd2)
{
  bool result;
  
  switch(m_sortColumn) {
    case MFLC_FILTER_NAME:
    {
      int compareResult = fd1.flt_key.cs_filtername.compare(fd2.flt_key.cs_filtername);
      if(compareResult == 0) {
        result = (static_cast<int>(fd1.flt_key.fpool) < static_cast<int>(fd2.flt_key.fpool));
      }
      else {
        result = (compareResult < 0);
      }
      break;
    }
    case MFLC_FILTER_SOURCE:
      if(fd1.flt_key.fpool != fd2.flt_key.fpool)
        result = (static_cast<int>(fd1.flt_key.fpool) < static_cast<int>(fd2.flt_key.fpool));
      else
        result = (fd1.flt_key.cs_filtername.compare(fd2.flt_key.cs_filtername) < 0);
      break;
    case MFLC_INUSE:
      if((fd1.flt_flags & MFLT_INUSE) != (fd2.flt_flags & MFLT_INUSE))
        result = ((fd1.flt_flags & MFLT_INUSE) < (fd2.flt_flags & MFLT_INUSE));
      else if (fd1.flt_key.fpool != fd2.flt_key.fpool)
        result = (static_cast<int>(fd1.flt_key.fpool) < static_cast<int>(fd2.flt_key.fpool));
      else
        result = (fd1.flt_key.cs_filtername.compare(fd2.flt_key.cs_filtername) < 0);
      break;
    case MFLC_COPYTODATABASE:
      if((fd1.flt_flags & MFLT_REQUEST_COPY_TO_DB) != (fd2.flt_flags & MFLT_REQUEST_COPY_TO_DB))
        result = ((fd1.flt_flags & MFLT_REQUEST_COPY_TO_DB) < (fd2.flt_flags & MFLT_REQUEST_COPY_TO_DB));
      else if (fd1.flt_key.fpool != fd2.flt_key.fpool)
        result = (static_cast<int>(fd1.flt_key.fpool) < static_cast<int>(fd2.flt_key.fpool));
      else
        result = (fd1.flt_key.cs_filtername.compare(fd2.flt_key.cs_filtername) < 0);
      break;
    case MFLC_EXPORT:
      if((fd1.flt_flags & MFLT_REQUEST_EXPORT) != (fd2.flt_flags & MFLT_REQUEST_EXPORT))
        result = ((fd1.flt_flags & MFLT_REQUEST_EXPORT) < (fd2.flt_flags & MFLT_REQUEST_EXPORT));
      else if (fd1.flt_key.fpool != fd2.flt_key.fpool)
        result = (static_cast<int>(fd1.flt_key.fpool) < static_cast<int>(fd2.flt_key.fpool));
      else
        result = (fd1.flt_key.cs_filtername.compare(fd2.flt_key.cs_filtername) < 0);
      break;
    default:
      wxASSERT(false);
      result = false;
      break;
  }
  
  return m_ascending ? result : ! result;
}

/*!
 * clear sorted filter content
 */

void pwSortedManageFilters::clear()
{
  m_SortFilterData.clear();
  m_activeFilterIdx = -1;
  m_selectedFilterIdx = -1;
}

/*!
 * insert new filter into sorted filter list
 * Actualize selected and active filter index, when needed.
 */

int pwSortedManageFilters::insert(struct st_FilterItemData &data)
{
  size_t i;

  for(i = 0; i < m_SortFilterData.size(); ++i) {
    if(! LessThan(m_SortFilterData.at(i), data)) {
      m_SortFilterData.insert(m_SortFilterData.begin() + i, data);
      // Correct index values
      if(m_activeFilterIdx != -1 && m_activeFilterIdx >= static_cast<int>(i)) {
        ++m_activeFilterIdx;
      }
      if(m_selectedFilterIdx != -1 && m_selectedFilterIdx >= static_cast<int>(i)) {
        ++m_selectedFilterIdx;
      }
      return static_cast<int>(i);
    }
  }
  if(i == m_SortFilterData.size()) {
    // Append
    m_SortFilterData.push_back(data);
    return static_cast<int>(i);
  }
  return -1;
}

/*!
 * remove filter from sorted filter list
 * Actualize selected and active filter index, when needed.
 */

void pwSortedManageFilters::remove(int idx)
{
  if(idx >= 0 && idx < static_cast<int>(m_SortFilterData.size())) {
    m_SortFilterData.erase(m_SortFilterData.begin() + idx);
    if(idx == m_activeFilterIdx)
      m_activeFilterIdx = -1;
    else if(m_activeFilterIdx != -1 && m_activeFilterIdx > idx)
      --m_activeFilterIdx;
    if(idx == m_selectedFilterIdx)
      m_selectedFilterIdx = -1;
    else if(m_selectedFilterIdx != -1 && m_selectedFilterIdx > idx)
      --m_selectedFilterIdx;
  }
  else {
    wxASSERT(false);
  }
}

/*!
 * remove selected filter from sorted filter list
 * Actualize selected and active filter index, when needed.
 */

void pwSortedManageFilters::remove()
{
  if(m_selectedFilterIdx != -1) {
    remove(m_selectedFilterIdx);
  }
}

/*!
 * SortByColumn sort by given column in demanded order (ascending or descending)
 * Actualize selected and active filter index.
 */

void pwSortedManageFilters::SortByColumn(int column, bool ascending)
{
  if(column == m_sortColumn && ascending != m_ascending) {
    m_ascending = ascending;
    // Change orderung backward
    for(int i = 0, e = static_cast<int>(m_SortFilterData.size()) - 1; i < e; ++i, --e) {
      std::swap(m_SortFilterData.at(i), m_SortFilterData.at(e));
      if(i == m_activeFilterIdx)
        m_activeFilterIdx = e;
      else if(e == m_activeFilterIdx)
        m_activeFilterIdx = i;
      if(i == m_selectedFilterIdx)
        m_selectedFilterIdx = e;
      else if(e == m_selectedFilterIdx)
        m_selectedFilterIdx = i;
    }
  }
  else {
    // Reorder
    m_ascending = ascending;
    m_sortColumn = column;
    
    // Sort
    size_t size = m_SortFilterData.size();
    for(int i = 1; i < static_cast<int>(size); ++i) {
      bool bChange = false;
      // Search for entry smaller than entry at i
      for(int x = 0; x < static_cast<int>(size) - 1; ++x) {
        if(LessThan(m_SortFilterData.at(i), m_SortFilterData.at(x))) {
          std::swap(m_SortFilterData.at(i), m_SortFilterData.at(x));
          if(i == m_activeFilterIdx)
            m_activeFilterIdx = x;
          else if(x == m_activeFilterIdx)
            m_activeFilterIdx = i;
          if(i == m_selectedFilterIdx)
            m_selectedFilterIdx = x;
          else if(x == m_selectedFilterIdx)
            m_selectedFilterIdx = i;
          bChange = true;
        }
      }
      if(! bChange)
        break;
    }
  }
}

/*!
 * GetSelectedFilterName returns name of selected filter
 */

stringT pwSortedManageFilters::GetSelectedFilterName()
{
  if(m_selectedFilterIdx >= 0 && m_selectedFilterIdx < static_cast<int>(m_SortFilterData.size())) {
    return m_SortFilterData.at(m_selectedFilterIdx).flt_key.cs_filtername;
  }
  return L"";
}

/*!
 * ClearFlagAt remove flag from given index
 */

void pwSortedManageFilters::ClearFlagAt(size_t idx, UINT mask)
{
  if(idx >= 0 && idx < m_SortFilterData.size()) {
    m_SortFilterData.at(idx).flt_flags &= ~mask;
  }
}

/*!
 * SetFlagAt set flag at given index
 */

void pwSortedManageFilters::SetFlagAt(size_t idx, UINT mask)
{
  if(idx >= 0 && idx < m_SortFilterData.size()) {
    m_SortFilterData.at(idx).flt_flags |= mask;
  }
}

/*!
 * IsFlagSetAt check flag at given index
 */

bool pwSortedManageFilters::IsFlagSetAt(size_t idx, UINT mask)
{
  if(idx >= 0 && idx < m_SortFilterData.size()) {
    return (m_SortFilterData.at(idx).flt_flags & mask) ? true : false;
  }
  return false;
}

/*!
 * GetPoolAt get pool value at given index
 */

FilterPool pwSortedManageFilters::GetPoolAt(size_t idx)
{
  if(idx >= 0 && idx < m_SortFilterData.size()) {
    return m_SortFilterData.at(idx).flt_key.fpool;
  }
  return FPOOL_LAST;
}

/*!
 * FindEntryByKey give index of given key (linear search)
 */

int pwSortedManageFilters::FindEntryByKey(const st_Filterkey &fk)
{
  size_t idx;
  
  size_t size = m_SortFilterData.size();
  for(idx = 0; idx < size; ++idx) {
    if(m_SortFilterData.at(idx).flt_key.fpool == fk.fpool &&
       m_SortFilterData.at(idx).flt_key.cs_filtername == fk.cs_filtername)
      break;
  }
  if(idx == size) return -1;
  return static_cast<int>(idx);
}

/*!
 * GetKeyAt returns key at given index
 */

st_Filterkey pwSortedManageFilters::GetKeyAt(size_t idx)
{
  if(idx >= 0 && idx < m_SortFilterData.size()) {
    return m_SortFilterData.at(idx).flt_key;
  }
  st_Filterkey fk;
  fk.fpool = FPOOL_LAST;
  fk.cs_filtername = L"";
  return fk;
}

/*!
 * GetFilterItemAt returns filter item data at given index
 */

struct st_FilterItemData& pwSortedManageFilters::GetFilterItemAt(size_t idx)
{
  if(idx >= 0 && idx < m_SortFilterData.size()) {
    return m_SortFilterData.at(idx);
  }
  static struct st_FilterItemData fi;
  fi.flt_key.fpool = FPOOL_LAST;
  fi.flt_key.cs_filtername = L"";
  fi.flt_flags = 0;
  return fi;
}
