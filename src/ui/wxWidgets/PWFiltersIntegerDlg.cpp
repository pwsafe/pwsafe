/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersIntegerDlg.cpp
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

#include <limits.h>
#include <vector>

#include "core/PWSFilters.h"
#include "PWFiltersIntegerDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"

//(*IdInit(pwFiltersIntegerDlg)
const PWSMatch::MatchRule pwFiltersIntegerDlg::m_mrpres[PW_NUM_PRESENT_ENUM] = {
                           PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT};
const PWSMatch::MatchRule pwFiltersIntegerDlg::m_mrcrit[PW_NUM_INT_CRITERIA_ENUM] = {
                           PWSMatch::MR_EQUALS, PWSMatch::MR_NOTEQUAL,
                           PWSMatch::MR_LT,     PWSMatch::MR_LE,
                           PWSMatch::MR_GT,     PWSMatch::MR_GE,
                           PWSMatch::MR_BETWEEN };
//*)

enum { PW_UNIT_BYTES = 0, PW_UNIT_KBYTES, PW_UNIT_MBYTES, PW_NUM_UNITS, PW_MAX_UNITS = PW_UNIT_MBYTES, PW_MIN_UNITS = PW_UNIT_BYTES };

static std::vector<int> entrySizeMmax = {
#if INT_MAX == 32768
  // In case we are running on a 32 bit machine
  INT_MAX, INT_MAX, 999
#else
  999999999, 999999, 999
#endif
};

/*!
 * pwFiltersIntegerDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersIntegerDlg, wxDialog )

/*!
 * pwFiltersIntegerDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersIntegerDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersIntegerDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX57, pwFiltersIntegerDlg::OnSelectionChange )
  EVT_SPINCTRL( ID_SPINCTRL58, pwFiltersIntegerDlg::OnFNum1Change )
  EVT_SPINCTRL( ID_SPINCTRL59, pwFiltersIntegerDlg::OnFNum2Change )
  EVT_RADIOBUTTON(  ID_RADIO_BT_BY, pwFiltersIntegerDlg::OnByteRadiobuttonSelected )
  EVT_RADIOBUTTON(  ID_RADIO_BT_KB, pwFiltersIntegerDlg::OnKiloByteRadiobuttonSelected )
  EVT_RADIOBUTTON(  ID_RADIO_BT_MB, pwFiltersIntegerDlg::OnMegaByteRadiobuttonSelected )
  EVT_BUTTON( wxID_CANCEL, pwFiltersIntegerDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersIntegerDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersIntegerDlg constructors
 */

pwFiltersIntegerDlg::pwFiltersIntegerDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, int *fnum1, int *fnum2, int *funit)
: m_ftype(ftype), m_prule(rule), m_pfnum1(fnum1), m_pfnum2(fnum2), m_pfunit(funit)
{
  wxASSERT(!parent || parent->IsTopLevel());

  Init();

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter Integer Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersIntegerDlg* pwFiltersIntegerDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule, int *fnum1, int *fnum2, int *funit)
{
  return new pwFiltersIntegerDlg(parent, ftype, rule, fnum1, fnum2, funit);
}

/*!
 * InitDialog set selection in choice list and optimize window size
 */

void pwFiltersIntegerDlg::InitDialog()
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
  
  for(size_t i = 0; i < PW_NUM_INT_CRITERIA_ENUM; i++) {
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
        m_FNum1Ctrl->Disable();
        m_FNum2Ctrl->Disable();
        break;
      }
    }
  }
  if(m_idx == -1) {
    for(int i = 0; i < PW_NUM_INT_CRITERIA_ENUM; i++) {
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
  
  if(m_ftype == FT_ENTRYSIZE) {
    m_FNum2Ctrl->Enable();
  }
  CheckControls();
}

/*!
 * SetValidators
 */

void pwFiltersIntegerDlg::SetValidators()
{
  m_ComboBox->SetValidator(wxGenericValidator(&m_idx));
}

/*!
 * Member initialisation
 */

void pwFiltersIntegerDlg::Init()
{
  switch (m_ftype) {
    case FT_PASSWORDLEN:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDefaultLength);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDefaultLength);
      break;
    case FT_XTIME_INT:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = true;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays);
      break;
    case HT_NUM:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = 0;
      m_max = INT_MAX;
      break;
    case HT_MAX:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = 0;
      m_max = 255; // One byte hex
      break;
    case PT_LENGTH:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDefaultLength);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDefaultLength);
      break;
    case PT_LOWERCASE:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWLowercaseMinLength);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWLowercaseMinLength);
      break;
    case PT_UPPERCASE:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWUppercaseMinLength);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWUppercaseMinLength);
      break;
    case PT_DIGITS:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDigitMinLength);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDigitMinLength);
      break;
    case PT_SYMBOLS:
      wxASSERT(m_pfunit == nullptr);
      m_add_present = false;
      m_min = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWSymbolMinLength);
      m_max = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWSymbolMinLength);
      break;
    case FT_ENTRYSIZE:
      wxASSERT(m_pfunit);
      m_add_present = false;
      m_min = 0;
      m_max = INT_MAX;
      break;
    default:
      wxASSERT(false);
      m_add_present = true;
      m_min = INT_MIN;
      m_max = INT_MAX;
  }
  
  m_fnum1 = *m_pfnum1;
  m_fnum2 = *m_pfnum2;
  if(m_pfunit) {
    m_funit = *m_pfunit;
  }
  // Setup unit
  if(m_funit < PW_MIN_UNITS || m_funit > PW_MAX_UNITS) {
    m_funit = PW_UNIT_BYTES;
  }
  if(m_ftype == FT_ENTRYSIZE) {
    // Set max. size values (~1GB)
    m_max = entrySizeMmax[m_funit];
    
    m_fnum1 >>= (m_funit * 10);
    m_fnum2 >>= (m_funit * 10);
  }
  
  infoFmtStr = _("Interval [%d to %d]");
}

/*!
 * Control creation for pwFiltersIntegerDlg
 */

void pwFiltersIntegerDlg::CreateControls()
{
  //(*Initialize(pwFiltersIntegerDlg
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
  
  // Rule at first row in the box
  auto choiceStaticText1 = new wxStaticText(this, wxID_ANY, _("Rule") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(choiceStaticText1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  m_ComboBox = new wxComboBox(this, ID_COMBOBOX57, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  itemFlexGridSizer1->Add(m_ComboBox, 1, wxALL|wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxEXPAND, 5);
  
  // Value on next row
  auto valueStaticText2 = new wxStaticText(this, wxID_ANY, _("Value") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer1->Add(valueStaticText2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  
  auto itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer1->Add(itemBoxSizer2, 1, wxALIGN_LEFT|wxALL, 5);
  
  wxString str;
  str = wxString::Format("%d", m_fnum1);
  m_FNum1Ctrl = new wxSpinCtrl(this, ID_SPINCTRL58, str, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_min, m_max, m_fnum1);
  itemBoxSizer2->Add(m_FNum1Ctrl, 1, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);
    
  auto andStaticText3 = new wxStaticText(this, wxID_ANY, _("and") + _T(" "), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer2->Add(andStaticText3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  
  str = wxString::Format("%d", m_fnum2);
  m_FNum2Ctrl = new wxSpinCtrl(this, ID_SPINCTRL59, str, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, m_min, m_max, m_fnum2);
  itemBoxSizer2->Add(m_FNum2Ctrl, 1, wxALIGN_LEFT/*|wxALIGN_CENTER_VERTICAL*/|wxALL|wxEXPAND, 5);

  // Only Entry Size is needing units
  if(m_pfunit) {
    itemFlexGridSizer1->AddSpacer(5);
    
    auto itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer3, 1, wxALIGN_LEFT|wxALL, 5);
  
    m_UnitByteCtrl = new wxRadioButton(this, ID_RADIO_BT_BY, _("Bytes"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_UnitByteCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_UnitByteCtrl->SetValue(m_funit == PW_UNIT_BYTES);
  
    itemBoxSizer3->AddStretchSpacer();
  
    m_UnitKByteCtrl = new wxRadioButton(this, ID_RADIO_BT_KB, _("KBytes"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_UnitKByteCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_UnitKByteCtrl->SetValue(m_funit == PW_UNIT_KBYTES);
  
    itemBoxSizer3->AddStretchSpacer();
  
    m_UnitMByteCtrl = new wxRadioButton(this, ID_RADIO_BT_MB, _("MBytes"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_UnitMByteCtrl, 0, /*wxALIGN_RIGHT |*/ wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_UnitMByteCtrl->SetValue(m_funit == PW_UNIT_MBYTES);
  }
  else {
    m_UnitByteCtrl = nullptr;
    m_UnitKByteCtrl = nullptr;
    m_UnitMByteCtrl = nullptr;
  }
  
  // Next Row
  itemFlexGridSizer1->AddSpacer(5);
  
  wxScreenDC dc;
  dc.SetFont(choiceStaticText1->GetFont());
  str = wxString::Format(infoFmtStr.c_str(), INT_MIN, INT_MAX);
  wxCoord width, height;
  dc.GetTextExtent(str, &width, &height);
  wxSize size(width+6, height);
  
  str = wxString::Format(infoFmtStr.c_str(), m_min, m_max);
  m_IntervalTextCtrl = new wxStaticText(this, wxID_ANY, str + _T(" "), wxDefaultPosition, size, 0);
  itemFlexGridSizer1->Add(m_IntervalTextCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
  
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

bool pwFiltersIntegerDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersIntegerDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersIntegerDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersIntegerDlg::OnSelectionChange(wxCommandEvent& WXUNUSED(event))
{
  m_idx = m_ComboBox->GetSelection();

  CheckControls();
}

/*!
 * CheckControls Disable or Enable controls depending on actual values
 */

void pwFiltersIntegerDlg::CheckControls()
{
  if(m_idx != -1)
    FindWindow(wxID_OK)->Enable();
  else
    FindWindow(wxID_OK)->Disable();
  if(m_add_present) {
    if((m_idx >= 0) && (m_idx < PW_NUM_PRESENT_ENUM)) {
      m_FNum1Ctrl->Disable();
      m_FNum2Ctrl->Disable();
    }
    else {
      m_FNum1Ctrl->Enable();
      if((m_idx >= PW_NUM_PRESENT_ENUM) && ((m_idx - PW_NUM_PRESENT_ENUM) < PW_NUM_INT_CRITERIA_ENUM)) {
        if(m_mrcrit[m_idx - PW_NUM_PRESENT_ENUM] == PWSMatch::MR_BETWEEN)
          m_FNum2Ctrl->Enable();
        else
          m_FNum2Ctrl->Disable();
      }
      else {
        m_FNum2Ctrl->Disable();
      }
    }
  }
  else {
    if((m_idx >= 0) && (m_idx < PW_NUM_INT_CRITERIA_ENUM)) {
      if(m_mrcrit[m_idx] == PWSMatch::MR_BETWEEN)
        m_FNum2Ctrl->Enable();
      else
        m_FNum2Ctrl->Disable();
    }
    else {
      m_FNum2Ctrl->Disable();
    }
  }
}

/*!
 * isRuleSelected Check if given rule is the selected one
 */

bool pwFiltersIntegerDlg::isRuleSelected(int idx, PWSMatch::MatchRule rule) const
{
  return (idx >= 0) &&
         ((m_add_present && (idx < (PW_NUM_PRESENT_ENUM + PW_NUM_INT_CRITERIA_ENUM)) && (idx >= PW_NUM_PRESENT_ENUM) && (m_mrcrit[idx - PW_NUM_PRESENT_ENUM] == rule)) ||
          (! m_add_present && (idx < PW_NUM_INT_CRITERIA_ENUM) && (m_mrcrit[idx] == rule)));
}

bool pwFiltersIntegerDlg::IsValid(bool showMessage) const
{
  const auto idx = m_ComboBox->GetSelection();

  const auto fnum1 = m_FNum1Ctrl->GetValue();
  const auto fnum2 = m_FNum2Ctrl->GetValue();

  if (isRuleSelected(idx, PWSMatch::MR_BETWEEN)) {
    if (fnum1 >= fnum2) {
      if (showMessage) {
        wxMessageBox(_("Please correct numeric values."), _("Second number must be greater than first"), wxOK|wxICON_ERROR);
      }
      return false;
    }
    else if (fnum1 == m_max) {
      if (showMessage) {
        wxMessageBox(_("Correct numeric values."), _("Maximum value for first number and 'Between' rule"), wxOK|wxICON_ERROR);
      }
      return false;
    }
  }
  else if(isRuleSelected(idx, PWSMatch::MR_LT) && (fnum1 == m_min)) {
    if (showMessage) {
      wxMessageBox(_("Correct numeric values."), _("Number is set to the minimum value. 'Less than' is not allowed."), wxOK|wxICON_ERROR);
    }
    return false;
  }
  else if(isRuleSelected(idx, PWSMatch::MR_GT) && (fnum1 == m_max)) {
    if (showMessage) {
      wxMessageBox(_("Correct numeric values."), _("Number is set to the maximum value. 'Greater than' is not allowed."), wxOK|wxICON_ERROR);
    }
    return false;
  }
  return true;
}

/*!
 * wxEVT_SPIN event handler for wxSpinCtrl
 * Called when number is changed
 */

void pwFiltersIntegerDlg::OnFNum1Change(wxSpinEvent& WXUNUSED(event))
{
  m_fnum1 = m_FNum1Ctrl->GetValue();
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

void pwFiltersIntegerDlg::OnFNum2Change(wxSpinEvent& WXUNUSED(event))
{
  m_fnum2 = m_FNum2Ctrl->GetValue();
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_ON
 */

void pwFiltersIntegerDlg::OnByteRadiobuttonSelected(wxCommandEvent& WXUNUSED(event))
{
  m_funit = PW_UNIT_BYTES;
  
  UpdateUnitSelection();
}

void pwFiltersIntegerDlg::OnKiloByteRadiobuttonSelected(wxCommandEvent& WXUNUSED(event))
{
  m_funit = PW_UNIT_KBYTES;
  
  UpdateUnitSelection();
}

void pwFiltersIntegerDlg::OnMegaByteRadiobuttonSelected(wxCommandEvent& WXUNUSED(event))
{
  m_funit = PW_UNIT_MBYTES;
  
  UpdateUnitSelection();
}

/*!
 * UpdateUnitSelection Set the radio buttons depending from current selection
 * On reduction reduce number to fit into range
 */

void pwFiltersIntegerDlg::UpdateUnitSelection()
{
  m_UnitByteCtrl->SetValue(m_funit == PW_UNIT_BYTES);
  m_UnitKByteCtrl->SetValue(m_funit == PW_UNIT_KBYTES);
  m_UnitMByteCtrl->SetValue(m_funit == PW_UNIT_MBYTES);
  m_max = entrySizeMmax[m_funit];
  
  // On too big values reduce by units
  if(m_fnum1 > m_max)
    m_fnum1 >>= 10;
  if(m_fnum1 > m_max)
    m_fnum1 >>= 10;
  
  if(m_fnum2 > m_max)
    m_fnum2 >>= 10;
  if(m_fnum2 > m_max)
    m_fnum2 >>= 10;
  
  // Set new range
  m_FNum1Ctrl->SetRange(m_min, m_max);
  m_FNum2Ctrl->SetRange(m_min, m_max);
  
  // Update is done silently, when needed. So fetch new values and check.
  m_fnum1 = m_FNum1Ctrl->GetValue();
  m_fnum2 = m_FNum2Ctrl->GetValue();
  
  wxString value = wxString::Format(infoFmtStr.c_str(), m_min, m_max);
  m_IntervalTextCtrl->SetLabel(value);
  
  // Do not check here, only after OK button pressed
  // (void) CheckBetween(true);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersIntegerDlg::OnOk(wxCommandEvent& WXUNUSED(event))
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
    
    m_fnum1 = m_FNum1Ctrl->GetValue();
    m_fnum2 = m_FNum2Ctrl->GetValue();

    if(m_add_present) {
      if((m_idx >= 0) && (m_idx < PW_NUM_PRESENT_ENUM)) {
        *m_prule = m_mrpres[m_idx];
      }
      else {
        m_idx -= PW_NUM_PRESENT_ENUM;
        if(m_idx < PW_NUM_INT_CRITERIA_ENUM)
          *m_prule = m_mrcrit[m_idx];
        else
          *m_prule = PWSMatch::MR_INVALID;
      }
    }
    else {
      if(m_idx < PW_NUM_INT_CRITERIA_ENUM)
        *m_prule = m_mrcrit[m_idx];
      else
        *m_prule = PWSMatch::MR_INVALID;
    }
    // Calculate size when entry size
    if(m_ftype == FT_ENTRYSIZE) {
      m_fnum1 <<= (m_funit * 10);
      m_fnum2 <<= (m_funit * 10);
    }

    *m_pfnum1 = m_fnum1;
    *m_pfnum2 = m_fnum2;
    if(m_pfunit)
      *m_pfunit = m_funit;
  }
  EndModal(wxID_OK);
}

bool pwFiltersIntegerDlg::IsChanged() const {
  const auto idx = m_ComboBox->GetSelection();

  if (idx < 0) {
    return false;
  }

  if (!IsValid(false)) {
    return true;
  }

  auto fnum1 = m_FNum1Ctrl->GetValue();
  auto fnum2 = m_FNum2Ctrl->GetValue();

  if (m_add_present) {
    if(idx >= 0 && idx < PW_NUM_PRESENT_ENUM) {
      if (*m_prule != m_mrpres[idx]) {
        return true;
      }
    }
    else {
      const auto tmpIdx = idx - PW_NUM_PRESENT_ENUM;
      if (tmpIdx < PW_NUM_INT_CRITERIA_ENUM) {
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
    if (idx < PW_NUM_INT_CRITERIA_ENUM) {
      if (*m_prule != m_mrcrit[idx]) {
        return true;
      }
    }
    else if (*m_prule != PWSMatch::MR_INVALID) {
      return true;
    }
  }

  if (m_pfunit && *m_pfunit != m_funit) {
    return true;
  }

  // Calculate size when entry size
  if (m_ftype == FT_ENTRYSIZE) {
    fnum1 <<= (m_funit * 10);
    fnum2 <<= (m_funit * 10);
  }

  if (*m_pfnum1 != fnum1 || *m_pfnum2 != fnum2) {
    return true;
  }

  return false;
}
