/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersPasswordDlg.cpp
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
#include <wx/datetime.h>
//#include <wx/dateevt.h>
//#include <wx/datectrl.h>

#include "core/PWSFilters.h"
#include "PWFiltersPasswordDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"

//(*IdInit(pwFiltersPasswordDlg)
const PWSMatch::MatchRule pwFiltersPasswordDlg::m_mrcrit[PW_NUM_PASSWORD_CRITERIA_ENUM] = {
  PWSMatch::MR_EQUALS,   PWSMatch::MR_NOTEQUAL,
  PWSMatch::MR_BEGINS,   PWSMatch::MR_NOTBEGIN,
  PWSMatch::MR_ENDS,     PWSMatch::MR_NOTEND,
  PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
  PWSMatch::MR_CNTNANY,  PWSMatch::MR_NOTCNTNANY,
  PWSMatch::MR_CNTNALL,  PWSMatch::MR_NOTCNTNALL,
  PWSMatch::MR_EXPIRED,  PWSMatch::MR_WILLEXPIRE };
//*)

/*!
 * pwFiltersPasswordDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersPasswordDlg, wxDialog )

/*!
 * pwFiltersPasswordDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersPasswordDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersPasswordDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX72, pwFiltersPasswordDlg::OnSelectionChange )
  EVT_TEXT( ID_TEXTCTRL73, pwFiltersPasswordDlg::OnTextChange )
  EVT_SPINCTRL( ID_SPINCTRL75, pwFiltersPasswordDlg::OnFNum1Change )
  EVT_BUTTON( wxID_CANCEL, pwFiltersPasswordDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersPasswordDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersPasswordDlg constructors
 */
pwFiltersPasswordDlg::pwFiltersPasswordDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase, int *fnum1)
: m_ftype(ftype),
 m_prule(rule), m_pvalue(value), m_pfcase(fcase), m_pfnum1(fnum1)
{
  wxASSERT(!parent || parent->IsTopLevel());

  Init();

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter Password Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersPasswordDlg* pwFiltersPasswordDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, wxString *value, bool *fcase, int *fnum1)
{
  return new pwFiltersPasswordDlg(parent, ftype, rule, value, fcase, fnum1);
}

/*!
 * InitDialog set selection list and optimize window size
 */

void pwFiltersPasswordDlg::InitDialog()
{
  wxSize actSize = m_ComboBox->GetSize(),
         oldSize = actSize,
         borderSize = m_ComboBox->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord width, height, border = (borderSize.GetWidth() * 2) + 2;
  
  dc.SetFont(m_ComboBox->GetFont());
  
  for(size_t i = 0; i < PW_NUM_PASSWORD_CRITERIA_ENUM; i++) {
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
  for(int i = 0; i < PW_NUM_PASSWORD_CRITERIA_ENUM; i++) {
    if(*m_prule == m_mrcrit[i]) {
      m_idx = i;
      break;
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
  m_FNum1Ctrl->SetValue(m_fnum1);
}

/*!
 * SetValidators
 */

void pwFiltersPasswordDlg::SetValidators()
{
  m_ComboBox->SetValidator(wxGenericValidator(&m_idx));
  m_TextCtrlValueString->SetValidator(wxGenericValidator(&m_string));
  m_CheckBoxFCase->SetValidator(wxGenericValidator(&m_fcase));
  m_FNum1Ctrl->SetValidator(wxGenericValidator(&m_fnum1));
}

/*!
 * Member initialisation
 */

void pwFiltersPasswordDlg::Init()
{
  m_string = *m_pvalue;
  m_fcase = *m_pfcase;
  m_fnum1 = *m_pfnum1;

  m_min = 1;
  // Last 32-bit date is 03:14:07 UTC on Tuesday, January 19, 2038
  // Find number of days from now to 2038/01/18 = max value here
  wxTimeSpan elapsedTime = wxDateTime(18, wxDateTime::Jan, 2038) - wxDateTime::Now();
  m_max = elapsedTime.GetDays();
  
  infoFmtStr = _("Interval [%d to %d]");
}

/*!
 * Control creation for pwFiltersPasswordDlg
 */

void pwFiltersPasswordDlg::CreateControls()
{
  //(*Initialize(pwFiltersPasswordDlg
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
  
  m_ComboBox = new wxComboBox(this, ID_COMBOBOX72, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBox, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);
  
  auto choiceStaticText2 = new wxStaticText(this, wxID_ANY, _("Text") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_TextCtrlValueString = new wxTextCtrl( this, ID_TEXTCTRL73, m_string, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer1->Add(m_TextCtrlValueString, 1, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);
  
  itemFlexGridSizer1->AddSpacer(5);
  
  m_CheckBoxFCase = new wxCheckBox( this, ID_CHECKBOX74, _("Case Sensitive"), wxDefaultPosition, wxDefaultSize, 0 );
  m_CheckBoxFCase->SetValue(m_fcase);
  itemFlexGridSizer1->Add(m_CheckBoxFCase, 0, wxALIGN_LEFT|wxBOTTOM|wxALL, 5);
  
  auto choiceStaticText3 = new wxStaticText(this, wxID_ANY, _("expiry in day(s):") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText3, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  wxString str;
  str = wxString::Format("%d", m_fnum1);
  m_FNum1Ctrl = new wxSpinCtrl(this, ID_SPINCTRL75, str, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_min, m_max, m_fnum1);
  itemFlexGridSizer1->Add(m_FNum1Ctrl, 1, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);

  // Next Row
  itemFlexGridSizer1->AddSpacer(5);
  
  wxScreenDC dc;
  dc.SetFont(choiceStaticText1->GetFont());
  str = wxString::Format(infoFmtStr.c_str(), INT_MIN, INT_MAX);
  wxCoord width, height;
  dc.GetTextExtent(str, &width, &height);
  wxSize size(width+6, height);
  
  str = wxString::Format(infoFmtStr.c_str(), m_min, m_max);
  auto choiceStaticText4 = new wxStaticText(this, wxID_ANY, str + _T(" "), wxDefaultPosition, size, 0);
  itemFlexGridSizer1->Add(choiceStaticText4, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
  
  // Last line is carrying dialog buttons
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

bool pwFiltersPasswordDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersPasswordDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersPasswordDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersPasswordDlg::OnSelectionChange(wxCommandEvent& WXUNUSED(event))
{
  m_idx = m_ComboBox->GetSelection();

  if(m_idx >= 0 && m_idx < PW_NUM_PASSWORD_CRITERIA_ENUM) {
    PWSMatch::MatchRule rule = m_mrcrit[m_idx];
    if(rule == PWSMatch::MR_WILLEXPIRE) {
      m_TextCtrlValueString->Disable();
      m_CheckBoxFCase->Disable();
      m_FNum1Ctrl->Enable();
      FindWindow(wxID_OK)->Enable();
    }
    else if(rule == PWSMatch::MR_EXPIRED) {
      m_TextCtrlValueString->Disable();
      m_CheckBoxFCase->Disable();
      m_FNum1Ctrl->Disable();
      FindWindow(wxID_OK)->Enable();
    }
    else {
      m_TextCtrlValueString->Enable();
      m_CheckBoxFCase->Enable();
      m_FNum1Ctrl->Disable();
      if(m_TextCtrlValueString && m_TextCtrlValueString->GetLineLength(0)) {
        FindWindow(wxID_OK)->Enable();
      }
      else {
        FindWindow(wxID_OK)->Disable();
      }
    }
  }
}

/*!
 * wxEVT_SPIN event handler for wxSpinCtrl
 */

void pwFiltersPasswordDlg::OnFNum1Change(wxSpinEvent& WXUNUSED(event))
{
  m_fnum1 = m_FNum1Ctrl->GetValue();
  FindWindow(wxID_OK)->Enable();
}

/*!
 * wxEVT_TEXT event handler for wxTextCtrl
 */

void pwFiltersPasswordDlg::OnTextChange(wxCommandEvent& WXUNUSED(event))
{
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

void pwFiltersPasswordDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    m_idx = m_ComboBox->GetSelection();
 
    if(m_idx < 0) {
      EndModal(wxID_CANCEL);
      return;
    }
    if(m_idx < PW_NUM_PASSWORD_CRITERIA_ENUM)
      *m_prule = m_mrcrit[m_idx];
    else
      *m_prule = PWSMatch::MR_INVALID;


    m_string = m_TextCtrlValueString->GetValue();

    if(*m_prule != PWSMatch::MR_EXPIRED &&
       *m_prule != PWSMatch::MR_WILLEXPIRE &&
       m_string.IsEmpty()) {
      wxMessageBox(_("Specify text."), _("Missing text for the selected rule."), wxOK|wxICON_ERROR);
      return;
    }
    
    m_fcase = m_CheckBoxFCase->GetValue();
    m_fnum1 = m_FNum1Ctrl->GetValue();

    *m_pvalue = m_string;
    if(*m_prule == PWSMatch::MR_EXPIRED ||
       *m_prule == PWSMatch::MR_WILLEXPIRE)
      *m_pfcase = false;
    else
      *m_pfcase = m_fcase;
    if (*m_prule == PWSMatch::MR_WILLEXPIRE) {
      if (m_fnum1 < 1)
        m_fnum1 = 1;
    } else
      m_fnum1 = 0;
    *m_pfnum1 = m_fnum1;
  }
  EndModal(wxID_OK);
}

bool pwFiltersPasswordDlg::IsChanged() const {
  const auto idx = m_ComboBox->GetSelection();

  if (idx < 0) {
    return false;
  }

  if (idx < PW_NUM_PASSWORD_CRITERIA_ENUM) {
    if (*m_prule != m_mrcrit[idx]) {
      return true;
    }
  }
  else if (*m_prule != PWSMatch::MR_INVALID) {
    return true;
  }

  const auto str = m_TextCtrlValueString->GetValue();
  if(*m_prule != PWSMatch::MR_EXPIRED && *m_prule != PWSMatch::MR_WILLEXPIRE && str.IsEmpty()) {
    return true;
  }

  if (*m_pvalue != str) {
    return true;
  }

  if(*m_prule != PWSMatch::MR_EXPIRED && *m_prule != PWSMatch::MR_WILLEXPIRE && *m_pfcase != m_CheckBoxFCase->GetValue()) {
    return true;
  }

  auto fnum1 = m_FNum1Ctrl->GetValue();
  if (*m_prule == PWSMatch::MR_WILLEXPIRE) {
    if (fnum1 < 1) {
      fnum1 = 1;
    }
  }
  else {
    fnum1 = 0;
  }

  if (*m_pfnum1 != fnum1) {
    return true;
  }

  return false;
}
