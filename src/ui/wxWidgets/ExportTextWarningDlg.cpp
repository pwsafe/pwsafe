/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file editshortcut.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SafeCombinationCtrl.h"
#include "ExportTextWarningDlg.h"
#include "SelectionCriteria.h"
#include "os/file.h"

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

IMPLEMENT_CLASS( CExportTextWarningDlgBase, wxDialog )

BEGIN_EVENT_TABLE( CExportTextWarningDlgBase, wxDialog )
  EVT_BUTTON( ID_ADVANCED, CExportTextWarningDlgBase::OnAdvancedSelection )
#ifndef NO_YUBI
  EVT_BUTTON( ID_YUBIBTN, CExportTextWarningDlgBase::OnYubibtnClick )
  EVT_TIMER(POLLING_TIMER_ID, CExportTextWarningDlgBase::OnPollingTimer)
#endif
END_EVENT_TABLE()

CExportTextWarningDlgBase::CExportTextWarningDlgBase(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxEmptyString,
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
  selCriteria(new SelectionCriteria), m_combinationEntry(nullptr), m_YubiBtn(nullptr), m_yubiStatusCtrl(nullptr), m_pollingTimer(nullptr)
{
  enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 10, ColSeparation = 20};

  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);

  wxString warningTxt(_("Warning! This operation will create an unprotected copy of ALL of the passwords\nin the database. Deleting this copy after use is NOT sufficient."));
  wxString warningTxt2(_("Please do not use this option unless you understand and accept the risks. This option\nbypasses the security provided by this program."));

  wxStaticText* rt = new wxStaticText(this, wxID_ANY, warningTxt + wxT("\n\n") + warningTxt2, wxDefaultPosition,
                                              wxSize(-1, 200));
  rt->SetForegroundColour(*wxRED);
  dlgSizer->Add(rt, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(1).Expand());
  dlgSizer->AddSpacer(RowSeparation);

  wxBoxSizer* pwdCtrl = new wxBoxSizer(wxHORIZONTAL);
  pwdCtrl->Add(new wxStaticText(this, wxID_ANY, _("Safe Combination:")));
  pwdCtrl->AddSpacer(ColSeparation);
  m_combinationEntry = new CSafeCombinationCtrl(this, wxID_ANY, &passKey);
  pwdCtrl->Add(m_combinationEntry, wxSizerFlags().Expand().Proportion(1));
  dlgSizer->Add(pwdCtrl, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());
#ifndef NO_YUBI
  auto yubiSizer = new wxBoxSizer(wxHORIZONTAL);
  yubiSizer->AddSpacer(ColSeparation);
  auto yubiBtn = new wxBitmapButton(this , ID_YUBIBTN, wxBitmap(Yubikey_button_xpm),
                                    wxDefaultPosition, ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  yubiSizer->Add(yubiBtn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);

  auto yubiStatusCtrl = new wxStaticText(this, ID_YUBISTATUS, _("Please insert your YubiKey"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
  yubiSizer->Add(yubiStatusCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  dlgSizer->Add(yubiSizer);
#endif
  dlgSizer->AddSpacer(RowSeparation);

  delimiter = wxT('\xbb');
  wxTextValidator delimValidator(wxFILTER_EXCLUDE_CHAR_LIST, &delimiter);
  const wxChar* excludes[] = {wxT("\""), 0};
  delimValidator.SetExcludes(wxArrayString(1, excludes));
  wxBoxSizer* delimRow = new wxBoxSizer(wxHORIZONTAL);
  delimRow->Add(new wxStaticText(this, wxID_ANY, _("Line delimiter in Notes field:")));
  delimRow->AddSpacer(ColSeparation);
  delimRow->Add(new wxTextCtrl(this, ID_LINE_DELIMITER, wxT("\xbb"), wxDefaultPosition, wxDefaultSize, 0,
                                delimValidator));
  delimRow->AddSpacer(ColSeparation);
  delimRow->Add(new wxStaticText(this, wxID_ANY, _("Also used to replace periods in the Title field")));

  dlgSizer->Add(delimRow, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin));
  dlgSizer->AddSpacer(RowSeparation);

  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Center());
  dlgSizer->AddSpacer(RowSeparation);

  wxStdDialogButtonSizer* buttons = CreateStdDialogButtonSizer(wxOK|wxCANCEL|wxHELP);
  //This might not be a very wise thing to do.  We are only supposed to add certain
  //pre-defined button-ids to StdDlgBtnSizer
  buttons->Add(new wxButton(this, ID_ADVANCED, _("Advanced...")), wxSizerFlags().Border(wxLEFT|wxRIGHT));
  dlgSizer->Add(buttons, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Center());

  dlgSizer->AddSpacer(BottomMargin);

  SetSizerAndFit(dlgSizer);
#ifndef NO_YUBI
  SetupMixin(FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_pollingTimer->Start(CYubiMixin::POLLING_INTERVAL);
#endif
}

CExportTextWarningDlgBase::~CExportTextWarningDlgBase()
{
  delete selCriteria;
  delete m_pollingTimer;
}

void CExportTextWarningDlgBase::OnAdvancedSelection( wxCommandEvent& evt )
{
  UNREFERENCED_PARAMETER(evt);
  DoAdvancedSelection();
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YUBIBTN
 */

void CExportTextWarningDlgBase::OnYubibtnClick( wxCommandEvent& /* event */ )
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

void CExportTextWarningDlgBase::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    HandlePollingTimer(); // in CYubiMixin
  }
}
#endif
