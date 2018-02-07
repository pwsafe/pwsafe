/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef _DELETECONFIRMATION_H_
#define _DELETECONFIRMATION_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
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
#define ID_DELETECONFIRMATION 10199
#define ID_CHECKBOX37 10200
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_DELETECONFIRMATION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#else
#define SYMBOL_DELETECONFIRMATION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#endif
#define SYMBOL_DELETECONFIRMATION_TITLE _("Delete Confirmation")
#define SYMBOL_DELETECONFIRMATION_IDNAME ID_DELETECONFIRMATION
#define SYMBOL_DELETECONFIRMATION_SIZE wxSize(400, 300)
#define SYMBOL_DELETECONFIRMATION_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * DeleteConfirmation class declaration
 */

class DeleteConfirmation: public wxDialog
{    
  DECLARE_CLASS( DeleteConfirmation )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  DeleteConfirmation(int num_children);
  DeleteConfirmation( wxWindow* parent, int num_children, wxWindowID id = SYMBOL_DELETECONFIRMATION_IDNAME, const wxString& caption = SYMBOL_DELETECONFIRMATION_TITLE, const wxPoint& pos = SYMBOL_DELETECONFIRMATION_POSITION, const wxSize& size = SYMBOL_DELETECONFIRMATION_SIZE, long style = SYMBOL_DELETECONFIRMATION_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DELETECONFIRMATION_IDNAME, const wxString& caption = SYMBOL_DELETECONFIRMATION_TITLE, const wxPoint& pos = SYMBOL_DELETECONFIRMATION_POSITION, const wxSize& size = SYMBOL_DELETECONFIRMATION_SIZE, long style = SYMBOL_DELETECONFIRMATION_STYLE );

  /// Destructor
  ~DeleteConfirmation();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin DeleteConfirmation event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_YES
  void OnYesClick( wxCommandEvent& evt);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NO
  void OnNoClick( wxCommandEvent& evt);

////@end DeleteConfirmation event handler declarations

////@begin DeleteConfirmation member function declarations

  bool GetConfirmdelete() const { return m_confirmdelete ; }
  void SetConfirmdelete(bool value) { m_confirmdelete = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end DeleteConfirmation member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin DeleteConfirmation member variables
  wxStaticText* m_areyousure;
private:
  bool m_confirmdelete;
////@end DeleteConfirmation member variables
  int m_numchildren;
};

#endif
  // _DELETECONFIRMATION_H_
