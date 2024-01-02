/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManageFiltersTable.h
* 
*/

#ifndef _MANAGEFILTERSTABLE_H_
#define _MANAGEFILTERSTABLE_H_

/*!
 * Includes
 */

////@begin includes
#include <vector>
#include <wx/string.h>
#include <wx/grid.h>
#include "core/PWSFilters.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class ManageFilters;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

// Subitem indices for list of filters
enum {
  MFLC_FILTER_NAME = 0,
  MFLC_FILTER_SOURCE,
  MFLC_INUSE,
  MFLC_COPYTODATABASE,
  MFLC_EXPORT,
  MFLC_NUM_COLUMNS
};

// Filter Flags
enum {MFLT_SELECTED           = 0x8000, // Not used this time with wxWidgets
      MFLT_REQUEST_COPY_TO_DB = 0x4000,
      MFLT_REQUEST_EXPORT     = 0x2000,
      MFLT_INUSE              = 0x1000,
      MFLT_UNUSED             = 0x0fff};

// Structure pointed used by GridCtrl
struct st_FilterItemData {
  st_Filterkey flt_key;
  UINT flt_flags;
};

class pwSortedManageFilters {
  
public:
  pwSortedManageFilters() : m_sortColumn(MFLC_FILTER_SOURCE), m_ascending(true), m_activeFilterIdx(-1), m_selectedFilterIdx(-1) {};

  void clear();
  size_t size() { return m_SortFilterData.size(); };
  int insert(struct st_FilterItemData &data);
  void remove(int idx);
  void remove(); // Will remove current selected
  void SortByColumn(int column, bool ascending);
  
  int GetActiveFilterIdx() { return m_activeFilterIdx; };
  int GetSelectedFilterIdx() { return m_selectedFilterIdx; };
  int FindEntryByKey(const st_Filterkey &fk);
  
  void SetActiveFilterIdx(int idx) { m_activeFilterIdx = idx; };
  void SetSelectedFilterIdx(int idx) { m_selectedFilterIdx = idx; };
  
  void SetActiveFilterInactive() { SetActiveFilterIdx(-1); };
  void SetSelectedFilterInactive() { SetSelectedFilterIdx(-1); };
  
  bool IsFilterSelected() { return m_selectedFilterIdx != -1; };
  bool IsNoFilterSelected() { return m_selectedFilterIdx == -1; };
  bool IsFilterActive() { return m_activeFilterIdx != -1; };
  bool IsNoFilterActive() { return m_activeFilterIdx == -1; };
  
  FilterPool GetSelectedPool() { return GetPoolAt(m_selectedFilterIdx); };
  stringT GetSelectedFilterName();
  st_Filterkey GetSelectedKey() { return GetKeyAt(m_selectedFilterIdx); };
  struct st_FilterItemData& GetSelectedFilterItem() { return GetFilterItemAt(m_selectedFilterIdx); };
  
  void ClearFlagAt(size_t idx, UINT mask);
  void SetFlagAt(size_t idx, UINT mask);
  bool IsFlagSetAt(size_t idx, UINT mask);
  
  FilterPool GetPoolAt(size_t idx);
  st_Filterkey GetKeyAt(size_t idx);
  struct st_FilterItemData& GetFilterItemAt(size_t idx);
  
  bool LessThan(struct st_FilterItemData &fd1, struct st_FilterItemData &fd2);
  
  std::vector<struct st_FilterItemData> &Data() { return m_SortFilterData; };
  
  int GetSortingColumn() { return m_sortColumn; };
  bool GetAscending() { return m_ascending; };
  
private:
  std::vector<struct st_FilterItemData> m_SortFilterData;
  int m_sortColumn;
  bool m_ascending;
  
  int m_activeFilterIdx; // Current active filter idx
  int m_selectedFilterIdx; // Current selected filter idx
};

/*!
 * pwManageFiltersTable class declaration
 */

class pwManageFiltersTable : public wxGridTableBase
{
  DECLARE_CLASS( pwManageFiltersTable )

  DECLARE_NO_COPY_CLASS(pwManageFiltersTable)
public:
  /// Constructors
  pwManageFiltersTable(std::vector<struct st_FilterItemData> &data);

  /// Destructor
  ~pwManageFiltersTable();

  /// overrides from wxGridTableBase
  virtual int GetNumberRows();
  virtual int GetNumberCols();
  virtual bool IsEmptyCell(int row, int col);
  virtual wxString GetValue(int row, int col);
  virtual wxString GetColLabelValue(int col);
  virtual void SetValue(int row, int col, const wxString& value);
  virtual bool DeleteRows(size_t pos = 0, size_t numRows = 1);
  virtual bool InsertRows(size_t pos = 0, size_t numRows = 1);
  virtual bool AppendRows(size_t numRows = 1);
  
  ///optional overrides
  virtual void Clear();
  
  virtual wxString GetTypeName(int row, int col );
  virtual bool CanGetValueAs(int row, int col, const wxString& typeName);
  virtual bool CanSetValueAs(int row, int col, const wxString& typeName);

  virtual bool GetValueAsBool(int row, int col); // typeName is wxGRID_VALUE_BOOL

  virtual void SetValueAsBool(int row, int col, bool value); // typeName is wxGRID_VALUE_BOOL
  
  // Supporting functions for string resources
  static wxString GetColLabelString(int col);
  static wxString getSourcePoolLabel(FilterPool pool);
  
private:
  std::vector<struct st_FilterItemData> *m_pdata;
  std::vector<int> sortedKeys; // Index into m_pdata after sorting
  
  wxString BoolString(bool value) { return wxString(value ? L"1" : L"0"); }
};

#endif // _MANAGEFILTERSTABLE_H_
