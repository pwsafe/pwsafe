/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PasswordPolicy.h
* 
*/


#ifndef _PASSWORDPOLICY_H_
#define _PASSWORDPOLICY_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
#include "wx/spinctrl.h"
////@end includes
#include "core/coredefs.h"
#include "core/PWPolicy.h"
#include "core/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGridSizer;
class wxBoxSizer;
class wxSpinCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CPASSWORDPOLICY 10221
#define ID_TEXTCTRL21 10223
#define ID_SPINCTRL3 10117
#define ID_CHECKBOX3 10118
#define ID_SPINCTRL5 10126
#define ID_CHECKBOX4 10119
#define ID_SPINCTRL6 10127
#define ID_CHECKBOX5 10120
#define ID_SPINCTRL7 10128
#define ID_CHECKBOX6 10121
#define ID_SPINCTRL8 10129
#define IDC_USE_DEFAULTSYMBOLS 10210
#define IDC_STATIC_DEFAULT_SYMBOLS 10213
#define IDC_USE_OWNSYMBOLS 10211
#define IDC_OWNSYMBOLS 10212
#define ID_CHECKBOX7 10122
#define ID_CHECKBOX8 10123
#define ID_CHECKBOX9 10124
#define SYMBOL_CPASSWORDPOLICY_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_CPASSWORDPOLICY_TITLE _("Password Policy")
#define SYMBOL_CPASSWORDPOLICY_IDNAME ID_CPASSWORDPOLICY
#define SYMBOL_CPASSWORDPOLICY_SIZE wxSize(400, 300)
#define SYMBOL_CPASSWORDPOLICY_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPasswordPolicy class declaration
 */

class CPasswordPolicy: public wxDialog
{    
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CPasswordPolicy( wxWindow* parent, PWScore &core,
		   wxWindowID id = SYMBOL_CPASSWORDPOLICY_IDNAME,
		   const wxString& caption = SYMBOL_CPASSWORDPOLICY_TITLE,
		   const wxPoint& pos = SYMBOL_CPASSWORDPOLICY_POSITION,
		   const wxSize& size = SYMBOL_CPASSWORDPOLICY_SIZE,
		   long style = SYMBOL_CPASSWORDPOLICY_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPASSWORDPOLICY_IDNAME, const wxString& caption = SYMBOL_CPASSWORDPOLICY_TITLE, const wxPoint& pos = SYMBOL_CPASSWORDPOLICY_POSITION, const wxSize& size = SYMBOL_CPASSWORDPOLICY_SIZE, long style = SYMBOL_CPASSWORDPOLICY_STYLE );

  /// Destructor
  ~CPasswordPolicy();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin CPasswordPolicy event handler declarations

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
  void OnPwPolUseClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );

////@end CPasswordPolicy event handler declarations

////@begin CPasswordPolicy member function declarations

  wxString GetPolname() const { return m_polname ; }
  void SetPolname(wxString value) { m_polname = value ; }

  int GetPwdefaultlength() const { return m_pwdefaultlength ; }
  void SetPwdefaultlength(int value) { m_pwdefaultlength = value ; }

  bool GetPwUseLowercase() const { return m_pwUseLowercase ; }
  void SetPwUseLowercase(bool value) { m_pwUseLowercase = value ; }

  bool GetPwUseUppercase() const { return m_pwUseUppercase ; }
  void SetPwUseUppercase(bool value) { m_pwUseUppercase = value ; }

  bool GetPwUseDigits() const { return m_pwUseDigits ; }
  void SetPwUseDigits(bool value) { m_pwUseDigits = value ; }

  bool GetPwUseSymbols() const { return m_pwUseSymbols ; }
  void SetPwUseSymbols(bool value) { m_pwUseSymbols = value ; }

  bool GetPwUseHex() const { return m_pwUseHex ; }
  void SetPwUseHex(bool value) { m_pwUseHex = value ; }

  bool GetPwUseEasyVision() const { return m_pwUseEasyVision ; }
  void SetPwUseEasyVision(bool value) { m_pwUseEasyVision = value ; }

  bool GetPwMakePronounceable() const { return m_pwMakePronounceable ; }
  void SetPwMakePronounceable(bool value) { m_pwMakePronounceable = value ; }

  int GetPwLowerMinLength() const { return m_pwLowerMinLength ; }
  void SetPwLowerMinLength(int value) { m_pwLowerMinLength = value ; }

  int GetPwUpperMinLength() const { return m_pwUpperMinLength ; }
  void SetPwUpperMinLength(int value) { m_pwUpperMinLength = value ; }

  int GetPwSymbolMinLength() const { return m_pwSymbolMinLength ; }
  void SetPwSymbolMinLength(int value) { m_pwSymbolMinLength = value ; }

  int GetPwDigitMinLength() const { return m_pwDigitMinLength ; }
  void SetPwDigitMinLength(int value) { m_pwDigitMinLength = value ; }

  wxString GetSymbols() const { return m_Symbols ; }
  void SetSymbols(wxString value) { m_Symbols = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CPasswordPolicy member function declarations
  void SetPolicyData(const wxString &policyname, PSWDPolicyMap &MapPSWDPLC);
  void GetPolicyData(st_PSWDPolicy &st_pp, wxString &policyname, PSWDPolicyMap &MapPSWDPLC)
  {st_pp = m_st_default_pp; policyname = m_polname; MapPSWDPLC = m_MapPSWDPLC;}

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CPasswordPolicy member variables
  wxGridSizer* m_pwMinsGSzr;
  wxCheckBox* m_pwpUseLowerCtrl;
  wxBoxSizer* m_pwNumLCbox;
  wxSpinCtrl* m_pwpLCSpin;
  wxCheckBox* m_pwpUseUpperCtrl;
  wxBoxSizer* m_pwNumUCbox;
  wxSpinCtrl* m_pwpUCSpin;
  wxCheckBox* m_pwpUseDigitsCtrl;
  wxBoxSizer* m_pwNumDigbox;
  wxSpinCtrl* m_pwpDigSpin;
  wxCheckBox* m_pwpSymCtrl;
  wxBoxSizer* m_pwNumSymbox;
  wxSpinCtrl* m_pwpSymSpin;
  wxCheckBox* m_pwpEasyCtrl;
  wxCheckBox* m_pwpPronounceCtrl;
  wxCheckBox* m_pwpHexCtrl;
private:
  wxString m_polname;
  int m_pwdefaultlength;
  bool m_pwUseLowercase;
  bool m_pwUseUppercase;
  bool m_pwUseDigits;
  bool m_pwUseSymbols;
  bool m_pwUseHex;
  bool m_pwUseEasyVision;
  bool m_pwMakePronounceable;
  int m_pwLowerMinLength;
  int m_pwUpperMinLength;
  int m_pwSymbolMinLength;
  int m_pwDigitMinLength;
  wxString m_Symbols;
////@end CPasswordPolicy member variables
  PWScore &m_core;
  wxString m_oldpolname;
  int m_oldpwdefaultlength;
  bool m_oldpwUseLowercase;
  bool m_oldpwUseUppercase;
  bool m_oldpwUseDigits;
  bool m_oldpwUseSymbols;
  bool m_oldpwUseHex;
  bool m_oldpwUseEasyVision;
  bool m_oldpwMakePronounceable;
  int m_oldpwLowerMinLength;
  int m_oldpwUpperMinLength;
  int m_oldpwSymbolMinLength;
  int m_oldpwDigitMinLength;
  wxString m_oldSymbols;
  st_PSWDPolicy m_st_default_pp;
  PSWDPolicyMap m_MapPSWDPLC;
  PSWDPolicyMapIter m_iter;

  stringT m_std_symbols, m_easyvision_symbols, m_pronounceable_symbols;
};

#endif
  // _PASSWORDPOLICY_H_
