/*
 * Initial version created as 'CryptKeyEntryDlg.cpp'
 * by rafaelx on 2019-03-02.
 *
 * Copyright (c) 2019-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file CryptKeyEntryDlg.cpp
* 
*/

#include <wx/msgdlg.h>

//(*InternalHeaders(CryptKeyEntryDlg)
#include <wx/button.h>
//*)

#include "CryptKeyEntryDlg.h"
#include "wxUtilities.h"

//(*IdInit(CryptKeyEntryDlg)
//*)

BEGIN_EVENT_TABLE(CryptKeyEntryDlg, wxDialog)
  //(*EventTable(CryptKeyEntryDlg)
  EVT_BUTTON( wxID_OK,     CryptKeyEntryDlg::OnOk     )
  EVT_BUTTON( wxID_CANCEL, CryptKeyEntryDlg::OnCancel )
  EVT_CLOSE(               CryptKeyEntryDlg::OnClose  )
  //*)
END_EVENT_TABLE()

CryptKeyEntryDlg::CryptKeyEntryDlg(Mode mode)
{
    //(*Initialize(CryptKeyEntryDlg)
    wxStaticText* StaticTextKey2;
    wxFlexGridSizer* FlexGridSizer1;
    wxStaticText* StaticTextKey1;
    wxBoxSizer* BoxSizer1;
    wxStaticText* StaticTextDescription;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    if (mode == Mode::ENCRYPT) {
      Create(nullptr, -1, _("Encryption"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL, _T("id"));
      StaticTextDescription = new wxStaticText(this, wxID_ANY, _("Enter an encryption key."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    }
    else {
      Create(nullptr, -1, _("Decryption"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL, _T("id"));
      StaticTextDescription = new wxStaticText(this, wxID_ANY, _("Enter a decryption key."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    }

    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    BoxSizer1->Add(StaticTextDescription, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    StaticTextKey1 = new wxStaticText(this, wxID_ANY, _("Enter Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer1->Add(StaticTextKey1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlKey1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("wxID_ANY"));
    TextCtrlKey1->SetFocus();
    FlexGridSizer1->Add(TextCtrlKey1, 1, wxALL|wxEXPAND, 5);

    if (mode == Mode::ENCRYPT) {
      StaticTextKey2 = new wxStaticText(this, wxID_ANY, _("Verify Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
      FlexGridSizer1->Add(StaticTextKey2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
      TextCtrlKey2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("wxID_ANY"));
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
    SetMinSize(wxSize(static_cast<int>(GetMinSize().x * 1.5), GetMinSize().y));
    SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));

    m_Mode = mode;
}

CryptKeyEntryDlg::~CryptKeyEntryDlg()
{
  //(*Destroy(CryptKeyEntryDlg)
  //*)
}

void CryptKeyEntryDlg::OnOk(wxCommandEvent& WXUNUSED(event))
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
        m_CryptKey = tostringx(TextCtrlKey1->GetValue());

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
        m_CryptKey = tostringx(TextCtrlKey1->GetValue());

        EndModal(wxID_OK);
      }
    }
  }
}

void CryptKeyEntryDlg::OnCancel(wxCommandEvent& WXUNUSED(event))
{
  EndModal(wxID_CANCEL);
}

void CryptKeyEntryDlg::OnClose(wxCloseEvent& WXUNUSED(event))
{
  EndModal(wxID_CLOSE);
}
