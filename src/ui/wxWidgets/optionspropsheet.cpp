/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file optionspropsheet.cpp
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
#include "wx/bookctrl.h"
////@end includes

#include "optionspropsheet.h"
#include "corelib/PWSprefs.h"

////@begin XPM images
////@end XPM images


/*!
 * COptions type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COptions, wxPropertySheetDialog )


/*!
 * COptions event table definition
 */

BEGIN_EVENT_TABLE( COptions, wxPropertySheetDialog )
EVT_BUTTON( wxID_OK, COptions::OnOk )

////@begin COptions event table entries
////@end COptions event table entries

END_EVENT_TABLE()


/*!
 * COptions constructors
 */

COptions::COptions()
{
  Init();
}

COptions::COptions( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * COptions creator
 */

bool COptions::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COptions creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  LayoutDialog();
  Centre();
////@end COptions creation
  PrefsToPropSheet();
  return true;
}


/*!
 * COptions destructor
 */

COptions::~COptions()
{
////@begin COptions destruction
////@end COptions destruction
}


/*!
 * Member initialisation
 */

void COptions::Init()
{
////@begin COptions member initialisation
  m_pwpLenCtrl = NULL;
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
////@end COptions member initialisation
}


/*!
 * Control creation for COptions
 */

void COptions::CreateControls()
{    
////@begin COptions content construction
  COptions* itemPropertySheetDialog1 = this;

  wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxCheckBox* itemCheckBox4 = new wxCheckBox( itemPanel2, ID_CHECKBOX10, _("Save database immediately after Edit or Add"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox4->SetValue(false);
  itemBoxSizer3->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Intermediate Backups"));
  wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
  itemBoxSizer3->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel2, ID_CHECKBOX11, _("Create intermediate backups (.ibak) before saving"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox6->SetValue(false);
  itemStaticBoxSizer5->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Backup Name"));
  wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
  itemStaticBoxSizer5->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText8 = new wxStaticText( itemPanel2, wxID_STATIC, _("Base:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton9 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON4, _("Database name"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton9->SetValue(false);
  itemStaticBoxSizer7->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);
  wxRadioButton* itemRadioButton11 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON5, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton11->SetValue(false);
  itemBoxSizer10->Add(itemRadioButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer10->Add(itemTextCtrl12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine13 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer7->Add(itemStaticLine13, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemPanel2, wxID_STATIC, _("Suffix:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer15, 0, wxGROW|wxALL, 0);
  wxArrayString itemComboBox16Strings;
  wxComboBox* itemComboBox16 = new wxComboBox( itemPanel2, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox16Strings, wxCB_DROPDOWN );
  itemBoxSizer15->Add(itemComboBox16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemPanel2, wxID_STATIC, _("Max."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl18 = new wxSpinCtrl( itemPanel2, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer15->Add(itemSpinCtrl18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText( itemPanel2, wxID_STATIC, _("Example:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticLine* itemStaticLine20 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer5->Add(itemStaticLine20, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText21 = new wxStaticText( itemPanel2, wxID_STATIC, _("Backup directory:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer5->Add(itemStaticText21, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton22 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON6, _("Same as database's"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton22->SetValue(false);
  itemStaticBoxSizer5->Add(itemRadioButton22, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer5->Add(itemBoxSizer23, 0, wxGROW|wxALL, 0);
  wxRadioButton* itemRadioButton24 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON7, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton24->SetValue(false);
  itemBoxSizer23->Add(itemRadioButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl25 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer23->Add(itemTextCtrl25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton26 = new wxButton( itemPanel2, ID_BUTTON, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer23->Add(itemButton26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  GetBookCtrl()->AddPage(itemPanel2, _("Backups"));

  wxPanel* itemPanel27 = new wxPanel( GetBookCtrl(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
  itemPanel27->SetSizer(itemBoxSizer28);

  wxCheckBox* itemCheckBox29 = new wxCheckBox( itemPanel27, ID_CHECKBOX12, _("Always keep Password Safe on top"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox29->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox29, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox30 = new wxCheckBox( itemPanel27, ID_CHECKBOX13, _("Show Username in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox30->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox30, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox31 = new wxCheckBox( itemPanel27, ID_CHECKBOX14, _("Show Password in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox31->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox31, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox32 = new wxCheckBox( itemPanel27, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox32->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox32, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox33 = new wxCheckBox( itemPanel27, ID_CHECKBOX16, _("Show Password in Add & Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox33->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox33, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox34 = new wxCheckBox( itemPanel27, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox34->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox34, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox35 = new wxCheckBox( itemPanel27, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox35->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox35, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer28->Add(itemBoxSizer36, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox37 = new wxCheckBox( itemPanel27, ID_CHECKBOX19, _("Warn"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox37->SetValue(false);
  itemBoxSizer36->Add(itemCheckBox37, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl38 = new wxSpinCtrl( itemPanel27, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxSize(itemPanel27->ConvertDialogToPixels(wxSize(25, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer36->Add(itemSpinCtrl38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText39 = new wxStaticText( itemPanel27, wxID_STATIC, _("days before passwords expire"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer36->Add(itemStaticText39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemRadioBox40Strings;
  itemRadioBox40Strings.Add(_("&Fully collapsed"));
  itemRadioBox40Strings.Add(_("&Fully expanded"));
  itemRadioBox40Strings.Add(_("&Same as when last saved"));
  wxRadioBox* itemRadioBox40 = new wxRadioBox( itemPanel27, ID_RADIOBOX, _("Initial Tree View"), wxDefaultPosition, wxDefaultSize, itemRadioBox40Strings, 1, wxRA_SPECIFY_COLS );
  itemRadioBox40->SetSelection(0);
  itemBoxSizer28->Add(itemRadioBox40, 0, wxGROW|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel27, _("Display"));

  wxPanel* itemPanel41 = new wxPanel( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxVERTICAL);
  itemPanel41->SetSizer(itemBoxSizer42);

  wxCheckBox* itemCheckBox43 = new wxCheckBox( itemPanel41, ID_CHECKBOX20, _("Confirm deletion of items"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox43->SetValue(false);
  itemBoxSizer42->Add(itemCheckBox43, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox44 = new wxCheckBox( itemPanel41, ID_CHECKBOX21, _("Record last access times"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox44->SetValue(false);
  itemBoxSizer42->Add(itemCheckBox44, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox45 = new wxCheckBox( itemPanel41, ID_CHECKBOX22, _("Escape key closes application"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox45->SetValue(false);
  itemBoxSizer42->Add(itemCheckBox45, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer46 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer42->Add(itemBoxSizer46, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText47 = new wxStaticText( itemPanel41, wxID_STATIC, _("Double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer46->Add(itemStaticText47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemComboBox48Strings;
  wxComboBox* itemComboBox48 = new wxComboBox( itemPanel41, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox48Strings, wxCB_DROPDOWN );
  itemBoxSizer46->Add(itemComboBox48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer49Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Autotype"));
  wxStaticBoxSizer* itemStaticBoxSizer49 = new wxStaticBoxSizer(itemStaticBoxSizer49Static, wxVERTICAL);
  itemBoxSizer42->Add(itemStaticBoxSizer49, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox50 = new wxCheckBox( itemPanel41, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox50->SetValue(false);
  itemStaticBoxSizer49->Add(itemCheckBox50, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer51 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer51, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText52 = new wxStaticText( itemPanel41, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer51->Add(itemStaticText52, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl53 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer51->Add(itemTextCtrl53, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer54Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Default Username"));
  wxStaticBoxSizer* itemStaticBoxSizer54 = new wxStaticBoxSizer(itemStaticBoxSizer54Static, wxVERTICAL);
  itemBoxSizer42->Add(itemStaticBoxSizer54, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer55 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer54->Add(itemBoxSizer55, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox56 = new wxCheckBox( itemPanel41, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox56->SetValue(false);
  itemBoxSizer55->Add(itemCheckBox56, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl57 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer55->Add(itemTextCtrl57, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText58 = new wxStaticText( itemPanel41, wxID_STATIC, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer55->Add(itemStaticText58, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox59 = new wxCheckBox( itemPanel41, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox59->SetValue(false);
  itemStaticBoxSizer54->Add(itemCheckBox59, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer60Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Alternate Browser"));
  wxStaticBoxSizer* itemStaticBoxSizer60 = new wxStaticBoxSizer(itemStaticBoxSizer60Static, wxVERTICAL);
  itemBoxSizer42->Add(itemStaticBoxSizer60, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer61 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer60->Add(itemBoxSizer61, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl62 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
  itemBoxSizer61->Add(itemTextCtrl62, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton63 = new wxButton( itemPanel41, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer61->Add(itemButton63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer60->Add(itemBoxSizer64, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl65 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(60, -1)).x, -1), 0 );
  itemBoxSizer64->Add(itemTextCtrl65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText66 = new wxStaticText( itemPanel41, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer64->Add(itemStaticText66, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel41, _("Misc."));

  wxPanel* itemPanel67 = new wxPanel( GetBookCtrl(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxStaticBox* itemStaticBoxSizer68Static = new wxStaticBox(itemPanel67, wxID_ANY, _("Random password generation rules"));
  wxStaticBoxSizer* itemStaticBoxSizer68 = new wxStaticBoxSizer(itemStaticBoxSizer68Static, wxVERTICAL);
  itemPanel67->SetSizer(itemStaticBoxSizer68);

  wxBoxSizer* itemBoxSizer69 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer68->Add(itemBoxSizer69, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText70 = new wxStaticText( itemPanel67, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer69->Add(itemStaticText70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLenCtrl = new wxSpinCtrl( itemPanel67, ID_SPINCTRL3, _T("8"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 1024, 8 );
  itemBoxSizer69->Add(m_pwpLenCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr = new wxGridSizer(6, 2, 0, 0);
  itemStaticBoxSizer68->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);
  m_pwpUseLowerCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText75 = new wxStaticText( itemPanel67, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText75, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemPanel67, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(itemPanel67->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText77 = new wxStaticText( itemPanel67, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText77, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText80 = new wxStaticText( itemPanel67, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText80, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemPanel67, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(itemPanel67->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText82 = new wxStaticText( itemPanel67, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText82, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText85 = new wxStaticText( itemPanel67, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText85, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemPanel67, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(itemPanel67->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText87 = new wxStaticText( itemPanel67, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText90 = new wxStaticText( itemPanel67, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText90, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemPanel67, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(itemPanel67->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText92 = new wxStaticText( itemPanel67, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText92, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpEasyCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpPronounceCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText97 = new wxStaticText( itemPanel67, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer68->Add(itemStaticText97, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemPanel67, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer68->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel67, _("Password Policy"));

  wxPanel* itemPanel99 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer100 = new wxBoxSizer(wxVERTICAL);
  itemPanel99->SetSizer(itemBoxSizer100);

  wxBoxSizer* itemBoxSizer101 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer100->Add(itemBoxSizer101, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox102 = new wxCheckBox( itemPanel99, ID_CHECKBOX26, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox102->SetValue(false);
  itemBoxSizer101->Add(itemCheckBox102, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl103 = new wxSpinCtrl( itemPanel99, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxSize(itemPanel99->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer101->Add(itemSpinCtrl103, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText104 = new wxStaticText( itemPanel99, wxID_STATIC, _("previous passwords per entry"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer101->Add(itemStaticText104, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer105Static = new wxStaticBox(itemPanel99, wxID_ANY, _("Manage password history of current entries"));
  wxStaticBoxSizer* itemStaticBoxSizer105 = new wxStaticBoxSizer(itemStaticBoxSizer105Static, wxVERTICAL);
  itemBoxSizer100->Add(itemStaticBoxSizer105, 0, wxGROW|wxALL, 5);
  wxRadioButton* itemRadioButton106 = new wxRadioButton( itemPanel99, ID_RADIOBUTTON8, _("No change"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton106->SetValue(false);
  itemStaticBoxSizer105->Add(itemRadioButton106, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton107 = new wxRadioButton( itemPanel99, ID_RADIOBUTTON9, _("Stop saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton107->SetValue(false);
  itemStaticBoxSizer105->Add(itemRadioButton107, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton108 = new wxRadioButton( itemPanel99, ID_RADIOBUTTON10, _("Start saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton108->SetValue(false);
  itemStaticBoxSizer105->Add(itemRadioButton108, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton109 = new wxRadioButton( itemPanel99, ID_RADIOBUTTON11, _("Set maximum number of paswords saved to above value"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton109->SetValue(false);
  itemStaticBoxSizer105->Add(itemRadioButton109, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton110 = new wxButton( itemPanel99, ID_BUTTON9, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer105->Add(itemButton110, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel99, _("Password History"));

  wxPanel* itemPanel111 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer112 = new wxBoxSizer(wxVERTICAL);
  itemPanel111->SetSizer(itemBoxSizer112);

  wxCheckBox* itemCheckBox113 = new wxCheckBox( itemPanel111, ID_CHECKBOX27, _("Clear clipboard upon minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox113->SetValue(false);
  itemBoxSizer112->Add(itemCheckBox113, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox114 = new wxCheckBox( itemPanel111, ID_CHECKBOX, _("Clear clipboard upon exit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox114->SetValue(false);
  itemBoxSizer112->Add(itemCheckBox114, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox115 = new wxCheckBox( itemPanel111, ID_CHECKBOX1, _("Confirm item copy to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox115->SetValue(false);
  itemBoxSizer112->Add(itemCheckBox115, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox116 = new wxCheckBox( itemPanel111, ID_CHECKBOX2, _("Lock password database on minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox116->SetValue(false);
  itemBoxSizer112->Add(itemCheckBox116, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox117 = new wxCheckBox( itemPanel111, ID_CHECKBOX28, _("Lock password database on workstation lock"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox117->SetValue(false);
  itemBoxSizer112->Add(itemCheckBox117, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer118 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer112->Add(itemBoxSizer118, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox119 = new wxCheckBox( itemPanel111, ID_CHECKBOX29, _("Lock password database after"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox119->SetValue(false);
  itemBoxSizer118->Add(itemCheckBox119, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl120 = new wxSpinCtrl( itemPanel111, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxSize(itemPanel111->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer118->Add(itemSpinCtrl120, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText121 = new wxStaticText( itemPanel111, wxID_STATIC, _("minutes idle"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer118->Add(itemStaticText121, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel111, _("Security"));

  wxPanel* itemPanel122 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer123 = new wxBoxSizer(wxVERTICAL);
  itemPanel122->SetSizer(itemBoxSizer123);

  wxStaticBox* itemStaticBoxSizer124Static = new wxStaticBox(itemPanel122, wxID_ANY, _("System Tray"));
  wxStaticBoxSizer* itemStaticBoxSizer124 = new wxStaticBoxSizer(itemStaticBoxSizer124Static, wxVERTICAL);
  itemBoxSizer123->Add(itemStaticBoxSizer124, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox125 = new wxCheckBox( itemPanel122, ID_CHECKBOX30, _("Put icon in System Tray"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox125->SetValue(false);
  itemStaticBoxSizer124->Add(itemCheckBox125, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer126 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer124->Add(itemBoxSizer126, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText127 = new wxStaticText( itemPanel122, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer126->Add(itemStaticText127, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl128 = new wxSpinCtrl( itemPanel122, ID_SPINCTRL13, _T("0"), wxDefaultPosition, wxSize(itemPanel122->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer126->Add(itemSpinCtrl128, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText129 = new wxStaticText( itemPanel122, wxID_STATIC, _("used entries in System Tray menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer126->Add(itemStaticText129, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox130 = new wxCheckBox( itemPanel122, ID_CHECKBOX31, _("Start PasswordSafe at Login"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox130->SetValue(false);
  itemStaticBoxSizer124->Add(itemCheckBox130, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer131Static = new wxStaticBox(itemPanel122, wxID_ANY, _("Recent PasswordSafe Databases"));
  wxStaticBoxSizer* itemStaticBoxSizer131 = new wxStaticBoxSizer(itemStaticBoxSizer131Static, wxVERTICAL);
  itemBoxSizer123->Add(itemStaticBoxSizer131, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer132 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer131->Add(itemBoxSizer132, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText133 = new wxStaticText( itemPanel122, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer132->Add(itemStaticText133, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl134 = new wxSpinCtrl( itemPanel122, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxSize(itemPanel122->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer132->Add(itemSpinCtrl134, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText135 = new wxStaticText( itemPanel122, wxID_STATIC, _("databases"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer132->Add(itemStaticText135, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox136 = new wxCheckBox( itemPanel122, ID_CHECKBOX32, _("Recent Databases on File Menu rather than as a sub-menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox136->SetValue(false);
  itemStaticBoxSizer131->Add(itemCheckBox136, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox137 = new wxCheckBox( itemPanel122, ID_CHECKBOX33, _("Open database as read-only by default"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox137->SetValue(false);
  itemBoxSizer123->Add(itemCheckBox137, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox138 = new wxCheckBox( itemPanel122, ID_CHECKBOX34, _("Allow multiple instances"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox138->SetValue(false);
  itemBoxSizer123->Add(itemCheckBox138, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel122, _("System"));

  wxPanel* itemPanel139 = new wxPanel( GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxGrid* itemGrid140 = new wxGrid( itemPanel139, ID_GRID1, wxDefaultPosition, itemPanel139->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid140->SetDefaultColSize(100);
  itemGrid140->SetDefaultRowSize(25);
  itemGrid140->SetColLabelSize(25);
  itemGrid140->SetRowLabelSize(50);
  itemGrid140->CreateGrid(50, 2, wxGrid::wxGridSelectCells);

  GetBookCtrl()->AddPage(itemPanel139, _("Shortcuts"));

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_saveimmediate) );
////@end COptions content construction
}


/*!
 * Should we show tooltips?
 */

bool COptions::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap COptions::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin COptions bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end COptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon COptions::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin COptions icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end COptions icon retrieval
}

void COptions::PrefsToPropSheet()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  // Backup-related preferences
  m_saveimmediate = prefs->GetPref(PWSprefs::SaveImmediately);
}

void COptions::PropSheetToPrefs()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  // Backup-related preferences
  prefs->SetPref(PWSprefs::SaveImmediately,m_saveimmediate);
}

void COptions::OnOk(wxCommandEvent& event)
{
  if (Validate() && TransferDataFromWindow()) {
    PropSheetToPrefs();
  }
  EndModal(wxID_OK);
}
