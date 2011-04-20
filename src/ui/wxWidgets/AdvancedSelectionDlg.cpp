/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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


CItemData::FieldType subgroups[] = {  CItemData::GROUP,
                                      CItemData::GROUPTITLE,
                                      CItemData::NOTES,
                                      CItemData::TITLE,
                                      CItemData::URL,
                                      CItemData::USER
                                  } ;

struct _subgroupFunctions subgroupFunctions[] = { {wxT("equals"),              PWSMatch::MR_EQUALS},
                                                  {wxT("does not equal"),      PWSMatch::MR_NOTEQUAL},
                                                  {wxT("begins with"),         PWSMatch::MR_BEGINS},
                                                  {wxT("does not begin with"), PWSMatch::MR_NOTBEGIN},
                                                  {wxT("ends with"),           PWSMatch::MR_ENDS},
                                                  {wxT("does not end with"),   PWSMatch::MR_NOTEND},
                                                  {wxT("contains"),            PWSMatch::MR_CONTAINS},
                                                  {wxT("does not contain"),    PWSMatch::MR_NOTCONTAIN} } ;

CItemData::FieldType searchFields[] = { CItemData::GROUP,
                                        CItemData::TITLE,
                                        CItemData::USER,
                                        CItemData::NOTES,
                                        CItemData::PASSWORD,
                                        CItemData::URL,
                                        CItemData::AUTOTYPE,
                                        CItemData::PWHIST,
                                        CItemData::RUNCMD,
                                        CItemData::EMAIL
                                      };



wxString GetSearchFieldName(CItemData::FieldType ft) {
  return towxstring(CItemData::FieldName(ft));
}

wxString SelectionCriteria::GetGroupSelectionDescription() const
{
  if (!m_fUseSubgroups)
    return _("All entries");
  else
    return wxString(_("Entries whose ")) << GetSearchFieldName(subgroups[m_subgroupObject]) << wxT(' ')
            << subgroupFunctions[m_subgroupFunction].name << wxT(" \"") << m_subgroupText
            << wxT("\" [") << (m_fCaseSensitive? wxT("") : _("not ")) << _("case-sensitive]");
}

//returns true if all fields have been selected
bool SelectionCriteria::GetFieldSelection(wxArrayString& selectedFields, wxArrayString& unselectedFields)
{
  for (size_t idx = 0; idx < NumberOf(searchFields); ++idx) {
    if (m_bsFields.test(searchFields[idx]))
      selectedFields.Add(GetSearchFieldName(searchFields[idx]));
    else
      unselectedFields.Add(GetSearchFieldName(searchFields[idx]));
  }
  return m_bsFields.count() == NumberOf(searchFields);
}

////////////////////////////////////////////////////////////////////////////
// AdvancedSelectionPanel implementation

IMPLEMENT_CLASS( AdvancedSelectionPanel, wxPanel )

enum {ID_SELECT_SOME = 101, ID_SELECT_ALL, ID_REMOVE_SOME, ID_REMOVE_ALL, ID_LB_AVAILABLE_FIELDS, ID_LB_SELECTED_FIELDS };

BEGIN_EVENT_TABLE( AdvancedSelectionPanel, wxPanel )
  EVT_BUTTON( ID_SELECT_SOME, AdvancedSelectionPanel::OnSelectSome )
  EVT_BUTTON( ID_SELECT_ALL, AdvancedSelectionPanel::OnSelectAll )
  EVT_BUTTON( ID_REMOVE_SOME, AdvancedSelectionPanel::OnRemoveSome )
  EVT_BUTTON( ID_REMOVE_ALL, AdvancedSelectionPanel::OnRemoveAll )
END_EVENT_TABLE()


AdvancedSelectionPanel::AdvancedSelectionPanel(wxWindow* parentWnd, 
                                               const SelectionCriteria& existingCriteria,
                                               bool autoValidate):
                                                  m_criteria(existingCriteria),
                                                  m_autoValidate(autoValidate)
{
  UNREFERENCED_PARAMETER(parentWnd);
  parentWnd->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
}

void AdvancedSelectionPanel::CreateControls(wxWindow* parentWnd)
{
  wxPanel::Create(parentWnd);
  
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  
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
    for (size_t idx = 0 ; idx < NumberOf(subgroups); ++idx) comboSubgroup->AppendString(GetSearchFieldName(subgroups[idx]));
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

  if (ShowFieldSelection()) {
    
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

      grid->Add(new wxListBox(this, ID_LB_AVAILABLE_FIELDS, wxDefaultPosition, 
                wxDefaultSize, 0, NULL, wxLB_EXTENDED), wxSizerFlags().Expand());
      
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

      grid->Add(new wxListBox(this, ID_LB_SELECTED_FIELDS, wxDefaultPosition, 
                    wxDefaultSize, 0, NULL, wxLB_EXTENDED), wxSizerFlags().Expand());

      dlgSizer->Add(grid, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, SideMargin));
    }
  }
  
  SetSizerAndFit(dlgSizer);
}

bool AdvancedSelectionPanel::TransferDataToWindow()
{
  if (wxPanel::TransferDataToWindow()) {
    if (ShowFieldSelection()) {

      wxListBox* lbAvailable  = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
      lbAvailable->Clear();

      for (size_t idx = 0; idx < NumberOf(searchFields); ++idx) {
        if (!m_criteria.m_bsFields.test(searchFields[idx]))
            lbAvailable->Append(GetSearchFieldName(searchFields[idx]), reinterpret_cast<void *>(idx));
      }

      wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
      lbSelected->Clear();

      for (size_t idx=0; idx < NumberOf(searchFields); ++idx) {
        if (m_criteria.m_bsFields.test(searchFields[idx])) {
          if (IsMandatoryField(searchFields[idx]))
            lbSelected->Append(GetSearchFieldName(searchFields[idx]) + _(" [Mandatory Field]"),
                                     reinterpret_cast<void *>(idx));
          else
            lbSelected->Append(GetSearchFieldName(searchFields[idx]), reinterpret_cast<void *>(idx));
        }
      }
    }
    return true;
  }
  return false;
}

bool AdvancedSelectionPanel::DoValidation()
{
  if (ShowFieldSelection()) {
    wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
    wxASSERT(lbSelected);
    if (lbSelected->GetCount() == 0) {
      wxMessageBox(wxString(_("You must select some of the fields to ")) << GetTaskWord(), 
                        wxT("No fields selected"), wxOK|wxICON_INFORMATION, this);
      return false;
    }
  }
  return true;
}

bool AdvancedSelectionPanel::Validate()
{
  return !m_autoValidate || DoValidation();
}

bool AdvancedSelectionPanel::TransferDataFromWindow()
{
  if ( wxPanel::TransferDataFromWindow()) {
    if (ShowFieldSelection()) {
      wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
      wxASSERT(lbSelected);

      //reset the selected field bits 
      m_criteria.m_bsFields.reset();
      const size_t count = lbSelected->GetCount();
      
      for (size_t idx = 0; idx < count; ++idx) {
          const size_t which = reinterpret_cast<size_t>(lbSelected->GetClientData(static_cast<unsigned int>(idx)));
          m_criteria.m_bsFields.set(searchFields[which], true);
      }
    }
    return true;
  }
  return false;
}

void AdvancedSelectionPanel::OnSelectSome( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  wxArrayInt aSelected;
  if (lbAvailable->GetSelections(aSelected)) {
    for (size_t idx = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = reinterpret_cast<size_t>(lbAvailable->GetClientData(static_cast<unsigned int>(aSelected[idx] - idx)));
      wxASSERT(which < NumberOf(searchFields));
      lbAvailable->Delete(static_cast<unsigned int>(aSelected[idx] - idx));
      lbSelected->Append(GetSearchFieldName(searchFields[which]), reinterpret_cast<void *>(which));
    }
  }
}

void AdvancedSelectionPanel::OnSelectAll( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  while (lbAvailable->GetCount()) {
      size_t which = reinterpret_cast<size_t>(lbAvailable->GetClientData(0));
      lbAvailable->Delete(0);
      lbSelected->Append(GetSearchFieldName(searchFields[which]), reinterpret_cast<void *>(which));
  }
}

void AdvancedSelectionPanel::OnRemoveSome( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  wxArrayInt aSelected;
  if (lbSelected->GetSelections(aSelected)) {
    for (size_t idx = 0, nRemoved = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = reinterpret_cast<size_t>(lbSelected->GetClientData(static_cast<unsigned int>(aSelected[idx] - nRemoved)));
      wxASSERT(which < NumberOf(searchFields));
      if (!IsMandatoryField(searchFields[which])) {
        lbSelected->Delete(static_cast<unsigned int>(aSelected[idx] - nRemoved++));
        lbAvailable->Append(GetSearchFieldName(searchFields[which]), reinterpret_cast<void *>(which));
      }
    }
  }
}

void AdvancedSelectionPanel::OnRemoveAll( wxCommandEvent& /* evt */ )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  for(size_t itemsLeft = lbSelected->GetCount(), idx = 0; idx < itemsLeft; ) {
      size_t which = reinterpret_cast<size_t>(lbSelected->GetClientData(reinterpret_cast<unsigned int &>(idx)));
      if (!IsMandatoryField(searchFields[which])) {
        lbSelected->Delete(reinterpret_cast<unsigned int &>(idx));
        lbAvailable->Append(GetSearchFieldName(searchFields[which]), reinterpret_cast<void *>(which));
        --itemsLeft;
      }
      else
        ++idx;
  }
}

