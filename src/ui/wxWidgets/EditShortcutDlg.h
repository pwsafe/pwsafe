/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file EditShortcutDlg.h
* 
*/

#ifndef _EDITSHORTCUTDLG_H_
#define _EDITSHORTCUTDLG_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes
#include "core/PWScore.h"
#include "core/ItemData.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_EDITSHORTCUTDLG 10174
#define ID_SC_DISP 10201
#define ID_SC_GROUP 10202
#define ID_TEXTCTRL16 10203
#define ID_TEXTCTRL17 10204
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_EDITSHORTCUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#else
#define SYMBOL_EDITSHORTCUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#endif
#define SYMBOL_EDITSHORTCUTDLG_TITLE _("Edit Shortcut")
#define SYMBOL_EDITSHORTCUTDLG_IDNAME ID_EDITSHORTCUTDLG
#define SYMBOL_EDITSHORTCUTDLG_SIZE wxSize(400, 300)
#define SYMBOL_EDITSHORTCUTDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * EditShortcutDlg class declaration
 */

class EditShortcutDlg : public wxDialog
{
  DECLARE_CLASS( EditShortcutDlg )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  EditShortcutDlg(wxWindow* parent, PWScore &core, CItemData *item,
               wxWindowID id = SYMBOL_EDITSHORTCUTDLG_IDNAME,
               const wxString& caption = SYMBOL_EDITSHORTCUTDLG_TITLE,
               const wxPoint& pos = SYMBOL_EDITSHORTCUTDLG_POSITION,
               const wxSize& size = SYMBOL_EDITSHORTCUTDLG_SIZE,
               long style = SYMBOL_EDITSHORTCUTDLG_STYLE);

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EDITSHORTCUTDLG_IDNAME, const wxString& caption = SYMBOL_EDITSHORTCUTDLG_TITLE, const wxPoint& pos = SYMBOL_EDITSHORTCUTDLG_POSITION, const wxSize& size = SYMBOL_EDITSHORTCUTDLG_SIZE, long style = SYMBOL_EDITSHORTCUTDLG_STYLE );

  /// Destructor
  ~EditShortcutDlg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin EditShortcutDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& evt);

////@end EditShortcutDlg event handler declarations

////@begin EditShortcutDlg member function declarations

  wxString GetCreated() const { return m_created ; }
  void SetCreated(wxString value) { m_created = value ; }

  wxString GetLastChanged() const { return m_lastChanged ; }
  void SetLastChanged(wxString value) { m_lastChanged = value ; }

  wxString GetLastAccess() const { return m_lastAccess ; }
  void SetLastAccess(wxString value) { m_lastAccess = value ; }

  wxString GetLastAny() const { return m_lastAny ; }
  void SetLastAny(wxString value) { m_lastAny = value ; }

  wxString GetTitle() const { return m_title ; }
  void SetTitle(wxString value) { m_title = value ; }

  wxString GetUser() const { return m_user ; }
  void SetUser(wxString value) { m_user = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end EditShortcutDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin EditShortcutDlg member variables

private:
  wxComboBox* m_groupCtrl;
  wxString m_created;
  wxString m_lastChanged;
  wxString m_lastAccess;
  wxString m_lastAny;
  wxString m_title;
  wxString m_user;
////@end EditShortcutDlg member variables
  void ItemFieldsToDialog();
  PWScore &m_core;
  CItemData *m_item;
};

#endif // _EDITSHORTCUTDLG_H_
