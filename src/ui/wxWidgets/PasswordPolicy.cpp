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
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

////@begin includes
////@end includes

#include "PasswordPolicy.h"
#include "core/PolicyManager.h"
#include "core/PWCharPool.h"
#include "pwsclip.h"
#include "./wxutils.h"

////@begin XPM images
#include "graphics/toolbar/new/copypassword.xpm"
#include "graphics/toolbar/new/copypassword_disabled.xpm"
////@end XPM images

/*!
 * CPasswordPolicy event table definition
 */

BEGIN_EVENT_TABLE( CPasswordPolicy, wxDialog )

////@begin CPasswordPolicy event table entries
  EVT_SPINCTRL( ID_SPINCTRL5        , CPasswordPolicy::OnAtLeastPasswordChars )
  EVT_SPINCTRL( ID_SPINCTRL6        , CPasswordPolicy::OnAtLeastPasswordChars )
  EVT_SPINCTRL( ID_SPINCTRL7        , CPasswordPolicy::OnAtLeastPasswordChars )
  EVT_SPINCTRL( ID_SPINCTRL8        , CPasswordPolicy::OnAtLeastPasswordChars )
  EVT_CHECKBOX( ID_CHECKBOX41       , CPasswordPolicy::OnUseNamedPolicy       )
  EVT_COMBOBOX( ID_COMBOBOX41       , CPasswordPolicy::OnPolicynameSelection  )
  EVT_CHECKBOX( ID_CHECKBOX3        , CPasswordPolicy::OnPwPolUseLowerCase    )
  EVT_CHECKBOX( ID_CHECKBOX4        , CPasswordPolicy::OnPwPolUseUpperCase    )
  EVT_CHECKBOX( ID_CHECKBOX5        , CPasswordPolicy::OnPwPolUseDigits       )
  EVT_CHECKBOX( ID_CHECKBOX6        , CPasswordPolicy::OnPwPolUseSymbols      )
  EVT_BUTTON(   ID_RESET_SYMBOLS    , CPasswordPolicy::OnResetSymbolsClick    )
  EVT_CHECKBOX( ID_CHECKBOX7        , CPasswordPolicy::OnEZreadCBClick        )
  EVT_CHECKBOX( ID_CHECKBOX8        , CPasswordPolicy::OnPronouceableCBClick  )
  EVT_BUTTON(   ID_GENERATEPASSWORD2, CPasswordPolicy::OnGeneratePassword     )
  EVT_BUTTON(   ID_COPYPASSWORD2    , CPasswordPolicy::OnCopyPassword         )
  EVT_BUTTON(   wxID_OK             , CPasswordPolicy::OnOkClick              )
  EVT_BUTTON(   wxID_CANCEL         , CPasswordPolicy::OnCancelClick          )
  EVT_BUTTON(   wxID_HELP           , CPasswordPolicy::OnHelpClick            )
////@end CPasswordPolicy event table entries

END_EVENT_TABLE()

/*!
 * CPasswordPolicy constructor
 */

CPasswordPolicy::CPasswordPolicy( wxWindow* parent, PWScore &core,
                                  const PSWDPolicyMap &polmap, DialogType type,
                                  wxWindowID id, const wxString& caption,
                                  const wxPoint& pos, const wxSize& size, long style )
: m_core(core), m_MapPSWDPLC(polmap), m_DialogType(type)
{
  Init();
  Create(parent, type, id, caption, pos, size, style);
}

/*!
 * CPasswordPolicy creator
 */

bool CPasswordPolicy::Create( wxWindow* parent, DialogType type, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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
  m_pwpLenCtrl = nullptr;
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

  m_pwpUseNamedPolicyCtrl = nullptr;
  m_pwpPoliciesSelectionCtrl = nullptr;
  m_passwordCtrl = nullptr;
  m_itemStaticBoxSizer6 = nullptr;
////@end CPasswordPolicy member initialisation

  // Collect all policy names to display in combobox control
  for (auto& policy : m_MapPSWDPLC) {
    m_Policynames.Add(stringx2std(policy.first));
  }
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

  /////////////////////////////////////////////////////////////////////////////
  // Top: Manage/Edit Mode
  /////////////////////////////////////////////////////////////////////////////

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);

  wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Policy Name:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemDialog1, ID_POLICYNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemTextCtrl5, 2, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  itemBoxSizer3->AddStretchSpacer();

  /////////////////////////////////////////////////////////////////////////////
  // Top: Generator Mode
  /////////////////////////////////////////////////////////////////////////////

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);

  m_pwpUseNamedPolicyCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX41, _("Use Named Policy"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseNamedPolicyCtrl->SetValue(true);
  itemBoxSizer4->Add(m_pwpUseNamedPolicyCtrl, 2, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpPoliciesSelectionCtrl = new wxComboBox( itemDialog1, ID_COMBOBOX41, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Policynames, wxCB_READONLY );
  itemBoxSizer4->Add(m_pwpPoliciesSelectionCtrl, 3, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  itemBoxSizer4->AddStretchSpacer();

  /////////////////////////////////////////////////////////////////////////////
  // Center: Password Generation Rules
  /////////////////////////////////////////////////////////////////////////////

  wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Random password generation rules"));
  m_itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
  itemBoxSizer2->Add(m_itemStaticBoxSizer6, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);

  wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
  m_itemStaticBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLenCtrl = new wxSpinCtrl(
    itemDialog1, ID_PWLENSB, _T("12"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDefaultLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDefaultLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWDefaultLength)
  );

  itemBoxSizer7->Add(m_pwpLenCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr = new wxGridSizer(0, 2, 0, 0);
  m_itemStaticBoxSizer6->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpUseLowerCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl(
    itemDialog1, ID_SPINCTRL5, wxT("0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWLowercaseMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWLowercaseMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWLowercaseMinLength)
  );

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

  m_pwpUCSpin = new wxSpinCtrl(
    itemDialog1, ID_SPINCTRL6, wxT("0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWUppercaseMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWUppercaseMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWUppercaseMinLength)
  );

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

  m_pwpDigSpin = new wxSpinCtrl(
    itemDialog1, ID_SPINCTRL7, wxT("0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDigitMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDigitMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWDigitMinLength)
  );

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

  m_pwpSymSpin = new wxSpinCtrl(
    itemDialog1, ID_SPINCTRL8, wxT("0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWSymbolMinLength),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWSymbolMinLength),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PWSymbolMinLength)
  );

  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText30 = new wxStaticText( itemDialog1, wxID_STATIC, wxT(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_OwnSymbols = new wxTextCtrl( itemDialog1, IDC_OWNSYMBOLS, wxEmptyString, wxDefaultPosition, wxSize(230, -1), 0 );
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
  m_itemStaticBoxSizer6->Add(itemStaticText37, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemDialog1, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  m_itemStaticBoxSizer6->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  /////////////////////////////////////////////////////////////////////////////
  // Bottom: Password Generation Controls for Generator Mode
  /////////////////////////////////////////////////////////////////////////////

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxALL, 5);

  m_passwordCtrl = new wxTextCtrl( itemDialog1, ID_GENERATEDPASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(m_passwordCtrl, 1, wxALIGN_CENTER_VERTICAL|wxLEFT, 20);

  wxButton* itemButton1 = new wxButton( itemDialog1, ID_GENERATEPASSWORD2, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton1 = new wxBitmapButton( itemDialog1, ID_COPYPASSWORD2, itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );

  if (CPasswordPolicy::ShowToolTips())
    itemBitmapButton1->SetToolTip(_("Copy Password to clipboard"));

  itemBoxSizer5->Add(itemBitmapButton1, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 20);

  /////////////////////////////////////////////////////////////////////////////
  // Bottom: Dialog Buttons for Manage/Edit Mode
  /////////////////////////////////////////////////////////////////////////////

  wxStdDialogButtonSizer* itemStdDialogButtonSizer39 = new wxStdDialogButtonSizer;
  itemBoxSizer2->Add(itemStdDialogButtonSizer39, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton40 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer39->AddButton(itemButton40);

  wxButton* itemButton41 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer39->AddButton(itemButton41);

  wxButton* itemButton42 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer39->AddButton(itemButton42);

  itemStdDialogButtonSizer39->Realize();

  /////////////////////////////////////////////////////////////////////////////
  // Bottom: Dialog Buttons for Generator Mode
  /////////////////////////////////////////////////////////////////////////////

  wxStdDialogButtonSizer* itemStdDialogButtonSizer40 = new wxStdDialogButtonSizer;
  itemBoxSizer2->Add(itemStdDialogButtonSizer40, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton43 = new wxButton( itemDialog1, wxID_CANCEL, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer40->AddButton(itemButton43);

  itemStdDialogButtonSizer40->Realize();

  // Set validators
  itemTextCtrl5->SetValidator( wxGenericValidator(& m_polname) );
  m_pwpLenCtrl->SetValidator( wxGenericValidator(& m_pwdefaultlength) );
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

  if (m_DialogType == DialogType::GENERATOR) {
    itemBoxSizer3->Show(false);
    itemBoxSizer4->Show(true);
    itemBoxSizer5->Show(true);
    itemStdDialogButtonSizer39->Show(false);
    itemStdDialogButtonSizer40->Show(true);

    EnableSizerChildren(m_itemStaticBoxSizer6, !m_pwpUseNamedPolicyCtrl->IsChecked());
  }
  else {
    itemBoxSizer3->Show(true);
    itemBoxSizer4->Show(false);
    itemBoxSizer5->Show(false);
    itemStdDialogButtonSizer39->Show(true);
    itemStdDialogButtonSizer40->Show(false);
  }
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
  if (name == _T("graphics/toolbar/new/copypassword.xpm")) {
    wxBitmap bitmap(copypassword_xpm);
    return bitmap;
  }
  else if (name == _T("graphics/toolbar/new/copypassword_disabled.xpm")) {
    wxBitmap bitmap(copypassword_disabled_xpm);
    return bitmap;
  }

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
  } else if ((m_polname != m_oldpolname) &&
             PolicyManager::IsDefaultPolicy(m_polname.wc_str())) {
    mess = _("Default policy name is not allowed");
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

void CPasswordPolicy::SetPolicyData(const wxString &policyname, const PWPolicy &policy)
{
  m_polname             = m_oldpolname             = policyname;

  m_pwUseLowercase      = m_oldpwUseLowercase      = (policy.flags & PWPolicy::UseLowercase)      == PWPolicy::UseLowercase;
  m_pwUseUppercase      = m_oldpwUseUppercase      = (policy.flags & PWPolicy::UseUppercase)      == PWPolicy::UseUppercase;
  m_pwUseDigits         = m_oldpwUseDigits         = (policy.flags & PWPolicy::UseDigits)         == PWPolicy::UseDigits;
  m_pwUseSymbols        = m_oldpwUseSymbols        = (policy.flags & PWPolicy::UseSymbols)        == PWPolicy::UseSymbols;
  m_pwUseHex            = m_oldpwUseHex            = (policy.flags & PWPolicy::UseHexDigits)      == PWPolicy::UseHexDigits;
  m_pwUseEasyVision     = m_oldpwUseEasyVision     = (policy.flags & PWPolicy::UseEasyVision)     == PWPolicy::UseEasyVision;
  m_pwMakePronounceable = m_oldpwMakePronounceable = (policy.flags & PWPolicy::MakePronounceable) == PWPolicy::MakePronounceable;
  m_pwdefaultlength     = m_oldpwdefaultlength     = policy.length;
  m_pwLowerMinLength    = m_oldpwLowerMinLength    = policy.lowerminlength;
  m_pwUpperMinLength    = m_oldpwUpperMinLength    = policy.upperminlength;
  m_pwDigitMinLength    = m_oldpwDigitMinLength    = policy.digitminlength;
  m_pwSymbolMinLength   = m_oldpwSymbolMinLength   = policy.symbolminlength;

  wxString symbols = policy.symbols.c_str();

  if (symbols.empty()) {
    SetDefaultSymbolDisplay(false);
  }
  else {
    m_Symbols = symbols;
  }

  m_oldSymbols = m_Symbols;

  if (PolicyManager::IsDefaultPolicy(m_polname.wc_str())) {

    // Disallow renaming of default policy in Edit/Manage mode
    FindWindow(ID_POLICYNAME)->Enable(false);

    // Select default policy as initial policy in Generator mode (see Init())
    auto index = m_pwpPoliciesSelectionCtrl->FindString(policyname);

    if (index != wxNOT_FOUND) {
      m_pwpPoliciesSelectionCtrl->SetSelection(index);
    }
    else {
      m_pwpPoliciesSelectionCtrl->SetSelection(0);
    }
  }
}

void CPasswordPolicy::CBox2Spin(wxCheckBox *checkbox, wxSpinCtrl *spinner)
{
  Validate();
  TransferDataFromWindow();

  spinner->Enable(checkbox->GetValue());

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

/*!
 * Iterates recursively through the hierarchy of sizers and enables/disables
 * all items of type wxWindow.
 * 
 * @param sizer the sizer from that the search for wxWindow objects is started.
 * @param state the state that shall be set for all wxWindow children of a wxSizer.
 */
void CPasswordPolicy::EnableSizerChildren(wxSizer* sizer, bool state)
{
  if (sizer == nullptr) {
    return;
  }

  auto& items = sizer->GetChildren();

  for (auto item : items) {

    if (item == nullptr) {
      continue;
    }
    else if (item->IsWindow()) {

      auto window = item->GetWindow();

      if (window) {
        window->Enable(state);
      }
    }
    else if (item->IsSizer()) {

      auto sizer = item->GetSizer();

      if (sizer) {
        EnableSizerChildren(sizer, state);
      }
    }
    else {
      // Don't care; could be a spacer, for instance
    }
  }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX41
 */

void CPasswordPolicy::OnUseNamedPolicy( wxCommandEvent& event )
{
  EnableSizerChildren(m_itemStaticBoxSizer6, !event.IsChecked());
  m_pwpPoliciesSelectionCtrl->Enable(event.IsChecked());

  if (!event.IsChecked()) {
    m_pwpLCSpin->Enable(m_pwpUseLowerCtrl->GetValue());
    m_pwpUCSpin->Enable(m_pwpUseUpperCtrl->GetValue());
    m_pwpDigSpin->Enable(m_pwpUseDigitsCtrl->GetValue());
    m_pwpSymSpin->Enable(m_pwpSymCtrl->GetValue());
  }
}

/*!
 * wxEVT_COMMAND_COMBOBOX_CLICKED event handler for ID_COMBOBOX41
 */

void CPasswordPolicy::OnPolicynameSelection( wxCommandEvent& WXUNUSED(event) )
{
  auto policyname = m_pwpPoliciesSelectionCtrl->GetValue();

  if (m_MapPSWDPLC.find(StringX(policyname.c_str())) == m_MapPSWDPLC.end()) {
    ASSERT(0);
    return;
  }

  auto policy = m_MapPSWDPLC.at(StringX(policyname.c_str()));

  m_pwUseLowercase      = (policy.flags & PWPolicy::UseLowercase)      == PWPolicy::UseLowercase;
  m_pwUseUppercase      = (policy.flags & PWPolicy::UseUppercase)      == PWPolicy::UseUppercase;
  m_pwUseDigits         = (policy.flags & PWPolicy::UseDigits)         == PWPolicy::UseDigits;
  m_pwUseSymbols        = (policy.flags & PWPolicy::UseSymbols)        == PWPolicy::UseSymbols;
  m_pwUseHex            = (policy.flags & PWPolicy::UseHexDigits)      == PWPolicy::UseHexDigits;
  m_pwUseEasyVision     = (policy.flags & PWPolicy::UseEasyVision)     == PWPolicy::UseEasyVision;
  m_pwMakePronounceable = (policy.flags & PWPolicy::MakePronounceable) == PWPolicy::MakePronounceable;
  m_Symbols             = policy.symbols.c_str();

  TransferDataToWindow();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERATEPASSWORD2
 */

void CPasswordPolicy::OnGeneratePassword( wxCommandEvent& WXUNUSED(event) )
{
  UpdatePolicy();

  m_passwordCtrl->SetValue(m_st_pp.MakeRandomPassword().c_str());
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYPASSWORD2
 */

void CPasswordPolicy::OnCopyPassword( wxCommandEvent& WXUNUSED(event) )
{
  if (!(m_passwordCtrl->GetValue()).IsEmpty()) {
    PWSclipboard::GetInstance()->SetData(tostringx(m_passwordCtrl->GetValue()));
  }
}

/**
 * wxEVT_SPINCTRL event handler for ID_SPINCTRL5, ID_SPINCTRL6, 
 * ID_SPINCTRL7, ID_SPINCTRL8
 * 
 * Ensures that the sum of each character class' minimum counts 
 * doesn't exceed the overall password length, increasing it as 
 * necessary to give the user some visual indication.
 * 
 * This is not comprehensive & foolproof since there are far too 
 * many ways to make the password length smaller than the sum of 
 * "at least" lengths, to even think of.
 *
 * In OnOk(), we just ensure the password length is greater than
 * the sum of all enabled "at least" lengths.  We have to do this 
 * in the UI, or else password generation crashes.
 */
void CPasswordPolicy::OnAtLeastPasswordChars( wxSpinEvent& WXUNUSED(event) )
{
  wxSpinCtrl* spinControls[] = { m_pwpUCSpin, m_pwpLCSpin, m_pwpDigSpin, m_pwpSymSpin };
  int total = 0;

  // Calculate the sum of each character class' minimum count
  for (const auto spinControl : spinControls) {
    total += spinControl->IsEnabled() ? spinControl->GetValue() : 0;
  }

  // Increase password length up to the allowed maximum
  if ((m_pwpLenCtrl->GetMax() > total) && (total > m_pwpLenCtrl->GetValue())) {
    m_pwpLenCtrl->SetValue(total);
  }
}
