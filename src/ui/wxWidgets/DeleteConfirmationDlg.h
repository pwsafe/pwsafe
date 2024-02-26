/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DeleteConfirmationDlg.h
* 
*/

#ifndef _DELETECONFIRMATIONDLG_H_
#define _DELETECONFIRMATIONDLG_H_

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
#define ID_DELETECONFIRMATIONDLG 10199
#define ID_CHECKBOX37 10200
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_DELETECONFIRMATIONDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#else
#define SYMBOL_DELETECONFIRMATIONDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#endif
#define SYMBOL_DELETECONFIRMATIONDLG_TITLE _("Delete Confirmation")
#define SYMBOL_DELETECONFIRMATIONDLG_IDNAME ID_DELETECONFIRMATIONDLG
#define SYMBOL_DELETECONFIRMATIONDLG_SIZE wxSize(400, 300)
#define SYMBOL_DELETECONFIRMATIONDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * DeleteConfirmationDlg class declaration
 */

class DeleteConfirmationDlg : public wxDialog
{
  DECLARE_CLASS( DeleteConfirmationDlg )
  DECLARE_EVENT_TABLE()

public:
  /// Creation
  static DeleteConfirmationDlg* Create(wxWindow *parent, bool isGroup, wxWindowID id = SYMBOL_DELETECONFIRMATIONDLG_IDNAME, const wxString& caption = SYMBOL_DELETECONFIRMATIONDLG_TITLE, const wxPoint& pos = SYMBOL_DELETECONFIRMATIONDLG_POSITION, const wxSize& size = SYMBOL_DELETECONFIRMATIONDLG_SIZE, long style = SYMBOL_DELETECONFIRMATIONDLG_STYLE );

  /// Destructor
  ~DeleteConfirmationDlg() = default;

  bool GetConfirmdelete() const { return m_confirmdelete ; }
  void SetConfirmdelete(bool value) { m_confirmdelete = value ; }
protected:
  /// Constructors
  DeleteConfirmationDlg(wxWindow *parent, bool isGroup, wxWindowID id = SYMBOL_DELETECONFIRMATIONDLG_IDNAME, const wxString& caption = SYMBOL_DELETECONFIRMATIONDLG_TITLE, const wxPoint& pos = SYMBOL_DELETECONFIRMATIONDLG_POSITION, const wxSize& size = SYMBOL_DELETECONFIRMATIONDLG_SIZE, long style = SYMBOL_DELETECONFIRMATIONDLG_STYLE );

  /// Creates the controls and sizers
  void CreateControls();

////@begin DeleteConfirmationDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_YES
  void OnYesClick( wxCommandEvent& evt);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NO
  void OnNoClick( wxCommandEvent& evt);

////@end DeleteConfirmationDlg event handler declarations

////@begin DeleteConfirmationDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end DeleteConfirmationDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin DeleteConfirmationDlg member variables
  wxStaticText* m_areyousure = nullptr;
private:
  bool m_isGroup;
  bool m_confirmdelete;
////@end DeleteConfirmationDlg member variables
};

#endif // _DELETECONFIRMATIONDLG_H_
