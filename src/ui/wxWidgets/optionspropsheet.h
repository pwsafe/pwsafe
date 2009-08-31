/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _OPTIONSPROPSHEET_H_
#define _OPTIONSPROPSHEET_H_


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
#define ID_COPTIONS 10130
#define SYMBOL_COPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_COPTIONS_TITLE _("Options")
#define SYMBOL_COPTIONS_IDNAME ID_COPTIONS
#define SYMBOL_COPTIONS_SIZE wxSize(400, 300)
#define SYMBOL_COPTIONS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * COptions class declaration
 */

class COptions: public wxPropertySheetDialog
{    
  DECLARE_DYNAMIC_CLASS( COptions )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  COptions();
  COptions( wxWindow* parent, wxWindowID id = SYMBOL_COPTIONS_IDNAME, const wxString& caption = SYMBOL_COPTIONS_TITLE, const wxPoint& pos = SYMBOL_COPTIONS_POSITION, const wxSize& size = SYMBOL_COPTIONS_SIZE, long style = SYMBOL_COPTIONS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COPTIONS_IDNAME, const wxString& caption = SYMBOL_COPTIONS_TITLE, const wxPoint& pos = SYMBOL_COPTIONS_POSITION, const wxSize& size = SYMBOL_COPTIONS_SIZE, long style = SYMBOL_COPTIONS_STYLE );

  /// Destructor
  ~COptions();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin COptions event handler declarations
////@end COptions event handler declarations

////@begin COptions member function declarations
  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end COptions member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin COptions member variables
////@end COptions member variables
};

#endif
  // _OPTIONSPROPSHEET_H_
