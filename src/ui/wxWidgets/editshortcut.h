/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _EDITSHORTCUT_H_
#define _EDITSHORTCUT_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/combo.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxComboCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_EDITSHORTCUT 10174
#define ID_SC_DISP 10201
#define ID_SC_GROUP 10202
#define ID_TEXTCTRL16 10203
#define ID_TEXTCTRL17 10204
#define SYMBOL_EDITSHORTCUT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_EDITSHORTCUT_TITLE _("Edit Shortcut")
#define SYMBOL_EDITSHORTCUT_IDNAME ID_EDITSHORTCUT
#define SYMBOL_EDITSHORTCUT_SIZE wxSize(400, 300)
#define SYMBOL_EDITSHORTCUT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * EditShortcut class declaration
 */

class EditShortcut: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( EditShortcut )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  EditShortcut();
  EditShortcut( wxWindow* parent, wxWindowID id = SYMBOL_EDITSHORTCUT_IDNAME, const wxString& caption = SYMBOL_EDITSHORTCUT_TITLE, const wxPoint& pos = SYMBOL_EDITSHORTCUT_POSITION, const wxSize& size = SYMBOL_EDITSHORTCUT_SIZE, long style = SYMBOL_EDITSHORTCUT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EDITSHORTCUT_IDNAME, const wxString& caption = SYMBOL_EDITSHORTCUT_TITLE, const wxPoint& pos = SYMBOL_EDITSHORTCUT_POSITION, const wxSize& size = SYMBOL_EDITSHORTCUT_SIZE, long style = SYMBOL_EDITSHORTCUT_STYLE );

  /// Destructor
  ~EditShortcut();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin EditShortcut event handler declarations

////@end EditShortcut event handler declarations

////@begin EditShortcut member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end EditShortcut member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin EditShortcut member variables
  wxComboCtrl* m_group;
  wxTextCtrl* m_title;
  wxTextCtrl* m_user;
  wxStaticText* m_created;
  wxStaticText* m_lastChanged;
  wxStaticText* m_lastAccess;
  wxStaticText* m_lastAny;
////@end EditShortcut member variables
};

#endif
  // _EDITSHORTCUT_H_
