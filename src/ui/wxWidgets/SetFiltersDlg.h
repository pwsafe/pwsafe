/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SetFiltersDlg.h
* 
*/


#ifndef _SETFILTERSDLG_H_
#define _SETFILTERSDLG_H_


/*!
 * Includes
 */
////@begin includes
#include "wx/valtext.h"
#include "wx/grid.h"
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
#define ID_SETFILTERS 10000
#define ID_FilterName 10001
#define ID_FILTERGRID 10002
#define SYMBOL_SETFILTERS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SETFILTERS_TITLE _("Set Display Filters")
#define SYMBOL_SETFILTERS_IDNAME ID_SETFILTERS
#define SYMBOL_SETFILTERS_SIZE wxSize(400, 300)
#define SYMBOL_SETFILTERS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * SetFilters class declaration
 */

class SetFilters: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( SetFilters )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  SetFilters();
  SetFilters( wxWindow* parent, wxWindowID id = SYMBOL_SETFILTERS_IDNAME, const wxString& caption = SYMBOL_SETFILTERS_TITLE, const wxPoint& pos = SYMBOL_SETFILTERS_POSITION, const wxSize& size = SYMBOL_SETFILTERS_SIZE, long style = SYMBOL_SETFILTERS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SETFILTERS_IDNAME, const wxString& caption = SYMBOL_SETFILTERS_TITLE, const wxPoint& pos = SYMBOL_SETFILTERS_POSITION, const wxSize& size = SYMBOL_SETFILTERS_SIZE, long style = SYMBOL_SETFILTERS_STYLE );

  /// Destructor
  ~SetFilters();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin SetFilters event handler declarations

  /// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERGRID
  void OnCellLeftClick( wxGridEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
  void OnApplyClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );

////@end SetFilters event handler declarations

////@begin SetFilters member function declarations

  wxString GetFilterName() const { return m_filterName ; }
  void SetFilterName(wxString value) { m_filterName = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end SetFilters member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin SetFilters member variables
  wxGrid* m_filterGrid;
private:
  wxString m_filterName;
////@end SetFilters member variables
};

#endif
  // _SETFILTERSDLG_H_
