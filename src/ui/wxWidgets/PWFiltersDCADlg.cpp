/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersDCADlg.cpp
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
#include "PWFiltersDCADlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"
#include "wxUtilities.h"

//(*IdInit(pwFiltersDCADlg)
const PWSMatch::MatchRule pwFiltersDCADlg::m_mrx[PW_NUM_DCA_RULE_ENUM] = {
                            PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT,
                            PWSMatch::MR_IS,      PWSMatch::MR_ISNOT};

const pwFiltersDCADlg::tDcaMapItem pwFiltersDCADlg::m_mdca[PW_NUM_DCA_ENUM] = {
 { IDSC_CURRENTDEFAULTDCA, -1 }, // -1 is default action
 { IDSC_DCACOPYPASSWORD, PWSprefs::DoubleClickCopyPassword },
 { IDSC_DCAVIEWEDIT, PWSprefs::DoubleClickViewEdit },
 { IDSC_DCAAUTOTYPE, PWSprefs::DoubleClickAutoType },
 { IDSC_DCABROWSE, PWSprefs::DoubleClickBrowse },
 { IDSC_DCACOPYNOTES, PWSprefs::DoubleClickCopyNotes },
 { IDSC_DCACOPYUSERNAME, PWSprefs::DoubleClickCopyUsername },
 { IDSC_DCACOPYPASSWORDMIN, PWSprefs::DoubleClickCopyPasswordMinimize },
 { IDSC_DCABROWSEPLUS, PWSprefs::DoubleClickBrowsePlus },
 { IDSC_DCARUN, PWSprefs::DoubleClickRun },
 { IDSC_DCASENDEMAIL, PWSprefs::DoubleClickSendEmail },
};
//*)

/*!
 * pwFiltersDCADlg type definition
 */

IMPLEMENT_CLASS( pwFiltersDCADlg, wxDialog )

/*!
 * pwFiltersDCADlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersDCADlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersDCADlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX55, pwFiltersDCADlg::OnSelectionChangeRule )
  EVT_COMBOBOX( ID_COMBOBOX56, pwFiltersDCADlg::OnSelectionChangeDCA )
  EVT_BUTTON( wxID_CANCEL, pwFiltersDCADlg::OnCancelClick )
  EVT_CLOSE( pwFiltersDCADlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersDCADlg constructors
 */

pwFiltersDCADlg::pwFiltersDCADlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, short *fdca)
: m_ftype(ftype)
{
  wxASSERT(!parent || parent->IsTopLevel());

  m_prule = rule;
  m_pfdca = fdca;

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter DCA Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersDCADlg* pwFiltersDCADlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, short *fdca)
{
  return new pwFiltersDCADlg(parent, ftype, rule, fdca);
}

/*!
 * CurrentDefaultDCAuiString returns the actual default value as string
 */

stringT pwFiltersDCADlg::CurrentDefaultDCAuiString()
{
  int ui_dca = -1;
  stringT result;
  const short si_DCA = (short)PWSprefs::GetInstance()->GetPref(m_ftype == FT_DCA ?
                                                               PWSprefs::DoubleClickAction :
                                                               PWSprefs::ShiftDoubleClickAction);

  switch (si_DCA) {
    case PWSprefs::DoubleClickAutoType:             ui_dca = IDSC_DCAAUTOTYPE;        break;
    case PWSprefs::DoubleClickBrowse:               ui_dca = IDSC_DCABROWSE;          break;
    case PWSprefs::DoubleClickCopyNotes:            ui_dca = IDSC_DCACOPYNOTES;       break;
    case PWSprefs::DoubleClickCopyPassword:         ui_dca = IDSC_DCACOPYPASSWORD;    break;
    case PWSprefs::DoubleClickCopyUsername:         ui_dca = IDSC_DCACOPYUSERNAME;    break;
    case PWSprefs::DoubleClickViewEdit:             ui_dca = IDSC_DCAVIEWEDIT;        break;
    case PWSprefs::DoubleClickCopyPasswordMinimize: ui_dca = IDSC_DCACOPYPASSWORDMIN; break;
    case PWSprefs::DoubleClickBrowsePlus:           ui_dca = IDSC_DCABROWSEPLUS;      break;
    case PWSprefs::DoubleClickRun:                  ui_dca = IDSC_DCARUN;             break;
    case PWSprefs::DoubleClickSendEmail:            ui_dca = IDSC_DCASENDEMAIL;       break;
    default:                                        ui_dca = IDSC_STATCOMPANY;
  }
  LoadAString(result, ui_dca);
  return result;
}

/*!
 * InitDialog set selection in choice list and optimize window size
 */

void pwFiltersDCADlg::InitDialog()
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
    for(size_t i = 0; i < PW_NUM_DCA_RULE_ENUM; i++) {
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
  // Search actual selected rule choice
  for(int i = 0; i < PW_NUM_DCA_RULE_ENUM; i++) {
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

  // Setup selection of DCA
  actSize = m_ComboBoxDCA->GetSize();
  oldSize = actSize;
  borderSize = m_ComboBoxDCA->GetWindowBorderSize();
  border = (borderSize.GetWidth() * 2) + 2;
  dc.SetFont(m_ComboBoxDCA->GetFont());

  if(! m_ComboBoxDCA->GetCount()) {
    for(size_t i = 0; i < PW_NUM_DCA_ENUM; i++) {
      wxString value;
      if(m_mdca[i].msgText == IDSC_CURRENTDEFAULTDCA) {
        stringT current = CurrentDefaultDCAuiString();
        stringT fstr;
        Format(fstr, IDSC_CURRENTDEFAULTDCA, current.c_str()); // Leading blank not needed
        value = towxstring(fstr.substr(1));
      }
      else {
        value = LoadAString(m_mdca[i].msgText);
      }
      m_ComboBoxDCA->Append(value);
      // Set column size to fit
      dc.GetTextExtent(value, &width, &height);
      width += border; // Align
      if(width > actSize.GetWidth())
        actSize.SetWidth(width + 6);
    }
  }
  
  // Search actual selected DCA choice
  m_idx_dca = -1;
  if(m_idx != -1) {
    for(int i = 0; i < PW_NUM_DCA_ENUM; i++) {
      if(*m_pfdca == m_mdca[i].dcaValue) {
        m_idx_dca = i;
        break;
      }
    }
  }

  if(m_idx >= 2 && // Index 0 and 1 are PRESENT or NOT PRESENT
     m_idx_dca == -1) {
    FindWindow(wxID_OK)->Disable();
  }

  m_ComboBoxDCA->SetSelection(m_idx_dca);
  
  if(m_idx >= 0 && m_idx < 2) { // Index 0 and 1 are PRESENT or NOT PRESENT
    m_ComboBoxDCA->Disable();
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
}

/*!
 * SetValidators
 */

void pwFiltersDCADlg::SetValidators()
{
  m_ComboBoxRule->SetValidator(wxGenericValidator(&m_idx));
  m_ComboBoxDCA->SetValidator(wxGenericValidator(&m_idx_dca));
}

/*!
 * Control creation for pwFiltersDCADlg
 */

void pwFiltersDCADlg::CreateControls()
{
  //(*Initialize(pwFiltersDCADlg
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

  m_ComboBoxRule = new wxComboBox(this, ID_COMBOBOX55, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBoxRule, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);
  
  auto choiceStaticText2 = new wxStaticText(this, wxID_ANY, _("DCA") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_ComboBoxDCA = new wxComboBox(this, ID_COMBOBOX56, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBoxDCA, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);

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

bool pwFiltersDCADlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersDCADlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersDCADlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersDCADlg::OnSelectionChangeRule(wxCommandEvent& WXUNUSED(event))
{
  if(m_ComboBoxRule && m_ComboBoxDCA) {
    m_idx = m_ComboBoxRule->GetSelection();
  
    if((m_idx > 1) && (m_idx < PW_NUM_DCA_RULE_ENUM)) { // is not MR_PRESENT or MR_NOTPRESENT
      m_ComboBoxDCA->Enable();
      if((m_idx_dca >= 0) && (m_idx_dca < PW_NUM_DCA_ENUM))
        FindWindow(wxID_OK)->Enable();
      else
        FindWindow(wxID_OK)->Disable();
    }
    else if((m_idx == 0) || (m_idx == 1)) { // MR_PRESENT or MR_NOTPRESENT
      FindWindow(wxID_OK)->Enable();
      m_ComboBoxDCA->Disable();
    }
  }
}

void pwFiltersDCADlg::OnSelectionChangeDCA(wxCommandEvent& WXUNUSED(event))
{
  if(m_ComboBoxDCA) {
    m_idx_dca = m_ComboBoxDCA->GetSelection();
    
    if((m_idx >= 0) && (m_idx < PW_NUM_DCA_RULE_ENUM) &&
       (m_idx_dca >= 0) && (m_idx_dca < PW_NUM_DCA_ENUM))
      FindWindow(wxID_OK)->Enable();
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersDCADlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    m_idx = m_ComboBoxRule->GetSelection();
    m_idx_dca = m_ComboBoxDCA->GetSelection();
 
    if((m_idx >= 0) && (m_idx < PW_NUM_DCA_RULE_ENUM)) {
      *m_prule = m_mrx[m_idx];
    }
    else {
      // On empty choice handle as cancel
      *m_prule = PWSMatch::MR_INVALID;
      EndModal(wxID_CANCEL);
      return;
    }
    if((m_idx_dca >= 0) && (m_idx_dca < PW_NUM_DCA_ENUM)) {
      *m_pfdca = m_mdca[m_idx_dca].dcaValue;
    }
    else if(m_idx > 1) { // is not MR_PRESENT or MR_NOTPRESENT
      // On empty choice handle as cancel
      *m_prule = PWSMatch::MR_INVALID;
      EndModal(wxID_CANCEL);
      return;
    }
  }
  EndModal(wxID_OK);
}

bool pwFiltersDCADlg::IsChanged() const {
  const auto idx = m_ComboBoxRule->GetSelection();
  if(idx >= 0 && idx < PW_NUM_DCA_RULE_ENUM) {
    if (*m_prule != m_mrx[idx]) {
      return true;
    }
  }
  else if (*m_prule != PWSMatch::MR_INVALID) {
    return true;
  }

  const auto idx_dca = m_ComboBoxDCA->GetSelection();

  if(idx_dca >= 0 && idx_dca < PW_NUM_DCA_ENUM) {
    if (*m_pfdca != m_mdca[idx_dca].dcaValue) {
      return true;
    }
  }
  else if(idx > 1) { // is not MR_PRESENT or MR_NOTPRESENT
    if (*m_prule != PWSMatch::MR_INVALID) {
      return true;
    }
  }

  return false;
}
