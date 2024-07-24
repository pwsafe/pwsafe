/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SafeCombinationPromptDlg.cpp
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

#include <wx/filename.h>

#include "os/file.h"

////@begin includes
#include "ExternalKeyboardButton.h"
#include "SafeCombinationCtrl.h"
#include "SafeCombinationPromptDlg.h"
#include "wxUtilities.h"
////@end includes

/*!
 * SafeCombinationPromptDlg type definition
 */

IMPLEMENT_CLASS( SafeCombinationPromptDlg, wxDialog )

/*!
 * SafeCombinationPromptDlg event table definition
 */

BEGIN_EVENT_TABLE( SafeCombinationPromptDlg, wxDialog )

#ifndef NO_YUBI
////@begin SafeCombinationPromptDlg event table entries
  EVT_ACTIVATE( SafeCombinationPromptDlg::OnActivate                     )
  EVT_BUTTON( ID_YUBIBTN,  SafeCombinationPromptDlg::OnYubibtnClick      )
  EVT_TIMER(  POLLING_TIMER_ID, SafeCombinationPromptDlg::OnPollingTimer )
////@end SafeCombinationPromptDlg event table entries
#endif

  EVT_BUTTON( wxID_OK,     SafeCombinationPromptDlg::OnOkClick           )
  EVT_BUTTON( wxID_CANCEL, SafeCombinationPromptDlg::OnCancelClick       )

END_EVENT_TABLE()

/*!
 * SafeCombinationPromptDlg constructors
 */

SafeCombinationPromptDlg::SafeCombinationPromptDlg(wxWindow *parent, PWScore &core,
                                               const wxString &fname,
                                               wxWindowID id,
                                               const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core), m_filename(fname)
{
  wxASSERT(!parent || parent->IsTopLevel());
////@begin SafeCombinationPromptDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end SafeCombinationPromptDlg creation
#ifndef NO_YUBI
  SetupMixin(this, FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
#endif
}


SafeCombinationPromptDlg* SafeCombinationPromptDlg::Create(wxWindow *parent, PWScore &core,
                                               const wxString &fname,
                                               wxWindowID id,
                                               const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
{
  return new SafeCombinationPromptDlg(parent, core, fname, id, caption, pos, size, style);
}

/*!
 * SafeCombinationPromptDlg destructor
 */

SafeCombinationPromptDlg::~SafeCombinationPromptDlg()
{
////@begin SafeCombinationPromptDlg destruction
////@end SafeCombinationPromptDlg destruction
}

/*!
 * Control creation for SafeCombinationPromptDlg
 */

void SafeCombinationPromptDlg::CreateControls()
{
  auto *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  auto *verticalBoxSizer1 = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(verticalBoxSizer1, 0, wxALL|wxEXPAND, 12);

  auto *itemStaticText7 = new wxStaticText(this, wxID_STATIC, _("Password Database"), wxDefaultPosition, wxDefaultSize, 0);
  auto textColor = itemStaticText7->GetForegroundColour();
  verticalBoxSizer1->Add(itemStaticText7, 0, wxBOTTOM, 5);

  m_textCtrlFilename = new wxTextCtrl(this, wxID_STATIC, _("filename"), wxDefaultPosition, wxDefaultSize, 0);
  m_textCtrlFilename->Disable();
  m_textCtrlFilename->SetForegroundColour(textColor);
  verticalBoxSizer1->Add(m_textCtrlFilename, 0, wxALL|wxEXPAND, 0);

  m_scctrl = wxUtilities::CreateLabeledSafeCombinationCtrl(this, ID_PASSWORD, _("Master Password"), &m_password, true);

#ifndef NO_YUBI
  auto yubiControls = wxUtilities::CreateYubiKeyControls(this, ID_YUBIBTN, ID_YUBISTATUS);
  m_YubiBtn = wxUtilities::GetYubiKeyButtonControl(yubiControls);
  m_yubiStatusCtrl = wxUtilities::GetYubiKeyStatusControl(yubiControls);
#endif

  mainSizer->AddStretchSpacer();

  auto horizontalBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizontalBoxSizer3, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  horizontalBoxSizer3->AddSpacer(90);
  horizontalBoxSizer3->AddStretchSpacer();

  horizontalBoxSizer3->Add(
    new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );

  auto *unlockButton = new wxButton(this, wxID_OK, _("&Unlock"), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer3->Add(
    unlockButton,
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );
  unlockButton->SetDefault();

  horizontalBoxSizer3->AddSpacer(60);
  horizontalBoxSizer3->AddStretchSpacer();

  if (wxUtilities::IsVirtualKeyboardSupported()) {
    auto *keyboardButton = new ExternalKeyboardButton(this);
    keyboardButton->SetFocusOnSafeCombinationCtrl(m_scctrl);
    horizontalBoxSizer3->Add(
      keyboardButton,
      0, wxALIGN_CENTER_VERTICAL|wxALL, 0
    );
  }

  // Event handler to update the file path name string if the size of the text field changed.
  m_textCtrlFilename->Bind(wxEVT_SIZE, [&](wxSizeEvent& WXUNUSED(event)) {
    EllipsizeFilePathname();
  });

  wxWindow* passwdCtrl = FindWindow(ID_PASSWORD);
  if (passwdCtrl) {
    passwdCtrl->SetFocus();
  }
}

/*!
 * Should we show tooltips?
 */

bool SafeCombinationPromptDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get icon resources
 */

wxIcon SafeCombinationPromptDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin SafeCombinationPromptDlg icon retrieval
  return wxNullIcon;
////@end SafeCombinationPromptDlg icon retrieval
}

bool SafeCombinationPromptDlg::ProcessPhrase()
{
  static unsigned tries = 0;

  if (m_core.CheckPasskey(tostringx(m_filename),
                          m_password) != PWScore::SUCCESS) {
    wxString errmess;
    if (++tries > 2) {
      errmess = wxString::Format(_("The master password has been entered %d times without success:\n"), tries);
      errmess += _("- Is Caps Lock off?\n");
      errmess += _("- Is the language correct (if multilingual)?\n");
      errmess += _("- Is this the correct database?\n");
      errmess += _("- Perhaps the database was damaged. Try opening a backup copy.");
    } else {
      errmess =  _("Incorrect master password,\n");
      errmess += _("not a Password Safe database,\n");
      errmess += _("or a corrupt database.");
    }
    wxMessageDialog err(this, errmess,
                        _("Can't open a password database"), wxOK | wxICON_EXCLAMATION);
    err.ShowModal();

    auto *txt = wxDynamicCast(FindWindow(ID_PASSWORD), wxTextCtrl);
    if (txt) {
      txt->SetSelection(-1,-1);
      txt->SetFocus();
    }
    return false;
  }
  m_core.SetCurFile(tostringx(m_filename));
  return true;
}

void SafeCombinationPromptDlg::EllipsizeFilePathname()
{
  if (m_filename.IsEmpty()) {
    return;
  }

  wxScreenDC dc;

  m_textCtrlFilename->ChangeValue(
    wxControl::Ellipsize(
      m_filename, dc, wxEllipsizeMode::wxELLIPSIZE_MIDDLE,
      /* The limiting width for the text is the text input field width reduced by the margins. */
      (m_textCtrlFilename->GetSize()).GetWidth() - 18
    )
  );
  m_textCtrlFilename->SetToolTip(m_filename);
}

void SafeCombinationPromptDlg::OnActivate(wxActivateEvent& WXUNUSED(event))
{
  if (!m_DialogActivated) {
    EllipsizeFilePathname();
    m_DialogActivated = true;
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SafeCombinationPromptDlg::OnOkClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    if (m_password.empty()) {
      wxMessageDialog err(this, _("The combination cannot be blank."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();

    } else if (!pws_os::FileExists(tostdstring(m_filename))) {
      wxMessageDialog err(this, _("File or path not found."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();

    } else if (ProcessPhrase()) {
      EndModal(wxID_OK);
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void SafeCombinationPromptDlg::OnCancelClick(wxCommandEvent& WXUNUSED(evt))
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in SafeCombinationPromptDlg.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CANCEL);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in SafeCombinationPromptDlg.
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
 */

void SafeCombinationPromptDlg::OnYubibtnClick(wxCommandEvent& WXUNUSED(event))
{
  m_scctrl->AllowEmptyCombinationOnce();  // Allow blank password when Yubi's used

  if (Validate() && TransferDataFromWindow()) {
    if (!pws_os::FileExists(tostdstring(m_filename))) {
      wxMessageDialog err(this, _("File or path not found."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();

    } else {
      StringX response;
      bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
      if (PerformChallengeResponse(this, m_password, response, oldYubiChallenge)) {
        m_password = response;
        if (ProcessPhrase()) {
          EndModal(wxID_OK);
        }
        UpdateStatus();
      }
    }
  }
}

void SafeCombinationPromptDlg::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    HandlePollingTimer(); // in YubiMixin
  }
}
#endif
