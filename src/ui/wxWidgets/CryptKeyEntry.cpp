/*
 * CryptKeyEntry.cpp - initial version by rafaelx 2019-03-02
 * Copyright (c) 2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "CryptKeyEntry.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(CryptKeyEntry)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
//*)


//(*IdInit(CryptKeyEntry)
const long CryptKeyEntry::ID_TEXTCTRL_KEY1 = wxNewId();
const long CryptKeyEntry::ID_TEXTCTRL_KEY2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CryptKeyEntry, wxDialog)
  //(*EventTable(CryptKeyEntry)
  EVT_BUTTON( wxID_OK,     CryptKeyEntry::OnOk     )
  EVT_BUTTON( wxID_CANCEL, CryptKeyEntry::OnCancel )
  EVT_CLOSE(               CryptKeyEntry::OnClose  )
  //*)
END_EVENT_TABLE()

CryptKeyEntry::CryptKeyEntry(Mode mode)
{
    //(*Initialize(CryptKeyEntry)
    wxStaticText* StaticTextKey2;
    wxFlexGridSizer* FlexGridSizer1;
    wxStaticText* StaticTextKey1;
    wxBoxSizer* BoxSizer1;
    wxStaticText* StaticTextDescription;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    if (mode == Mode::ENCRYPT) {
      Create(nullptr, -1, _("Encrytion"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL, _T("id"));
      StaticTextDescription = new wxStaticText(this, wxID_ANY, _("Please enter an encryption key."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    }
    else {
      Create(nullptr, -1, _("Decryption"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL, _T("id"));
      StaticTextDescription = new wxStaticText(this, wxID_ANY, _("Please enter a decryption key."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    }

    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    BoxSizer1->Add(StaticTextDescription, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    StaticTextKey1 = new wxStaticText(this, wxID_ANY, _("Enter Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer1->Add(StaticTextKey1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlKey1 = new wxTextCtrl(this, ID_TEXTCTRL_KEY1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL_KEY1"));
    TextCtrlKey1->SetFocus();
    FlexGridSizer1->Add(TextCtrlKey1, 1, wxALL|wxEXPAND, 5);

    if (mode == Mode::ENCRYPT) {
      StaticTextKey2 = new wxStaticText(this, wxID_ANY, _("Verify Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
      FlexGridSizer1->Add(StaticTextKey2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
      TextCtrlKey2 = new wxTextCtrl(this, ID_TEXTCTRL_KEY2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL_KEY2"));
      FlexGridSizer1->Add(TextCtrlKey2, 1, wxALL|wxEXPAND, 5);
    }

    BoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
    auto okButton = new wxButton(this, wxID_OK, wxEmptyString);
    okButton->SetDefault();
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(okButton);
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    BoxSizer1->Add(StdDialogButtonSizer1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(BoxSizer1);
    BoxSizer1->Fit(this);
    BoxSizer1->SetSizeHints(this);
    //*)

    // Allow to resize the dialog only in width.
    SetMinSize(wxSize(GetMinSize().x * 1.5, GetMinSize().y));
    SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));

    m_Mode = mode;
}

CryptKeyEntry::~CryptKeyEntry()
{
  //(*Destroy(CryptKeyEntry)
  //*)
}

void CryptKeyEntry::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {

    if (m_Mode == Mode::ENCRYPT) {
      if (TextCtrlKey1->GetValue().IsEmpty() || TextCtrlKey2->GetValue().IsEmpty()) {
        wxMessageDialog messageBox(
          this, _("The combination cannot be blank."),
          _("Error"), wxOK | wxICON_EXCLAMATION
        );

        messageBox.ShowModal();
        return;
      }
      else if (TextCtrlKey1->GetValue() != TextCtrlKey2->GetValue()) {
        wxMessageDialog messageBox(
          this, _("The two entries do not match."),
          _("Error"), wxOK | wxICON_EXCLAMATION
        );

        messageBox.ShowModal();
        return;
      }
      else {
        m_CryptKey = TextCtrlKey1->GetValue();

        EndModal(wxID_OK);
      }
    }
    else {
      if (TextCtrlKey1->GetValue().IsEmpty()) {
        wxMessageDialog messageBox(
          this, _("The entry cannot be blank."),
          _("Error"), wxOK | wxICON_EXCLAMATION
        );

        messageBox.ShowModal();
        return;
      }
      else {
        m_CryptKey = TextCtrlKey1->GetValue();

        EndModal(wxID_OK);
      }
    }
  }
}

void CryptKeyEntry::OnCancel(wxCommandEvent& WXUNUSED(event))
{
  EndModal(wxID_CANCEL);
}

void CryptKeyEntry::OnClose(wxCloseEvent& WXUNUSED(event))
{
  EndModal(wxID_CLOSE);
}
