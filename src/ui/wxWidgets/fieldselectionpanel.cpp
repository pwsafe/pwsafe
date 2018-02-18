/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#include "./fieldselectionpanel.h"

#include "./wxutils.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

//control ids used in the panel
enum {
      ID_SELECT_SOME = 101,
      ID_SELECT_ALL,
      ID_REMOVE_SOME,
      ID_REMOVE_ALL,
      ID_LB_AVAILABLE_FIELDS,
      ID_LB_SELECTED_FIELDS
};

//we derive it from wxClientData so that we can use
//SetClientData instead of SetClientObject.  The former
//has the advantage that the data is deleted automatically
class FieldData: public wxClientData
{
  FieldData(); //undefined

  bool m_mandatory;
  CItemData::FieldType m_ft;
public:
  FieldData(bool mandatory, CItemData::FieldType ft): m_mandatory(mandatory), m_ft(ft)
  {}
  bool IsMandatory() const { return m_mandatory; }
  operator CItemData::FieldType() const { return m_ft; }
};

BEGIN_EVENT_TABLE( FieldSelectionPanel, wxPanel )
  EVT_BUTTON( ID_SELECT_SOME, FieldSelectionPanel::OnSelectSome )
  EVT_BUTTON( ID_SELECT_ALL, FieldSelectionPanel::OnSelectAll )
  EVT_BUTTON( ID_REMOVE_SOME, FieldSelectionPanel::OnRemoveSome )
  EVT_BUTTON( ID_REMOVE_ALL, FieldSelectionPanel::OnRemoveAll )
END_EVENT_TABLE()

FieldSelectionPanel::FieldSelectionPanel(wxWindow* parent): wxPanel(parent),
                                                            m_lbSelected(0),
                                                            m_lbAvailable(0)
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

  m_lbAvailable = new wxListBox(this, ID_LB_AVAILABLE_FIELDS, wxDefaultPosition,
                                         wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
  grid->Add(m_lbAvailable, wxSizerFlags().Expand());

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

  m_lbSelected = new wxListBox(this, ID_LB_SELECTED_FIELDS, wxDefaultPosition,
                                        wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
  grid->Add(m_lbSelected, wxSizerFlags().Expand());

  SetSizer(grid);
}

FieldSelectionPanel::~FieldSelectionPanel()
{
}

void FieldSelectionPanel::AddField(CItemData::FieldType ft, bool selected, bool mandatory)
{
  wxCHECK_RET(!mandatory || selected, wxT("A mandatory field must also be pre-selected"));

  const wxString fieldName(towxstring(CItemData::FieldName(ft)));

  if (FindField(ft, selected? m_lbSelected: m_lbAvailable) != wxNOT_FOUND) {
    wxLogDebug(wxT("%ls already in %ls list, not adding it again"),
                    fieldName,
                    selected? wxT("selected"): wxT("available"));
    return;
  }

  //if the field is already in another listbox, just move it
  const int index = FindField(ft, selected? m_lbAvailable: m_lbSelected);
  if (index != wxNOT_FOUND) {
    wxLogDebug(wxT("%ls already in %ls list, moving it to %ls"),
                    fieldName,
                    selected? wxT("available"): wxT("selected"),
                    selected? wxT("selected"): wxT("available"));
    MoveItem(index, selected? m_lbAvailable: m_lbSelected, selected? m_lbSelected: m_lbAvailable);
    return;
  }

  //else, add it
  wxString title(fieldName);
  if (mandatory)
    title += _(" [Mandatory Field]");

  FieldData *data = new FieldData(mandatory, ft);

  wxListBox* lb = selected? m_lbSelected: m_lbAvailable;
  lb->Append(title, data);
}

int FieldSelectionPanel::FindField(CItemData::FieldType ft, wxListBox* lb) const
{
  unsigned int count = lb->GetCount();
  for (int idx = 0; (unsigned int)idx < count; ++idx) {
    FieldData* fd = dynamic_cast<FieldData*>(lb->GetClientObject(idx));
    if (*fd == ft)
      return idx;
  }
  return wxNOT_FOUND;
}

size_t FieldSelectionPanel::GetNumSelectedFields() const
{
  return m_lbSelected->GetCount();
}

bool FieldSelectionPanel::ItemIsMandatory(size_t index) const
{
  if (m_lbSelected->GetCount() > index) {
    FieldData* data = dynamic_cast<FieldData*>(m_lbSelected->GetClientObject(index));
    return data && data->IsMandatory();
  }
  return false;
}

CItemData::FieldType FieldSelectionPanel::GetSelectedFieldAt(size_t index) const
{
  FieldData* data = dynamic_cast<FieldData*>(m_lbSelected->GetClientObject(index));
  if (data)
    return *data;
  return CItemData::END;
}

void FieldSelectionPanel::MoveItem(int index, wxListBox* from, wxListBox* to)
{

  FieldData* oldData = dynamic_cast<FieldData*>(from->GetClientObject(index));
  FieldData* newData = oldData? new FieldData(*oldData): 0;
  to->Append(from->GetString(index), newData);
  from->Delete(index);
}

void FieldSelectionPanel::OnSelectSome( wxCommandEvent& /* evt */ )
{
  wxArrayInt aSelected;
  if (m_lbAvailable->GetSelections(aSelected)) {
    aSelected.Sort(pless);
    for (size_t idx = 0; idx < aSelected.GetCount(); ++idx) {
      MoveItem(aSelected[idx] - idx, m_lbAvailable, m_lbSelected);
    }
  }
}

void FieldSelectionPanel::OnSelectAll( wxCommandEvent& /* evt */ )
{
  while (m_lbAvailable->GetCount()) {
    MoveItem(0, m_lbAvailable, m_lbSelected);
  }
}

void FieldSelectionPanel::OnRemoveSome( wxCommandEvent& /* evt */ )
{
  wxArrayInt aSelected;
  if (m_lbSelected->GetSelections(aSelected)) {
    aSelected.Sort(pless);
    for (size_t idx = 0, nRemoved = 0; idx < aSelected.GetCount(); ++idx) {
      const int listIndex = aSelected[idx] - nRemoved;
      if (!ItemIsMandatory(listIndex)) {
        MoveItem(listIndex, m_lbSelected, m_lbAvailable);
        nRemoved++;
      }
    }
  }
}

void FieldSelectionPanel::OnRemoveAll( wxCommandEvent& /* evt */ )
{
  for(size_t itemsLeft = m_lbSelected->GetCount(), idx = 0; idx < itemsLeft; ) {
    if (!ItemIsMandatory(idx)) {
      MoveItem(idx, m_lbSelected, m_lbAvailable);
      --itemsLeft;
    }
    else
      ++idx;
  }
}

///////////////////////////////////////////////////////////////////////////
// FieldSelectionPanelValidator
//
#define A2ITR(a, n) a, a + (a? n: 0)
FieldSelectionPanelValidator::FieldSelectionPanelValidator(const CItemData::FieldType* available, size_t navail,
                                                           const CItemData::FieldType* mandatory, size_t nmand,
                                                           FieldSet& userSelection,
                                                           const wxString& validationMessage,
                                                           const wxString& validationTitle)
                                                            :m_availableFields(A2ITR(available,navail)),
                                                             m_mandatoryFields(A2ITR(mandatory,nmand)),
                                                             m_userSelectedFields(userSelection),
                                                             m_validationMessage(validationMessage),
                                                             m_validationTitle(validationTitle)
{
  SanitizeSelections();
}

FieldSelectionPanelValidator::FieldSelectionPanelValidator(const FieldSet& available,
                                                           const FieldSet& mandatory,
                                                           FieldSet& userSelection,
                                                           const wxString& validationMessage,
                                                           const wxString& validationTitle)
                                                            :m_availableFields(available),
                                                             m_mandatoryFields(mandatory),
                                                             m_userSelectedFields(userSelection),
                                                             m_validationMessage(validationMessage),
                                                             m_validationTitle(validationTitle)
{
  SanitizeSelections();
}

void FieldSelectionPanelValidator::SanitizeSelections()
{
  //all mandatory fields will be pre-selected
  m_userSelectedFields.insert(m_mandatoryFields.begin(), m_mandatoryFields.end());

  //selected fields must be a sub-set of available fields
  m_availableFields.insert(m_userSelectedFields.begin(), m_userSelectedFields.end());
}

bool FieldSelectionPanelValidator::TransferFromWindow()
{
  if (GetWindow() && wxIS_KIND_OF(GetWindow(), FieldSelectionPanel)) {
    m_userSelectedFields.clear();
    FieldSelectionPanel* fsp = wxDynamicCast(GetWindow(), FieldSelectionPanel);
    for (size_t idx = 0; idx < fsp->GetNumSelectedFields(); ++idx) {
      CItemData::FieldType ft = fsp->GetSelectedFieldAt(idx);
      wxCHECK2_MSG(ft != CItemData::END, continue, wxT("Unexpected field type in selected item"));
      m_userSelectedFields.insert(ft);
    }
    return true;
  }
  return false;
}

bool FieldSelectionPanelValidator::TransferToWindow()
{
  if (GetWindow() && wxIS_KIND_OF(GetWindow(), FieldSelectionPanel)) {
    FieldSelectionPanel* fsp = wxDynamicCast(GetWindow(), FieldSelectionPanel);
    for (FieldSet::const_iterator itr = m_availableFields.begin(); itr != m_availableFields.end(); ++itr) {
      fsp->AddField(*itr, m_userSelectedFields.find(*itr) != m_userSelectedFields.end(),
                                    m_mandatoryFields.find(*itr) != m_mandatoryFields.end());
    }
    return true;
  }
  return false;
}

bool FieldSelectionPanelValidator::Validate(wxWindow* parent)
{
  if (GetWindow() && wxIS_KIND_OF(GetWindow(), FieldSelectionPanel)) {
    FieldSelectionPanel* fsp = wxDynamicCast(GetWindow(), FieldSelectionPanel);
    if (fsp->GetNumSelectedFields() == 0) {
      wxMessageBox(m_validationMessage, m_validationTitle, wxOK|wxICON_EXCLAMATION, parent);
      return false;
    }
  }
  return true;
}
