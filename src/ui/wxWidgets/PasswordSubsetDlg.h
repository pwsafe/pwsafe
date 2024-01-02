/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PasswordSubsetDlg.h
* 
*/

#ifndef _PASSWORDSUBSETDLG_H_
#define _PASSWORDSUBSETDLG_H_

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
#define ID_PASSWORDSUBSETDLG 10000
#define ID_TEXTCTRL_POS 10001
#define ID_TEXTCTRL_VAL 10002
#define ID_BITMAPBUTTON 10003
#define SYMBOL_PASSWORDSUBSETDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_PASSWORDSUBSETDLG_TITLE _("Show a subset of the Password")
#define SYMBOL_PASSWORDSUBSETDLG_IDNAME ID_PASSWORDSUBSETDLG
#define SYMBOL_PASSWORDSUBSETDLG_SIZE wxSize(400, 300)
#define SYMBOL_PASSWORDSUBSETDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * PasswordSubsetDlg class declaration
 */

class PasswordSubsetDlg : public wxDialog
{
  DECLARE_DYNAMIC_CLASS( PasswordSubsetDlg )
  DECLARE_EVENT_TABLE()

public:
   static PasswordSubsetDlg* Create(wxWindow *parent, const StringX &password,
                   wxWindowID id = SYMBOL_PASSWORDSUBSETDLG_IDNAME, const wxString& caption = SYMBOL_PASSWORDSUBSETDLG_TITLE, const wxPoint& pos = SYMBOL_PASSWORDSUBSETDLG_POSITION, const wxSize& size = SYMBOL_PASSWORDSUBSETDLG_SIZE, long style = SYMBOL_PASSWORDSUBSETDLG_STYLE );
  /// Destructor
  ~PasswordSubsetDlg() = default;

protected:
  /// Constructors
  PasswordSubsetDlg() = default;
  PasswordSubsetDlg(wxWindow *parent, const StringX &password, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style);

  /// Creates the controls and sizers
  void CreateControls();

////@begin PasswordSubsetDlg event handler declarations

  /// wxEVT_CHAR event handler for ID_TEXTCTRL_POS
  void OnChar( wxKeyEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
  void OnBitmapbuttonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick( wxCommandEvent& event );

  void OnPosListChanged( wxCommandEvent& event );

////@end PasswordSubsetDlg event handler declarations

////@begin PasswordSubsetDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end PasswordSubsetDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

private:
  bool GetSubsetString(const wxString& subset, bool with_delims, StringX& result) const;
  const StringX m_password;
////@begin PasswordSubsetDlg member variables
  wxTextCtrl* m_pos = nullptr;
  wxTextCtrl* m_vals = nullptr;
  wxStaticText* m_error = nullptr;
  wxBitmapButton* m_copyBtn = nullptr;
////@end PasswordSubsetDlg member variables
};

#endif // _PASSWORDSUBSETDLG_H_
