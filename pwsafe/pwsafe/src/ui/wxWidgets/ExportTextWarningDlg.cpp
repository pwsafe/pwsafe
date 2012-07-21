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

#include <wx/statline.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

enum { ID_COMBINATION = 100, ID_VKBD, ID_LINE_DELIMITER, ID_ADVANCED };

IMPLEMENT_CLASS( CExportTextWarningDlgBase, wxDialog )

BEGIN_EVENT_TABLE( CExportTextWarningDlgBase, wxDialog )
  EVT_BUTTON( ID_ADVANCED, CExportTextWarningDlgBase::OnAdvancedSelection )
END_EVENT_TABLE()


CExportTextWarningDlgBase::CExportTextWarningDlgBase(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxEmptyString,
                                                                      wxDefaultPosition, wxDefaultSize, 
                                                                      wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
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
  pwdCtrl->Add(new CSafeCombinationCtrl(this, wxID_ANY, &passKey), wxSizerFlags().Expand().Proportion(1));
  dlgSizer->Add(pwdCtrl, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());
  dlgSizer->AddSpacer(RowSeparation);

  delimiter = wxT('\xbb');
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
}

void CExportTextWarningDlgBase::OnAdvancedSelection( wxCommandEvent& evt )
{
  UNREFERENCED_PARAMETER(evt);
  DoAdvancedSelection();
}
