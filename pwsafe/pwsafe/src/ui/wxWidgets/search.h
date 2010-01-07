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

#ifndef _SEARCH_H_
#define _SEARCH_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
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
#define ID_ADVANCEDSEARCHOPTIONSDLG 10188
#define ID_CHECKBOX35 10189
#define ID_COMBOCTRL 10190
#define ID_COMBOCTRL1 10191
#define ID_TEXTCTRL15 10192
#define ID_CHECKBOX36 10193
#define ID_LISTBOX1 10194
#define ID_BUTTON10 10195
#define ID_BUTTON11 10196
#define ID_BUTTON12 10197
#define ID_BUTTON13 10198
#define ID_LISTBOX 10000
#define SYMBOL_ADVANCEDSEARCHOPTIONSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_ADVANCEDSEARCHOPTIONSDLG_TITLE _("Advanced Find options")
#define SYMBOL_ADVANCEDSEARCHOPTIONSDLG_IDNAME ID_ADVANCEDSEARCHOPTIONSDLG
#define SYMBOL_ADVANCEDSEARCHOPTIONSDLG_SIZE wxSize(400, 300)
#define SYMBOL_ADVANCEDSEARCHOPTIONSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * AdvancedSearchOptionsDlg class declaration
 */

class AdvancedSearchOptionsDlg: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( AdvancedSearchOptionsDlg )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  AdvancedSearchOptionsDlg();
  AdvancedSearchOptionsDlg( wxWindow* parent, wxWindowID id = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_SIZE, long style = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_SIZE, long style = SYMBOL_ADVANCEDSEARCHOPTIONSDLG_STYLE );

  /// Destructor
  ~AdvancedSearchOptionsDlg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin AdvancedSearchOptionsDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON10
  void OnSelOneButtonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
  void OnSelAllButtonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
  void OnDeSelOneButtonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13
  void OnDeSelAllButtonClick( wxCommandEvent& event );

////@end AdvancedSearchOptionsDlg event handler declarations

////@begin AdvancedSearchOptionsDlg member function declarations

  bool GetRestrict2subset() const { return m_restrict2subset ; }
  void SetRestrict2subset(bool value) { m_restrict2subset = value ; }

  wxString GetFieldText() const { return m_fieldText ; }
  void SetFieldText(wxString value) { m_fieldText = value ; }

  bool GetCaseSensitive() const { return m_caseSensitive ; }
  void SetCaseSensitive(bool value) { m_caseSensitive = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end AdvancedSearchOptionsDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin AdvancedSearchOptionsDlg member variables
  wxComboCtrl* m_fieldsCombo;
  wxComboCtrl* m_relationCombo;
  wxListBox* m_availFields;
  wxListBox* m_selectedFields;
private:
  bool m_restrict2subset;
  wxString m_fieldText;
  bool m_caseSensitive;
////@end AdvancedSearchOptionsDlg member variables
};

#endif
  // _SEARCH_H_
