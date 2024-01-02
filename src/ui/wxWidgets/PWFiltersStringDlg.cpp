/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersStringDlg.cpp
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
#include "PWFiltersStringDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"

//(*IdInit(pwFiltersStringDlg)
const PWSMatch::MatchRule pwFiltersStringDlg::m_mrpres[PW_NUM_PRESENT_ENUM] = {
                           PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT};
const PWSMatch::MatchRule pwFiltersStringDlg::m_mrcrit[PW_NUM_STR_CRITERIA_ENUM] = {
                           PWSMatch::MR_EQUALS,   PWSMatch::MR_NOTEQUAL,
                           PWSMatch::MR_BEGINS,   PWSMatch::MR_NOTBEGIN,
                           PWSMatch::MR_ENDS,     PWSMatch::MR_NOTEND,
                           PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
                           PWSMatch::MR_CNTNANY,  PWSMatch::MR_NOTCNTNANY,
                           PWSMatch::MR_CNTNALL,  PWSMatch::MR_NOTCNTNALL };
//*)

/*!
 * pwFiltersStringDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersStringDlg, wxDialog )

/*!
 * pwFiltersStringDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersStringDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersStringDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX51, pwFiltersStringDlg::OnSelectionChange )
  EVT_TEXT( ID_TEXTCTRL52, pwFiltersStringDlg::OnTextChange )
  EVT_BUTTON( wxID_CANCEL, pwFiltersStringDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersStringDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersStringDlg constructors
 */

pwFiltersStringDlg::pwFiltersStringDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase)
: m_ftype(ftype), 
  m_prule(rule), m_pvalue(value), m_pfcase(fcase)
{
  wxASSERT(!parent || parent->IsTopLevel());

  Init();

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter String Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersStringDlg* pwFiltersStringDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase)
{
  return new pwFiltersStringDlg(parent, ftype, rule, value, fcase);
}
/*!
 * InitDialog set selection in choice list and optimize window size
 */

void pwFiltersStringDlg::InitDialog()
{
  wxSize actSize = m_ComboBox->GetSize(),
         oldSize = actSize,
         borderSize = m_ComboBox->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord width, height, border = (borderSize.GetWidth() * 2) + 2;
  
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
  
  for(size_t i = 0; i < PW_NUM_STR_CRITERIA_ENUM; i++) {
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
  // Search actual selected choice
  if(m_add_present) {
    for(int i = 0; i < PW_NUM_PRESENT_ENUM; i++) {
      if(*m_prule == m_mrpres[i]) {
        m_idx = i;
        // By default disable when present or not present is selected
        m_TextCtrlValueString->Disable();
        m_CheckBoxFCase->Disable();
        break;
      }
    }
  }
  if(m_idx == -1) {
    for(int i = 0; i < PW_NUM_STR_CRITERIA_ENUM; i++) {
      if(*m_prule == m_mrcrit[i]) {
        m_idx = i + (m_add_present ? PW_NUM_PRESENT_ENUM : 0); // Selection of present is optionally before the choice of criteria
        break;
      }
    }
  }
  
  m_ComboBox->SetSelection(m_idx);
  
  // Resize window if too small to include full choice string
  if(actSize.GetWidth() != oldSize.GetWidth()) {
    GetSize(&width, &height);
    width += actSize.GetWidth() - oldSize.GetWidth() + 6;
    int displayWidth, displayHight;
    ::wxDisplaySize(&displayWidth, &displayHight);
    if(width > displayWidth) width = displayWidth;
    SetSize(width, height);
  }
  
  m_TextCtrlValueString->SetValue(m_string);
  m_CheckBoxFCase->SetValue(m_fcase);
}

/*!
 * SetValidators
 */

void pwFiltersStringDlg::SetValidators()
{
  m_ComboBox->SetValidator(wxGenericValidator(&m_idx));
  m_TextCtrlValueString->SetValidator(wxGenericValidator(&m_string));
  m_CheckBoxFCase->SetValidator(wxGenericValidator(&m_fcase));
}

/*!
 * Member initialisation
 */

void pwFiltersStringDlg::Init()
{
  switch (m_ftype) {
    case FT_GROUPTITLE:
    case FT_TITLE:
      m_add_present = false;
      break;
    case FT_GROUP:
    case FT_USER:
    case FT_NOTES:
    case FT_URL:
    case FT_AUTOTYPE:
    case FT_RUNCMD:
    case FT_EMAIL:
    case FT_SYMBOLS:
    case FT_POLICYNAME:
      m_add_present = true;
      break;
    case AT_FILENAME:
      m_add_present = false;
      break;
    case AT_TITLE:
    case AT_FILEPATH:
      m_add_present = true;
      break;
    default:
      wxASSERT(false);
      m_add_present = true;
  }
  
  m_string = *m_pvalue;
  m_fcase = *m_pfcase;
}

/*!
 * Control creation for pwFiltersStringDlg
 */

void pwFiltersStringDlg::CreateControls()
{
  //(*Initialize(pwFiltersStringDlg
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
  
  m_ComboBox = new wxComboBox(this, ID_COMBOBOX51, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBox, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);
  
  auto choiceStaticText2 = new wxStaticText(this, wxID_ANY, _("Text") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_TextCtrlValueString = new wxTextCtrl( this, ID_TEXTCTRL52, m_string, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer1->Add(m_TextCtrlValueString, 1, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);
  
  itemFlexGridSizer1->AddSpacer(5);
  
  m_CheckBoxFCase = new wxCheckBox( this, ID_CHECKBOX53, _("Case Sensitive"), wxDefaultPosition, wxDefaultSize, 0 );
  m_CheckBoxFCase->SetValue(m_fcase);
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
  m_controlsReady = true;
}

/*!
 * Should we show tooltips?
 */

bool pwFiltersStringDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersStringDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersStringDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersStringDlg::OnSelectionChange(wxCommandEvent& WXUNUSED(event))
{
  if (!m_controlsReady) {
    return;
  }
  int idx = m_ComboBox->GetSelection();

  if(m_add_present) {
    if((idx >= 0) && (idx < PW_NUM_PRESENT_ENUM)) {
      FindWindow(wxID_OK)->Enable();
      m_TextCtrlValueString->Disable();
      m_CheckBoxFCase->Disable();
    }
    else {
      if(m_TextCtrlValueString && m_TextCtrlValueString->GetLineLength(0)) {
        FindWindow(wxID_OK)->Enable();
      }
      else {
        FindWindow(wxID_OK)->Disable();
      }
      m_TextCtrlValueString->Enable();
      m_CheckBoxFCase->Enable();
    }
  }
  else {
    if(m_TextCtrlValueString && m_TextCtrlValueString->GetLineLength(0)) {
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

void pwFiltersStringDlg::OnTextChange(wxCommandEvent& WXUNUSED(event))
{
  if (!m_controlsReady) {
    return;
  }
  if(m_TextCtrlValueString && m_TextCtrlValueString->GetLineLength(0)) {
    FindWindow(wxID_OK)->Enable();
  }
  else {
    FindWindow(wxID_OK)->Disable();
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersStringDlg::OnOk(wxCommandEvent& WXUNUSED(event))
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
        if(idx < PW_NUM_STR_CRITERIA_ENUM)
          *m_prule = m_mrcrit[idx];
        else
          *m_prule = PWSMatch::MR_INVALID;
        
        m_string = m_TextCtrlValueString->GetValue();
        m_fcase = m_CheckBoxFCase->GetValue();
      }
    }
    else {
      if(idx < PW_NUM_STR_CRITERIA_ENUM)
        *m_prule = m_mrcrit[idx];
      else
        *m_prule = PWSMatch::MR_INVALID;
      
      m_string = m_TextCtrlValueString->GetValue();
      m_fcase = m_CheckBoxFCase->GetValue();
    }
    
    *m_pvalue = m_string;
    *m_pfcase = m_fcase;
  }
  EndModal(wxID_OK);
}

bool pwFiltersStringDlg::IsChanged() const {
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
      if (tmpIdx < PW_NUM_STR_CRITERIA_ENUM) {
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
    if (idx < PW_NUM_STR_CRITERIA_ENUM) {
      if (*m_prule != m_mrcrit[idx]) {
        return true;
      }
    }
    else if (*m_prule != PWSMatch::MR_INVALID) {
      return true;
    }
  }

  if (*m_pvalue != m_TextCtrlValueString->GetValue()) {
    return true;
  }

  if (*m_pfcase != m_CheckBoxFCase->GetValue()) {
    return true;
  }

  return false;
}
