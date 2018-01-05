/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file passwordsubset.h
* 
*/

#ifndef _PASSWORDSUBSET_H_
#define _PASSWORDSUBSET_H_

/*!
 * Includes
 */
#include "core/StringX.h"
////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

// Following since DialogBlocks insists that wxDIALOG_MODAL exists...
#ifndef wxDIALOG_MODAL
#define wxDIALOG_MODAL 0
#endif

////@begin control identifiers
#define ID_CPASSWORDSUBSET 10000
#define ID_TEXTCTRL_POS 10001
#define ID_TEXTCTRL_VAL 10002
#define ID_BITMAPBUTTON 10003
#define SYMBOL_CPASSWORDSUBSET_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_CPASSWORDSUBSET_TITLE _("Show a subset of the Password")
#define SYMBOL_CPASSWORDSUBSET_IDNAME ID_CPASSWORDSUBSET
#define SYMBOL_CPASSWORDSUBSET_SIZE wxSize(400, 300)
#define SYMBOL_CPASSWORDSUBSET_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CPasswordSubset class declaration
 */

class CPasswordSubset: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( CPasswordSubset )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CPasswordSubset();
  CPasswordSubset( wxWindow* parent, const StringX &password,
                   wxWindowID id = SYMBOL_CPASSWORDSUBSET_IDNAME, const wxString& caption = SYMBOL_CPASSWORDSUBSET_TITLE, const wxPoint& pos = SYMBOL_CPASSWORDSUBSET_POSITION, const wxSize& size = SYMBOL_CPASSWORDSUBSET_SIZE, long style = SYMBOL_CPASSWORDSUBSET_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPASSWORDSUBSET_IDNAME, const wxString& caption = SYMBOL_CPASSWORDSUBSET_TITLE, const wxPoint& pos = SYMBOL_CPASSWORDSUBSET_POSITION, const wxSize& size = SYMBOL_CPASSWORDSUBSET_SIZE, long style = SYMBOL_CPASSWORDSUBSET_STYLE );

  /// Destructor
  ~CPasswordSubset();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin CPasswordSubset event handler declarations

  /// wxEVT_CHAR event handler for ID_TEXTCTRL_POS
  void OnChar( wxKeyEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
  void OnBitmapbuttonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick( wxCommandEvent& event );

////@end CPasswordSubset event handler declarations

////@begin CPasswordSubset member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CPasswordSubset member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();
 private:
  const StringX m_password;
////@begin CPasswordSubset member variables
  wxTextCtrl* m_pos;
  wxTextCtrl* m_vals;
  wxStaticText* m_error;
////@end CPasswordSubset member variables
};

#endif
  // _PASSWORDSUBSET_H_
