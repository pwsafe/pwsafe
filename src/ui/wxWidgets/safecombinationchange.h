/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file safecombinationchange.h
* 
*/

#ifndef _SAFECOMBINATIONCHANGE_H_
#define _SAFECOMBINATIONCHANGE_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes
#include "corelib/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSAFECOMBINATIONCHANGE 10074
#define ID_OLDPASSWD 10075
#define ID_NEWPASSWD 10076
#define ID_CONFIRM 10077
#define SYMBOL_CSAFECOMBINATIONCHANGE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_CSAFECOMBINATIONCHANGE_TITLE _("Change Safe Combination")
#define SYMBOL_CSAFECOMBINATIONCHANGE_IDNAME ID_CSAFECOMBINATIONCHANGE
#define SYMBOL_CSAFECOMBINATIONCHANGE_SIZE wxSize(400, 300)
#define SYMBOL_CSAFECOMBINATIONCHANGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSafeCombinationChange class declaration
 */

class CSafeCombinationChange: public wxDialog
{    
  DECLARE_CLASS( CSafeCombinationChange )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CSafeCombinationChange(wxWindow* parent, PWScore &core,
                         wxWindowID id = SYMBOL_CSAFECOMBINATIONCHANGE_IDNAME, const wxString& caption = SYMBOL_CSAFECOMBINATIONCHANGE_TITLE, const wxPoint& pos = SYMBOL_CSAFECOMBINATIONCHANGE_POSITION, const wxSize& size = SYMBOL_CSAFECOMBINATIONCHANGE_SIZE, long style = SYMBOL_CSAFECOMBINATIONCHANGE_STYLE );

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id = SYMBOL_CSAFECOMBINATIONCHANGE_IDNAME, const wxString& caption = SYMBOL_CSAFECOMBINATIONCHANGE_TITLE, const wxPoint& pos = SYMBOL_CSAFECOMBINATIONCHANGE_POSITION, const wxSize& size = SYMBOL_CSAFECOMBINATIONCHANGE_SIZE, long style = SYMBOL_CSAFECOMBINATIONCHANGE_STYLE );

  /// Destructor
  ~CSafeCombinationChange();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin CSafeCombinationChange event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

////@end CSafeCombinationChange event handler declarations

////@begin CSafeCombinationChange member function declarations

  wxString GetOldpasswd() const { return m_oldpasswd ; }
  void SetOldpasswd(wxString value) { m_oldpasswd = value ; }

  wxString GetNewpasswd() const { return m_newpasswd ; }
  void SetNewpasswd(wxString value) { m_newpasswd = value ; }

  wxString GetConfirm() const { return m_confirm ; }
  void SetConfirm(wxString value) { m_confirm = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CSafeCombinationChange member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CSafeCombinationChange member variables
private:
  wxString m_oldpasswd;
  wxString m_newpasswd;
  wxString m_confirm;
////@end CSafeCombinationChange member variables
  PWScore &m_core;
};

#endif
  // _SAFECOMBINATIONCHANGE_H_
