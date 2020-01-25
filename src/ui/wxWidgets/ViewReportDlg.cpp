/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ViewReportDlg.cpp
* 
*/

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "core/Report.h"

#include "Clipboard.h"
#include "ViewReportDlg.h"
#include "wxUtilities.h"

ViewReportDlg::ViewReportDlg(wxWindow* parent, CReport* pRpt) :
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

  Connect(wxID_SAVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ViewReportDlg::OnSave) );
  Connect(wxID_COPY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ViewReportDlg::OnCopy) );
  Connect(wxID_CLOSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ViewReportDlg::OnClose) );

  dlgSizer->Add(bs, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxBOTTOM).Expand());

  SetSizerAndFit(dlgSizer);
}

ViewReportDlg::~ViewReportDlg()
{
}

void ViewReportDlg::OnSave(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  m_pRpt->SaveToDisk();
}

void ViewReportDlg::OnClose(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  EndModal(0);
}

void ViewReportDlg::OnCopy(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  Clipboard::GetInstance()->SetData(m_pRpt->GetString());
}
