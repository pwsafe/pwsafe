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
#include "SafeCombinationCtrl.h"
#include "SafeCombinationPromptDlg.h"
#include "wxUtilities.h"
////@end includes

#ifndef NO_YUBI
////@begin XPM images
#include "graphics/Yubikey-button.xpm"
////@end XPM images
#endif

#include "graphics/cpane.xpm"

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
  EVT_CHECKBOX(ID_READONLY,SafeCombinationPromptDlg::OnReadonlyClick     )

END_EVENT_TABLE()

/*!
 * SafeCombinationPromptDlg constructors
 */

SafeCombinationPromptDlg::SafeCombinationPromptDlg(wxWindow *parent, PWScore &core,
                                               const wxString &fname, const bool allowExit,
                                               wxWindowID id,
                                               const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core), m_filename(fname), m_allowExit(allowExit)
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
                                               const wxString &fname, const bool allowExit,
                                               wxWindowID id,
                                               const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
{
  return new SafeCombinationPromptDlg(parent, core, fname, allowExit, id, caption, pos, size, style);
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

  auto *itemStaticText6 = new wxStaticText(this, wxID_STATIC, _("Enter the Master Password to unlock the password database:"), wxDefaultPosition, wxDefaultSize, 0);
  mainSizer->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 12);

  auto *itemStaticText7 = new wxStaticText(this, wxID_STATIC, _("filename"), wxDefaultPosition, wxDefaultSize, 0);
  mainSizer->Add(itemStaticText7, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT, 12);

  auto *verticalBoxSizer1 = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(verticalBoxSizer1, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *itemStaticText9 = new wxStaticText(this, wxID_STATIC, _("Master Password"), wxDefaultPosition, wxDefaultSize, 0);
  verticalBoxSizer1->Add(itemStaticText9, 0, wxBOTTOM, 5);

  m_scctrl = new SafeCombinationCtrl(this, ID_PASSWORD, &m_password, wxDefaultPosition, wxDefaultSize);
  m_scctrl->SetFocus();
  verticalBoxSizer1->Add(m_scctrl, 1, wxALL|wxEXPAND, 0);
  
  auto *horizontalBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizontalBoxSizer1, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *itemCheckBox15 = new wxCheckBox(this, ID_READONLY, _("Open as read-only"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox15->SetValue(false);
  m_readOnly = false;

  auto *showCombinationCheckBox = new wxCheckBox(this, wxID_ANY, _("Show Master Password"), wxDefaultPosition, wxDefaultSize, 0 );
  showCombinationCheckBox->SetValue(false);
  showCombinationCheckBox->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& event) {m_scctrl->SecureTextfield(!event.IsChecked());});

  horizontalBoxSizer1->Add(itemCheckBox15, 2, wxALIGN_LEFT|wxALL, 0);
  horizontalBoxSizer1->AddSpacer(1);
  horizontalBoxSizer1->Add(showCombinationCheckBox, 2, wxALIGN_LEFT|wxALL, 0);
  
  itemCheckBox15->SetValidator( wxGenericValidator(& m_readOnly) );
  UpdateReadOnlyCheckbox(itemCheckBox15);

  auto *horizontalBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizontalBoxSizer2, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

#ifndef NO_YUBI
  horizontalBoxSizer2->AddStretchSpacer(1);

  m_yubiStatusCtrl = new wxStaticText(this, ID_YUBISTATUS, _("Insert your YubiKey"), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer2->Add(m_yubiStatusCtrl, 2, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);

  m_YubiBtn = new wxBitmapButton(this, ID_YUBIBTN, GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, ConvertDialogToPixels(wxSize(40, 12)), wxBU_AUTODRAW);
  horizontalBoxSizer2->Add(m_YubiBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
#endif

  mainSizer->AddStretchSpacer();

  auto horizontalBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizontalBoxSizer3, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  horizontalBoxSizer3->Add(
    new wxButton(this, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5
  );

  horizontalBoxSizer3->Add(
    new wxButton(this, wxID_EXIT, _("&Exit"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5
  );

  horizontalBoxSizer3->AddStretchSpacer();

  horizontalBoxSizer3->Add(
    new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );

  auto *okButton = new wxButton(this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer3->Add(
    okButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );
  okButton->SetDefault();

  if(! m_allowExit)
    FindWindow(wxID_EXIT)->Disable();
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
    return;
  }
  m_core.SetReadOnly(m_readOnly);
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

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_READONLY
 */

void SafeCombinationPromptDlg::OnReadonlyClick( wxCommandEvent& event )
{
  m_readOnly = event.IsChecked();
}

void SafeCombinationPromptDlg::UpdateReadOnlyCheckbox(wxCheckBox *checkBox)
{
  wxFileName fn(m_filename);

  wxASSERT_MSG(checkBox, wxT("checkbox NULL"));
  if (m_core.IsDbOpen()) {
    m_readOnly = m_core.IsReadOnly();
    checkBox->SetValue(m_readOnly);
    checkBox->Enable(false);
  } else if ( fn.FileExists() ) { // Do nothing if the file doesn't exist
    bool writeable = fn.IsFileWritable();
    bool defaultRO = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO);
    m_readOnly = (writeable? defaultRO : true);
    checkBox->SetValue(m_readOnly);
    checkBox->Enable(writeable);
  }
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
