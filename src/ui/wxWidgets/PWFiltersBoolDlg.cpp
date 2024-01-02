/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwFiltersBoolDlg.cpp
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
#include "PWFiltersBoolDlg.h"
#include "PWFiltersEditor.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"


//(*IdInit(pwFiltersBoolDlg)
const PWSMatch::MatchRule pwFiltersBoolDlg::m_mrxp[PW_NUM_BOOL_ENUM] = {PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT};
const PWSMatch::MatchRule pwFiltersBoolDlg::m_mrxa[PW_NUM_BOOL_ENUM] = {PWSMatch::MR_ACTIVE,  PWSMatch::MR_INACTIVE};
const PWSMatch::MatchRule pwFiltersBoolDlg::m_mrxs[PW_NUM_BOOL_ENUM] = {PWSMatch::MR_SET,     PWSMatch::MR_NOTSET};
const PWSMatch::MatchRule pwFiltersBoolDlg::m_mrxi[PW_NUM_BOOL_ENUM] = {PWSMatch::MR_IS,      PWSMatch::MR_ISNOT};
//*)

/*!
 * pwFiltersBoolDlg type definition
 */

IMPLEMENT_CLASS( pwFiltersBoolDlg, wxDialog )

/*!
 * pwFiltersBoolDlg event table definition
 */

BEGIN_EVENT_TABLE( pwFiltersBoolDlg, wxDialog )

  EVT_BUTTON( wxID_OK, pwFiltersBoolDlg::OnOk )
  EVT_COMBOBOX( ID_COMBOBOX53, pwFiltersBoolDlg::OnSelectionChange )
  EVT_BUTTON( wxID_CANCEL, pwFiltersBoolDlg::OnCancelClick )
  EVT_CLOSE( pwFiltersBoolDlg::OnClose )

END_EVENT_TABLE()

/*!
 * pwFiltersBoolDlg constructors
 */

pwFiltersBoolDlg::pwFiltersBoolDlg(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule)
: m_ftype(ftype), m_prule(rule)
{
  wxASSERT(!parent || parent->IsTopLevel());

  m_btype = ConvertType(m_ftype);
  
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, wxID_ANY, _("Display Filter Boolean Value"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

  CreateControls();

  // Allow to resize the dialog in width, only.
  SetMaxSize(wxSize(wxDefaultCoord, GetMinSize().y));
  Centre();

  SetValidators();
}

pwFiltersBoolDlg* pwFiltersBoolDlg::Create(wxWindow *parent, FieldType ftype, PWSMatch::MatchRule *rule)
{
  return new pwFiltersBoolDlg(parent, ftype, rule);
}

/*!
 * InitDialog select bool type depending selection list and optimize window size
 */

void pwFiltersBoolDlg::InitDialog()
{
  wxSize actSize = m_ComboBoxBool->GetSize(),
         oldSize = actSize,
         borderSize = m_ComboBoxBool->GetWindowBorderSize();
  wxScreenDC dc;
  wxCoord width, height, border = (borderSize.GetWidth() * 2) + 2;
  
  dc.SetFont(m_ComboBoxBool->GetFont());
  
  switch (m_btype) {
    case BT_PRESENT:
      m_pmrx = m_mrxp;
      break;
    case BT_ACTIVE:
      m_pmrx = m_mrxa;
      break;
    case BT_SET:
      m_pmrx = m_mrxs;
      break;
    case BT_IS:
      m_pmrx = m_mrxi;
      break;
    default:
      wxASSERT(false);
      m_pmrx = m_mrxi;
  }
  
  // Determine needed size for the bollean choice
  if(! m_ComboBoxBool->GetCount()) {
    for(size_t i = 0; i < PW_NUM_BOOL_ENUM; i++) {
      int iumsg = PWSMatch::GetRule(m_pmrx[i]);
      wxString value = LoadAString(iumsg);
      m_ComboBoxBool->Append(value);
      // Set column size to fit
      dc.GetTextExtent(value, &width, &height);
      width += border; // Align
      if(width > actSize.GetWidth())
        actSize.SetWidth(width + 6);
    }
  }
  
  // Determine actual index
  wxASSERT(PW_NUM_BOOL_ENUM == 2);
  if(*m_prule == m_pmrx[0])
    m_idx = 0;
  else if(*m_prule == m_pmrx[1])
    m_idx = 1;
  else {
    m_idx = -1;
    FindWindow(wxID_OK)->Disable();
  }
  
  m_ComboBoxBool->SetSelection(m_idx);

  // Resize window if too small to include full choice string
  if(actSize.GetWidth() != oldSize.GetWidth()) {
    GetSize(&width, &height);
    width += actSize.GetWidth() - oldSize.GetWidth() + 6;
    int displayWidth, displayHight;
    ::wxDisplaySize(&displayWidth, &displayHight);
    if(width > displayWidth) width = displayWidth;
    SetSize(width, height);
  }
}

/*!
 * SetValidators
 */

void pwFiltersBoolDlg::SetValidators()
{
  m_ComboBoxBool->SetValidator(wxGenericValidator(&m_idx));
}

/*!
 * Member initialisation
 */

pwFiltersBoolDlg::BoolType pwFiltersBoolDlg::ConvertType(FieldType ftype)
{
  switch (ftype) {
    case FT_PROTECTED:
      return BT_IS;
    case FT_KBSHORTCUT:
      return BT_PRESENT;
    case FT_UNKNOWNFIELDS:
      return BT_PRESENT;
    case HT_PRESENT:
      return BT_PRESENT;
    case HT_ACTIVE:
      return BT_ACTIVE;
    case PT_PRESENT:
      return BT_PRESENT;
    case PT_EASYVISION:
    case PT_PRONOUNCEABLE:
    case PT_HEXADECIMAL:
      return BT_SET;
    case AT_PRESENT:
      return BT_PRESENT;
    default:
      wxASSERT(false);
      return BT_IS;
  }
}

/*!
 * Control creation for pwFiltersBoolDlg
 */

void pwFiltersBoolDlg::CreateControls()
{
  //(*Initialize(pwFiltersBoolDlg
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
  
  auto choiceStaticText1 = new wxStaticText(this, wxID_ANY, _("Rule") + _T(": "), wxDefaultPosition, wxDefaultSize, 0);
  staticBoxSizer1->Add(choiceStaticText1, 0, wxALL/*|wxALIGN_RIGHT*/|wxALIGN_CENTER_VERTICAL, 5);
  
  m_ComboBoxBool = new wxComboBox(this, ID_COMBOBOX53, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN|wxCB_READONLY, wxDefaultValidator);
  staticBoxSizer1->Add(m_ComboBoxBool, 1, wxALL/*|wxALIGN_LEFT*/|wxALIGN_CENTER_VERTICAL, 5);

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

bool pwFiltersBoolDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap pwFiltersBoolDlg::GetBitmapResource(const wxString& WXUNUSED(name))
{
  // Bitmap retrieval
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon pwFiltersBoolDlg::GetIconResource(const wxString& WXUNUSED(name))
{
  // Icon retrieval
  return wxNullIcon;
}

/*!
 * wxEVT_COMBOBOX event handler for wxComboBox
 */

void pwFiltersBoolDlg::OnSelectionChange(wxCommandEvent& WXUNUSED(event))
{
  if(m_ComboBoxBool) {
    m_idx = m_ComboBoxBool->GetSelection();
    
    if((m_idx == 0) || (m_idx == 1))
      FindWindow(wxID_OK)->Enable();
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void pwFiltersBoolDlg::OnOk(wxCommandEvent& WXUNUSED(event))
{
  if (Validate() && TransferDataFromWindow()) {
    m_idx = m_ComboBoxBool->GetSelection();
    
    if(m_idx == 0)
      *m_prule = m_pmrx[0];
    else if(m_idx == 1)
      *m_prule = m_pmrx[1];
    else {
      // On empty choice handle as cancel
      *m_prule = PWSMatch::MR_INVALID;
      EndModal(wxID_CANCEL);
      return;
    }
  }
  EndModal(wxID_OK);
}

bool pwFiltersBoolDlg::IsChanged() const {
  switch (m_ComboBoxBool->GetSelection()) {
    case 0:
      return *m_prule != m_pmrx[0];
    case 1:
      return *m_prule != m_pmrx[1];
    default:
      return *m_prule != PWSMatch::MR_INVALID;
  }
}
