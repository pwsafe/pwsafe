/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersMediaTypesDlg.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/valgen.h>

#include "core/PWSFilters.h"
#include "PWFiltersMediaDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"

//(*IdInit(pwFiltersMediaTypesDlg)
const PWSMatch::MatchRule pwFiltersMediaTypesDlg::m_mrpres[PW_NUM_PRESENT_ENUM] = {
      PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT };
const PWSMatch::MatchRule pwFiltersMediaTypesDlg::m_mrcrit[PW_NUM_MEDIA_TYPES_CRITERIA_ENUM] = {
      PWSMatch::MR_EQUALS, PWSMatch::MR_NOTEQUAL,
      PWSMatch::MR_BEGINS, PWSMatch::MR_NOTBEGIN,
      PWSMatch::MR_ENDS, PWSMatch::MR_NOTEND,
      PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
      PWSMatch::MR_CNTNANY, PWSMatch::MR_NOTCNTNANY,
      PWSMatch::MR_CNTNALL, PWSMatch::MR_NOTCNTNALL };
//*)

/*!
 * pwFiltersMediaTypesDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersMediaTypesDlg, wxDialog )

/*!
 * pwFiltersMediaTypesDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersMediaTypesDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersMediaTypesDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX69, pwFiltersMediaTypesDlg::OnSelectionChange )
  EVT_COMBOBOX( ID_COMBOBOX70, pwFiltersMediaTypesDlg::OnMediaTypeChange )
  EVT_TEXT( ID_COMBOBOX70, pwFiltersMediaTypesDlg::OnMediaTypeChange )
  EVT_BUTTON( wxID_CANCEL, pwFiltersMediaTypesDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersMediaTypesDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersMediaTypesDlg constructors
 */

pwFiltersMediaTypesDlg::pwFiltersMediaTypesDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase, const std::set<StringX> *psMediaTypes)
: m_ftype(ftype), m_psMediaTypes(psMediaTypes),
  m_prule(rule), m_pvalue(value), m_pfcase(fcase)
{
  wxASSERT(!parent || parent->IsTopLevel());

  Init();
  
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter Media Types Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersMediaTypesDlg* pwFiltersMediaTypesDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase, const std::set<StringX> *psMediaTypes)
{
  return new pwFiltersMediaTypesDlg(parent, ftype, rule, value, fcase, psMediaTypes);
}

/*!
 * InitDialog set selection list and optimize window size
 */

void pwFiltersMediaTypesDlg::InitDialog()
{
  wxSize actSize = m_ComboBox->GetSize(),
         oldSize = actSize,
         borderSize = m_ComboBox->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord deltaWidth = 0,
          width, height,
          border = (borderSize.GetWidth() * 2) + 2;
  
  dc.SetFont(m_ComboBox->GetFont());
  
  if(m_add_present) {
    for(size_t i = 0; i < PW_NUM_PRESENT_ENUM; i++) {
      int iumsg = PWSMatch::GetRule(m_mrpres[i]);
      wxString value = LoadAString(iumsg);
      m_ComboBox->Append(value);
      // Determin new size
      dc.GetTextExtent(value, &width, &height);
      width += border; // Align
      if(width > actSize.GetWidth())
        actSize.SetWidth(width + 6);
    }
  }
  
  for(size_t i = 0; i < PW_NUM_MEDIA_TYPES_CRITERIA_ENUM; i++) {
    int iumsg = PWSMatch::GetRule(m_mrcrit[i]);
    wxString value = LoadAString(iumsg);
    m_ComboBox->Append(value);
    // Determin new size
    dc.GetTextExtent(value, &width, &height);
    width += border; // Align
    if(width > actSize.GetWidth())
      actSize.SetWidth(width + 6);
  }

  m_idx = -1;
  // Determine actual index
  if(m_add_present) {
    for(int i = 0; i < PW_NUM_PRESENT_ENUM; i++) {
      if(*m_prule == m_mrpres[i]) {
        m_idx = i;
        // By default disable when present or not present is selected
        m_MediaTypes->Disable();
        m_CheckBoxFCase->Disable();
        break;
      }
    }
  }
  if(m_idx == -1) {
    for(int i = 0; i < PW_NUM_MEDIA_TYPES_CRITERIA_ENUM; i++) {
      if(*m_prule == m_mrcrit[i]) {
        m_idx = i + (m_add_present ? PW_NUM_PRESENT_ENUM : 0); // Selection of present is optionally before the choice of criteria
        break;
      }
    }
  }
  
  m_ComboBox->SetSelection(m_idx);

  if(actSize.GetWidth() != oldSize.GetWidth())
    deltaWidth = actSize.GetWidth() - oldSize.GetWidth();

  // Setup selection of Media Type
  actSize = m_MediaTypes->GetSize();
  oldSize = actSize;
  borderSize = m_MediaTypes->GetWindowBorderSize();
  border = (borderSize.GetWidth() * 2) + 2;
  dc.SetFont(m_MediaTypes->GetFont());

  m_MediaTypes->Append(_T("")); // Allow empty selection
  if (m_psMediaTypes != nullptr) {
    for (auto iter = m_psMediaTypes->begin();
         iter != m_psMediaTypes->end();
         iter++) {
      wxString value = iter->c_str();
      m_MediaTypes->Append(value);
      // Set column size to fit
      dc.GetTextExtent(value, &width, &height);
      width += border; // Align
      if(width > actSize.GetWidth())
        actSize.SetWidth(width + 6);
    }
  }

  if((actSize.GetWidth() - oldSize.GetWidth()) > deltaWidth)
    deltaWidth = actSize.GetWidth() - oldSize.GetWidth();
  
  // Resize window if too small to include full choice string
  if(deltaWidth) {
    GetSize(&width, &height);
    width += deltaWidth + 6;
    int displayWidth, displayHight;
    ::wxDisplaySize(&displayWidth, &displayHight);
    if(width > displayWidth) width = displayWidth;
    SetSize(width, height);
  }
  
  m_MediaTypes->SetValue(m_string);
  m_CheckBoxFCase->SetValue(m_fcase);
  
  if(m_add_present && (m_idx >= 0) && (m_idx < PW_NUM_PRESENT_ENUM)) {
    FindWindow(wxID_OK)->Enable();
    m_MediaTypes->Disable();
    m_CheckBoxFCase->Disable();
  }
  else if(m_string.IsEmpty()) {
    FindWindow(wxID_OK)->Disable();
  }
}

/*!
 * SetValidators
 */

void pwFiltersMediaTypesDlg::SetValidators()
{
  m_ComboBox->SetValidator(wxGenericValidator(&m_idx));
  m_MediaTypes->SetValidator(wxGenericValidator(&m_string));
  m_CheckBoxFCase->SetValidator(wxGenericValidator(&m_fcase));
}

/*!
 * Member initialisation
 */

void pwFiltersMediaTypesDlg::Init()
{
  switch (m_ftype) {
    case AT_MEDIATYPE:
      m_add_present = false;
      break;
    default:
      wxASSERT(false);
      m_add_present = true;
  }
  
  m_string = *m_pvalue;
  m_fcase = *m_pfcase;
}

/*!
 * Control creation for pwFiltersMediaTypesDlg
 */

void pwFiltersMediaTypesDlg::CreateControls()
{
  //(*Initialize(pwFiltersMediaTypesDlg
  auto BoxSizer1 = new wxBoxSizer(wxVERTICAL);
  
  // First line with Type
  auto itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  BoxSizer1->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL, 5);

  auto itemStaticText1 = new wxStaticText( this, wxID_STATIC, pwFiltersTable::GetColLabelString(FLC_FLD_COMBOBOX) + _T(": "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer1->Add(itemStaticText1, 0, wxALIGN_CENTER_VERTICAL/*|wxALIGN_RIGHT*/|wxALL, 5);
  auto itemStaticText2 = new wxStaticText( this, wxID_STATIC, pwFiltersFTChoiceRenderer::getFieldTypeString(m_ftype) + _T(" "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_CENTER_VERTICAL/*|wxALIGN_LEFT*/|wxALL, 5);
  
  // Second line with choice inside a box
  auto staticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this);
  BoxSizer1->Add(staticBoxSizer1, 1, wxALL|wxEXPAND, 5);
    
  auto *itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  staticBoxSizer1->Add(itemFlexGridSizer1, 1, wxEXPAND | wxALL, 5);
  itemFlexGridSizer1->AddGrowableCol(1);
  
  auto choiceStaticText1 = new wxStaticText(this, wxID_ANY, _("Rule") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_ComboBox = new wxComboBox(this, ID_COMBOBOX69, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBox, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);
  
  auto choiceStaticText2 = new wxStaticText(this, wxID_ANY, _("Media Type") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_MediaTypes = new wxComboBox(this, ID_COMBOBOX70, m_string, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_MediaTypes, 1, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);
  
  itemFlexGridSizer1->AddSpacer(5);
  
  m_CheckBoxFCase = new wxCheckBox( this, ID_CHECKBOX71, _("Case Sensitive"), wxDefaultPosition, wxDefaultSize, 0 );
  m_CheckBoxFCase->SetValue(false);
  itemFlexGridSizer1->Add(m_CheckBoxFCase, 0, wxALIGN_LEFT|wxBOTTOM|wxALL, 5);
  
  // Third line is carrying dialog buttons
  auto StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_HELP, wxEmptyString));
  StdDialogButtonSizer1->Realize();
  BoxSizer1->Add(StdDialogButtonSizer1, 0, wxALL|wxEXPAND, 5);

  SetSizer(BoxSizer1);
  BoxSizer1->Fit(this);
  BoxSizer1->SetSizeHints(this);
  //*)
}

/*!
 * Should we show tooltips?
 */

bool pwFiltersMediaTypesDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersMediaTypesDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersMediaTypesDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersMediaTypesDlg::OnSelectionChange(wxCommandEvent& WXUNUSED(event))
{
  m_idx = m_ComboBox->GetSelection();

  if(m_add_present) {
    if((m_idx >= 0) && (m_idx < PW_NUM_PRESENT_ENUM)) {
      FindWindow(wxID_OK)->Enable();
      m_MediaTypes->Disable();
      m_CheckBoxFCase->Disable();
    }
    else {
      if(m_MediaTypes && ! m_MediaTypes->GetValue().IsEmpty()) {
        FindWindow(wxID_OK)->Enable();
      }
      else {
        FindWindow(wxID_OK)->Disable();
      }
      m_MediaTypes->Enable();
      m_CheckBoxFCase->Enable();
    }
  }
  else {
    if(m_MediaTypes && ! m_MediaTypes->GetValue().IsEmpty()) {
      FindWindow(wxID_OK)->Enable();
    }
    else {
      FindWindow(wxID_OK)->Disable();
    }
  }
}

/*!
 * wxEVT_TEXT event handler for wxTextCtrl
 */

void pwFiltersMediaTypesDlg::OnMediaTypeChange(wxCommandEvent& WXUNUSED(event))
{
  if(m_MediaTypes && ! m_MediaTypes->GetValue().IsEmpty()) {
    FindWindow(wxID_OK)->Enable();
  }
  else {
    FindWindow(wxID_OK)->Disable();
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersMediaTypesDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    int idx = m_ComboBox->GetSelection();
 
    if(idx < 0) {
      EndModal(wxID_CANCEL);
      return;
    }
    if(m_add_present) {
      if((idx >= 0) && (idx < PW_NUM_PRESENT_ENUM)) {
        *m_prule = m_mrpres[idx];
        m_string = L"";
        m_fcase = false;
      }
      else {
        idx -= PW_NUM_PRESENT_ENUM;
        if(idx < PW_NUM_MEDIA_TYPES_CRITERIA_ENUM)
          *m_prule = m_mrcrit[idx];
        else
          *m_prule = PWSMatch::MR_INVALID;
        
        m_string = m_MediaTypes->GetValue();
        m_fcase = m_CheckBoxFCase->GetValue();
      }
    }
    else {
      if(idx < PW_NUM_MEDIA_TYPES_CRITERIA_ENUM)
        *m_prule = m_mrcrit[idx];
      else
        *m_prule = PWSMatch::MR_INVALID;
      
      m_string = m_MediaTypes->GetValue();
      m_fcase = m_CheckBoxFCase->GetValue();
    }
    
    *m_pvalue = m_string;
    *m_pfcase = m_fcase;
  }
  EndModal(wxID_OK);
}

bool pwFiltersMediaTypesDlg::IsChanged() const {
  const auto idx = m_ComboBox->GetSelection();

  if (idx < 0) {
    return false;
  }

  if (m_add_present) {
    if (idx >= 0 && idx < PW_NUM_PRESENT_ENUM) {
      if (*m_prule != m_mrpres[idx]) {
        return true;
      }
    }
    else {
      const auto tmpIdx = idx - PW_NUM_PRESENT_ENUM;
      if(tmpIdx < PW_NUM_MEDIA_TYPES_CRITERIA_ENUM) {
        if (*m_prule != m_mrcrit[tmpIdx]) {
          return true;
        }
      }
      else if (*m_prule != PWSMatch::MR_INVALID) {
        return true;
      }
    }
  }
  else {
    if(idx < PW_NUM_MEDIA_TYPES_CRITERIA_ENUM) {
      if (*m_prule != m_mrcrit[idx]) {
        return true;
      }
    }
    else if (*m_prule != PWSMatch::MR_INVALID) {
      return true;
    }
  }

  if (*m_pvalue != m_MediaTypes->GetValue()) {
    return true;
  }

  if (*m_pfcase != m_CheckBoxFCase->GetValue()) {
    return true;
  }

  return false;
}
