/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file safecombinationprompt.cpp
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

////@begin includes
#include "SafeCombinationCtrl.h"
////@end includes

#include "safecombinationprompt.h"
#include "os/file.h"
#include "./wxutils.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#ifndef NO_YUBI
////@begin XPM images
#include "graphics/Yubikey-button.xpm"
////@end XPM images
#endif

#include "./graphics/cpane.xpm"

/*!
 * CSafeCombinationPrompt type definition
 */

IMPLEMENT_CLASS( CSafeCombinationPrompt, wxDialog )

/*!
 * CSafeCombinationPrompt event table definition
 */

BEGIN_EVENT_TABLE( CSafeCombinationPrompt, wxDialog )

#ifndef NO_YUBI
////@begin CSafeCombinationPrompt event table entries
  EVT_BUTTON( ID_YUBIBTN, CSafeCombinationPrompt::OnYubibtnClick )

////@end CSafeCombinationPrompt event table entries
EVT_TIMER(POLLING_TIMER_ID, CSafeCombinationPrompt::OnPollingTimer)
#endif

  EVT_BUTTON( wxID_OK, CSafeCombinationPrompt::OnOkClick )

  EVT_BUTTON( wxID_CANCEL, CSafeCombinationPrompt::OnCancelClick )

  EVT_BUTTON( wxID_EXIT, CSafeCombinationPrompt::OnExitClick )

END_EVENT_TABLE()

/*!
 * CSafeCombinationPrompt constructors
 */

CSafeCombinationPrompt::CSafeCombinationPrompt(wxWindow* parent, PWScore &core,
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
 * CSafeCombinationPrompt creator
 */

bool CSafeCombinationPrompt::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSafeCombinationPrompt creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CSafeCombinationPrompt creation
#ifndef NO_YUBI
  SetupMixin(FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_pollingTimer->Start(CYubiMixin::POLLING_INTERVAL);
#endif
  return true;
}

/*!
 * CSafeCombinationPrompt destructor
 */

CSafeCombinationPrompt::~CSafeCombinationPrompt()
{
////@begin CSafeCombinationPrompt destruction
////@end CSafeCombinationPrompt destruction
#ifndef NO_YUBI
  delete m_pollingTimer;
#endif
}

/*!
 * Member initialisation
 */

void CSafeCombinationPrompt::Init()
{
////@begin CSafeCombinationPrompt member initialisation
  m_scctrl = nullptr;
#ifndef NO_YUBI
  m_YubiBtn = nullptr;
  m_yubiStatusCtrl = nullptr;
#endif

////@end CSafeCombinationPrompt member initialisation
}

/*!
 * Control creation for CSafeCombinationPrompt
 */

void CSafeCombinationPrompt::CreateControls()
{
////@begin CSafeCombinationPrompt content construction
  CSafeCombinationPrompt* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticBitmap* itemStaticBitmap4 = new wxStaticBitmap( itemDialog1, wxID_STATIC, itemDialog1->GetBitmapResource(L"graphics/cpane.xpm"), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(49, 46)), 0 );
  itemBoxSizer3->Add(itemStaticBitmap4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please enter the safe combination for this password database."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("filename"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(itemBoxSizer8, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Safe combination:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_scctrl = new CSafeCombinationCtrl( itemDialog1, ID_PASSWORD, &m_password, wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1) );
  itemBoxSizer8->Add(m_scctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
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

  itemBoxSizer4->Add(
    new wxButton(itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0), 
    0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5
  );

  // Set validators
  itemStaticText7->SetValidator( wxGenericValidator(& m_filename) );
////@end CSafeCombinationPrompt content construction
  wxWindow* passwdCtrl = FindWindow(ID_PASSWORD);
  if (passwdCtrl) {
    passwdCtrl->SetFocus();
  }
}

/*!
 * Should we show tooltips?
 */

bool CSafeCombinationPrompt::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSafeCombinationPrompt::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CSafeCombinationPrompt bitmap retrieval
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
////@end CSafeCombinationPrompt bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSafeCombinationPrompt::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin CSafeCombinationPrompt icon retrieval
  return wxNullIcon;
////@end CSafeCombinationPrompt icon retrieval
}

void CSafeCombinationPrompt::ProcessPhrase()
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
    wxTextCtrl *txt = dynamic_cast<wxTextCtrl *>(FindWindow(ID_PASSWORD));
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

void CSafeCombinationPrompt::OnOkClick( wxCommandEvent& /* evt */ )
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

void CSafeCombinationPrompt::OnCancelClick( wxCommandEvent& /* evt */ )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationPrompt.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CANCEL);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationPrompt.
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT
 */

void CSafeCombinationPrompt::OnExitClick( wxCommandEvent& /* evt */ )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT in CSafeCombinationPrompt.
  // Before editing this code, remove the block markers.
  EndModal(wxID_EXIT);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationPrompt.
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
 */

void CSafeCombinationPrompt::OnYubibtnClick( wxCommandEvent& /* event */ )
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

void CSafeCombinationPrompt::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    HandlePollingTimer(); // in CYubiMixin
  }
}
#endif
