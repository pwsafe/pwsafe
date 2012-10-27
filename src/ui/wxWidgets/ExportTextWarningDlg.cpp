/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "ExportTextWarningDlg.h"
#include "SafeCombinationCtrl.h"

#include "graphics/Yubikey-button.xpm"

#include <wx/statline.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

enum { ID_COMBINATION = 100, ID_VKBD, ID_LINE_DELIMITER,
       ID_ADVANCED, ID_YUBIBTN, ID_YUBISTATUS};

IMPLEMENT_CLASS( CExportTextWarningDlgBase, wxDialog )

BEGIN_EVENT_TABLE( CExportTextWarningDlgBase, wxDialog )
  EVT_BUTTON( ID_ADVANCED, CExportTextWarningDlgBase::OnAdvancedSelection )
  EVT_BUTTON( ID_YUBIBTN, CExportTextWarningDlgBase::OnYubibtnClick )
  EVT_TIMER(POLLING_TIMER_ID, CExportTextWarningDlgBase::OnPollingTimer)
END_EVENT_TABLE()


CExportTextWarningDlgBase::CExportTextWarningDlgBase(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxEmptyString,
                                                                                  wxDefaultPosition, wxDefaultSize, 
                                                                                  wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
  defDelim(wxT('\xbb')), delimiter(defDelim)
{
  enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 10, ColSeparation = 20};
  
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);
  
  wxString warningTxt(_("Warning! This operation will create an unprotected copy of ALL of the passwords\nin the database. Deleting this copy after use is NOT sufficient."));
  wxString warningTxt2(_("Please do not use this option unless you understand and accept the risks. This option\nbypasses the security provided by this program."));
  
  wxStaticText* rt = new wxStaticText(this, wxID_ANY, warningTxt + _("\n\n") + warningTxt2, wxDefaultPosition, 
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
  dlgSizer->AddSpacer(RowSeparation);

  wxBoxSizer* yubiSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBitmap yubi_bitmap(Yubikey_button_xpm);
  m_YubiBtn = new wxBitmapButton( this, ID_YUBIBTN, yubi_bitmap, wxDefaultPosition, this->ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  yubiSizer->Add(m_YubiBtn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);

  m_yubiStatusCtrl = new wxStaticText( this, ID_YUBISTATUS, _("Please insert your YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  yubiSizer->Add(m_yubiStatusCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  dlgSizer->Add(yubiSizer, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());
  dlgSizer->AddSpacer(RowSeparation);

  wxTextValidator delimValidator(wxFILTER_EXCLUDE_CHAR_LIST, &delimiter);
  const wxChar* excludes[] = {_("\""), 0};
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

  SetupMixin(FindWindow(ID_YUBIBTN), FindWindow(ID_YUBISTATUS));
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
  m_pollingTimer->Start(250); // check for Yubikey every 250ms.

}

void CExportTextWarningDlgBase::OnAdvancedSelection( wxCommandEvent & )
{
  DoAdvancedSelection();
}

void CExportTextWarningDlgBase::OnYubibtnClick( wxCommandEvent & )
{
  m_combinationEntry->AllowEmptyCombinationOnce();  // Allow blank password when Yubi's used

  if (Validate() && TransferDataFromWindow()) {
    StringX response;
    if (PerformChallengeResponse(passKey, response)) {
      passKey = response;
      EndModal(wxID_OK);
    }
  }
}

void CExportTextWarningDlgBase::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    HandlePollingTimer(); // in CYubiMixin
  }
}
