/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file managefilters.h
* 
*/


#ifndef _MANAGEFILTERS_H_
#define _MANAGEFILTERS_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_MANAGEFILTERS 10000
#define ID_FILTERSGRID 10001
#define ID_EDIT 10003
#define ID_IMPORT 10002
#define ID_EXPORT 10004
#define ID_GRID 10005
#define SYMBOL_MANAGEFILTERS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_MANAGEFILTERS_TITLE _("Manage Filters")
#define SYMBOL_MANAGEFILTERS_IDNAME ID_MANAGEFILTERS
#define SYMBOL_MANAGEFILTERS_SIZE wxSize(400, 300)
#define SYMBOL_MANAGEFILTERS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ManageFilters class declaration
 */

class ManageFilters: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( ManageFilters )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  ManageFilters();
  ManageFilters( wxWindow* parent, wxWindowID id = SYMBOL_MANAGEFILTERS_IDNAME, const wxString& caption = SYMBOL_MANAGEFILTERS_TITLE, const wxPoint& pos = SYMBOL_MANAGEFILTERS_POSITION, const wxSize& size = SYMBOL_MANAGEFILTERS_SIZE, long style = SYMBOL_MANAGEFILTERS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MANAGEFILTERS_IDNAME, const wxString& caption = SYMBOL_MANAGEFILTERS_TITLE, const wxPoint& pos = SYMBOL_MANAGEFILTERS_POSITION, const wxSize& size = SYMBOL_MANAGEFILTERS_SIZE, long style = SYMBOL_MANAGEFILTERS_STYLE );

  /// Destructor
  ~ManageFilters();

  /// Initialises member variables
  void Init();

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

////@end ManageFilters event handler declarations

////@begin ManageFilters member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end ManageFilters member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin ManageFilters member variables
////@end ManageFilters member variables
};

#endif
  // _MANAGEFILTERS_H_
