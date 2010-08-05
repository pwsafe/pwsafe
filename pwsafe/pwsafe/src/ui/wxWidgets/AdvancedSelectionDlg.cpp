/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file about.cpp
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

#include <wx/valgen.h>
#include <wx/statline.h>

#include "AdvancedSelectionDlg.h"


struct _subgroups subgroups[] = { {wxT("Group"),       CItemData::GROUP},
                                  {wxT("Group/Title"), CItemData::GROUPTITLE},
                                  {wxT("Notes"),       CItemData::NOTES},
                                  {wxT("Title"),       CItemData::TITLE},
                                  {wxT("URL"),         CItemData::URL},
                                  {wxT("User Name"),   CItemData::USER} } ;

struct _subgroupFunctions subgroupFunctions[] = { {wxT("equals"),              PWSMatch::MR_EQUALS},
                                                  {wxT("does not equal"),      PWSMatch::MR_NOTEQUAL},
                                                  {wxT("begins with"),         PWSMatch::MR_BEGINS},
                                                  {wxT("does not begin with"), PWSMatch::MR_NOTBEGIN},
                                                  {wxT("ends with"),           PWSMatch::MR_ENDS},
                                                  {wxT("does not end with"),   PWSMatch::MR_NOTEND},
                                                  {wxT("contains"),            PWSMatch::MR_CONTAINS},
                                                  {wxT("does not contain"),    PWSMatch::MR_NOTCONTAIN} } ;

struct _fieldNames {
    const charT* name;
    CItemData::FieldType type;
} fieldNames[] = {  {wxT("Group"),              CItemData::GROUP},
                    {wxT("Title"),              CItemData::TITLE},
                    {wxT("User Name"),          CItemData::USER},
                    {wxT("Notes"),              CItemData::NOTES},
                    {wxT("Password"),           CItemData::PASSWORD},
                    {wxT("URL"),                CItemData::URL},
                    {wxT("Autotype"),           CItemData::AUTOTYPE},
                    {wxT("Password History"),   CItemData::PWHIST},
                    {wxT("Run Command"),        CItemData::RUNCMD},
                    {wxT("Email"),              CItemData::EMAIL}
                 };




////////////////////////////////////////////////////////////////////////////
// AdvancedSelectionDlgBase implementation

IMPLEMENT_CLASS( AdvancedSelectionDlgBase, wxDialog )

enum {ID_SELECT_SOME = 101, ID_SELECT_ALL, ID_REMOVE_SOME, ID_REMOVE_ALL, ID_LB_AVAILABLE_FIELDS, ID_LB_SELECTED_FIELDS };

BEGIN_EVENT_TABLE( AdvancedSelectionDlgBase, wxDialog )
  EVT_BUTTON( wxID_OK, AdvancedSelectionDlgBase::OnOk )
  EVT_BUTTON( ID_SELECT_SOME, AdvancedSelectionDlgBase::OnSelectSome )
  EVT_BUTTON( ID_SELECT_ALL, AdvancedSelectionDlgBase::OnSelectAll )
  EVT_BUTTON( ID_REMOVE_SOME, AdvancedSelectionDlgBase::OnRemoveSome )
  EVT_BUTTON( ID_REMOVE_ALL, AdvancedSelectionDlgBase::OnRemoveAll )
END_EVENT_TABLE()


AdvancedSelectionDlgBase::AdvancedSelectionDlgBase(wxWindow* parentWnd, const SelectionCriteria& existingCriteria):
                                                  m_criteria(existingCriteria)
{
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
}

void AdvancedSelectionDlgBase::CreateControls(wxWindow* parentWnd)
{
  enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 10, ColSeparation = 20};

  wxDialog::Create(parentWnd, wxID_ANY, GetAdvancedSelectionTitle(),
                    wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
  
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);
  
  //Subset entries
  {
    wxStaticBoxSizer* sizer = new wxStaticBoxSizer(wxVERTICAL, this);

    wxCheckBox* check = new wxCheckBox(this, wxID_ANY, wxT("&Restrict to a subset of entries:"));
    check->SetValidator(wxGenericValidator(&m_criteria.m_fUseSubgroups));
    sizer->Add(check, wxSizerFlags().Border());

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, wxT("&Where")), wxSizerFlags(0));
    hbox->AddSpacer(ColSeparation);
    
    wxComboBox* comboSubgroup = new wxComboBox(this, wxID_ANY);
    for (size_t idx = 0 ; idx < NumberOf(subgroups); ++idx) comboSubgroup->AppendString(subgroups[idx].name);
    comboSubgroup->SetValidator(wxGenericValidator(&m_criteria.m_subgroupObject));
    hbox->Add(comboSubgroup, wxSizerFlags(1).Expand());
    
    hbox->AddSpacer(ColSeparation);
    
    wxComboBox* comboFunctions = new wxComboBox(this, wxID_ANY);
    for( size_t idx = 0; idx < NumberOf(subgroupFunctions); ++idx) comboFunctions->AppendString(subgroupFunctions[idx].name);
    comboFunctions->SetValidator(wxGenericValidator(&m_criteria.m_subgroupFunction));
    hbox->Add(comboFunctions, wxSizerFlags(1).Expand());
    
    sizer->Add(hbox, wxSizerFlags().Border().Expand());

    sizer->Add( new wxStaticText(this, wxID_ANY, wxT("the &following text:")), wxSizerFlags().Border());

    wxTextCtrl* txtCtrl = new wxTextCtrl(this, wxID_ANY, _("*"), wxDefaultPosition, wxSize(200, -1));
    txtCtrl->SetValidator(wxGenericValidator(&m_criteria.m_subgroupText));
    sizer->Add(txtCtrl, wxSizerFlags().Border().Expand().FixedMinSize());

    wxCheckBox* checkCaseSensitivity = new wxCheckBox(this, wxID_ANY, wxT("&Case Sensitive"));
    checkCaseSensitivity->SetValidator(wxGenericValidator(&m_criteria.m_fCaseSensitive));
    sizer->Add( checkCaseSensitivity, wxSizerFlags().Border() );
    
    dlgSizer->Add(sizer, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());
  }

  dlgSizer->AddSpacer(RowSeparation);
  
  {
    wxFlexGridSizer* grid = new wxFlexGridSizer(3, RowSeparation, ColSeparation);
    
    //first and third columns are growable
    grid->AddGrowableCol(0, 1);  
    grid->AddGrowableCol(2, 1);
    grid->AddGrowableRow(1, 1);
    grid->SetFlexibleDirection(wxBOTH);
    
    //first row is labels, with a spacer in between
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("&Available Fields:")));
    grid->AddSpacer(0);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("&Selected Fields:")));
    
    //second row is the listboxes, with buttons in between
    wxListBox* lbFields = new wxListBox(this, ID_LB_AVAILABLE_FIELDS, wxDefaultPosition, 
              wxDefaultSize, 0, NULL, wxLB_EXTENDED);
    for (size_t idx = 0; idx < NumberOf(fieldNames); ++idx)
        if (!m_criteria.m_bsFields.test(fieldNames[idx].type))
            lbFields->Append(fieldNames[idx].name, (void*)(idx));

    grid->Add(lbFields, wxSizerFlags().Expand());
    
    wxBoxSizer* buttonBox = new wxBoxSizer(wxVERTICAL);
    buttonBox->AddStretchSpacer();
    buttonBox->Add( new wxButton(this, ID_SELECT_SOME, wxT(">")) );
    buttonBox->AddSpacer(RowSeparation);
    buttonBox->Add( new wxButton(this, ID_SELECT_ALL, wxT(">>")) );
    buttonBox->AddSpacer(RowSeparation*2);
    buttonBox->Add( new wxButton(this, ID_REMOVE_SOME, wxT("<")) );
    buttonBox->AddSpacer(RowSeparation);
    buttonBox->Add( new wxButton(this, ID_REMOVE_ALL, wxT("<<")) );
    buttonBox->AddStretchSpacer();
    
    grid->Add(buttonBox, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));


    wxListBox* lbSelectedFields = new wxListBox(this, ID_LB_SELECTED_FIELDS, wxDefaultPosition, 
                  wxDefaultSize, 0, NULL, wxLB_EXTENDED);
    for (size_t idx=0; idx < NumberOf(fieldNames); ++idx)
        if (m_criteria.m_bsFields.test(fieldNames[idx].type)) {
          if (IsMandatoryField(fieldNames[idx].type))
            lbSelectedFields->Append(wxString(fieldNames[idx].name) + _(" [Mandatory Field]"), (void*)(idx));
          else
            lbSelectedFields->Append(fieldNames[idx].name, (void*)(idx));
        }

    
    grid->Add(lbSelectedFields, wxSizerFlags().Expand());

    dlgSizer->Add(grid, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, SideMargin));
  }
  
  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Center());
  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(CreateStdDialogButtonSizer(wxOK|wxCANCEL|wxHELP), wxSizerFlags().Center());
  dlgSizer->AddSpacer(BottomMargin);
  
  SetSizerAndFit(dlgSizer);
}

void AdvancedSelectionDlgBase::OnOk( wxCommandEvent& evt )
{
  TransferDataFromWindow();

  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  wxASSERT(lbSelected);

  //reset the selected field bits 
  m_criteria.m_bsFields.reset();
  const size_t count = lbSelected->GetCount();
  
  for (size_t idx = 0; idx < count; ++idx) {
      const size_t which = (size_t)lbSelected->GetClientData((unsigned int)idx);
      m_criteria.m_bsFields.set(fieldNames[which].type, true);
  }

  //Let wxDialog handle it as well, to close the window
  evt.Skip(true);
}

void AdvancedSelectionDlgBase::OnSelectSome( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  wxArrayInt aSelected;
  if (lbAvailable->GetSelections(aSelected)) {
    for (size_t idx = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = (size_t)lbAvailable->GetClientData((unsigned int)(aSelected[idx] - idx));
      wxASSERT(which < NumberOf(fieldNames));
      lbAvailable->Delete((unsigned int)(aSelected[idx] - idx));
      lbSelected->Append(fieldNames[which].name, (void *)which);
    }
  }
}

void AdvancedSelectionDlgBase::OnSelectAll( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  while (lbAvailable->GetCount()) {
      size_t which = (size_t)lbAvailable->GetClientData(0);
      lbAvailable->Delete(0);
      lbSelected->Append(fieldNames[which].name, (void*)which);
  }
}

void AdvancedSelectionDlgBase::OnRemoveSome( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  wxArrayInt aSelected;
  if (lbSelected->GetSelections(aSelected)) {
    for (size_t idx = 0, nRemoved = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = (size_t)lbSelected->GetClientData((unsigned int)(aSelected[idx] - nRemoved));
      wxASSERT(which < NumberOf(fieldNames));
      if (!IsMandatoryField(fieldNames[which].type)) {
        lbSelected->Delete((unsigned int)(aSelected[idx] - nRemoved++));
        lbAvailable->Append(fieldNames[which].name, (void *)which);
      }
    }
  }
}

void AdvancedSelectionDlgBase::OnRemoveAll( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  for(size_t itemsLeft = lbSelected->GetCount(), idx = 0; idx < itemsLeft; ) {
      size_t which = (size_t)lbSelected->GetClientData(idx);
      if (!IsMandatoryField(fieldNames[which].type)) {
        lbSelected->Delete(idx);
        lbAvailable->Append(fieldNames[which].name, (void*)which);
        --itemsLeft;
      }
      else
        ++idx;
  }
}

