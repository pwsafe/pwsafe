/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file addeditpropsheet.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/bookctrl.h"
////@end includes
#include <wx/datetime.h>

#include <vector>
#include "core/PWSprefs.h"
#include "core/PWCharPool.h"
#include "core/PWHistory.h"
#include "core/UIinterface.h"
#include "os/run.h"

#include "addeditpropsheet.h"
#include "pwsclip.h"
#include "./wxutils.h"

#include <algorithm>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * AddEditPropSheet type definition
 */

IMPLEMENT_CLASS( AddEditPropSheet, wxPropertySheetDialog )

/*!
 * AddEditPropSheet event table definition
 */

BEGIN_EVENT_TABLE( AddEditPropSheet, wxPropertySheetDialog )

  EVT_BUTTON( wxID_OK, AddEditPropSheet::OnOk )
////@begin AddEditPropSheet event table entries
  EVT_BUTTON( ID_BUTTON2, AddEditPropSheet::OnShowHideClick )
  EVT_BUTTON( ID_BUTTON3, AddEditPropSheet::OnGenerateButtonClick )
  EVT_BUTTON( ID_GO_BTN, AddEditPropSheet::OnGoButtonClick )
  EVT_BUTTON( ID_SEND_BTN, AddEditPropSheet::OnSendButtonClick )
  EVT_CHECKBOX( ID_CHECKBOX1, AddEditPropSheet::OnKeepHistoryClick )
  EVT_RADIOBUTTON( ID_RADIOBUTTON, AddEditPropSheet::OnExpRadiobuttonSelected )
  EVT_DATE_CHANGED( ID_DATECTRL, AddEditPropSheet::OnExpDateChanged )
  EVT_RADIOBUTTON( ID_RADIOBUTTON1, AddEditPropSheet::OnExpRadiobuttonSelected )
  EVT_SPINCTRL( ID_SPINCTRL2, AddEditPropSheet::OnExpIntervalChanged )
  EVT_RADIOBUTTON( ID_RADIOBUTTON4, AddEditPropSheet::OnExpRadiobuttonSelected )
  EVT_RADIOBUTTON( ID_RADIOBUTTON2, AddEditPropSheet::OnPWPRBSelected )
  EVT_COMBOBOX( ID_POLICYLIST, AddEditPropSheet::OnPolicylistSelected )
  EVT_RADIOBUTTON( ID_RADIOBUTTON3, AddEditPropSheet::OnPWPRBSelected )
  EVT_CHECKBOX( ID_CHECKBOX3, AddEditPropSheet::OnLowercaseCB )
  EVT_CHECKBOX( ID_CHECKBOX4, AddEditPropSheet::OnUppercaseCB )
  EVT_CHECKBOX( ID_CHECKBOX5, AddEditPropSheet::OnDigitsCB )
  EVT_CHECKBOX( ID_CHECKBOX6, AddEditPropSheet::OnSymbolsCB )
  EVT_BUTTON( ID_RESET_SYMBOLS, AddEditPropSheet::OnResetSymbolsClick )
  EVT_CHECKBOX( ID_CHECKBOX7, AddEditPropSheet::OnEZreadCBClick )
  EVT_CHECKBOX( ID_CHECKBOX8, AddEditPropSheet::OnPronouceableCBClick )
  EVT_CHECKBOX( ID_CHECKBOX9, AddEditPropSheet::OnUseHexCBClick )
////@end AddEditPropSheet event table entries
  EVT_SPINCTRL(ID_SPINCTRL5, AddEditPropSheet::OnAtLeastChars)
  EVT_SPINCTRL(ID_SPINCTRL6, AddEditPropSheet::OnAtLeastChars)
  EVT_SPINCTRL(ID_SPINCTRL7, AddEditPropSheet::OnAtLeastChars)
  EVT_SPINCTRL(ID_SPINCTRL8, AddEditPropSheet::OnAtLeastChars)

  EVT_BUTTON( ID_BUTTON1,    AddEditPropSheet::OnClearPWHist )
END_EVENT_TABLE()

/*!
 * AddEditPropSheet constructors
 */

AddEditPropSheet::AddEditPropSheet(wxWindow* parent, PWScore &core,
                                   SheetType type, const CItemData *item, UIInterFace* ui,
                                   const wxString& selectedGroup,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size,
                                   long style)
: m_core(core), m_ui(ui), m_selectedGroup(selectedGroup), m_type(type)
{
  if (item != nullptr)
    m_item = *item; // copy existing item to display values
  else
    m_item.CreateUUID(); // We're adding a new entry
  Init();
  wxString dlgTitle;
  if (caption == SYMBOL_AUTOPROPSHEET_TITLE) {
    switch(m_type) {
      case SheetType::ADD:
        dlgTitle = SYMBOL_ADDPROPSHEET_TITLE;
        break;
      case SheetType::EDIT:
        dlgTitle = SYMBOL_EDITPROPSHEET_TITLE;
        break;
      case SheetType::VIEW:
        dlgTitle = SYMBOL_VIEWPROPSHEET_TITLE;
        break;
      default:
        dlgTitle = caption;
        break;
    }
  }
  Create(parent, id, dlgTitle, pos, size, style);
}

/*!
 * AddEditPropSheet creator
 */

bool AddEditPropSheet::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin AddEditPropSheet creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  int flags = (m_type == SheetType::VIEW) ? (wxCLOSE|wxHELP) : (wxOK|wxCANCEL|wxHELP);
  CreateButtons(flags);
  CreateControls();
  Centre();
////@end AddEditPropSheet creation
  ItemFieldsToPropSheet();
  LayoutDialog();
  return true;
}

/*!
 * AddEditPropSheet destructor
 */

AddEditPropSheet::~AddEditPropSheet()
{
////@begin AddEditPropSheet destruction
////@end AddEditPropSheet destruction
}

/*!
 * Member initialisation
 */

void AddEditPropSheet::Init()
{
////@begin AddEditPropSheet member initialisation
  m_XTimeInt = 0;
  m_isNotesHidden = !PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault);
  m_BasicPanel = nullptr;
  m_BasicFGSizer = nullptr;
  m_groupCtrl = nullptr;
  m_UsernameCtrl = nullptr;
  m_PasswordCtrl = nullptr;
  m_ShowHideCtrl = nullptr;
  m_Password2Ctrl = nullptr;
  m_noteTX = nullptr;
  m_DCAcomboBox = nullptr;
  m_SDCAcomboBox = nullptr;
  m_MaxPWHistCtrl = nullptr;
  m_PWHgrid = nullptr;
  m_OnRB = nullptr;
  m_ExpDate = nullptr;
  m_InRB = nullptr;
  m_ExpTimeCtrl = nullptr;
  m_RecurringCtrl = nullptr;
  m_NeverRB = nullptr;
  m_defPWPRB = nullptr;
  m_cbxPolicyNames = nullptr;
  m_ourPWPRB = nullptr;
  m_pwpLenCtrl = nullptr;
  m_pwMinsGSzr = nullptr;
  m_pwpUseLowerCtrl = nullptr;
  m_pwNumLCbox = nullptr;
  m_pwpLCSpin = nullptr;
  m_pwpUseUpperCtrl = nullptr;
  m_pwNumUCbox = nullptr;
  m_pwpUCSpin = nullptr;
  m_pwpUseDigitsCtrl = nullptr;
  m_pwNumDigbox = nullptr;
  m_pwpDigSpin = nullptr;
  m_pwpSymCtrl = nullptr;
  m_pwNumSymbox = nullptr;
  m_pwpSymSpin = nullptr;
  m_ownsymbols = nullptr;
  m_pwpEasyCtrl = nullptr;
  m_pwpPronounceCtrl = nullptr;
  m_pwpHexCtrl = nullptr;
////@end AddEditPropSheet member initialisation
}

class PolicyValidator : public MultiCheckboxValidator
{
public:
  PolicyValidator(int rbID, int ids[], size_t num,
      const wxString& msg, const wxString& title)
    : MultiCheckboxValidator(ids, num, msg, title), m_rbID(rbID) {}
  PolicyValidator(const PolicyValidator &other)
    : MultiCheckboxValidator(other), m_rbID(other.m_rbID) {}
  ~PolicyValidator() {}

  wxObject* Clone() const {return new PolicyValidator(m_rbID, m_ids, m_count, m_msg, m_title);}
  bool Validate(wxWindow* parent) {
    wxWindow* win = GetWindow()->FindWindow(m_rbID);
    if (win && win->IsEnabled()) {
      wxRadioButton* rb = wxDynamicCast(win, wxRadioButton);
      if (rb && rb->GetValue()) {
  return true;
      }
    }
    return MultiCheckboxValidator::Validate(parent);
  }
private:
  int m_rbID;
};

static void setupDCAStrings(wxArrayString &as)
{
  // semi-duplicated in SetupDCAComboBoxes(),
  // but leaving these empty now causes an assert
  as.Add(_("Auto Type"));
  as.Add(_("Browse"));
  as.Add(_("Browse + Auto Type"));
  as.Add(_("Copy Notes"));
  as.Add(_("Copy Password"));
  as.Add(_("Copy Password + Minimize"));
  as.Add(_("Copy Username"));
  as.Add(_("Edit/View Entry"));
  as.Add(_("Execute Run command"));
}

/*!
 * Control creation for AddEditPropSheet
 */

void AddEditPropSheet::CreateControls()
{
////@begin AddEditPropSheet content construction

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Basic"
  /////////////////////////////////////////////////////////////////////////////
  
  m_BasicPanel = new wxPanel( GetBookCtrl(), ID_PANEL_BASIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  m_BasicPanel->SetSizer(itemBoxSizer3);

  wxStaticText* itemStaticText4 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("To add a new entry, simply fill in the fields below. At least a title and a\npassword are required. If you have set a default username, it will appear in the\nusername field."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_BasicFGSizer = new wxFlexGridSizer(0, 3, 0, 0);
  m_BasicFGSizer->AddGrowableCol(1);

  itemBoxSizer3->Add(m_BasicFGSizer, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  wxStaticText* itemStaticText6 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("Group:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_groupCtrlStrings;
  m_groupCtrl = new wxComboBox( m_BasicPanel, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_groupCtrlStrings, wxCB_DROPDOWN );
  m_BasicFGSizer->Add(m_groupCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_BasicFGSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  wxStaticText* itemStaticText9 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemTextCtrl10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_BasicFGSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  wxStaticText* itemStaticText12 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_UsernameCtrl = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(m_UsernameCtrl , 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_BasicFGSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  wxStaticText* itemStaticText15 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_PasswordCtrl = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(m_PasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  m_BasicFGSizer->Add(itemBoxSizer17, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_ShowHideCtrl = new wxButton( m_BasicPanel, ID_BUTTON2, _("&Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(m_ShowHideCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 20);

  wxButton* itemButton21 = new wxButton( m_BasicPanel, ID_BUTTON3, _("&Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 10);

  wxStaticText* itemStaticText22 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("Confirm:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Password2Ctrl = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
  m_BasicFGSizer->Add(m_Password2Ctrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_BasicFGSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  wxStaticText* itemStaticText25 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl26 = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemTextCtrl26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  m_BasicFGSizer->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  
  wxButton* itemButton29 = new wxButton( m_BasicPanel, ID_GO_BTN, _("Go"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemButton29, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 20);

  wxStaticText* itemStaticText30 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("email:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemStaticText30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl31 = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL20, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicFGSizer->Add(itemTextCtrl31, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer32 = new wxBoxSizer(wxHORIZONTAL);
  m_BasicFGSizer->Add(itemBoxSizer32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  
  wxButton* itemButton34 = new wxButton( m_BasicPanel, ID_SEND_BTN, _("Send"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer32->Add(itemButton34, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 20);

  auto *itemBoxSizer35 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer35, 1, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText36 = new wxStaticText( m_BasicPanel, wxID_STATIC, _("Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer35->Add(itemStaticText36, 0, wxALIGN_TOP|wxALL, 5);
  itemBoxSizer35->Add(27, 13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_noteTX = new wxTextCtrl( m_BasicPanel, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxSize(-1, m_BasicPanel->ConvertDialogToPixels(wxSize(-1, 50)).y), wxTE_MULTILINE );
  itemBoxSizer35->Add(m_noteTX, 5, wxGROW|wxALL, 3);
  
  GetBookCtrl()->AddPage(m_BasicPanel, _("Basic"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Additional"
  /////////////////////////////////////////////////////////////////////////////
  
  wxPanel* itemPanel38 = new wxPanel( GetBookCtrl(), ID_PANEL_ADDITIONAL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer39 = new wxBoxSizer(wxVERTICAL);
  itemPanel38->SetSizer(itemBoxSizer39);

  auto *itemFlexGridSizer40 = new wxFlexGridSizer(0, 2, 0, 0);
  itemBoxSizer39->Add(itemFlexGridSizer40, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText41 = new wxStaticText( itemPanel38, wxID_STATIC, _("Autotype:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer40->Add(itemStaticText41, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl42 = new wxTextCtrl( itemPanel38, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer40->Add(itemTextCtrl42, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText43 = new wxStaticText( itemPanel38, wxID_STATIC, _("Run Cmd:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer40->Add(itemStaticText43, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl44 = new wxTextCtrl( itemPanel38, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer40->Add(itemTextCtrl44, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText45 = new wxStaticText( itemPanel38, wxID_STATIC, _("Double-Click\nAction:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer40->Add(itemStaticText45, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_DCAcomboBoxStrings;
  setupDCAStrings(m_DCAcomboBoxStrings);
  m_DCAcomboBox = new wxComboBox( itemPanel38, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_DCAcomboBoxStrings, wxCB_READONLY );
  itemFlexGridSizer40->Add(m_DCAcomboBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText47 = new wxStaticText( itemPanel38, wxID_STATIC, _("Shift-Double-Click\nAction:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer40->Add(itemStaticText47, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_SDCAcomboBoxStrings;
  setupDCAStrings(m_SDCAcomboBoxStrings);
  m_SDCAcomboBox = new wxComboBox( itemPanel38, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_SDCAcomboBoxStrings, wxCB_READONLY );
  itemFlexGridSizer40->Add(m_SDCAcomboBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer49Static = new wxStaticBox(itemPanel38, wxID_ANY, _("Password History"));
  auto *itemStaticBoxSizer49 = new wxStaticBoxSizer(itemStaticBoxSizer49Static, wxVERTICAL);
  itemBoxSizer39->Add(itemStaticBoxSizer49, 0, wxGROW|wxALL, 5);
  auto *itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer50, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox51 = new wxCheckBox( itemPanel38, ID_CHECKBOX1, _("Keep"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox51->SetValue(false);
  itemBoxSizer50->Add(itemCheckBox51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_MaxPWHistCtrl = new wxSpinCtrl( itemPanel38, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer50->Add(m_MaxPWHistCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText53 = new wxStaticText( itemPanel38, wxID_STATIC, _("last passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(itemStaticText53, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_PWHgrid = new wxGrid( itemPanel38, ID_GRID, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PWHgrid->SetDefaultColSize(150);
  m_PWHgrid->SetDefaultRowSize(25);
  m_PWHgrid->SetColLabelSize(25);
  m_PWHgrid->SetRowLabelSize(0);
  m_PWHgrid->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemStaticBoxSizer49->Add(m_PWHgrid, 0, wxGROW|wxALL, 5);

  auto *itemBoxSizer55 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer55, 0, wxGROW|wxALL, 5);
  wxButton* itemButton56 = new wxButton( itemPanel38, ID_BUTTON1, _("Clear History"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer55->Add(itemButton56, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer55->AddStretchSpacer();

  wxButton* itemButton58 = new wxButton( itemPanel38, ID_BUTTON4, _("Copy All"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer55->Add(itemButton58, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel38, _("Additional"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Dates and Times"
  /////////////////////////////////////////////////////////////////////////////
  
  wxPanel* itemPanel59 = new wxPanel( GetBookCtrl(), ID_PANEL_DTIME, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer60 = new wxBoxSizer(wxVERTICAL);
  itemPanel59->SetSizer(itemBoxSizer60);

  wxStaticBox* itemStaticBoxSizer61Static = new wxStaticBox(itemPanel59, wxID_ANY, _("Password Expiry"));
  auto *itemStaticBoxSizer61 = new wxStaticBoxSizer(itemStaticBoxSizer61Static, wxVERTICAL);
  itemBoxSizer60->Add(itemStaticBoxSizer61, 0, wxGROW|wxALL, 5);
  auto *itemBoxSizer62 = new wxBoxSizer(wxVERTICAL);
  itemStaticBoxSizer61->Add(itemBoxSizer62, 0, wxGROW|wxALL, 0);
  auto *itemFlexGridSizer63 = new wxFlexGridSizer(0, 3, 0, 0);
  itemBoxSizer62->Add(itemFlexGridSizer63, 0, wxGROW|wxALL, 5);
  m_OnRB = new wxRadioButton( itemPanel59, ID_RADIOBUTTON, _("On"), wxDefaultPosition, wxDefaultSize, 0 );
  m_OnRB->SetValue(false);
  itemFlexGridSizer63->Add(m_OnRB, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_ExpDate = new wxDatePickerCtrl( itemPanel59, ID_DATECTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
  itemFlexGridSizer63->Add(m_ExpDate, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer63->AddStretchSpacer();

  m_InRB = new wxRadioButton( itemPanel59, ID_RADIOBUTTON1, _("In"), wxDefaultPosition, wxDefaultSize, 0 );
  m_InRB->SetValue(false);
  itemFlexGridSizer63->Add(m_InRB, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer68 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer63->Add(itemBoxSizer68, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  m_ExpTimeCtrl = new wxSpinCtrl( itemPanel59, ID_SPINCTRL2, _T("90"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3650, 90 );
  itemBoxSizer68->Add(m_ExpTimeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText70 = new wxStaticText( itemPanel59, wxID_STATIC, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer68->Add(itemStaticText70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_RecurringCtrl = new wxCheckBox( itemPanel59, ID_CHECKBOX2, _("Recurring"), wxDefaultPosition, wxDefaultSize, 0 );
  m_RecurringCtrl->SetValue(false);
  itemFlexGridSizer63->Add(m_RecurringCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer72 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer72, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);
  m_NeverRB = new wxRadioButton( itemPanel59, ID_RADIOBUTTON4, _("Never"), wxDefaultPosition, wxDefaultSize, 0 );
  m_NeverRB->SetValue(false);
  itemBoxSizer72->Add(m_NeverRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer74 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer74, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText75 = new wxStaticText( itemPanel59, wxID_STATIC, _("Original Value:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer74->Add(itemStaticText75, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText76 = new wxStaticText( itemPanel59, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer74->Add(itemStaticText76, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer77Static = new wxStaticBox(itemPanel59, wxID_ANY, _("Statistics"));
  auto *itemStaticBoxSizer77 = new wxStaticBoxSizer(itemStaticBoxSizer77Static, wxVERTICAL);
  itemBoxSizer60->Add(itemStaticBoxSizer77, 0, wxGROW|wxALL, 5);
  auto *itemFlexGridSizer78 = new wxFlexGridSizer(0, 2, 0, 0);
  itemStaticBoxSizer77->Add(itemFlexGridSizer78, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText79 = new wxStaticText( itemPanel59, wxID_STATIC, _("Created on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText79, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText80 = new wxStaticText( itemPanel59, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText80, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText81 = new wxStaticText( itemPanel59, wxID_STATIC, _("Password last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText81, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText82 = new wxStaticText( itemPanel59, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText82, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText83 = new wxStaticText( itemPanel59, wxID_STATIC, _("Last accessed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText83, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText84 = new wxStaticText( itemPanel59, wxID_STATIC, _("N/A"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText84, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText85 = new wxStaticText( itemPanel59, wxID_STATIC, _("Any field last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText85, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText86 = new wxStaticText( itemPanel59, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText86, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel59, _("Dates and Times"));
  
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password Policy"
  /////////////////////////////////////////////////////////////////////////////
  
  wxPanel* itemPanel87 = new wxPanel( GetBookCtrl(), ID_PANEL_PPOLICY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL );
  auto *itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
  itemPanel87->SetSizer(itemBoxSizer61);
  
  wxStaticBox* itemStaticBoxSizer88Static = new wxStaticBox(itemPanel87, wxID_ANY, _("Random password generation rules"));
  auto *itemStaticBoxSizer88 = new wxStaticBoxSizer(itemStaticBoxSizer88Static, wxVERTICAL);
  itemBoxSizer61->Add(itemStaticBoxSizer88, 0, wxGROW|wxALL, 5);

  auto *itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer88->Add(itemBoxSizer89, 0, wxGROW|wxALL, 0);
  m_defPWPRB = new wxRadioButton( itemPanel87, ID_RADIOBUTTON2, _("Use Database Policy"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  m_defPWPRB->SetValue(false);
  itemBoxSizer89->Add(m_defPWPRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10);

  itemBoxSizer89->AddStretchSpacer();

  wxArrayString m_cbxPolicyNamesStrings;
  m_cbxPolicyNames = new wxComboBox( itemPanel87, ID_POLICYLIST, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_cbxPolicyNamesStrings, wxCB_READONLY );
  itemBoxSizer89->Add(m_cbxPolicyNames, 0, wxALIGN_TOP|wxALL, 10);

  m_ourPWPRB = new wxRadioButton( itemPanel87, ID_RADIOBUTTON3, _("Use the policy below:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ourPWPRB->SetValue(false);
  itemStaticBoxSizer88->Add(m_ourPWPRB, 0, wxALIGN_LEFT|wxALL, 10);

  wxStaticLine* itemStaticLine94 = new wxStaticLine( itemPanel87, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer88->Add(itemStaticLine94, 0, wxGROW|wxALL, 5);

  auto *itemBoxSizer95 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer88->Add(itemBoxSizer95, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText96 = new wxStaticText( itemPanel87, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer95->Add(itemStaticText96, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5);

  m_pwpLenCtrl = new wxSpinCtrl( itemPanel87, ID_SPINCTRL3, _T("12"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 1024, 12 );
  itemBoxSizer95->Add(m_pwpLenCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10);

  m_pwMinsGSzr = new wxGridSizer(0, 2, 0, 0);
  itemStaticBoxSizer88->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);
  m_pwpUseLowerCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText101 = new wxStaticText( itemPanel87, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText101, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemPanel87, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(itemPanel87->ConvertDialogToPixels(wxSize(40, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText103 = new wxStaticText( itemPanel87, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText103, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText106 = new wxStaticText( itemPanel87, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText106, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemPanel87, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(itemPanel87->ConvertDialogToPixels(wxSize(40, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText108 = new wxStaticText( itemPanel87, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText108, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText111 = new wxStaticText( itemPanel87, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText111, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemPanel87, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(itemPanel87->ConvertDialogToPixels(wxSize(40, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText113 = new wxStaticText( itemPanel87, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText113, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText116 = new wxStaticText( itemPanel87, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText116, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemPanel87, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(itemPanel87->ConvertDialogToPixels(wxSize(40, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText118 = new wxStaticText( itemPanel87, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText118, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_ownsymbols = new wxTextCtrl( itemPanel87, IDC_OWNSYMBOLS, wxEmptyString, wxDefaultPosition, wxSize(itemPanel87->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
  m_pwMinsGSzr->Add(m_ownsymbols, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton120 = new wxButton( itemPanel87, ID_RESET_SYMBOLS, _("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwMinsGSzr->Add(itemButton120, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpEasyCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr->AddStretchSpacer();

  m_pwpPronounceCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr->AddStretchSpacer();

  wxStaticText* itemStaticText125 = new wxStaticText( itemPanel87, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer88->Add(itemStaticText125, 0, wxALIGN_LEFT|wxALL, 10);

  m_pwpHexCtrl = new wxCheckBox( itemPanel87, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer88->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 10);

  GetBookCtrl()->AddPage(itemPanel87, _("Password Policy"));

  /////////////////////////////////////////////////////////////////////////////
  // End of Tab Creation
  /////////////////////////////////////////////////////////////////////////////
  
  // Set validators
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_title) );
  m_UsernameCtrl->SetValidator( wxGenericValidator(& m_user) );
  itemTextCtrl26->SetValidator( wxGenericValidator(& m_url) );
  itemTextCtrl31->SetValidator( wxGenericValidator(& m_email) );
  m_noteTX->SetValidator( wxGenericValidator(& m_notes) );
  itemTextCtrl42->SetValidator( wxGenericValidator(& m_autotype) );
  itemTextCtrl44->SetValidator( wxGenericValidator(& m_runcmd) );
  itemCheckBox51->SetValidator( wxGenericValidator(& m_keepPWHist) );
  m_MaxPWHistCtrl->SetValidator( wxGenericValidator(& m_maxPWHist) );
  m_ExpTimeCtrl->SetValidator( wxGenericValidator(& m_XTimeInt) );
  m_RecurringCtrl->SetValidator( wxGenericValidator(& m_Recurring) );
  itemStaticText76->SetValidator( wxGenericValidator(& m_CurXTime) );
  itemStaticText80->SetValidator( wxGenericValidator(& m_CTime) );
  itemStaticText82->SetValidator( wxGenericValidator(& m_PMTime) );
  itemStaticText84->SetValidator( wxGenericValidator(& m_ATime) );
  itemStaticText86->SetValidator( wxGenericValidator(& m_RMTime) );
  m_ownsymbols->SetValidator( wxGenericValidator(& m_symbols) );
  // Connect events and objects
  m_noteTX->Connect(ID_TEXTCTRL7, wxEVT_SET_FOCUS, wxFocusEventHandler(AddEditPropSheet::OnNoteSetFocus), nullptr, this);
  m_ownsymbols->Connect(IDC_OWNSYMBOLS, wxEVT_SET_FOCUS, wxFocusEventHandler(AddEditPropSheet::OnOwnSymSetFocus), nullptr, this);
////@end AddEditPropSheet content construction

  // Non-DialogBlock initializations:
  m_PWHgrid->SetColLabelValue(0, _("Set Date/Time"));
  m_PWHgrid->SetColLabelValue(1, _("Password"));

  // Setup symbols
  m_symbols = CPasswordCharPool::GetDefaultSymbols().c_str();
  m_ownsymbols->SetValue(m_symbols);

  m_ExpTimeCtrl->SetRange(1, 3650);
}

/*!
 * Should we show tooltips?
 */

bool AddEditPropSheet::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap AddEditPropSheet::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin AddEditPropSheet bitmap retrieval
  return wxNullBitmap;
////@end AddEditPropSheet bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AddEditPropSheet::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin AddEditPropSheet icon retrieval
  return wxNullIcon;
////@end AddEditPropSheet icon retrieval
}

static void EnableSizerChildren(wxSizer *sizer, bool enable)
{
  wxSizerItemList items = sizer->GetChildren();
  wxSizerItemList::iterator iter;
  for (iter = items.begin(); iter != items.end(); iter++) {
    wxWindow *childW = (*iter)->GetWindow();
    if (childW != nullptr)
      childW->Enable(enable);
    else { // if another sizer, recurse!
      wxSizer *childS = (*iter)->GetSizer();
      if (childS != nullptr)
        EnableSizerChildren(childS, enable);
    }
  }
}

void AddEditPropSheet::UpdatePWPolicyControls(const PWPolicy& pwp)
{
  bool bUseVal; // keep picky compiler happy, code readable

  EnableSizerChildren(m_pwMinsGSzr, !m_pwpHexCtrl->GetValue());
  m_pwpLenCtrl->SetValue(pwp.length);
  bUseVal = (pwp.flags & PWPolicy::UseLowercase) != 0;
  m_pwpUseLowerCtrl->SetValue(bUseVal);
  m_pwpLCSpin->SetValue(pwp.lowerminlength);
  bUseVal = (pwp.flags & PWPolicy::UseUppercase) != 0;
  m_pwpUseUpperCtrl->SetValue(bUseVal);
  m_pwpUCSpin->SetValue(pwp.upperminlength);
  bUseVal = (pwp.flags & PWPolicy::UseDigits) != 0;
  m_pwpUseDigitsCtrl->SetValue(bUseVal);
  m_pwpDigSpin->SetValue(pwp.digitminlength);

  bUseVal = (pwp.flags & PWPolicy::UseSymbols) != 0;
  m_pwpSymCtrl->SetValue(bUseVal);
  m_pwpSymSpin->SetValue(pwp.symbolminlength);

  bUseVal = (pwp.flags & PWPolicy::UseEasyVision) != 0;
  m_pwpEasyCtrl->SetValue(bUseVal);
  bUseVal = (pwp.flags & PWPolicy::MakePronounceable) != 0;
  m_pwpPronounceCtrl->SetValue(bUseVal);
  bUseVal = (pwp.flags & PWPolicy::UseHexDigits) != 0;
  m_pwpHexCtrl->SetValue(bUseVal);

  ShowPWPSpinners(!m_pwpPronounceCtrl->GetValue() && !m_pwpEasyCtrl->GetValue());

  if (!pwp.symbols.empty()) {
    m_symbols = pwp.symbols.c_str();
    Validate(); TransferDataToWindow();
  }
}

void AddEditPropSheet::EnablePWPolicyControls(bool enable)
{
  m_pwpLenCtrl->Enable(enable);
  EnableSizerChildren(m_pwMinsGSzr, enable && !m_pwpHexCtrl->GetValue());
  m_pwpHexCtrl->Enable(enable);
  if (enable) {
    // Be more specific for character set controls
    m_pwpLCSpin->Enable(m_pwpUseLowerCtrl->GetValue());
    m_pwpUCSpin->Enable(m_pwpUseUpperCtrl->GetValue());
    m_pwpDigSpin->Enable(m_pwpUseDigitsCtrl->GetValue());
    bool useSyms = m_pwpSymCtrl->GetValue();
    m_ownsymbols->Enable(useSyms);
    m_pwpSymSpin->Enable(useSyms);
    FindWindow(ID_RESET_SYMBOLS)->Enable(useSyms);
  }
}

struct newer {
  bool operator()(const PWHistEntry& first, const PWHistEntry& second) const {
    return first.changetttdate > second.changetttdate;
  }
};

void AddEditPropSheet::SetupDCAComboBoxes(wxComboBox *pcbox, short &iDCA, bool isShift)
{
static struct {short pv; wxString name;}
 dcaMapping[] =
   {{PWSprefs::DoubleClickAutoType, _("Auto Type")},
    {PWSprefs::DoubleClickBrowse, _("Browse")},
    {PWSprefs::DoubleClickBrowsePlus, _("Browse + Auto Type")},
    {PWSprefs::DoubleClickCopyNotes, _("Copy Notes")},
    {PWSprefs::DoubleClickCopyPassword, _("Copy Password")},
    {PWSprefs::DoubleClickCopyPasswordMinimize, _("Copy Password + Minimize")},
    {PWSprefs::DoubleClickCopyUsername, _("Copy Username")},
    {PWSprefs::DoubleClickViewEdit, _("Edit/View Entry")},
    {PWSprefs::DoubleClickRun, _("Execute Run command")},
   };

  int16 dca;
  if (isShift)
    m_item.GetShiftDCA(dca);
  else
    m_item.GetDCA(dca);

  bool useDefault = (dca < PWSprefs::minDCA || dca > PWSprefs::maxDCA);
  short defDCA =  short(PWSprefs::GetInstance()->
      GetPref(isShift ?
        PWSprefs::ShiftDoubleClickAction : PWSprefs::DoubleClickAction));
  // Following loop:
  // - sets CB's client data to pref value
  // - Adds " (default)" to default string
  // - Selects current value
  for (size_t i = 0; i < sizeof(dcaMapping)/sizeof(dcaMapping[0]); i++) {
    pcbox->SetClientData(i, reinterpret_cast<void *>(dcaMapping[i].pv));
    if (dcaMapping[i].pv == defDCA) {
      wxString dv = dcaMapping[i].name;
      dv += wxT(" ("); dv += _("default"); dv += wxT(")");
      pcbox->SetString(i, dv);
      if (useDefault || iDCA == defDCA) {
        pcbox->SetValue(dv);
      }
    }
    else if (iDCA == dcaMapping[i].pv)
      pcbox->SetValue(dcaMapping[i].name);
  }
}

void AddEditPropSheet::UpdateExpTimes()
{
  // From m_item to display

  m_item.GetXTime(m_tttXTime);
  m_item.GetXTimeInt(m_XTimeInt);
  m_XTime = m_CurXTime = m_item.GetXTimeL().c_str();

  if (m_XTime.empty())
    m_XTime = m_CurXTime = _("Never");

  wxCommandEvent dummy;
  wxDateTime exp;
  if (m_tttXTime == 0) { // never expires
    m_OnRB->SetValue(false);
    m_InRB->SetValue(false);
    m_NeverRB->SetValue(true);
    exp = wxDateTime::Now();
    dummy.SetEventObject(m_NeverRB);
  } else {
    exp = wxDateTime(m_tttXTime);
    if (m_XTimeInt == 0) { // expiration specified as date
      m_OnRB->SetValue(true);
      m_InRB->SetValue(false);
      m_NeverRB->SetValue(false);
      m_ExpTimeCtrl->Enable(false);
      m_Recurring = false;
      dummy.SetEventObject(m_OnRB);
    } else { // exp. specified as interval
      m_OnRB->SetValue(false);
      m_InRB->SetValue(true);
      m_NeverRB->SetValue(false);
      m_ExpDate->Enable(false);
      m_ExpTimeCtrl->SetValue(m_XTimeInt);
      m_Recurring = true;
      dummy.SetEventObject(m_InRB);
    }
    m_RecurringCtrl->Enable(m_Recurring);
    m_ExpDate->SetValue(exp);
  }

  if (exp > wxDateTime::Today())
    exp = wxDateTime::Today(); // otherwise we can never move exp date back
  m_ExpDate->SetRange(exp, wxDateTime(time_t(-1)));

  OnExpRadiobuttonSelected(dummy); // setup enable/disable of expiry-related controls
}

void AddEditPropSheet::ItemFieldsToPropSheet()
{
  std::vector<stringT> svec;
  std::vector<stringT>::iterator sviter;

  PWSprefs *prefs = PWSprefs::GetInstance();
  
  // Populate the group combo box
  m_core.GetAllGroups(svec);
  for (sviter = svec.begin(); sviter != svec.end(); sviter++)
    m_groupCtrl->Append(sviter->c_str());

  // select relevant group
  const StringX group = (m_type == SheetType::ADD ? tostringx(m_selectedGroup): m_item.GetGroup());
  if (!group.empty()) {
    bool foundGroup = false;
    for (size_t igrp = 0; igrp < svec.size(); igrp++) {
      if (group == svec[igrp].c_str()) {
        m_groupCtrl->SetSelection((int)igrp);
        foundGroup =true;
        break;
      }
    }
    if (!foundGroup)
      m_groupCtrl->SetValue(m_selectedGroup);
  }
  m_title = m_item.GetTitle().c_str();
  m_user = m_item.GetUser().c_str();
  m_url = m_item.GetURL().c_str();
  m_email = m_item.GetEmail().c_str();
  m_password = m_item.GetPassword();

  if (m_item.IsAlias()) {
    // Update password to alias form
    // Show text stating that it is an alias

    const CItemData *pbci = m_core.GetBaseEntry(&m_item);
    ASSERT(pbci);
    if (pbci) {
      m_password = L"[" +
                pbci->GetGroup() + L":" +
                pbci->GetTitle() + L":" +
                pbci->GetUser()  + L"]";
    }
  } // IsAlias

  if (prefs->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  m_PasswordCtrl->ChangeValue(m_password.c_str());
  // Enable Go button iff m_url isn't empty
  wxWindow *goBtn = FindWindow(ID_GO_BTN);
  goBtn->Enable(!m_url.empty());
  // Enable Send button iff m_email isn't empty
  wxWindow *sendBtn = FindWindow(ID_SEND_BTN);
#ifdef NOTYET
  sendBtn->Enable(!m_email.empty());
#endif
  // XXX since PWSRun not yet implemented in Linux, Send button's always disabled:
  sendBtn->Enable(false);
  m_notes = (m_type != SheetType::ADD && m_isNotesHidden) ?
    wxString(_("[Notes hidden - click here to display]")) : towxstring(m_item.GetNotes(TCHAR('\n')));
  // Following has no effect under Linux :-(
  long style = m_noteTX->GetExtraStyle();
  if (prefs->GetPref(PWSprefs::NotesWordWrap))
    style |= wxTE_WORDWRAP;
  else
    style &= ~wxTE_WORDWRAP;
  m_noteTX->SetExtraStyle(style);
  m_autotype = m_item.GetAutoType().c_str();
  m_runcmd = m_item.GetRunCommand().c_str();

  // double-click actions:
  m_item.GetDCA(m_DCA, false);
  m_item.GetDCA(m_ShiftDCA, true);
  SetupDCAComboBoxes(m_DCAcomboBox, m_DCA, false);
  SetupDCAComboBoxes(m_SDCAcomboBox, m_ShiftDCA, true);

  // History: If we're adding, use preferences, otherwise,
  // get values from m_item
  if (m_type == SheetType::ADD) {
    // Get history preferences
    m_keepPWHist = prefs->GetPref(PWSprefs::SavePasswordHistory);
    m_maxPWHist = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
    
    // Get default user name preference
    if (prefs->GetPref(PWSprefs::UseDefaultUser)) {
      m_user = towxstring(prefs->GetPref(PWSprefs::DefaultUsername));
    }
  } else { // EDIT or VIEW
    PWHistList pwhl;
    size_t pwh_max, num_err;

    const StringX pwh_str = m_item.GetPWHistory();
    if (!pwh_str.empty()) {
      m_PWHistory = towxstring(pwh_str);
      m_keepPWHist = CreatePWHistoryList(pwh_str,
                                         pwh_max, num_err,
                                         pwhl, PWSUtil::TMC_LOCALE);
      if (size_t(m_PWHgrid->GetNumberRows()) < pwhl.size()) {
        m_PWHgrid->AppendRows(pwhl.size() - m_PWHgrid->GetNumberRows());
      }
      m_maxPWHist = int(pwh_max);
      //reverse-sort the history entries so that we list the newest first
      std::sort(pwhl.begin(), pwhl.end(), newer());
      int row = 0;
      for (PWHistList::iterator iter = pwhl.begin(); iter != pwhl.end();
           ++iter) {
        m_PWHgrid->SetCellValue(row, 0, iter->changedate.c_str());
        m_PWHgrid->SetCellValue(row, 1, iter->password.c_str());
        row++;
      }
    } else { // empty history string
      // Get history preferences
      m_keepPWHist = prefs->GetPref(PWSprefs::SavePasswordHistory);
      m_maxPWHist = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
    }
  } // m_type

  // Password Expiration
  UpdateExpTimes();
  // Modification times
  m_CTime = m_item.GetCTimeL().c_str();
  m_PMTime = m_item.GetPMTimeL().c_str();
  m_ATime = m_item.GetATimeL().c_str();
  m_RMTime = m_item.GetRMTimeL().c_str();

  // Password policy
  PWPolicy policy;
  // Populate the policy names combo box:
  m_cbxPolicyNames->Append(_("Default Policy"));
  m_core.GetPolicyNames(svec);
  for (sviter = svec.begin(); sviter != svec.end(); sviter++)
    m_cbxPolicyNames->Append(sviter->c_str());
  // Does item use a named policy or item-specific policy?
  bool namedPwPolicy = !m_item.GetPolicyName().empty();
  UNREFERENCED_PARAMETER(namedPwPolicy); // Remove MS Compiler warning

  bool specificPwPolicy = !m_item.GetPWPolicy().empty();
  ASSERT(!(namedPwPolicy && specificPwPolicy)); // both cannot be true!
  m_defPWPRB->SetValue(!specificPwPolicy);
  m_ourPWPRB->SetValue(specificPwPolicy);
  if (specificPwPolicy) {
    m_item.GetPWPolicy(policy);
    policy.symbols = m_item.GetSymbols().c_str();
    if (!policy.symbols.empty())
      m_symbols = policy.symbols.c_str();
  } else { // no item-specific policy, either default or named
    // Select item's named policy, or Default
    const wxString itemPolName = m_item.GetPolicyName().c_str();
    if (!itemPolName.IsEmpty()) {
      m_cbxPolicyNames->SetValue(itemPolName);
      m_core.GetPolicyFromName(tostringx(itemPolName), policy);
    } else {
      m_cbxPolicyNames->SetValue(_("Default Policy"));
      policy = prefs->GetDefaultPolicy();
    }
  }
  UpdatePWPolicyControls(policy);
  EnablePWPolicyControls(specificPwPolicy);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN
 */

void AddEditPropSheet::OnGoButtonClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow() && !m_url.IsEmpty())
    ::wxLaunchDefaultBrowser(m_url, wxBROWSER_NEW_WINDOW);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void AddEditPropSheet::OnGenerateButtonClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    PWPolicy pwp = GetSelectedPWPolicy();
    StringX password = pwp.MakeRandomPassword();
    if (password.empty()) {
      wxMessageBox(_("Couldn't generate password - invalid policy"),
                   _("Error"), wxOK|wxICON_INFORMATION, this);
      return;
    }

    PWSclipboard::GetInstance()->SetData(password);
    m_password = password.c_str();
    m_PasswordCtrl->ChangeValue(m_password.c_str());
    if (m_isPWHidden) {
      m_Password2Ctrl->ChangeValue(m_password.c_str());
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void AddEditPropSheet::OnShowHideClick( wxCommandEvent& /* evt */ )
{
  m_password = m_PasswordCtrl->GetValue().c_str(); // save visible password
  if (m_isPWHidden) {
    ShowPassword();
  } else {
    HidePassword();
  }
}

void AddEditPropSheet::ShowPassword()
{
  m_isPWHidden = false;
  m_ShowHideCtrl->SetLabel(_("&Hide"));

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  wxTextCtrl *tmp = m_PasswordCtrl;
  const wxString pwd = m_password.c_str();
  m_PasswordCtrl = new wxTextCtrl(m_BasicPanel, ID_TEXTCTRL2,
                                  pwd,
                                  wxDefaultPosition, wxDefaultSize,
                                  0);
  if (!pwd.IsEmpty()) {
    m_PasswordCtrl->ChangeValue(pwd);
    m_PasswordCtrl->SetModified(true);
  }
  m_PasswordCtrl->MoveAfterInTabOrder(m_UsernameCtrl);
  m_BasicFGSizer->Replace(tmp, m_PasswordCtrl);
  delete tmp;
  m_BasicFGSizer->Layout();
  // Disable confirmation Ctrl, as the user can see the password entered
  m_Password2Ctrl->Clear();
  m_Password2Ctrl->Enable(false);
}

void AddEditPropSheet::HidePassword()
{
  m_isPWHidden = true;
  m_ShowHideCtrl->SetLabel(_("&Show"));

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  // Need verification as the user can not see the password entered
  wxTextCtrl *tmp = m_PasswordCtrl;
  const wxString pwd = m_password.c_str();
  m_PasswordCtrl = new wxTextCtrl(m_BasicPanel, ID_TEXTCTRL2,
                                  pwd,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxTE_PASSWORD);
  ApplyPasswordFont(m_PasswordCtrl);
  m_PasswordCtrl->MoveAfterInTabOrder(m_UsernameCtrl);
  m_BasicFGSizer->Replace(tmp, m_PasswordCtrl);
  delete tmp;
  m_BasicFGSizer->Layout();
  if (!pwd.IsEmpty()) {
    m_PasswordCtrl->ChangeValue(pwd);
    m_PasswordCtrl->SetModified(true);
  }
  m_Password2Ctrl->ChangeValue(pwd);
  m_Password2Ctrl->Enable(true);
}

static short GetSelectedDCA(const wxComboBox *pcbox,
                           short lastval, short defval)
{
  UNREFERENCED_PARAMETER(lastval);

  int sel = pcbox->GetSelection();
  if (sel == wxNOT_FOUND) { // no selection
    return -1;
  } else {
    auto ival = reinterpret_cast<intptr_t>(pcbox->GetClientData(sel));
    return (ival == defval) ? -1 : ival;
  }
}

void AddEditPropSheet::OnOk(wxCommandEvent& /* evt */)
{
  if (Validate() && TransferDataFromWindow()) {
    time_t t;
    const wxString group = m_groupCtrl->GetValue();
    const StringX password = tostringx(m_PasswordCtrl->GetValue());

    if (m_title.IsEmpty() || password.empty()) {
      GetBookCtrl()->SetSelection(0);
      if (m_title.IsEmpty())
        FindWindow(ID_TEXTCTRL5)->SetFocus();
      else
        m_PasswordCtrl->SetFocus();

      wxMessageBox(wxString::Format(wxString(_("This entry must have a %ls")),
                                    (m_title.IsEmpty() ? _("title"): _("password"))),
                   _("Error"), wxOK|wxICON_INFORMATION, this);
      return;
    }

    if (m_isPWHidden) { // hidden passwords - compare both values
      const StringX p2 = tostringx(m_Password2Ctrl->GetValue());
      if (password != p2) {
        wxMessageDialog msg(this, _("Passwords do not match"), _("Error"),
                            wxOK|wxICON_ERROR);
        msg.ShowModal();
        return;
      }
    }

    switch (m_type) {
    case SheetType::EDIT: {
      bool bIsModified, bIsPSWDModified;
      short lastDCA, lastShiftDCA;
      const PWSprefs *prefs = PWSprefs::GetInstance();
      m_item.GetDCA(lastDCA);
      m_DCA = GetSelectedDCA(m_DCAcomboBox, lastDCA,
                             short(prefs->GetPref(PWSprefs::DoubleClickAction)));

      m_item.GetShiftDCA(lastShiftDCA);
      m_ShiftDCA = GetSelectedDCA(m_SDCAcomboBox, lastShiftDCA,
                                  short(prefs->GetPref(PWSprefs::ShiftDoubleClickAction)));
      // Check if modified
      int lastXTimeInt;
      m_item.GetXTimeInt(lastXTimeInt);
      time_t lastXtime;
      m_item.GetXTime(lastXtime);
      // Following ensures that untouched & hidden note
      // isn't marked as modified. Relies on fact that
      // Note field can't be modified w/o first getting focus
      // and that we turn off m_isNotesHidden when that happens.
      if (m_type != SheetType::ADD && m_isNotesHidden)
        m_notes = m_item.GetNotes(TCHAR('\n')).c_str();

      // Create a new PWHistory string based on settings in this dialog, and compare it
      // with the PWHistory string from the item being edited, to see if the user modified it.
      // Note that we are not erasing the history here, even if the user has chosen to not
      // track PWHistory.  So there could be some password entries in the history
      // but the first byte could be zero, meaning we are not tracking it _FROM_NOW_.
      // Clearing the history is something the user must do himself with the "Clear History" button

      // First, Get a list of all password history entries
      size_t pwh_max, num_err;
      PWHistList pwhl;
      (void)CreatePWHistoryList(tostringx(m_PWHistory), pwh_max, num_err,
                                pwhl, PWSUtil::TMC_LOCALE);

      // Create a new PWHistory header, as per settings in this dialog
      size_t numEntries = std::min(pwhl.size(), static_cast<size_t>(m_maxPWHist));
      m_PWHistory = towxstring(MakePWHistoryHeader(m_keepPWHist, m_maxPWHist, numEntries));
      //reverse-sort the history entries to retain only the newest
      std::sort(pwhl.begin(), pwhl.end(), newer());
      // Now add all the existing history entries, up to a max of what the user wants to track
      // This code is from CItemData::UpdatePasswordHistory()
      PWHistList::iterator iter;
      for (iter = pwhl.begin(); iter != pwhl.end() && numEntries > 0; iter++, numEntries--) {
        StringX buffer;
        Format(buffer, _T("%08x%04x%ls"),
               static_cast<long>(iter->changetttdate), iter->password.length(),
               iter->password.c_str());
        m_PWHistory += towxstring(buffer);
      }

      wxASSERT_MSG(numEntries ==0, wxT("Could not save existing password history entries"));

      PWPolicy oldPWP, pwp;
      // get item's effective policy:
      const StringX oldPolName = m_item.GetPolicyName();
      if (oldPolName.empty()) { // either item-specific or default:
        if (m_item.GetPWPolicy().empty())
          oldPWP = PWSprefs::GetInstance()->GetDefaultPolicy();
        else
          m_item.GetPWPolicy(oldPWP);
      } else {
        m_core.GetPolicyFromName(oldPolName, oldPWP);
      }
      // now get dbox's effective policy:
      pwp = GetSelectedPWPolicy();

      bIsModified = (group        != m_item.GetGroup().c_str()       ||
                     m_title      != m_item.GetTitle().c_str()       ||
                     m_user       != m_item.GetUser().c_str()        ||
                     m_notes      != m_item.GetNotes(TCHAR('\n')).c_str()       ||
                     m_url        != m_item.GetURL().c_str()         ||
                     m_email      != m_item.GetEmail().c_str()       ||
                     m_autotype   != m_item.GetAutoType().c_str()    ||
                     m_runcmd     != m_item.GetRunCommand().c_str()  ||
                     m_DCA        != lastDCA                         ||
                     m_ShiftDCA   != lastShiftDCA                    ||
                     m_PWHistory  != m_item.GetPWHistory().c_str()   ||
                     m_tttXTime   != lastXtime                       ||
                     m_XTimeInt   != lastXTimeInt                    ||
                     m_symbols    != m_item.GetSymbols().c_str()     ||
                     oldPWP       != pwp);


        if (!m_item.IsAlias()) {
          bIsPSWDModified = (password != m_item.GetPassword());
        }
        else {
          // Update password to alias form
          // Show text stating that it is an alias
          const CItemData *pbci = m_core.GetBaseEntry(&m_item);
          ASSERT(pbci);
          if (pbci) {
            StringX alias = L"[" +
                pbci->GetGroup() + L":" +
                pbci->GetTitle() + L":" +
                pbci->GetUser()  + L"]";
            bIsPSWDModified = (password != alias);
          }
          else {
            bIsPSWDModified = true;
          }
        }

      if (bIsModified) {
        // Just modify all - even though only 1 may have actually been modified
        m_item.SetGroup(tostringx(group));
        m_item.SetTitle(tostringx(m_title));
        m_item.SetUser(m_user.empty() ?
                       PWSprefs::GetInstance()->
                       GetPref(PWSprefs::DefaultUsername).c_str() : m_user.c_str());
        m_item.SetNotes(tostringx(m_notes));
        m_item.SetURL(tostringx(m_url));
        m_item.SetEmail(tostringx(m_email));
        m_item.SetAutoType(tostringx(m_autotype));
        m_item.SetRunCommand(tostringx(m_runcmd));
        m_item.SetPWHistory(tostringx(m_PWHistory));
        wxString polName;
        if (m_defPWPRB->GetValue()) {
          polName = m_cbxPolicyNames->GetValue();
          if (polName == _("Default Policy"))
            polName = wxEmptyString;
        } else {
          m_item.SetPWPolicy(pwp);
        }
        m_item.SetPolicyName(tostringx(polName));
        m_item.SetDCA(m_DCA);
        m_item.SetShiftDCA(m_ShiftDCA);
        // Check for Group/Username/Title uniqueness
        auto listindex = m_core.Find(m_item.GetGroup(), m_item.GetTitle(), m_item.GetUser());
        if (listindex != m_core.GetEntryEndIter()) {
          auto listItem = m_core.GetEntry(listindex);
          if (listItem.GetUUID() != m_item.GetUUID()) {
            wxMessageDialog msg(this,
                                _("An entry or shortcut with the same Group, Title and Username already exists."),
                                _("Error"), wxOK|wxICON_ERROR);
            msg.ShowModal();
            return;
          }
        }
      } // bIsModified

      time(&t);
      if (bIsPSWDModified) {
        m_item.UpdatePassword(password);
        m_item.SetPMTime(t);
      }
      if (bIsModified || bIsPSWDModified)
        m_item.SetRMTime(t);
      if (m_tttXTime != lastXtime)
        m_item.SetXTime(m_tttXTime);
      if (m_Recurring) {
      if (m_XTimeInt != lastXTimeInt)
        m_item.SetXTimeInt(m_XTimeInt);
      } else
        m_item.SetXTimeInt(0);
      // All fields in m_item now reflect user's edits
      // Let's update the core's data
      uuid_array_t uuid;
      m_item.GetUUID(uuid);
      auto listpos = m_core.Find(uuid);
      ASSERT(listpos != m_core.GetEntryEndIter());
      m_core.Execute(EditEntryCommand::Create(&m_core,
                                              m_core.GetEntry(listpos),
                                              m_item));
      if (m_ui)
        m_ui->GUIRefreshEntry(m_item);
    }
      break;

    case SheetType::ADD:
      m_item.SetGroup(tostringx(group));
      m_item.SetTitle(tostringx(m_title));
      m_item.SetUser(m_user.empty() ?
                     PWSprefs::GetInstance()->
                      GetPref(PWSprefs::DefaultUsername).c_str() : m_user.c_str());
      // Check for Group/Username/Title uniqueness
      if (m_core.Find(m_item.GetGroup(), m_item.GetTitle(), m_item.GetUser()) !=
          m_core.GetEntryEndIter()) {
        wxMessageDialog msg(this,
                            _("An entry or shortcut with the same Group, Title and Username already exists."),
                            _("Error"), wxOK|wxICON_ERROR);
        msg.ShowModal();
        return;
      }
      m_item.SetNotes(tostringx(m_notes));
      m_item.SetURL(tostringx(m_url));
      m_item.SetEmail(tostringx(m_email));
      m_item.SetPassword(password);
      m_item.SetAutoType(tostringx(m_autotype));
      m_item.SetRunCommand(tostringx(m_runcmd));
      m_item.SetDCA(m_DCA);
      m_item.SetShiftDCA(m_ShiftDCA);
      time(&t);
      m_item.SetCTime(t);
      if (m_keepPWHist)
        m_item.SetPWHistory(MakePWHistoryHeader(true, m_maxPWHist, 0));

      m_item.SetXTime(m_tttXTime);
      if (m_XTimeInt > 0 && m_XTimeInt <= 3650)
        m_item.SetXTimeInt(m_XTimeInt);
      if (m_ourPWPRB->GetValue())
        m_item.SetPWPolicy(GetPWPolicyFromUI());

#ifdef NOTYET
      if (m_AEMD.ibasedata > 0) {
        // Password in alias format AND base entry exists
        // No need to check if base is an alias as already done in
        // call to PWScore::ParseBaseEntryPWD
        uuid_array_t alias_uuid;
        m_item.GetUUID(alias_uuid);
        m_AEMD.pcore->AddDependentEntry(m_AEMD.base_uuid, alias_uuid, CItemData::ET_ALIAS);
        m_item.SetPassword(_T("[Alias]"));
        m_item.SetAlias();
        ItemListIter iter = m_AEMD.pcore->Find(m_AEMD.base_uuid);
        if (iter != m_AEMD.pDbx->End()) {
          const CItemData &cibase = iter->second;
          DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
          int nImage = m_AEMD.pDbx->GetEntryImage(cibase);
          m_AEMD.pDbx->SetEntryImage(di->list_index, nImage, true);
          m_AEMD.pDbx->SetEntryImage(di->tree_item, nImage, true);
        }
      } else {
        m_item.SetPassword(m_AEMD.realpassword);
        m_item.SetNormal();
      }
#endif
      if (m_item.IsAlias()) {
        m_item.SetXTime((time_t)0);
        m_item.SetPWPolicy(wxEmptyString);
      } else {
        m_item.SetXTime(m_tttXTime);
      }
      break;
    case SheetType::VIEW:
      // No Update
      break;
    default:
      ASSERT(0);
      break;
    }
    EndModal(wxID_OK);
  }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1
 */

void AddEditPropSheet::OnKeepHistoryClick(wxCommandEvent &)
{
   if (Validate() && TransferDataFromWindow()) {
     // disable spinbox if checkbox is false
     m_MaxPWHistCtrl->Enable(m_keepPWHist);
   }
}

#if 0 // XXX Remove, as we did away with this checkbox!
void AddEditPropSheet::OnOverrideDCAClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    m_DCAcomboBox->Enable(!m_useDefaultDCA);
    if (m_useDefaultDCA) { // restore default
      short dca = short(PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DoubleClickAction));
      for (size_t i = 0; i < sizeof(dcaMapping)/sizeof(dcaMapping[0]); i++)
        if (dca == dcaMapping[i].pv) {
          m_DCAcomboBox->SetValue(dcaMapping[i].name);
          break;
        }
      m_DCA = -1; // 'use default' value
    }
  }
}
#endif

void AddEditPropSheet::SetXTime(wxObject *src)
{
  if (Validate() && TransferDataFromWindow()) {
    wxDateTime xdt;
    if (src == m_ExpDate) { // expiration date changed, update interval
      xdt = m_ExpDate->GetValue();
      xdt.SetHour(0);
      xdt.SetMinute(1);
      wxTimeSpan delta = xdt.Subtract(wxDateTime::Today());
      m_XTimeInt = delta.GetDays();
      m_XTime = xdt.FormatDate();
    } else if (src == m_ExpTimeCtrl) { // expiration interval changed, update date
      // If it's a non-recurring interval, just set XTime to
      // now + interval, XTimeInt should be stored as zero
      // (one-shot semantics)
      // Otherwise, XTime += interval, keep XTimeInt
        xdt = wxDateTime::Now();
        xdt += wxDateSpan(0, 0, 0, m_XTimeInt);
        m_ExpDate->SetValue(xdt);
        m_XTime = xdt.FormatDate();
      if (m_Recurring) {
        wxString rstr;
        rstr.Printf(_(" (every %d days)"), m_XTimeInt);
        m_XTime += rstr;
      }
    } else {
      ASSERT(0);
    }
    m_tttXTime = xdt.GetTicks();
    Validate(); TransferDataToWindow();
  } // Validated & transferred from controls
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
 */

void AddEditPropSheet::OnExpRadiobuttonSelected( wxCommandEvent& evt )
{
  bool On = (evt.GetEventObject() == m_OnRB);
  bool Never = (evt.GetEventObject() == m_NeverRB);

  if (Never) {
    m_XTime = _("Never");
    m_CurXTime.Clear();
    m_tttXTime = time_t(0);
    m_XTimeInt = 90;
    wxDateTime xdt(wxDateTime::Now());
    xdt += wxDateSpan(0, 0, 0, m_XTimeInt);
    m_ExpDate->SetValue(xdt);
    m_Recurring = false;
    TransferDataToWindow();
  }

  m_ExpDate->Enable(On && !Never);
  m_ExpTimeCtrl->Enable(!On && !Never);
  m_RecurringCtrl->Enable(!On && !Never);
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON2
 */

void AddEditPropSheet::OnPWPRBSelected( wxCommandEvent& evt )
{
  EnablePWPolicyControls(evt.GetEventObject() != m_defPWPRB);
}

void AddEditPropSheet::ShowPWPSpinners(bool show)
{
  m_pwMinsGSzr->Show(m_pwNumLCbox,  show, true);
  m_pwMinsGSzr->Show(m_pwNumUCbox,  show, true);
  m_pwMinsGSzr->Show(m_pwNumDigbox, show, true);
  m_pwMinsGSzr->Show(m_pwNumSymbox, show, true);
  m_pwMinsGSzr->Layout();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
 */

void AddEditPropSheet::OnEZreadOrPronounceable(wxCommandEvent& evt)
{
 if (Validate() && TransferDataFromWindow()) {
   if (m_pwpEasyCtrl->GetValue() && m_pwpPronounceCtrl->GetValue()) {
    wxMessageBox(_("Sorry, 'pronounceable' and 'easy-to-read' are not supported together"),
                        _("Password Policy"), wxOK | wxICON_EXCLAMATION, this);
    if (evt.GetEventObject() == m_pwpPronounceCtrl)
      m_pwpPronounceCtrl->SetValue(false);
    else
      m_pwpEasyCtrl->SetValue(false);
   }
   else {
     ShowPWPSpinners(!evt.IsChecked());
   }
 }
}

void AddEditPropSheet::EnableNonHexCBs(bool enable)
{
  EnableSizerChildren(m_pwMinsGSzr, enable);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX9
 */

void AddEditPropSheet::OnUseHexCBClick( wxCommandEvent& /* evt */ )
{
 if (Validate() && TransferDataFromWindow()) {
   bool useHex = m_pwpHexCtrl->GetValue();
   EnableNonHexCBs(!useHex);
 }
}

/*!
 * wxEVT_SET_FOCUS event handler for ID_TEXTCTRL7
 */

void AddEditPropSheet::OnNoteSetFocus( wxFocusEvent& /* evt */ )
{
  if (m_type != SheetType::ADD && m_isNotesHidden) {
    m_isNotesHidden = false;
    m_notes = m_item.GetNotes(TCHAR('\n')).c_str();
    m_noteTX->ChangeValue(m_notes);
  }
}

PWPolicy AddEditPropSheet::GetPWPolicyFromUI()
{
  wxASSERT_MSG(m_ourPWPRB->GetValue() && !m_defPWPRB->GetValue(), wxT("Trying to get Password policy from UI when db defaults are to be used"));

  PWPolicy pwp;

  pwp.length = m_pwpLenCtrl->GetValue();
  pwp.flags = 0;
  pwp.lowerminlength = pwp.upperminlength =
    pwp.digitminlength = pwp.symbolminlength = 0;
  if (m_pwpUseLowerCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseLowercase;
    pwp.lowerminlength = m_pwpLCSpin->GetValue();
  }
  if (m_pwpUseUpperCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseUppercase;
    pwp.upperminlength = m_pwpUCSpin->GetValue();
  }
  if (m_pwpUseDigitsCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseDigits;
    pwp.digitminlength = m_pwpDigSpin->GetValue();
  }
  if (m_pwpSymCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseSymbols;
    pwp.symbolminlength = m_pwpSymSpin->GetValue();
  }

  wxASSERT_MSG(!m_pwpEasyCtrl->GetValue() || !m_pwpPronounceCtrl->GetValue(), wxT("UI Bug: both pronounceable and easy-to-read are set"));

  if (m_pwpEasyCtrl->GetValue())
    pwp.flags |= PWPolicy::UseEasyVision;
  else if (m_pwpPronounceCtrl->GetValue())
    pwp.flags |= PWPolicy::MakePronounceable;
  if (m_pwpHexCtrl->GetValue())
    pwp.flags = PWPolicy::UseHexDigits; //yes, its '=' and not '|='

  pwp.symbols = m_symbols.c_str();

  return pwp;
}

PWPolicy AddEditPropSheet::GetSelectedPWPolicy()
{
  PWPolicy pwp;
  if (m_defPWPRB->GetValue()) {
    const wxString polName = m_cbxPolicyNames->GetValue();
    m_core.GetPolicyFromName(tostringx(polName), pwp);
  } else
    pwp = GetPWPolicyFromUI();
  return pwp;
}

void AddEditPropSheet::OnUpdateResetPWPolicyButton(wxUpdateUIEvent& evt)
{
  evt.Enable(m_ourPWPRB->GetValue());
}

/*
 * Just trying to give the user some visual indication that
 * the password length has to be bigger than the sum of all
 * "at least" lengths.  This is not comprehensive & foolproof
 * since there are far too many ways to make the password length
 * smaller than the sum of "at least" lengths, to even think of.
 *
 * In OnOk(), we just ensure the password length is greater than
 * the sum of all enabled "at least" lengths.  We have to do this in the
 * UI, or else password generation crashes
 */
void AddEditPropSheet::OnAtLeastChars(wxSpinEvent& /*evt*/)
{
  const int min = GetRequiredPWLength();
  //m_pwpLenCtrl->SetRange(min, pwlenCtrl->GetMax());
  if (min > m_pwpLenCtrl->GetValue())
    m_pwpLenCtrl->SetValue(min);
}

int AddEditPropSheet::GetRequiredPWLength() const {
  wxSpinCtrl* spinCtrls[] = {m_pwpUCSpin, m_pwpLCSpin, m_pwpDigSpin, m_pwpSymSpin};
  int total = 0;
  for (size_t idx = 0; idx < WXSIZEOF(spinCtrls); ++idx) {
    if (spinCtrls[idx]->IsEnabled())
      total += spinCtrls[idx]->GetValue();
  }
  return total;
}

void AddEditPropSheet::OnClearPWHist(wxCommandEvent& /*evt*/)
{
  m_PWHgrid->ClearGrid();
  if (m_MaxPWHistCtrl->TransferDataFromWindow() && m_keepPWHist && m_maxPWHist > 0) {
    m_PWHistory = towxstring(MakePWHistoryHeader(m_keepPWHist, m_maxPWHist, 0));
  }
  else
    m_PWHistory.Empty();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
 */

void AddEditPropSheet::OnEZreadCBClick(wxCommandEvent& evt)
{
  stringT st_symbols;
  if (evt.IsChecked()) {
    // Check if pronounceable is also set - forbid both
    if (m_pwpPronounceCtrl->GetValue()) {
      m_pwpEasyCtrl->SetValue(false);
      wxMessageBox(_("Sorry, \"easy-to-read\" and \"pronounceable\" cannot be both selected"),
                   _("Error"), wxOK|wxICON_ERROR, this);
      return;
    }

    st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
  } else { // not checked - restore default symbols to appropriate value
    if (m_pwpPronounceCtrl->GetValue())
      st_symbols = CPasswordCharPool::GetPronounceableSymbols();
    else
      st_symbols = CPasswordCharPool::GetDefaultSymbols();
  }
  m_symbols = st_symbols.c_str();
  m_ownsymbols->SetValue(m_symbols);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX8
 */

void AddEditPropSheet::OnPronouceableCBClick( wxCommandEvent& evt)
{
  stringT st_symbols;
  if (evt.IsChecked()) {
    // Check if ezread is also set - forbid both
    if (m_pwpEasyCtrl->GetValue()) {
      m_pwpPronounceCtrl->SetValue(false);
      wxMessageBox(_("Sorry, \"pronounceable\" and \"easy-to-read\" cannot be both selected"),
                   _("Error"), wxOK|wxICON_ERROR, this);
      return;
    }
    st_symbols = CPasswordCharPool::GetPronounceableSymbols();
  } else { // not checked - restore default symbols to appropriate value
    if (m_pwpEasyCtrl->GetValue())
      st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
    else
      st_symbols = CPasswordCharPool::GetDefaultSymbols();
  }
  m_symbols = st_symbols.c_str();
  m_ownsymbols->SetValue(m_symbols);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON9
 */

void AddEditPropSheet::OnSendButtonClick( wxCommandEvent& event )
{
  UNREFERENCED_PARAMETER(event);
  /*
   * Format is the standard 'mailto:' rules as per RFC 2368.
   * 'mailto:' is prefixed the the string passed to this routine.
   *
   * sAddress[sHeaders]
   *
   * sAddress
   *  One or more valid email addresses separated by a semicolon.
   *  You must use Internet-safe characters. Use %20 for the space character.
   *
   * sHeaders
   *  Optional. One or more name-value pairs. The first pair should be
   *  prefixed by a "?" and any additional pairs should be prefixed by a "&".
   *
   *  The name can be one of the following strings:
   *    subject
   *       Text to appear in the subject line of the message.
   *    body
   *       Text to appear in the body of the message.
   *    CC
   *       Addresses to be included in the "cc" (carbon copy) section of the
   *       message.
   *    BCC
   *       Addresses to be included in the "bcc" (blind carbon copy) section
   *       of the message.
   *
   * Example:
   *   user@example.com?subject=Message Title&body=Message Content"
   */
  if (Validate() && TransferDataFromWindow() && !m_email.IsEmpty()) {
    StringX mail_cmd= tostringx(_("mailto:"));
    mail_cmd += tostringx(m_email);
    PWSRun runner;
    runner.issuecmd(mail_cmd, wxEmptyString, false);
  }
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_POLICYLIST
 */

void AddEditPropSheet::OnPolicylistSelected( wxCommandEvent& event )
{
  const wxString polName = event.GetString();
  PWPolicy policy;
  if (polName == _("Default Policy")) {
    policy = PWSprefs::GetInstance()->GetDefaultPolicy();
  } else {
    if (!m_core.GetPolicyFromName(tostringx(polName), policy)) {
      pws_os::Trace(wxT("Couldn't find policy %ls\n"), ToStr(polName));
      return;
    }
  }
  m_defPWPRB->SetValue(true);
  UpdatePWPolicyControls(policy);
  EnablePWPolicyControls(false);
}

/*!
 * wxEVT_DATE_CHANGED event handler for ID_DATECTRL
 */

void AddEditPropSheet::OnExpDateChanged( wxDateEvent& event )
{
  SetXTime(event.GetEventObject());
}

/*!
 * wxEVT_COMMAND_SPINCTRL_UPDATED event handler for ID_SPINCTRL2
 */

void AddEditPropSheet::OnExpIntervalChanged( wxSpinEvent& event )
{
  SetXTime(event.GetEventObject());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX6
 */

void AddEditPropSheet::OnSymbolsCB( wxCommandEvent& event )
{
  bool checked = event.IsChecked();
  m_ownsymbols->Enable(checked);
  m_pwpSymSpin->Enable(checked);
  FindWindow(ID_RESET_SYMBOLS)->Enable(checked);
}

/*!
 * wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS
 */

void AddEditPropSheet::OnOwnSymSetFocus( wxFocusEvent& event )
{
////@begin wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS in AddEditPropSheet.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS in AddEditPropSheet.
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RESET_SYMBOLS
 */

void AddEditPropSheet::OnResetSymbolsClick( wxCommandEvent& WXUNUSED(event) )
{
  stringT st_symbols;
  if (m_pwpEasyCtrl->GetValue())
    st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
  else if (m_pwpPronounceCtrl->GetValue())
    st_symbols = CPasswordCharPool::GetPronounceableSymbols();
  else
    st_symbols = CPasswordCharPool::GetDefaultSymbols();
  m_symbols = st_symbols.c_str();
  m_ownsymbols->SetValue(m_symbols);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX5
 */

void AddEditPropSheet::OnDigitsCB( wxCommandEvent& event )
{
  m_pwpDigSpin->Enable(event.IsChecked());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX4
 */

void AddEditPropSheet::OnUppercaseCB( wxCommandEvent& event )
{
  m_pwpUCSpin->Enable(event.IsChecked());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
 */

void AddEditPropSheet::OnLowercaseCB( wxCommandEvent& event )
{
  m_pwpLCSpin->Enable(event.IsChecked());
}
