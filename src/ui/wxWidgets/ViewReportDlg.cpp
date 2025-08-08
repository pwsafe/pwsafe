/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
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

#include <wx/msgdlg.h>
#include <algorithm>

#include "core/Report.h"

#include "Clipboard.h"
#include "ViewReportDlg.h"
#include "wxUtilities.h"

ViewReportDlg::ViewReportDlg(wxWindow *parent, CReport* pRpt, bool fromFile) :
                wxDialog(parent, wxID_ANY, _("Report"), wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),  m_pRpt(pRpt)
{
  wxASSERT(pRpt);
  wxASSERT(!parent || parent->IsTopLevel());

  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);

  StringX reportString = pRpt->GetString();
  wxString reportText;
  
  try {
    reportText = towxstring(reportString);
  } catch (...) {
    reportText.Clear();
  }
    

  if (!reportText.IsEmpty()) {
    std::replace_if(reportText.begin(), reportText.end(),
                   [](wxUniChar ch) { return !wxIsprint(ch) && !wxIsspace(ch); },
                   wxT(' '));
  }
  
  wxTextCtrl* textCtrl = new wxTextCtrl(this, wxID_ANY, reportText,
                                      wxDefaultPosition, wxSize(640,480), wxTE_MULTILINE|wxTE_READONLY);
  dlgSizer->Add(textCtrl, wxSizerFlags().Border(wxALL).Expand().Proportion(1));

  wxStdDialogButtonSizer* bs = CreateStdDialogButtonSizer(0);

  wxASSERT_MSG(bs, wxT("Could not create an empty wxStdDlgButtonSizer"));

  if(fromFile) {
    auto deleteButton = new wxButton(this, wxID_APPLY, _("&Delete"));
    deleteButton->SetToolTip(_("Delete report file"));
    bs->Add(deleteButton);
  }
  else {
    auto saveButton = new wxButton(this, wxID_SAVE, _("&Save"));
    saveButton->SetToolTip(_("Save report as file"));
    bs->Add(saveButton);
  }
  bs->AddSpacer(ColSeparation);
  auto copyButton = new wxButton(this, wxID_COPY, _("&Copy"));
  copyButton->SetToolTip(_("Copy report to clipboard"));
  bs->Add(copyButton);
  bs->AddSpacer(ColSeparation);
  auto closeButton = new wxButton(this, wxID_CLOSE);
  closeButton->SetDefault();
  bs->Add(closeButton);

  bs->Realize();

  Bind(wxEVT_BUTTON, &ViewReportDlg::OnSave, this, wxID_SAVE);
  Bind(wxEVT_BUTTON, &ViewReportDlg::OnDelete, this, wxID_APPLY);
  Bind(wxEVT_BUTTON, &ViewReportDlg::OnCopy, this, wxID_COPY);
  Bind(wxEVT_BUTTON, &ViewReportDlg::OnClose, this, wxID_CLOSE);

  dlgSizer->Add(bs, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxBOTTOM).Expand());

  SetSizerAndFit(dlgSizer);
}

ViewReportDlg* ViewReportDlg::Create(wxWindow *parent, CReport* pRpt, bool fromFile)
{
  return new ViewReportDlg(parent, pRpt, fromFile);
}

void ViewReportDlg::OnSave(wxCommandEvent& WXUNUSED(evt))
{
  m_pRpt->SaveToDisk();
}

void ViewReportDlg::OnDelete(wxCommandEvent& WXUNUSED(evt))
{
  wxString fileName(m_pRpt->GetFileName());
  wxMessageDialog dlg(this, fileName, _("Delete Report?"), wxYES_NO);
  if(dlg.ShowModal() == wxID_YES) {
    m_pRpt->PurgeFromDisk();
  }
}

void ViewReportDlg::OnClose(wxCommandEvent& WXUNUSED(evt))
{
  EndModal(0);
}

void ViewReportDlg::OnCopy(wxCommandEvent& WXUNUSED(evt))
{
  Clipboard::GetInstance()->SetData(m_pRpt->GetString());
}
