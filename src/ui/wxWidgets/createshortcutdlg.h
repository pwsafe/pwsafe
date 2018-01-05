/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file createshortcutdlg.h
* 
*/

#ifndef _CREATESHORTCUTDLG_H_
#define _CREATESHORTCUTDLG_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes
#include "core/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class UIInterFace;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CREATESHORTCUTDLG 10205
#define ID_COMBOBOX4 10206
#define ID_TEXTCTRL18 10207
#define ID_TEXTCTRL19 10208
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_CREATESHORTCUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#else
#define SYMBOL_CREATESHORTCUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#endif
#define SYMBOL_CREATESHORTCUTDLG_TITLE _("Create Shortcut")
#define SYMBOL_CREATESHORTCUTDLG_IDNAME ID_CREATESHORTCUTDLG
#define SYMBOL_CREATESHORTCUTDLG_SIZE wxSize(300, 300)
#define SYMBOL_CREATESHORTCUTDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CreateShortcutDlg class declaration
 */

class CreateShortcutDlg: public wxDialog
{    
  DECLARE_CLASS( CreateShortcutDlg )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CreateShortcutDlg(wxWindow* parent, PWScore &core, CItemData *base,
                    wxWindowID id = SYMBOL_CREATESHORTCUTDLG_IDNAME,
                    const wxString& caption = SYMBOL_CREATESHORTCUTDLG_TITLE,
                    const wxPoint& pos = SYMBOL_CREATESHORTCUTDLG_POSITION,
                    const wxSize& size = SYMBOL_CREATESHORTCUTDLG_SIZE,
                    long style = SYMBOL_CREATESHORTCUTDLG_STYLE);

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREATESHORTCUTDLG_IDNAME, const wxString& caption = SYMBOL_CREATESHORTCUTDLG_TITLE, const wxPoint& pos = SYMBOL_CREATESHORTCUTDLG_POSITION, const wxSize& size = SYMBOL_CREATESHORTCUTDLG_SIZE, long style = SYMBOL_CREATESHORTCUTDLG_STYLE );

  /// Destructor
  ~CreateShortcutDlg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin CreateShortcutDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& evt);

////@end CreateShortcutDlg event handler declarations

////@begin CreateShortcutDlg member function declarations

  wxString GetHeading() const { return m_heading ; }
  void SetHeading(wxString value) { m_heading = value ; }

  wxString GetTitle() const { return m_title ; }
  void SetTitle(wxString value) { m_title = value ; }

  wxString GetUser() const { return m_user ; }
  void SetUser(wxString value) { m_user = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CreateShortcutDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CreateShortcutDlg member variables
  wxComboBox* m_groupCtrl;
  wxString m_user;
private:
  wxString m_heading;
  wxString m_title;
////@end CreateShortcutDlg member variables
  void ItemFieldsToDialog();
  PWScore &m_core;
  CItemData *m_base;
  UIInterFace *m_ui;
};

#endif
  // _CREATESHORTCUTDLG_H_
