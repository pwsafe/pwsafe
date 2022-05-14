/*
 * Copyright (c) 2003-2022 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SafeCombinationPromptDlg.h
* 
*/

#ifndef _SAFECOMBINATIONPROMPTDLG_H_
#define _SAFECOMBINATIONPROMPTDLG_H_

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
class SafeCombinationCtrl;
////@end forward declarations
class wxTimer;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SAFECOMBINATIONPROMPTDLG 10062
#define ID_PASSWORD 10008
#define ID_READONLY 10005
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_SAFECOMBINATIONPROMPTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#else
#define SYMBOL_SAFECOMBINATIONPROMPTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#endif
#define ID_YUBIBTN 10229
#define ID_YUBISTATUS 10230
#define SYMBOL_SAFECOMBINATIONPROMPTDLG_TITLE _("Enter Safe Combination")
#define SYMBOL_SAFECOMBINATIONPROMPTDLG_IDNAME ID_SAFECOMBINATIONPROMPTDLG
#define SYMBOL_SAFECOMBINATIONPROMPTDLG_SIZE wxSize(400, 300)
#define SYMBOL_SAFECOMBINATIONPROMPTDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * SafeCombinationPromptDlg class declaration
 */

#ifndef NO_YUBI
class SafeCombinationPromptDlg : public wxDialog, private YubiMixin
#else
class SafeCombinationPromptDlg : public wxDialog
#endif
{
  DECLARE_CLASS( SafeCombinationPromptDlg )
  DECLARE_EVENT_TABLE()


public:
  static SafeCombinationPromptDlg* Create(wxWindow *parent, PWScore &core, const wxString &fname, const bool allowExit = true,
                         wxWindowID id = SYMBOL_SAFECOMBINATIONPROMPTDLG_IDNAME, const wxString& caption = SYMBOL_SAFECOMBINATIONPROMPTDLG_TITLE, const wxPoint& pos = SYMBOL_SAFECOMBINATIONPROMPTDLG_POSITION, const wxSize& size = SYMBOL_SAFECOMBINATIONPROMPTDLG_SIZE, long style = SYMBOL_SAFECOMBINATIONPROMPTDLG_STYLE );
   /// Destructor
~SafeCombinationPromptDlg();

  StringX GetPassword() const {return m_password;}
protected:
  /// Constructors
  SafeCombinationPromptDlg(wxWindow *parent, PWScore &core, const wxString &fname, const bool allowExit,
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style);

  /// Creates the controls and sizers
  void CreateControls();

////@begin SafeCombinationPromptDlg event handler declarations

#ifndef NO_YUBI
  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
  void OnYubibtnClick( wxCommandEvent& event );

////@end SafeCombinationPromptDlg event handler declarations
  void OnPollingTimer(wxTimerEvent& timerEvent);
#endif

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT
  void OnExitClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_READONLY
  void OnReadonlyClick( wxCommandEvent& event );
  
////@begin SafeCombinationPromptDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end SafeCombinationPromptDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin SafeCombinationPromptDlg member variables
  SafeCombinationCtrl* m_scctrl = nullptr;
////@end SafeCombinationPromptDlg member variables
  PWScore &m_core;
  wxString m_filename;
  StringX  m_password;
  unsigned m_tries;
  bool m_readOnly;
  bool m_allowExit;
  
#ifndef NO_YUBI
  wxBitmapButton* m_YubiBtn = nullptr;
  wxStaticText* m_yubiStatusCtrl = nullptr;
  wxTimer* m_pollingTimer = nullptr; // for Yubi, but can't go into mixin :-(
#endif

  void ProcessPhrase();
  void UpdateReadOnlyCheckbox(wxCheckBox *checkBox);
};

#endif // _SAFECOMBINATIONPROMPTDLG_H_
