/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/**
 *  \file addeditpropsheet.h
 */

#ifndef _ADDEDITPROPSHEET_H_
#define _ADDEDITPROPSHEET_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/propdlg.h"
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
#define ID_ADDEDITPROPSHEET 10083
#define ID_PANEL_BASIC 10084
#define ID_COMBOBOX 10088
#define ID_TEXTCTRL1 10089
#define ID_TEXTCTRL2 10090
#define ID_TEXTCTRL3 10091
#define ID_BUTTON 10092
#define ID_BUTTON1 10093
#define ID_TEXTCTRL4 10094
#define ID_PANEL_ADDITIONAL 10085
#define ID_PANEL_DTIME 10086
#define ID_PANEL_PPOLICY 10087
#define SYMBOL_ADDEDITPROPSHEET_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_ADDEDITPROPSHEET_TITLE _("Edit Entry")
#define SYMBOL_ADDEDITPROPSHEET_IDNAME ID_ADDEDITPROPSHEET
#define SYMBOL_ADDEDITPROPSHEET_SIZE wxSize(400, 300)
#define SYMBOL_ADDEDITPROPSHEET_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * AddEditPropSheet class declaration
 */

class AddEditPropSheet: public wxPropertySheetDialog
{    
  DECLARE_DYNAMIC_CLASS( AddEditPropSheet )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  AddEditPropSheet();
  AddEditPropSheet( wxWindow* parent, wxWindowID id = SYMBOL_ADDEDITPROPSHEET_IDNAME, const wxString& caption = SYMBOL_ADDEDITPROPSHEET_TITLE, const wxPoint& pos = SYMBOL_ADDEDITPROPSHEET_POSITION, const wxSize& size = SYMBOL_ADDEDITPROPSHEET_SIZE, long style = SYMBOL_ADDEDITPROPSHEET_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ADDEDITPROPSHEET_IDNAME, const wxString& caption = SYMBOL_ADDEDITPROPSHEET_TITLE, const wxPoint& pos = SYMBOL_ADDEDITPROPSHEET_POSITION, const wxSize& size = SYMBOL_ADDEDITPROPSHEET_SIZE, long style = SYMBOL_ADDEDITPROPSHEET_STYLE );

  /// Destructor
  ~AddEditPropSheet();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin AddEditPropSheet event handler declarations

////@end AddEditPropSheet event handler declarations

////@begin AddEditPropSheet member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end AddEditPropSheet member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin AddEditPropSheet member variables
////@end AddEditPropSheet member variables
};

#endif
  // _ADDEDITPROPSHEET_H_
