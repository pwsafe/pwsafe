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

#ifndef _SAFECOMBINATIONPROMPT_H_
#define _SAFECOMBINATIONPROMPT_H_


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
#define ID_CSAFECOMBINATIONPROMPT 10062
#define ID_TEXTCTRL 10063
#define SYMBOL_CSAFECOMBINATIONPROMPT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_CSAFECOMBINATIONPROMPT_TITLE _("Enter Safe Combination")
#define SYMBOL_CSAFECOMBINATIONPROMPT_IDNAME ID_CSAFECOMBINATIONPROMPT
#define SYMBOL_CSAFECOMBINATIONPROMPT_SIZE wxSize(400, 300)
#define SYMBOL_CSAFECOMBINATIONPROMPT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSafeCombinationPrompt class declaration
 */

class CSafeCombinationPrompt: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( CSafeCombinationPrompt )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CSafeCombinationPrompt();
  CSafeCombinationPrompt( wxWindow* parent, wxWindowID id = SYMBOL_CSAFECOMBINATIONPROMPT_IDNAME, const wxString& caption = SYMBOL_CSAFECOMBINATIONPROMPT_TITLE, const wxPoint& pos = SYMBOL_CSAFECOMBINATIONPROMPT_POSITION, const wxSize& size = SYMBOL_CSAFECOMBINATIONPROMPT_SIZE, long style = SYMBOL_CSAFECOMBINATIONPROMPT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSAFECOMBINATIONPROMPT_IDNAME, const wxString& caption = SYMBOL_CSAFECOMBINATIONPROMPT_TITLE, const wxPoint& pos = SYMBOL_CSAFECOMBINATIONPROMPT_POSITION, const wxSize& size = SYMBOL_CSAFECOMBINATIONPROMPT_SIZE, long style = SYMBOL_CSAFECOMBINATIONPROMPT_STYLE );

  /// Destructor
  ~CSafeCombinationPrompt();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  void SetFileName(const wxString &fname) {m_filename = fname;}
  wxString GetPassword() const {return m_passwd;}
  
////@begin CSafeCombinationPrompt event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

////@end CSafeCombinationPrompt event handler declarations

////@begin CSafeCombinationPrompt member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CSafeCombinationPrompt member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CSafeCombinationPrompt member variables
  wxString m_filename;
  wxString m_passwd;
////@end CSafeCombinationPrompt member variables
};

#endif
  // _SAFECOMBINATIONPROMPT_H_
