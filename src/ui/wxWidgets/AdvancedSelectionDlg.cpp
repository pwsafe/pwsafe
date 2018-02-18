/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "SelectionCriteria.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

void EnableSizerElements(wxSizer* sizer, wxWindow* ignore, bool enable);

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
                                               SelectionCriteria* existingCriteria,
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

    wxCheckBox* check = new wxCheckBox(this, wxID_ANY, _("&Restrict to a subset of entries:"));
    check->SetValidator(wxGenericValidator(&m_criteria->m_fUseSubgroups));
    sizer->Add(check, wxSizerFlags().Border());
    check->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                    wxCommandEventHandler(AdvancedSelectionPanel::OnRestrictSearchItems));

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(this, wxID_ANY, _("&Where")), wxSizerFlags(0));
    hbox->AddSpacer(ColSeparation);

    wxComboBox* comboSubgroup = new wxComboBox(this, wxID_ANY);
    for (size_t idx = 0 ; idx < SelectionCriteria::GetNumSubgroups(); ++idx) comboSubgroup->AppendString(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSubgroup(idx)));
    comboSubgroup->SetValidator(wxGenericValidator(&m_criteria->m_subgroupObject));
    hbox->Add(comboSubgroup, wxSizerFlags(1).Expand());

    hbox->AddSpacer(ColSeparation);

    wxComboBox* comboFunctions = new wxComboBox(this, wxID_ANY);
    for( size_t idx = 0; idx < SelectionCriteria::GetNumSubgroupFunctions(); ++idx) comboFunctions->AppendString(SelectionCriteria::GetSubgroupFunctionName(idx));
    comboFunctions->SetValidator(wxGenericValidator(&m_criteria->m_subgroupFunction));
    hbox->Add(comboFunctions, wxSizerFlags(1).Expand());

    sizer->Add(hbox, wxSizerFlags().Border().Expand());

    sizer->Add( new wxStaticText(this, wxID_ANY, _("the &following text:")), wxSizerFlags().Border());

    wxTextCtrl* txtCtrl = new wxTextCtrl(this, wxID_ANY, wxT("*"), wxDefaultPosition, wxSize(200, -1));
    txtCtrl->SetValidator(wxGenericValidator(&m_criteria->m_subgroupText));
    sizer->Add(txtCtrl, wxSizerFlags().Border().Expand().FixedMinSize());

    wxCheckBox* checkCaseSensitivity = new wxCheckBox(this, wxID_ANY, _("&Case Sensitive"));
    checkCaseSensitivity->SetValidator(wxGenericValidator(&m_criteria->m_fCaseSensitive));
    sizer->Add( checkCaseSensitivity, wxSizerFlags().Border() );

    dlgSizer->Add(sizer, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());

    EnableSizerElements(sizer, check, m_criteria->HasSubgroupRestriction());
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
      grid->Add(new wxStaticText(this, wxID_ANY, _("&Available Fields:")));
      grid->AddSpacer(0);
      grid->Add(new wxStaticText(this, wxID_ANY, _("&Selected Fields:")));

      //second row is the listboxes, with buttons in between

      wxListBox* lbAvailable = new wxListBox(this, ID_LB_AVAILABLE_FIELDS, wxDefaultPosition,
                                             wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
      grid->Add(lbAvailable, wxSizerFlags().Expand());

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

      wxListBox* lbSelected = new wxListBox(this, ID_LB_SELECTED_FIELDS, wxDefaultPosition,
                                            wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
      grid->Add(lbSelected, wxSizerFlags().Expand());

      dlgSizer->Add(grid, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, SideMargin));

      //add all the field names to both listboxes to size the dialog/wizard page correctly
      //These are anyway removed in TransferDataToWindow below before doing anything else
      for (size_t idx=0; idx < SelectionCriteria::GetNumFieldsSelectable(); ++idx) {
        lbAvailable->Append(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSelectableField(idx)), reinterpret_cast<void *>(idx));
        lbSelected->Append(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSelectableField(idx)), reinterpret_cast<void *>(idx));
      }
    }
  }

  SetSizerAndFit(dlgSizer);
}

bool AdvancedSelectionPanel::TransferDataToWindow()
{
  if (wxPanel::TransferDataToWindow()) {
    if (ShowFieldSelection()) {
      // Temporary hack until I can write a proper validator for SelectionCriteria class
      // which would set its dirty flag automatically
      const bool criteriaChanged = (*m_criteria != SelectionCriteria());
      wxListBox* lbAvailable  = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
      wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
      lbAvailable->Clear();
      lbSelected->Clear();

      for (size_t idx = 0; idx < SelectionCriteria::GetNumFieldsSelectable(); ++idx) {
        const CItemData::FieldType ft = SelectionCriteria::GetSelectableField(idx);
        if (IsUsableField(ft)) {
          if ( (criteriaChanged && m_criteria->IsFieldSelected(ft)) ||
                              (!criteriaChanged && IsPreselectedField(ft)) ) {
            const wxString title = SelectionCriteria::GetSelectableFieldName(ft) + (IsMandatoryField(ft)? _(" [Mandatory Field]"): wxS(""));
            lbSelected->Append(title, reinterpret_cast<void *>(idx));
          }
          else {
            lbAvailable->Append(SelectionCriteria::GetSelectableFieldName(ft), reinterpret_cast<void *>(idx));
          }
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
                        _("No fields selected"), wxOK|wxICON_INFORMATION, this);
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
      m_criteria->m_bsFields.reset();
      const size_t count = lbSelected->GetCount();

      for (size_t idx = 0; idx < count; ++idx) {
          const size_t which = reinterpret_cast<size_t>(lbSelected->GetClientData(static_cast<unsigned int>(idx)));
          m_criteria->SelectField(SelectionCriteria::GetSelectableField(which));
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
    aSelected.Sort(pless);
    for (size_t idx = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = reinterpret_cast<size_t>(lbAvailable->GetClientData(static_cast<unsigned int>(aSelected[idx] - idx)));
      wxASSERT(which < SelectionCriteria::GetNumFieldsSelectable());
      lbAvailable->Delete(static_cast<unsigned int>(aSelected[idx] - idx));
      lbSelected->Append(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSelectableField(which)), reinterpret_cast<void *>(which));
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
      lbSelected->Append(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSelectableField(which)), reinterpret_cast<void *>(which));
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
    aSelected.Sort(pless);
    for (size_t idx = 0, nRemoved = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = reinterpret_cast<size_t>(lbSelected->GetClientData(static_cast<unsigned int>(aSelected[idx] - nRemoved)));
      wxASSERT(which < SelectionCriteria::GetNumFieldsSelectable());
      if (!IsMandatoryField(SelectionCriteria::GetSelectableField(which))) {
        lbSelected->Delete(static_cast<unsigned int>(aSelected[idx] - nRemoved++));
        lbAvailable->Append(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSelectableField(which)), reinterpret_cast<void *>(which));
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
      if (!IsMandatoryField(SelectionCriteria::GetSelectableField(which))) {
        lbSelected->Delete(reinterpret_cast<unsigned int &>(idx));
        lbAvailable->Append(SelectionCriteria::GetSelectableFieldName(SelectionCriteria::GetSelectableField(which)), reinterpret_cast<void *>(which));
        --itemsLeft;
      }
      else
        ++idx;
  }
}

/*
 * Recursively enables/disables all sizer elements.  The <ignore> window
 * is not disabled
 */
void EnableSizerElements(wxSizer* sizer, wxWindow* ignore, bool enable)
{
  wxCHECK_RET(sizer, wxT("Null sizer passed to EnableSizerElements"));

  wxSizerItemList& items = sizer->GetChildren();
  for (wxSizerItemList::iterator itr = items.begin(); itr != items.end(); ++itr) {
    wxSizerItem* item = *itr;
    if (item->IsWindow() && item->GetWindow() != ignore)
      item->GetWindow()->Enable(enable);
    else if (item->IsSizer())
      EnableSizerElements(item->GetSizer(), ignore, enable);
  }
}

void AdvancedSelectionPanel::OnRestrictSearchItems(wxCommandEvent& evt)
{
  wxWindow* checkbox = wxDynamicCast(evt.GetEventObject(), wxWindow);
  wxCHECK_RET(checkbox, wxT("Could not get checkbox from check event object"));
  wxSizer* sizer = checkbox->GetContainingSizer();
  wxCHECK_RET(sizer, wxT("Could not get the sizer owning the checkbox"));
  EnableSizerElements(sizer, checkbox, evt.IsChecked());
}
