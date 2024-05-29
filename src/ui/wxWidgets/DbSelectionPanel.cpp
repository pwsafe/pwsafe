/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DbSelectionPanel.cpp
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

#include <wx/filename.h>

#include "core/PWScore.h"

#include "DbSelectionPanel.h"
#include "OpenFilePickerValidator.h"
#include "SafeCombinationCtrl.h"
#include "wxUtilities.h"

#ifndef NO_YUBI
#include "graphics/Yubikey-button.xpm"

#define ID_YUBIBTN 10001
#define ID_YUBISTATUS 10002
#endif

DbSelectionPanel::DbSelectionPanel(wxWindow* parent, 
                                    const wxString& filePrompt,
                                    const wxString& filePickerCtrlTitle,
                                    bool autoValidate,
                                    PWScore* core,
                                    unsigned rowsep,
                                    int buttonConfirmationId,
                                    const wxString filename) : wxPanel(parent),
                                                                m_filepicker(nullptr),
                                                                m_sc(nullptr),
                                                                m_bAutoValidate(autoValidate),
                                                                m_core(core),
                                                                m_confirmationButtonId(buttonConfirmationId)
{
  wxSizerFlags borderFlags = wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin);

  /* This doesn't work since the second Border() call overwrites all the
   * previous border values.  So now we have to insert separators by hand
   */
  //wxSizerFlags rowFlags = borderFlags.Border(wxBOTTOM, RowSeparation);

  wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
  panelSizer->AddSpacer(TopMargin);

  panelSizer->Add(new wxStaticText(this, wxID_ANY, filePrompt), borderFlags);
  panelSizer->AddSpacer(RowSeparation);
  m_filepath = filename;
  OpenFilePickerValidator validator(m_filepath);
  m_filepicker = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
                                          filePickerCtrlTitle,
                                          _("Password Safe Databases (*.psafe4; *.psafe3; *.dat)|*.psafe4;*.psafe3;*.dat|Password Safe Backups (*.bak)|*.bak|Password Safe Intermediate Backups (*.ibak)|*.ibak|All files (*.*; *)|*.*;*"), 
                                          wxDefaultPosition, wxDefaultSize, 
                                          wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL, 
                                          validator);
  panelSizer->Add(m_filepicker, borderFlags.Expand());
  panelSizer->AddSpacer(RowSeparation*rowsep);
  m_filepicker->Connect( m_filepicker->GetEventType(), 
             wxFileDirPickerEventHandler(DbSelectionPanel::OnFilePicked),
             nullptr, this);

  panelSizer->Add(new wxStaticText(this, wxID_ANY, _("Master Password:")), borderFlags);
  panelSizer->AddSpacer(RowSeparation);
  
  m_sc = new SafeCombinationCtrl(this);
  m_sc->SetValidatorTarget(&m_combination);

  auto horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
  horizontalSizer->Add(m_sc, 1, wxALL|wxEXPAND, 0);

#ifndef NO_YUBI
  auto yubiBtn = new wxBitmapButton(this , ID_YUBIBTN, wxBitmap(Yubikey_button_xpm),
    wxDefaultPosition, ConvertDialogToPixels(wxSize(40, 12)), wxBU_AUTODRAW );
  horizontalSizer->Add(yubiBtn, 0, wxALIGN_CENTER|wxLEFT|wxSHAPED, 5);
  Bind(wxEVT_BUTTON, &DbSelectionPanel::OnYubibtnClick, this);
#endif

  panelSizer->Add(horizontalSizer, borderFlags.Expand());
  panelSizer->AddSpacer(5);

#ifndef NO_YUBI
  auto yubiStatusCtrl = new wxStaticText(this, ID_YUBISTATUS, _("Insert YubiKey"),
    wxDefaultPosition, wxDefaultSize, 0 );
  panelSizer->Add(yubiStatusCtrl, borderFlags.Expand());
#endif

  SetSizerAndFit(panelSizer);

#ifndef NO_YUBI
  SetupMixin(this, FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  Bind(wxEVT_TIMER, &DbSelectionPanel::OnPollingTimer, this);
#endif
  
  //The parent window must call our TransferDataToWindow and TransferDataFromWindow
  m_parent->SetExtraStyle(m_parent->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
}

void DbSelectionPanel::SelectCombinationText()
{
  m_sc->SelectCombinationText();
}

bool DbSelectionPanel::DoValidation()
{
  //the data has not been transferred from the window to our members yet, so get them from the controls
  if (wxWindow::Validate()) {

    wxFileName wxfn(m_filepicker->GetPath());

    //Did the user enter a valid file path
    if (!wxfn.FileExists()) {
      wxMessageBox( _("File or path not found."), _("Error"), wxOK | wxICON_EXCLAMATION, this);
      return false;
    }

    //Did he enter the same file that's currently open?
    if (wxfn.SameAs(wxFileName(towxstring(m_core->GetCurFile())))) {
      // It is the same damn file
      wxMessageBox(_("That file is already open."), _("Error"), wxOK | wxICON_WARNING, this);
      return false;
    }

    m_combination = m_yubiCombination.empty() ? m_sc->GetCombination() : m_yubiCombination;
    //Does the combination match?
    if (m_core->CheckPasskey(tostringx(wxfn.GetFullPath()), m_combination) != PWScore::SUCCESS) {
      wxString errmess(_("Incorrect master password, not a Password Safe database,\nor a corrupt database."));
      wxMessageBox(errmess, _("Can't open a password database"), wxOK | wxICON_ERROR, this);
      SelectCombinationText();
      m_combination.clear();
      return false;
    }

    return true;
  }
  else {
    return false;
  }
}

bool DbSelectionPanel::TransferDataFromWindow()
{
  // We need to override this way because putting the Yubikey-derived passphrase in the m_sc control
  // would potentially expose it to users.
  bool retval = wxPanel::TransferDataFromWindow();
  if (!m_yubiCombination.empty()) {
    m_combination = m_yubiCombination;
  }
  return retval;
}


void DbSelectionPanel::OnFilePicked(wxFileDirPickerEvent& WXUNUSED(event))
{
  // Don't shift focus if we are in the text ctrl
  if ( !wxDynamicCast(FindFocus(), wxTextCtrl) )
    m_sc->SelectCombinationText();
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
 */

void DbSelectionPanel::OnYubibtnClick(wxCommandEvent& WXUNUSED(event))
{
  m_sc->AllowEmptyCombinationOnce();  // Allow blank password when Yubi's used

  if (TransferDataFromWindow()) { // don't Validate(), as password won't be right. (BR877)
    StringX response;
    bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
    if (PerformChallengeResponse(this, m_combination, response, oldYubiChallenge)) {
      m_yubiCombination = m_combination = response;
      GetParent()->GetEventHandler()->AddPendingEvent(wxCommandEvent(wxEVT_BUTTON, m_confirmationButtonId));
      return;
    }
  }
}

void DbSelectionPanel::OnPollingTimer(wxTimerEvent& event)
{
  if (event.GetId() == POLLING_TIMER_ID) {
    HandlePollingTimer(); // in YubiMixin
  }
}
#endif
