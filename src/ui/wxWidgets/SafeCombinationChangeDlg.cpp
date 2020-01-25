/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SafeCombinationChangeDlg.cpp
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

#include "core/PWCharPool.h" // for CheckPassword()

////@begin includes
#include "ExternalKeyboardButton.h"
#include "SafeCombinationCtrl.h"
#include "SafeCombinationChangeDlg.h"
#include "wxUtilities.h"          // for ApplyPasswordFont
////@end includes

#ifndef NO_YUBI
////@begin XPM images
#include "graphics/Yubikey-button.xpm"
////@end XPM images
#endif

/*!
 * SafeCombinationChangeDlg type definition
 */

IMPLEMENT_CLASS( SafeCombinationChangeDlg, wxDialog )

/*!
 * SafeCombinationChangeDlg event table definition
 */

BEGIN_EVENT_TABLE( SafeCombinationChangeDlg, wxDialog )

////@begin SafeCombinationChangeDlg event table entries
#ifndef NO_YUBI
  EVT_BUTTON( ID_YUBIBTN,                   SafeCombinationChangeDlg::OnYubibtnClick  )
  EVT_BUTTON( ID_YUBIBTN2,                  SafeCombinationChangeDlg::OnYubibtn2Click )
  EVT_TIMER(  YubiMixin::POLLING_TIMER_ID, SafeCombinationChangeDlg::OnPollingTimer  )
#endif
  EVT_BUTTON( wxID_OK,                      SafeCombinationChangeDlg::OnOkClick       )
  EVT_BUTTON( wxID_CANCEL,                  SafeCombinationChangeDlg::OnCancelClick   )

////@end SafeCombinationChangeDlg event table entries
END_EVENT_TABLE()

/*!
 * SafeCombinationChangeDlg constructors
 */

SafeCombinationChangeDlg::SafeCombinationChangeDlg(wxWindow* parent, PWScore &core,
                                               wxWindowID id, const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * SafeCombinationChangeDlg creator
 */

bool SafeCombinationChangeDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin SafeCombinationChangeDlg creation
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
////@end SafeCombinationChangeDlg creation
#ifndef NO_YUBI
  m_yubiMixin1.SetupMixin(FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  m_yubiMixin1.SetPrompt1(_("Enter old safe combination (if any) and click on top Yubikey button"));
  m_yubiMixin2.SetupMixin(FindWindow(ID_YUBIBTN2), FindWindow(ID_YUBISTATUS));
  m_yubiMixin2.SetPrompt1(_("Enter old safe combination (if any) and click on top Yubikey button"));
  m_pollingTimer = new wxTimer(this, YubiMixin::POLLING_TIMER_ID);
  m_pollingTimer->Start(2*YubiMixin::POLLING_INTERVAL); // 2 controls
#endif
  return true;
}

/*!
 * SafeCombinationChangeDlg destructor
 */

SafeCombinationChangeDlg::~SafeCombinationChangeDlg()
{
////@begin SafeCombinationChangeDlg destruction
////@end SafeCombinationChangeDlg destruction
#ifndef NO_YUBI
  delete m_pollingTimer;
#endif
}

/*!
 * Member initialisation
 */

void SafeCombinationChangeDlg::Init()
{
////@begin SafeCombinationChangeDlg member initialisation
  m_oldPasswdEntry = nullptr;
  m_newPasswdEntry = nullptr;
  m_confirmEntry = nullptr;
#ifndef NO_YUBI
  m_YubiBtn = nullptr;
  m_YubiBtn2 = nullptr;
  m_yubiStatusCtrl = nullptr;
#endif
////@end SafeCombinationChangeDlg member initialisation
}

/*!
 * Control creation for SafeCombinationChangeDlg
 */

void SafeCombinationChangeDlg::CreateControls()
{
////@begin SafeCombinationChangeDlg content construction
  SafeCombinationChangeDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please enter the current combination, followed by a new combination.\nType the new combination once again to confirm it."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

#ifndef NO_YUBI
  enum { DLGITEM_COLS = 3 };
#else
  enum { DLGITEM_COLS = 2 };
#endif

  wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(DLGITEM_COLS, 0, 0);
  itemFlexGridSizer4->AddGrowableCol(1);
  itemBoxSizer2->Add(itemFlexGridSizer4, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Old safe combination:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_oldPasswdEntry = new SafeCombinationCtrl( itemDialog1, ID_OLDPASSWD, &m_oldpasswd, wxDefaultPosition, wxDefaultSize );
  itemFlexGridSizer4->Add(m_oldPasswdEntry, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5);

#ifndef NO_YUBI
  m_YubiBtn = new wxBitmapButton( itemDialog1, ID_YUBIBTN, itemDialog1->GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  itemFlexGridSizer4->Add(m_YubiBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);
#endif

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("New safe combination:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_newPasswdEntry = new SafeCombinationCtrl( itemDialog1, ID_NEWPASSWD, &m_newpasswd, wxDefaultPosition, wxDefaultSize );
  itemFlexGridSizer4->Add(m_newPasswdEntry, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5);

#ifndef NO_YUBI
  m_YubiBtn2 = new wxBitmapButton( itemDialog1, ID_YUBIBTN2, itemDialog1->GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  itemFlexGridSizer4->Add(m_YubiBtn2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);
#endif

  wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Confirmation:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_confirmEntry = new SafeCombinationCtrl( itemDialog1, ID_CONFIRM, &m_confirm, wxDefaultPosition, wxDefaultSize );
  itemFlexGridSizer4->Add(m_confirmEntry, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5);

  itemFlexGridSizer4->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifndef NO_YUBI
  m_yubiStatusCtrl = new wxStaticText( itemDialog1, ID_YUBISTATUS, _("Please insert your YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(m_yubiStatusCtrl, 0, wxGROW|wxALL, 5);
#endif

  wxStdDialogButtonSizer* itemStdDialogButtonSizer15 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer15, 0, wxGROW|wxALL, 5);
  wxButton* itemButton16 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton16->SetDefault();
  itemStdDialogButtonSizer15->AddButton(itemButton16);

  wxButton* itemButton17 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer15->AddButton(itemButton17);

  wxButton* itemButton18 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer15->AddButton(itemButton18);

  itemStdDialogButtonSizer15->Realize();

////@end SafeCombinationChangeDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool SafeCombinationChangeDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

#ifndef NO_YUBI
wxBitmap SafeCombinationChangeDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin SafeCombinationChangeDlg bitmap retrieval
  if (name == _T("graphics/Yubikey-button.xpm"))
  {
    wxBitmap bitmap(Yubikey_button_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end SafeCombinationChangeDlg bitmap retrieval
}
#endif

/*!
 * Get icon resources
 */

wxIcon SafeCombinationChangeDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin SafeCombinationChangeDlg icon retrieval
  return wxNullIcon;
////@end SafeCombinationChangeDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SafeCombinationChangeDlg::OnOkClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    StringX errmess;
    const StringX old = m_oldresponse.empty() ? m_oldpasswd : m_oldresponse;
    int rc = m_core.CheckPasskey(m_core.GetCurFile(), old);
    if (rc == PWScore::WRONG_PASSWORD) {
      wxMessageDialog err(this, _("The old safe combination is not correct"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (rc == PWScore::CANT_OPEN_FILE) {
      wxMessageDialog err(this, _("Cannot verify old safe combination - file gone?"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (m_confirm != m_newpasswd) {
      wxMessageDialog err(this, _("New safe combination and confirmation do not match"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    // Vox populi vox dei - folks want the ability to use a weak
    // passphrase, best we can do is warn them...
    // If someone want to build a version that insists on proper
    // passphrases, then just define the preprocessor macro
    // PWS_FORCE_STRONG_PASSPHRASE in the build properties/Makefile
    // (also used in CPasskeySetup)
    } else if (!CPasswordCharPool::CheckPassword(m_newpasswd, errmess)) {
      wxString msg = _("Weak passphrase:");
      msg += wxT("\n\n");
      msg += errmess.c_str();
#ifndef PWS_FORCE_STRONG_PASSPHRASE
      msg += wxT("\n");
      msg += _("Use it anyway?");
      wxMessageDialog err(this, msg,
                          _("Error"), wxYES_NO | wxICON_HAND);
      int rc1 = err.ShowModal();
      if (rc1 == wxID_YES)
        EndModal(wxID_OK);
#else
      wxMessageDialog err(this, msg,
                          _("Error"), wxOK | wxICON_HAND);
      err.ShowModal();
#endif // PWS_FORCE_STRONG_PASSPHRASE
    } else { // password checks out OK.
      EndModal(wxID_OK);
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void SafeCombinationChangeDlg::OnCancelClick(wxCommandEvent& WXUNUSED(evt))
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in SafeCombinationChangeDlg.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CANCEL);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in SafeCombinationChangeDlg.
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
 */

void SafeCombinationChangeDlg::OnYubibtnClick(wxCommandEvent& WXUNUSED(event))
{
  // Here we just need to get the existing c/r. We verify it as a courtesy to the user,
  // that is, to indicate asap that it's incorrect.
  m_oldresponse.clear();
  // Allow blank password when Yubi's used
  m_oldPasswdEntry->AllowEmptyCombinationOnce();
  m_newPasswdEntry->AllowEmptyCombinationOnce();
  m_confirmEntry->AllowEmptyCombinationOnce();

  if (Validate() && TransferDataFromWindow()) {
    bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
    if (m_yubiMixin1.PerformChallengeResponse(this, m_oldpasswd,
                m_oldresponse, oldYubiChallenge)) {
      // Verify the response - a convenience, as we double check in OnYubibtn2Click().
      int rc = m_core.CheckPasskey(m_core.GetCurFile(), m_oldresponse);
      if (rc == PWScore::WRONG_PASSWORD) {
        m_oldresponse.clear();
        m_yubiStatusCtrl->SetForegroundColour(*wxRED);
        m_yubiStatusCtrl->SetLabel(_("YubiKey safe combination incorrect"));
      } else {
        m_yubiMixin2.SetPrompt1(_("Enter new safe combination (if any) and click on bottom Yubikey button"));
        m_yubiMixin2.UpdateStatus();
      }
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN2
 */

void SafeCombinationChangeDlg::OnYubibtn2Click(wxCommandEvent& WXUNUSED(event))
{
  // Allow blank password when Yubi's used:
  m_oldPasswdEntry->AllowEmptyCombinationOnce();
  m_newPasswdEntry->AllowEmptyCombinationOnce();
  m_confirmEntry->AllowEmptyCombinationOnce();
  if (Validate() && TransferDataFromWindow()) {
    int rc;
    // First check existing password/response:
    // 1. Both old password and old response can't be blank
    if (m_oldpasswd.empty() && m_oldresponse.empty()) {
      m_yubiStatusCtrl->SetForegroundColour(*wxRED);
      m_yubiStatusCtrl->SetLabel(_("Please confirm existing combination"));
      return;
    }
    // 2. If there's an old response, it should already have been checked, but JIC:
    if (!m_oldresponse.empty()) {
      rc = m_core.CheckPasskey(m_core.GetCurFile(), m_oldresponse);
      if (rc == PWScore::WRONG_PASSWORD) {
        m_oldresponse.clear();
        m_yubiStatusCtrl->SetForegroundColour(*wxRED);
        m_yubiStatusCtrl->SetLabel(_("YubiKey safe combination incorrect"));
        return;
      }
    } else {
      // 3. No old response, we can only check the old password
      rc = m_core.CheckPasskey(m_core.GetCurFile(), m_oldpasswd);
      if (rc == PWScore::WRONG_PASSWORD) {
        m_yubiStatusCtrl->SetForegroundColour(*wxRED);
        m_yubiStatusCtrl->SetLabel(_("Current safe combination incorrect"));
        return;
      }
    }
    StringX response;
    bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
    if (m_yubiMixin2.PerformChallengeResponse(this, m_newpasswd, response, oldYubiChallenge)) {
      m_newpasswd = response;
      EndModal(wxID_OK);
    }
  }
}

void SafeCombinationChangeDlg::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == YubiMixin::POLLING_TIMER_ID) {
    m_yubiMixin1.HandlePollingTimer();
    m_yubiMixin2.HandlePollingTimer();
  }
}
#endif
