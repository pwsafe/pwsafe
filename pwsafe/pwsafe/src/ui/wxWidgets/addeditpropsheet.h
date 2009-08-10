/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "wx/propdlg.h"
#include "wx/valgen.h"
#include "wx/spinctrl.h"
#include "wx/grid.h"
#include "wx/datectrl.h"
#include "wx/dateevt.h"
#include "wx/statline.h"
////@end includes
#include "corelib/ItemData.h"
#include "corelib/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSpinCtrl;
class wxGrid;
class wxDatePickerCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ADDEDITPROPSHEET 10083
#define ID_PANEL_BASIC 10084
#define ID_COMBOBOX1 10095
#define ID_TEXTCTRL5 10096
#define ID_TEXTCTRL1 10088
#define ID_TEXTCTRL2 10089
#define ID_BUTTON2 10090
#define ID_BUTTON3 10097
#define ID_TEXTCTRL3 10091
#define ID_TEXTCTRL4 10092
#define ID_GO_BTN 10093
#define ID_TEXTCTRL7 10098
#define ID_PANEL_ADDITIONAL 10085
#define ID_TEXTCTRL6 10094
#define ID_TEXTCTRL8 10099
#define ID_CHECKBOX 10100
#define ID_COMBOBOX 10101
#define ID_CHECKBOX1 10102
#define ID_SPINCTRL 10103
#define ID_GRID 10104
#define ID_BUTTON1 10105
#define ID_BUTTON4 10106
#define ID_PANEL_DTIME 10086
#define ID_RADIOBUTTON 10107
#define ID_DATECTRL 10108
#define ID_SPINCTRL1 10109
#define ID_RADIOBUTTON1 10110
#define ID_SPINCTRL2 10111
#define ID_CHECKBOX2 10112
#define ID_BUTTON5 10113
#define ID_BUTTON6 10114
#define ID_PANEL_PPOLICY 10087
#define ID_RADIOBUTTON2 10115
#define ID_RADIOBUTTON3 10116
#define ID_SPINCTRL3 10117
#define ID_CHECKBOX3 10118
#define ID_CHECKBOX4 10119
#define ID_CHECKBOX5 10120
#define ID_CHECKBOX6 10121
#define ID_CHECKBOX7 10122
#define ID_CHECKBOX8 10123
#define ID_CHECKBOX9 10124
#define ID_BUTTON7 10125
#define SYMBOL_ADDEDITPROPSHEET_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_ADDEDITPROPSHEET_TITLE _("Edit Entry")
#define SYMBOL_ADDEDITPROPSHEET_IDNAME ID_ADDEDITPROPSHEET
#define SYMBOL_ADDEDITPROPSHEET_SIZE wxSize(400, 300)
#define SYMBOL_ADDEDITPROPSHEET_POSITION wxDefaultPosition
////@end control identifiers


class PWSGrid;
class PWSTreeCtrl;


/*!
 * AddEditPropSheet class declaration
 */

class AddEditPropSheet: public wxPropertySheetDialog
{    
  DECLARE_CLASS( AddEditPropSheet )
  DECLARE_EVENT_TABLE()

public:
  enum AddOrEdit {ADD, EDIT, VIEW}; // to tweak UI, mainly
  /// Constructor
  // item is NULL for ADD, otherwise its values are retrieved and displayed
  AddEditPropSheet(wxWindow* parent, PWScore &core,
                   PWSGrid *grid, PWSTreeCtrl *tree,
                   AddOrEdit type, const CItemData *item = NULL,
                   wxWindowID id = SYMBOL_ADDEDITPROPSHEET_IDNAME,
                   const wxString& caption = SYMBOL_ADDEDITPROPSHEET_TITLE,
                   const wxPoint& pos = SYMBOL_ADDEDITPROPSHEET_POSITION,
                   const wxSize& size = SYMBOL_ADDEDITPROPSHEET_SIZE,
                   long style = SYMBOL_ADDEDITPROPSHEET_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ADDEDITPROPSHEET_IDNAME, const wxString& caption = SYMBOL_ADDEDITPROPSHEET_TITLE, const wxPoint& pos = SYMBOL_ADDEDITPROPSHEET_POSITION, const wxSize& size = SYMBOL_ADDEDITPROPSHEET_SIZE, long style = SYMBOL_ADDEDITPROPSHEET_STYLE );

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

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX
  void OnOverrideDCAClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1
  void OnKeepHistoryClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
  void OnRadiobuttonSelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
  void OnSetXTime( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
  void OnClearXTime( wxCommandEvent& event );

////@end AddEditPropSheet event handler declarations
  void OnOk(wxCommandEvent& event);
////@begin AddEditPropSheet member function declarations

  wxString GetTitle() const { return m_title ; }
  void SetTitle(wxString value) { m_title = value ; }

  wxString GetUser() const { return m_user ; }
  void SetUser(wxString value) { m_user = value ; }

  wxString GetUrl() const { return m_url ; }
  void SetUrl(wxString value) { m_url = value ; }

  wxString GetNotes() const { return m_notes ; }
  void SetNotes(wxString value) { m_notes = value ; }

  wxString GetAutotype() const { return m_autotype ; }
  void SetAutotype(wxString value) { m_autotype = value ; }

  wxString GetRuncmd() const { return m_runcmd ; }
  void SetRuncmd(wxString value) { m_runcmd = value ; }

  bool GetUseDefaultDCA() const { return m_useDefaultDCA ; }
  void SetUseDefaultDCA(bool value) { m_useDefaultDCA = value ; }

  int GetMaxPWHist() const { return m_maxPWHist ; }
  void SetMaxPWHist(int value) { m_maxPWHist = value ; }

  bool GetKeepPWHist() const { return m_keepPWHist ; }
  void SetKeepPWHist(bool value) { m_keepPWHist = value ; }

  wxString GetCTime() const { return m_CTime ; }
  void SetCTime(wxString value) { m_CTime = value ; }

  wxString GetPMTime() const { return m_PMTime ; }
  void SetPMTime(wxString value) { m_PMTime = value ; }

  wxString GetATime() const { return m_ATime ; }
  void SetATime(wxString value) { m_ATime = value ; }

  wxString GetRMTime() const { return m_RMTime ; }
  void SetRMTime(wxString value) { m_RMTime = value ; }

  wxString GetXTime() const { return m_XTime ; }
  void SetXTime(wxString value) { m_XTime = value ; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end AddEditPropSheet member function declarations
  const CItemData &GetItem() const {return m_item;} // for ADD mode
  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin AddEditPropSheet member variables
  wxComboBox* m_groupCtrl;
  wxTextCtrl* m_PasswordCtrl;
  wxButton* m_ShowHideCtrl;
  wxTextCtrl* m_Password2Ctrl;
  wxComboBox* m_DCAcomboBox;
  wxSpinCtrl* m_MaxPWHistCtrl;
  wxGrid* m_PWHgrid;
  wxRadioButton* m_OnRB;
  wxDatePickerCtrl* m_ExpDate;
  wxSpinCtrl* m_ExpTime;
  wxRadioButton* m_InRB;
  wxSpinCtrl* m_ExpInterval;
  wxCheckBox* m_Recurring;
  wxString m_RMTime; // Any field modification time
private:
  wxString m_title;
  wxString m_user;
  wxString m_url;
  wxString m_notes;
  wxString m_autotype;
  wxString m_runcmd;
  bool m_useDefaultDCA; // Use Default Double Click Action?
  int m_maxPWHist; // How many passwords to keep
  bool m_keepPWHist;
  wxString m_CTime; // Creation time
  wxString m_PMTime; // Password Modification time
  wxString m_ATime; // Access Time
  wxString m_XTime; // Password eXpiration time
  ////@end AddEditPropSheet member variables
  short m_DCA;
  wxString m_PWHistory; // string as stored in CItemData
  StringX m_password;
  bool m_isPWHidden;

  PWScore &m_core;
  PWSGrid* m_grid; // to update display
  PWSTreeCtrl* m_tree; // to update display

  AddOrEdit m_type;
  CItemData m_item;
  void ItemFieldsToPropSheet();
  void ShowPassword();
  void HidePassword();
};

#endif
  // _ADDEDITPROPSHEET_H_
