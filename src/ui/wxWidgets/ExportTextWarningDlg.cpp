/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

#include "ExportTextWarningDlg.h"
#include "ExternalKeyboardButton.h"
#include "SafeCombinationCtrl.h"
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
  selCriteria(new SelectionCriteria), m_combinationEntry(nullptr)
{
  wxASSERT(!parent || parent->IsTopLevel());

  auto *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  wxString text(
    _("Warning! This operation will create an unprotected copy of ALL of the passwords\nin the database. Deleting this copy after use is NOT sufficient.")
    + wxT("\n\n") +
    _("Do not use this option unless you understand and accept the risks. This option\nbypasses the security provided by this program.")
  );

  auto *warningText = new wxStaticText(this, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, 0);
  warningText->SetForegroundColour(*wxRED);
  mainSizer->Add(warningText, 0, wxALIGN_LEFT|wxALL, 12);

  m_combinationEntry = wxUtilities::CreateLabeledSafeCombinationCtrl(this, wxID_ANY, _("Master Password"), &passKey, true);

  delimiter = wxT('\xbb');
  wxTextValidator delimValidator(wxFILTER_EXCLUDE_CHAR_LIST, &delimiter);
  const wxChar* excludes[] = {wxT("\""), nullptr};
  delimValidator.SetExcludes(wxArrayString(1, excludes));

  auto *verticalBoxSizer1 = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(verticalBoxSizer1, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *delimiterStaticText = new wxStaticText(this, wxID_ANY, _("Line Delimiter"), wxDefaultPosition, wxDefaultSize, 0);
  verticalBoxSizer1->Add(delimiterStaticText, 0, wxBOTTOM|wxALIGN_LEFT, 5);

  auto *horizontalBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  verticalBoxSizer1->Add(horizontalBoxSizer1, 1, wxEXPAND, 0);

  auto *delimiterTextCtrl = new wxTextCtrl(this, ID_LINE_DELIMITER, wxT("\xbb"), wxDefaultPosition, wxDefaultSize, 0, delimValidator);
  delimiterTextCtrl->SetToolTip(_("The delimiter in the Notes field, also used to replace periods in the Title field."));
  auto *advancedButton = new wxButton(this, ID_ADVANCED, _("&Advanced..."), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer1->Add(delimiterTextCtrl, 0, 0, 0);
  horizontalBoxSizer1->AddStretchSpacer();
  horizontalBoxSizer1->Add(advancedButton, 0, 0, 0);

#ifndef NO_YUBI
  wxUtilities::CreateYubiKeyControls(this, ID_YUBIBTN, ID_YUBISTATUS);
#endif

  mainSizer->AddStretchSpacer();

  auto *horizontalBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizontalBoxSizer2, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 12);

  auto *helpButton = new wxButton(this, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0);
  horizontalBoxSizer2->Add(helpButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  horizontalBoxSizer2->AddStretchSpacer();

  auto *stdDialogButtonSizer = new wxStdDialogButtonSizer;
  horizontalBoxSizer2->Add(stdDialogButtonSizer, 0, wxALIGN_CENTER_VERTICAL, 0);

  auto exportButton = new wxButton(this, wxID_OK, _("&Export"), wxDefaultPosition, wxDefaultSize, 0);
  exportButton->SetDefault();
  stdDialogButtonSizer->AddButton(exportButton);

  auto *cancelButton = new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  stdDialogButtonSizer->AddButton(cancelButton);

  stdDialogButtonSizer->Realize();

  horizontalBoxSizer2->AddStretchSpacer();

  if (wxUtilities::IsVirtualKeyboardSupported()) {
    auto *keyboardButton = new ExternalKeyboardButton(this);
    keyboardButton->SetFocusOnSafeCombinationCtrl(m_combinationEntry);
    horizontalBoxSizer2->Add(
      keyboardButton,
      0, wxALIGN_CENTER_VERTICAL|wxALL, 0
    );
  }

  SetSizerAndFit(mainSizer);
#ifndef NO_YUBI
  SetupMixin(this, FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
#endif
}

ExportTextWarningDlgBase::~ExportTextWarningDlgBase()
{
  delete selCriteria;
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
