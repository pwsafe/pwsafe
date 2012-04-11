/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
  SetDefaultSymbolDisplay();
  return true;
}

void CPasswordPolicy::SetDefaultSymbolDisplay()
{
  stringT symset;
  if (m_pwUseEasyVision) 
    CPasswordCharPool::GetEasyVisionSymbols(symset);
  else if (m_pwMakePronounceable)
    CPasswordCharPool::GetPronounceableSymbols(symset);
  else
    CPasswordCharPool::GetDefaultSymbols(symset);
  FindWindow(IDC_STATIC_DEFAULT_SYMBOLS)->SetLabel(symset.c_str());
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
  m_pwMinsGSzr = NULL;
  m_pwpUseLowerCtrl = NULL;
  m_pwNumLCbox = NULL;
  m_pwpLCSpin = NULL;
  m_pwpUseUpperCtrl = NULL;
  m_pwNumUCbox = NULL;
  m_pwpUCSpin = NULL;
  m_pwpUseDigitsCtrl = NULL;
  m_pwNumDigbox = NULL;
  m_pwpDigSpin = NULL;
  m_pwpSymCtrl = NULL;
  m_pwNumSymbox = NULL;
  m_pwpSymSpin = NULL;
  m_pwpEasyCtrl = NULL;
  m_pwpPronounceCtrl = NULL;
  m_pwpHexCtrl = NULL;
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

  m_pwMinsGSzr = new wxGridSizer(6, 2, 0, 0);
  itemStaticBoxSizer6->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpUseLowerCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(40, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(40, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText20 = new wxStaticText( itemDialog1, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText23 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(40, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText25 = new wxStaticText( itemDialog1, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText28 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemDialog1, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(40, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText30 = new wxStaticText( itemDialog1, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxRadioButton* itemRadioButton31 = new wxRadioButton( itemDialog1, IDC_USE_DEFAULTSYMBOLS, _("Default set"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton31->SetValue(false);
  m_pwMinsGSzr->Add(itemRadioButton31, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText32 = new wxStaticText( itemDialog1, IDC_STATIC_DEFAULT_SYMBOLS, _("Static text"), wxDefaultPosition, wxSize(140, -1), 0 );
  m_pwMinsGSzr->Add(itemStaticText32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxRadioButton* itemRadioButton33 = new wxRadioButton( itemDialog1, IDC_USE_OWNSYMBOLS, _("Special set"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton33->SetValue(false);
  m_pwMinsGSzr->Add(itemRadioButton33, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl34 = new wxTextCtrl( itemDialog1, IDC_OWNSYMBOLS, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
  m_pwMinsGSzr->Add(itemTextCtrl34, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpEasyCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpPronounceCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText39 = new wxStaticText( itemDialog1, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer6->Add(itemStaticText39, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer6->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer41 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer41, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton42 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer41->AddButton(itemButton42);

  wxButton* itemButton43 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer41->AddButton(itemButton43);

  wxButton* itemButton44 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer41->AddButton(itemButton44);

  itemStdDialogButtonSizer41->Realize();

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
  itemTextCtrl34->SetValidator( wxGenericValidator(& m_Symbols) );
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

wxBitmap CPasswordPolicy::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CPasswordPolicy bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end CPasswordPolicy bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPasswordPolicy::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CPasswordPolicy icon retrieval
  wxUnusedVar(name);
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
    mess = _("Hexadecimal is mutually exculsive to all other options.");
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
              (m_MapPSWDPLC.find(m_polname.c_str()) != m_MapPSWDPLC.end()))) {
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

    m_st_pp.pwp.flags = 0;
    if (m_pwUseLowercase == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::UseLowercase;
    if (m_pwUseUppercase == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::UseUppercase;
    if (m_pwUseDigits == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::UseDigits;
    if (m_pwUseSymbols == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::UseSymbols;
    if (m_pwUseHex == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::UseHexDigits;
    if (m_pwUseEasyVision == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::UseEasyVision;
    if (m_pwMakePronounceable == TRUE)
      m_st_pp.pwp.flags |= PWPolicy::MakePronounceable;

    m_st_pp.pwp.length = m_pwdefaultlength;
    m_st_pp.pwp.digitminlength = m_pwDigitMinLength;
    m_st_pp.pwp.lowerminlength = m_pwLowerMinLength;
    m_st_pp.pwp.symbolminlength = m_pwSymbolMinLength;
    m_st_pp.pwp.upperminlength = m_pwUpperMinLength;
#ifdef NOTYET
    m_st_pp.symbols = (m_pwUseSymbols == TRUE && m_UseOwnSymbols == OWN_SYMBOLS) ?
      m_Symbols : L"";
#endif
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

void CPasswordPolicy::SetPolicyData(const wxString &polname, const st_PSWDPolicy &pol)
{
  m_polname = m_oldpolname = polname;

  m_pwUseLowercase = m_oldpwUseLowercase =
    (pol.pwp.flags & PWPolicy::UseLowercase) ==
                       PWPolicy::UseLowercase;
  m_pwUseUppercase = m_oldpwUseUppercase =
    (pol.pwp.flags & PWPolicy::UseUppercase) ==
                       PWPolicy::UseUppercase;
  m_pwUseDigits = m_oldpwUseDigits =
    (pol.pwp.flags & PWPolicy::UseDigits) ==
                       PWPolicy::UseDigits;
  m_pwUseSymbols = m_oldpwUseSymbols =
    (pol.pwp.flags & PWPolicy::UseSymbols) ==
                       PWPolicy::UseSymbols;
  m_pwUseHex = m_oldpwUseHex =
    (pol.pwp.flags & PWPolicy::UseHexDigits) ==
                       PWPolicy::UseHexDigits;
  m_pwUseEasyVision = m_oldpwUseEasyVision =
    (pol.pwp.flags & PWPolicy::UseEasyVision) ==
                       PWPolicy::UseEasyVision;
  m_pwMakePronounceable = m_oldpwMakePronounceable =
    (pol.pwp.flags & PWPolicy::MakePronounceable) ==
                       PWPolicy::MakePronounceable;
  m_pwdefaultlength = m_oldpwdefaultlength = pol.pwp.length;
  m_pwDigitMinLength = m_oldpwDigitMinLength = pol.pwp.digitminlength;
  m_pwLowerMinLength = m_oldpwLowerMinLength = pol.pwp.lowerminlength;
  m_pwSymbolMinLength = m_oldpwSymbolMinLength = pol.pwp.symbolminlength;
  m_pwUpperMinLength = m_oldpwUpperMinLength = pol.pwp.upperminlength;

  wxString symbols = pol.symbols.c_str();
  m_Symbols = m_oldSymbols = symbols;
#ifdef NOTYET
  m_UseOwnSymbols = m_oldUseOwnSymbols = cs_symbols.IsEmpty() ? DEFAULT_SYMBOLS : OWN_SYMBOLS;
#endif
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
  FindWindow(IDC_USE_DEFAULTSYMBOLS)->Enable(checked);
  FindWindow(IDC_USE_OWNSYMBOLS)->Enable(checked);
  FindWindow(IDC_STATIC_DEFAULT_SYMBOLS)->Enable(checked);
  FindWindow(IDC_OWNSYMBOLS)->Enable(checked);
}

