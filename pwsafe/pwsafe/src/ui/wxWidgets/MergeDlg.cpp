/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "./OpenFilePickerValidator.h"
#include "./SafeCombinationCtrl.h"
#include "../../core/PWScore.h"
#include "./wxutils.h"
#include "./AdvancedSelectionDlg.h"
#include "../../os/file.h"

#include <wx/statline.h>

enum {ID_ADVANCED = 5126, ID_COMBINATION = 6982};

IMPLEMENT_CLASS( MergeDlg, wxDialog )

BEGIN_EVENT_TABLE( MergeDlg, wxDialog )
  EVT_BUTTON( ID_ADVANCED,  MergeDlg::OnAdvancedSelection )
  EVT_BUTTON( wxID_OK,      MergeDlg::OnOk )
END_EVENT_TABLE()


MergeDlg::MergeDlg(wxWindow* parent, PWScore* core) : 
                      wxDialog(parent, wxID_ANY, wxString(_("Merge Another Database"))),
                      m_core(core), m_selection(new SelectionCriteria)
{
  enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 10, ColSeparation = 20};

  wxSizerFlags borderFlags = wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand();
  wxSizerFlags separatorFlags = wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin/2).Expand();

  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);

  const wxString dlgTitle(wxString(_("Choose Database to Merge into \"")) << 
                                          towxstring(m_core->GetCurFile()) << _("\""));
  dlgSizer->Add(new wxStaticText(this, wxID_ANY, dlgTitle), borderFlags);
  dlgSizer->AddSpacer(RowSeparation/2);
  COpenFilePickerValidator validator(m_filepath);
  dlgSizer->Add(new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, 
                                          _("Please Choose a Database to Merge into current database"), 
                                          _("Password Safe Databases (*.psafe3; *.dat)|*.psafe3;*.dat|Password Safe Backups (*.bak)|*.bak|Password Safe Intermediate Backups (*.ibak)|*.ibak|All files (*.*; *)|*.*;*"), 
                                          wxDefaultPosition, wxDefaultSize, 
                                          wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL, 
                                          validator), borderFlags);
  dlgSizer->AddSpacer(RowSeparation);

  wxBoxSizer* horzSizer = new wxBoxSizer(wxHORIZONTAL);

  horzSizer->Add(new wxStaticText(this, wxID_ANY, _("Safe Combination:")));
  horzSizer->AddSpacer(ColSeparation/2);
  CSafeCombinationCtrl* sc = new CSafeCombinationCtrl(this, ID_COMBINATION);
  sc->textCtrl->SetValidator(wxTextValidator(wxFILTER_NONE, &m_combination));
  horzSizer->Add(sc, wxSizerFlags().Expand().Proportion(1));
  dlgSizer->Add(horzSizer, borderFlags);

  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(new wxStaticLine(this), separatorFlags);
  dlgSizer->AddSpacer(RowSeparation);

  wxStdDialogButtonSizer* buttons = CreateStdDialogButtonSizer(wxOK|wxCANCEL|wxHELP);
  //This might not be a very wise thing to do.  We are only supposed to add certain
  //pre-defined button-ids to StdDlgBtnSizer
  buttons->Add(new wxButton(this, ID_ADVANCED, _("Advanced...")), wxSizerFlags().Border(wxLEFT|wxRIGHT));
  dlgSizer->Add(buttons, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Center());

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

void MergeDlg::OnOk( wxCommandEvent& )
{
  //code copied from CSafeCombinationEntry::OnOk
  if (Validate() && TransferDataFromWindow()) {
    if (m_combination.empty()) {
      wxMessageBox(_("The combination cannot be blank."), _("Error"), wxOK | wxICON_EXCLAMATION);
      return;
    }
    if (!pws_os::FileExists(m_filepath.c_str())) {
      wxMessageBox( _("File or path not found."), _("Error"), wxOK | wxICON_EXCLAMATION);
      return;
    }
    if (m_core->CheckPasskey(m_filepath.c_str(),
                            m_combination.c_str()) != PWScore::SUCCESS) {
      wxString errmess(_("Incorrect passkey, not a PasswordSafe database, or a corrupt database. (Backup database has same name as original, ending with '~')"));
      wxMessageBox(errmess, _("Error"), wxOK | wxICON_ERROR);
      wxTextCtrl *txt = (wxTextCtrl *)FindWindow(ID_COMBINATION);
      txt->SetSelection(-1,-1);
      txt->SetFocus();
      return;
    }
    EndModal(wxID_OK);
  }
}

struct AdvancedMergeOptions {
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced Merge Options");
  }
  
  static bool IsMandatoryField(CItemData::FieldType /*field*/) {
    return false;
  }
  
  static bool ShowFieldSelection() {
    return false;
  }
};

IMPLEMENT_CLASS_TEMPLATE( AdvancedSelectionDlg, AdvancedSelectionDlgBase, AdvancedMergeOptions )

void MergeDlg::OnAdvancedSelection(wxCommandEvent& )
{
  AdvancedSelectionDlg<AdvancedMergeOptions> dlg(this, *m_selection);
  if (dlg.ShowModal() == wxID_OK)
    *m_selection = dlg.m_criteria;
}

