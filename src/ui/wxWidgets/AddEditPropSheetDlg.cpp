/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file AddEditPropSheetDlg.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin includes
#include <wx/bookctrl.h>
////@end includes

#include "AddEditPropSheetDlg.h"
#include "Clipboard.h"

#include "core/PWCharPool.h"
#include "core/PWHistory.h"
#include "os/media.h"
#include "os/run.h"

#include "SelectAliasDlg.h"
////@begin XPM images
////@end XPM images

/*!
 * AddEditPropSheetDlg type definition
 */

IMPLEMENT_CLASS( AddEditPropSheetDlg, wxPropertySheetDialog )

/*!
 * AddEditPropSheetDlg event table definition
 */

BEGIN_EVENT_TABLE( AddEditPropSheetDlg, wxPropertySheetDialog )

  EVT_CLOSE(                                 AddEditPropSheetDlg::OnClose                   )
  EVT_BUTTON(       wxID_OK,                 AddEditPropSheetDlg::OnOk                      )
  EVT_BUTTON(       wxID_CANCEL,             AddEditPropSheetDlg::OnCancel                  )
////@begin AddEditPropSheetDlg event table entries
  EVT_TEXT(         ID_TEXTCTRL_PASSWORD,    AddEditPropSheetDlg::OnPasswordChanged         )
  EVT_TEXT(         ID_TEXTCTRL_PASSWORD2,   AddEditPropSheetDlg::OnPasswordChanged         )
  EVT_BUTTON(       ID_BUTTON_SHOWHIDE,      AddEditPropSheetDlg::OnShowHideClick           )
  EVT_BUTTON(       ID_BUTTON_GENERATE,      AddEditPropSheetDlg::OnGenerateButtonClick     )
  EVT_BUTTON(       ID_BUTTON_ALIAS,         AddEditPropSheetDlg::OnAliasButtonClick        )
  EVT_BUTTON(       ID_GO_BTN,               AddEditPropSheetDlg::OnGoButtonClick           )
  EVT_BUTTON(       ID_SEND_BTN,             AddEditPropSheetDlg::OnSendButtonClick         )
  EVT_CHECKBOX(     ID_CHECKBOX_KEEP,        AddEditPropSheetDlg::OnKeepHistoryClick        )
  EVT_RADIOBUTTON(  ID_RADIOBUTTON_ON,       AddEditPropSheetDlg::OnExpRadiobuttonSelected  )
  EVT_DATE_CHANGED( ID_DATECTRL_EXP_DATE,    AddEditPropSheetDlg::OnExpDateChanged          )
  EVT_RADIOBUTTON(  ID_RADIOBUTTON_IN,       AddEditPropSheetDlg::OnExpRadiobuttonSelected  )
  EVT_SPINCTRL(     ID_SPINCTRL_EXP_TIME,    AddEditPropSheetDlg::OnExpIntervalChanged      )
  EVT_RADIOBUTTON(  ID_RADIOBUTTON_NEVER,    AddEditPropSheetDlg::OnExpRadiobuttonSelected  )
  EVT_COMBOBOX(     ID_POLICYLIST,           AddEditPropSheetDlg::OnPolicylistSelected      )
  EVT_CHECKBOX(     ID_CHECKBOX42,           AddEditPropSheetDlg::OnPasswordPolicySelected  )
  EVT_CHECKBOX(     ID_CHECKBOX3,            AddEditPropSheetDlg::OnLowercaseCB             )
  EVT_CHECKBOX(     ID_CHECKBOX4,            AddEditPropSheetDlg::OnUppercaseCB             )
  EVT_CHECKBOX(     ID_CHECKBOX5,            AddEditPropSheetDlg::OnDigitsCB                )
  EVT_CHECKBOX(     ID_CHECKBOX6,            AddEditPropSheetDlg::OnSymbolsCB               )
  EVT_BUTTON(       ID_RESET_SYMBOLS,        AddEditPropSheetDlg::OnResetSymbolsClick       )
  EVT_CHECKBOX(     ID_CHECKBOX7,            AddEditPropSheetDlg::OnEasyReadCBClick         )
  EVT_CHECKBOX(     ID_CHECKBOX8,            AddEditPropSheetDlg::OnPronouceableCBClick     )
  EVT_CHECKBOX(     ID_CHECKBOX9,            AddEditPropSheetDlg::OnUseHexCBClick           )
////@end AddEditPropSheetDlg event table entries
  EVT_SPINCTRL(     ID_SPINCTRL5,            AddEditPropSheetDlg::OnAtLeastPasswordChars    )
  EVT_SPINCTRL(     ID_SPINCTRL6,            AddEditPropSheetDlg::OnAtLeastPasswordChars    )
  EVT_SPINCTRL(     ID_SPINCTRL7,            AddEditPropSheetDlg::OnAtLeastPasswordChars    )
  EVT_SPINCTRL(     ID_SPINCTRL8,            AddEditPropSheetDlg::OnAtLeastPasswordChars    )

  EVT_BUTTON(       ID_BUTTON_CLEAR_HIST,    AddEditPropSheetDlg::OnClearPasswordHistory    )

  EVT_UPDATE_UI(    ID_COMBOBOX_GROUP,       AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_BUTTON_SHOWHIDE,      AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_BUTTON_GENERATE,      AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_BUTTON_ALIAS,         AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_TITLE,       AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_USERNAME,    AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_PASSWORD,    AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_PASSWORD2,   AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_URL,         AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_EMAIL,       AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_NOTES,       AddEditPropSheetDlg::OnUpdateUI                )

  EVT_UPDATE_UI(    ID_TEXTCTRL_AUTOTYPE,    AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_TEXTCTRL_RUN_CMD,     AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_COMBOBOX_DBC_ACTION,  AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_COMBOBOX_SDBC_ACTION, AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_CHECKBOX_KEEP,        AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_SPINCTRL_MAX_PW_HIST, AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_GRID_PW_HIST,         AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_BUTTON_CLEAR_HIST,    AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_BUTTON_COPY_ALL,      AddEditPropSheetDlg::OnUpdateUI                )

  EVT_UPDATE_UI(    ID_RADIOBUTTON_ON,       AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_DATECTRL_EXP_DATE,    AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_RADIOBUTTON_IN,       AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_SPINCTRL_EXP_TIME,    AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_STATICTEXT_DAYS,      AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_CHECKBOX_RECURRING,   AddEditPropSheetDlg::OnUpdateUI                )
  EVT_UPDATE_UI(    ID_RADIOBUTTON_NEVER,    AddEditPropSheetDlg::OnUpdateUI                )
END_EVENT_TABLE()

/*!
 * AddEditPropSheetDlg constructors
 */

AddEditPropSheetDlg::AddEditPropSheetDlg(wxWindow *parent, PWScore &core,
                                   SheetType type, const CItemData *item,
                                   const wxString& selectedGroup,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size,
                                   long style)
: m_Core(core), m_SelectedGroup(selectedGroup), m_Type(type)
{
  wxASSERT(!parent || parent->IsTopLevel());

  if (item != nullptr) {
    m_Item = *item; // copy existing item to display values
  }
  else {
    m_Item.CreateUUID(); // We're adding a new entry
  }
  
  m_IsNotesHidden = !PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault);

  wxString dlgTitle;
  if (caption == SYMBOL_AUTOPROPSHEETDLG_TITLE) {
    switch(m_Type) {
      case SheetType::ADD:
        dlgTitle = SYMBOL_ADDPROPSHEETDLG_TITLE;
        break;
      case SheetType::EDIT:
        dlgTitle = SYMBOL_EDITPROPSHEETDLG_TITLE;
        break;
      case SheetType::VIEW:
        dlgTitle = SYMBOL_VIEWPROPSHEETDLG_TITLE;
        break;
      default:
        dlgTitle = caption;
        break;
    }
  }

////@begin AddEditPropSheetDlg creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  int flags = (m_Type == SheetType::VIEW) ? (wxCANCEL|wxHELP) : (wxOK|wxCANCEL|wxHELP); // Use Cancel instead of wxCLOSE on view to allow Command-C as copy operation in macOS, otherwise the Command-C is connected to the Close-Button.
  CreateButtons(flags);
  CreateControls();
  ApplyFontPreferences();
  Centre();
////@end AddEditPropSheetDlg creation
  ItemFieldsToPropSheet();
  LayoutDialog();

  // Additional width is needed by static text (itemStaticText4) at "Basic" tab,
  // otherwise text is not correctly shown due to Auto Word Wrap. (At least at KDE)
  SetSizeHints(GetSize().GetWidth() + 20, GetSize().GetHeight());

  if (m_Core.GetReadFileVersion() == PWSfile::V40) {
    InitAttachmentTab();
  }

  // Set the initial focus to the Title control (Otherwise it defaults to the Group control)
  m_BasicTitleTextCtrl->SetFocus();

  bitmapCheckmarkPlaceholder = wxUtilities::GetBitmapResource(wxT("graphics/checkmark_placeholder.xpm"));
  bitmapCheckmarkGreen = wxUtilities::GetBitmapResource(wxT("graphics/checkmark_green.xpm"));
  bitmapCheckmarkGray = wxUtilities::GetBitmapResource(wxT("graphics/checkmark_gray.xpm"));
}

AddEditPropSheetDlg* AddEditPropSheetDlg::Create(wxWindow *parent, PWScore &core,
  SheetType type, const CItemData *item, const wxString &selectedGroup,
  wxWindowID id, const wxString &caption, const wxPoint &pos, 
  const wxSize &size, long style)
{
  return new AddEditPropSheetDlg(parent, core, type, item, selectedGroup, id, caption, pos, size, style);
}
                      
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
 * Control creation for AddEditPropSheetDlg
 */

void AddEditPropSheetDlg::CreateControls()
{
////@begin AddEditPropSheetDlg content construction

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Basic"
  /////////////////////////////////////////////////////////////////////////////

  m_BasicPanel = CreateBasicPanel();
  GetBookCtrl()->AddPage(m_BasicPanel, _("Basic"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Additional"
  /////////////////////////////////////////////////////////////////////////////

  m_AdditionalPanel = CreateAdditionalPanel();
  GetBookCtrl()->AddPage(m_AdditionalPanel, _("Additional"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Dates and Times"
  /////////////////////////////////////////////////////////////////////////////

  GetBookCtrl()->AddPage(CreateDatesTimesPanel(), _("Dates and Times"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password Policy"
  /////////////////////////////////////////////////////////////////////////////

  m_PasswordPolicyPanel = CreatePasswordPolicyPanel();
  GetBookCtrl()->AddPage(m_PasswordPolicyPanel, _("Password Policy"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Attachment"
  /////////////////////////////////////////////////////////////////////////////

  if (m_Core.GetReadFileVersion() == PWSfile::V40) {
    m_AttachmentPanel = CreateAttachmentPanel();
    GetBookCtrl()->AddPage(m_AttachmentPanel, _("Attachment"));
  }

  /////////////////////////////////////////////////////////////////////////////
  // End of Tab Creation
  /////////////////////////////////////////////////////////////////////////////

  // Connect events and objects
  m_BasicNotesTextCtrl->Connect(ID_TEXTCTRL_NOTES, wxEVT_SET_FOCUS, wxFocusEventHandler(AddEditPropSheetDlg::OnNoteSetFocus), nullptr, this);
  m_PasswordPolicyOwnSymbolsTextCtrl->Connect(IDC_OWNSYMBOLS, wxEVT_SET_FOCUS, wxFocusEventHandler(AddEditPropSheetDlg::OnOwnSymSetFocus), nullptr, this);
////@end AddEditPropSheetDlg content construction

  // Non-DialogBlock initializations:
  m_AdditionalPasswordHistoryGrid->SetColLabelValue(0, _("Set Date/Time"));
  m_AdditionalPasswordHistoryGrid->SetColLabelValue(1, _("Password"));
  m_AdditionalPasswordHistoryGrid->EnableEditing(false);

  // Setup symbols
  m_Symbols = CPasswordCharPool::GetDefaultSymbols().c_str();
  m_PasswordPolicyOwnSymbolsTextCtrl->SetValue(m_Symbols);
}

wxPanel* AddEditPropSheetDlg::CreateBasicPanel()
{
  auto *panel = new wxPanel( GetBookCtrl(), ID_PANEL_BASIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(itemBoxSizer3);

  auto *itemStaticTextHint = new wxStaticText(panel, wxID_STATIC, _("All fields marked with an asterisk (*) are required."), wxDefaultPosition, wxDefaultSize, 0);
  itemStaticTextHint->SetFont((itemStaticTextHint->GetFont()).Italic());
  itemBoxSizer3->Add(itemStaticTextHint, 0, wxALIGN_LEFT|wxALL, 10);

  m_BasicSizer = new wxGridBagSizer(/*vgap:*/ 5, /*hgap:*/ 5);
  itemBoxSizer3->Add(m_BasicSizer, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP|wxLEFT|wxBOTTOM|wxRIGHT, 10);

  auto *itemStaticText6 = new wxStaticText( panel, wxID_STATIC, _("Group"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText6, wxGBPosition(/*row:*/ 0, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5),  wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicGroupNamesCtrl = new wxComboBox( panel, ID_COMBOBOX_GROUP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN);
  m_BasicSizer->Add(m_BasicGroupNamesCtrl, wxGBPosition(/*row:*/ 1, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  auto *itemStaticText9 = new wxStaticText( panel, wxID_STATIC, _("Title"), wxDefaultPosition, wxDefaultSize, 0 );
  auto *itemStaticText10 = new wxStaticText( panel, wxID_STATIC, wxT("*"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText10->SetForegroundColour(*wxRED);
  auto *itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer4->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL, 0);
  itemBoxSizer4->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL, 0);
  m_BasicSizer->Add(itemBoxSizer4, wxGBPosition(/*row:*/ 2, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicTitleTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_TITLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicTitleTextCtrl, wxGBPosition(/*row:*/ 3, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  auto *itemStaticText12 = new wxStaticText( panel, wxID_STATIC, _("Username"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText12, wxGBPosition(/*row:*/ 4, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicUsernameTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_USERNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicUsernameTextCtrl , wxGBPosition(/*row:*/ 5, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  m_BasicPasswordTextLabel = new wxStaticText( panel, wxID_STATIC, _("Password"), wxDefaultPosition, wxDefaultSize, 0 );
  auto *itemStaticText11 = new wxStaticText( panel, wxID_STATIC, wxT("*"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText11->SetForegroundColour(*wxRED);
  auto *itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(m_BasicPasswordTextLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
  itemBoxSizer5->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL, 0);
  m_BasicSizer->Add(itemBoxSizer5, wxGBPosition(/*row:*/ 6, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicPasswordTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicPasswordTextCtrl, wxGBPosition(/*row:*/ 7, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  m_BasicPasswordBitmap = new wxStaticBitmap(panel, wxID_ANY, bitmapCheckmarkPlaceholder, wxDefaultPosition, wxDefaultSize, 0);
  m_BasicSizer->Add(m_BasicPasswordBitmap, wxGBPosition(/*row:*/ 7, /*column:*/ 3), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxBOTTOM, 7);

  m_BasicShowHideCtrl = new wxBitmapButton(panel, ID_BUTTON_SHOWHIDE, wxUtilities::GetBitmapResource(wxT("graphics/eye.xpm")), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
  m_BasicSizer->Add(m_BasicShowHideCtrl, wxGBPosition(/*row:*/ 7, /*column:*/ 4), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  auto *itemButton21 = new wxButton( panel, ID_BUTTON_GENERATE, _("&Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton21, wxGBPosition(/*row:*/ 7, /*column:*/ 5), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 7);

  m_BasicPasswordConfirmationTextLabel = new wxStaticText( panel, wxID_STATIC, _("Confirm"), wxDefaultPosition, wxDefaultSize, 0 );
  auto *itemStaticText13 = new wxStaticText( panel, ID_STATICTEXT_PASSWORD2, wxT("*"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText13->SetForegroundColour(*wxRED);
  auto *itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer6->Add(m_BasicPasswordConfirmationTextLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
  itemBoxSizer6->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL, 0);
  m_BasicSizer->Add(itemBoxSizer6, wxGBPosition(/*row:*/ 8, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicPasswordConfirmationTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_PASSWORD2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
  m_BasicSizer->Add(m_BasicPasswordConfirmationTextCtrl, wxGBPosition(/*row:*/ 9, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  m_BasicPasswordConfirmationBitmap = new wxStaticBitmap(panel, wxID_ANY, bitmapCheckmarkPlaceholder, wxDefaultPosition, wxDefaultSize, 0);
  m_BasicSizer->Add(m_BasicPasswordConfirmationBitmap, wxGBPosition(/*row:*/ 9, /*column:*/ 3), wxDefaultSpan, wxALIGN_CENTER|wxALIGN_LEFT|wxBOTTOM, 7);
  
  auto *itemButton22 = new wxButton( panel, ID_BUTTON_ALIAS, _("&Alias To..."), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton22, wxGBPosition(/*row:*/ 9, /*column:*/ 5), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 7);
  if(! PWSprefs::GetInstance()->GetPref(PWSprefs::ShowAliasSelection)) {
    // Per default do not show this button
    itemButton22->Hide();
  }
  
  auto *itemStaticText25 = new wxStaticText( panel, wxID_STATIC, _("URL"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText25, wxGBPosition(/*row:*/ 10, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicUrlTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_URL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicUrlTextCtrl, wxGBPosition(/*row:*/ 11, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  auto *itemButton29 = new wxButton( panel, ID_GO_BTN, _("Go"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton29, wxGBPosition(/*row:*/ 11, /*column:*/ 5), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 7);

  auto *itemStaticText30 = new wxStaticText( panel, wxID_STATIC, _("Email"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText30, wxGBPosition(/*row:*/ 12, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicEmailTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_EMAIL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicEmailTextCtrl, wxGBPosition(/*row:*/ 13, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 5), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 7);

  auto *itemButton34 = new wxButton( panel, ID_SEND_BTN, _("Send"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton34, wxGBPosition(/*row:*/ 13, /*column:*/ 5), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 7);

  auto *itemStaticText36 = new wxStaticText( panel, wxID_STATIC, _("Notes"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText36, wxGBPosition(/*row:*/ 14, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 6), wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM|wxBOTTOM, 0);

  m_BasicNotesTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_NOTES, wxEmptyString, wxDefaultPosition, wxSize(-1, 100), wxTE_MULTILINE );
  m_BasicSizer->Add(m_BasicNotesTextCtrl, wxGBPosition(/*row:*/ 15, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 6), wxEXPAND, 0);

  m_BasicSizer->AddGrowableCol(2);  // Growable text entry fields
  m_BasicSizer->AddGrowableRow(15); // Growable notes field

  m_BasicTitleTextCtrl->SetValidator(wxGenericValidator(&m_Title));
  m_BasicUsernameTextCtrl->SetValidator(wxGenericValidator(&m_User));
  m_BasicUrlTextCtrl->SetValidator(wxGenericValidator(&m_Url));
  m_BasicEmailTextCtrl->SetValidator(wxGenericValidator(&m_Email));
  m_BasicNotesTextCtrl->SetValidator(wxGenericValidator(&m_Notes));

  return panel;
}

wxPanel* AddEditPropSheetDlg::CreateAdditionalPanel()
{
  auto *panel = new wxPanel(GetBookCtrl(), ID_PANEL_ADDITIONAL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *mainSizer = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(mainSizer);

  auto *vBoxSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(vBoxSizer, 1, wxEXPAND|wxALL, 10);

  auto *itemStaticText41 = new wxStaticText(panel, wxID_STATIC, _("Autotype"), wxDefaultPosition, wxDefaultSize, 0);
  vBoxSizer->Add(itemStaticText41, 0, wxALIGN_LEFT|wxBOTTOM, 5);

  auto *itemTextCtrl42 = new wxTextCtrl(panel, ID_TEXTCTRL_AUTOTYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  vBoxSizer->Add(itemTextCtrl42, 0, wxALIGN_LEFT|wxEXPAND|wxBOTTOM, 12);

  auto *itemStaticText43 = new wxStaticText(panel, wxID_STATIC, _("Run Command"), wxDefaultPosition, wxDefaultSize, 0);
  vBoxSizer->Add(itemStaticText43, 0, wxALIGN_LEFT|wxBOTTOM, 5);

  auto *itemTextCtrl44 = new wxTextCtrl(panel, ID_TEXTCTRL_RUN_CMD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  vBoxSizer->Add(itemTextCtrl44, 0, wxALIGN_LEFT|wxEXPAND|wxBOTTOM, 12);

  auto *itemStaticText45 = new wxStaticText(panel, wxID_STATIC, _("Double-Click Action"), wxDefaultPosition, wxDefaultSize, 0);
  vBoxSizer->Add(itemStaticText45, 0, wxALIGN_LEFT|wxBOTTOM, 5);

  wxArrayString dcaComboBoxStrings;
  setupDCAStrings(dcaComboBoxStrings);
  m_AdditionalDoubleClickActionCtrl = new wxComboBox(panel, ID_COMBOBOX_DBC_ACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, dcaComboBoxStrings, wxCB_READONLY);
  vBoxSizer->Add(m_AdditionalDoubleClickActionCtrl, 0, wxALIGN_LEFT|wxEXPAND|wxBOTTOM, 12);

  auto *itemStaticText47 = new wxStaticText(panel, wxID_STATIC, _("Shift-Double-Click Action"), wxDefaultPosition, wxDefaultSize, 0);
  vBoxSizer->Add(itemStaticText47, 0, wxALIGN_LEFT|wxBOTTOM, 5);

  wxArrayString sdcaComboBoxStrings;
  setupDCAStrings(sdcaComboBoxStrings);
  m_AdditionalShiftDoubleClickActionCtrl = new wxComboBox(panel, ID_COMBOBOX_SDBC_ACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, sdcaComboBoxStrings, wxCB_READONLY);
  vBoxSizer->Add(m_AdditionalShiftDoubleClickActionCtrl, 0, wxALIGN_LEFT|wxEXPAND|wxBOTTOM, 12);

  auto *itemStaticBoxSizer49Static = new wxStaticBox(panel, wxID_ANY, _("Password History"));
  auto *itemStaticBoxSizer49 = new wxStaticBoxSizer(itemStaticBoxSizer49Static, wxVERTICAL);
  vBoxSizer->Add(itemStaticBoxSizer49, 1, wxEXPAND | wxALL, 0);
  auto *itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer50, 0, wxEXPAND | wxALL, 5);
  auto *itemCheckBox51 = new wxCheckBox(panel, ID_CHECKBOX_KEEP, _("Keep"), wxDefaultPosition, wxDefaultSize, 0);
  itemCheckBox51->SetValue(false);
  itemBoxSizer50->Add(itemCheckBox51, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_AdditionalMaxPasswordHistoryCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL_MAX_PW_HIST, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::NumPWHistoryDefault),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::NumPWHistoryDefault),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::NumPWHistoryDefault)
  );

  FixInitialSpinnerSize(m_AdditionalMaxPasswordHistoryCtrl);

  itemBoxSizer50->Add(m_AdditionalMaxPasswordHistoryCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText53 = new wxStaticText(panel, wxID_STATIC, _("last passwords"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer50->Add(itemStaticText53, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_AdditionalPasswordHistoryGrid = new wxGrid(panel, ID_GRID_PW_HIST, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
  m_AdditionalPasswordHistoryGrid->SetDefaultColSize(225);
  m_AdditionalPasswordHistoryGrid->SetDefaultRowSize(25);
  m_AdditionalPasswordHistoryGrid->SetColLabelSize(25);
  m_AdditionalPasswordHistoryGrid->SetRowLabelSize(0);
  m_AdditionalPasswordHistoryGrid->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemStaticBoxSizer49->Add(m_AdditionalPasswordHistoryGrid, 1, wxEXPAND | wxALL, 5);

  auto *itemBoxSizer55 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer55, 0, wxEXPAND | wxALL, 5);
  auto *itemButton56 = new wxButton(panel, ID_BUTTON_CLEAR_HIST, _("Clear History"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer55->Add(itemButton56, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  itemBoxSizer55->AddStretchSpacer();

  auto *itemButton58 = new wxButton(panel, ID_BUTTON_COPY_ALL, _("Copy All"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer55->Add(itemButton58, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  itemTextCtrl42->SetValidator(wxGenericValidator(&m_Autotype));
  itemTextCtrl44->SetValidator(wxGenericValidator(&m_RunCommand));
  itemCheckBox51->SetValidator(wxGenericValidator(&m_KeepPasswordHistory));

  m_AdditionalMaxPasswordHistoryCtrl->SetValidator(wxGenericValidator(&m_MaxPasswordHistory));

  return panel;
}

wxPanel* AddEditPropSheetDlg::CreateDatesTimesPanel()
{
  auto *panel = new wxPanel(GetBookCtrl(), ID_PANEL_DTIME, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *itemBoxSizer60 = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(itemBoxSizer60);

  auto *itemStaticBoxSizer61Static = new wxStaticBox(panel, wxID_ANY, _("Password Expiry"));
  auto *itemStaticBoxSizer61 = new wxStaticBoxSizer(itemStaticBoxSizer61Static, wxVERTICAL);
  itemBoxSizer60->Add(itemStaticBoxSizer61, 0, wxEXPAND | wxALL, 10);
  auto *itemBoxSizer62 = new wxBoxSizer(wxVERTICAL);
  itemStaticBoxSizer61->Add(itemBoxSizer62, 0, wxEXPAND | wxALL, 0);
  auto *itemFlexGridSizer63 = new wxFlexGridSizer(0, 3, 0, 0);
  itemBoxSizer62->Add(itemFlexGridSizer63, 0, wxEXPAND | wxALL, 5);
  m_DatesTimesExpireOnCtrl = new wxRadioButton(panel, ID_RADIOBUTTON_ON, _("On"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer63->Add(m_DatesTimesExpireOnCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto DatePickerType = wxDP_DEFAULT;
#if wxCHECK_VERSION(3, 1, 0)
  if ((wxGetOsVersion() & wxOS_MAC) && wxCheckOsVersion(10, 15, 4))
    DatePickerType = wxDP_DROPDOWN;
#endif
  m_DatesTimesExpiryDateCtrl = new wxDatePickerCtrl(panel, ID_DATECTRL_EXP_DATE, wxDateTime(), wxDefaultPosition, wxDefaultSize, DatePickerType);
  itemFlexGridSizer63->Add(m_DatesTimesExpiryDateCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  itemFlexGridSizer63->AddStretchSpacer();

  m_DatesTimesExpireInCtrl = new wxRadioButton(panel, ID_RADIOBUTTON_IN, _("In"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer63->Add(m_DatesTimesExpireInCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemBoxSizer68 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer63->Add(itemBoxSizer68, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 0);

  m_DatesTimesExpiryTimeCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL_EXP_TIME, _T("90"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::DefaultExpiryDays)
  );

  FixInitialSpinnerSize(m_DatesTimesExpiryTimeCtrl);

  itemBoxSizer68->Add(m_DatesTimesExpiryTimeCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_DatesTimesStaticTextDays = new wxStaticText(panel, ID_STATICTEXT_DAYS, _("days"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer68->Add(m_DatesTimesStaticTextDays, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_DatesTimesRecurringExpiryCtrl = new wxCheckBox(panel, ID_CHECKBOX_RECURRING, _("Recurring"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer63->Add(m_DatesTimesRecurringExpiryCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemBoxSizer72 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer72, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM, 5);
  m_DatesTimesNeverExpireCtrl = new wxRadioButton(panel, ID_RADIOBUTTON_NEVER, _("Never"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer72->Add(m_DatesTimesNeverExpireCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemBoxSizer74 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer74, 0, wxEXPAND | wxALL, 5);
  auto *itemStaticText75 = new wxStaticText(panel, wxID_STATIC, _("Current Setting:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer74->Add(itemStaticText75, 0, wxALIGN_TOP | wxALL, 5);

  m_DatesTimesCurrentCtrl = new wxStaticText(panel, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(-1, 40), 0);
  itemBoxSizer74->Add(m_DatesTimesCurrentCtrl, 0, wxALIGN_TOP | wxALL, 5);

  auto *itemStaticBoxSizer77Static = new wxStaticBox(panel, wxID_ANY, _("Statistics"));
  auto *itemStaticBoxSizer77 = new wxStaticBoxSizer(itemStaticBoxSizer77Static, wxVERTICAL);
  itemBoxSizer60->Add(itemStaticBoxSizer77, 0, wxEXPAND | wxALL, 10);
  auto *itemFlexGridSizer78 = new wxFlexGridSizer(0, 2, 0, 0);
  itemStaticBoxSizer77->Add(itemFlexGridSizer78, 0, wxALIGN_LEFT | wxALL, 5);
  auto *itemStaticText79 = new wxStaticText(panel, wxID_STATIC, _("Created on:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText79, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText80 = new wxStaticText(panel, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText80, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText81 = new wxStaticText(panel, wxID_STATIC, _("Password last changed on:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText81, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText82 = new wxStaticText(panel, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText82, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText83 = new wxStaticText(panel, wxID_STATIC, _("Last accessed on:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText83, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText84 = new wxStaticText(panel, wxID_STATIC, _("N/A"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText84, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText85 = new wxStaticText(panel, wxID_STATIC, _("Any field last changed on:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText85, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText86 = new wxStaticText(panel, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer78->Add(itemStaticText86, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_DatesTimesExpiryTimeCtrl->SetValidator(wxGenericValidator(&m_ExpirationTimeInterval));
  m_DatesTimesRecurringExpiryCtrl->SetValidator(wxGenericValidator(&m_Recurring));
  m_DatesTimesCurrentCtrl->SetValidator(wxGenericValidator(&m_OriginalExpirationStr));
  itemStaticText80->SetValidator(wxGenericValidator(&m_CreationTime));
  itemStaticText82->SetValidator(wxGenericValidator(&m_ModificationTime));
  itemStaticText84->SetValidator(wxGenericValidator(&m_AccessTime));
  itemStaticText86->SetValidator(wxGenericValidator(&m_RMTime));

  return panel;
}

wxPanel* AddEditPropSheetDlg::CreatePasswordPolicyPanel()
{
  auto *panel = new wxPanel(GetBookCtrl(), ID_PANEL_PPOLICY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(itemBoxSizer61);

  auto *itemStaticBoxSizer88Static = new wxStaticBox(panel, wxID_ANY, _("Random password generation rules"));
  auto *itemStaticBoxSizer88 = new wxStaticBoxSizer(itemStaticBoxSizer88Static, wxVERTICAL);
  itemBoxSizer61->Add(itemStaticBoxSizer88, 0, wxEXPAND | wxALL, 10);

  auto *itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer88->Add(itemBoxSizer89, 2, wxEXPAND | wxALL, 0);

  m_PasswordPolicyUseDatabaseCtrl = new wxCheckBox(panel, ID_CHECKBOX42, _("Use Named Policy"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  m_PasswordPolicyUseDatabaseCtrl->SetValue(false);
  itemBoxSizer89->Add(m_PasswordPolicyUseDatabaseCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  itemBoxSizer89->AddStretchSpacer();

  wxArrayString m_cbxPolicyNamesStrings;
  m_PasswordPolicyNamesCtrl = new wxComboBox(panel, ID_POLICYLIST, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_cbxPolicyNamesStrings, wxCB_READONLY);
  itemBoxSizer89->Add(m_PasswordPolicyNamesCtrl, 3, wxALIGN_TOP | wxALL, 5);

  auto *itemStaticLine94 = new wxStaticLine(panel, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  itemStaticBoxSizer88->Add(itemStaticLine94, 0, wxEXPAND | wxALL, 5);

  auto *itemBoxSizer95 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer88->Add(itemBoxSizer95, 0, wxALIGN_LEFT | wxALL, 5);
  m_PasswordPolicyPasswordLengthText = new wxStaticText(panel, wxID_STATIC, _("Password length:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer95->Add(m_PasswordPolicyPasswordLengthText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxALL, 5);

  m_PasswordPolicyPasswordLengthCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL3, _T("12"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDefaultLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDefaultLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWDefaultLength)
  );

  FixInitialSpinnerSize(m_PasswordPolicyPasswordLengthCtrl);

  itemBoxSizer95->Add(m_PasswordPolicyPasswordLengthCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicySizer = new wxFlexGridSizer(0, 2, 0, 0);
  itemStaticBoxSizer88->Add(m_PasswordPolicySizer, 0, wxALIGN_LEFT | wxALL, 5);

  // Lower Case Rules
  m_PasswordPolicyUseLowerCaseCtrl = new wxCheckBox(panel, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUseLowerCaseCtrl->SetValue(false);
  m_PasswordPolicySizer->Add(m_PasswordPolicyUseLowerCaseCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicyLowerCaseMinSizer = new wxBoxSizer(wxHORIZONTAL);
  m_PasswordPolicySizer->Add(m_PasswordPolicyLowerCaseMinSizer, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
  auto *itemStaticText101 = new wxStaticText(panel, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyLowerCaseMinSizer->Add(itemStaticText101, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  m_PasswordPolicyLowerCaseMinCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWLowercaseMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWLowercaseMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWLowercaseMinLength)
  );

  FixInitialSpinnerSize(m_PasswordPolicyLowerCaseMinCtrl);

  m_PasswordPolicyLowerCaseMinSizer->Add(m_PasswordPolicyLowerCaseMinCtrl, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);

  auto *itemStaticText103 = new wxStaticText(panel, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyLowerCaseMinSizer->Add(itemStaticText103, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  // Upper Case Rules
  m_PasswordPolicyUseUpperCaseCtrl = new wxCheckBox(panel, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUseUpperCaseCtrl->SetValue(false);
  m_PasswordPolicySizer->Add(m_PasswordPolicyUseUpperCaseCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicyUpperCaseMinSizer = new wxBoxSizer(wxHORIZONTAL);
  m_PasswordPolicySizer->Add(m_PasswordPolicyUpperCaseMinSizer, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
  auto *itemStaticText106 = new wxStaticText(panel, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUpperCaseMinSizer->Add(itemStaticText106, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  m_PasswordPolicyUpperCaseMinCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWUppercaseMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWUppercaseMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWUppercaseMinLength)
  );

  FixInitialSpinnerSize(m_PasswordPolicyUpperCaseMinCtrl);

  m_PasswordPolicyUpperCaseMinSizer->Add(m_PasswordPolicyUpperCaseMinCtrl, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);

  auto *itemStaticText108 = new wxStaticText(panel, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUpperCaseMinSizer->Add(itemStaticText108, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  // Digits Rules
  m_PasswordPolicyUseDigitsCtrl = new wxCheckBox(panel, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUseDigitsCtrl->SetValue(false);
  m_PasswordPolicySizer->Add(m_PasswordPolicyUseDigitsCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicyDigitsMinSizer = new wxBoxSizer(wxHORIZONTAL);
  m_PasswordPolicySizer->Add(m_PasswordPolicyDigitsMinSizer, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
  auto *itemStaticText111 = new wxStaticText(panel, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyDigitsMinSizer->Add(itemStaticText111, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  m_PasswordPolicyDigitsMinCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDigitMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDigitMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWDigitMinLength)
  );

  FixInitialSpinnerSize(m_PasswordPolicyDigitsMinCtrl);

  m_PasswordPolicyDigitsMinSizer->Add(m_PasswordPolicyDigitsMinCtrl, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);

  auto *itemStaticText113 = new wxStaticText(panel, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyDigitsMinSizer->Add(itemStaticText113, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  // Symbols Rules
  m_PasswordPolicyUseSymbolsCtrl = new wxCheckBox(panel, ID_CHECKBOX6, _("Use symbols"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUseSymbolsCtrl->SetValue(false);
  m_PasswordPolicyUseSymbolsCtrl->Bind(wxEVT_MOTION, [&](wxMouseEvent & WXUNUSED(event)) { m_PasswordPolicyUseSymbolsCtrl->SetToolTip(_("i.e., ., %, $, etc.")); });
  m_PasswordPolicySizer->Add(m_PasswordPolicyUseSymbolsCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicySymbolsMinSizer = new wxBoxSizer(wxHORIZONTAL);
  m_PasswordPolicySizer->Add(m_PasswordPolicySymbolsMinSizer, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
  auto *itemStaticText116 = new wxStaticText(panel, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicySymbolsMinSizer->Add(itemStaticText116, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  m_PasswordPolicySymbolsMinCtrl = new wxSpinCtrl(
    panel, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWSymbolMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWSymbolMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWSymbolMinLength)
  );

  FixInitialSpinnerSize(m_PasswordPolicySymbolsMinCtrl);

  m_PasswordPolicySymbolsMinSizer->Add(m_PasswordPolicySymbolsMinCtrl, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM, 5);

  auto *itemStaticText118 = new wxStaticText(panel, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicySymbolsMinSizer->Add(itemStaticText118, 0, wxALIGN_CENTER_VERTICAL | wxLEFT|wxBOTTOM|wxRIGHT, 5);

  // Own Symbols Rules
  m_PasswordPolicyOwnSymbolsTextCtrl = new wxTextCtrl(panel, IDC_OWNSYMBOLS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicySizer->Add(m_PasswordPolicyOwnSymbolsTextCtrl, 1, wxALIGN_LEFT | wxEXPAND | wxALL, 5);

  auto *itemButton120 = new wxButton(panel, ID_RESET_SYMBOLS, _("Reset"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicySizer->Add(itemButton120, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicyUseEasyCtrl = new wxCheckBox(panel, ID_CHECKBOX7, _("Use only easy-to-read characters"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUseEasyCtrl->SetValue(false);
  m_PasswordPolicyUseEasyCtrl->Bind(wxEVT_MOTION, [&](wxMouseEvent & WXUNUSED(event)) { m_PasswordPolicyUseEasyCtrl->SetToolTip(_("i.e., no 'l', '1', etc.")); });
  m_PasswordPolicySizer->Add(m_PasswordPolicyUseEasyCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicySizer->AddStretchSpacer();

  m_PasswordPolicyUsePronounceableCtrl = new wxCheckBox(panel, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUsePronounceableCtrl->SetValue(false);
  m_PasswordPolicySizer->Add(m_PasswordPolicyUsePronounceableCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_PasswordPolicySizer->AddStretchSpacer();

  auto *itemStaticText125 = new wxStaticText(panel, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicySizer->Add(itemStaticText125, 0, wxALIGN_LEFT | wxALL, 5);

  m_PasswordPolicySizer->AddStretchSpacer();

  m_PasswordPolicyUseHexadecimalOnlyCtrl = new wxCheckBox(panel, ID_CHECKBOX9, _("Use hexadecimal digits only"), wxDefaultPosition, wxDefaultSize, 0);
  m_PasswordPolicyUseHexadecimalOnlyCtrl->SetValue(false);
  m_PasswordPolicyUseHexadecimalOnlyCtrl->Bind(wxEVT_MOTION, [&](wxMouseEvent & WXUNUSED(event)) { m_PasswordPolicyUseHexadecimalOnlyCtrl->SetToolTip(_("0-9, a-f")); });
  m_PasswordPolicySizer->Add(m_PasswordPolicyUseHexadecimalOnlyCtrl, 0, wxALIGN_LEFT | wxALL, 5);

  m_PasswordPolicyOwnSymbolsTextCtrl->SetValidator(wxGenericValidator(&m_Symbols));

  return panel;
}

wxPanel* AddEditPropSheetDlg::CreateAttachmentPanel()
{
  ID_BUTTON_IMPORT = wxWindow::NewControlId();
  ID_BUTTON_EXPORT = wxWindow::NewControlId();
  ID_BUTTON_REMOVE = wxWindow::NewControlId();

  auto *panel = new wxPanel(GetBookCtrl(), ID_PANEL_ADDITIONAL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *BoxSizerMain = new wxBoxSizer(wxVERTICAL);

  StaticBoxSizerPreview = new wxStaticBoxSizer(wxHORIZONTAL, panel, _("Preview"));
  m_AttachmentImagePanel = new ImagePanel(panel, wxDefaultSize);
  StaticBoxSizerPreview->Add(m_AttachmentImagePanel, 1, wxALL|wxEXPAND, 5);
  m_AttachmentPreviewStatus = new wxStaticText(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL, _T("ID_STATICTEXT_STATUS"));
  StaticBoxSizerPreview->Add(m_AttachmentPreviewStatus, 1, wxALL|wxALIGN_CENTER, 5);
  StaticBoxSizerPreview->SetMinSize(wxSize(-1, 300));
  BoxSizerMain->Add(StaticBoxSizerPreview, 1, wxALL|wxEXPAND, 10);

  auto *StaticBoxSizerFile = new wxStaticBoxSizer(wxVERTICAL, panel, _("File"));
  m_AttachmentFilePath = new wxStaticText(panel, wxID_ANY, _("N/A"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxST_ELLIPSIZE_MIDDLE, _T("ID_STATICTEXT_PATH"));
  StaticBoxSizerFile->Add(m_AttachmentFilePath, 0, wxLEFT|wxBOTTOM|wxRIGHT|wxEXPAND, 10);

  auto *BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  m_AttachmentButtonImport = new wxButton(panel, ID_BUTTON_IMPORT, _("Import..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
  BoxSizer3->Add(m_AttachmentButtonImport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentButtonExport = new wxButton(panel, ID_BUTTON_EXPORT, _("Export..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EXPORT"));
  BoxSizer3->Add(m_AttachmentButtonExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentButtonRemove = new wxButton(panel, ID_BUTTON_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE"));
  BoxSizer3->Add(m_AttachmentButtonRemove, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  StaticBoxSizerFile->Add(BoxSizer3, 0, wxALL|wxEXPAND, 5);
  BoxSizerMain->Add(StaticBoxSizerFile, 0, wxLEFT|wxBOTTOM|wxRIGHT|wxEXPAND, 10);

  auto *StaticBoxSizerProperties = new wxStaticBoxSizer(wxHORIZONTAL, panel, _("Properties"));
  auto *FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  FlexGridSizer1->AddGrowableCol(1);

  auto *StaticText3 = new wxStaticText(panel, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(StaticText3, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentTitle = new wxTextCtrl(panel, wxID_ANY, _("Text"), wxDefaultPosition, wxSize(217,35), 0, wxDefaultValidator);
  FlexGridSizer1->Add(m_AttachmentTitle, 1, wxALL|wxEXPAND, 5);

  auto *StaticText2 = new wxStaticText(panel, wxID_ANY, _("Media Type:"), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(StaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentMediaType = new wxStaticText(panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(m_AttachmentMediaType, 1, wxALL|wxEXPAND, 5);

  auto *StaticText4 = new wxStaticText(panel, wxID_ANY, _("Creation Date:"), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(StaticText4, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentCreationDate = new wxStaticText(panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(m_AttachmentCreationDate, 1, wxALL|wxEXPAND, 5);

  auto *StaticText5 = new wxStaticText(panel, wxID_ANY, _("File Size:"), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(StaticText5, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentFileSize = new wxStaticText(panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(m_AttachmentFileSize, 1, wxALL|wxEXPAND, 5);

  auto *StaticText7 = new wxStaticText(panel, wxID_ANY, _("File Creation Date:"), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(StaticText7, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentFileCreationDate = new wxStaticText(panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(m_AttachmentFileCreationDate, 1, wxALL|wxEXPAND, 5);

  auto *StaticText9 = new wxStaticText(panel, wxID_ANY, _("File Last Modified Date:"), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(StaticText9, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentFileLastModifiedDate = new wxStaticText(panel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  FlexGridSizer1->Add(m_AttachmentFileLastModifiedDate, 1, wxALL|wxEXPAND, 5);
  StaticBoxSizerProperties->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
  BoxSizerMain->Add(StaticBoxSizerProperties, 0, wxLEFT|wxBOTTOM|wxRIGHT|wxEXPAND, 10);

  panel->SetSizer(BoxSizerMain);

  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditPropSheetDlg::OnImport, this, static_cast<int>(ID_BUTTON_IMPORT));
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditPropSheetDlg::OnExport, this, static_cast<int>(ID_BUTTON_EXPORT));
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditPropSheetDlg::OnRemove, this, static_cast<int>(ID_BUTTON_REMOVE));
  //*)

  return panel;
}

void AddEditPropSheetDlg::InitAttachmentTab()
{
  if (m_Item.HasAttRef()) {

    // Get attachment item
    auto uuid = m_Item.GetAttUUID();
    m_ItemAttachment = m_Core.GetAtt(uuid);

    if (m_ItemAttachment.HasContent()) {

      // Mark the attachment from the 'Core' explicitly as 'CLEAN'.
      // The status will be updated on any modification and evaluated at 'OnOk'
      // to determine the necessary action.
      m_ItemAttachment.SetStatus(CItem::EntryStatus::ES_CLEAN);

      // Show attachment data on UI.
      ShowAttachmentData(m_ItemAttachment);

      if (m_Core.IsReadOnly()) {
        DisableAttachmentControls();
      }
      else {
        // Attachment must be removed before a new one can be imported again.
        DisableImport();
      }
    }
    else {
      ResetAttachmentData();
    }
  }
  else {
    ResetAttachmentData();
    HideImagePreview(_("No Attachment Available"));
    if (m_Core.IsReadOnly()) {
      DisableAttachmentControls();
    }
    else {
      EnableImport();
    }
  }
}

/**
 * Updates the attachment data on the UI.
 */
void AddEditPropSheetDlg::ShowAttachmentData(const CItemAtt &itemAttachment)
{
  // Get attachment's file path
  auto filePath = itemAttachment.GetFilePath() + itemAttachment.GetFileName();

  if (filePath.empty()) {
    m_AttachmentFilePath->SetLabel(_("N/A"));
  }
  else {
    m_AttachmentFilePath->SetLabel(stringx2std(filePath));
  }

  // Get attachment's title
  if (itemAttachment.GetTitle().empty()) {
    m_AttachmentTitle->SetValue(_("N/A"));
  }
  else {
    m_AttachmentTitle->SetValue(stringx2std(itemAttachment.GetTitle()));
  }

  // Get attachment's media type
  auto mediaTypeDescription = stringx2std(itemAttachment.GetMediaType());

  if (mediaTypeDescription.empty()) {
    m_AttachmentMediaType->SetLabel(_("N/A"));
  }
  else if (mediaTypeDescription == L"unknown") {
    m_AttachmentMediaType->SetLabel(_("Unknown"));
  }
  else {
    m_AttachmentMediaType->SetLabel(mediaTypeDescription);
  }

  // Get attachment's creation date
  if (itemAttachment.GetCTime().empty()) {
    m_AttachmentCreationDate->SetLabel(_("N/A"));
  }
  else {
    m_AttachmentCreationDate->SetLabel(stringx2std(itemAttachment.GetCTime()));
  }

  // Get attachment's size
  m_AttachmentFileSize->SetLabel(wxString::Format(wxT("%u"), (unsigned int)itemAttachment.GetContentSize()));

  // Get attachment's file creation date
  if (itemAttachment.GetFileCTime().empty()) {
    m_AttachmentFileCreationDate->SetLabel(_("N/A"));
  }
  else {
    m_AttachmentFileCreationDate->SetLabel(stringx2std(itemAttachment.GetFileCTime()));
  }

  // Get attachment's last modification date
  if (itemAttachment.GetFileMTime().empty()) {
    m_AttachmentFileLastModifiedDate->SetLabel(_("N/A"));
  }
  else {
    m_AttachmentFileLastModifiedDate->SetLabel(stringx2std(itemAttachment.GetFileMTime()));
  }

  // Show attachment preview if it is an image,
  // otherwise show text indicating that no preview is available.
  if (IsMimeTypeImage(mediaTypeDescription)) {
    if (LoadImagePreview(itemAttachment)) {
      ShowImagePreview();
    }
    else {
      HideImagePreview(_("No preview available due to an error"));
    }
  }
  else {
    HideImagePreview(_("No preview available - unsupported media type"));
  }
}

void AddEditPropSheetDlg::ResetAttachmentData()
{
  m_AttachmentFilePath->SetLabel(_("N/A"));
  m_AttachmentTitle->SetValue(_("N/A"));
  m_AttachmentMediaType->SetLabel(_("N/A"));
  m_AttachmentCreationDate->SetLabel(_("N/A"));
  m_AttachmentFileSize->SetLabel(_("N/A"));
  m_AttachmentFileCreationDate->SetLabel(_("N/A"));
  m_AttachmentFileLastModifiedDate->SetLabel(_("N/A"));
  m_AttachmentImagePanel->Clear();
  m_ItemAttachment.Clear();
}

/**
 * Loads the attachments image data for showing it on the image panel.
 */
bool AddEditPropSheetDlg::LoadImagePreview(const CItemAtt &itemAttachment)
{
  return m_AttachmentImagePanel->LoadFromAttachment(itemAttachment, this, _("Image Preview"));
}

/**
 * Show image panel and hide the static text
 * that indicates that no preview is available.
 */
void AddEditPropSheetDlg::ShowImagePreview()
{
  m_AttachmentPreviewStatus->SetLabel(wxEmptyString);
  StaticBoxSizerPreview->Hide(m_AttachmentPreviewStatus);
  StaticBoxSizerPreview->Show(m_AttachmentImagePanel);
  StaticBoxSizerPreview->Layout();
}

/**
 * Hide the image panel and show static text
 * that indicates that no preview is available.
 */
void AddEditPropSheetDlg::HideImagePreview(const wxString &reason)
{
  m_AttachmentPreviewStatus->SetLabel(reason);
  StaticBoxSizerPreview->Hide(m_AttachmentImagePanel);
  StaticBoxSizerPreview->Show(m_AttachmentPreviewStatus);
  StaticBoxSizerPreview->Layout();
}

/**
 * Returns the mime type extension that follows after the slash.
 *
 * Example: "image/png" -> "png", "application/zip" -> "zip"
 */
wxString AddEditPropSheetDlg::GetMimeTypeExtension(const stringT &mimeTypeDescription)
{
  if (mimeTypeDescription.find('/') == std::string::npos) {
    return wxEmptyString;
  }
  else {
    return mimeTypeDescription.substr(mimeTypeDescription.find('/') + 1, mimeTypeDescription.length());
  }
}

void AddEditPropSheetDlg::EnableImport()
{
  m_AttachmentButtonImport->Enable();
  m_AttachmentButtonExport->Disable();
  m_AttachmentButtonRemove->Disable();
}

void AddEditPropSheetDlg::DisableImport()
{
  m_AttachmentButtonImport->Disable();
  m_AttachmentButtonExport->Enable();
  m_AttachmentButtonRemove->Enable();
}

void AddEditPropSheetDlg::DisableAttachmentControls()
{
  m_AttachmentButtonImport->Disable();
  m_AttachmentButtonExport->Disable();
  m_AttachmentButtonRemove->Disable();
}

void AddEditPropSheetDlg::OnImport(wxCommandEvent& WXUNUSED(event))
{
  wxString fileFilter =
  _("Image files ") + 
  wxImage::GetImageExtWildcard() + wxT("|") +
  _("All files (*.*)|*.*");

  wxFileDialog fileDialog(
    this, _("Import Attachment"), "", "",
    fileFilter,
    wxFD_OPEN | wxFD_FILE_MUST_EXIST
  );

  if (fileDialog.ShowModal() != wxID_OK) {
    return;
  }

  auto status = m_ItemAttachment.Import(fileDialog.GetPath().ToStdWstring());

  switch (status)
  {
  case PWScore::SUCCESS: {
      m_ItemAttachment.CreateUUID(); // Used by 'AddEntryCommand' to associate the attachment with the item.
      m_ItemAttachment.SetStatus(CItem::EntryStatus::ES_ADDED);

      // Show attachment data on UI.
      ShowAttachmentData(m_ItemAttachment);

      // Attachment must be removed before a new one can be imported again.
      DisableImport();
    }
    break;
  case PWScore::FAILURE:
    wxMessageDialog(
      this,
      _("Failed to allocate required memory to import attachment data."), _("Import Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  case PWScore::CANT_OPEN_FILE:
    wxMessageDialog(
      this,
      _("Failed to open file."), _("Import Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  case PWScore::MAX_SIZE_EXCEEDED:
    wxMessageDialog(
      this,
      _("File exceeds the allowed maximum size of 4.3GB (2^32GB)."), _("Import Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  case PWScore::READ_FAIL:
    wxMessageDialog(
      this,
      _("An error occurred while reading the file."), _("Import Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  default:
    wxMessageDialog(
      this,
      _("Unexpected error occurred during attachment import."), _("Import Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  }
}

void AddEditPropSheetDlg::OnExport(wxCommandEvent& WXUNUSED(event))
{
  if (!m_ItemAttachment.HasContent()) {

    wxMessageDialog(
      this,
      _("No attachment data to export."), _("Export Attachment"),
      wxICON_ERROR
    ).ShowModal();

    return;
  }

  wxString fileFilter;

  auto mimeTypeExtension = GetMimeTypeExtension(stringx2std(m_ItemAttachment.GetMediaType()));

  if (!mimeTypeExtension.empty()) {

    wxString mimeTypeFilter;

    if (mimeTypeExtension.Lower() == wxT("jpeg")) {
      mimeTypeFilter = wxString::Format(_("%s files (*.%s;*.jpg)|*.%s;*.jpg|"),
        mimeTypeExtension.Upper(), mimeTypeExtension.Lower(), mimeTypeExtension.Lower()
      );
    }
    else if (mimeTypeExtension.Lower() == wxT("gzip")) {
      mimeTypeFilter = wxString::Format(_("%s files (*.%s;*.gz)|*.%s;*.gz|"),
        mimeTypeExtension.Upper(), mimeTypeExtension.Lower(), mimeTypeExtension.Lower()
      );
    }
    else {
      mimeTypeFilter = wxString::Format(_("%s files (*.%s)|*.%s|"),
        mimeTypeExtension.Upper(), mimeTypeExtension.Lower(), mimeTypeExtension.Lower()
      );
    }

    fileFilter.Append(mimeTypeFilter);
  }

  fileFilter.Append(_("All files (*.*)|*.*"));

  wxFileDialog fileDialog(
    this, _("Export Attachment"), "", m_ItemAttachment.GetFileName().c_str(),
    fileFilter,
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
  );

  if (fileDialog.ShowModal() != wxID_OK) {
    return;
  }

  auto status = m_ItemAttachment.Export(fileDialog.GetPath().ToStdWstring());

  switch (status)
  {
  case PWScore::SUCCESS:
    break;
  case PWScore::FAILURE:
    wxMessageDialog(
      this,
      _("Failed to allocate required memory to export attachment data."), _("Export Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  case PWScore::CANT_OPEN_FILE:
    wxMessageDialog(
      this,
      _("Failed to open file."), _("Export Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  case PWScore::WRITE_FAIL:
    wxMessageDialog(
      this,
      _("An error occurred while writing the file."), _("Export Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  default:
    wxMessageDialog(
      this,
      _("Unexpected error occurred during attachment export."), _("Export Attachment"),
      wxICON_ERROR
    ).ShowModal();
    break;
  }
}

void AddEditPropSheetDlg::OnRemove(wxCommandEvent& WXUNUSED(event))
{
  HideImagePreview();
  EnableImport();

  ResetAttachmentData();

  m_ItemAttachment.SetStatus(CItem::EntryStatus::ES_DELETED);
}

void AddEditPropSheetDlg::ApplyFontPreferences()
{
  // Tab: "Basic"
  ApplyFontPreference(m_BasicGroupNamesCtrl, PWSprefs::StringPrefs::AddEditFont);                 // Group
  ApplyFontPreference(m_BasicTitleTextCtrl, PWSprefs::StringPrefs::AddEditFont);                  // Title
  ApplyFontPreference(m_BasicUsernameTextCtrl, PWSprefs::StringPrefs::AddEditFont);               // Username
  ApplyFontPreference(m_BasicPasswordTextCtrl, PWSprefs::StringPrefs::PasswordFont);              // Password
  ApplyFontPreference(m_BasicPasswordConfirmationTextCtrl, PWSprefs::StringPrefs::PasswordFont);  // Confirmation Password
  ApplyFontPreference(m_BasicUrlTextCtrl, PWSprefs::StringPrefs::AddEditFont);                    // URL
  ApplyFontPreference(m_BasicEmailTextCtrl, PWSprefs::StringPrefs::AddEditFont);                  // Email
  ApplyFontPreference(m_BasicNotesTextCtrl, PWSprefs::StringPrefs::NotesFont);                    // Notes

  // Tab: "Password Policy"
  ApplyFontPreference(m_PasswordPolicyOwnSymbolsTextCtrl, PWSprefs::StringPrefs::PasswordFont);   // User defined symbols
}

/*!
 * Should we show tooltips?
 */

bool AddEditPropSheetDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap AddEditPropSheetDlg::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin AddEditPropSheetDlg bitmap retrieval
  return wxNullBitmap;
////@end AddEditPropSheetDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AddEditPropSheetDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin AddEditPropSheetDlg icon retrieval
  return wxNullIcon;
////@end AddEditPropSheetDlg icon retrieval
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

void AddEditPropSheetDlg::UpdatePWPolicyControls(const PWPolicy& pwp)
{
  bool bUseVal; // keep picky compiler happy, code readable

  EnableSizerChildren(m_PasswordPolicySizer, !m_PasswordPolicyUseHexadecimalOnlyCtrl->GetValue());
  m_PasswordPolicyPasswordLengthCtrl->SetValue(pwp.length);
  bUseVal = (pwp.flags & PWPolicy::UseLowercase) != 0;
  m_PasswordPolicyUseLowerCaseCtrl->SetValue(bUseVal);
  m_PasswordPolicyLowerCaseMinCtrl->SetValue(pwp.lowerminlength);
  bUseVal = (pwp.flags & PWPolicy::UseUppercase) != 0;
  m_PasswordPolicyUseUpperCaseCtrl->SetValue(bUseVal);
  m_PasswordPolicyUpperCaseMinCtrl->SetValue(pwp.upperminlength);
  bUseVal = (pwp.flags & PWPolicy::UseDigits) != 0;
  m_PasswordPolicyUseDigitsCtrl->SetValue(bUseVal);
  m_PasswordPolicyDigitsMinCtrl->SetValue(pwp.digitminlength);

  bUseVal = (pwp.flags & PWPolicy::UseSymbols) != 0;
  m_PasswordPolicyUseSymbolsCtrl->SetValue(bUseVal);
  m_PasswordPolicySymbolsMinCtrl->SetValue(pwp.symbolminlength);

  bUseVal = (pwp.flags & PWPolicy::UseEasyVision) != 0;
  m_PasswordPolicyUseEasyCtrl->SetValue(bUseVal);
  bUseVal = (pwp.flags & PWPolicy::MakePronounceable) != 0;
  m_PasswordPolicyUsePronounceableCtrl->SetValue(bUseVal);
  bUseVal = (pwp.flags & PWPolicy::UseHexDigits) != 0;
  m_PasswordPolicyUseHexadecimalOnlyCtrl->SetValue(bUseVal);

  ShowPWPSpinners(!m_PasswordPolicyUsePronounceableCtrl->GetValue() && !m_PasswordPolicyUseEasyCtrl->GetValue());

  if (!pwp.symbols.empty()) {
    m_Symbols = pwp.symbols.c_str();

    auto policyPanel = FindWindow(ID_PANEL_PPOLICY);

    if (policyPanel) {
      policyPanel->Validate();
      policyPanel->TransferDataToWindow();
    }
  }
}

void AddEditPropSheetDlg::EnablePWPolicyControls(bool enable)
{
  m_PasswordPolicyNamesCtrl->Enable(!enable);
  m_PasswordPolicyPasswordLengthText->Enable(enable);
  m_PasswordPolicyPasswordLengthCtrl->Enable(enable);
  EnableSizerChildren(m_PasswordPolicySizer, enable && !m_PasswordPolicyUseHexadecimalOnlyCtrl->GetValue());
  m_PasswordPolicyUseHexadecimalOnlyCtrl->Enable(enable);
  if (enable) {
    // Be more specific for character set controls
    m_PasswordPolicyLowerCaseMinCtrl->Enable(m_PasswordPolicyUseLowerCaseCtrl->GetValue());
    m_PasswordPolicyUpperCaseMinCtrl->Enable(m_PasswordPolicyUseUpperCaseCtrl->GetValue());
    m_PasswordPolicyDigitsMinCtrl->Enable(m_PasswordPolicyUseDigitsCtrl->GetValue());
    bool useSyms = m_PasswordPolicyUseSymbolsCtrl->GetValue();
    m_PasswordPolicyOwnSymbolsTextCtrl->Enable(useSyms);
    m_PasswordPolicySymbolsMinCtrl->Enable(useSyms);
    FindWindow(ID_RESET_SYMBOLS)->Enable(useSyms);
  }
}

struct newer {
  bool operator()(const PWHistEntry& first, const PWHistEntry& second) const {
    return first.changetttdate > second.changetttdate;
  }
};

void AddEditPropSheetDlg::SetupDCAComboBoxes(wxComboBox *pcbox, short &iDCA, bool isShift)
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
    m_Item.GetShiftDCA(dca);
  else
    m_Item.GetDCA(dca);

  bool useDefault = (dca < PWSprefs::minDCA || dca > PWSprefs::maxDCA);
  short defDCA =  short(PWSprefs::GetInstance()->
      GetPref(isShift ?
        PWSprefs::ShiftDoubleClickAction : PWSprefs::DoubleClickAction));
  // Following loop:
  // - sets CB's client data to pref value
  // - Adds " (default)" to default string
  // - Selects current value
  for (size_t i = 0; i < sizeof(dcaMapping)/sizeof(dcaMapping[0]); i++) {
    pcbox->SetClientData(static_cast<unsigned int>(i), reinterpret_cast<void *>(dcaMapping[i].pv));
    if (dcaMapping[i].pv == defDCA) {
      wxString dv = dcaMapping[i].name;
      dv += wxT(" ("); dv += _("default"); dv += wxT(")");
      pcbox->SetString(static_cast<unsigned int>(i), dv);
      if (useDefault || iDCA == defDCA) {
        pcbox->SetValue(dv);
      }
    }
    else if (iDCA == dcaMapping[i].pv)
      pcbox->SetValue(dcaMapping[i].name);
  }
}

// Build a string to describe the original expiry setting in the entry
wxString AddEditPropSheetDlg::makeExpiryString()
{
  wxString finished;
  wxDateTime dtX(m_OriginalDayttt);
  wxString dateStr = dtX.FormatISODate();  // Expiration date as YYYY-MM-DD
  int expDays = IntervalFromDate(dtX);     // Days until expiration

  // Specific date
  if (m_OriginalDayttt && !m_OriginalRecurring) {
    if (expDays > 0) {
      wxString str = (expDays == 1) ? _("Expires in %d day (%s)") : _("Expires in %d days (%s)") ;
      finished.Printf(str, expDays, dateStr);
    } else {
      finished.Printf(_("Expired on %s"), dateStr);
      m_DatesTimesCurrentCtrl->SetForegroundColour(*wxRED);
    }
  }

  // Recurring interval
  if (m_OriginalRecurring) {
    wxString str = (m_ExpirationTimeInterval == 1) ? _("Every %d day from last change")
                                                   : _("Every %d days from last change") ;

    finished.Printf(str, m_ExpirationTimeInterval);
    if (expDays > 0) {
      str.Printf(_("\n(Next expiration on %s)"), dateStr);
    } else {
      str.Printf(_("\n(Expired on %s)"), dateStr);
      m_DatesTimesCurrentCtrl->SetForegroundColour(*wxRED);
    }
    finished += str;
  }

  // Never expires
  if (finished.empty())
    finished = _("Never Expires");

  return finished;
}

// Called once to initialize the expiration controls
void AddEditPropSheetDlg::InitializeExpTimes()
{
  // From m_item to display
  time_t tttExpirationTime;
  m_Item.GetXTime(tttExpirationTime);
  m_Item.GetXTimeInt(m_ExpirationTimeInterval);
  m_OriginalDayttt = 0;
  m_FirstInClick = true;

  // Special case: Some entries, created with recent versions of pwsafe, might have
  // an interval but no date, which is interpreted as "Never".  We are going to ignore
  // the interval and use the user-set default.  If an expiry change is made, the entry will
  // be re-written correctly.
  int defaultInterval = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultExpiryDays);

  // Initialize these controls as disabled, they will be enabled as needed.
  m_DatesTimesExpiryDateCtrl->Disable();
  m_DatesTimesExpiryTimeCtrl->Disable();
  m_DatesTimesStaticTextDays->Disable();
  m_DatesTimesRecurringExpiryCtrl->Disable();

  wxDateTime expiryDate;
  if (tttExpirationTime == 0) { // never expires
    m_DatesTimesNeverExpireCtrl->SetValue(true);
    m_OriginalRecurring = false;
    m_Item.SetXTimeInt(0);  // Special case: No date, there should be no interval
    m_ExpirationTimeInterval = defaultInterval;
    expiryDate = TodayPlusInterval(m_ExpirationTimeInterval);
    m_OriginalButton = m_DatesTimesNeverExpireCtrl;

  } else {
    expiryDate = wxDateTime(tttExpirationTime).GetDateOnly();  // Remove time part
    m_OriginalDayttt = expiryDate.GetTicks();

    if (m_ExpirationTimeInterval == 0) { // expiration specified as date
      m_DatesTimesExpireOnCtrl->SetValue(true);
      m_DatesTimesExpiryDateCtrl->Enable();
      m_OriginalRecurring = false;

      // Set initierval to days until expiration
      // If it's already expired, use the default value
      m_ExpirationTimeInterval = IntervalFromDate(expiryDate);
      if (m_ExpirationTimeInterval <= 0)
        m_ExpirationTimeInterval = defaultInterval;

      m_OriginalButton = m_DatesTimesExpireOnCtrl;

    } else { // exp. specified as recurring interval
      m_DatesTimesExpireInCtrl->SetValue(true);
      m_DatesTimesExpiryTimeCtrl->Enable();
      m_DatesTimesStaticTextDays->Enable();
      m_DatesTimesRecurringExpiryCtrl->Enable();
      m_OriginalRecurring = true;
      m_FirstInClick = false;
      expiryDate = TodayPlusInterval(m_ExpirationTimeInterval);
      m_OriginalButton = m_DatesTimesExpireInCtrl;
    }
  }

  // The date picker controls on different platforms (i.e. Mac vs. GTK)
  // behave differently with respect to handling the time portion.  This
  // results in different values when converting to or from time_t and
  // false or missed change detections.
  // GTK seems to remove the time part, macOS preserves it.
  // Since we only care about the date for expiration, let's just
  // remove the time wherever we need the date.
  // Note the wxWidgets documentation says Today() returns the
  // time part set to 0, and Today() and Now() both use the local time zone.
  m_DatesTimesExpiryDateCtrl->SetValue(expiryDate);

  // Set the recurring checkbox default state.
  // The Recurring checkbox is only used if the user selects the interval radio button.
  m_Recurring = m_OriginalRecurring;

  // Build a string to describe the original setting in the entry
  m_OriginalExpirationStr = makeExpiryString();

  if (expiryDate > wxDateTime::Today())
    expiryDate = wxDateTime::Today(); // otherwise we can never move exp date back
  m_DatesTimesExpiryDateCtrl->SetRange(expiryDate, wxDateTime(time_t(-1)));
  m_DatesTimesExpiryTimeCtrl->SetRange(1, 3650);
}

void AddEditPropSheetDlg::ItemFieldsToPropSheet()
{
  std::vector<stringT> names;
  wxSize actSize = m_BasicGroupNamesCtrl->GetSize(), oldSize = actSize, borderSize = m_BasicGroupNamesCtrl->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord width, height, border = (borderSize.GetWidth() * 2) + 2;
  
  PWSprefs *prefs = PWSprefs::GetInstance();

  dc.SetFont(m_BasicGroupNamesCtrl->GetFont());
  
  // Populate the group combo box
  m_Core.GetAllGroups(names);
  
  m_BasicGroupNamesCtrl->Append(""); // Also allow selection of emtpy group
  for (auto const& name : names) {
    m_BasicGroupNamesCtrl->Append(name);
    dc.GetTextExtent(name, &width, &height);
    width += border;
    if(width > actSize.GetWidth()) actSize.SetWidth(width);
  }
  if(actSize.GetWidth() != oldSize.GetWidth()) {
    GetSize(&width, &height);
    width += actSize.GetWidth() - oldSize.GetWidth();
    int displayWidth, displayHight;
    ::wxDisplaySize(&displayWidth, &displayHight);
    if(width > displayWidth) width = displayWidth;
    SetSize(width, height);
  }

  // select relevant group
  const StringX group = (m_Type == SheetType::ADD ? tostringx(m_SelectedGroup): m_Item.GetGroup());
  if (!group.empty()) {
    auto position = m_BasicGroupNamesCtrl->FindString(stringx2std(group));
    if (position != wxNOT_FOUND) {
      m_BasicGroupNamesCtrl->SetSelection(position);
    }
    else {
      m_BasicGroupNamesCtrl->SetValue(m_SelectedGroup);
    }
  }
  m_Title = m_Item.GetTitle().c_str();
  m_User = m_Item.GetUser().c_str();
  m_Url = m_Item.GetURL().c_str();
  m_Email = m_Item.GetEmail().c_str();
  m_Password = m_Item.GetPassword();

  if (m_Item.IsAlias()) {
    // Update password to alias form
    // Show text stating that it is an alias
    ShowAlias();
  } // IsAlias
  else {
    m_BasicPasswordTextCtrl->ChangeValue(m_Password.c_str());
    if (prefs->GetPref(PWSprefs::ShowPWDefault)) {
      ShowPassword();
    } else {
      HidePassword();
    }
  }
  // Enable Go button iff m_url isn't empty
  wxWindow *goBtn = FindWindow(ID_GO_BTN);
  goBtn->Enable(!m_Url.empty());
  // Enable Send button iff m_email isn't empty
  wxWindow *sendBtn = FindWindow(ID_SEND_BTN);
#ifdef NOTYET
  sendBtn->Enable(!m_email.empty());
#endif
  // XXX since PWSRun not yet implemented in Linux, Send button's always disabled:
  sendBtn->Enable(false);
  m_Notes = (m_Type != SheetType::ADD && m_IsNotesHidden) ?
    wxString(_("[Notes hidden - click here to display]")) : towxstring(m_Item.GetNotes(TCHAR('\n')));
  // Following has no effect under Linux :-(
  long style = m_BasicNotesTextCtrl->GetExtraStyle();
  if (prefs->GetPref(PWSprefs::NotesWordWrap))
    style |= wxTE_WORDWRAP;
  else
    style &= ~wxTE_WORDWRAP;
  m_BasicNotesTextCtrl->SetExtraStyle(style);
  m_Autotype = m_Item.GetAutoType().c_str();
  m_RunCommand = m_Item.GetRunCommand().c_str();

  // double-click actions:
  m_Item.GetDCA(m_DoubleClickAction, false);
  m_Item.GetDCA(m_ShiftDoubleClickAction, true);
  SetupDCAComboBoxes(m_AdditionalDoubleClickActionCtrl, m_DoubleClickAction, false);
  SetupDCAComboBoxes(m_AdditionalShiftDoubleClickActionCtrl, m_ShiftDoubleClickAction, true);

  // History: If we're adding, use preferences, otherwise,
  // get values from m_item
  if (m_Type == SheetType::ADD) {
    // Get history preferences
    m_KeepPasswordHistory = prefs->GetPref(PWSprefs::SavePasswordHistory);
    m_MaxPasswordHistory = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
    
    // Get default user name preference
    if (prefs->GetPref(PWSprefs::UseDefaultUser)) {
      m_User = towxstring(prefs->GetPref(PWSprefs::DefaultUsername));
    }
  } else { // EDIT or VIEW
    const StringX pwh_str = m_Item.GetPWHistory();
    if (!pwh_str.empty()) {
      m_PasswordHistory = towxstring(pwh_str);

      PWHistList pwhl(pwh_str, PWSUtil::TMC_LOCALE);
      m_KeepPasswordHistory = pwhl.isSaving();

      if (size_t(m_AdditionalPasswordHistoryGrid->GetNumberRows()) < pwhl.size()) {
        m_AdditionalPasswordHistoryGrid->AppendRows(static_cast<int>(pwhl.size() - m_AdditionalPasswordHistoryGrid->GetNumberRows()));
      }
      m_MaxPasswordHistory = int(pwhl.getMax());
      //reverse-sort the history entries so that we list the newest first
      std::sort(pwhl.begin(), pwhl.end(), newer());
      int row = 0;
      for (PWHistList::iterator iter = pwhl.begin(); iter != pwhl.end();
           ++iter) {
        m_AdditionalPasswordHistoryGrid->SetCellValue(row, 0, iter->changedate.c_str());
        m_AdditionalPasswordHistoryGrid->SetCellValue(row, 1, iter->password.c_str());
        row++;
      }
    } else { // empty history string
      // Get history preferences
      m_KeepPasswordHistory = prefs->GetPref(PWSprefs::SavePasswordHistory);
      m_MaxPasswordHistory = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
    }
  } // m_type

  // Password Expiration
  InitializeExpTimes();
  // Modification times
  m_CreationTime = m_Item.GetCTimeL().c_str();
  m_ModificationTime = m_Item.GetPMTimeL().c_str();
  m_AccessTime = m_Item.GetATimeL().c_str();
  m_RMTime = m_Item.GetRMTimeL().c_str();

  // Password policy
  PWPolicy policy;
  // Populate the policy names combo box:
  m_PasswordPolicyNamesCtrl->Append(_("Default Policy"));
  m_Core.GetPolicyNames(names);
  for (auto const& name : names) {
    m_PasswordPolicyNamesCtrl->Append(name);
  }
  // Does item use a named policy or item-specific policy?
  bool namedPwPolicy = !m_Item.GetPolicyName().empty();
  UNREFERENCED_PARAMETER(namedPwPolicy); // Remove MS Compiler warning

  bool specificPwPolicy = !m_Item.GetPWPolicy().empty();
  ASSERT(!(namedPwPolicy && specificPwPolicy)); // both cannot be true!
  m_PasswordPolicyUseDatabaseCtrl->SetValue(!specificPwPolicy);

  if (specificPwPolicy) { /* item specific policy */
    m_Item.GetPWPolicy(policy);
    policy.symbols = m_Item.GetSymbols().c_str();
    if (!policy.symbols.empty()) {
      m_Symbols = policy.symbols.c_str();
    }
  }
  else if (namedPwPolicy) { /* named policy */
    const wxString itemPolName = m_Item.GetPolicyName().c_str();
    m_PasswordPolicyNamesCtrl->SetValue(itemPolName);
    m_Core.GetPolicyFromName(tostringx(itemPolName), policy);
  }
  else { /* default policy */
    m_PasswordPolicyNamesCtrl->SetValue(_("Default Policy"));
    policy = prefs->GetDefaultPolicy();
  }
  UpdatePWPolicyControls(policy);
  EnablePWPolicyControls(specificPwPolicy);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN
 */

void AddEditPropSheetDlg::OnGoButtonClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow() && !m_Url.IsEmpty())
    ::wxLaunchDefaultBrowser(m_Url, wxBROWSER_NEW_WINDOW);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_GENERATE
 */

void AddEditPropSheetDlg::OnGenerateButtonClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow() && !m_Item.IsAlias()) {
    PWPolicy pwp = GetSelectedPWPolicy();
    StringX password = pwp.MakeRandomPassword();
    if (password.empty()) {
      wxMessageBox(_("Couldn't generate password - invalid policy"),
                   _("Error"), wxOK|wxICON_INFORMATION, this);
      return;
    }

    Clipboard::GetInstance()->SetData(password);
    m_Password = password.c_str();
    m_BasicPasswordTextCtrl->ChangeValue(m_Password.c_str());
    if (m_IsPasswordHidden) {
      m_BasicPasswordConfirmationTextCtrl->ChangeValue(m_Password.c_str());
    }
  }
}

/*!
 * wxEVT_TEXT event handler for ID_TEXTCTRL_PASSWORD and ID_TEXTCTRL_PASSWORD2
 */

void AddEditPropSheetDlg::OnPasswordChanged(wxCommandEvent& event)
{
  UpdatePasswordConfirmationIcons();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SHOWHIDE
 */

void AddEditPropSheetDlg::OnShowHideClick(wxCommandEvent& WXUNUSED(evt))
{
  if(m_Item.IsAlias()) {
    if (m_IsPasswordHidden) {
      const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
      ASSERT(pbci);
      if (pbci) {
        m_IsPasswordHidden = false;
        UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordConfirmationTextCtrl, pbci->GetPassword().c_str(), m_BasicPasswordTextCtrl, wxTE_READONLY);
        m_BasicPasswordConfirmationTextCtrl->Enable(true);
        m_BasicPasswordConfirmationTextCtrl->SetModified(false);   // Reset the modification flag to indicate no changes made by the user.
                                                                   // See also 'UpdatePasswordConfirmationIcons'.
      }
    }
    else {
      m_IsPasswordHidden = true;
      UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordConfirmationTextCtrl, wxEmptyString, m_BasicPasswordTextCtrl, wxTE_READONLY);
      m_BasicPasswordConfirmationTextCtrl->Enable(false);
      m_BasicPasswordConfirmationTextCtrl->SetModified(false);     // Reset the modification flag to indicate no changes made by the user.
                                                                   // See also 'UpdatePasswordConfirmationIcons'.
    }
  }
  else {
    m_Password = m_BasicPasswordTextCtrl->GetValue().c_str(); // save visible password
    if (m_IsPasswordHidden) {
      ShowPassword();
      UpdatePasswordConfirmationIcons(false);     // Hide confirmation icons
      UpdatePasswordConfirmationAsterisk(false);  // Hide asterisk at password confirmation label, if only one password entry field is shown
    } else {
      HidePassword();
      UpdatePasswordConfirmationIcons(true);      // Show confirmation icons
      UpdatePasswordConfirmationAsterisk(true);   // Show asterisk at password confirmation label when two password entry fields are shown
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ALIAS
 */

void AddEditPropSheetDlg::OnAliasButtonClick(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&AddEditPropSheetDlg::DoAliasButtonClick);
}

void AddEditPropSheetDlg::DoAliasButtonClick()
{
  CItemData *pbci = (m_Item.IsAlias() ? m_Core.GetBaseEntry(&m_Item) : nullptr);
  
  if(m_Item.IsShortcutBase()) {
    wxMessageBox(_("On changing this entry of type Shortcut Base to an Alias all Shortcut to this entry will be removed!"), _("Warning"), wxOK | wxICON_EXCLAMATION);
  }
  
  
  int rc = ShowModalAndGetResult<SelectAliasDlg>(this, &m_Core, &m_Item, &pbci);
  if(rc == wxID_OK) {
    if(! m_Core.IsReadOnly()) {
      bool bChangeToBaseEntry = false;
      if(pbci && pbci->IsShortcut()) {
        CItemData *pbci_shortcut = m_Core.GetBaseEntry(pbci);
        if(pbci_shortcut) {
          pbci = pbci_shortcut;
          bChangeToBaseEntry = true;
        }
      } else if(pbci && pbci->IsAlias()) {
        CItemData *pbci_shortcut = m_Core.GetBaseEntry(pbci);
        if(pbci_shortcut) {
          pbci = pbci_shortcut;
          bChangeToBaseEntry = true;
        }
      } else if(pbci && pbci->IsShortcutBase()) {
        wxMessageBox(_("On changing this entry of type Shortcut Base to an Alias all Shortcut to this entry will be removed!"), _("Warning"), wxOK | wxICON_EXCLAMATION);
      }
      if(bChangeToBaseEntry) {
        wxMessageBox(_("Shortcut or Alias selected, use Base entry instead"), _("Warning"), wxOK | wxICON_EXCLAMATION);
      }
      if(m_Item.IsAlias() && (pbci == nullptr)) {
        m_Item.SetEntryType(CItemData::ET_NORMAL);
        m_Password = m_Item.GetPassword();
        RemoveAlias();
      }
      else if(m_Item.IsAlias() && (m_Core.GetBaseEntry(&m_Item) != pbci)) {
        const pws_os::CUUID baseUUID = pbci->GetUUID();
        m_Item.SetBaseUUID(baseUUID);
        m_Password = L"[" +
                    pbci->GetGroup() + L":" +
                    pbci->GetTitle() + L":" +
                    pbci->GetUser()  + L"]";
        m_BasicPasswordTextCtrl->SetValue(m_Password.c_str());
        if(! m_IsPasswordHidden)
          m_BasicPasswordConfirmationTextCtrl->SetValue(pbci->GetPassword().c_str());
      }
      else if(! m_Item.IsAlias() && pbci) {
        const pws_os::CUUID baseUUID = pbci->GetUUID();
        m_Item.SetAlias();
        m_Item.SetBaseUUID(baseUUID);
        m_Password = L"[" +
                    pbci->GetGroup() + L":" +
                    pbci->GetTitle() + L":" +
                    pbci->GetUser()  + L"]";
        ShowAlias();
      }
    }
  }
}

void AddEditPropSheetDlg::ShowPassword()
{
  m_IsPasswordHidden = false;
  UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordTextCtrl, m_Password.c_str(), m_BasicUsernameTextCtrl, 0);
  // Disable confirmation Ctrl, as the user can see the password entered
  m_BasicPasswordConfirmationTextCtrl->ChangeValue(wxEmptyString); // Use of ChangeValue instead of SetValue to not trigger an input event.
  m_BasicPasswordConfirmationTextCtrl->Enable(false);
  m_BasicPasswordConfirmationTextCtrl->SetModified(false);         // Reset of modification flag for password confirmation icon handling.
  m_BasicShowHideCtrl->SetBitmapLabel(wxUtilities::GetBitmapResource(wxT("graphics/eye_close.xpm")));
  m_BasicShowHideCtrl->SetToolTip(_("Hide password"));
}

void AddEditPropSheetDlg::HidePassword()
{
  m_IsPasswordHidden = true;
  const wxString pwd = m_Password.c_str();
  UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordTextCtrl, pwd, m_BasicUsernameTextCtrl, wxTE_PASSWORD);
  m_BasicPasswordConfirmationTextCtrl->ChangeValue(pwd);           // Use of ChangeValue instead of SetValue to not trigger an input event.
  m_BasicPasswordConfirmationTextCtrl->Enable(true);
  m_BasicPasswordConfirmationTextCtrl->SetModified(false);         // Reset of modification flag for password confirmation icon handling.
  m_BasicShowHideCtrl->SetBitmapLabel(wxUtilities::GetBitmapResource(wxT("graphics/eye.xpm")));
  m_BasicShowHideCtrl->SetToolTip(_("Show password"));
}

void AddEditPropSheetDlg::UpdatePasswordConfirmationIcons(bool show)
{
  // There is nothing to do if there is no user input, but the content of
  // the password input fields may have been changed by the hide/show functionality.
  if (!m_BasicPasswordTextCtrl->IsModified() && !m_BasicPasswordConfirmationTextCtrl->IsModified()) {
    return;
  }
  // If both passwords entered are the same, the green checkmark icons will appear to indicate the match.
  if (m_BasicPasswordTextCtrl->GetValue() == m_BasicPasswordConfirmationTextCtrl->GetValue()) {
    m_BasicPasswordBitmap->SetBitmap(bitmapCheckmarkGreen);
    m_BasicPasswordConfirmationBitmap->SetBitmap(bitmapCheckmarkGreen);
  }
  // The gray checkmark icons will be shown to indicate that some input is given that do not match.
  else {
    m_BasicPasswordBitmap->SetBitmap(bitmapCheckmarkGray);
    m_BasicPasswordConfirmationBitmap->SetBitmap(bitmapCheckmarkGray);
  }
  // Only display the check mark symbol to the right of each input field when there is some input.
  if (show) {
    m_BasicPasswordBitmap->Show(!m_BasicPasswordTextCtrl->IsEmpty());
    m_BasicPasswordConfirmationBitmap->Show(!m_BasicPasswordConfirmationTextCtrl->IsEmpty());
  }
  // Show empty icons to mimic hidden icons, avoiding layout issues with text input fields.
  else {
    m_BasicPasswordBitmap->SetBitmap(bitmapCheckmarkPlaceholder);
    m_BasicPasswordBitmap->Show();
    m_BasicPasswordConfirmationBitmap->SetBitmap(bitmapCheckmarkPlaceholder);
    m_BasicPasswordConfirmationBitmap->Show();
  }
}

void AddEditPropSheetDlg::UpdatePasswordConfirmationAsterisk(bool show)
{
  FindWindow(ID_STATICTEXT_PASSWORD2)->Show(show);
}

void AddEditPropSheetDlg::ShowAlias()
{
  wxASSERT(m_Item.IsAlias());
  
  const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
  ASSERT(pbci);
  if (pbci) {
    m_Password = L"[" +
                pbci->GetGroup() + L":" +
                pbci->GetTitle() + L":" +
                pbci->GetUser()  + L"]";
  }
  m_BasicPasswordTextLabel->SetLabel(_("Alias:"));
  UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordTextCtrl, m_Password.c_str(), m_BasicUsernameTextCtrl, wxTE_READONLY);
  
  m_BasicPasswordConfirmationTextLabel->SetLabel(_("Password:"));
  if (pbci && PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWDefault)) {
    m_IsPasswordHidden = false;
    const wxString pwd = pbci->GetPassword().c_str();
    UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordConfirmationTextCtrl, pwd, m_BasicPasswordTextCtrl, wxTE_READONLY);
    ApplyFontPreference(m_BasicPasswordConfirmationTextCtrl, PWSprefs::StringPrefs::PasswordFont);
    m_BasicPasswordConfirmationTextCtrl->ChangeValue(pwd);
    m_BasicPasswordConfirmationTextCtrl->Enable(true);
  }
  else {
    m_IsPasswordHidden = true;
    UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordConfirmationTextCtrl, wxEmptyString, m_BasicPasswordTextCtrl, wxTE_READONLY);
    ApplyFontPreference(m_BasicPasswordConfirmationTextCtrl, PWSprefs::StringPrefs::PasswordFont);
    m_BasicPasswordConfirmationTextCtrl->Clear();
    m_BasicPasswordConfirmationTextCtrl->Enable(false);
  }
  FindWindow(ID_BUTTON_ALIAS)->Show();
}



void AddEditPropSheetDlg::RemoveAlias()
{
  wxASSERT(!m_Item.IsAlias());
  
  m_BasicPasswordTextLabel->SetLabel(_("Password:"));
  m_BasicPasswordConfirmationTextLabel->SetLabel(_("Confirm:"));
  
  const wxString pwd = m_Password.c_str();
  
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWDefault)) {
    m_IsPasswordHidden = false;
    UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordTextCtrl, pwd, m_BasicUsernameTextCtrl, 0);
    UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordConfirmationTextCtrl, pwd, m_BasicPasswordTextCtrl, wxTE_PASSWORD);
    m_BasicPasswordConfirmationTextCtrl->Clear();
    m_BasicPasswordConfirmationTextCtrl->Enable(false);
  }
  else {
    m_IsPasswordHidden = true;
    UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordTextCtrl, pwd, m_BasicUsernameTextCtrl, wxTE_PASSWORD);
    UpdatePasswordTextCtrl(m_BasicSizer, m_BasicPasswordConfirmationTextCtrl, pwd, m_BasicPasswordTextCtrl, wxTE_PASSWORD);
    m_BasicPasswordConfirmationTextCtrl->ChangeValue(pwd);
    m_BasicPasswordConfirmationTextCtrl->Enable(true);
  }
}

static short GetSelectedDCA(const wxComboBox *pcbox, short defval)
{
  int sel = pcbox->GetSelection();
  if (sel == wxNOT_FOUND) { // no selection
    return -1;
  } else {
    auto ival = reinterpret_cast<intptr_t>(pcbox->GetClientData(sel));
    return (ival == defval) ? -1 : ival;
  }
}

bool AddEditPropSheetDlg::ValidateBasicData()
{
  const StringX password = tostringx(m_BasicPasswordTextCtrl->GetValue());

  if (m_Title.IsEmpty() || password.empty()) {
    GetBookCtrl()->SetSelection(0);

    if (m_Title.IsEmpty()) {
      FindWindow(ID_TEXTCTRL_TITLE)->SetFocus();
    }
    else {
      m_BasicPasswordTextCtrl->SetFocus();
    }

    wxMessageBox(
      wxString::Format(
        wxString(_("This entry must have a %ls")),
        (m_Title.IsEmpty() ? _("title"): _("password"))),
      _("Error"), wxOK|wxICON_INFORMATION, this
    );

    return false;
  }

  if (m_IsPasswordHidden && !m_Item.IsAlias()) { // hidden passwords - compare both values
    const StringX secondPassword = tostringx(m_BasicPasswordConfirmationTextCtrl->GetValue());

    if (password != secondPassword) {
      wxMessageDialog msg(
        this,
        _("Passwords do not match."),
        _("Mismatching passwords"),
        wxOK|wxICON_ERROR
      );
      msg.ShowModal();

      return false;
    }
  }

  if (m_Type == SheetType::ADD) {
    return IsGroupUsernameTitleCombinationUnique();
  }

  return true;
}

bool AddEditPropSheetDlg::ValidatePasswordPolicy()
{
  if (m_PasswordPolicyUseDatabaseCtrl->GetValue() && (m_PasswordPolicyNamesCtrl->GetValue().IsEmpty())) {
    wxMessageDialog msg(
      this,
      _("Database name must not be empty if a database policy shall be used."),
      _("Error"),
      wxOK|wxICON_ERROR
    );
    msg.ShowModal();
    return false;
  }
  if(! m_PasswordPolicyUseDatabaseCtrl->GetValue() && ! CheckPWPolicyFromUI()) {
    return false;
  }

  return true;
}

bool AddEditPropSheetDlg::IsGroupUsernameTitleCombinationUnique()
{
  // Check for Group/Username/Title uniqueness
  auto listindex = m_Core.Find(m_Item.GetGroup(), m_Item.GetTitle(), m_Item.GetUser());
  if (listindex != m_Core.GetEntryEndIter()) {
    auto listItem = m_Core.GetEntry(listindex);
    if (listItem.GetUUID() != m_Item.GetUUID()) {
      wxMessageDialog msg(
        this,
        _("An entry or shortcut with the same Group, Title and Username already exists."),
        _("Duplicate entry"),
        wxOK|wxICON_ERROR
      );
      msg.ShowModal();

      return false;
    }
  }

  return true;
}


Command* AddEditPropSheetDlg::NewAddEntryCommand(bool bNewCTime)
{
  time_t t;
  const wxString group = m_BasicGroupNamesCtrl->GetValue();
  const StringX password = tostringx(m_BasicPasswordTextCtrl->GetValue());

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Basic"
  /////////////////////////////////////////////////////////////////////////////

  m_Item.SetGroup(tostringx(group));
  m_Item.SetTitle(tostringx(m_Title));
  m_Item.SetUser(m_User.empty() ?
                 PWSprefs::GetInstance()->
                   GetPref(PWSprefs::DefaultUsername).c_str() : m_User.c_str());

  m_Item.SetNotes(tostringx(m_Notes));
  m_Item.SetURL(tostringx(m_Url));
  m_Item.SetEmail(tostringx(m_Email));
  if(! m_Item.IsAlias())
    m_Item.SetPassword(password);
  else
    m_Item.SetPassword(L"");

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Additional"
  /////////////////////////////////////////////////////////////////////////////

  m_Item.SetAutoType(tostringx(m_Autotype));
  m_Item.SetRunCommand(tostringx(m_RunCommand));
  m_Item.SetDCA(m_DoubleClickAction);
  m_Item.SetShiftDCA(m_ShiftDoubleClickAction);

  if(bNewCTime) {
    time(&t);
    m_Item.SetCTime(t);
  }
  wxASSERT(m_Item.IsCreationTimeSet());
  if (m_KeepPasswordHistory) {
    m_Item.SetPWHistory(PWHistList::MakePWHistoryHeader(true, m_MaxPasswordHistory));
  }

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Dates and Times"
  /////////////////////////////////////////////////////////////////////////////


  if (m_Item.IsAlias()) {
    m_Item.SetXTime(time_t(0));
    m_Item.SetXTimeInt(time_t(0));

  } else if (!m_DatesTimesNeverExpireCtrl->GetValue()) {
    m_Item.SetXTime(NormalizeExpDate(m_DatesTimesExpiryDateCtrl->GetValue()).GetTicks());
    if (m_DatesTimesExpireInCtrl->GetValue() && m_Recurring) {
      m_Item.SetXTimeInt(m_ExpirationTimeInterval);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password Policy"
  /////////////////////////////////////////////////////////////////////////////

  if (!m_PasswordPolicyUseDatabaseCtrl->GetValue()) {
    m_Item.SetPWPolicy(GetPWPolicyFromUI());
  }

  if (m_Item.IsAlias()) {
    m_Item.SetPWPolicy(wxEmptyString);
  }
  // Alias is added in AddEntryCommand

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Attachment"
  /////////////////////////////////////////////////////////////////////////////

  if (m_Core.GetReadFileVersion() == PWSfile::V40) {

    /*
      Case: New item shall get an attachment.
      Steps:
        1) Update attachment meta data.
    */
    if (!m_Item.HasAttRef() && m_ItemAttachment.HasUUID() && m_ItemAttachment.HasContent()) {

      // Step 1)
      if (m_AttachmentTitle->GetValue() != _T("N/A")) {
        m_ItemAttachment.SetTitle(std2stringx(stringT(m_AttachmentTitle->GetValue())));
      }

      time_t timestamp;
      time(&timestamp);
      m_ItemAttachment.SetCTime(timestamp);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Create Command
  /////////////////////////////////////////////////////////////////////////////

  m_Item.SetStatus(CItemData::ES_ADDED);

  auto *commands = MultiCommands::Create(&m_Core);
  auto itemGroup = m_Item.GetGroup();

  if (m_Core.IsEmptyGroup(itemGroup)) { // The group is no longer empty if a new item is added
    commands->Add(
      DBEmptyGroupsCommand::Create(&m_Core, itemGroup, DBEmptyGroupsCommand::EG_DELETE)
    );
  }

  if(m_Item.IsAlias()) { // If alias is pointing to shortcut base the shortcuts must be removed before converting to alias base
    const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
    ASSERT(pbci);
    if (pbci && pbci->IsShortcutBase()) {
      // Delete shortcuts of shortcut base
      UUIDVector tlist;
      m_Core.GetAllDependentEntries(pbci->GetUUID(), tlist, CItemData::ET_SHORTCUT);
      for (size_t idep = 0; idep < tlist.size(); idep++) {
        ItemListIter shortcut_iter = m_Core.Find(tlist[idep]);
        commands->Add(
          DeleteEntryCommand::Create(&m_Core, shortcut_iter->second)
        );
      }
    }
  }

  commands->Add(
    AddEntryCommand::Create(
      &m_Core, m_Item, m_Item.GetBaseUUID(),
      (m_ItemAttachment.HasUUID() && m_ItemAttachment.HasContent()) ? &m_ItemAttachment : nullptr
    )
  );
  return commands;
}

uint32_t AddEditPropSheetDlg::GetChanges() const
{
  const PWSprefs *prefs = PWSprefs::GetInstance();

  uint32_t changes = Changes::None;
  if (tostringx(m_BasicGroupNamesCtrl->GetValue()) != m_Item.GetGroup()) {
    changes |= Changes::Group;
  }
  if (tostringx(m_Title) != m_Item.GetTitle()) {
    changes |= Changes::Title;
  }
  if (tostringx(m_User) != m_Item.GetUser()) {
    changes |= Changes::User;
  }
  // Following ensures that untouched & hidden note
  // isn't marked as modified. Relies on fact that
  // Note field can't be modified w/o first getting focus
  // and that we turn off m_isNotesHidden when that happens.
  if ((m_Type == SheetType::ADD || !m_IsNotesHidden) && tostringx(m_Notes) != m_Item.GetNotes(TCHAR('\n'))) {
    changes |= Changes::Notes;
  }
  if (tostringx(m_Url) != m_Item.GetURL()) {
    changes |= Changes::Url;
  }
  if (tostringx(m_Email) != m_Item.GetEmail()) {
    changes |= Changes::Email;
  }
  if (tostringx(m_Autotype) != m_Item.GetAutoType()) {
    changes |= Changes::Autotype;
  }
  if (tostringx(m_RunCommand) != m_Item.GetRunCommand()) {
    changes |= Changes::RunCommand;
  }
  // Prepare a string from the dialog and make sure the current item is sorted the same way
  if (PreparePasswordHistory() != (StringX)(PWHistList(m_Item.GetPWHistory(), PWSUtil::TMC_LOCALE)) &&
      !(m_Item.GetPWHistory().empty() && m_PasswordHistory.empty() && static_cast<unsigned int>(m_MaxPasswordHistory) == prefs->GetPref(PWSprefs::NumPWHistoryDefault) && m_KeepPasswordHistory == prefs->GetPref(PWSprefs::SavePasswordHistory))
  ) {
    changes |= Changes::History;
  }
  // symbols
  {
    const auto oldSymbols = m_Item.GetSymbols();
    if (tostringx(m_Symbols) != oldSymbols && (!oldSymbols.empty() || m_Symbols != CPasswordCharPool::GetDefaultSymbols())) {
      changes |= Changes::Symbols;
    }
  }
  {
    short lastDCA;
    m_Item.GetDCA(lastDCA);
    const auto def = short(prefs->GetPref(PWSprefs::DoubleClickAction));
    const auto selected = GetSelectedDCA(m_AdditionalDoubleClickActionCtrl, def);
    if (lastDCA != selected && lastDCA != def) {
      changes |= Changes::DCA;
    }
  }
  {
    short lastShiftDCA;
    m_Item.GetShiftDCA(lastShiftDCA);
    const auto def = short(prefs->GetPref(PWSprefs::ShiftDoubleClickAction));
    const auto selected = GetSelectedDCA(m_AdditionalShiftDoubleClickActionCtrl, def);
    if (lastShiftDCA != selected && lastShiftDCA != def) {
      changes |= Changes::ShiftDCA;
    }
  }
  {
    int lastXTimeInt;
    time_t lastXtime;
    m_Item.GetXTime(lastXtime);
    m_Item.GetXTimeInt(lastXTimeInt);

    time_t newExpirationDate = m_DatesTimesExpiryDateCtrl->GetValue().GetDateOnly().GetTicks();
    if ( m_DatesTimesExpireOnCtrl->GetValue() && ((m_OriginalButton != m_DatesTimesExpireOnCtrl)
                                                  || (newExpirationDate != m_OriginalDayttt)) ) {
      changes |= Changes::XTime;
    }

    if ( m_DatesTimesExpireInCtrl->GetValue() && ((m_OriginalButton != m_DatesTimesExpireInCtrl)
                                                  || (m_ExpirationTimeInterval != lastXTimeInt)
                                                  || (m_Recurring != m_OriginalRecurring)) ) {
      changes |= Changes::XTimeInt;
    }

    if (m_DatesTimesNeverExpireCtrl->GetValue() && (m_OriginalButton != m_DatesTimesNeverExpireCtrl)) {
      changes |= Changes::XTimeNever;
    }
  }
  // password
  {
    const StringX password = tostringx(m_BasicPasswordTextCtrl->GetValue());
    if (!m_Item.IsAlias()) {
      if (password != m_Item.GetPassword()) {
        changes |= Changes::Password;
      }
    }
    else {
      // Update password to alias form
      // Show text stating that it is an alias
      const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
      ASSERT(pbci);
      if (pbci) {
        const StringX alias = L"[" + pbci->GetGroup() + L":" + pbci->GetTitle() + L":" + pbci->GetUser()  + L"]";
        if (password != alias) {
          changes |= Changes::Password;
        }
      }
      else {
        changes |= Changes::Password;
      }
    }
  }
  // policy options string (symbol list is checked above)
  {
    StringX oldPWP;
    // get item's effective policy:
    const StringX oldPolName = m_Item.GetPolicyName();
    if (oldPolName.empty()) { // either item-specific or default:
      if (m_Item.GetPWPolicy().empty()) {
        oldPWP = PWSprefs::GetInstance()->GetDefaultPolicy();
      }
      else {
        oldPWP = m_Item.GetPWPolicy();
      }
    }
    else {
      PWPolicy pol;
      m_Core.GetPolicyFromName(oldPolName, pol);
      oldPWP = pol;  // convert to StringX
    }

    // Now check with dbox's effective policy:
    // If using a defined policy but the name is empty, it will cause
    // an assertion in GetSelectedPWPolicy() - GetPolicyFromName().
    // This should only happen if an incomplete policy edit is canceled.
    if (   (m_PasswordPolicyUseDatabaseCtrl->GetValue() && m_PasswordPolicyNamesCtrl->GetValue().empty())
        || (oldPWP != StringX(GetSelectedPWPolicy()))
    ) {
      changes |= Changes::Policy;
    }
  }
  // attachment
  {
    if (m_Core.GetReadFileVersion() == PWSfile::V40) {
      if (!m_Item.HasAttRef() && !m_ItemAttachment.HasUUID() && !m_ItemAttachment.HasContent()) {
        // no old & new attachment
      }
      else if ((!m_Item.HasAttRef() && m_ItemAttachment.HasUUID() && m_ItemAttachment.HasContent()) // added
          || (m_Item.HasAttRef() && !m_ItemAttachment.HasUUID() && !m_ItemAttachment.HasContent()) // deleted
      ) {
        changes |= Changes::Attachment;
      }
      else if (m_Item.HasAttRef()) { // changed ?
        if (m_ItemAttachment.GetTitle() != tostringx(m_AttachmentTitle->GetValue())) {
          changes |= Changes::Attachment;
        }
        else {
          auto uuid = m_Item.GetAttUUID();
          auto itemAttachment = m_Core.GetAtt(uuid);
          if (m_ItemAttachment != itemAttachment || m_ItemAttachment.GetUUID() != itemAttachment.GetUUID()) {
            changes |= Changes::Attachment;
          }
        }
      }
    }
  }
  return changes;
}

StringX AddEditPropSheetDlg::PreparePasswordHistory() const
{
  // Create a new PWHistory string based on settings in this dialog.
  // Note that we are not erasing the history here, even if the user has chosen to not
  // track PWHistory.  So there could be some password entries in the history
  // but the first byte could be zero, meaning we are not tracking it _FROM_NOW_.
  // Clearing the history is something the user must do himself with the "Clear History" button

  // First, Get a list of all password history entries
  PWHistList pwhl(tostringx(m_PasswordHistory), PWSUtil::TMC_LOCALE);

  // Encode the list into the proper StringX format, trim if necessarry
  pwhl.setMax(m_MaxPasswordHistory);
  pwhl.setSaving(m_KeepPasswordHistory);
  return pwhl;
}

Command* AddEditPropSheetDlg::NewEditEntryCommand()
{
  const auto changes = GetChanges();
  const PWSprefs *prefs = PWSprefs::GetInstance();

  if (changes & Changes::Group) {
    m_Item.SetGroup(tostringx(m_BasicGroupNamesCtrl->GetValue()));
  }
  if (changes & Changes::Title) {
    m_Item.SetTitle(tostringx(m_Title));
  }
  if (changes & Changes::User) {
    m_Item.SetUser(m_User.empty() ?
                   PWSprefs::GetInstance()->
                     GetPref(PWSprefs::DefaultUsername).c_str() : m_User.c_str());
  }
  if (changes & Changes::Notes) {
    m_Item.SetNotes(tostringx(m_Notes));
  }
  if (changes & Changes::Url) {
    m_Item.SetURL(tostringx(m_Url));
  }
  if (changes & Changes::Email) {
    m_Item.SetEmail(tostringx(m_Email));
  }
  if (changes & Changes::Autotype) {
    m_Item.SetAutoType(tostringx(m_Autotype));
  }
  if (changes & Changes::RunCommand) {
    m_Item.SetRunCommand(tostringx(m_RunCommand));
  }
  if (changes & Changes::History) {
    m_PasswordHistory = towxstring(PreparePasswordHistory());
    m_Item.SetPWHistory(tostringx(m_PasswordHistory));
  }
  if (changes & Changes::Symbols) {
    m_Item.SetSymbols(tostringx(m_Symbols));
  }
  if (changes & Changes::Policy) {
    if (m_PasswordPolicyUseDatabaseCtrl->GetValue()) {
      // User has selected to use a named policy
      wxString polName = m_PasswordPolicyNamesCtrl->GetValue();

      // The default policy is neither item specific nor stored in the database
      if (polName == _("Default Policy")) {
        // Remove database policy information from item
        m_Item.SetPolicyName(tostringx(wxEmptyString));

        // Remove item specific policy information from item
        m_Item.SetPWPolicy(tostringx(wxEmptyString));
      }
      // If it is not the default policy than it's a named policy from the database
      else {
        // Use policy that is stored in the database
        m_Item.SetPolicyName(tostringx(polName));

        // Remove item specific policy information from item
        m_Item.SetPWPolicy(tostringx(wxEmptyString));
      }
    }
    // User has selected to use an item specific policy
    else {
      // Use the data of the item specific policy collected from the UI
      m_Item.SetPWPolicy(GetSelectedPWPolicy());

      // Remove database policy information from item
      m_Item.SetPolicyName(tostringx(wxEmptyString));
    }
  }
  if (changes & Changes::DCA) {
    m_DoubleClickAction = GetSelectedDCA(m_AdditionalDoubleClickActionCtrl, short(prefs->GetPref(PWSprefs::DoubleClickAction)));
    m_Item.SetDCA(m_DoubleClickAction);
  }
  if (changes & Changes::ShiftDCA) {
    m_ShiftDoubleClickAction = GetSelectedDCA(m_AdditionalShiftDoubleClickActionCtrl, short(prefs->GetPref(PWSprefs::ShiftDoubleClickAction)));
    m_Item.SetShiftDCA(m_ShiftDoubleClickAction);
  }
  
  if (changes != Changes::None && !IsGroupUsernameTitleCombinationUnique()) {
    return nullptr;
  }

  time_t t;
  time(&t);
  if (changes & Changes::Password) {
    m_Item.UpdatePassword(tostringx(m_BasicPasswordTextCtrl->GetValue()));
    m_Item.SetPMTime(t);
  }
  if ((changes & ~Changes::Attachment) != Changes::None) { // anything besides attachment
    m_Item.SetRMTime(t);
    m_Item.SetStatus(CItemData::ES_MODIFIED);
  }
  
  // Setting a specific date
  if (changes & Changes::XTime) {
    m_Item.SetXTime(NormalizeExpDate(m_DatesTimesExpiryDateCtrl->GetValue()).GetTicks());
    m_Item.SetXTimeInt(0);
  }

  // Setting by interval
  // The date control should already be correct.  Only save the interval value if recurring is set
  if (changes & Changes::XTimeInt) {
    m_Item.SetXTime(NormalizeExpDate(m_DatesTimesExpiryDateCtrl->GetValue()).GetTicks());
    if (m_Recurring) {
      m_Item.SetXTimeInt(m_ExpirationTimeInterval);
    } else {
      m_Item.SetXTimeInt(0);
    }
  }

  // Never expire, zeros are not written to the file
  if (changes & Changes::XTimeNever) {
    m_Item.SetXTime(0);
    m_Item.SetXTimeInt(0);
  }

  auto commands = MultiCommands::Create(&m_Core);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Attachment"
  /////////////////////////////////////////////////////////////////////////////

  if (changes & Changes::Attachment) {
    /*
      Case: Item doesn't have an attachment and doesn't get one.
      Steps:
        - none
    */
    if (!m_Item.HasAttRef() && !m_ItemAttachment.HasUUID() && !m_ItemAttachment.HasContent()) {
      ; // Nothing to do. - Phew!
    }

    /*
      Case: Item doesn't have an attachment and shall get one.
      Steps:
        1) Update attachment meta data.
        2) Create DeleteEntryCommand to remove the item without attachment reference.
        3) Create AddEntryCommand for item and attachment.
           Only AddEntryCommand will associate an attachment item with a password item.
           The "delete existing - create new" approach also takes into account any changes
           to the password element that all other commands cannot.

           TODO: Check Undo if password item and attachment item have changed.
                 A command like EditEntryCommand would handle this proparly,
                 but unfortunately doesn't support attaching an attachment.
    */
    else if (!m_Item.HasAttRef() && m_ItemAttachment.HasUUID() && m_ItemAttachment.HasContent()) {

      // Step 1)
      if (m_AttachmentTitle->GetValue() != _T("N/A")) {
        m_ItemAttachment.SetTitle(std2stringx(stringT(m_AttachmentTitle->GetValue())));
      }

      time_t timestamp;
      time(&timestamp);
      m_ItemAttachment.SetCTime(timestamp);

      // Step 2)
      if(m_Item.IsAlias()) {
        commands->Add(RemoveDependentEntryCommand::Create(&m_Core,
                                                          m_Item.GetBaseUUID(),
                                                          m_Item.GetUUID(),
                                                          CItemData::ET_ALIAS));
      }
      commands->Add(DeleteEntryCommand::Create(&m_Core, m_Item));

      // Step 3)
      commands->Add(NewAddEntryCommand(false)); // Do not create a new creation time (C Time), as the old entry is replaced

      // If additional changes were made to the password element,
      // these are also taken into account by NewAddEntryCommand,
      // so that we can return with this command.
      return commands;
    }

    /*
      Case: Item has an attachment which shall be removed.
      Steps:
        1) Create DeleteAttachmentCommand command.
        2) Remove reference to attachment item.
    */
    else if (m_Item.HasAttRef() && !m_ItemAttachment.HasUUID() && !m_ItemAttachment.HasContent()) {

      // Step 1)
      commands->Add(DeleteAttachmentCommand::Create(&m_Core, m_Item));

      // Step 2)
      m_Item.ClearAttUUID();
    }

    /*
      Case: Item has an attachment which shall be replaced by a new one.
      Steps:
        1) Update attachment meta data.
        2) Create EditAttachmentCommand command to update attachment
    */
    else if (m_Item.HasAttRef()) {
      auto uuid = m_Item.GetAttUUID();
      auto itemAttachment = m_Core.GetAtt(uuid);

      auto hasTitleChanges = (towxstring(m_ItemAttachment.GetTitle()) != m_AttachmentTitle->GetValue());
      auto hasAttachmentChanges = (m_ItemAttachment != itemAttachment);
      
      if (hasTitleChanges || hasAttachmentChanges) {
        // Step 1)
        if (m_AttachmentTitle->GetValue() != _T("N/A"))
        {
          m_ItemAttachment.SetTitle(std2stringx(stringT(m_AttachmentTitle->GetValue())));
        }
        time_t timestamp;
        time(&timestamp);
        m_ItemAttachment.SetCTime(timestamp);
      }
      
      if(m_ItemAttachment.GetUUID() != itemAttachment.GetUUID()) {
        // The content has changed at all, remove and add again with new content

        // Step 2)
        if(m_Item.IsAlias()) {
          commands->Add(RemoveDependentEntryCommand::Create(&m_Core,
                                                            m_Item.GetBaseUUID(),
                                                            m_Item.GetUUID(),
                                                            CItemData::ET_ALIAS));
        }
        commands->Add(DeleteEntryCommand::Create(&m_Core, m_Item));

        // Step 3)
        commands->Add(NewAddEntryCommand(false)); // Do not create a new creation time (C Time), as the old entry is replaced

        // If additional changes were made to the password element,
        // these are also taken into account by NewAddEntryCommand,
        // so that we can return with this command.
        return commands;
      }
      else if (hasTitleChanges || hasAttachmentChanges) {
        // Step 2)
        commands->Add(
          EditAttachmentCommand::Create(&m_Core, itemAttachment, m_ItemAttachment)
        );

        // Note:
        // The item might also have modifications,
        // so we still do not return with the commands we have so far.
      }
    }
    else {
      ;
    }
  }

  if (changes != Changes::None) {
    // All fields in m_item now reflect user's edits
    // Let's update the core's data
    uuid_array_t uuid;
    m_Item.GetUUID(uuid);
    auto listpos = m_Core.Find(uuid);
    ASSERT(listpos != m_Core.GetEntryEndIter());

    if (listpos != m_Core.GetEntryEndIter()) {
      bool bTemporaryChangeOfPWH(false);
      CItemData &origItem = m_Core.GetEntry(listpos);
      StringX sxPWH = origItem.GetPWHistory();
      
      if(m_Item.IsAlias()) { // If alias is pointing to shortcut base the shortcuts must be removed before converting to alias base
        const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
        ASSERT(pbci);
        if (pbci && pbci->IsShortcutBase()) {
          // Delete shortcuts of shortcut base
          UUIDVector tlist;
          m_Core.GetAllDependentEntries(pbci->GetUUID(), tlist, CItemData::ET_SHORTCUT);
          for (size_t idep = 0; idep < tlist.size(); idep++) {
            ItemListIter shortcut_iter = m_Core.Find(tlist[idep]);
            commands->Add(
              DeleteEntryCommand::Create(&m_Core, shortcut_iter->second)
            );
          }
        }
      }

      if(origItem.IsNormal() && m_Item.IsAlias()) { // Change fron Normal entry to Alias
        commands->Add(
          AddDependentEntryCommand::Create(&m_Core, m_Item.GetBaseUUID(), origItem.GetUUID(), CItemData::ET_ALIAS)
        );
      }
      else if(origItem.IsAlias() && m_Item.IsNormal()) { // No longer an alias
        commands->Add(
          RemoveDependentEntryCommand::Create(&m_Core, origItem.GetBaseUUID(), origItem.GetUUID(), CItemData::ET_ALIAS)
        );
        // Temporarily disable password history so it doesn't have the special
        // password of [Alias] saved into it on reverting to normal
        if (!sxPWH.empty() && sxPWH.substr(0, 1) == L"1") {
          bTemporaryChangeOfPWH = true;
          sxPWH[0] = L'0';
          m_Item.SetPWHistory(sxPWH);
        }
      }
      else if(origItem.IsAlias() && m_Item.IsAlias() && (origItem.GetBaseUUID() != m_Item.GetBaseUUID())) { // Change Alias Base
        commands->Add(
          RemoveDependentEntryCommand::Create(&m_Core, origItem.GetBaseUUID(), origItem.GetUUID(), CItemData::ET_ALIAS)
        );
        commands->Add(
          AddDependentEntryCommand::Create(&m_Core, m_Item.GetBaseUUID(), origItem.GetUUID(), CItemData::ET_ALIAS)
        );
        
      }
      else if(origItem.IsAliasBase() && m_Item.IsAlias()) { // Change from AliasBase to Alias
        commands->Add(
          AddDependentEntryCommand::Create(&m_Core, m_Item.GetBaseUUID(), origItem.GetUUID(), CItemData::ET_ALIAS)
        );
        commands->Add(
          MoveDependentEntriesCommand::Create(&m_Core, origItem.GetBaseUUID(), m_Item.GetUUID(), CItemData::ET_ALIAS)
        );
        // Now actually move the aliases
        UUIDVector tlist;
        m_Core.GetAllDependentEntries(origItem.GetUUID(), tlist, CItemData::ET_ALIAS);
        for (size_t idep = 0; idep < tlist.size(); idep++) {
          ItemListIter alias_iter = m_Core.Find(tlist[idep]);
          CItemData ci_oldalias(alias_iter->second);
          CItemData ci_newalias(ci_oldalias);
          ci_newalias.SetBaseUUID(m_Item.GetBaseUUID());
          commands->Add(
            EditEntryCommand::Create(&m_Core, ci_oldalias, ci_newalias)
          );
        }
      }
      else if(origItem.IsShortcutBase() && m_Item.IsAlias()) { // Change from ShortcutBase to Alias
        commands->Add(
          AddDependentEntryCommand::Create(&m_Core, m_Item.GetBaseUUID(), origItem.GetUUID(), CItemData::ET_ALIAS)
        );
        // Delete shortcuts
        UUIDVector tlist;
        m_Core.GetAllDependentEntries(origItem.GetUUID(), tlist, CItemData::ET_SHORTCUT);
        for (size_t idep = 0; idep < tlist.size(); idep++) {
          ItemListIter shortcut_iter = m_Core.Find(tlist[idep]);
          commands->Add(
            DeleteEntryCommand::Create(&m_Core, shortcut_iter->second)
          );
        }
      }
      
      commands->Add(
        EditEntryCommand::Create(&m_Core, m_Core.GetEntry(listpos), m_Item)
      );
      
      if(bTemporaryChangeOfPWH) {
        sxPWH[0] = L'1';
        commands->Add(
          UpdateEntryCommand::Create(&m_Core, m_Item, CItemData::PWHIST, sxPWH)
        );
      }
    }

    return commands;
  }

  // No changes
  delete commands;
  return nullptr;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void AddEditPropSheetDlg::OnOk(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {

    if (!ValidateBasicData() || !ValidatePasswordPolicy()) {
      return; // don't exit dialog box (BR759)
    }

    Command *command = nullptr;

    switch (m_Type) {
      case SheetType::EDIT:
        command = NewEditEntryCommand();
        break;
      case SheetType::ADD:
        command = NewAddEntryCommand();
        break;
      case SheetType::VIEW:
        // No Update
        break;
      default: {
        ASSERT(0);
        break;
      }
    }

    if (command) {
      m_Core.Execute(command);
    }

    EndModal(wxID_OK);
  }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_KEEP
 */

void AddEditPropSheetDlg::OnKeepHistoryClick(wxCommandEvent &)
{
   if (Validate() && TransferDataFromWindow()) {
     // disable spinbox if checkbox is false
     m_AdditionalMaxPasswordHistoryCtrl->Enable(m_KeepPasswordHistory);
   }
}

void AddEditPropSheetDlg::SetXTime(wxObject *src)
{
  if (Validate() && TransferDataFromWindow()) {
    wxDateTime xdt;
    if (src == m_DatesTimesExpiryDateCtrl) { // expiration date changed, update interval
      xdt = m_DatesTimesExpiryDateCtrl->GetValue().GetDateOnly();
      m_ExpirationTimeInterval = IntervalFromDate(xdt);

    } else if (src == m_DatesTimesExpiryTimeCtrl) { // expiration interval changed, update date
      xdt = TodayPlusInterval(m_ExpirationTimeInterval);
      m_DatesTimesExpiryDateCtrl->SetValue(xdt);

    } else {
      ASSERT(0);
    }
    Validate(); TransferDataToWindow();
  } // Validated & transferred from controls
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_ON
 */

void AddEditPropSheetDlg::OnExpRadiobuttonSelected( wxCommandEvent& evt )
{
  bool On = evt.GetEventObject() == m_DatesTimesExpireOnCtrl;
  bool Never = evt.GetEventObject() == m_DatesTimesNeverExpireCtrl;

  // Sync the date with the interval so the user can see when it will expire
  if (!On && !Never) {
    wxDateTime xdt = TodayPlusInterval(m_DatesTimesExpiryTimeCtrl->GetValue());
    m_DatesTimesExpiryDateCtrl->SetValue(xdt);
    if (m_FirstInClick) {
      m_DatesTimesRecurringExpiryCtrl->SetValue(true);
      m_FirstInClick = false;
    }
  }

  m_DatesTimesExpiryDateCtrl->Enable(On && !Never);
  m_DatesTimesExpiryTimeCtrl->Enable(!On && !Never);
  m_DatesTimesStaticTextDays->Enable(!On && !Never);
  m_DatesTimesRecurringExpiryCtrl->Enable(!On && !Never);
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_CHECKBOX42
 */

void AddEditPropSheetDlg::OnPasswordPolicySelected( wxCommandEvent& evt )
{
  EnablePWPolicyControls(!evt.IsChecked());
}

void AddEditPropSheetDlg::ShowPWPSpinners(bool show)
{
  m_PasswordPolicySizer->Show(m_PasswordPolicyLowerCaseMinSizer,  show, true);
  m_PasswordPolicySizer->Show(m_PasswordPolicyUpperCaseMinSizer,  show, true);
  m_PasswordPolicySizer->Show(m_PasswordPolicyDigitsMinSizer, show, true);
  m_PasswordPolicySizer->Show(m_PasswordPolicySymbolsMinSizer, show, true);
  m_PasswordPolicySizer->Layout();
}

void AddEditPropSheetDlg::EnableNonHexCBs(bool enable)
{
  EnableSizerChildren(m_PasswordPolicySizer, enable);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX9
 */

void AddEditPropSheetDlg::OnUseHexCBClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    bool useHex = m_PasswordPolicyUseHexadecimalOnlyCtrl->GetValue();
    EnableNonHexCBs(!useHex);
    m_PasswordPolicyUseHexadecimalOnlyCtrl->Enable(true);
  }
}

/*!
 * wxEVT_UPDATE_UI event handler for all command ids
 */

void AddEditPropSheetDlg::OnUpdateUI(wxUpdateUIEvent& event)
{
  bool dbIsReadOnly = m_Core.IsReadOnly() || m_Item.IsProtected();

  switch (event.GetId()) {
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Basic"
  /////////////////////////////////////////////////////////////////////////////
    case ID_COMBOBOX_GROUP:
      event.Enable(!dbIsReadOnly);
      break;
    case ID_BUTTON_ALIAS:
      event.Enable(!dbIsReadOnly || m_Item.IsAlias());
      break;
    case ID_BUTTON_SHOWHIDE:
      m_BasicShowHideCtrl->SetLabel(m_IsPasswordHidden ? _("&Show") : _("&Hide"));
      break;
    case ID_BUTTON_GENERATE:
      event.Enable(!dbIsReadOnly && !m_Item.IsAlias()); // Do not generate password for alias entry
      break;
    case ID_TEXTCTRL_TITLE:
    case ID_TEXTCTRL_USERNAME:
    case ID_TEXTCTRL_URL:
    case ID_TEXTCTRL_EMAIL:
    case ID_TEXTCTRL_NOTES:
    {
      auto window = m_BasicPanel->FindWindow(event.GetId());
      if (window != nullptr) {
        auto control = wxDynamicCast(window, wxTextCtrl);
        if (control != nullptr) {
          control->SetEditable(!dbIsReadOnly);
        }
      }
      break;
    }
    case ID_TEXTCTRL_PASSWORD:
    case ID_TEXTCTRL_PASSWORD2:
    {
      auto window = m_BasicPanel->FindWindow(event.GetId());
      if (window != nullptr) {
        auto control = wxDynamicCast(window, wxTextCtrl);
        if (control != nullptr) {
          control->SetEditable(!dbIsReadOnly && !m_Item.IsAlias()); // Alias is not editable (at password), edit base entry instead
        }
      }
      break;
    }

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Additional"
  /////////////////////////////////////////////////////////////////////////////
    case ID_TEXTCTRL_AUTOTYPE:
    case ID_TEXTCTRL_RUN_CMD:
    {
      auto window = m_AdditionalPanel->FindWindow(event.GetId());
      if (window != nullptr) {
        auto control = wxDynamicCast(window, wxTextCtrl);
        if (control != nullptr) {
          control->SetEditable(!dbIsReadOnly);
        }
      }
      break;
    }
    case ID_COMBOBOX_DBC_ACTION:
    case ID_COMBOBOX_SDBC_ACTION:
    case ID_CHECKBOX_KEEP:
    case ID_SPINCTRL_MAX_PW_HIST:
    case ID_GRID_PW_HIST:
    case ID_BUTTON_CLEAR_HIST:
    case ID_BUTTON_COPY_ALL:
      event.Enable(!dbIsReadOnly);
      break;

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Dates and Times"
  /////////////////////////////////////////////////////////////////////////////
    case ID_RADIOBUTTON_ON:
    case ID_DATECTRL_EXP_DATE:
    case ID_RADIOBUTTON_IN:
    case ID_SPINCTRL_EXP_TIME:
    case ID_STATICTEXT_DAYS:
    case ID_CHECKBOX_RECURRING:
    case ID_RADIOBUTTON_NEVER:
      // Disable these if DB is read-only; otherwise they are controlled elsewhere.
      if (dbIsReadOnly)
        event.Enable(false);

      break;

    default:
      break;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password Policy"
  /////////////////////////////////////////////////////////////////////////////

  if (dbIsReadOnly) {
    m_PasswordPolicyPanel->Enable(!dbIsReadOnly);
  }

  //
  // Iconization of dialog depending on parent frame.
  //
  auto parent = GetParent();

  if ((parent != nullptr) && (parent->IsTopLevel())) {
    auto topLevelWindow = wxDynamicCast(parent, wxTopLevelWindow);

    /*
     * At some window managers this dialog gets also iconized when the applicaions main frame is iconized.
     * At KDE for instance, this dialog remains open while the main frame is iconized.
     * Hence, we iconize this dialog only if it was not already done.
     */
    if ((topLevelWindow != nullptr) && topLevelWindow->IsIconized() && !IsIconized()) {
      Iconize(true);
    }
  }
}

/*!
 * wxEVT_SET_FOCUS event handler for ID_TEXTCTRL_NOTES
 */

void AddEditPropSheetDlg::OnNoteSetFocus(wxFocusEvent& WXUNUSED(evt))
{
  if (m_Type != SheetType::ADD && m_IsNotesHidden) {
    m_IsNotesHidden = false;
    m_Notes = m_Item.GetNotes(TCHAR('\n')).c_str();
    m_BasicNotesTextCtrl->ChangeValue(m_Notes);
  }
}

PWPolicy AddEditPropSheetDlg::GetPWPolicyFromUI() const
{
  wxASSERT_MSG(!m_PasswordPolicyUseDatabaseCtrl->GetValue(), wxT("Trying to get Password policy from UI when db defaults are to be used"));

  PWPolicy pwp;

  pwp.length = m_PasswordPolicyPasswordLengthCtrl->GetValue();
  pwp.flags = 0;
  pwp.lowerminlength = pwp.upperminlength =
    pwp.digitminlength = pwp.symbolminlength = 0;
  if (m_PasswordPolicyUseLowerCaseCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseLowercase;
    pwp.lowerminlength = m_PasswordPolicyLowerCaseMinCtrl->GetValue();
  }
  if (m_PasswordPolicyUseUpperCaseCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseUppercase;
    pwp.upperminlength = m_PasswordPolicyUpperCaseMinCtrl->GetValue();
  }
  if (m_PasswordPolicyUseDigitsCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseDigits;
    pwp.digitminlength = m_PasswordPolicyDigitsMinCtrl->GetValue();
  }
  if (m_PasswordPolicyUseSymbolsCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseSymbols;
    pwp.symbolminlength = m_PasswordPolicySymbolsMinCtrl->GetValue();
  }

  wxASSERT_MSG(!m_PasswordPolicyUseEasyCtrl->GetValue() || !m_PasswordPolicyUsePronounceableCtrl->GetValue(), wxT("UI Bug: both pronounceable and easy-to-read are set"));

  if (m_PasswordPolicyUseEasyCtrl->GetValue())
    pwp.flags |= PWPolicy::UseEasyVision;
  else if (m_PasswordPolicyUsePronounceableCtrl->GetValue())
    pwp.flags |= PWPolicy::MakePronounceable;
  if (m_PasswordPolicyUseHexadecimalOnlyCtrl->GetValue())
    pwp.flags = PWPolicy::UseHexDigits; //yes, its '=' and not '|='

  pwp.symbols = m_Symbols.c_str();

  return pwp;
}

bool AddEditPropSheetDlg::CheckPWPolicyFromUI()
{
  wxASSERT_MSG(!m_PasswordPolicyUseDatabaseCtrl->GetValue(), wxT("Trying to get Password policy from UI when db defaults are to be used"));

  PWPolicy pwp;

  pwp.length = m_PasswordPolicyPasswordLengthCtrl->GetValue();
  pwp.flags = 0;
  pwp.lowerminlength = pwp.upperminlength =
    pwp.digitminlength = pwp.symbolminlength = 0;
  if (m_PasswordPolicyUseLowerCaseCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseLowercase;
    pwp.lowerminlength = m_PasswordPolicyLowerCaseMinCtrl->GetValue();
  }
  if (m_PasswordPolicyUseUpperCaseCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseUppercase;
    pwp.upperminlength = m_PasswordPolicyUpperCaseMinCtrl->GetValue();
  }
  if (m_PasswordPolicyUseDigitsCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseDigits;
    pwp.digitminlength = m_PasswordPolicyDigitsMinCtrl->GetValue();
  }
  if (m_PasswordPolicyUseSymbolsCtrl->GetValue()) {
    pwp.flags |= PWPolicy::UseSymbols;
    pwp.symbolminlength = m_PasswordPolicySymbolsMinCtrl->GetValue();
  }

  wxASSERT_MSG(!m_PasswordPolicyUseEasyCtrl->GetValue() || !m_PasswordPolicyUsePronounceableCtrl->GetValue(), wxT("UI Bug: both pronounceable and easy-to-read are set"));

  if (m_PasswordPolicyUseEasyCtrl->GetValue())
    pwp.flags |= PWPolicy::UseEasyVision;
  else if (m_PasswordPolicyUsePronounceableCtrl->GetValue())
    pwp.flags |= PWPolicy::MakePronounceable;
  if (m_PasswordPolicyUseHexadecimalOnlyCtrl->GetValue())
    pwp.flags = PWPolicy::UseHexDigits; //yes, its '=' and not '|='

  pwp.symbols = m_Symbols.c_str();
  
  int total_sublength = (
    ((pwp.flags & PWPolicy::UseLowercase) ? pwp.lowerminlength : 0) +
    ((pwp.flags & PWPolicy::UseUppercase) ? pwp.upperminlength : 0) +
    ((pwp.flags & PWPolicy::UseDigits) ? pwp.digitminlength : 0) +
    ((pwp.flags & PWPolicy::UseSymbols) ? pwp.symbolminlength : 0));

  if(pwp.flags && pwp.length < total_sublength) {
    wxMessageDialog msg(
      this,
      _("Total Length of policy too small"),
      _("Error"),
      wxOK|wxICON_ERROR
    );
    msg.ShowModal();
    return false;
  }
  
  if (pwp.length != 0) {// if length != 0 we assume the policy isn't empty, and so the following must hold:
    // At least one set of characters is specified
    if(! ((pwp.flags & PWPolicy::UseLowercase) || (pwp.flags & PWPolicy::UseUppercase) || (pwp.flags & PWPolicy::UseDigits) || (pwp.flags & PWPolicy::UseSymbols) || (pwp.flags & PWPolicy::UseHexDigits))) {
      wxMessageDialog msg(
        this,
        _("With password length at least one of the flags has to be set"),
        _("Error"),
        wxOK|wxICON_ERROR
      );
      msg.ShowModal();
      return false;
    }
    // HexDigits implies no easyvision or pronounceable
    if (pwp.flags & PWPolicy::UseHexDigits && ! ((pwp.flags & (PWPolicy::UseEasyVision | PWPolicy::MakePronounceable)) == 0)) {
      wxMessageDialog msg(
        this,
        _("HexDigits implies no easyvision or pronounceable to be set"),
        _("Error"),
        wxOK|wxICON_ERROR
      );
      msg.ShowModal();
      return false;
    }
  }
  return true;
}

PWPolicy AddEditPropSheetDlg::GetSelectedPWPolicy() const
{
  PWPolicy pwp;
  if (m_PasswordPolicyUseDatabaseCtrl->GetValue()) {
    const wxString polName = m_PasswordPolicyNamesCtrl->GetValue();
    m_Core.GetPolicyFromName(tostringx(polName), pwp);
  } else
    pwp = GetPWPolicyFromUI();
  return pwp;
}

/**
 * wxEVT_SPINCTRL event handler for ID_SPINCTRL5, ID_SPINCTRL6, 
 * ID_SPINCTRL7, ID_SPINCTRL8
 * 
 * Ensures that the sum of each character class' minimum counts 
 * doesn't exceed the overall password length, increasing it as 
 * necessary to give the user some visual indication.
 * 
 * This is not comprehensive & foolproof since there are far too 
 * many ways to make the password length smaller than the sum of 
 * "at least" lengths, to even think of.
 *
 * In OnOk(), we just ensure the password length is greater than
 * the sum of all enabled "at least" lengths.  We have to do this 
 * in the UI, or else password generation crashes.
 */
void AddEditPropSheetDlg::OnAtLeastPasswordChars(wxSpinEvent& WXUNUSED(event))
{
  const int min = GetRequiredPWLength();

  // Increase password length up to the allowed maximum
  if ((m_PasswordPolicyPasswordLengthCtrl->GetMax() > min) && (min > m_PasswordPolicyPasswordLengthCtrl->GetValue())) {
    m_PasswordPolicyPasswordLengthCtrl->SetValue(min);
  }
}

int AddEditPropSheetDlg::GetRequiredPWLength() const
{
  wxSpinCtrl* spinControls[] = { m_PasswordPolicyUpperCaseMinCtrl, m_PasswordPolicyLowerCaseMinCtrl, m_PasswordPolicyDigitsMinCtrl, m_PasswordPolicySymbolsMinCtrl };
  int total = 0;

  // Calculate the sum of each character class' minimum count
  for (const auto spinControl : spinControls) {
    total += spinControl->IsEnabled() ? spinControl->GetValue() : 0;
  }

  return total;
}

void AddEditPropSheetDlg::OnClearPasswordHistory(wxCommandEvent& WXUNUSED(evt))
{
  m_AdditionalPasswordHistoryGrid->ClearGrid();
  if (m_AdditionalMaxPasswordHistoryCtrl->TransferDataFromWindow() && m_KeepPasswordHistory && m_MaxPasswordHistory > 0) {
    m_PasswordHistory = towxstring(PWHistList::MakePWHistoryHeader(m_KeepPasswordHistory, m_MaxPasswordHistory));
  }
  else
    m_PasswordHistory.Empty();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
 */

void AddEditPropSheetDlg::OnEasyReadCBClick(wxCommandEvent& evt)
{
  stringT st_symbols;
  if (evt.IsChecked()) {
    // Check if pronounceable is also set - forbid both
    if (m_PasswordPolicyUsePronounceableCtrl->GetValue()) {
      m_PasswordPolicyUseEasyCtrl->SetValue(false);
      wxMessageBox(_("\"Easy-to-read\" and \"pronounceable\" cannot be both selected."),
                   _("Unsupported selection"), wxOK|wxICON_ERROR, this);
      return;
    }

    st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
  } else { // not checked - restore default symbols to appropriate value
    if (m_PasswordPolicyUsePronounceableCtrl->GetValue())
      st_symbols = CPasswordCharPool::GetPronounceableSymbols();
    else
      st_symbols = CPasswordCharPool::GetDefaultSymbols();
  }
  m_Symbols = st_symbols.c_str();
  m_PasswordPolicyOwnSymbolsTextCtrl->SetValue(m_Symbols);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX8
 */

void AddEditPropSheetDlg::OnPronouceableCBClick( wxCommandEvent& evt)
{
  stringT st_symbols;
  if (evt.IsChecked()) {
    // Check if ezread is also set - forbid both
    if (m_PasswordPolicyUseEasyCtrl->GetValue()) {
      m_PasswordPolicyUsePronounceableCtrl->SetValue(false);
      wxMessageBox(_("\"Pronounceable\" and \"easy-to-read\" cannot be both selected."),
                   _("Unsupported selection"), wxOK|wxICON_ERROR, this);
      return;
    }
    st_symbols = CPasswordCharPool::GetPronounceableSymbols();
  } else { // not checked - restore default symbols to appropriate value
    if (m_PasswordPolicyUseEasyCtrl->GetValue())
      st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
    else
      st_symbols = CPasswordCharPool::GetDefaultSymbols();
  }
  m_Symbols = st_symbols.c_str();
  m_PasswordPolicyOwnSymbolsTextCtrl->SetValue(m_Symbols);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON9
 */

void AddEditPropSheetDlg::OnSendButtonClick( wxCommandEvent& event )
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
  if (Validate() && TransferDataFromWindow() && !m_Email.IsEmpty()) {
    StringX mail_cmd= tostringx(_("mailto:"));
    mail_cmd += tostringx(m_Email);
    PWSRun runner;
    runner.issuecmd(mail_cmd, wxEmptyString, false);
  }
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_POLICYLIST
 */

void AddEditPropSheetDlg::OnPolicylistSelected( wxCommandEvent& event )
{
  const wxString polName = event.GetString();
  PWPolicy policy;
  if (polName == _("Default Policy")) {
    policy = PWSprefs::GetInstance()->GetDefaultPolicy();
  } else {
    if (!m_Core.GetPolicyFromName(tostringx(polName), policy)) {
      pws_os::Trace(wxT("Couldn't find policy %ls\n"), ToStr(polName));
      return;
    }
  }
  m_PasswordPolicyUseDatabaseCtrl->SetValue(true);
  UpdatePWPolicyControls(policy);
  EnablePWPolicyControls(false);
}

/*!
 * wxEVT_DATE_CHANGED event handler for ID_DATECTRL_EXP_DATE
 */

void AddEditPropSheetDlg::OnExpDateChanged( wxDateEvent& event )
{
  SetXTime(event.GetEventObject());
}

/*!
 * wxEVT_COMMAND_SPINCTRL_UPDATED event handler for ID_SPINCTRL_EXP_TIME
 */

void AddEditPropSheetDlg::OnExpIntervalChanged( wxSpinEvent& event )
{
  SetXTime(event.GetEventObject());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX6
 */

void AddEditPropSheetDlg::OnSymbolsCB( wxCommandEvent& event )
{
  bool checked = event.IsChecked();
  m_PasswordPolicyOwnSymbolsTextCtrl->Enable(checked);
  m_PasswordPolicySymbolsMinCtrl->Enable(checked);
  FindWindow(ID_RESET_SYMBOLS)->Enable(checked);
}

/*!
 * wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS
 */

void AddEditPropSheetDlg::OnOwnSymSetFocus( wxFocusEvent& event )
{
////@begin wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS in AddEditPropSheetDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_SET_FOCUS event handler for IDC_OWNSYMBOLS in AddEditPropSheetDlg.
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RESET_SYMBOLS
 */

void AddEditPropSheetDlg::OnResetSymbolsClick( wxCommandEvent& WXUNUSED(event) )
{
  stringT st_symbols;
  if (m_PasswordPolicyUseEasyCtrl->GetValue())
    st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
  else if (m_PasswordPolicyUsePronounceableCtrl->GetValue())
    st_symbols = CPasswordCharPool::GetPronounceableSymbols();
  else
    st_symbols = CPasswordCharPool::GetDefaultSymbols();
  m_Symbols = st_symbols.c_str();
  m_PasswordPolicyOwnSymbolsTextCtrl->SetValue(m_Symbols);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX5
 */

void AddEditPropSheetDlg::OnDigitsCB( wxCommandEvent& event )
{
  m_PasswordPolicyDigitsMinCtrl->Enable(event.IsChecked());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX4
 */

void AddEditPropSheetDlg::OnUppercaseCB( wxCommandEvent& event )
{
  m_PasswordPolicyUpperCaseMinCtrl->Enable(event.IsChecked());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
 */

void AddEditPropSheetDlg::OnLowercaseCB( wxCommandEvent& event )
{
  m_PasswordPolicyLowerCaseMinCtrl->Enable(event.IsChecked());
}

bool AddEditPropSheetDlg::SyncAndQueryCancel(bool showDialog) {
  // when edit forbidden, allow cancel without additional checks
  if (m_Type == SheetType::VIEW || m_Core.IsReadOnly() || m_Item.IsProtected()) {
    return true;
  }
  else if (!(Validate() && TransferDataFromWindow()) || GetChanges() != Changes::None) {
    if (showDialog) {
      wxMessageDialog dialog(
        nullptr,
        _("One or more values have been changed.\nDo you want to discard the changes?"), wxEmptyString,
        wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_EXCLAMATION
      );
      dialog.SetOKLabel(_("Discard"));

      auto res = dialog.ShowModal();
      if (res == wxID_OK) {
        return true;
      }
    }
    return false;
  }
  return true;
}

void AddEditPropSheetDlg::OnCancel(wxCommandEvent& WXUNUSED(evt))
{
  if (SyncAndQueryCancel(true)) {
    EndModal(wxID_CANCEL);
  }
}

/// wxEVT_CLOSE event handler
void AddEditPropSheetDlg::OnClose(wxCloseEvent &event)
{
  if (event.CanVeto()) {
    // when trying to closing app/db, don't ask questions when data changed
    if (!SyncAndQueryCancel(!IsCloseInProgress())) {
      event.Veto();
      return;
    }
  }
  EndDialog(wxID_CANCEL); // cancel directly (if we skip event, OnCancel will be called and ask one more time)
}
