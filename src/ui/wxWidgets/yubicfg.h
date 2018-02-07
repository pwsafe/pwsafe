/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file yubicfg.h
* 
*/

#ifndef _YUBICFG_H_
#define _YUBICFG_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes
#include "core/PWSfileHeader.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class wxTimer;
class PWScore;

/*!
 * Control identifiers
 */

#ifndef wxDIALOG_MODAL
#define wxDIALOG_MODAL 0
#endif

////@begin control identifiers
#define ID_YUBICFGDLG 10109
#define ID_YK_SERNUM 10113
#define ID_YKSK 10114
#define ID_YK_HIDESHOW 10125
#define ID_YK_GENERATE 10224
#define ID_YK_SET 10228
#define SYMBOL_YUBICFGDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_YUBICFGDLG_TITLE _("YubiKey Configuration")
#define SYMBOL_YUBICFGDLG_IDNAME ID_YUBICFGDLG
#define SYMBOL_YUBICFGDLG_SIZE wxSize(400, 300)
#define SYMBOL_YUBICFGDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * YubiCfgDlg class declaration
 */

class YubiCfgDlg: public wxDialog
{    
  DECLARE_CLASS( YubiCfgDlg )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  YubiCfgDlg( wxWindow* parent, PWScore &core, wxWindowID id = SYMBOL_YUBICFGDLG_IDNAME, const wxString& caption = SYMBOL_YUBICFGDLG_TITLE, const wxPoint& pos = SYMBOL_YUBICFGDLG_POSITION, const wxSize& size = SYMBOL_YUBICFGDLG_SIZE, long style = SYMBOL_YUBICFGDLG_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_YUBICFGDLG_IDNAME, const wxString& caption = SYMBOL_YUBICFGDLG_TITLE, const wxPoint& pos = SYMBOL_YUBICFGDLG_POSITION, const wxSize& size = SYMBOL_YUBICFGDLG_SIZE, long style = SYMBOL_YUBICFGDLG_STYLE );

  /// Destructor
  ~YubiCfgDlg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin YubiCfgDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_HIDESHOW
  void OnYkHideshowClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_GENERATE
  void OnYkGenerateClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET
  void OnYkSetClick( wxCommandEvent& event );

////@end YubiCfgDlg event handler declarations
  void OnPollingTimer(wxTimerEvent& timerEvent);

////@begin YubiCfgDlg member function declarations

  wxString GetYksernum() const { return m_yksernum ; }
  void SetYksernum(wxString value) { m_yksernum = value ; }

  wxString GetYksk() const { return m_yksk ; }
  void SetYksk(wxString value) { m_yksk = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end YubiCfgDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin YubiCfgDlg member variables
  wxStaticBoxSizer* m_SKSizer;
  wxTextCtrl* m_SKCtrl;
  wxStaticText* m_ykstatus;
private:
  wxString m_yksernum; // Device's serial number
  wxString m_yksk; // Device's secret key
////@end YubiCfgDlg member variables
  enum {YUBI_SK_LEN = PWSfileHeader::YUBI_SK_LEN};
  void ReadYubiSN();
  bool IsYubiInserted() const;
  void yubiInserted(void); // called when Yubikey's inserted
  void yubiRemoved(void);  // called when Yubikey's removed
  void ShowSK();
  void HideSK();

  enum { POLLING_TIMER_ID = 66 } ; 
  wxTimer* m_pollingTimer;
  bool m_present; // key present?
  bool m_isSKHidden;
  mutable wxMutex m_mutex; // protect against race conditions when calling Yubi API
  PWScore &m_core;
};

#endif
  // _YUBICFG_H_
