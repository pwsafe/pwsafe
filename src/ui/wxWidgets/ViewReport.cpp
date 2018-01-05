/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ViewReport.h"
#include "./wxutils.h"

#include "../../core/Report.h"

#include "pwsclip.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

CViewReport::CViewReport(wxWindow* parent, CReport* pRpt) :
                wxDialog(parent, wxID_ANY, _("View Report"), wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),  m_pRpt(pRpt)
{
  wxASSERT(pRpt);

  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);

  wxTextCtrl* textCtrl = new wxTextCtrl(this, wxID_ANY, towxstring(pRpt->GetString()),
                                      wxDefaultPosition, wxSize(640,480), wxTE_MULTILINE|wxTE_READONLY);
  dlgSizer->Add(textCtrl, wxSizerFlags().Border(wxALL).Expand().Proportion(1));

  wxStdDialogButtonSizer* bs = CreateStdDialogButtonSizer(0);

  wxASSERT_MSG(bs, wxT("Could not create an empty wxStdDlgButtonSizer"));

  bs->Add(new wxButton(this, wxID_SAVE, _("&Save to Disk")));
  bs->AddSpacer(ColSeparation);
  bs->Add(new wxButton(this, wxID_COPY, _("&Copy to Clipboard")));
  bs->AddSpacer(ColSeparation);
  wxButton* finishButton = new wxButton(this, wxID_CLOSE, _("&Finish"));
  finishButton->SetDefault();
  bs->Add(finishButton);

  bs->Realize();

  Connect(wxID_SAVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CViewReport::OnSave) );
  Connect(wxID_COPY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CViewReport::OnCopy) );
  Connect(wxID_CLOSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CViewReport::OnClose) );

  dlgSizer->Add(bs, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxBOTTOM).Expand());

  SetSizerAndFit(dlgSizer);
}

CViewReport::~CViewReport()
{
}

void CViewReport::OnSave(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  m_pRpt->SaveToDisk();
}

void CViewReport::OnClose(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  EndModal(0);
}

void CViewReport::OnCopy(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  PWSclipboard::GetInstance()->SetData(m_pRpt->GetString());
}
