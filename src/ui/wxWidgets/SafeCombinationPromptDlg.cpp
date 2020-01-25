/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
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

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "os/file.h"

////@begin includes
#include "SafeCombinationCtrl.h"
////@end includes

#include "SafeCombinationPromptDlg.h"
#include "wxUtilities.h"

#ifndef NO_YUBI
////@begin XPM images
#include "graphics/Yubikey-button.xpm"
////@end XPM images
#endif

#include "./graphics/cpane.xpm"

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
  EVT_BUTTON( ID_YUBIBTN,  SafeCombinationPromptDlg::OnYubibtnClick      )
  EVT_TIMER(  POLLING_TIMER_ID, SafeCombinationPromptDlg::OnPollingTimer )
////@end SafeCombinationPromptDlg event table entries
#endif

  EVT_BUTTON( wxID_OK,     SafeCombinationPromptDlg::OnOkClick           )
  EVT_BUTTON( wxID_CANCEL, SafeCombinationPromptDlg::OnCancelClick       )
  EVT_BUTTON( wxID_EXIT,   SafeCombinationPromptDlg::OnExitClick         )

END_EVENT_TABLE()

/*!
 * SafeCombinationPromptDlg constructors
 */

SafeCombinationPromptDlg::SafeCombinationPromptDlg(wxWindow* parent, PWScore &core,
                                               const wxString &fname, wxWindowID id,
                                               const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core), m_filename(fname), m_tries(0)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * SafeCombinationPromptDlg creator
 */

bool SafeCombinationPromptDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin SafeCombinationPromptDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();
////@end SafeCombinationPromptDlg creation
#ifndef NO_YUBI
  SetupMixin(FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_pollingTimer->Start(YubiMixin::POLLING_INTERVAL);
#endif
  return true;
}

/*!
 * SafeCombinationPromptDlg destructor
 */

SafeCombinationPromptDlg::~SafeCombinationPromptDlg()
{
////@begin SafeCombinationPromptDlg destruction
////@end SafeCombinationPromptDlg destruction
#ifndef NO_YUBI
  delete m_pollingTimer;
#endif
}

/*!
 * Member initialisation
 */

void SafeCombinationPromptDlg::Init()
{
////@begin SafeCombinationPromptDlg member initialisation
  m_scctrl = nullptr;
#ifndef NO_YUBI
  m_YubiBtn = nullptr;
  m_yubiStatusCtrl = nullptr;
#endif

////@end SafeCombinationPromptDlg member initialisation
}

/*!
 * Control creation for SafeCombinationPromptDlg
 */

void SafeCombinationPromptDlg::CreateControls()
{
////@begin SafeCombinationPromptDlg content construction
  SafeCombinationPromptDlg* itemDialog1 = this;

  auto *itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  auto *itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 1, /*wxALIGN_CENTER_HORIZONTAL|*/wxALL|wxEXPAND, 5);

  wxStaticBitmap* itemStaticBitmap4 = new wxStaticBitmap( itemDialog1, wxID_STATIC, itemDialog1->GetBitmapResource(L"graphics/cpane.xpm"), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(49, 46)), 0 );
  itemBoxSizer3->Add(itemStaticBitmap4, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxGROW, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please enter the safe combination for this password database."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("filename"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(itemBoxSizer8, 0, wxGROW|wxALL|wxEXPAND, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Safe combination:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_scctrl = new SafeCombinationCtrl( itemDialog1, ID_PASSWORD, &m_password, wxDefaultPosition, wxDefaultSize );
  itemBoxSizer8->Add(m_scctrl, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5);

  auto *itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(itemBoxSizer11, 0, wxGROW|wxALL, 5);

#ifndef NO_YUBI
  m_YubiBtn = new wxBitmapButton( itemDialog1, ID_YUBIBTN, itemDialog1->GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  itemBoxSizer11->Add(m_YubiBtn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);
#endif

  itemBoxSizer11->Add(4, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

#ifndef NO_YUBI
  m_yubiStatusCtrl = new wxStaticText( itemDialog1, ID_YUBISTATUS, _("Please insert your YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer11->Add(m_yubiStatusCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

  auto itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_BOTTOM|wxALL|wxEXPAND, 5);

  itemBoxSizer4->Add(
    new wxButton(itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0), 
    0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5
  );

  itemBoxSizer4->Add(
    new wxButton(itemDialog1, wxID_EXIT, _("&Exit"), wxDefaultPosition, wxDefaultSize, 0), 
    0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5
  );

  itemBoxSizer4->AddStretchSpacer();

  itemBoxSizer4->Add(
    new wxButton(itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0), 
    0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5
  );

  auto *okButton = new wxButton(itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer4->Add(
    okButton, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5
  );
  okButton->SetDefault();

  // Set validators
  itemStaticText7->SetValidator( wxGenericValidator(& m_filename) );
////@end SafeCombinationPromptDlg content construction
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
 * Get bitmap resources
 */

wxBitmap SafeCombinationPromptDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin SafeCombinationPromptDlg bitmap retrieval
  if (name == L"graphics/cpane.xpm")
  {
    wxBitmap bitmap(cpane_xpm);
    return bitmap;
  }
#ifndef NO_YUBI
  else if (name == _T("graphics/Yubikey-button.xpm"))
  {
    wxBitmap bitmap(Yubikey_button_xpm);
    return bitmap;
  }
#endif
  return wxNullBitmap;
////@end SafeCombinationPromptDlg bitmap retrieval
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

void SafeCombinationPromptDlg::ProcessPhrase()
{
  if (m_core.CheckPasskey(tostringx(m_filename),
                          m_password) != PWScore::SUCCESS) {
    wxString errmess;
    if (m_tries >= 2) {
      errmess = _("Three strikes - yer out!");
    } else {
      m_tries++;
      errmess = _("Incorrect passkey, not a PasswordSafe database, or a corrupt database. (Backup database has same name as original, ending with '~')");
    }
    wxMessageDialog err(this, errmess,
                        _("Error"), wxOK | wxICON_EXCLAMATION);
    err.ShowModal();
    auto *txt = dynamic_cast<wxTextCtrl *>(FindWindow(ID_PASSWORD));
    txt->SetSelection(-1,-1);
    txt->SetFocus();
    return;
  }
  // m_core.SetReadOnly(m_readOnly);
  m_core.SetCurFile(tostringx(m_filename));
  EndModal(wxID_OK);
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
      return;
    }
    if (!pws_os::FileExists(tostdstring(m_filename))) {
      wxMessageDialog err(this, _("File or path not found."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
      return;
    }
    ProcessPhrase();
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

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT
 */

void SafeCombinationPromptDlg::OnExitClick(wxCommandEvent& WXUNUSED(evt))
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT in SafeCombinationPromptDlg.
  // Before editing this code, remove the block markers.
  EndModal(wxID_EXIT);
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
      return;
    }

    StringX response;
    bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
    if (PerformChallengeResponse(this, m_password, response, oldYubiChallenge)) {
      m_password = response;
      ProcessPhrase();
      UpdateStatus();
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
