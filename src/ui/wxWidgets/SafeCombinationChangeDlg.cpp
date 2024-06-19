/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

#include <wx/statline.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

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
  EVT_TIMER(  YubiMixin::POLLING_TIMER_ID,  SafeCombinationChangeDlg::OnPollingTimer  )
#endif
  EVT_BUTTON( wxID_OK,                      SafeCombinationChangeDlg::OnOkClick       )
  EVT_BUTTON( wxID_CANCEL,                  SafeCombinationChangeDlg::OnCancelClick   )

////@end SafeCombinationChangeDlg event table entries
END_EVENT_TABLE()

/*!
 * SafeCombinationChangeDlg constructors
 */

SafeCombinationChangeDlg::SafeCombinationChangeDlg(wxWindow *parent, PWScore &core,
                                               wxWindowID id, const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core)
{
  wxASSERT(!parent || parent->IsTopLevel());
////@begin SafeCombinationChangeDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end SafeCombinationChangeDlg creation
#ifndef NO_YUBI
  m_yubiMixin1.SetupMixin(this, FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS), YubiMixin::POLLING_TIMER_ID);
  m_yubiMixin1.SetPrompt1(_("Enter old master password and click on top Yubikey button"));
  m_yubiMixin2.SetupMixin(this, FindWindow(ID_YUBIBTN2), FindWindow(ID_YUBISTATUS), YubiMixin::POLLING_TIMER_NONE);
  m_yubiMixin2.SetPrompt1(_("Enter old master password and click on top Yubikey button"));
#endif
}


SafeCombinationChangeDlg* SafeCombinationChangeDlg::Create(wxWindow *parent, PWScore &core,
  wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
  return new SafeCombinationChangeDlg(parent, core, id, caption, pos, size, style);
}

/*!
 * SafeCombinationChangeDlg destructor
 */

SafeCombinationChangeDlg::~SafeCombinationChangeDlg()
{
////@begin SafeCombinationChangeDlg destruction
////@end SafeCombinationChangeDlg destruction
}

/*!
 * Control creation for SafeCombinationChangeDlg
 */

void SafeCombinationChangeDlg::CreateControls()
{
  auto *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  auto *itemStaticText3 = new wxStaticText(
    this, wxID_STATIC,
    _("Enter the current master password, followed by a new one.\nType the new one again to confirm it."),
    wxDefaultPosition, wxDefaultSize, 0
  );
  mainSizer->Add(itemStaticText3, 0, wxALL|wxEXPAND, 12);

#ifndef NO_YUBI
  enum { DLGITEM_COLS = 2 };
#else
  enum { DLGITEM_COLS = 1 };
#endif

  auto *itemFlexGridSizer4 = new wxFlexGridSizer(DLGITEM_COLS, 0, 0);
  itemFlexGridSizer4->AddGrowableCol(0);
  mainSizer->Add(itemFlexGridSizer4, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *itemStaticText5 = new wxStaticText(this, wxID_STATIC, _("Old Master Password"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5);
#ifndef NO_YUBI
  itemFlexGridSizer4->AddStretchSpacer(0);
#endif

  m_oldPasswdEntry = new SafeCombinationCtrl(this, ID_OLDPASSWD, &m_oldpasswd, wxDefaultPosition, wxDefaultSize);
  m_oldPasswdEntry->SetFocus();
  itemFlexGridSizer4->Add(m_oldPasswdEntry, 1, wxALIGN_LEFT|wxBOTTOM|wxEXPAND, 12);
#ifndef NO_YUBI
  m_YubiBtn = new wxBitmapButton(this, ID_YUBIBTN, GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  itemFlexGridSizer4->Add(m_YubiBtn, 0, wxLEFT|wxBOTTOM|wxEXPAND, 12);
#endif

  auto *itemStaticText8 = new wxStaticText(this, wxID_STATIC, _("New Master Password"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer4->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5);
#ifndef NO_YUBI
  itemFlexGridSizer4->AddStretchSpacer(0);
#endif

  m_newPasswdEntry = new SafeCombinationCtrl(this, ID_NEWPASSWD, &m_newpasswd, wxDefaultPosition, wxDefaultSize);
  itemFlexGridSizer4->Add(m_newPasswdEntry, 1, wxALIGN_LEFT|wxBOTTOM|wxEXPAND, 12);
#ifndef NO_YUBI
  m_YubiBtn2 = new wxBitmapButton(this, ID_YUBIBTN2, GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  itemFlexGridSizer4->Add(m_YubiBtn2, 0, wxLEFT|wxBOTTOM|wxEXPAND, 12);
#endif

  auto *itemStaticText11 = new wxStaticText(this, wxID_STATIC, _("Confirmation"), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5);
#ifndef NO_YUBI
  itemFlexGridSizer4->AddStretchSpacer(0);
#endif

  m_confirmEntry = new SafeCombinationCtrl(this, ID_CONFIRM, &m_confirm, wxDefaultPosition, wxDefaultSize);
  itemFlexGridSizer4->Add(m_confirmEntry, 1, wxALIGN_LEFT|wxBOTTOM|wxEXPAND, 12);
#ifndef NO_YUBI
  itemFlexGridSizer4->AddStretchSpacer(0);
#endif

#ifndef NO_YUBI
  auto* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1,  40));
  mainSizer->Add(panel, 0, wxLEFT|wxRIGHT|wxEXPAND|wxBOTTOM, 12);

  auto* horizontalBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  panel->SetSizer(horizontalBoxSizer1);

  m_yubiStatusCtrl = new wxStaticText(panel, ID_YUBISTATUS, _("Insert your YubiKey"), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer1->Add(m_yubiStatusCtrl, 0, wxALIGN_LEFT|wxEXPAND, 0);
#endif

  auto horizontalBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizontalBoxSizer3, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 12);

  horizontalBoxSizer3->AddSpacer(90);
  horizontalBoxSizer3->AddStretchSpacer();

  horizontalBoxSizer3->Add(
    new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );

  auto *changeButton = new wxButton(this, wxID_OK, _("&Change"), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer3->Add(
    changeButton,
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );
  changeButton->SetDefault();

  horizontalBoxSizer3->AddSpacer(60);
  horizontalBoxSizer3->AddStretchSpacer();

  if (wxUtilities::IsVirtualKeyboardSupported()) {
    auto *keyboardButton = new ExternalKeyboardButton(this);
    keyboardButton->SetFocusOnSafeCombinationCtrl(m_oldPasswdEntry);
    horizontalBoxSizer3->Add(
      keyboardButton,
      0, wxALIGN_CENTER_VERTICAL|wxALL, 0
    );
  }
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
      wxMessageDialog err(this, _("The old master password is not correct"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (rc == PWScore::CANT_OPEN_FILE) {
      wxMessageDialog err(this, _("Cannot verify old master password - file gone?"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (m_confirm != m_newpasswd) {
      wxMessageDialog err(this, _("New master password and confirmation do not match"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (CheckPasswordStrengthAndWarn(this, m_newpasswd)) {
      EndModal(wxID_OK); // password checks out OK.
    }
    // If we got here, there was an error in a PW entry.  A case exists where switching from a
    // (Yubikey without a PW), to (PW only), we may need to allow the old PW to be
    // blank again for the next retry.
    if (!m_oldresponse.empty() && m_oldpasswd.empty())
      m_oldPasswdEntry->AllowEmptyCombinationOnce();
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
        m_yubiStatusCtrl->SetLabel(_("YubiKey master password incorrect"));
      } else {
        // The old password has been validated.  If it was blank, allow it to be blank on the next check.
        // e.g. changing from (Yubikey w/blank PW) to (PW only).
        if (m_oldpasswd.empty()) {
          m_oldPasswdEntry->AllowEmptyCombinationOnce();
        }
        m_yubiMixin2.SetPrompt1(_("Enter new master password and click on bottom Yubikey button"));
        m_yubiMixin2.UpdateStatus();
        m_newPasswdEntry->SetFocus();
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
      m_yubiStatusCtrl->SetLabel(_("Confirm existing master password"));
      return;
    }
    // 2. If there's an old response, it should already have been checked, but JIC:
    if (!m_oldresponse.empty()) {
      rc = m_core.CheckPasskey(m_core.GetCurFile(), m_oldresponse);
      if (rc == PWScore::WRONG_PASSWORD) {
        m_oldresponse.clear();
        m_yubiStatusCtrl->SetForegroundColour(*wxRED);
        m_yubiStatusCtrl->SetLabel(_("YubiKey master password incorrect"));
        return;
      }
    } else {
      // 3. No old response, we can only check the old password
      rc = m_core.CheckPasskey(m_core.GetCurFile(), m_oldpasswd);
      if (rc == PWScore::WRONG_PASSWORD) {
        m_yubiStatusCtrl->SetForegroundColour(*wxRED);
        m_yubiStatusCtrl->SetLabel(_("Current master password incorrect"));
        return;
      }
    }

    if (m_confirm != m_newpasswd) {
      wxMessageDialog err(this, _("New master password and confirmation do not match"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
      return;
    }
    // A blank password with a Yubikey is a common use case
    if (!m_newpasswd.empty() && !CheckPasswordStrengthAndWarn(this, m_newpasswd)) {
      return;
    }

    StringX response;
    bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
    if (m_yubiMixin2.PerformChallengeResponse(this, m_newpasswd, response, oldYubiChallenge)) {
      m_newpasswd = response;
      m_IsYubiProtected = true; // Don't erase YubiSK, since database is protected by Yubikey
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
