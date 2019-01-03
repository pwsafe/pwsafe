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

#include "./MergeDlg.h"
#include "../../core/PWScore.h"
#include "./wxutils.h"
#include "./AdvancedSelectionDlg.h"
#include "../../os/file.h"
#include "./DbSelectionPanel.h"
#include "./SelectionCriteria.h"

#include <wx/statline.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

enum {ID_ADVANCED = 5126, ID_COMBINATION = 6982};

IMPLEMENT_CLASS( MergeDlg, wxDialog )

BEGIN_EVENT_TABLE( MergeDlg, wxDialog )
  EVT_BUTTON( ID_ADVANCED,  MergeDlg::OnAdvancedSelection )
END_EVENT_TABLE()

MergeDlg::MergeDlg(wxWindow* parent, PWScore* core) :
                      wxDialog(parent, wxID_ANY, wxString(_("Merge Another Database"))),
                      m_core(core), m_selection(new SelectionCriteria), m_dbPanel(0)
{
  const wxString filePrompt(wxString(_("Choose Database to Merge into \"")) <<
                                          towxstring(m_core->GetCurFile()) << wxT("\""));
  const wxString filePickerCtrlTitle(_("Please Choose a Database to Merge into current database"));

  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);

  //4th arg = true means the panel validates automatically
  m_dbPanel = new DbSelectionPanel(this, filePrompt, filePickerCtrlTitle, true, core, 2);

  dlgSizer->Add(m_dbPanel, wxSizerFlags().Expand().Proportion(1).Border());

  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());
  dlgSizer->AddSpacer(RowSeparation);

  wxSizer* buttons = CreateStdDialogButtonSizer(wxOK|wxCANCEL|wxHELP);
  //This might not be a very wise thing to do.  We are only supposed to add certain
  //pre-defined button-ids to StdDlgBtnSizer
  buttons->Add(new wxButton(this, ID_ADVANCED, _("Advanced...")), wxSizerFlags().DoubleBorder(wxLEFT|wxRIGHT));
  dlgSizer->Add(buttons, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());

  dlgSizer->AddSpacer(BottomMargin);

  SetSizerAndFit(dlgSizer);
}

MergeDlg::~MergeDlg()
{
  delete m_selection;
}

SelectionCriteria MergeDlg::GetSelectionCriteria() const
{
  return *m_selection;
}

struct AdvancedMergeOptions {
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced Merge Options");
  }

  static bool IsMandatoryField(CItemData::FieldType /*field*/) {
    return false;
  }

  static bool IsPreselectedField(CItemData::FieldType /*field*/) {
    wxFAIL_MSG(wxT("Advanced field pre-selection options are not available for Merge"));
    return true;
  }

  static bool IsUsableField(CItemData::FieldType /*field*/) {
    wxFAIL_MSG(wxT("Advanced field usability options are not available for Merge"));
    return true;
  }

  static bool ShowFieldSelection() {
    return false;
  }

  static wxString GetTaskWord() {
    return _("merge");
  }
};

IMPLEMENT_CLASS_TEMPLATE( AdvancedSelectionDlg, wxDialog, AdvancedMergeOptions )

void MergeDlg::OnAdvancedSelection(wxCommandEvent& )
{
  AdvancedSelectionDlg<AdvancedMergeOptions> dlg(this, m_selection);
  dlg.ShowModal();
}

wxString MergeDlg::GetOtherSafePath() const
{
  return m_dbPanel->m_filepath;
}

StringX MergeDlg::GetOtherSafeCombination() const
{
  return m_dbPanel->m_combination;
}
