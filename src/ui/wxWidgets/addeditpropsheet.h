/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/**
 *  \file addeditpropsheet.h
 */

#ifndef _ADDEDITPROPSHEET_H_
#define _ADDEDITPROPSHEET_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/propdlg.h>
#include <wx/valgen.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>
#include <wx/grid.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/statline.h>
////@end includes
#include "core/ItemData.h"
#include "core/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
class wxSpinCtrl;
class wxGrid;
class wxDatePickerCtrl;
class wxGridSizer;
class wxBoxSizer;
////@end forward declarations

#ifndef wxDIALOG_MODAL
#define wxDIALOG_MODAL 0
#endif

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ADDEDITPROPSHEET 10083
#define ID_PANEL_BASIC 10084
#define ID_COMBOBOX_GROUP 10095
#define ID_TEXTCTRL_TITLE 10096
#define ID_TEXTCTRL_USERNAME 10088
#define ID_TEXTCTRL_PASSWORD 10089
#define ID_BUTTON2 10090
#define ID_BUTTON_GENERATE 10097
#define ID_TEXTCTRL_PASSWORD2 10091
#define ID_TEXTCTRL_URL 10092
#define ID_GO_BTN 10093
#define ID_TEXTCTRL_EMAIL 10100
#define ID_SEND_BTN 10214
#define ID_TEXTCTRL_NOTES 10098
#define ID_PANEL_ADDITIONAL 10085
#define ID_TEXTCTRL_AUTOTYPE 10094
#define ID_TEXTCTRL_RUN_CMD 10099
#define ID_COMBOBOX_DBC_ACTION 10101
#define ID_COMBOBOX_SDBC_ACTION 10000
#define ID_CHECKBOX_KEEP 10102
#define ID_SPINCTRL_MAX_PW_HIST 10103
#define ID_GRID_PW_HIST 10104
#define ID_BUTTON_CLEAR_HIST 10105
#define ID_BUTTON_COPY_ALL 10106
#define ID_PANEL_DTIME 10086
#define ID_RADIOBUTTON_ON 10107
#define ID_DATECTRL_EXP_DATE 10108
#define ID_RADIOBUTTON_IN 10110
#define ID_SPINCTRL_EXP_TIME 10111
#define ID_CHECKBOX_RECURRING 10112
#define ID_RADIOBUTTON_NEVER 10001
#define ID_PANEL_PPOLICY 10087
#define ID_CHECKBOX42 10115
#define ID_POLICYLIST 10063
#define ID_SPINCTRL3 10117
#define ID_CHECKBOX3 10118
#define ID_SPINCTRL5 10126
#define ID_CHECKBOX4 10119
#define ID_SPINCTRL6 10127
#define ID_CHECKBOX5 10120
#define ID_SPINCTRL7 10128
#define ID_CHECKBOX6 10121
#define ID_SPINCTRL8 10129
#define IDC_OWNSYMBOLS 10212
#define ID_RESET_SYMBOLS 10109
#define ID_CHECKBOX7 10122
#define ID_CHECKBOX8 10123
#define ID_CHECKBOX9 10124
#define ID_STATICTEXT_DAYS 11125
#define SYMBOL_ADDEDITPROPSHEET_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_ADDEDITPROPSHEET_TITLE _("Edit Entry")
#define SYMBOL_ADDEDITPROPSHEET_IDNAME ID_ADDEDITPROPSHEET
#define SYMBOL_ADDEDITPROPSHEET_SIZE wxSize(400, 300)
#define SYMBOL_ADDEDITPROPSHEET_POSITION wxDefaultPosition
////@end control identifiers
#define SYMBOL_ADDPROPSHEET_TITLE _("Add Entry")
#define SYMBOL_EDITPROPSHEET_TITLE _("Edit Entry")
#define SYMBOL_VIEWPROPSHEET_TITLE _("View Entry")
#define SYMBOL_AUTOPROPSHEET_TITLE _("Add, Edit or View Entry")

/*!
 * AddEditPropSheet class declaration
 */

class AddEditPropSheet: public wxPropertySheetDialog
{
  DECLARE_CLASS( AddEditPropSheet )
  DECLARE_EVENT_TABLE()

public:
  enum class SheetType {ADD, EDIT, VIEW}; // to tweak UI, mainly
  /// Constructor
  // item is nullptr for ADD, otherwise its values are retrieved and displayed
  AddEditPropSheet(wxWindow* parent, PWScore &core,
                   SheetType type, const CItemData *item = nullptr,
                   const wxString& selectedGroup = wxEmptyString,
                   wxWindowID id = SYMBOL_ADDEDITPROPSHEET_IDNAME,
                   const wxString& caption = SYMBOL_AUTOPROPSHEET_TITLE,
                   const wxPoint& pos = SYMBOL_ADDEDITPROPSHEET_POSITION,
                   const wxSize& size = SYMBOL_ADDEDITPROPSHEET_SIZE,
                   long style = SYMBOL_ADDEDITPROPSHEET_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ADDEDITPROPSHEET_IDNAME, const wxString& caption = SYMBOL_AUTOPROPSHEET_TITLE, const wxPoint& pos = SYMBOL_ADDEDITPROPSHEET_POSITION, const wxSize& size = SYMBOL_ADDEDITPROPSHEET_SIZE, long style = SYMBOL_ADDEDITPROPSHEET_STYLE );

  /// Destructor
  ~AddEditPropSheet();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin AddEditPropSheet event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
  void OnShowHideClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
  void OnGenerateButtonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN
  void OnGoButtonClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEND_BTN
  void OnSendButtonClick( wxCommandEvent& event );

  /// wxEVT_SET_FOCUS event handler for ID_TEXTCTRL_NOTES
  void OnNoteSetFocus( wxFocusEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_KEEP
  void OnKeepHistoryClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_ON
  void OnExpRadiobuttonSelected( wxCommandEvent& event );

  /// wxEVT_DATE_CHANGED event handler for ID_DATECTRL_EXP_DATE
  void OnExpDateChanged( wxDateEvent& event );

  /// wxEVT_COMMAND_SPINCTRL_UPDATED event handler for ID_SPINCTRL_EXP_TIME
  void OnExpIntervalChanged( wxSpinEvent& event );

  /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_CHECKBOX42
  void OnPasswordPolicySelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_POLICYLIST
  void OnPolicylistSelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
  void OnLowercaseCB( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX4
  void OnUppercaseCB( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX5
  void OnDigitsCB( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX6
  void OnSymbolsCB( wxCommandEvent& event );

  /// wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS
  void OnOwnSymSetFocus( wxFocusEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RESET_SYMBOLS
  void OnResetSymbolsClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
  void OnEZreadCBClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX8
  void OnPronouceableCBClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX9
  void OnUseHexCBClick( wxCommandEvent& event );

  /// wxEVT_UPDATE_UI event handler for all command ids
  void OnUpdateUI(wxUpdateUIEvent& event );

////@end AddEditPropSheet event handler declarations
  void OnEZreadOrPronounceable( wxCommandEvent& evt);
  void OnClearPWHist(wxCommandEvent& evt);
  void OnOk(wxCommandEvent& evt);

  /// wxEVT_SPINCTRL event handler for ID_SPINCTRL5, ID_SPINCTRL6, ID_SPINCTRL7, ID_SPINCTRL8
  void OnAtLeastPasswordChars(wxSpinEvent& evt);
////@begin AddEditPropSheet member function declarations

  wxString GetATime() const { return m_ATime ; }
  void SetATime(wxString value) { m_ATime = value ; }

  wxString GetCTime() const { return m_CTime ; }
  void SetCTime(wxString value) { m_CTime = value ; }

  wxString GetCurXTime() const { return m_CurXTime ; }
  void SetCurXTime(wxString value) { m_CurXTime = value ; }

  wxString GetPMTime() const { return m_PMTime ; }
  void SetPMTime(wxString value) { m_PMTime = value ; }

  wxString GetRMTime() const { return m_RMTime ; }
  void SetRMTime(wxString value) { m_RMTime = value ; }

  bool GetRecurring() const { return m_Recurring ; }
  void SetRecurring(bool value) { m_Recurring = value ; }

  wxString GetXTime() const { return m_XTime ; }
  void SetXTime(wxString value) { m_XTime = value ; }

  int GetExpInterval() const { return m_XTimeInt ; }
  void SetExpInterval(int value) { m_XTimeInt = value ; }

  wxString GetAutotype() const { return m_autotype ; }
  void SetAutotype(wxString value) { m_autotype = value ; }

  wxString GetEmail() const { return m_email ; }
  void SetEmail(wxString value) { m_email = value ; }

  bool GetIsNotesHidden() const { return m_isNotesHidden ; }
  void SetIsNotesHidden(bool value) { m_isNotesHidden = value ; }

  bool GetKeepPWHist() const { return m_keepPWHist ; }
  void SetKeepPWHist(bool value) { m_keepPWHist = value ; }

  int GetMaxPWHist() const { return m_maxPWHist ; }
  void SetMaxPWHist(int value) { m_maxPWHist = value ; }

  wxString GetNotes() const { return m_notes ; }
  void SetNotes(wxString value) { m_notes = value ; }

  wxString GetRuncmd() const { return m_runcmd ; }
  void SetRuncmd(wxString value) { m_runcmd = value ; }

  wxString GetSymbols() const { return m_symbols ; }
  void SetSymbols(wxString value) { m_symbols = value ; }

  wxString GetTitle() const { return m_title ; }
  void SetTitle(wxString value) { m_title = value ; }

  wxString GetUrl() const { return m_url ; }
  void SetUrl(wxString value) { m_url = value ; }

  wxString GetUser() const { return m_user ; }
  void SetUser(wxString value) { m_user = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end AddEditPropSheet member function declarations
  const CItemData &GetItem() const {return m_item;} // for ADD mode
  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin AddEditPropSheet member variables
  wxPanel* m_BasicPanel;
  wxPanel* m_AdditionalPanel;
  wxPanel* m_PasswordPolicyPanel;
  wxGridBagSizer* m_BasicGBSizer;
  wxComboBox* m_groupCtrl;
  wxTextCtrl* m_UsernameCtrl;
  wxTextCtrl* m_PasswordCtrl;
  wxButton* m_ShowHideCtrl;
  wxTextCtrl* m_Password2Ctrl;
  wxTextCtrl* m_noteTX;
  wxComboBox* m_DCAcomboBox;
  wxComboBox* m_SDCAcomboBox;
  wxSpinCtrl* m_MaxPWHistCtrl;
  wxGrid* m_PWHgrid;
  wxRadioButton* m_OnRB;
  wxDatePickerCtrl* m_ExpDate;
  wxRadioButton* m_InRB;
  wxSpinCtrl* m_ExpTimeCtrl;
  wxCheckBox* m_RecurringCtrl;
  wxRadioButton* m_NeverRB;
  wxCheckBox* m_UseDatabasePolicyCtrl;
  wxComboBox* m_cbxPolicyNames;
  wxSpinCtrl* m_pwpLenCtrl;
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
  wxTextCtrl* m_ownsymbols;
  wxCheckBox* m_pwpEasyCtrl;
  wxCheckBox* m_pwpPronounceCtrl;
  wxCheckBox* m_pwpHexCtrl;
  wxString m_RMTime; // Any field modification time
private:
  wxString m_ATime; // Access Time
  wxString m_CTime; // Creation time
  wxString m_CurXTime; // Current Exp. time
  wxString m_PMTime; // Password Modification time
  bool m_Recurring;
  wxString m_XTime; // Password eXpiration time
  int m_XTimeInt; // Password Exp. Interval (days)
  wxString m_autotype;
  wxString m_email;
  bool m_isNotesHidden;
  bool m_keepPWHist;
  int m_maxPWHist; // How many passwords to keep
  wxString m_notes;
  wxString m_runcmd;
  wxString m_symbols;
  wxString m_title;
  wxString m_url;
  wxString m_user;
  ////@end AddEditPropSheet member variables
  short m_DCA;
  short m_ShiftDCA;
  time_t m_tttXTime; // Password Exp.date in time_t
  wxString m_PWHistory; // string as stored in CItemData
  StringX m_password;
  bool m_isPWHidden;
  PWScore &m_core;
  wxString m_selectedGroup;  //Group title in tree view user right-clicked on to add an item

  SheetType m_type;
  CItemData m_item;
  void ItemFieldsToPropSheet();
  void SetupDCAComboBoxes(wxComboBox *pcbox, short &iDCA, bool isShift);
  void UpdateExpTimes(); // entry -> controls
  void SetXTime(wxObject *src); // sync controls + controls -> entry
  void UpdatePWPolicyControls(const PWPolicy& pwp);
  void EnablePWPolicyControls(bool enable);
  PWPolicy GetPWPolicyFromUI();
  PWPolicy GetSelectedPWPolicy();
  void ShowPWPSpinners(bool show);
  void EnableNonHexCBs(bool enable);
  void ShowPassword();
  void HidePassword();
  int GetRequiredPWLength() const;
};

#endif
  // _ADDEDITPROPSHEET_H_
