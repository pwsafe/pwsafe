/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file managefilters.h
* 
*/


#ifndef _MANAGEFILTERSDLG_H_
#define _MANAGEFILTERSDLG_H_

/*!
 * Includes
 */

////@begin includes
#include <vector>
#include <set>

#include "wx/grid.h"
#include "wx/headerctrl.h"

#include "core/core.h"
#include "core/PWSFilters.h"

#include "PWFiltersGrid.h"
#include "ManageFiltersTable.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class pwFiltersGrid;
////@end forward declarations

/*!
 * Control identifiers
 */
#if wxCHECK_VERSION(2, 9, 1)   // At least version 2.9.1 must be present
#define PW_FILTERS_GIRD_USE_NATIVE_HEADER 1
#endif

////@begin control identifiers
#ifndef ID_MANAGEFILTERS
#define ID_MANAGEFILTERS 10045
#endif
#define ID_FILTERSGRID 10001
#ifndef ID_EDIT
#define ID_EDIT 10024
#endif
#define ID_IMPORT 10002
#define ID_EXPORT 10004
#define ID_GRID 10005
#define SYMBOL_MANAGEFILTERS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_MANAGEFILTERS_TITLE _("Manage Filters")
#define SYMBOL_MANAGEFILTERS_IDNAME ID_MANAGEFILTERS
#define SYMBOL_MANAGEFILTERS_SIZE wxSize(400, 300)
#define SYMBOL_MANAGEFILTERS_POSITION wxDefaultPosition

// Number of rows at start-up
// Start with 6 lines
#define FLT_DEFAULT_NUM_ROWS 6
////@end control identifiers

/*!
 * ManageFiltersGrid class declaration
 */

class ManageFiltersGrid : public wxGrid {
  
  DECLARE_CLASS( ManageFiltersGrid )
  DECLARE_EVENT_TABLE()
  
public:
  /// Constructors
  ManageFiltersGrid(wxWindow* parent, pwSortedManageFilters *data, wxWindowID id = ID_FILTERSGRID, const wxPoint& pos = wxDefaultPosition,
          const wxSize& size = wxDefaultSize, long style = wxHSCROLL|wxVSCROLL);
  
  /// EVT_HEADER_CLICK
  void OnHeaderClick(wxHeaderCtrlEvent& event);
  
  /// EVT_GRID_RANGE_SELECT
  void OnGridRangeSelect(wxGridRangeSelectEvent& evt);
  
  /// EVT_COMMAND for event handler for EVT_SELECT_MANAGE_GRID_ROW
  void OnAutoSelectGridRow(wxCommandEvent& evt);
  
  /// wxEVT_CHAR_HOOK event handler for WXK_UP, WXK_DOWN, WXK_LEFT, WXK_RIGHT
  void OnChar(wxKeyEvent& evt);
  
  /// Sorting of columns
  void SortGrid(int column, bool ascending);
  void SortRefresh();
  
  /// Bind events for sorting of columns
  void BindEvents();
private:
  pwSortedManageFilters *m_pMapFilterData;
};

/*!
 * ManageFilters class declaration
 */

class ManageFiltersDlg: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( ManageFilters )
  DECLARE_EVENT_TABLE()

public:
  static ManageFiltersDlg* Create(wxWindow *parent, PWScore *core, PWSFilters &MapFilters, st_filters *currentFilters, FilterPool *activefilterpool, stringT *activefiltername, bool *bFilterActive, const bool bCanHaveAttachments = false, const std::set<StringX> *psMediaTypes = nullptr, bool readOnly = true, wxWindowID id = SYMBOL_MANAGEFILTERS_IDNAME, const wxString& caption = SYMBOL_MANAGEFILTERS_TITLE, const wxPoint& pos = SYMBOL_MANAGEFILTERS_POSITION, const wxSize& size = SYMBOL_MANAGEFILTERS_SIZE, long style = SYMBOL_MANAGEFILTERS_STYLE );

  /// Destructor
  ~ManageFiltersDlg() = default;
protected:
  /// Constructors
  ManageFiltersDlg() = default;
  ManageFiltersDlg(wxWindow *parent, PWScore *core, PWSFilters &MapFilters, st_filters *currentFilters, FilterPool *activefilterpool, stringT *activefiltername, bool *bFilterActive, const bool bCanHaveAttachments, const std::set<StringX> *psMediaTypes, bool readOnly, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style);

  /// Creates the controls and sizers
  void CreateControls();

////@begin ManageFilters event handler declarations

  /// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERSGRID
  void OnCellLeftClick( wxGridEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
  void OnNewClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT
  void OnEditClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_COPY
  void OnCopyClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
  void OnDeleteClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT
  void OnImportClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT
  void OnExportClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick( wxCommandEvent& event );
  
  /// wxEVT_SIZE event handler for resize
  void OnSize ( wxSizeEvent &event );
  
////@end ManageFilters event handler declarations

////@begin ManageFiltersDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end ManageFiltersDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();
  
private:
  
  void InitDialog();
  
  void UpdateFilterList(bool bRefreshGrid = true); // Read filter from filter map
  void ShowSelectedFilter(); // Show the selected filter in the grid view
  void SetSelectedActiveFilter();   // Actual selected filter will be set to active Filter
  void ClearSelectedActiveFilter(); // Clear active filter
  void DeleteSelectedFilter();
  int FindIndex(const st_Filterkey &fk) { return m_MapFilterData.FindEntryByKey(fk); };
  int InsertEntry(const st_Filterkey &fk, const st_filters &filters);
  bool SelectIfSingleEntry(); // When only one entry is present, this one will be selected by default
  void SelectEntry(int idx);
  void UpdateActiveFilterContent(const st_Filterkey &fk, const st_filters &filters);
  void ClearActiveFilter(); // Clear active filter at the screen in pwSafe main window
  void MarkAppliedFilter(); // Mark selected index as in use as active filter
  
  // Functions for handling of columns and rows
  void SetManageGridColFormat(int col, wxGridCellRenderer *renderer);
  void RefreshFiltersRow(int row);
  void RefreshFiltersCell(int row, int col);
  void SetGridColLeftAligned(int col);
  wxSize ExtendColSize(int col, int extend = 30);
  
  void DoNewClick();
  void DoEditClick();
  
  
  ////@begin ManageFiltersDlg member variables
  PWSFilters *m_pMapAllFilters = nullptr;  // The map of all present filters
  st_filters *m_pCurrentFilters = nullptr; // Pointer to the currect active (or de-activated) filter
  const bool m_bCanHaveAttachments = false;
  const std::set<StringX> *m_psMediaTypes = nullptr;
  const bool m_bReadOnly = false;
  PWScore   *m_core = nullptr;
  
  // Active filter is shown on the pwSafe main windows
  FilterPool *m_pActiveFilterPool = nullptr;
  stringT *m_pActiveFilterName = nullptr;
  bool    *m_pbFilterActive = nullptr;
  
  ManageFiltersGrid *m_MapFiltersGrid = nullptr;     // Grid to show all filters
  wxStaticText      *m_SelectedFilterText = nullptr; // Dialog item to be updated with the shown filters name
  pwFiltersGrid     *m_filterGrid = nullptr;         // Grid to show the filters content
  
  // The slected filter is marked in the list of filters and the content is shown in the lower grid
  FilterPool m_SelectedFilterPool = FPOOL_LAST;
  stringT    m_SelectedFilterName;
  st_filters m_SelectedFilter;
  
  int m_num_to_copy = 0;    // Number of selected entries in copy to DB column
  int m_num_to_export = 0;  // Number of selected entries for export operation
  
  pwSortedManageFilters m_MapFilterData;   // Map to handle sorting for filter in the filters grid
  
  bool m_bDBFiltersChanged = false;

  // Height of used font
  int m_FontHeight = 15;
  
  // Old window size to determine change when resize is called
  wxSize windowSize;
  
////@end ManageFiltersDlg member variables
};

#endif
  // _MANAGEFILTERSDLG_H_
