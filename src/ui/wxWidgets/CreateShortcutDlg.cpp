/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file CreateShortcutDlg.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/valgen.h>

#include "CreateShortcutDlg.h"
#include "wxUtilities.h"

//(*IdInit(CreateShortcutDlg)
const long CreateShortcutDlg::ID_COMBOBOX1 = wxNewId();
const long CreateShortcutDlg::ID_TEXTCTRL1 = wxNewId();
const long CreateShortcutDlg::ID_TEXTCTRL2 = wxNewId();
const long CreateShortcutDlg::ID_STATICTEXT7 = wxNewId();
const long CreateShortcutDlg::ID_STATICTEXT8 = wxNewId();
const long CreateShortcutDlg::ID_STATICTEXT9 = wxNewId();
//*)

/*!
 * CreateShortcutDlg type definition
 */

IMPLEMENT_CLASS( CreateShortcutDlg, wxDialog )

/*!
 * CreateShortcutDlg event table definition
 */

BEGIN_EVENT_TABLE( CreateShortcutDlg, wxDialog )

  EVT_BUTTON( wxID_OK, CreateShortcutDlg::OnOk )

END_EVENT_TABLE()

/*!
 * CreateShortcutDlg constructors
 */

CreateShortcutDlg::CreateShortcutDlg(wxWindow* parent, PWScore &core, CItemData *base)
: m_Core(core), m_Base(base)
{
  ASSERT(m_Base != nullptr);
  Init();
  Create(parent);
}

/*!
 * CreateShortcutDlg destructor
 */

CreateShortcutDlg::~CreateShortcutDlg()
{
}

/*!
 * CreateShortcutDlg creator
 */

bool CreateShortcutDlg::Create(wxWindow* parent)
{
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Create Shortcut"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
  UpdateControls();
  ItemFieldsToDialog();
  return true;
}

void CreateShortcutDlg::ItemFieldsToDialog()
{
  // Populate the combo box
  std::vector<stringT> allGroupNames;

  m_Core.GetAllGroups(allGroupNames, false);

  for (auto const& groupName : allGroupNames) {
    m_ComboBoxShortcutGroup->Append(groupName);
  }

  if (m_Base != nullptr) {
    m_BaseEntryTitle = stringx2std(m_Base->GetTitle());
    m_BaseEntryUsername = stringx2std(m_Base->GetUser());

    m_ShortcutTitle = _("Shortcut to ") + m_BaseEntryTitle;
    m_ShortcutUsername = m_BaseEntryUsername;

    if (m_Base->IsGroupSet() && !(m_Base->GetGroup().empty())) {
      m_BaseEntryGroup = stringx2std(m_Base->GetGroup());

      auto position = m_ComboBoxShortcutGroup->FindString(m_BaseEntryGroup);

      if (position != wxNOT_FOUND) {
        m_ShortcutGroup = m_BaseEntryGroup;
      }
      else {
        m_ShortcutGroup = wxEmptyString;
      }
    }
    else {
      m_BaseEntryGroup = wxEmptyString;
    }
  }
  else {
    m_ShortcutGroup = wxEmptyString;
    m_ShortcutTitle = wxEmptyString;
    m_ShortcutUsername = wxEmptyString;

    m_BaseEntryGroup = _("N/A");
    m_BaseEntryTitle = _("N/A");
    m_BaseEntryUsername = _("N/A");
  }
}

void CreateShortcutDlg::SetValidators()
{
  m_ComboBoxShortcutGroup->SetValidator(wxGenericValidator(&m_ShortcutGroup));
  m_TextCtrlShortcutTitle->SetValidator(wxGenericValidator(&m_ShortcutTitle));
  m_TextCtrlShortcutUsername->SetValidator(wxGenericValidator(&m_ShortcutUsername));

  m_StaticTextBaseEntryGroup->SetValidator(wxGenericValidator(&m_BaseEntryGroup));
  m_StaticTextBaseEntryTitle->SetValidator(wxGenericValidator(&m_BaseEntryTitle));
  m_StaticTextBaseEntryUsername->SetValidator(wxGenericValidator(&m_BaseEntryUsername));
}

void CreateShortcutDlg::UpdateControls()
{
  if (m_Core.IsReadOnly()) {
    m_ComboBoxShortcutGroup->Disable();
    m_TextCtrlShortcutTitle->Disable();
    m_TextCtrlShortcutUsername->Disable();
  }
}

/*!
 * Member initialisation
 */

void CreateShortcutDlg::Init()
{
  m_ComboBoxShortcutGroup = nullptr;
  m_TextCtrlShortcutTitle = nullptr;
  m_TextCtrlShortcutUsername = nullptr;
  m_StaticTextBaseEntryGroup = nullptr;
  m_StaticTextBaseEntryTitle = nullptr;
  m_StaticTextBaseEntryUsername = nullptr;
}

/*!
 * Control creation for CreateShortcutDlg
 */

void CreateShortcutDlg::CreateControls()
{
  //(*Initialize(ShortcutsDialogDial
  auto BoxSizer1 = new wxBoxSizer(wxVERTICAL);
  auto StaticText1 = new wxStaticText(this, wxID_ANY, _("Please enter the shortcut properties to the selected base entry."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  BoxSizer1->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);

  auto StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Shortcut"));
  auto FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  FlexGridSizer1->AddGrowableCol(1);

  auto StaticTextShortcutGroup = new wxStaticText(this, wxID_ANY, _("Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutGroup, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_ComboBoxShortcutGroup = new wxComboBox(this, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN, wxDefaultValidator, _T("ID_COMBOBOX1"));
  FlexGridSizer1->Add(m_ComboBoxShortcutGroup, 1, wxALL|wxEXPAND, 5);

  auto StaticTextShortcutTitle = new wxStaticText(this, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutTitle, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_TextCtrlShortcutTitle = new wxTextCtrl(this, ID_TEXTCTRL1, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
  FlexGridSizer1->Add(m_TextCtrlShortcutTitle, 1, wxALL|wxEXPAND, 5);

  auto StaticTextShortcutUsername = new wxStaticText(this, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutUsername, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_TextCtrlShortcutUsername = new wxTextCtrl(this, ID_TEXTCTRL2, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
  FlexGridSizer1->Add(m_TextCtrlShortcutUsername, 1, wxALL|wxEXPAND, 5);

  StaticBoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
  BoxSizer1->Add(StaticBoxSizer1, 0, wxALL|wxEXPAND, 5);

  auto StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Base Entry"));
  auto FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
  FlexGridSizer2->AddGrowableCol(1);

  auto StaticTextBaseEntryGroup = new wxStaticText(this, wxID_ANY, _("Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer2->Add(StaticTextBaseEntryGroup, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextBaseEntryGroup = new wxStaticText(this, ID_STATICTEXT7, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
  FlexGridSizer2->Add(m_StaticTextBaseEntryGroup, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextBaseEntryTitle = new wxStaticText(this, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer2->Add(StaticTextBaseEntryTitle, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextBaseEntryTitle = new wxStaticText(this, ID_STATICTEXT8, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
  FlexGridSizer2->Add(m_StaticTextBaseEntryTitle, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextBaseEntryUsername = new wxStaticText(this, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer2->Add(StaticTextBaseEntryUsername, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextBaseEntryUsername = new wxStaticText(this, ID_STATICTEXT9, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
  FlexGridSizer2->Add(m_StaticTextBaseEntryUsername, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  StaticBoxSizer2->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
  BoxSizer1->Add(StaticBoxSizer2, 0, wxALL|wxEXPAND, 5);

  auto StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_HELP, wxEmptyString));
  StdDialogButtonSizer1->Realize();
  BoxSizer1->Add(StdDialogButtonSizer1, 0, wxALL|wxEXPAND, 5);

  SetSizer(BoxSizer1);
  BoxSizer1->Fit(this);
  BoxSizer1->SetSizeHints(this);
  //*)
}

/*!
 * Should we show tooltips?
 */

bool CreateShortcutDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CreateShortcutDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CreateShortcutDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CreateShortcutDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    bool valid = !m_ShortcutTitle.empty();

    if (!valid)
      return;

    CItemData shortcut;
    shortcut.SetShortcut();
    shortcut.CreateUUID();
    shortcut.SetPassword(wxT("[Shortcut]"));

    if (!m_ShortcutGroup.empty()) {
      shortcut.SetGroup(tostringx(m_ShortcutGroup));
    }

    shortcut.SetTitle(tostringx(m_ShortcutTitle));

    if (!m_ShortcutUsername.empty()) {
      shortcut.SetUser(tostringx(m_ShortcutUsername));
    }

    time_t t;
    time(&t);
    shortcut.SetCTime(t);
    shortcut.SetXTime(time_t(0));
    shortcut.SetStatus(CItemData::ES_ADDED);

    m_Core.Execute(
      AddEntryCommand::Create(&m_Core, shortcut, m_Base->GetUUID())
    );
  }
  EndModal(wxID_OK);
}
