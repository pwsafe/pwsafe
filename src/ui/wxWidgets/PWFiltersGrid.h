/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFilterGridl.h
* 
*/


#ifndef _PWFILTERSGRID_H_
#define _PWFILTERSGRID_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"

#include "core/core.h"
#include "core/PWScore.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers
#if wxCHECK_VERSION(2, 9, 1)   // At least version 2.9.1 must be present
#define PW_GRID_USE_NATIVE_HEADER 1
#endif
#ifdef PW_GRID_USE_NATIVE_HEADER
#define PW_COLL_SIZE_EXTEND  10  // More space is needed for native indictation of sorting
#else
#define PW_COLL_SIZE_EXTEND  6
#endif

// Subitem indices (column indices)
#define FLC_FILTER_NUMBER 0
#define FLC_ENABLE_BUTTON 1
#define FLC_ADD_BUTTON    2
#define FLC_REM_BUTTON    3
#define FLC_LGC_COMBOBOX  4
#define FLC_FLD_COMBOBOX  5
#define FLC_CRITERIA_TEXT 6
#define FLC_NUM_COLUMNS   7

typedef vFilterRows::iterator vFilterRowsIter;

// Number of rows at start-up
// Start with 8 lines
#define FLC_DEFAULT_NUM_ROWS 8

/*!
 * pwFiltersGrid class declaration
 */

class pwFiltersGrid : public wxGrid
{
  DECLARE_DYNAMIC_CLASS( pwFiltersGrid )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  pwFiltersGrid() : m_pfilters(nullptr),
                    m_bCanHaveAttachments(false),
                    m_psMediaTypes(nullptr),
                    m_bAllowSet(false),
                    m_filtertype(DFTYPE_INVALID),
                    m_filterpool(FPOOL_LAST) { Init(); wxGrid::Init(); }

  pwFiltersGrid(wxWindow *parent,
                wxWindowID id,
                st_filters *pfilters,
                const FilterType &filtertype,
                const FilterPool filterpool,
                const bool bCanHaveAttachments = false,
                const std::set<StringX> *psMediaTypes = NULL,
                const bool bAllowSet = true,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxWANTS_CHARS,
                const wxString& name = wxGridNameStr);
  
  /// Destructor
  ~pwFiltersGrid();
  
  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();
  
  /// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERGRID
  void OnCellLeftClick( wxGridEvent& event );
  
  /// EVT_GRID_RANGE_SELECT
  void OnGridRangeSelect(wxGridRangeSelectEvent& evt);
  
  /// EVT_COMMAND for event handler for EVT_SELECT_GRID_ROW
  void OnAutoSelectGridRow(wxCommandEvent& evt);
  
  /// wxEVT_CHAR_HOOK event handler for WXK_UP, WXK_DOWN, WXK_LEFT, WXK_RIGHT
  void OnChar(wxKeyEvent& evt);
  
  // Access functions for the filter elements and size
  int GetNumRows() { ASSERT(m_currentFilter); return static_cast<int>(m_currentFilter->size());};
  int GetNumMRows() { ASSERT(m_pfilters); return static_cast<int>(m_pfilters->vMfldata.size()); };
  int GetNumHRows() { ASSERT(m_pfilters); return static_cast<int>(m_pfilters->vHfldata.size()); };
  int GetNumPRows() { ASSERT(m_pfilters); return static_cast<int>(m_pfilters->vPfldata.size()); };
  int GetNumARows() { ASSERT(m_pfilters); return static_cast<int>(m_pfilters->vAfldata.size()); };
  FilterType GetFilterAndRow(int row, vFilterRows **filter, int &frow);
  bool IsRowActive(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                              return (*m_currentFilter)[row].bFilterActive; };
  bool IsRowComplete(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                return (*m_currentFilter)[row].bFilterComplete; };
  LogicConnect RowLC(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                return (*m_currentFilter)[row].ltype; };
  FieldType RowFieldType(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                    return (*m_currentFilter)[row].ftype; };
  PWSMatch::MatchType RowMatchType(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                              return (*m_currentFilter)[row].mtype; };
  PWSMatch::MatchRule RowMatchRule(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                              return (*m_currentFilter)[row].rule; };
  
  // Setting functions for the filter fields
  void SetRowActive(int row, bool value) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                           (*m_currentFilter)[row].bFilterActive = value; };
  void SetRowComplete(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                (*m_currentFilter)[row].bFilterComplete = true; };
  void ResetRowComplete(int row) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                (*m_currentFilter)[row].bFilterComplete = false; };
  void SetRowLC(int row, LogicConnect value) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                               (*m_currentFilter)[row].ltype = value; };
  void SetRowFieldType(int row, FieldType value) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                                   (*m_currentFilter)[row].ftype = value;
                                                   UpdateMatchType(row); };
  void SetRowMatchType(int row, PWSMatch::MatchType value) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                                             (*m_currentFilter)[row].mtype = value; };
  void SetRowMatchRule(int row, PWSMatch::MatchRule value) { ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
                                                             (*m_currentFilter)[row].rule = value; };
  
  // Helper function for the grid
  void DoCheckFilterIsComplete(int row);
  void DoEditCriteria(int row);
  
  bool IsSameAsDefault(int row);
  void ClearIfEmpty();
  
  void DoRowAppend(size_t numRows = 1);
  void DoRowInsert(size_t row, size_t numRows = 1);
  void DoRowDelete(size_t row) { DoRowsDelete(row, 1); };
  void DoRowsDelete(size_t row, size_t numRows = 1);
  st_FilterRow& FilterRow(size_t row) { ASSERT(m_currentFilter && row < m_currentFilter->size());
                                        return m_currentFilter->at(row); };
  void ResetFilter();
  void ClearFilter();
  void RefreshFilter();
  void ClearPWHist() { ASSERT(m_pfilters); m_pfilters->vHfldata.clear(); m_pfilters->num_Hactive = 0; };
  void ClearPolicy() { ASSERT(m_pfilters); m_pfilters->vPfldata.clear(); m_pfilters->num_Pactive = 0; };
  void ClearAttachment() { ASSERT(m_pfilters); m_pfilters->vAfldata.clear(); m_pfilters->num_Aactive = 0; };
  bool IsSetPWHist() { ASSERT(m_pfilters); return ! m_pfilters->vHfldata.empty(); };
  bool IsSetPolicy() { ASSERT(m_pfilters); return ! m_pfilters->vPfldata.empty(); };
  bool IsSetAttachment() { ASSERT(m_pfilters); return ! m_pfilters->vAfldata.empty(); };
  
  void RefreshRow(int row);
  void RefreshCell(int row, int col);
  
  bool IsReadOnly() { return !m_bAllowSet; };
  bool IsReadWrite() { return m_bAllowSet; };
  
private:
  void SetGridColFormat(int col, wxGridCellRenderer *renderer, wxGridCellEditor *editor = nullptr);
  void SetGridColReadOnly(int col);
  void UpdateMatchType(int row);
  bool GetCriterion(int row);
  
  st_filters *m_pfilters; // Pointer to the handled filter
  const bool m_bCanHaveAttachments;
  const std::set<StringX> *m_psMediaTypes;
  const bool m_bAllowSet; // R/W or R/O
  const FilterType m_filtertype;
  const FilterPool m_filterpool;
  static bool m_subDialogCalled; // Semaphore to not call history, policy or attachment dialog a second time
  
  // Depending from m_filtertype remember actual used filter and active counter
  vFilterRows *m_currentFilter;
  int *m_currentCounter;
  // Remember if filter had been empty at the start, so leave it empty when nothing had been changed
  bool m_initialyEmpty;
  // current selected row
  int m_currentSelection;
  // Height of used font
  int m_FontHeight;
};


#endif // _PMFILTERSGRID_H_
