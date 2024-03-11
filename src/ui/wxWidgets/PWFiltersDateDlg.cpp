/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersDateDlg.cpp
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
#include <wx/gbsizer.h>

#include "core/PWSFilters.h"
#include "PWFiltersDateDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"

//(*IdInit(pwFiltersDateDlg)
const PWSMatch::MatchRule pwFiltersDateDlg::m_mrpres[PW_NUM_PRESENT_ENUM] = {
                           PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT };
const PWSMatch::MatchRule pwFiltersDateDlg::m_mrcrit[PW_NUM_DATE_CRITERIA_ENUM] = {
                           PWSMatch::MR_EQUALS, PWSMatch::MR_NOTEQUAL,
                           PWSMatch::MR_BEFORE, PWSMatch::MR_AFTER,
                           PWSMatch::MR_BETWEEN };
//*)

/*!
 * pwFiltersDateDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersDateDlg, wxDialog )

/*!
 * pwFiltersDateDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersDateDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersDateDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX64, pwFiltersDateDlg::OnSelectionChange )
  EVT_DATE_CHANGED( ID_DATECTRL65, pwFiltersDateDlg::OnExpDate1Changed )
  EVT_DATE_CHANGED( ID_DATECTRL66, pwFiltersDateDlg::OnExpDate2Changed )
  EVT_SPINCTRL( ID_SPINCTRL67, pwFiltersDateDlg::OnFNum1Change )
  EVT_SPINCTRL( ID_SPINCTRL68, pwFiltersDateDlg::OnFNum2Change )
  EVT_RADIOBUTTON(  ID_RADIO_BT_ON, pwFiltersDateDlg::OnRadiobuttonOnSelected )
  EVT_RADIOBUTTON(  ID_RADIO_BT_IN, pwFiltersDateDlg::OnRadiobuttonInSelected )
  EVT_BUTTON( wxID_CANCEL, pwFiltersDateDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersDateDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersDateDlg constructors
 */

pwFiltersDateDlg::pwFiltersDateDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, time_t *fdate1, time_t *fdate2, int *fnum1, int *fnum2, int *fdatetype)
: m_ftype(ftype), m_add_present(false), m_min(-1), m_max(-1),
  m_prule(rule), m_pfdate1(fdate1), m_pfdate2(fdate2), 
  m_pfnum1(fnum1), m_pfnum2(fnum2),
  m_pfdatetype(fdatetype)
{
  wxASSERT(!parent || parent->IsTopLevel());

  Init();

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter Date Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersDateDlg* pwFiltersDateDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, time_t *fdate1, time_t *fdate2, int *fnum1, int *fnum2, int *fdatetype)
{
  return new pwFiltersDateDlg(parent, ftype, rule, fdate1, fdate2, fnum1, fnum2, fdatetype);
}

/*!
 * InitDialog set selection list and optimize window size
 */

void pwFiltersDateDlg::InitDialog()
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
  
  for(size_t i = 0; i < PW_NUM_DATE_CRITERIA_ENUM; i++) {
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
        m_ExpDate1Ctrl->Disable();
        m_ExpDate2Ctrl->Disable();
        m_FNum1Ctrl->Disable();
        m_FNum2Ctrl->Disable();
        m_OnCtrl->Disable();
        m_InCtrl->Disable();
        break;
      }
    }
  }
  if(m_idx == -1) {
    for(int i = 0; i < PW_NUM_DATE_CRITERIA_ENUM; i++) {
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
  
  m_FNum1Ctrl->SetValue(m_fnum1);
  m_FNum2Ctrl->SetValue(m_fnum2);
  
  CheckControls();
}

/*!
 * SetValidators
 */

void pwFiltersDateDlg::SetValidators()
{
  m_ComboBox->SetValidator(wxGenericValidator(&m_idx));
  // Hint: Date validation is not running well in 3.0.5
  m_FNum1Ctrl->SetValidator(wxGenericValidator(&m_fnum1));
  m_FNum2Ctrl->SetValidator(wxGenericValidator(&m_fnum2));
}

/*!
 * Member initialisation
 */

void pwFiltersDateDlg::Init()
{
  switch (m_ftype) {
    case FT_CTIME:
    case FT_PMTIME:
    case FT_ATIME:
    case FT_XTIME:
    case FT_RMTIME:
      m_add_present = true;
      break;
    case HT_CHANGEDATE:
      m_add_present = false;
      break;
    case AT_CTIME:
    case AT_FILECTIME:
    case AT_FILEMTIME:
    case AT_FILEATIME:
      m_add_present = true;
      break;
    default:
      wxASSERT(false);
      m_add_present = true;
  }

  m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays);
  if(m_min >= 0) {
    m_min = -PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays);
  }
  m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays);
  
  m_fdate1 = *m_pfdate1;
  m_fdate2 = *m_pfdate2;
  m_fnum1 = *m_pfnum1;
  m_fnum2 = *m_pfnum2;
  m_fdatetype = *m_pfdatetype;

  if(m_fdatetype < PW_DATE_ABS || m_fdatetype > PW_DATE_REL)
    m_fdatetype = PW_DATE_ABS;
  if(! m_fdate1.IsValid() || m_fdate1.GetTicks() == static_cast<time_t>(0))
    m_fdate1 = wxDateTime::Now() - wxDateSpan(0, 0, 0, 1);
  if(! m_fdate2.IsValid() || m_fdate2.GetTicks() == static_cast<time_t>(0))
    m_fdate2 = wxDateTime::Now();
  if(m_fnum1 < m_min || m_fnum1 > m_max)
    m_fnum1 = PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::DefaultExpiryDays);
  if(m_fnum2 < m_min || m_fnum2 > m_max)
    m_fnum2 = PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::DefaultExpiryDays);
}

/*!
 * Control creation for pwFiltersDateDlg
 */

void pwFiltersDateDlg::CreateControls()
{
  //(*Initialize(pwFiltersDateDlg
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
  
  wxGridBagSizer *basicGridSizer = new wxGridBagSizer();
  staticBoxSizer1->Add(basicGridSizer, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP|wxALL, 0);
 
  // Rule at first row in the box (Choice)
  auto choiceStaticText1 = new wxStaticText(this, wxID_ANY, _("Rule") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(choiceStaticText1, wxGBPosition(/*row:*/ 0, /*column:*/ 0), wxDefaultSpan,  wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  m_ComboBox = new wxComboBox(this, ID_COMBOBOX64, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  basicGridSizer->Add(m_ComboBox, wxGBPosition(/*row:*/ 0, /*column:*/ 1), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3),  wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);
  
  // Value on next row (Date)
  m_OnCtrl = new wxRadioButton(this, ID_RADIO_BT_ON, _("Absolute") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(m_OnCtrl, wxGBPosition(/*row:*/ 1, /*column:*/ 0), wxDefaultSpan,  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_OnCtrl->SetValue(m_fdatetype == PW_DATE_ABS);
  
  m_ExpDate1Ctrl = new wxDatePickerCtrl(this, ID_DATECTRL65, m_fdate1, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
  basicGridSizer->Add(m_ExpDate1Ctrl, wxGBPosition(/*row:*/ 1, /*column:*/ 1), wxDefaultSpan,  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  auto andStaticText2 = new wxStaticText(this, wxID_ANY, _("and") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(andStaticText2, wxGBPosition(/*row:*/ 1, /*column:*/ 2), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  m_ExpDate2Ctrl = new wxDatePickerCtrl(this, ID_DATECTRL66, m_fdate2, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
  basicGridSizer->Add(m_ExpDate2Ctrl, wxGBPosition(/*row:*/ 1, /*column:*/ 3), wxDefaultSpan,  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  // Value on next row (Number explanation)
  auto andStaticText3 = new wxStaticText(this, wxID_ANY, _("Relative dates are in days relative to today e.g. '-1' is yesterday and '1' is tomorrow") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(andStaticText3, wxGBPosition(/*row:*/ 2, /*column:*/ 0), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 4),  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  // Value on next row (Absolute)
  m_InCtrl = new wxRadioButton(this, ID_RADIO_BT_IN, _("Relative") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(m_InCtrl, wxGBPosition(/*row:*/ 3, /*column:*/ 0), wxDefaultSpan,  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_OnCtrl->SetValue(m_fdatetype == PW_DATE_REL);
  
  wxString str;
  str = wxString::Format("%d", m_fnum1);
  m_FNum1Ctrl = new wxSpinCtrl(this, ID_SPINCTRL67, str, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_min, m_max, m_fnum1);
  basicGridSizer->Add(m_FNum1Ctrl, wxGBPosition(/*row:*/ 4, /*column:*/ 1), wxDefaultSpan,  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  auto andStaticText4 = new wxStaticText(this, wxID_ANY, _("and") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(andStaticText4, wxGBPosition(/*row:*/ 4, /*column:*/ 2), wxDefaultSpan,  wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  str = wxString::Format("%d", m_fnum2);
  m_FNum2Ctrl = new wxSpinCtrl(this, ID_SPINCTRL68, str, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_min, m_max, m_fnum2);
  basicGridSizer->Add(m_FNum2Ctrl, wxGBPosition(/*row:*/ 4, /*column:*/ 3), wxDefaultSpan,  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
  // Value on next row (Number range)
  str = wxString::Format(_("Interval [%d to %d]").c_str(), m_min, m_max);
  auto andStaticText5 = new wxStaticText(this, wxID_ANY, str + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  basicGridSizer->Add(andStaticText5, wxGBPosition(/*row:*/ 5, /*column:*/ 1), wxGBSpan(/*rowspan:*/ 1, /*columnspan:*/ 3),  wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
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

bool pwFiltersDateDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersDateDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersDateDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersDateDlg::OnSelectionChange(wxCommandEvent& WXUNUSED(event))
{
  m_idx = m_ComboBox->GetSelection();

  CheckControls();
}

/*!
 * CheckControls Disable or Enable controls depending on actual values
 */

void pwFiltersDateDlg::CheckControls()
{
  if(m_idx != -1)
    FindWindow(wxID_OK)->Enable();
  else
    FindWindow(wxID_OK)->Disable();
  if(m_add_present) {
    if((m_idx >= 0) && (m_idx < PW_NUM_PRESENT_ENUM)) {
      m_ExpDate1Ctrl->Disable();
      m_ExpDate2Ctrl->Disable();
      m_FNum1Ctrl->Disable();
      m_FNum2Ctrl->Disable();
      m_OnCtrl->Disable();
      m_InCtrl->Disable();
    }
    else {
      m_OnCtrl->Enable();
      m_InCtrl->Enable();
      if(m_fdatetype == PW_DATE_ABS) {
        m_ExpDate1Ctrl->Enable();
        m_FNum1Ctrl->Disable();
        m_OnCtrl->SetValue(true);
        m_InCtrl->SetValue(false);
      }
      else {
        m_ExpDate1Ctrl->Disable();
        m_FNum1Ctrl->Enable();
        m_OnCtrl->SetValue(false);
        m_InCtrl->SetValue(true);
      }
      if((m_idx >= PW_NUM_PRESENT_ENUM) && ((m_idx - PW_NUM_PRESENT_ENUM) < PW_NUM_DATE_CRITERIA_ENUM)) {
        if(m_mrcrit[m_idx - PW_NUM_PRESENT_ENUM] == PWSMatch::MR_BETWEEN) {
          if(m_fdatetype == PW_DATE_ABS) {
            m_ExpDate2Ctrl->Enable();
            m_FNum2Ctrl->Disable();
          }
          else {
            m_ExpDate2Ctrl->Disable();
            m_FNum2Ctrl->Enable();
          }
        }
        else {
          m_ExpDate2Ctrl->Disable();
          m_FNum2Ctrl->Disable();
        }
      }
      else {
        m_ExpDate2Ctrl->Disable();
        m_FNum2Ctrl->Disable();
      }
    }
  }
  else {
    if(m_fdatetype == PW_DATE_ABS) {
      m_ExpDate1Ctrl->Enable();
      m_FNum1Ctrl->Disable();
      m_OnCtrl->SetValue(true);
      m_InCtrl->SetValue(false);
    }
    else {
      m_ExpDate1Ctrl->Disable();
      m_FNum1Ctrl->Enable();
      m_OnCtrl->SetValue(false);
      m_InCtrl->SetValue(true);
    }
    if((m_idx >= 0) && (m_idx < PW_NUM_DATE_CRITERIA_ENUM)) {
      if(m_mrcrit[m_idx] == PWSMatch::MR_BETWEEN) {
        if(m_fdatetype == PW_DATE_ABS) {
          m_ExpDate2Ctrl->Enable();
          m_FNum2Ctrl->Disable();
        }
        else {
          m_ExpDate2Ctrl->Disable();
          m_FNum2Ctrl->Enable();
        }
      }
      else {
        m_ExpDate2Ctrl->Disable();
        m_FNum2Ctrl->Disable();
      }
    }
    else {
      m_ExpDate2Ctrl->Disable();
      m_FNum2Ctrl->Disable();
    }
  }
}

/*!
 * isRuleSelected Check if given rule is the selected one
 */

bool pwFiltersDateDlg::isRuleSelected(int idx, PWSMatch::MatchRule rule) const
{
  return (idx >= 0) &&
         ((m_add_present && (idx < (PW_NUM_PRESENT_ENUM + PW_NUM_DATE_CRITERIA_ENUM)) && (idx >= PW_NUM_PRESENT_ENUM) && (m_mrcrit[idx - PW_NUM_PRESENT_ENUM] == rule)) ||
          (! m_add_present && (idx < PW_NUM_DATE_CRITERIA_ENUM) && (m_mrcrit[idx] == rule)));
}

/*!
 * wxEVT_SPIN event handler for wxSpinCtrl
 */

void pwFiltersDateDlg::OnFNum1Change(wxSpinEvent& WXUNUSED(event))
{
  m_fnum1 = m_FNum1Ctrl->GetValue();
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

void pwFiltersDateDlg::OnFNum2Change(wxSpinEvent& WXUNUSED(event))
{
  m_fnum2 = m_FNum2Ctrl->GetValue();
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

void pwFiltersDateDlg::OnExpDate1Changed(wxDateEvent& event)
{
  m_fdate1 = m_ExpDate1Ctrl->GetValue();
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

void pwFiltersDateDlg::OnExpDate2Changed(wxDateEvent& event)
{
  m_fdate2 = m_ExpDate2Ctrl->GetValue();
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_ON
 */

void pwFiltersDateDlg::OnRadiobuttonOnSelected(wxCommandEvent& WXUNUSED(event))
{
  m_fdatetype = PW_DATE_ABS;
  m_OnCtrl->SetValue(true);
  m_InCtrl->SetValue(false);
  
  CheckControls();
}

void pwFiltersDateDlg::OnRadiobuttonInSelected(wxCommandEvent& WXUNUSED(event))
{
  m_fdatetype = PW_DATE_REL;
  m_OnCtrl->SetValue(false);
  m_InCtrl->SetValue(true);
  
  CheckControls();
}

bool pwFiltersDateDlg::IsValid(bool showMessage) const {
  const auto idx = m_ComboBox->GetSelection();
  const auto fnum1 = m_FNum1Ctrl->GetValue();
  const auto fnum2 = m_FNum2Ctrl->GetValue();
  const auto fdate1 = m_ExpDate1Ctrl->GetValue();
  const auto fdate2 = m_ExpDate2Ctrl->GetValue();

  // CheckBetween Check if given rule is the BETWEEN and if both numbers are in right order or lower value is below maximum
  if (isRuleSelected(idx, PWSMatch::MR_BETWEEN)) {
    if (m_fdatetype == PW_DATE_REL) {
      if (fnum1 >= fnum2) {
        if(showMessage) {
          wxMessageBox(_("Correct numeric values."), _("Second number must be greater than first"), wxOK|wxICON_ERROR);
        }
        return false;
      }
      else if(fnum1 == m_max) {
        if (showMessage) {
          wxMessageBox(_("Correct numeric values."), _("Maximum value for first number and 'Between' rule"), wxOK|wxICON_ERROR);
        }
        return false;
      }
    }
    else {
      if (fdate1 >= fdate2) {
        if (showMessage) {
          wxMessageBox(_("Please correct date values."), _("Second date must be later than first"), wxOK|wxICON_ERROR);
        }
        return false;
      }
    }
  }

  if (m_fdatetype == PW_DATE_REL) {
    if (isRuleSelected(idx, PWSMatch::MR_BEFORE) && (fnum1 == m_min)) {
      if (showMessage) {
        wxMessageBox(_("Correct numeric values."), _("Number is set to the minimum value. 'Less than' is not allowed."), wxOK|wxICON_ERROR);
      }
      return false;
    }

    if (isRuleSelected(idx, PWSMatch::MR_AFTER) && (fnum1 == m_max)) {
      if (showMessage) {
        wxMessageBox(_("Correct numeric values."), _("Number is set to the maximum value. 'Greater than' is not allowed."), wxOK|wxICON_ERROR);
      }
      return false;
    }
    if (m_ftype != FT_XTIME) {
      // All dates, except History must be located in the history
      if ((fnum1 > 0) || (isRuleSelected(idx, PWSMatch::MR_BETWEEN) && (fnum2 > 0))) {
        if (showMessage) {
          wxMessageBox(_("Correct numeric values."), _("A future date is not valid for this field"), wxOK|wxICON_ERROR);
        }
        return false;
      }
    }
  }
  else if (m_ftype != FT_XTIME) {
    // All dates, except History must be located in the history
    if ((fdate1 > wxDateTime::Now()) || (isRuleSelected(idx, PWSMatch::MR_BETWEEN) && (fdate2 > wxDateTime::Now()))) {
      if (showMessage) {
        wxMessageBox(_("Correct date values."), _("A future date is not valid for this field"), wxOK|wxICON_ERROR);
      }
      return false;
    }
  }
  return true;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersDateDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    m_idx = m_ComboBox->GetSelection();
 
    if(m_idx < 0) {
      EndModal(wxID_CANCEL);
      return;
    }

    if (!IsValid(true)) {
      return;
    }
    
    m_fdate1 = m_ExpDate1Ctrl->GetValue();
    m_fdate2 = m_ExpDate2Ctrl->GetValue();
    m_fnum1 = m_FNum1Ctrl->GetValue();
    m_fnum2 = m_FNum2Ctrl->GetValue();

    if(m_add_present) {
      if((m_idx >= 0) && (m_idx < PW_NUM_PRESENT_ENUM)) {
        *m_prule = m_mrpres[m_idx];
      }
      else {
        m_idx -= PW_NUM_PRESENT_ENUM;
        if(m_idx < PW_NUM_DATE_CRITERIA_ENUM)
          *m_prule = m_mrcrit[m_idx];
        else
          *m_prule = PWSMatch::MR_INVALID;
        m_idx += PW_NUM_PRESENT_ENUM; // Is used later in isRuleSelected()
      }
    }
    else {
      if(m_idx < PW_NUM_DATE_CRITERIA_ENUM)
        *m_prule = m_mrcrit[m_idx];
      else
        *m_prule = PWSMatch::MR_INVALID;
    }
    
    *m_pfdatetype = m_fdatetype;

    if(*m_prule == PWSMatch::MR_PRESENT || *m_prule == PWSMatch::MR_NOTPRESENT) {
      *m_pfdate1 = static_cast<time_t>(0);
      *m_pfdate2 = static_cast<time_t>(0);
      *m_pfnum1 = 0;
      *m_pfnum2 = 0;
    }
    else {
      if(m_fdatetype == PW_DATE_REL) {
        *m_pfnum1 = m_fnum1;
        if(isRuleSelected(m_idx, PWSMatch::MR_BETWEEN))
          *m_pfnum2 = m_fnum2;
        else
          *m_pfnum2 = 0;
        *m_pfdate1 = static_cast<time_t>(0);
        *m_pfdate2 = static_cast<time_t>(0);
      }
      else {
        *m_pfdate1 = m_fdate1.GetTicks();
        if(isRuleSelected(m_idx, PWSMatch::MR_BETWEEN))
          *m_pfdate2 = m_fdate2.GetTicks();
        else
          *m_pfdate2 = static_cast<time_t>(0);
        *m_pfnum1 = 0;
        *m_pfnum2 = 0;
      }
    }
  }
  EndModal(wxID_OK);
}

bool pwFiltersDateDlg::IsChanged() const {
  const auto idx = m_ComboBox->GetSelection();
  if (idx < 0) {
    return false;
  }

  if (!IsValid(false)) {
    return true;
  }

  if (m_add_present) {
    if (idx >= 0 && idx < PW_NUM_PRESENT_ENUM) {
      if (*m_prule != m_mrpres[idx]) {
        return true;
      }
    }
    else {
      const auto tmpIdx = idx - PW_NUM_PRESENT_ENUM;
      if (tmpIdx < PW_NUM_DATE_CRITERIA_ENUM) {
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
    if (idx < PW_NUM_DATE_CRITERIA_ENUM) {
      if (*m_prule != m_mrcrit[idx]) {
        return true;
      }
    }
    else if (*m_prule != PWSMatch::MR_INVALID) {
      return true;
    }
  }

  if (*m_pfdatetype != m_fdatetype) {
    return true;
  }

  if (*m_prule != PWSMatch::MR_PRESENT && *m_prule != PWSMatch::MR_NOTPRESENT) {
    const auto fnum1 = m_FNum1Ctrl->GetValue();
    const auto fnum2 = m_FNum2Ctrl->GetValue();

    if (m_fdatetype == PW_DATE_REL) {
      if (*m_pfnum1 != fnum1) {
        return true;
      }
      if (isRuleSelected(idx, PWSMatch::MR_BETWEEN)) {
        if (*m_pfnum2 != fnum2) {
          return true;
        }
      }
    }
    else {
      const auto fdate1 = m_ExpDate1Ctrl->GetValue();
      const auto fdate2 = m_ExpDate2Ctrl->GetValue();

      if (*m_pfdate1 != fdate1.GetTicks()) {
        return true;
      }
      if (isRuleSelected(idx, PWSMatch::MR_BETWEEN)) {
        if (*m_pfdate2 != fdate2.GetTicks()) {
          return true;
        }
      }
    }
  }
  return false;
}
