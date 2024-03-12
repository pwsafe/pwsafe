/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file EditShortcutDlg.cpp
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

#include "wx/valgen.h"

#include "EditShortcutDlg.h"
#include "wxUtilities.h"

//(*IdInit(EditShortcutDlg)
const wxWindowID EditShortcutDlg::ID_COMBOBOX1 = wxNewId();
const wxWindowID EditShortcutDlg::ID_TEXTCTRL2 = wxNewId();
const wxWindowID EditShortcutDlg::ID_TEXTCTRL3 = wxNewId();
const wxWindowID EditShortcutDlg::ID_STATICTEXT6 = wxNewId();
const wxWindowID EditShortcutDlg::ID_STATICTEXT8 = wxNewId();
const wxWindowID EditShortcutDlg::ID_STATICTEXT10 = wxNewId();
const wxWindowID EditShortcutDlg::ID_STATICTEXT2 = wxNewId();
const wxWindowID EditShortcutDlg::ID_STATICTEXT4 = wxNewId();
const wxWindowID EditShortcutDlg::ID_STATICTEXT7 = wxNewId();
//*)

/*!
 * EditShortcutDlg type definition
 */

IMPLEMENT_CLASS( EditShortcutDlg, wxDialog )

/*!
 * EditShortcutDlg event table definition
 */

BEGIN_EVENT_TABLE( EditShortcutDlg, wxDialog )

  EVT_BUTTON( wxID_OK, EditShortcutDlg::OnOk )
  EVT_CLOSE( EditShortcutDlg::OnClose )
  EVT_BUTTON( wxID_CANCEL, EditShortcutDlg::OnCancelClick )

END_EVENT_TABLE()

/*!
 * EditShortcutDlg constructors
 */

EditShortcutDlg::EditShortcutDlg(wxWindow *parent, PWScore &core, CItemData *shortcut)
: m_Core(core), m_Shortcut(shortcut)
{
  ASSERT(m_Shortcut != nullptr);
  wxASSERT(!parent || parent->IsTopLevel());

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Edit Shortcut"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
  UpdateControls();
  ItemFieldsToDialog();

}

EditShortcutDlg* EditShortcutDlg::Create(wxWindow *parent, PWScore &core, CItemData *shortcut)
{
  return new EditShortcutDlg(parent, core, shortcut);
}

void EditShortcutDlg::ItemFieldsToDialog()
{
  // Populate the combo box
  std::vector<stringT> allGroupNames;
  wxSize actSize = m_ComboBoxShortcutGroup->GetSize(), oldSize = actSize, borderSize = m_ComboBoxShortcutGroup->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord width, height, border = (borderSize.GetWidth() * 2) + 2;
  
  dc.SetFont(m_ComboBoxShortcutGroup->GetFont());
  
  m_Core.GetAllGroups(allGroupNames, true);  // Also allow short cut in empty groups

  m_ComboBoxShortcutGroup->Append(""); // Also allow selection of emtpy group
  for (auto const& groupName : allGroupNames) {
    m_ComboBoxShortcutGroup->Append(groupName);
    dc.GetTextExtent(groupName, &width, &height);
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

  if (m_Shortcut != nullptr) {

    // Set shortcut group
    if (m_Shortcut->IsGroupSet() && !(m_Shortcut->GetGroup()).empty()) {

      const auto shortcutGroupName = m_Shortcut->GetGroup();

      auto position = m_ComboBoxShortcutGroup->FindString(stringx2std(shortcutGroupName));

      if (position != wxNOT_FOUND) {
        m_ShortcutGroup = stringx2std(shortcutGroupName);
        m_ComboBoxShortcutGroup->SetSelection(position);
      }
      else {
        m_ShortcutGroup = wxEmptyString;
      }
    }
    else {
      m_ShortcutGroup = wxEmptyString;
    }

    // Set shortcut title
    m_ShortcutTitle = stringx2std(m_Shortcut->GetTitle());

    // Set shortcut username
    m_ShortcutUsername = stringx2std(m_Shortcut->GetUser());

    // Set shortcut created on date/time
    m_ShortcutCreated = stringx2std(m_Shortcut->GetCTimeL());

    // Set shortcut's base entry last changed on date/time
    const auto base = m_Core.GetBaseEntry(m_Shortcut);

    if (base != nullptr) {
      m_ShortcutChanged = stringx2std(base->GetRMTimeL());
    }
    else {
      m_ShortcutChanged = _("N/A");
    }

    // Set shortcut last accessed on date/time
    m_ShortcutAccessed = stringx2std(m_Shortcut->GetATimeL());

    // Set shortcut any field last changed on date/time
    m_ShortcutAnyChange = stringx2std(m_Shortcut->GetRMTimeL());

    if (base != nullptr) {
      // Set base entry group
      m_BaseEntryGroup = stringx2std(base->GetGroup());

      // Set base entry title
      m_BaseEntryTitle = stringx2std(base->GetTitle());

      // Set base entry username
      m_BaseEntryUsername = stringx2std(base->GetUser());
    }
    else {
      m_BaseEntryGroup = _("N/A");
      m_BaseEntryTitle = _("N/A");
      m_BaseEntryUsername = _("N/A");
    }
  }
  else {
    m_ShortcutGroup = wxEmptyString;
    m_ShortcutTitle = wxEmptyString;
    m_ShortcutUsername = wxEmptyString;

    m_ShortcutCreated = _("N/A");
    m_ShortcutChanged = _("N/A");
    m_ShortcutAccessed = _("N/A");
    m_ShortcutAnyChange = _("N/A");

    m_BaseEntryGroup = _("N/A");
    m_BaseEntryTitle = _("N/A");
    m_BaseEntryUsername = _("N/A");
  }
}

void EditShortcutDlg::SetValidators()
{
  m_ComboBoxShortcutGroup->SetValidator(wxGenericValidator(&m_ShortcutGroup));
  m_TextCtrlShortcutTitle->SetValidator(wxGenericValidator(&m_ShortcutTitle));
  m_TextCtrlShortcutUsername->SetValidator(wxGenericValidator(&m_ShortcutUsername));

  m_StaticTextShortcutCreated->SetValidator(wxGenericValidator(&m_ShortcutCreated));
  m_StaticTextShortcutChanged->SetValidator(wxGenericValidator(&m_ShortcutChanged));
  m_StaticTextShortcutAccessed->SetValidator(wxGenericValidator(&m_ShortcutAccessed));
  m_StaticTextShortcutAnyChange->SetValidator(wxGenericValidator(&m_ShortcutAnyChange));

  m_StaticTextBaseEntryGroup->SetValidator(wxGenericValidator(&m_BaseEntryGroup));
  m_StaticTextBaseEntryTitle->SetValidator(wxGenericValidator(&m_BaseEntryTitle));
  m_StaticTextBaseEntryUsername->SetValidator(wxGenericValidator(&m_BaseEntryUsername));
}

void EditShortcutDlg::UpdateControls()
{
  if (m_Core.IsReadOnly()) {
    m_ComboBoxShortcutGroup->Disable();
    m_TextCtrlShortcutTitle->Disable();
    m_TextCtrlShortcutUsername->Disable();
  }
}

/*!
 * Control creation for EditShortcutDlg
 */

void EditShortcutDlg::CreateControls()
{
  //(*Initialize(EditShortcutDlgDialog)
  auto BoxSizer1 = new wxBoxSizer(wxVERTICAL);
  auto StaticText1 = new wxStaticText(this, wxID_ANY, _("Edit the shortcut properties to the selected base entry."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  BoxSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);

  auto StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Shortcut"));
  auto FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  FlexGridSizer1->AddGrowableCol(1);

  auto StaticTextShortcutGroup = new wxStaticText(this, wxID_ANY, _("Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutGroup, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_ComboBoxShortcutGroup = new wxComboBox(this, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN, wxDefaultValidator, _T("ID_COMBOBOX1"));
  FlexGridSizer1->Add(m_ComboBoxShortcutGroup, 1, wxALL|wxEXPAND, 5);

  auto StaticTextShortcutTitle = new wxStaticText(this, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutTitle, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_TextCtrlShortcutTitle = new wxTextCtrl(this, ID_TEXTCTRL2, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
  FlexGridSizer1->Add(m_TextCtrlShortcutTitle, 1, wxALL|wxEXPAND, 5);

  auto StaticTextShortcutUsername = new wxStaticText(this, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutUsername, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_TextCtrlShortcutUsername = new wxTextCtrl(this, ID_TEXTCTRL3, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
  FlexGridSizer1->Add(m_TextCtrlShortcutUsername, 1, wxALL|wxEXPAND, 5);

  auto StaticTextShortcutCreated = new wxStaticText(this, wxID_ANY, _("Created on:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutCreated, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextShortcutCreated = new wxStaticText(this, wxID_ANY, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(m_StaticTextShortcutCreated, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextShortcutChanged = new wxStaticText(this, wxID_ANY, _("Target last changed on:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutChanged, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextShortcutChanged = new wxStaticText(this, ID_STATICTEXT6, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
  FlexGridSizer1->Add(m_StaticTextShortcutChanged, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextShortcutAccessed = new wxStaticText(this, wxID_ANY, _("Last accessed on:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutAccessed, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextShortcutAccessed = new wxStaticText(this, ID_STATICTEXT8, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
  FlexGridSizer1->Add(m_StaticTextShortcutAccessed, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextShortcutAnyChanged = new wxStaticText(this, wxID_ANY, _("Any field last changed on:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticTextShortcutAnyChanged, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextShortcutAnyChange = new wxStaticText(this, ID_STATICTEXT10, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
  FlexGridSizer1->Add(m_StaticTextShortcutAnyChange, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
  StaticBoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
  BoxSizer1->Add(StaticBoxSizer1, 0, wxALL|wxEXPAND, 5);

  auto StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Base Entry"));
  auto FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
  FlexGridSizer2->AddGrowableCol(1);

  auto StaticTextBaseEntryGroup = new wxStaticText(this, wxID_ANY, _("Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer2->Add(StaticTextBaseEntryGroup, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextBaseEntryGroup = new wxStaticText(this, ID_STATICTEXT2, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
  FlexGridSizer2->Add(m_StaticTextBaseEntryGroup, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextBaseEntryTitle = new wxStaticText(this, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer2->Add(StaticTextBaseEntryTitle, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextBaseEntryTitle = new wxStaticText(this, ID_STATICTEXT4, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
  FlexGridSizer2->Add(m_StaticTextBaseEntryTitle, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

  auto StaticTextBaseEntryUsername = new wxStaticText(this, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer2->Add(StaticTextBaseEntryUsername, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_StaticTextBaseEntryUsername = new wxStaticText(this, ID_STATICTEXT7, _("N/A"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
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

bool EditShortcutDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap EditShortcutDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon EditShortcutDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void EditShortcutDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    const auto changes = GetChanges();
    if (changes != Changes::None) {
      CItemData modifiedShortcut(*m_Shortcut);

      // Has group changed?
      if (changes & Changes::Group) {
        modifiedShortcut.SetGroup(tostringx(m_ShortcutGroup));
      }

      // Has title changed?
      if (changes & Changes::Title) {
        modifiedShortcut.SetTitle(tostringx(m_ShortcutTitle));
      }

      // Has username changed?
      if (changes & Changes::User) {
        modifiedShortcut.SetUser(tostringx(m_ShortcutUsername));
      }

      {
        time_t t;
        time(&t);

        modifiedShortcut.SetRMTime(t);
        modifiedShortcut.SetStatus(CItemData::ES_MODIFIED);
      }

      m_Core.Execute(
        EditEntryCommand::Create(&m_Core, *m_Shortcut, modifiedShortcut)
      );
    }
  }
  EndModal(wxID_OK);
}

bool EditShortcutDlg::SyncAndQueryCancel(bool showDialog) {
  // when edit forbidden, allow cancel without additional checks
  if (m_Core.IsReadOnly()) {
    return true;
  }
  return QueryCancelDlg::SyncAndQueryCancel(showDialog);
}

uint32_t EditShortcutDlg::GetChanges() const {
  uint32_t changes = Changes::None;
  if (tostringx(m_ShortcutTitle) != m_Shortcut->GetTitle()) {
      changes |= Changes::Title;
  }
  if (tostringx(m_ShortcutUsername) != m_Shortcut->GetUser()) {
    changes |= Changes::User;
  }
    
  if (tostringx(m_ShortcutGroup) != m_Shortcut->GetGroup()) {
    changes |= Changes::Group;
  }
  return changes;
}

bool EditShortcutDlg::IsChanged() const {
  return GetChanges() != Changes::None;
}
