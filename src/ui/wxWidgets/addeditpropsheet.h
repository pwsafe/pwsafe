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
#include "wx/spinctrl.h"
#include "wx/grid.h"
#include "wx/datectrl.h"
#include "wx/dateevt.h"
#include "wx/statline.h"
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
#define ID_COMBOBOX1 10095
#define ID_TEXTCTRL5 10096
#define ID_TEXTCTRL1 10088
#define ID_TEXTCTRL2 10089
#define ID_BUTTON2 10090
#define ID_BUTTON3 10097
#define ID_TEXTCTRL3 10091
#define ID_TEXTCTRL4 10092
#define ID_BUTTON 10093
#define ID_TEXTCTRL7 10098
#define ID_PANEL_ADDITIONAL 10085
#define ID_TEXTCTRL6 10094
#define ID_TEXTCTRL8 10099
#define ID_CHECKBOX 10100
#define ID_COMBOBOX 10101
#define ID_CHECKBOX1 10102
#define ID_SPINCTRL 10103
#define ID_GRID 10104
#define ID_BUTTON1 10105
#define ID_BUTTON4 10106
#define ID_PANEL_DTIME 10086
#define ID_RADIOBUTTON 10107
#define ID_DATECTRL 10108
#define ID_SPINCTRL1 10109
#define ID_RADIOBUTTON1 10110
#define ID_SPINCTRL2 10111
#define ID_CHECKBOX2 10112
#define ID_BUTTON5 10113
#define ID_BUTTON6 10114
#define ID_PANEL_PPOLICY 10087
#define ID_RADIOBUTTON2 10115
#define ID_RADIOBUTTON3 10116
#define ID_SPINCTRL3 10117
#define ID_CHECKBOX3 10118
#define ID_CHECKBOX4 10119
#define ID_CHECKBOX5 10120
#define ID_CHECKBOX6 10121
#define ID_CHECKBOX7 10122
#define ID_CHECKBOX8 10123
#define ID_CHECKBOX9 10124
#define ID_BUTTON7 10125
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
