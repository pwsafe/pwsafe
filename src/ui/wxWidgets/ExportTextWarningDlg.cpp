/*
 * Copyright (c) 2003-2022 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ExportTextWarningDlg.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SafeCombinationCtrl.h"
#include "ExportTextWarningDlg.h"
#include "SelectionCriteria.h"

#include <wx/statline.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#ifndef NO_YUBI
#include "graphics/Yubikey-button.xpm"

#define ID_YUBIBTN 10001
#define ID_YUBISTATUS 10002
#endif

enum { ID_COMBINATION = 100, ID_VKBD, ID_LINE_DELIMITER, ID_ADVANCED };

IMPLEMENT_CLASS( ExportTextWarningDlgBase, wxDialog )

BEGIN_EVENT_TABLE( ExportTextWarningDlgBase, wxDialog )
  EVT_BUTTON( ID_ADVANCED, ExportTextWarningDlgBase::OnAdvancedSelection )
#ifndef NO_YUBI
  EVT_BUTTON( ID_YUBIBTN, ExportTextWarningDlgBase::OnYubibtnClick )
  EVT_TIMER(POLLING_TIMER_ID, ExportTextWarningDlgBase::OnPollingTimer)
#endif
END_EVENT_TABLE()

ExportTextWarningDlgBase::ExportTextWarningDlgBase(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxEmptyString,
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
  selCriteria(new SelectionCriteria), m_combinationEntry(nullptr), m_pollingTimer(nullptr)
{
  wxASSERT(!parent || parent->IsTopLevel());

  auto mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->AddSpacer(TopMargin);

  wxString text(
    _("Warning! This operation will create an unprotected copy of ALL of the passwords\nin the database. Deleting this copy after use is NOT sufficient.")
    + wxT("\n\n") +
    _("Please do not use this option unless you understand and accept the risks. This option\nbypasses the security provided by this program.")
  );

  auto warningText = new wxStaticText(this, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, 0);

  warningText->SetForegroundColour(*wxRED);
  mainSizer->Add(warningText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, SideMargin);
  mainSizer->AddSpacer(BottomMargin);

#ifndef NO_YUBI
  int DLGITEM_COLS = 3;
#else
  int DLGITEM_COLS = 2;
#endif

  auto flexGridSizer = new wxFlexGridSizer(DLGITEM_COLS /*cols*/, 0 /*vgap*/, 0 /*hgap*/);
  flexGridSizer->AddGrowableCol(1);

  auto safeCombinationStaticText = new wxStaticText(this, wxID_ANY, _("Safe Combination:"), wxDefaultPosition, wxDefaultSize, 0);
  m_combinationEntry = new SafeCombinationCtrl(this, wxID_ANY, &passKey);
  m_combinationEntry->SetFocus();
  auto showCombinationCheckBox = new wxCheckBox(this, wxID_ANY, _("Show Combination"), wxDefaultPosition, wxDefaultSize, 0 );
  showCombinationCheckBox->SetValue(false);
  showCombinationCheckBox->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& event) {m_combinationEntry->SecureTextfield(!event.IsChecked());});

  flexGridSizer->Add(safeCombinationStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  flexGridSizer->Add(m_combinationEntry       , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

#ifndef NO_YUBI
  auto yubiBtn = new wxBitmapButton(this , ID_YUBIBTN, wxBitmap(Yubikey_button_xpm),
    wxDefaultPosition, ConvertDialogToPixels(wxSize(40, 12)), wxBU_AUTODRAW );
  flexGridSizer->Add(yubiBtn, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxSHAPED, 5);
#endif

  flexGridSizer->AddStretchSpacer(0);                                // 1st column of wxFlexGridSizer
  flexGridSizer->Add(showCombinationCheckBox, 0, wxALL|wxEXPAND, 5); // 2nd column of wxFlexGridSizer

#ifndef NO_YUBI
  flexGridSizer->AddStretchSpacer(0);                                // 3rd column of wxFlexGridSizer
#endif

#ifndef NO_YUBI
  auto yubiStatusCtrl = new wxStaticText(this, ID_YUBISTATUS, _("Please insert your YubiKey"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->AddStretchSpacer(0);                                // 1st column of wxFlexGridSizer
  flexGridSizer->Add(yubiStatusCtrl, 1, wxALL|wxEXPAND, 5);          // 2nd column of wxFlexGridSizer
  flexGridSizer->AddStretchSpacer(0);                                // 3rd column of wxFlexGridSizer
#endif

  delimiter = wxT('\xbb');
  wxTextValidator delimValidator(wxFILTER_EXCLUDE_CHAR_LIST, &delimiter);
  const wxChar* excludes[] = {wxT("\""), nullptr};
  delimValidator.SetExcludes(wxArrayString(1, excludes));

  auto delimiterStaticText     = new wxStaticText(this, wxID_ANY, _("Line delimiter in Notes field:"), wxDefaultPosition, wxDefaultSize, 0);
  auto delimiterTextCtrl       = new wxTextCtrl(this, ID_LINE_DELIMITER, wxT("\xbb"), wxDefaultPosition, wxDefaultSize, 0, delimValidator);
  auto delimiterInfoStaticText = new wxStaticText(this, wxID_ANY, _("Also used to replace periods in the Title field"), wxDefaultPosition, wxDefaultSize, 0);

  auto delimiterSizer = new wxBoxSizer(wxHORIZONTAL);
  delimiterSizer->Add(delimiterTextCtrl      , 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5);
  delimiterSizer->Add(delimiterInfoStaticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  flexGridSizer->Add(delimiterStaticText     , 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  flexGridSizer->Add(delimiterSizer          , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  mainSizer->Add(flexGridSizer, 0, wxLEFT|wxRIGHT|wxEXPAND, SideMargin);
  mainSizer->AddSpacer(RowSeparation);

  mainSizer->AddStretchSpacer();

  mainSizer->Add(new wxStaticLine(this), 0, wxLEFT|wxRIGHT|wxEXPAND, SideMargin);
  mainSizer->AddSpacer(RowSeparation);

  auto stdDialogButtonSizer = new wxStdDialogButtonSizer;

  auto okButton = new wxButton(this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
  okButton->SetDefault();
  stdDialogButtonSizer->AddButton(okButton);

  auto cancelButton = new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  stdDialogButtonSizer->AddButton(cancelButton);

  auto helpButton = new wxButton(this, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0);
  stdDialogButtonSizer->AddButton(helpButton);

  stdDialogButtonSizer->Realize();

  //This might not be a very wise thing to do.  We are only supposed to add certain
  //pre-defined button-ids to StdDlgBtnSizer
  auto advancedButton = new wxButton(this, ID_ADVANCED, _("&Advanced..."), wxDefaultPosition, wxDefaultSize, 0);
  stdDialogButtonSizer->Add(advancedButton, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, SideMargin);

  mainSizer->Add(stdDialogButtonSizer, 0, wxLEFT|wxRIGHT|wxEXPAND, SideMargin);

  mainSizer->AddSpacer(BottomMargin);

  SetSizerAndFit(mainSizer);
#ifndef NO_YUBI
  SetupMixin(FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_pollingTimer->Start(YubiMixin::POLLING_INTERVAL);
#endif
}

ExportTextWarningDlgBase::~ExportTextWarningDlgBase()
{
  delete selCriteria;
  delete m_pollingTimer;
}

void ExportTextWarningDlgBase::OnAdvancedSelection( wxCommandEvent& evt )
{
  UNREFERENCED_PARAMETER(evt);
  CallAfter(&ExportTextWarningDlgBase::DoAdvancedSelection);
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
 */

void ExportTextWarningDlgBase::OnYubibtnClick(wxCommandEvent& WXUNUSED(event))
{
  m_combinationEntry->AllowEmptyCombinationOnce();  // Allow blank password when Yubi's used

  if (Validate() && TransferDataFromWindow()) {
    StringX response;
    bool oldYubiChallenge = ::wxGetKeyState(WXK_SHIFT); // for pre-0.94 databases
    if (PerformChallengeResponse(this, passKey, response, oldYubiChallenge)) {
      passKey = response;
      EndModal(wxID_OK);
      return;
    }
  }
}

void ExportTextWarningDlgBase::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    HandlePollingTimer(); // in YubiMixin
  }
}
#endif
