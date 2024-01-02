/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersTypeDlg.cpp
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
#include "PWFiltersTypeDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"
#include "wxUtilities.h"

//(*IdInit(pwFiltersTypeDlg)
const PWSMatch::MatchRule pwFiltersTypeDlg::m_mrx[PW_NUM_TYPE_RULE_ENUM] = {
                            PWSMatch::MR_IS,      PWSMatch::MR_ISNOT};

const pwFiltersTypeDlg::tEtypeMapItem pwFiltersTypeDlg::m_mtype[PW_NUM_TYPE_ENUM] = {
 { IDSC_FNORMAL, CItemData::ET_NORMAL },
 { IDSC_FALIAS, CItemData::ET_ALIAS },
 { IDSC_FSHORTCUT, CItemData::ET_SHORTCUT },
 { IDSC_FALIASBASE, CItemData::ET_ALIASBASE },
 { IDSC_FSHORTCUTBASE, CItemData::ET_SHORTCUTBASE },
};
//*)

/*!
 * pwFiltersTypeDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersTypeDlg, wxDialog )

/*!
 * pwFiltersTypeDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersTypeDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersTypeDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX62, pwFiltersTypeDlg::OnSelectionChangeRule )
  EVT_COMBOBOX( ID_COMBOBOX63, pwFiltersTypeDlg::OnSelectionChangeType )
  EVT_BUTTON( wxID_CANCEL, pwFiltersTypeDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersTypeDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersTypeDlg constructors
 */

pwFiltersTypeDlg::pwFiltersTypeDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, CItemData::EntryType *etype)
: m_ftype(ftype), m_prule(rule), m_petype(etype)
{
  wxASSERT(!parent || parent->IsTopLevel());

  m_etype = *m_petype;

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter Entry Type Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersTypeDlg* pwFiltersTypeDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, CItemData::EntryType *etype)
{
  return new pwFiltersTypeDlg(parent, ftype, rule, etype);
}

/*!
 * InitDialog set selection list and optimize window size
 */

void pwFiltersTypeDlg::InitDialog()
{
  wxSize actSize = m_ComboBoxRule->GetSize(),
         oldSize = actSize,
         borderSize = m_ComboBoxRule->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord deltaWidth = 0,
          width, height,
          border = (borderSize.GetWidth() * 2) + 2;
  
  dc.SetFont(m_ComboBoxRule->GetFont());
  
  // Setup selection of Rule
  if(! m_ComboBoxRule->GetCount()) {
    for(size_t i = 0; i < PW_NUM_TYPE_RULE_ENUM; i++) {
      int iumsg = PWSMatch::GetRule(m_mrx[i]);
      wxString value = LoadAString(iumsg);
      m_ComboBoxRule->Append(value);
      // Set column size to fit
      dc.GetTextExtent(value, &width, &height);
      width += border; // Align
      if(width > actSize.GetWidth())
        actSize.SetWidth(width + 6);
    }
  }
  
  m_idx = -1;
  // Determine actual index
  for(int i = 0; i < PW_NUM_TYPE_RULE_ENUM; i++) {
    if(*m_prule == m_mrx[i]) {
      m_idx = i;
      break;
    }
  }
  if(m_idx == -1) {
    FindWindow(wxID_OK)->Disable();
  }
  
  m_ComboBoxRule->SetSelection(m_idx);

  if(actSize.GetWidth() != oldSize.GetWidth())
    deltaWidth = actSize.GetWidth() - oldSize.GetWidth();

  // Setup selection of Type
  actSize = m_ComboBoxType->GetSize();
  oldSize = actSize;
  borderSize = m_ComboBoxType->GetWindowBorderSize();
  border = (borderSize.GetWidth() * 2) + 2;
  dc.SetFont(m_ComboBoxType->GetFont());

  if(! m_ComboBoxType->GetCount()) {
    for(size_t i = 0; i < PW_NUM_TYPE_ENUM; i++) {
      wxString value = LoadAString(m_mtype[i].msgText);
      m_ComboBoxType->Append(value);
      // Set column size to fit
      dc.GetTextExtent(value, &width, &height);
      width += border; // Align
      if(width > actSize.GetWidth())
        actSize.SetWidth(width + 6);
    }
  }
  
  // Search actual selected type choice
  m_idx_type = -1;
  if(m_idx != -1) {
    for(int i = 0; i < PW_NUM_TYPE_ENUM; i++) {
      if(m_etype == m_mtype[i].typeValue) {
        m_idx_type = i;
        break;
      }
    }
  }

  if(m_idx_type == -1) {
    FindWindow(wxID_OK)->Disable();
  }

  m_ComboBoxType->SetSelection(m_idx_type);
  
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
}

/*!
 * SetValidators
 */

void pwFiltersTypeDlg::SetValidators()
{
  m_ComboBoxRule->SetValidator(wxGenericValidator(&m_idx));
  m_ComboBoxType->SetValidator(wxGenericValidator(&m_idx_type));
}

/*!
 * Control creation for pwFiltersTypeDlg
 */

void pwFiltersTypeDlg::CreateControls()
{
  //(*Initialize(pwFiltersTypeDlg
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
  BoxSizer1->Add(staticBoxSizer1, 0, wxALL|wxEXPAND, 5);
  
  auto *itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  staticBoxSizer1->Add(itemFlexGridSizer1, 1, wxEXPAND | wxALL, 5);
  itemFlexGridSizer1->AddGrowableCol(1);
  
  auto choiceStaticText1 = new wxStaticText(this, wxID_ANY, _("Rule") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);

  m_ComboBoxRule = new wxComboBox(this, ID_COMBOBOX62, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBoxRule, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);
  
  auto choiceStaticText2 = new wxStaticText(this, wxID_ANY, _("Type") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_ComboBoxType = new wxComboBox(this, ID_COMBOBOX63, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBoxType, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);

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

bool pwFiltersTypeDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersTypeDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersTypeDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersTypeDlg::OnSelectionChangeRule(wxCommandEvent& WXUNUSED(event))
{
  m_idx = m_ComboBoxRule->GetSelection();
  
  if((m_idx >= 0) && (m_idx < PW_NUM_TYPE_RULE_ENUM) &&
     (m_idx_type >= 0) && (m_idx_type < PW_NUM_TYPE_ENUM))
    FindWindow(wxID_OK)->Enable();
}

void pwFiltersTypeDlg::OnSelectionChangeType(wxCommandEvent& WXUNUSED(event))
{
  m_idx_type = m_ComboBoxType->GetSelection();
    
  if((m_idx >= 0) && (m_idx < PW_NUM_TYPE_RULE_ENUM) &&
     (m_idx_type >= 0) && (m_idx_type < PW_NUM_TYPE_ENUM))
    FindWindow(wxID_OK)->Enable();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersTypeDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    m_idx = m_ComboBoxRule->GetSelection();
    m_idx_type = m_ComboBoxType->GetSelection();
 
    if((m_idx >= 0) && (m_idx < PW_NUM_TYPE_RULE_ENUM)) {
      *m_prule = m_mrx[m_idx];
    }
    else {
      // On empty choice handle as cancel
      *m_prule = PWSMatch::MR_INVALID;
      EndModal(wxID_CANCEL);
      return;
    }
    if((m_idx_type >= 0) && (m_idx_type < PW_NUM_TYPE_ENUM)) {
      *m_petype = m_mtype[m_idx_type].typeValue;
    }
    else {
      // On empty choice handle as cancel
      *m_prule = PWSMatch::MR_INVALID;
      EndModal(wxID_CANCEL);
      return;
    }
  }
  EndModal(wxID_OK);
}

bool pwFiltersTypeDlg::IsChanged() const {
  const auto idx = m_ComboBoxRule->GetSelection();

  if(idx >= 0 && idx < PW_NUM_TYPE_RULE_ENUM) {
    if (*m_prule != m_mrx[idx]) {
      return true;
    }
  }
  else if (*m_prule != PWSMatch::MR_INVALID) {
    return true;
  }

  const auto idx_type = m_ComboBoxType->GetSelection();
  if(idx_type >= 0 && idx_type < PW_NUM_TYPE_ENUM) {
    if (*m_petype != m_mtype[idx_type].typeValue) {
      return true;
    }
  }
  else if (*m_prule != PWSMatch::MR_INVALID) {
    return true;
  }
  return false;
}
