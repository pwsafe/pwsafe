/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/datetime.h>
#include <wx/mstream.h>
////@end includes

#include "AddEditPropSheetDlg.h"
#include "Clipboard.h"
#include "wxUtilities.h"

#include "core/PWCharPool.h"
#include "core/PWHistory.h"
#include "core/PWSprefs.h"
#include "os/media.h"
#include "os/run.h"

#include <algorithm>
#include <vector>


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

  EVT_BUTTON(       wxID_OK,                 AddEditPropSheetDlg::OnOk                      )
////@begin AddEditPropSheetDlg event table entries
  EVT_BUTTON(       ID_BUTTON_SHOWHIDE,      AddEditPropSheetDlg::OnShowHideClick           )
  EVT_BUTTON(       ID_BUTTON_GENERATE,      AddEditPropSheetDlg::OnGenerateButtonClick     )
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
  EVT_UPDATE_UI(    ID_BUTTON_GENERATE,      AddEditPropSheetDlg::OnUpdateUI                )
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

//(*IdInit(AttachmentTab)
const long AddEditPropSheetDlg::ID_IMAGEPANEL1 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_STATICTEXT1 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_BUTTON_IMPORT = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_BUTTON_EXPORT = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_BUTTON_REMOVE = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_TEXTCTRL2 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_STATICTEXT4 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_STATICTEXT5 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_STATICTEXT6 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_STATICTEXT8 = wxWindow::NewControlId();
const long AddEditPropSheetDlg::ID_STATICTEXT10 = wxWindow::NewControlId();
//*)


/*!
 * AddEditPropSheetDlg constructors
 */

AddEditPropSheetDlg::AddEditPropSheetDlg(wxWindow* parent, PWScore &core,
                                   SheetType type, const CItemData *item,
                                   const wxString& selectedGroup,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size,
                                   long style)
: m_Core(core), m_SelectedGroup(selectedGroup), m_Type(type)
{
  if (item != nullptr)
    m_Item = *item; // copy existing item to display values
  else
    m_Item.CreateUUID(); // We're adding a new entry
  Init();
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
  Create(parent, id, dlgTitle, pos, size, style);

  if (m_Core.GetReadFileVersion() == PWSfile::V40) {
    InitAttachmentTab();
  }
}

/*!
 * AddEditPropSheetDlg creator
 */

bool AddEditPropSheetDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin AddEditPropSheetDlg creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  int flags = (m_Type == SheetType::VIEW) ? (wxCLOSE|wxHELP) : (wxOK|wxCANCEL|wxHELP);
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
  return true;
}

/*!
 * AddEditPropSheetDlg destructor
 */

AddEditPropSheetDlg::~AddEditPropSheetDlg()
{
////@begin AddEditPropSheetDlg destruction
////@end AddEditPropSheetDlg destruction
}

/*!
 * Member initialisation
 */

void AddEditPropSheetDlg::Init()
{
////@begin AddEditPropSheetDlg member initialisation
  m_ExpirationTimeInterval = 0;
  m_IsNotesHidden = !PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault);
  m_BasicPanel = nullptr;
  m_AdditionalPanel = nullptr;
  m_PasswordPolicyPanel = nullptr;
  m_BasicSizer = nullptr;
  m_BasicGroupNamesCtrl = nullptr;
  m_BasicUsernameTextCtrl = nullptr;
  m_BasicPasswordTextCtrl = nullptr;
  m_BasicShowHideCtrl = nullptr;
  m_BasicPasswordConfirmationTextCtrl = nullptr;
  m_BasicNotesTextCtrl = nullptr;
  m_AdditionalDoubleClickActionCtrl = nullptr;
  m_AdditionalShiftDoubleClickActionCtrl = nullptr;
  m_AdditionalMaxPasswordHistoryCtrl = nullptr;
  m_AdditionalPasswordHistoryGrid = nullptr;
  m_DatesTimesExpireOnCtrl = nullptr;
  m_DatesTimesExpiryDateCtrl = nullptr;
  m_DatesTimesExpireInCtrl = nullptr;
  m_DatesTimesExpiryTimeCtrl = nullptr;
  m_DatesTimesRecurringExpiryCtrl = nullptr;
  m_DatesTimesNeverExpireCtrl = nullptr;
  m_PasswordPolicyUseDatabaseCtrl = nullptr;
  m_PasswordPolicyNamesCtrl = nullptr;
  m_PasswordPolicyPasswordLengthText = nullptr;
  m_PasswordPolicyPasswordLengthCtrl = nullptr;
  m_PasswordPolicySizer = nullptr;
  m_PasswordPolicyUseLowerCaseCtrl = nullptr;
  m_PasswordPolicyLowerCaseMinSizer = nullptr;
  m_PasswordPolicyLowerCaseMinCtrl = nullptr;
  m_PasswordPolicyUseUpperCaseCtrl = nullptr;
  m_PasswordPolicyUpperCaseMinSizer = nullptr;
  m_PasswordPolicyUpperCaseMinCtrl = nullptr;
  m_PasswordPolicyUseDigitsCtrl = nullptr;
  m_PasswordPolicyDigitsMinSizer = nullptr;
  m_PasswordPolicyDigitsMinCtrl = nullptr;
  m_PasswordPolicyUseSymbolsCtrl = nullptr;
  m_PasswordPolicySymbolsMinSizer = nullptr;
  m_PasswordPolicySymbolsMinCtrl = nullptr;
  m_PasswordPolicyOwnSymbolsTextCtrl = nullptr;
  m_PasswordPolicyUseEasyCtrl = nullptr;
  m_PasswordPolicyUsePronounceableCtrl = nullptr;
  m_PasswordPolicyUseHexadecimalOnlyCtrl = nullptr;
  m_AttachmentPanel = nullptr;
  m_AttachmentImagePanel = nullptr;
  m_AttachmentButtonImport = nullptr;
  m_AttachmentButtonExport = nullptr;
  m_AttachmentButtonRemove = nullptr;
  m_AttachmentFilePath = nullptr;
  m_AttachmentTitle = nullptr;
  m_AttachmentMediaType = nullptr;
  m_AttachmentCreationDate = nullptr;
  m_AttachmentFileSize = nullptr;
  m_AttachmentFileCreationDate = nullptr;
  m_AttachmentFileLastModifiedDate = nullptr;
  m_AttachmentPreviewStatus = nullptr;
////@end AddEditPropSheetDlg member initialisation
}

#if 0
/*
  This class is currently unused, but it should be easy to incorporate into the Policy checks.
  Validate() works as follows:
  return true if (specified radio button is enabled AND set) OR at least one of the specified checkboxes are selected OR all checkboxes are disabled

  If we decide this is useless, then MultiCheckboxValidator in wxutil.{cpp.h} should be removed as well.
*/
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
#endif /* 0 */

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

  // Setup symbols
  m_Symbols = CPasswordCharPool::GetDefaultSymbols().c_str();
  m_PasswordPolicyOwnSymbolsTextCtrl->SetValue(m_Symbols);

  m_DatesTimesExpiryTimeCtrl->SetRange(1, 3650);
}

wxPanel* AddEditPropSheetDlg::CreateBasicPanel()
{
  auto *panel = new wxPanel( GetBookCtrl(), ID_PANEL_BASIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(itemBoxSizer3);

  auto *itemStaticText4 = new wxStaticText( panel, wxID_STATIC, _(
    "To add a new entry, simply fill in the fields below. At least a title and\n"
    "a password are required. If you have set a default username, it will\n"
    "appear in the username field."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL, 5);

  m_BasicSizer = new wxGridBagSizer();

  itemBoxSizer3->Add(m_BasicSizer, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP|wxALL, 0);
  auto *itemStaticText6 = new wxStaticText( panel, wxID_STATIC, _("Group:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText6, wxGBPosition(/*row:*/ 0, /*column:*/ 0), wxDefaultSpan,  wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicGroupNamesCtrl = new wxComboBox( panel, ID_COMBOBOX_GROUP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN);
  m_BasicSizer->Add(m_BasicGroupNamesCtrl, wxGBPosition(/*row:*/ 0, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemStaticText9 = new wxStaticText( panel, wxID_STATIC, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText9, wxGBPosition(/*row:*/ 1, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicTitleTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_TITLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicTitleTextCtrl, wxGBPosition(/*row:*/ 1, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemStaticText12 = new wxStaticText( panel, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText12, wxGBPosition(/*row:*/ 2, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicUsernameTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_USERNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicUsernameTextCtrl , wxGBPosition(/*row:*/ 2, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemStaticText15 = new wxStaticText( panel, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText15, wxGBPosition(/*row:*/ 3, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicPasswordTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicPasswordTextCtrl, wxGBPosition(/*row:*/ 3, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicShowHideCtrl = new wxButton( panel, ID_BUTTON_SHOWHIDE, _("&Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicShowHideCtrl, wxGBPosition(/*row:*/ 3, /*column:*/ 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemButton21 = new wxButton( panel, ID_BUTTON_GENERATE, _("&Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton21, wxGBPosition(/*row:*/ 3, /*column:*/ 3), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemStaticText22 = new wxStaticText( panel, wxID_STATIC, _("Confirm:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText22, wxGBPosition(/*row:*/ 4, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicPasswordConfirmationTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_PASSWORD2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
  m_BasicSizer->Add(m_BasicPasswordConfirmationTextCtrl, wxGBPosition(/*row:*/ 4, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemStaticText25 = new wxStaticText( panel, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText25, wxGBPosition(/*row:*/ 5, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicUrlTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_URL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicUrlTextCtrl, wxGBPosition(/*row:*/ 5, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemButton29 = new wxButton( panel, ID_GO_BTN, _("Go"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton29, wxGBPosition(/*row:*/ 5, /*column:*/ 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

  auto *itemStaticText30 = new wxStaticText( panel, wxID_STATIC, _("EMail:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText30, wxGBPosition(/*row:*/ 6, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_BasicEmailTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_EMAIL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(m_BasicEmailTextCtrl, wxGBPosition(/*row:*/ 6, /*column:*/ 1), wxDefaultSpan, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemButton34 = new wxButton( panel, ID_SEND_BTN, _("Send"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemButton34, wxGBPosition(/*row:*/ 6, /*column:*/ 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

  auto *itemStaticText36 = new wxStaticText( panel, wxID_STATIC, _("Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_BasicSizer->Add(itemStaticText36, wxGBPosition(/*row:*/ 7, /*column:*/ 0), wxDefaultSpan, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

  m_BasicNotesTextCtrl = new wxTextCtrl( panel, ID_TEXTCTRL_NOTES, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
  m_BasicSizer->Add(m_BasicNotesTextCtrl, wxGBPosition(/*row:*/ 7, /*column:*/ 1), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3) , wxEXPAND|wxALL, 5);

  m_BasicSizer->AddGrowableCol(1);  // Growable text entry fields
  m_BasicSizer->AddGrowableRow(7);  // Growable notes field

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
  auto *itemBoxSizer39 = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(itemBoxSizer39);

  auto *itemFlexGridSizer40 = new wxFlexGridSizer(0, 2, 0, 0);
  itemBoxSizer39->Add(itemFlexGridSizer40, 0, wxEXPAND | wxALL, 5);
  auto *itemStaticText41 = new wxStaticText(panel, wxID_STATIC, _("Autotype:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer40->Add(itemStaticText41, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemTextCtrl42 = new wxTextCtrl(panel, ID_TEXTCTRL_AUTOTYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer40->Add(itemTextCtrl42, 0, wxEXPAND | wxALL, 5);

  auto *itemStaticText43 = new wxStaticText(panel, wxID_STATIC, _("Run Cmd:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer40->Add(itemStaticText43, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemTextCtrl44 = new wxTextCtrl(panel, ID_TEXTCTRL_RUN_CMD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer40->Add(itemTextCtrl44, 0, wxEXPAND | wxALL, 5);

  auto *itemStaticText45 = new wxStaticText(panel, wxID_STATIC, _("Double-Click Action:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer40->Add(itemStaticText45, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxArrayString dcaComboBoxStrings;
  setupDCAStrings(dcaComboBoxStrings);
  m_AdditionalDoubleClickActionCtrl = new wxComboBox(panel, ID_COMBOBOX_DBC_ACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, dcaComboBoxStrings, wxCB_READONLY);
  itemFlexGridSizer40->Add(m_AdditionalDoubleClickActionCtrl, 0, wxEXPAND | wxALL, 5);

  auto *itemStaticText47 = new wxStaticText(panel, wxID_STATIC, _("Shift-Double-Click Action:"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer40->Add(itemStaticText47, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxArrayString sdcaComboBoxStrings;
  setupDCAStrings(sdcaComboBoxStrings);
  m_AdditionalShiftDoubleClickActionCtrl = new wxComboBox(panel, ID_COMBOBOX_SDBC_ACTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, sdcaComboBoxStrings, wxCB_READONLY);
  itemFlexGridSizer40->Add(m_AdditionalShiftDoubleClickActionCtrl, 0, wxEXPAND | wxALL, 5);

  auto *itemStaticBoxSizer49Static = new wxStaticBox(panel, wxID_ANY, _("Password History"));
  auto *itemStaticBoxSizer49 = new wxStaticBoxSizer(itemStaticBoxSizer49Static, wxVERTICAL);
  itemBoxSizer39->Add(itemStaticBoxSizer49, 0, wxEXPAND | wxALL, 5);
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
  m_AdditionalPasswordHistoryGrid->SetDefaultColSize(150);
  m_AdditionalPasswordHistoryGrid->SetDefaultRowSize(25);
  m_AdditionalPasswordHistoryGrid->SetColLabelSize(25);
  m_AdditionalPasswordHistoryGrid->SetRowLabelSize(0);
  m_AdditionalPasswordHistoryGrid->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemStaticBoxSizer49->Add(m_AdditionalPasswordHistoryGrid, 0, wxEXPAND | wxALL, 5);

  auto *itemBoxSizer55 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer55, 0, wxEXPAND | wxALL, 5);
  wxButton *itemButton56 = new wxButton(panel, ID_BUTTON_CLEAR_HIST, _("Clear History"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer55->Add(itemButton56, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  itemBoxSizer55->AddStretchSpacer();

  wxButton *itemButton58 = new wxButton(panel, ID_BUTTON_COPY_ALL, _("Copy All"), wxDefaultPosition, wxDefaultSize, 0);
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
  itemBoxSizer60->Add(itemStaticBoxSizer61, 0, wxEXPAND | wxALL, 5);
  auto *itemBoxSizer62 = new wxBoxSizer(wxVERTICAL);
  itemStaticBoxSizer61->Add(itemBoxSizer62, 0, wxEXPAND | wxALL, 0);
  auto *itemFlexGridSizer63 = new wxFlexGridSizer(0, 3, 0, 0);
  itemBoxSizer62->Add(itemFlexGridSizer63, 0, wxEXPAND | wxALL, 5);
  m_DatesTimesExpireOnCtrl = new wxRadioButton(panel, ID_RADIOBUTTON_ON, _("On"), wxDefaultPosition, wxDefaultSize, 0);
  m_DatesTimesExpireOnCtrl->SetValue(false);
  itemFlexGridSizer63->Add(m_DatesTimesExpireOnCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_DatesTimesExpiryDateCtrl = new wxDatePickerCtrl(panel, ID_DATECTRL_EXP_DATE, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
  itemFlexGridSizer63->Add(m_DatesTimesExpiryDateCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  itemFlexGridSizer63->AddStretchSpacer();

  m_DatesTimesExpireInCtrl = new wxRadioButton(panel, ID_RADIOBUTTON_IN, _("In"), wxDefaultPosition, wxDefaultSize, 0);
  m_DatesTimesExpireInCtrl->SetValue(false);
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

  auto *itemStaticText70 = new wxStaticText(panel, ID_STATICTEXT_DAYS, _("days"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer68->Add(itemStaticText70, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_DatesTimesRecurringExpiryCtrl = new wxCheckBox(panel, ID_CHECKBOX_RECURRING, _("Recurring"), wxDefaultPosition, wxDefaultSize, 0);
  m_DatesTimesRecurringExpiryCtrl->SetValue(false);
  itemFlexGridSizer63->Add(m_DatesTimesRecurringExpiryCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemBoxSizer72 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer72, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM, 5);
  m_DatesTimesNeverExpireCtrl = new wxRadioButton(panel, ID_RADIOBUTTON_NEVER, _("Never"), wxDefaultPosition, wxDefaultSize, 0);
  m_DatesTimesNeverExpireCtrl->SetValue(false);
  itemBoxSizer72->Add(m_DatesTimesNeverExpireCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemBoxSizer74 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer74, 0, wxEXPAND | wxALL, 5);
  auto *itemStaticText75 = new wxStaticText(panel, wxID_STATIC, _("Original Value:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer74->Add(itemStaticText75, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticText76 = new wxStaticText(panel, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer74->Add(itemStaticText76, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  auto *itemStaticBoxSizer77Static = new wxStaticBox(panel, wxID_ANY, _("Statistics"));
  auto *itemStaticBoxSizer77 = new wxStaticBoxSizer(itemStaticBoxSizer77Static, wxVERTICAL);
  itemBoxSizer60->Add(itemStaticBoxSizer77, 0, wxEXPAND | wxALL, 5);
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
  itemStaticText76->SetValidator(wxGenericValidator(&m_CurrentExpirationTime));
  itemStaticText80->SetValidator(wxGenericValidator(&m_CreationTime));
  itemStaticText82->SetValidator(wxGenericValidator(&m_ModificationTime));
  itemStaticText84->SetValidator(wxGenericValidator(&m_AccessTime));
  itemStaticText86->SetValidator(wxGenericValidator(&m_RMTime));

  return panel;
}

wxPanel* AddEditPropSheetDlg::CreatePasswordPolicyPanel()
{
  auto *panel = new wxPanel(GetBookCtrl(), ID_PANEL_PPOLICY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxTAB_TRAVERSAL);
  auto *itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(itemBoxSizer61);

  auto *itemStaticBoxSizer88Static = new wxStaticBox(panel, wxID_ANY, _("Random password generation rules"));
  auto *itemStaticBoxSizer88 = new wxStaticBoxSizer(itemStaticBoxSizer88Static, wxVERTICAL);
  itemBoxSizer61->Add(itemStaticBoxSizer88, 0, wxEXPAND | wxALL, 5);

  auto *itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer88->Add(itemBoxSizer89, 2, wxEXPAND | wxALL, 0);

  m_PasswordPolicyUseDatabaseCtrl = new wxCheckBox(panel, ID_CHECKBOX42, _("Use Database Policy"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
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
  auto *panel = new wxPanel(GetBookCtrl(), ID_PANEL_ADDITIONAL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *BoxSizerMain = new wxBoxSizer(wxVERTICAL);

  StaticBoxSizerPreview = new wxStaticBoxSizer(wxHORIZONTAL, panel, _("Preview"));
  m_AttachmentImagePanel = new ImagePanel(panel, wxDefaultSize);
  StaticBoxSizerPreview->Add(m_AttachmentImagePanel, 1, wxALL|wxEXPAND, 5);
  m_AttachmentPreviewStatus = new wxStaticText(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL, _T("ID_STATICTEXT_STATUS"));
  StaticBoxSizerPreview->Add(m_AttachmentPreviewStatus, 1, wxALL|wxALIGN_CENTER, 5);
  StaticBoxSizerPreview->SetMinSize(wxSize(-1, 300));
  BoxSizerMain->Add(StaticBoxSizerPreview, 1, wxALL|wxEXPAND, 5);

  auto *StaticBoxSizerFile = new wxStaticBoxSizer(wxVERTICAL, panel, _("File"));
  m_AttachmentFilePath = new wxStaticText(panel, wxID_ANY, _("N/A"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxST_ELLIPSIZE_MIDDLE, _T("ID_STATICTEXT_PATH"));
  StaticBoxSizerFile->Add(m_AttachmentFilePath, 0, wxALL|wxEXPAND, 5);

  auto *BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  m_AttachmentButtonImport = new wxButton(panel, ID_BUTTON_IMPORT, _("Import..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
  BoxSizer3->Add(m_AttachmentButtonImport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentButtonExport = new wxButton(panel, ID_BUTTON_EXPORT, _("Export..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EXPORT"));
  BoxSizer3->Add(m_AttachmentButtonExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentButtonRemove = new wxButton(panel, ID_BUTTON_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE"));
  BoxSizer3->Add(m_AttachmentButtonRemove, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  StaticBoxSizerFile->Add(BoxSizer3, 0, wxALL|wxEXPAND, 5);
  BoxSizerMain->Add(StaticBoxSizerFile, 0, wxALL|wxEXPAND, 5);

  auto *StaticBoxSizerProperties = new wxStaticBoxSizer(wxHORIZONTAL, panel, _("Properties"));
  auto *FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  FlexGridSizer1->AddGrowableCol(1);

  auto *StaticText3 = new wxStaticText(panel, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText3, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentTitle = new wxTextCtrl(panel, ID_TEXTCTRL2, _("Text"), wxDefaultPosition, wxSize(217,35), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
  FlexGridSizer1->Add(m_AttachmentTitle, 1, wxALL|wxEXPAND, 5);

  auto *StaticText2 = new wxStaticText(panel, wxID_ANY, _("Media Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentMediaType = new wxStaticText(panel, ID_STATICTEXT4, _("Label1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
  FlexGridSizer1->Add(m_AttachmentMediaType, 1, wxALL|wxEXPAND, 5);

  auto *StaticText4 = new wxStaticText(panel, wxID_ANY, _("Creation Date:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText4, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentCreationDate = new wxStaticText(panel, ID_STATICTEXT5, _("Label2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
  FlexGridSizer1->Add(m_AttachmentCreationDate, 1, wxALL|wxEXPAND, 5);

  auto *StaticText5 = new wxStaticText(panel, wxID_ANY, _("File Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText5, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentFileSize = new wxStaticText(panel, ID_STATICTEXT6, _("Label3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
  FlexGridSizer1->Add(m_AttachmentFileSize, 1, wxALL|wxEXPAND, 5);

  auto *StaticText7 = new wxStaticText(panel, wxID_ANY, _("File Creation Date:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText7, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentFileCreationDate = new wxStaticText(panel, ID_STATICTEXT8, _("Label4"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
  FlexGridSizer1->Add(m_AttachmentFileCreationDate, 1, wxALL|wxEXPAND, 5);

  auto *StaticText9 = new wxStaticText(panel, wxID_ANY, _("File Last Modified Date:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText9, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_AttachmentFileLastModifiedDate = new wxStaticText(panel, ID_STATICTEXT10, _("Label5"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
  FlexGridSizer1->Add(m_AttachmentFileLastModifiedDate, 1, wxALL|wxEXPAND, 5);
  StaticBoxSizerProperties->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
  BoxSizerMain->Add(StaticBoxSizerProperties, 0, wxALL|wxEXPAND, 5);

  panel->SetSizer(BoxSizerMain);

  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditPropSheetDlg::OnImport, this, ID_BUTTON_IMPORT);
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditPropSheetDlg::OnExport, this, ID_BUTTON_EXPORT);
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AddEditPropSheetDlg::OnRemove, this, ID_BUTTON_REMOVE);
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

      // Attachment must be removed before a new one can be imported again.
      DisableImport();
    }
    else {
      ResetAttachmentData();
    }
  }
  else {
    ResetAttachmentData();
    HideImagePreview(_("No Attachment Available"));
    EnableImport();
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
 * Returns 'true' if file's mime type is image, otherwise 'false'.
 */
bool AddEditPropSheetDlg::IsFileMimeTypeImage(const wxString &filename)
{
  return IsMimeTypeImage(pws_os::GetMediaType(tostdstring(filename)));
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
      _("Unexpected error occured during attachment import."), _("Import Attachment"),
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
    this, _("Export Attachment"), "", "",
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
      _("Unexpected error occured during attachment export."), _("Export Attachment"),
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

void AddEditPropSheetDlg::UpdateExpTimes()
{
  // From m_item to display

  m_Item.GetXTime(m_tttExpirationTime);
  m_Item.GetXTimeInt(m_ExpirationTimeInterval);
  m_ExpirationTime = m_CurrentExpirationTime = m_Item.GetXTimeL().c_str();

  if (m_ExpirationTime.empty())
    m_ExpirationTime = m_CurrentExpirationTime = _("Never");

  wxCommandEvent dummy;
  wxDateTime exp;
  if (m_tttExpirationTime == 0) { // never expires
    m_DatesTimesExpireOnCtrl->SetValue(false);
    m_DatesTimesExpireInCtrl->SetValue(false);
    m_DatesTimesNeverExpireCtrl->SetValue(true);
    exp = wxDateTime::Now();
    dummy.SetEventObject(m_DatesTimesNeverExpireCtrl);
  } else {
    exp = wxDateTime(m_tttExpirationTime);
    if (m_ExpirationTimeInterval == 0) { // expiration specified as date
      m_DatesTimesExpireOnCtrl->SetValue(true);
      m_DatesTimesExpireInCtrl->SetValue(false);
      m_DatesTimesNeverExpireCtrl->SetValue(false);
      m_DatesTimesExpiryTimeCtrl->Enable(false);
      m_Recurring = false;
      dummy.SetEventObject(m_DatesTimesExpireOnCtrl);
    } else { // exp. specified as interval
      m_DatesTimesExpireOnCtrl->SetValue(false);
      m_DatesTimesExpireInCtrl->SetValue(true);
      m_DatesTimesNeverExpireCtrl->SetValue(false);
      m_DatesTimesExpiryDateCtrl->Enable(false);
      m_DatesTimesExpiryTimeCtrl->SetValue(m_ExpirationTimeInterval);
      m_Recurring = true;
      dummy.SetEventObject(m_DatesTimesExpireInCtrl);
    }
    m_DatesTimesRecurringExpiryCtrl->Enable(m_Recurring);
    m_DatesTimesExpiryDateCtrl->SetValue(exp);
  }

  if (exp > wxDateTime::Today())
    exp = wxDateTime::Today(); // otherwise we can never move exp date back
  m_DatesTimesExpiryDateCtrl->SetRange(exp, wxDateTime(time_t(-1)));

  OnExpRadiobuttonSelected(dummy); // setup enable/disable of expiry-related controls
}

void AddEditPropSheetDlg::ItemFieldsToPropSheet()
{
  std::vector<stringT> names;

  PWSprefs *prefs = PWSprefs::GetInstance();
  
  // Populate the group combo box
  m_Core.GetAllGroups(names);
  for (auto const& name : names) {
    m_BasicGroupNamesCtrl->Append(name);
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

    const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
    ASSERT(pbci);
    if (pbci) {
      m_Password = L"[" +
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

  m_BasicPasswordTextCtrl->ChangeValue(m_Password.c_str());
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
    PWHistList pwhl;
    size_t pwh_max, num_err;

    const StringX pwh_str = m_Item.GetPWHistory();
    if (!pwh_str.empty()) {
      m_PasswordHistory = towxstring(pwh_str);
      m_KeepPasswordHistory = CreatePWHistoryList(pwh_str,
                                         pwh_max, num_err,
                                         pwhl, PWSUtil::TMC_LOCALE);
      if (size_t(m_AdditionalPasswordHistoryGrid->GetNumberRows()) < pwhl.size()) {
        m_AdditionalPasswordHistoryGrid->AppendRows(pwhl.size() - m_AdditionalPasswordHistoryGrid->GetNumberRows());
      }
      m_MaxPasswordHistory = int(pwh_max);
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
  UpdateExpTimes();
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
  if (Validate() && TransferDataFromWindow()) {
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
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void AddEditPropSheetDlg::OnShowHideClick(wxCommandEvent& WXUNUSED(evt))
{
  m_Password = m_BasicPasswordTextCtrl->GetValue().c_str(); // save visible password
  if (m_IsPasswordHidden) {
    ShowPassword();
  } else {
    HidePassword();
  }
}

void AddEditPropSheetDlg::ShowPassword()
{
  m_IsPasswordHidden = false;
  m_BasicShowHideCtrl->SetLabel(_("&Hide"));

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  wxTextCtrl *tmp = m_BasicPasswordTextCtrl;
  const wxString pwd = m_Password.c_str();
  m_BasicPasswordTextCtrl = new wxTextCtrl(m_BasicPanel, ID_TEXTCTRL_PASSWORD,
                                  pwd,
                                  wxDefaultPosition, wxDefaultSize,
                                  0);
  if (!pwd.IsEmpty()) {
    m_BasicPasswordTextCtrl->ChangeValue(pwd);
    m_BasicPasswordTextCtrl->SetModified(true);
  }
  ApplyFontPreference(m_BasicPasswordTextCtrl, PWSprefs::StringPrefs::PasswordFont);
  m_BasicPasswordTextCtrl->MoveAfterInTabOrder(m_BasicUsernameTextCtrl);
  m_BasicSizer->Replace(tmp, m_BasicPasswordTextCtrl);
  delete tmp;
  m_BasicSizer->Layout();
  // Disable confirmation Ctrl, as the user can see the password entered
  ApplyFontPreference(m_BasicPasswordConfirmationTextCtrl, PWSprefs::StringPrefs::PasswordFont);
  m_BasicPasswordConfirmationTextCtrl->Clear();
  m_BasicPasswordConfirmationTextCtrl->Enable(false);
}

void AddEditPropSheetDlg::HidePassword()
{
  m_IsPasswordHidden = true;
  m_BasicShowHideCtrl->SetLabel(_("&Show"));

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  // Need verification as the user can not see the password entered
  wxTextCtrl *tmp = m_BasicPasswordTextCtrl;
  const wxString pwd = m_Password.c_str();
  m_BasicPasswordTextCtrl = new wxTextCtrl(m_BasicPanel, ID_TEXTCTRL_PASSWORD,
                                  pwd,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxTE_PASSWORD);
  ApplyFontPreference(m_BasicPasswordTextCtrl, PWSprefs::StringPrefs::PasswordFont);
  m_BasicPasswordTextCtrl->MoveAfterInTabOrder(m_BasicUsernameTextCtrl);
  m_BasicSizer->Replace(tmp, m_BasicPasswordTextCtrl);
  delete tmp;
  m_BasicSizer->Layout();
  if (!pwd.IsEmpty()) {
    m_BasicPasswordTextCtrl->ChangeValue(pwd);
    m_BasicPasswordTextCtrl->SetModified(true);
  }
  ApplyFontPreference(m_BasicPasswordConfirmationTextCtrl, PWSprefs::StringPrefs::PasswordFont);
  m_BasicPasswordConfirmationTextCtrl->ChangeValue(pwd);
  m_BasicPasswordConfirmationTextCtrl->Enable(true);
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

void AddEditPropSheetDlg::OnOk(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    time_t t;
    const wxString group = m_BasicGroupNamesCtrl->GetValue();
    const StringX password = tostringx(m_BasicPasswordTextCtrl->GetValue());

    /////////////////////////////////////////////////////////////////////////////
    // Tab: "Basic"
    /////////////////////////////////////////////////////////////////////////////

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
      return;
    }

    if (m_IsPasswordHidden) { // hidden passwords - compare both values
      const StringX secondPassword = tostringx(m_BasicPasswordConfirmationTextCtrl->GetValue());

      if (password != secondPassword) {
        wxMessageDialog msg(
          this,
          _("Passwords do not match"),
          _("Error"),
          wxOK|wxICON_ERROR
        );
        msg.ShowModal();
        return;
      }
    }

    /////////////////////////////////////////////////////////////////////////////
    // Tab: "Password Policy"
    /////////////////////////////////////////////////////////////////////////////

    if (m_PasswordPolicyUseDatabaseCtrl->GetValue() && (m_PasswordPolicyNamesCtrl->GetValue().IsEmpty())) {
      wxMessageDialog msg(
        this,
        _("Database name must not be empty if a database policy shall be used."),
        _("Error"),
        wxOK|wxICON_ERROR
      );
      msg.ShowModal();
      return;
    }

    switch (m_Type) {
      case SheetType::EDIT: {
        bool bIsModified, bIsPSWDModified;
        short lastDCA, lastShiftDCA;
        const PWSprefs *prefs = PWSprefs::GetInstance();
        m_Item.GetDCA(lastDCA);
        m_DoubleClickAction = GetSelectedDCA(m_AdditionalDoubleClickActionCtrl, lastDCA,
                              short(prefs->GetPref(PWSprefs::DoubleClickAction)));

        m_Item.GetShiftDCA(lastShiftDCA);
        m_ShiftDoubleClickAction = GetSelectedDCA(m_AdditionalShiftDoubleClickActionCtrl, lastShiftDCA,
                                    short(prefs->GetPref(PWSprefs::ShiftDoubleClickAction)));
        // Check if modified
        int lastXTimeInt;
        m_Item.GetXTimeInt(lastXTimeInt);
        time_t lastXtime;
        m_Item.GetXTime(lastXtime);
        // Following ensures that untouched & hidden note
        // isn't marked as modified. Relies on fact that
        // Note field can't be modified w/o first getting focus
        // and that we turn off m_isNotesHidden when that happens.
        if (m_Type != SheetType::ADD && m_IsNotesHidden)
          m_Notes = m_Item.GetNotes(TCHAR('\n')).c_str();

        // Create a new PWHistory string based on settings in this dialog, and compare it
        // with the PWHistory string from the item being edited, to see if the user modified it.
        // Note that we are not erasing the history here, even if the user has chosen to not
        // track PWHistory.  So there could be some password entries in the history
        // but the first byte could be zero, meaning we are not tracking it _FROM_NOW_.
        // Clearing the history is something the user must do himself with the "Clear History" button

        // First, Get a list of all password history entries
        size_t pwh_max, num_err;
        PWHistList pwhl;
        (void)CreatePWHistoryList(tostringx(m_PasswordHistory), pwh_max, num_err,
                                  pwhl, PWSUtil::TMC_LOCALE);

        // Create a new PWHistory header, as per settings in this dialog
        size_t numEntries = std::min(pwhl.size(), static_cast<size_t>(m_MaxPasswordHistory));
        m_PasswordHistory = towxstring(MakePWHistoryHeader(m_KeepPasswordHistory, m_MaxPasswordHistory, numEntries));
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
          m_PasswordHistory += towxstring(buffer);
        }

        wxASSERT_MSG(numEntries ==0, wxT("Could not save existing password history entries"));

        PWPolicy oldPWP, pwp;
        // get item's effective policy:
        const StringX oldPolName = m_Item.GetPolicyName();
        if (oldPolName.empty()) { // either item-specific or default:
          if (m_Item.GetPWPolicy().empty()) {
            oldPWP = PWSprefs::GetInstance()->GetDefaultPolicy();
          }
          else {
            m_Item.GetPWPolicy(oldPWP);
          }
        }
        else {
          m_Core.GetPolicyFromName(oldPolName, oldPWP);
        }
        // now get dbox's effective policy:
        pwp = GetSelectedPWPolicy();

        bIsModified = (
          group                     != m_Item.GetGroup().c_str()            ||
          m_Title                   != m_Item.GetTitle().c_str()            ||
          m_User                    != m_Item.GetUser().c_str()             ||
          m_Notes                   != m_Item.GetNotes(TCHAR('\n')).c_str() ||
          m_Url                     != m_Item.GetURL().c_str()              ||
          m_Email                   != m_Item.GetEmail().c_str()            ||
          m_Autotype                != m_Item.GetAutoType().c_str()         ||
          m_RunCommand              != m_Item.GetRunCommand().c_str()       ||
          m_DoubleClickAction       != lastDCA                              ||
          m_ShiftDoubleClickAction  != lastShiftDCA                         ||
          m_PasswordHistory         != m_Item.GetPWHistory().c_str()        ||
          m_tttExpirationTime       != lastXtime                            ||
          m_ExpirationTimeInterval  != lastXTimeInt                         ||
          m_Symbols                 != m_Item.GetSymbols().c_str()          ||
          oldPWP                    != pwp
        );

        /*
          Fixme:
          Even if there have been no changes and the user has pressed the Ok button,
          the password history check and symbols check result to 'true'.
        */

        if (!m_Item.IsAlias()) {
          bIsPSWDModified = (password != m_Item.GetPassword());
        }
        else {
          // Update password to alias form
          // Show text stating that it is an alias
          const CItemData *pbci = m_Core.GetBaseEntry(&m_Item);
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
          m_Item.SetGroup(tostringx(group));
          m_Item.SetTitle(tostringx(m_Title));
          m_Item.SetUser(m_User.empty() ?
                        PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DefaultUsername).c_str() : m_User.c_str());
          m_Item.SetNotes(tostringx(m_Notes));
          m_Item.SetURL(tostringx(m_Url));
          m_Item.SetEmail(tostringx(m_Email));
          m_Item.SetAutoType(tostringx(m_Autotype));
          m_Item.SetRunCommand(tostringx(m_RunCommand));
          m_Item.SetPWHistory(tostringx(m_PasswordHistory));

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
            m_Item.SetPWPolicy(pwp);

            // Remove database policy information from item
            m_Item.SetPolicyName(tostringx(wxEmptyString));
          }
          m_Item.SetDCA(m_DoubleClickAction);
          m_Item.SetShiftDCA(m_ShiftDoubleClickAction);
          // Check for Group/Username/Title uniqueness
          auto listindex = m_Core.Find(m_Item.GetGroup(), m_Item.GetTitle(), m_Item.GetUser());
          if (listindex != m_Core.GetEntryEndIter()) {
            auto listItem = m_Core.GetEntry(listindex);
            if (listItem.GetUUID() != m_Item.GetUUID()) {
              wxMessageDialog msg(
                this,
                _("An entry or shortcut with the same Group, Title and Username already exists."),
                _("Error"),
                wxOK|wxICON_ERROR
              );
              msg.ShowModal();
              return;
            }
          }
        } // bIsModified

        time(&t);
        if (bIsPSWDModified) {
          m_Item.UpdatePassword(password);
          m_Item.SetPMTime(t);
        }
        if (bIsModified || bIsPSWDModified) {
          m_Item.SetRMTime(t);
        }
        if (m_tttExpirationTime != lastXtime) {
          m_Item.SetXTime(m_tttExpirationTime);
        }
        if (m_Recurring) {
          if (m_ExpirationTimeInterval != lastXTimeInt) {
            m_Item.SetXTimeInt(m_ExpirationTimeInterval);
          }
        } else {
          m_Item.SetXTimeInt(0);
        }

        /////////////////////////////////////////////////////////////////////////////
        // Tab: "Attachment"
        /////////////////////////////////////////////////////////////////////////////

        bool isAttachmentModified = false;

        if (m_Core.GetReadFileVersion() == PWSfile::V40) {

          if (m_Item.HasAttRef()) {
            auto uuid = m_Item.GetAttUUID();
            auto itemAttachment = m_Core.GetAtt(uuid);

            auto hasTitleChanges = (towxstring(m_ItemAttachment.GetTitle()) != m_AttachmentTitle->GetValue());
            auto hasAttachmentChanges = (m_ItemAttachment != itemAttachment);

            isAttachmentModified = (
              hasTitleChanges ||
              hasAttachmentChanges
            );
          }

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
              2) Associate the attachment with the item (CItemData).
              3) Add attachment (CItemAtt) to the core.
              4) Update item's status.
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
            m_Item.SetAttUUID(m_ItemAttachment.GetUUID());
            m_ItemAttachment.IncRefcount();

            // Step 3)
            m_Core.PutAtt(m_ItemAttachment);

            // Step 4)
            m_ItemAttachment.SetStatus(CItem::EntryStatus::ES_ADDED);
            m_Item.SetStatus(CItem::EntryStatus::ES_MODIFIED);
          }

          /*
            Case: Item has an attachment which shall be removed.
            Steps:
              1) Detach the attachment (CItemAtt) from the item.
              2) Remove the attachment from the core.
              3) Update item's status.
          */
          else if (m_Item.HasAttRef() && !m_ItemAttachment.HasUUID() && !m_ItemAttachment.HasContent()) {

            // Step 1)
            auto uuid = m_Item.GetAttUUID();
            auto itemAttachment = m_Core.GetAtt(uuid);
            m_Item.ClearAttUUID();
            itemAttachment.DecRefcount();

            // Step 2)
            m_Core.RemoveAtt(itemAttachment.GetUUID());

            // Step 3)
            m_Item.SetStatus(CItem::EntryStatus::ES_MODIFIED);
          }

          /*
            Case: Item has an attachment which shall be replaced by a new one.
            Steps:
              1) Update attachment meta data.
              2) Detach the attachment (CItemAtt) from the item.
              3) Remove the attachment from the Core.
              4) Associate the new attachment with the item (CItemData).
              5) Add new attachment (CItemAtt) to the core.
              6) Update item's status.
          */
          else if (isAttachmentModified) {

            // Step 1)
            if (m_AttachmentTitle->GetValue() != _T("N/A")) {
              m_ItemAttachment.SetTitle(std2stringx(stringT(m_AttachmentTitle->GetValue())));
            }

            time_t timestamp;
            time(&timestamp);
            m_ItemAttachment.SetCTime(timestamp);

            // Step 2)
            auto uuid = m_Item.GetAttUUID();
            auto itemAttachment = m_Core.GetAtt(uuid);
            m_Item.ClearAttUUID();
            itemAttachment.DecRefcount();

            // Step 3)
            m_Core.RemoveAtt(itemAttachment.GetUUID());

            // Step 4)
            m_Item.SetAttUUID(m_ItemAttachment.GetUUID());
            m_ItemAttachment.IncRefcount();

            // Step 5)
            m_Core.PutAtt(m_ItemAttachment);

            // Step 6)
            m_Item.SetStatus(CItem::EntryStatus::ES_MODIFIED);
          }
          else {
            ;
          }
        }

        if (bIsModified || bIsPSWDModified || isAttachmentModified) {
          // All fields in m_item now reflect user's edits
          // Let's update the core's data
          uuid_array_t uuid;
          m_Item.GetUUID(uuid);
          auto listpos = m_Core.Find(uuid);
          ASSERT(listpos != m_Core.GetEntryEndIter());
          m_Core.Execute(EditEntryCommand::Create(&m_Core,
                                                  m_Core.GetEntry(listpos),
                                                  m_Item));
        }
      }
      break;
      case SheetType::ADD: {
        m_Item.SetGroup(tostringx(group));
        m_Item.SetTitle(tostringx(m_Title));
        m_Item.SetUser(m_User.empty() ?
                      PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DefaultUsername).c_str() : m_User.c_str());
        // Check for Group/Username/Title uniqueness
        if (m_Core.Find(m_Item.GetGroup(), m_Item.GetTitle(), m_Item.GetUser()) !=
            m_Core.GetEntryEndIter()) {
          wxMessageDialog msg(
            this,
            _("An entry or shortcut with the same Group, Title and Username already exists."),
            _("Error"),
            wxOK|wxICON_ERROR
          );
          msg.ShowModal();
          return;
        }
        m_Item.SetNotes(tostringx(m_Notes));
        m_Item.SetURL(tostringx(m_Url));
        m_Item.SetEmail(tostringx(m_Email));
        m_Item.SetPassword(password);
        m_Item.SetAutoType(tostringx(m_Autotype));
        m_Item.SetRunCommand(tostringx(m_RunCommand));
        m_Item.SetDCA(m_DoubleClickAction);
        m_Item.SetShiftDCA(m_ShiftDoubleClickAction);
        time(&t);
        m_Item.SetCTime(t);
        if (m_KeepPasswordHistory) {
          m_Item.SetPWHistory(MakePWHistoryHeader(true, m_MaxPasswordHistory, 0));
        }
        m_Item.SetXTime(m_tttExpirationTime);
        if (m_ExpirationTimeInterval > 0 && m_ExpirationTimeInterval <= 3650) {
          m_Item.SetXTimeInt(m_ExpirationTimeInterval);
        }
        if (!m_PasswordPolicyUseDatabaseCtrl->GetValue()) {
          m_Item.SetPWPolicy(GetPWPolicyFromUI());
        }
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
        }
        else {
          m_item.SetPassword(m_AEMD.realpassword);
          m_item.SetNormal();
        }
  #endif
        if (m_Item.IsAlias()) {
          m_Item.SetXTime(time_t(0));
          m_Item.SetPWPolicy(wxEmptyString);
        }
        else {
          m_Item.SetXTime(m_tttExpirationTime);
        }

        /////////////////////////////////////////////////////////////////////////////
        // Tab: "Attachment"
        /////////////////////////////////////////////////////////////////////////////

        if (m_Core.GetReadFileVersion() == PWSfile::V40) {

          /*
            Case: New item shall get an attachment.
            Steps:
              1) Update attachment meta data.
              2) Associate the attachment with the item (CItemData).
              3) Add attachment (CItemAtt) to the core.
              4) Update item's status.
          */
          if (!m_Item.HasAttRef() && m_ItemAttachment.HasUUID() && m_ItemAttachment.HasContent()) {

            // Step 1)
            if (m_AttachmentTitle->GetValue() != _T("N/A")) {
              m_ItemAttachment.SetTitle(std2stringx(stringT(m_AttachmentTitle->GetValue())));
            }

            time_t timestamp;
            time(&timestamp);
            m_ItemAttachment.SetCTime(timestamp);

            // Step 2)
            m_Item.SetAttUUID(m_ItemAttachment.GetUUID());
            m_ItemAttachment.IncRefcount();

            // Step 3)
            m_Core.PutAtt(m_ItemAttachment);

            // Step 4)
            m_ItemAttachment.SetStatus(CItem::EntryStatus::ES_ADDED);
            m_Item.SetStatus(CItem::EntryStatus::ES_MODIFIED);
          }
        }
      }
      break;
      case SheetType::VIEW: {
        // No Update
      }
      break;
      default: {
        ASSERT(0);
        break;
      }
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

#if 0 // XXX Remove, as we did away with this checkbox!
void AddEditPropSheetDlg::OnOverrideDCAClick(wxCommandEvent& WXUNUSED(evt))
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

void AddEditPropSheetDlg::SetXTime(wxObject *src)
{
  if (Validate() && TransferDataFromWindow()) {
    wxDateTime xdt;
    if (src == m_DatesTimesExpiryDateCtrl) { // expiration date changed, update interval
      xdt = m_DatesTimesExpiryDateCtrl->GetValue();
      xdt.SetHour(0);
      xdt.SetMinute(1);
      wxTimeSpan delta = xdt.Subtract(wxDateTime::Today());
      m_ExpirationTimeInterval = delta.GetDays();
      m_ExpirationTime = xdt.FormatDate();
    } else if (src == m_DatesTimesExpiryTimeCtrl) { // expiration interval changed, update date
      // If it's a non-recurring interval, just set XTime to
      // now + interval, XTimeInt should be stored as zero
      // (one-shot semantics)
      // Otherwise, XTime += interval, keep XTimeInt
        xdt = wxDateTime::Now();
        xdt += wxDateSpan(0, 0, 0, m_ExpirationTimeInterval);
        m_DatesTimesExpiryDateCtrl->SetValue(xdt);
        m_ExpirationTime = xdt.FormatDate();
      if (m_Recurring) {
        wxString rstr;
        rstr.Printf(_(" (every %d days)"), m_ExpirationTimeInterval);
        m_ExpirationTime += rstr;
      }
    } else {
      ASSERT(0);
    }
    m_tttExpirationTime = xdt.GetTicks();
    Validate(); TransferDataToWindow();
  } // Validated & transferred from controls
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_ON
 */

void AddEditPropSheetDlg::OnExpRadiobuttonSelected( wxCommandEvent& evt )
{
  bool On = (evt.GetEventObject() == m_DatesTimesExpireOnCtrl);
  bool Never = (evt.GetEventObject() == m_DatesTimesNeverExpireCtrl);

  if (Never) {
    m_ExpirationTime = _("Never");
    m_CurrentExpirationTime.Clear();
    m_tttExpirationTime = time_t(0);
    m_ExpirationTimeInterval = 90;
    wxDateTime xdt(wxDateTime::Now());
    xdt += wxDateSpan(0, 0, 0, m_ExpirationTimeInterval);
    m_DatesTimesExpiryDateCtrl->SetValue(xdt);
    m_Recurring = false;
    TransferDataToWindow();
  }

  m_DatesTimesExpiryDateCtrl->Enable(On && !Never);
  m_DatesTimesExpiryTimeCtrl->Enable(!On && !Never);
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

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
 */

void AddEditPropSheetDlg::OnEasyReadOrPronounceable(wxCommandEvent& evt)
{
 if (Validate() && TransferDataFromWindow()) {
   if (m_PasswordPolicyUseEasyCtrl->GetValue() && m_PasswordPolicyUsePronounceableCtrl->GetValue()) {
    wxMessageBox(_("Sorry, 'pronounceable' and 'easy-to-read' are not supported together"),
                        _("Password Policy"), wxOK | wxICON_EXCLAMATION, this);
    if (evt.GetEventObject() == m_PasswordPolicyUsePronounceableCtrl)
      m_PasswordPolicyUsePronounceableCtrl->SetValue(false);
    else
      m_PasswordPolicyUseEasyCtrl->SetValue(false);
   }
   else {
     ShowPWPSpinners(!evt.IsChecked());
   }
 }
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
  bool dbIsReadOnly = m_Core.IsReadOnly();

  switch (event.GetId()) {
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Basic"
  /////////////////////////////////////////////////////////////////////////////
    case ID_COMBOBOX_GROUP:
    case ID_BUTTON_GENERATE:
      event.Enable(!dbIsReadOnly);
      break;
    case ID_TEXTCTRL_TITLE:
    case ID_TEXTCTRL_USERNAME:
    case ID_TEXTCTRL_PASSWORD:
    case ID_TEXTCTRL_PASSWORD2:
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
      event.Enable(!dbIsReadOnly);
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

PWPolicy AddEditPropSheetDlg::GetPWPolicyFromUI()
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

PWPolicy AddEditPropSheetDlg::GetSelectedPWPolicy()
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
    m_PasswordHistory = towxstring(MakePWHistoryHeader(m_KeepPasswordHistory, m_MaxPasswordHistory, 0));
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
      wxMessageBox(_("Sorry, \"easy-to-read\" and \"pronounceable\" cannot be both selected"),
                   _("Error"), wxOK|wxICON_ERROR, this);
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
      wxMessageBox(_("Sorry, \"pronounceable\" and \"easy-to-read\" cannot be both selected"),
                   _("Error"), wxOK|wxICON_ERROR, this);
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
