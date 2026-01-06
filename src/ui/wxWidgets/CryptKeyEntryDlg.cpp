/*
 * Initial version created as 'CryptKeyEntryDlg.cpp'
 * by rafaelx on 2019-03-02.
 *
 * Copyright (c) 2019-2026 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/statbmp.h>
//*)

#include "CryptKeyEntryDlg.h"
#include "version.h"
#include "wxUtilities.h"

#include "graphics/cpane.xpm"
#include "graphics/eye.xpm"
#include "graphics/eye_close.xpm"

//(*IdInit(CryptKeyEntryDlg)
//*)

BEGIN_EVENT_TABLE(CryptKeyEntryDlg, wxDialog)
  //(*EventTable(CryptKeyEntryDlg)
  EVT_BUTTON( wxID_OK,     CryptKeyEntryDlg::OnOk     )
  EVT_BUTTON( wxID_CANCEL, CryptKeyEntryDlg::OnCancel )
  EVT_CLOSE(               CryptKeyEntryDlg::OnClose  )
  //*)
END_EVENT_TABLE()

CryptKeyEntryDlg::CryptKeyEntryDlg(Mode mode) : m_Mode(mode)
{
  //(*Initialize(CryptKeyEntryDlg)
  wxString modeVariant = IsEncryptionMode() ? _("Encryption") : _("Decryption");
  wxString title = 
#if defined(REVISION) && (REVISION != 0)
    wxString::Format(wxT("%ls by %ls v%d.%d.%d %ls"),
                          modeVariant, pwsafeAppName,
                          MAJORVERSION, MINORVERSION,
                          REVISION, SPECIALBUILD);
#else
    wxString::Format(wxT("%ls by %ls v%d.%d %ls"),
                          modeVariant, pwsafeAppName,
                          MAJORVERSION, MINORVERSION, SPECIALBUILD);
#endif
  Create(nullptr, -1, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL, _T("id"));
  //*)
  CreateControls();
  // Allow to resize the dialog only in width.
  SetMinSize(wxSize(static_cast<int>(GetMinSize().x * 1.5), GetMinSize().y));
}

CryptKeyEntryDlg::~CryptKeyEntryDlg()
{
  //(*Destroy(CryptKeyEntryDlg)
  //*)
}

void CryptKeyEntryDlg::CreateControls()
{
    auto* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(mainSizer);

    auto* pwSafeLogo = new wxStaticBitmap(this, wxID_STATIC, wxBitmap(cpane_xpm), wxDefaultPosition, ConvertDialogToPixels(wxSize(49, 46)), 0);
    mainSizer->Add(pwSafeLogo, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 12);

    auto* vBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(vBoxSizer1, 1, wxEXPAND|wxALL, 12);

    wxString description = IsEncryptionMode() ? _("Enter an encryption key.") : _("Enter a decryption key.");
    auto* staticTextDescription = new wxStaticText(this, wxID_ANY, description, wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    vBoxSizer1->Add(staticTextDescription, 0, wxALIGN_LEFT|wxTOP|wxBOTTOM, 12);

    auto* staticTextKey1 = new wxStaticText(this, wxID_ANY, _("Key"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    vBoxSizer1->Add(staticTextKey1, 0, wxALIGN_LEFT|wxBOTTOM, 5);

    auto* hBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    vBoxSizer1->Add(hBoxSizer1, 1, wxEXPAND|wxBOTTOM, 12);

    m_TextCtrlKey1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("wxID_ANY"));
    m_TextCtrlKey1->SetFocus();
    ApplyFontPreference(m_TextCtrlKey1, PWSprefs::StringPrefs::PasswordFont);
    hBoxSizer1->Add(m_TextCtrlKey1, 1, wxEXPAND, 0);

    auto *showHideButtonKey = new wxBitmapButton(this, wxID_ANY, wxBitmap(eye_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    showHideButtonKey->SetToolTip(_("Show key"));
    showHideButtonKey->Bind(wxEVT_BUTTON, [&, hBoxSizer1, showHideButtonKey](wxCommandEvent& event) {
      UpdatePasswordTextCtrl(hBoxSizer1, m_TextCtrlKey1, m_TextCtrlKey1->GetValue(), nullptr, isCryptKeyHidden ? 0 : wxTE_PASSWORD);
      showHideButtonKey->SetBitmapLabel(wxBitmap(isCryptKeyHidden ? eye_close_xpm : eye_xpm));
      showHideButtonKey->SetToolTip(isCryptKeyHidden ? _("Hide key") : _("Show key"));
      isCryptKeyHidden = !isCryptKeyHidden;
    });
    hBoxSizer1->Add(showHideButtonKey, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);

    if (IsEncryptionMode()) {
      auto* staticTextKey2 = new wxStaticText(this, wxID_ANY, _("Verification Key"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
      vBoxSizer1->Add(staticTextKey2, 0, wxALIGN_LEFT|wxBOTTOM, 5);

      auto* hBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
      vBoxSizer1->Add(hBoxSizer2, 1, wxEXPAND|wxBOTTOM, 12);

      m_TextCtrlKey2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("wxID_ANY"));
      ApplyFontPreference(m_TextCtrlKey2, PWSprefs::StringPrefs::PasswordFont);
      hBoxSizer2->Add(m_TextCtrlKey2, 1, wxEXPAND, 0);

      auto *showHideButtonVerificationKey = new wxBitmapButton(this, wxID_ANY, wxBitmap(eye_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
      showHideButtonVerificationKey->SetToolTip(_("Show verification key"));
      showHideButtonVerificationKey->Bind(wxEVT_BUTTON, [&, hBoxSizer2, showHideButtonVerificationKey](wxCommandEvent& event) {
        UpdatePasswordTextCtrl(hBoxSizer2, m_TextCtrlKey2, m_TextCtrlKey2->GetValue(), nullptr, isCryptVerificationKeyHidden ? 0 : wxTE_PASSWORD);
        showHideButtonVerificationKey->SetBitmapLabel(wxBitmap(isCryptVerificationKeyHidden ? eye_close_xpm : eye_xpm));
        showHideButtonVerificationKey->SetToolTip(isCryptVerificationKeyHidden ? _("Hide verification key") : _("Show verification key"));
        isCryptVerificationKeyHidden = !isCryptVerificationKeyHidden;
      });
      hBoxSizer2->Add(showHideButtonVerificationKey, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);
    }

    auto* okButton = new wxButton(this, wxID_OK, wxEmptyString);
    okButton->SetDefault();
    auto* stdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    stdDialogButtonSizer1->AddButton(okButton);
    stdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    stdDialogButtonSizer1->Realize();
    vBoxSizer1->Add(stdDialogButtonSizer1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
}

void CryptKeyEntryDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {

    if (m_Mode == Mode::ENCRYPT) {
      if (m_TextCtrlKey1->GetValue().IsEmpty() || m_TextCtrlKey2->GetValue().IsEmpty()) {
        wxMessageDialog messageBox(
          this, _("The combination cannot be blank."),
          _("Error"), wxOK | wxICON_EXCLAMATION
        );

        messageBox.ShowModal();
        return;
      }
      else if (m_TextCtrlKey1->GetValue() != m_TextCtrlKey2->GetValue()) {
        wxMessageDialog messageBox(
          this, _("The two entries do not match."),
          _("Error"), wxOK | wxICON_EXCLAMATION
        );

        messageBox.ShowModal();
        return;
      }
      else {
        m_CryptKey = tostringx(m_TextCtrlKey1->GetValue());

        EndModal(wxID_OK);
      }
    }
    else {
      if (m_TextCtrlKey1->GetValue().IsEmpty()) {
        wxMessageDialog messageBox(
          this, _("The entry cannot be blank."),
          _("Error"), wxOK | wxICON_EXCLAMATION
        );

        messageBox.ShowModal();
        return;
      }
      else {
        m_CryptKey = tostringx(m_TextCtrlKey1->GetValue());

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
