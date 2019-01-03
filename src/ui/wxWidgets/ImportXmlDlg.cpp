/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ImportXmlDlg.h"
#include "./OpenFilePickerValidator.h"

#include <wx/valgen.h>
#include <wx/statline.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_CLASS( CImportXMLDlg, wxDialog )

CImportXMLDlg::CImportXMLDlg(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxString(_("Import XML Settings"))),
                                                  importUnderGroup(false), 
                                                  importPasswordsOnly(false)
{
  enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 10, ColSeparation = 20};
  
  wxSizerFlags borderFlags = wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand();
  wxSizerFlags separatorFlags = wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin/2).Expand();
  
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);

  dlgSizer->Add(new wxStaticText(this, wxID_ANY, _("XML file to import:")), borderFlags);
  dlgSizer->AddSpacer(RowSeparation/2);
  COpenFilePickerValidator validator(filepath);
  dlgSizer->Add(new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, 
                                          _("Please Choose a XML File to Import"), 
                                          _("XML files (*.xml)|*.xml"), 
                                          wxDefaultPosition, wxDefaultSize, 
                                          wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL, 
                                          validator), borderFlags);
  dlgSizer->AddSpacer(RowSeparation);

  dlgSizer->Add(new wxStaticLine(this), separatorFlags);
  dlgSizer->AddSpacer(RowSeparation);
  
  wxBoxSizer* horzSizer = new wxBoxSizer(wxHORIZONTAL);
  horzSizer->Add(CheckBox(_("Import under Group"), &importUnderGroup), wxSizerFlags().Proportion(0));
  horzSizer->AddSpacer(ColSeparation);
  horzSizer->Add(TextCtrl(&groupName), wxSizerFlags().Proportion(1));
  dlgSizer->Add(horzSizer, borderFlags);
  dlgSizer->AddSpacer(RowSeparation);
  
  dlgSizer->Add(CheckBox(_("Import to change passwords of existing entries ONLY"), &importPasswordsOnly),
                    borderFlags);
  dlgSizer->AddSpacer(RowSeparation);
  
  dlgSizer->Add(new wxStaticLine(this), separatorFlags);
  dlgSizer->AddSpacer(RowSeparation);
  
  dlgSizer->Add(CreateStdDialogButtonSizer(wxOK|wxCANCEL|wxHELP), borderFlags);
  dlgSizer->AddSpacer(BottomMargin);
  
  SetSizerAndFit(dlgSizer);
}

wxCheckBox* CImportXMLDlg::CheckBox(const wxString& label, bool* validatorTarget)
{
  return new wxCheckBox(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0,
                          wxGenericValidator(validatorTarget));
}

wxTextCtrl* CImportXMLDlg::TextCtrl(wxString* validatorTarget)
{
  return new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 
                                  wxTextValidator(wxFILTER_NONE, validatorTarget));
}
