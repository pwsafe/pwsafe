/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/valgen.h>
////@end includes
#include "core/PWScore.h"

#ifndef NO_YUBI
#include "YubiMixin.h"
#endif

/*!
 * Forward declarations
 */

////@begin forward declarations
class CSafeCombinationCtrl;
////@end forward declarations
class wxTimer;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSAFECOMBINATIONPROMPT 10062
#define ID_PASSWORD 10008
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_CSAFECOMBINATIONPROMPT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#else
#define SYMBOL_CSAFECOMBINATIONPROMPT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#endif
#define ID_YUBIBTN 10229
#define ID_YUBISTATUS 10230
#define SYMBOL_CSAFECOMBINATIONPROMPT_TITLE _("Enter Safe Combination")
#define SYMBOL_CSAFECOMBINATIONPROMPT_IDNAME ID_CSAFECOMBINATIONPROMPT
#define SYMBOL_CSAFECOMBINATIONPROMPT_SIZE wxSize(400, 300)
#define SYMBOL_CSAFECOMBINATIONPROMPT_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CSafeCombinationPrompt class declaration
 */

#ifndef NO_YUBI
class CSafeCombinationPrompt: public wxDialog, private CYubiMixin
#else
class CSafeCombinationPrompt: public wxDialog
#endif
{
  DECLARE_CLASS( CSafeCombinationPrompt )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CSafeCombinationPrompt(wxWindow* parent, PWScore &core, const wxString &fname,
                         wxWindowID id = SYMBOL_CSAFECOMBINATIONPROMPT_IDNAME, const wxString& caption = SYMBOL_CSAFECOMBINATIONPROMPT_TITLE, const wxPoint& pos = SYMBOL_CSAFECOMBINATIONPROMPT_POSITION, const wxSize& size = SYMBOL_CSAFECOMBINATIONPROMPT_SIZE, long style = SYMBOL_CSAFECOMBINATIONPROMPT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSAFECOMBINATIONPROMPT_IDNAME, const wxString& caption = SYMBOL_CSAFECOMBINATIONPROMPT_TITLE, const wxPoint& pos = SYMBOL_CSAFECOMBINATIONPROMPT_POSITION, const wxSize& size = SYMBOL_CSAFECOMBINATIONPROMPT_SIZE, long style = SYMBOL_CSAFECOMBINATIONPROMPT_STYLE );

  /// Destructor
  ~CSafeCombinationPrompt();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  StringX GetPassword() const {return m_password;}

////@begin CSafeCombinationPrompt event handler declarations

#ifndef NO_YUBI
  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
  void OnYubibtnClick( wxCommandEvent& event );

////@end CSafeCombinationPrompt event handler declarations
  void OnPollingTimer(wxTimerEvent& timerEvent);
#endif

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT
  void OnExitClick( wxCommandEvent& event );

////@begin CSafeCombinationPrompt member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CSafeCombinationPrompt member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CSafeCombinationPrompt member variables
  CSafeCombinationCtrl* m_scctrl;
////@end CSafeCombinationPrompt member variables
  PWScore &m_core;
  wxString m_filename;
  StringX  m_password;
  unsigned m_tries;

#ifndef NO_YUBI
  wxBitmapButton* m_YubiBtn;
  wxStaticText* m_yubiStatusCtrl;
  wxTimer* m_pollingTimer; // for Yubi, but can't go into mixin :-(
#endif

  void ProcessPhrase();
};

#endif
  // _SAFECOMBINATIONPROMPT_H_
