/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PasswordPolicy.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "PasswordPolicy.h"
#include "core/PWCharPool.h"
#include "./wxutils.h"

////@begin XPM images
////@end XPM images

/*!
 * CPasswordPolicy event table definition
 */

BEGIN_EVENT_TABLE( CPasswordPolicy, wxDialog )

////@begin CPasswordPolicy event table entries
  EVT_CHECKBOX( ID_CHECKBOX3, CPasswordPolicy::OnPwPolUseLowerCase )
  EVT_CHECKBOX( ID_CHECKBOX4, CPasswordPolicy::OnPwPolUseUpperCase )
  EVT_CHECKBOX( ID_CHECKBOX5, CPasswordPolicy::OnPwPolUseDigits )
  EVT_CHECKBOX( ID_CHECKBOX6, CPasswordPolicy::OnPwPolUseSymbols )
  EVT_BUTTON( ID_RESET_SYMBOLS, CPasswordPolicy::OnResetSymbolsClick )
  EVT_CHECKBOX( ID_CHECKBOX7, CPasswordPolicy::OnEZreadCBClick )
  EVT_CHECKBOX( ID_CHECKBOX8, CPasswordPolicy::OnPronouceableCBClick )
  EVT_BUTTON( wxID_OK, CPasswordPolicy::OnOkClick )
  EVT_BUTTON( wxID_CANCEL, CPasswordPolicy::OnCancelClick )
  EVT_BUTTON( wxID_HELP, CPasswordPolicy::OnHelpClick )
////@end CPasswordPolicy event table entries

END_EVENT_TABLE()

/*!
 * CPasswordPolicy constructor
 */

CPasswordPolicy::CPasswordPolicy( wxWindow* parent, PWScore &core,
                                  const PSWDPolicyMap &polmap,
                                  wxWindowID id, const wxString& caption,
                                  const wxPoint& pos, const wxSize& size, long style )
: m_core(core), m_MapPSWDPLC(polmap)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * CPasswordPolicy creator
 */

bool CPasswordPolicy::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPasswordPolicy creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CPasswordPolicy creation
  return true;
}

void CPasswordPolicy::SetDefaultSymbolDisplay(bool restore_defaults)
{
  stringT symset;
  if (m_pwUseEasyVision)
    symset = CPasswordCharPool::GetEasyVisionSymbols();
  else if (m_pwMakePronounceable)
    symset = CPasswordCharPool::GetPronounceableSymbols();
  else {
    if (restore_defaults)
      CPasswordCharPool::ResetDefaultSymbols(); // restore the preference!
    symset = CPasswordCharPool::GetDefaultSymbols();
  }
  m_Symbols = symset.c_str();
  m_OwnSymbols->SetValue(m_Symbols);
}

/*!
 * CPasswordPolicy destructor
 */

CPasswordPolicy::~CPasswordPolicy()
{
////@begin CPasswordPolicy destruction
////@end CPasswordPolicy destruction
}

/*!
 * Member initialisation
 */

void CPasswordPolicy::Init()
{
////@begin CPasswordPolicy member initialisation
  m_pwMinsGSzr = nullptr;
  m_pwpUseLowerCtrl = nullptr;
  m_pwNumLCbox = nullptr;
  m_pwpLCSpin = nullptr;
  m_pwpUseUpperCtrl = nullptr;
  m_pwNumUCbox = nullptr;
  m_pwpUCSpin = nullptr;
  m_pwpUseDigitsCtrl = nullptr;
  m_pwNumDigbox = nullptr;
  m_pwpDigSpin = nullptr;
  m_pwpSymCtrl = nullptr;
  m_pwNumSymbox = nullptr;
  m_pwpSymSpin = nullptr;
  m_OwnSymbols = nullptr;
  m_pwpEasyCtrl = nullptr;
  m_pwpPronounceCtrl = nullptr;
  m_pwpHexCtrl = nullptr;
////@end CPasswordPolicy member initialisation
}

/*!
 * Control creation for CPasswordPolicy
 */

void CPasswordPolicy::CreateControls()
{
////@begin CPasswordPolicy content construction
  CPasswordPolicy* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Policy Name:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 10);

  wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemDialog1, ID_POLICYNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemTextCtrl5, 2, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Random password generation rules"));
  wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl9 = new wxSpinCtrl( itemDialog1, ID_PWLENSB, _T("12"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 1024, 12 );
  itemBoxSizer7->Add(itemSpinCtrl9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr = new wxGridSizer(0, 2, 0, 0);
  itemStaticBoxSizer6->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpUseLowerCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL5, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, wxT(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL6, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText20 = new wxStaticText( itemDialog1, wxID_STATIC, wxT(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText23 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL7, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText25 = new wxStaticText( itemDialog1, wxID_STATIC, wxT(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText28 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL8, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText30 = new wxStaticText( itemDialog1, wxID_STATIC, wxT(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_OwnSymbols = new wxTextCtrl( itemDialog1, IDC_OWNSYMBOLS, wxEmptyString, wxDefaultPosition, wxSize(220, -1), 0 );
  m_pwMinsGSzr->Add(m_OwnSymbols, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxButton* itemButton32 = new wxButton( itemDialog1, ID_RESET_SYMBOLS, _("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwMinsGSzr->Add(itemButton32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

  m_pwpEasyCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpPronounceCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText37 = new wxStaticText( itemDialog1, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer6->Add(itemStaticText37, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer6->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer39 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer39, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton40 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer39->AddButton(itemButton40);

  wxButton* itemButton41 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer39->AddButton(itemButton41);

  wxButton* itemButton42 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer39->AddButton(itemButton42);

  itemStdDialogButtonSizer39->Realize();

  // Set validators
  itemTextCtrl5->SetValidator( wxGenericValidator(& m_polname) );
  itemSpinCtrl9->SetValidator( wxGenericValidator(& m_pwdefaultlength) );
  m_pwpUseLowerCtrl->SetValidator( wxGenericValidator(& m_pwUseLowercase) );
  m_pwpLCSpin->SetValidator( wxGenericValidator(& m_pwLowerMinLength) );
  m_pwpUseUpperCtrl->SetValidator( wxGenericValidator(& m_pwUseUppercase) );
  m_pwpUCSpin->SetValidator( wxGenericValidator(& m_pwUpperMinLength) );
  m_pwpUseDigitsCtrl->SetValidator( wxGenericValidator(& m_pwUseDigits) );
  m_pwpDigSpin->SetValidator( wxGenericValidator(& m_pwDigitMinLength) );
  m_pwpSymCtrl->SetValidator( wxGenericValidator(& m_pwUseSymbols) );
  m_pwpSymSpin->SetValidator( wxGenericValidator(& m_pwSymbolMinLength) );
  m_OwnSymbols->SetValidator( wxGenericValidator(& m_Symbols) );
  m_pwpEasyCtrl->SetValidator( wxGenericValidator(& m_pwUseEasyVision) );
  m_pwpPronounceCtrl->SetValidator( wxGenericValidator(& m_pwMakePronounceable) );
  m_pwpHexCtrl->SetValidator( wxGenericValidator(& m_pwUseHex) );
////@end CPasswordPolicy content construction
}

/*!
 * Should we show tooltips?
 */

bool CPasswordPolicy::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPasswordPolicy::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin CPasswordPolicy bitmap retrieval
  return wxNullBitmap;
////@end CPasswordPolicy bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPasswordPolicy::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin CPasswordPolicy icon retrieval
  return wxNullIcon;
////@end CPasswordPolicy icon retrieval
}

bool CPasswordPolicy::Verify()
{
  wxString mess;
  bool retval = true;
  int id = 0;

  // Check that options, as set, are valid.
  if (m_pwUseHex &&
     (m_pwUseLowercase || m_pwUseUppercase || m_pwUseDigits ||
      m_pwUseSymbols || m_pwUseEasyVision || m_pwMakePronounceable)) {
    mess = _("Hexadecimal is mutually exclusive to all other options.");
    retval = false;
  } else if (m_pwUseHex) {
    if (m_pwdefaultlength % 2 != 0) {
      mess = _("Hexadecimal passwords must have even length - each byte is two characters");
      retval = false;
    }
  } else if (!m_pwUseLowercase && !m_pwUseUppercase &&
      !m_pwUseDigits && !m_pwUseSymbols) {
    mess = _("At least one type of character (lowercase, uppercase, digits, symbols or hexadecimal) must be chosen");
    retval = false;
  }

  if ((m_pwdefaultlength < 4) || (m_pwdefaultlength > 1024)) {
    mess = _("Password must be between 4 and 1024 characters");
    id = ID_PWLENSB;
    retval = false;
  }

  if (!(m_pwUseHex || m_pwUseEasyVision || m_pwMakePronounceable) &&
      (m_pwDigitMinLength + m_pwLowerMinLength +
       m_pwSymbolMinLength + m_pwUpperMinLength) > m_pwdefaultlength) {
    mess = _("Password length is less than sum of 'at least' constraints");
    id = ID_PWLENSB;
    retval = false;
  }

  if ((m_pwUseHex || m_pwUseEasyVision || m_pwMakePronounceable))
    m_pwDigitMinLength = m_pwLowerMinLength =
      m_pwSymbolMinLength = m_pwUpperMinLength = 1;

  if (m_polname.IsEmpty()) {
    mess = _("Policy name cannot be blank");
    id = ID_POLICYNAME;
    retval = false;
  } else if ((m_polname != m_oldpolname &&
              (m_MapPSWDPLC.find(tostringx(m_polname)) != m_MapPSWDPLC.end()))) {
    mess = _("Policy name is already in use");
    id = ID_POLICYNAME;
    retval = false;
  }

  if (!retval) {
    wxMessageDialog md(this, mess, _("Policy Error"), wxOK | wxICON_ERROR);
    md.ShowModal();
    if (id != 0)
      FindWindow(id)->SetFocus();
  }
  return retval;
}

bool CPasswordPolicy::UpdatePolicy()
{
  if (Validate() && TransferDataFromWindow() && Verify()) {

    m_st_pp.flags = 0;
    if (m_pwUseLowercase == TRUE)
      m_st_pp.flags |= PWPolicy::UseLowercase;
    if (m_pwUseUppercase == TRUE)
      m_st_pp.flags |= PWPolicy::UseUppercase;
    if (m_pwUseDigits == TRUE)
      m_st_pp.flags |= PWPolicy::UseDigits;
    if (m_pwUseSymbols == TRUE)
      m_st_pp.flags |= PWPolicy::UseSymbols;
    if (m_pwUseHex == TRUE)
      m_st_pp.flags |= PWPolicy::UseHexDigits;
    if (m_pwUseEasyVision == TRUE)
      m_st_pp.flags |= PWPolicy::UseEasyVision;
    if (m_pwMakePronounceable == TRUE)
      m_st_pp.flags |= PWPolicy::MakePronounceable;

    m_st_pp.length = m_pwdefaultlength;
    m_st_pp.digitminlength = m_pwDigitMinLength;
    m_st_pp.lowerminlength = m_pwLowerMinLength;
    m_st_pp.symbolminlength = m_pwSymbolMinLength;
    m_st_pp.upperminlength = m_pwUpperMinLength;
    m_st_pp.symbols = m_Symbols.c_str();
    return true;
  }
  return false;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CPasswordPolicy::OnOkClick( wxCommandEvent& )
{
  if (m_core.IsReadOnly())
    return;
  if (!UpdatePolicy())
    return;
  EndModal(wxID_OK);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CPasswordPolicy::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CPasswordPolicy.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CPasswordPolicy.
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void CPasswordPolicy::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in CPasswordPolicy.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in CPasswordPolicy.
}

void CPasswordPolicy::SetPolicyData(const wxString &polname, const PWPolicy &pol)
{
  m_polname = m_oldpolname = polname;

  m_pwUseLowercase = m_oldpwUseLowercase =
    (pol.flags & PWPolicy::UseLowercase) ==
                       PWPolicy::UseLowercase;
  m_pwUseUppercase = m_oldpwUseUppercase =
    (pol.flags & PWPolicy::UseUppercase) ==
                       PWPolicy::UseUppercase;
  m_pwUseDigits = m_oldpwUseDigits =
    (pol.flags & PWPolicy::UseDigits) ==
                       PWPolicy::UseDigits;
  m_pwUseSymbols = m_oldpwUseSymbols =
    (pol.flags & PWPolicy::UseSymbols) ==
                       PWPolicy::UseSymbols;
  m_pwUseHex = m_oldpwUseHex =
    (pol.flags & PWPolicy::UseHexDigits) ==
                       PWPolicy::UseHexDigits;
  m_pwUseEasyVision = m_oldpwUseEasyVision =
    (pol.flags & PWPolicy::UseEasyVision) ==
                       PWPolicy::UseEasyVision;
  m_pwMakePronounceable = m_oldpwMakePronounceable =
    (pol.flags & PWPolicy::MakePronounceable) ==
                       PWPolicy::MakePronounceable;
  m_pwdefaultlength = m_oldpwdefaultlength = pol.length;
  m_pwDigitMinLength = m_oldpwDigitMinLength = pol.digitminlength;
  m_pwLowerMinLength = m_oldpwLowerMinLength = pol.lowerminlength;
  m_pwSymbolMinLength = m_oldpwSymbolMinLength = pol.symbolminlength;
  m_pwUpperMinLength = m_oldpwUpperMinLength = pol.upperminlength;

  wxString symbols = pol.symbols.c_str();
  if (symbols.empty())
    SetDefaultSymbolDisplay(false);
  else
    m_Symbols = symbols;
  m_oldSymbols = m_Symbols;
}

void CPasswordPolicy::CBox2Spin(wxCheckBox *cb, wxSpinCtrl *sp)
{
  Validate();
  TransferDataFromWindow();
  bool checked = cb->GetValue();
  sp->Enable(checked);
  Validate();
  TransferDataFromWindow();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
 */

void CPasswordPolicy::OnPwPolUseLowerCase( wxCommandEvent& )
{
  CBox2Spin(m_pwpUseLowerCtrl, m_pwpLCSpin);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX4
 */

void CPasswordPolicy::OnPwPolUseUpperCase( wxCommandEvent& )
{
  CBox2Spin(m_pwpUseUpperCtrl, m_pwpUCSpin);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX5
 */

void CPasswordPolicy::OnPwPolUseDigits( wxCommandEvent& )
{
  CBox2Spin(m_pwpUseDigitsCtrl, m_pwpDigSpin);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX6
 */

void CPasswordPolicy::OnPwPolUseSymbols( wxCommandEvent& )
{
  CBox2Spin(m_pwpSymCtrl, m_pwpSymSpin);
  bool checked = m_pwpSymCtrl->GetValue();
  m_OwnSymbols->Enable(checked);
  FindWindow(ID_RESET_SYMBOLS)->Enable(checked);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RESET_SYMBOLS
 */

void CPasswordPolicy::OnResetSymbolsClick( wxCommandEvent& WXUNUSED(event) )
{
  SetDefaultSymbolDisplay(true);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
 */

void CPasswordPolicy::OnPronouceableCBClick( wxCommandEvent& event )
{
  if (event.IsChecked()) {
    // Check if ezread is also set - forbid both
    if (m_pwpEasyCtrl->GetValue()) {
      m_pwpPronounceCtrl->SetValue(false);
      wxMessageBox(_("Sorry, \"pronounceable\" and \"easy-to-read\" cannot be both selected"),
                   _("Error"), wxOK|wxICON_ERROR, this);
      return;
    }
  }
  if (Validate() && TransferDataFromWindow())
    SetDefaultSymbolDisplay(false);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX7
 */

void CPasswordPolicy::OnEZreadCBClick( wxCommandEvent& event )
{
  if (event.IsChecked()) {
    // Check if pronounceable is also set - forbid both
    if (m_pwpPronounceCtrl->GetValue()) {
      m_pwpEasyCtrl->SetValue(false);
      wxMessageBox(_("Sorry, \"easy-to-read\" and \"pronounceable\" cannot be both selected"),
                   _("Error"), wxOK|wxICON_ERROR, this);
      return;
    }
  }
  if (Validate() && TransferDataFromWindow())
    SetDefaultSymbolDisplay(false);
}
