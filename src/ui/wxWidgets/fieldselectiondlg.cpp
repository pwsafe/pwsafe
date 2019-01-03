/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file fieldselectionpanel.cpp
 * 
 */
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "./fieldselectiondlg.h"
#include "./fieldselectionpanel.h"
#include "./wxutils.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

wxString defValidationTitle();

wxString ValidationMessage(const wxString& msg, const wxString& operation) {
  return msg == wxEmptyString? (_T("You must select some fields to ") + operation) : msg;
}

wxString FieldSelTitle(const wxString& title, const wxString& operation) {
  return title == wxEmptyString? (operation + _T(" items")) : title;
}

wxString FieldSelText(const wxString& txt, const wxString& operation) {
  return txt == wxEmptyString? (_T("Select fields to ") + operation) : txt;
}

DECLARE_EVENT_TYPE(EVT_RELAYOUT_DLG, -1)
DEFINE_EVENT_TYPE(EVT_RELAYOUT_DLG)

BEGIN_EVENT_TABLE( FieldSelectionDlg, wxDialog )
  EVT_INIT_DIALOG(FieldSelectionDlg::OnInitDialog)
  EVT_COMMAND(wxID_ANY, EVT_RELAYOUT_DLG, FieldSelectionDlg::OnRelayoutDlg)
END_EVENT_TABLE()

FieldSelectionDlg::FieldSelectionDlg(wxWindow* parent,
                                     const CItemData::FieldType* available, size_t navail,
                                     const CItemData::FieldType* mandatory, size_t nmandatory,
                                     FieldSet& userSelection,
                                     const wxString& operation,
                                     const wxString& dlgtitle/* = wxEmptyString*/,
                                     const wxString& dlgText/* = wxEmptyString*/,
                                     const wxString& vMsg/* = wxEmptyString*/, 
                                     const wxString& vTitle/* = wxEmptyString*/)
                                      :wxDialog(parent, wxID_ANY, FieldSelTitle(dlgtitle, operation),
                                                wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  
  dlgSizer->AddSpacer(TopMargin);
  dlgSizer->Add(new wxStaticText(this, wxID_ANY, FieldSelText(dlgText, operation)), wxSizerFlags(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin));
  dlgSizer->AddSpacer(RowSeparation);
  FieldSelectionPanel* fsp = new FieldSelectionPanel(this);
  FieldSelectionPanelValidator fspValidator(available, navail,
                                            mandatory, nmandatory,
                                            userSelection,
                                            ValidationMessage(vMsg, operation)
                                            ,
                                            FieldSelTitle(vTitle, operation));
  fsp->SetValidator(fspValidator);
  
  dlgSizer->Add(fsp, wxSizerFlags(1).Expand().Border(wxLEFT|wxRIGHT, SideMargin));
  dlgSizer->AddSpacer(TopMargin);
  wxSizer* btnSizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
  if (btnSizer)
    dlgSizer->Add(btnSizer, wxSizerFlags(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin));
  dlgSizer->AddSpacer(BottomMargin);
  SetSizer(dlgSizer);
}

void FieldSelectionDlg::OnInitDialog(wxInitDialogEvent& evt)
{
  evt.Skip();
  wxCommandEvent layoutEvent(EVT_RELAYOUT_DLG);
  GetEventHandler()->AddPendingEvent(layoutEvent);
}

void InvalidateBestSizeRecursively(wxWindow* win)
{
  win->InvalidateBestSize();
  wxWindowList& children = win->GetChildren();
  for (wxWindowList::iterator itr = children.begin(); itr != children.end(); ++itr)
    InvalidateBestSizeRecursively(*itr);
  
}

void FieldSelectionDlg::OnRelayoutDlg(wxCommandEvent& /*evt*/)  
{
  InvalidateBestSizeRecursively(this);
  GetSizer()->RecalcSizes();
  Fit();
}
