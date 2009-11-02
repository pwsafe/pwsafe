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

#include "wx/dirdlg.h"
#include "wx/debug.h"

#include "passwordsafeframe.h"
#include "optionspropsheet.h"
#include "corelib/PWSprefs.h"
#include "corelib/Util.h" // for datetime string

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
  EVT_CHECKBOX( ID_CHECKBOX11, COptions::OnBackupB4SaveClick )

  EVT_RADIOBUTTON( ID_RADIOBUTTON4, COptions::OnBuPrefix )

  EVT_RADIOBUTTON( ID_RADIOBUTTON5, COptions::OnBuPrefix )

  EVT_COMBOBOX( ID_COMBOBOX2, COptions::OnSuffixCBSet )

  EVT_RADIOBUTTON( ID_RADIOBUTTON6, COptions::OnBuDirRB )

  EVT_RADIOBUTTON( ID_RADIOBUTTON7, COptions::OnBuDirRB )

  EVT_BUTTON( ID_BUTTON, COptions::OnBuDirBrowseClick )

  EVT_CHECKBOX( ID_CHECKBOX13, COptions::OnShowUsernameInTreeCB )

////@end COptions event table entries

END_EVENT_TABLE()


const wxChar *BUSuffix[] = {
  _("None"),
  _("YYYYMMMDD_HHMMSS"),
  _("Incremented Number [001-999]"),
};

const wxArrayString BUSuffixArray(sizeof(BUSuffix)/sizeof(BUSuffix[0]),
                                  BUSuffix);

enum {NO_SFX, TS_SFX, INC_SFX};

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
  wxCommandEvent dummyEv;
  OnSuffixCBSet(dummyEv);
  OnBuDirRB(dummyEv);
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
  m_dfltbuprefixRB = NULL;
  m_usrbuprefixRB = NULL;
  m_usrbuprefixTxt = NULL;
  m_busuffixCB = NULL;
  m_bumaxinc = NULL;
  m_suffixExample = NULL;
  m_dfltbudirRB = NULL;
  m_usrbudirRB = NULL;
  m_usrbudirTxt = NULL;
  m_buDirBN = NULL;
  m_showpasswordintreeCB = NULL;
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

  m_dfltbuprefixRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON4, _("Database name"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  m_dfltbuprefixRB->SetValue(false);
  itemStaticBoxSizer7->Add(m_dfltbuprefixRB, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);
  m_usrbuprefixRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON5, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_usrbuprefixRB->SetValue(false);
  itemBoxSizer10->Add(m_usrbuprefixRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_usrbuprefixTxt = new wxTextCtrl( itemPanel2, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer10->Add(m_usrbuprefixTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine13 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer7->Add(itemStaticLine13, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemPanel2, wxID_STATIC, _("Suffix:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer15, 0, wxGROW|wxALL, 0);
  m_busuffixCB = new wxComboBox( itemPanel2, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(140, -1)).x, -1),
                                 BUSuffixArray, wxCB_DROPDOWN );
  itemBoxSizer15->Add(m_busuffixCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemPanel2, wxID_STATIC, _("Max."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_bumaxinc = new wxSpinCtrl( itemPanel2, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(25, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer15->Add(m_bumaxinc, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer19, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText20 = new wxStaticText( itemPanel2, wxID_STATIC, _("Example:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer19->Add(itemStaticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_suffixExample = new wxStaticText( itemPanel2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(160, -1)).x, -1), 0 );
  itemBoxSizer19->Add(m_suffixExample, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine22 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer5->Add(itemStaticLine22, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText23 = new wxStaticText( itemPanel2, wxID_STATIC, _("Backup directory:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer5->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALL, 5);

  m_dfltbudirRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON6, _("Same as database's"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  m_dfltbudirRB->SetValue(false);
  itemStaticBoxSizer5->Add(m_dfltbudirRB, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer5->Add(itemBoxSizer25, 0, wxGROW|wxALL, 0);
  m_usrbudirRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON7, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_usrbudirRB->SetValue(false);
  itemBoxSizer25->Add(m_usrbudirRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_usrbudirTxt = new wxTextCtrl( itemPanel2, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer25->Add(m_usrbudirTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_buDirBN = new wxButton( itemPanel2, ID_BUTTON, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(m_buDirBN, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  GetBookCtrl()->AddPage(itemPanel2, _("Backups"));

  wxPanel* itemPanel29 = new wxPanel( GetBookCtrl(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxVERTICAL);
  itemPanel29->SetSizer(itemBoxSizer30);

  wxCheckBox* itemCheckBox31 = new wxCheckBox( itemPanel29, ID_CHECKBOX12, _("Always keep Password Safe on top"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox31->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox31, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox32 = new wxCheckBox( itemPanel29, ID_CHECKBOX13, _("Show Username in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox32->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox32, 0, wxALIGN_LEFT|wxALL, 5);

  m_showpasswordintreeCB = new wxCheckBox( itemPanel29, ID_CHECKBOX14, _("Show Password in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  m_showpasswordintreeCB->SetValue(false);
  itemBoxSizer30->Add(m_showpasswordintreeCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox34 = new wxCheckBox( itemPanel29, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox34->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox34, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox35 = new wxCheckBox( itemPanel29, ID_CHECKBOX16, _("Show Password in Add & Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox35->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox35, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox36 = new wxCheckBox( itemPanel29, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox36->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox36, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox37 = new wxCheckBox( itemPanel29, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox37->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox37, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer38 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer30->Add(itemBoxSizer38, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox39 = new wxCheckBox( itemPanel29, ID_CHECKBOX19, _("Warn"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox39->SetValue(false);
  itemBoxSizer38->Add(itemCheckBox39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl40 = new wxSpinCtrl( itemPanel29, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxSize(itemPanel29->ConvertDialogToPixels(wxSize(25, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer38->Add(itemSpinCtrl40, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText41 = new wxStaticText( itemPanel29, wxID_STATIC, _("days before passwords expire"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer38->Add(itemStaticText41, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemRadioBox42Strings;
  itemRadioBox42Strings.Add(_("&Fully collapsed"));
  itemRadioBox42Strings.Add(_("&Fully expanded"));
  itemRadioBox42Strings.Add(_("&Same as when last saved"));
  wxRadioBox* itemRadioBox42 = new wxRadioBox( itemPanel29, ID_RADIOBOX, _("Initial Tree View"), wxDefaultPosition, wxDefaultSize, itemRadioBox42Strings, 1, wxRA_SPECIFY_COLS );
  itemRadioBox42->SetSelection(0);
  itemBoxSizer30->Add(itemRadioBox42, 0, wxGROW|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel29, _("Display"));

  wxPanel* itemPanel43 = new wxPanel( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxVERTICAL);
  itemPanel43->SetSizer(itemBoxSizer44);

  wxCheckBox* itemCheckBox45 = new wxCheckBox( itemPanel43, ID_CHECKBOX20, _("Confirm deletion of items"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox45->SetValue(false);
  itemBoxSizer44->Add(itemCheckBox45, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox46 = new wxCheckBox( itemPanel43, ID_CHECKBOX21, _("Record last access times"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox46->SetValue(false);
  itemBoxSizer44->Add(itemCheckBox46, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox47 = new wxCheckBox( itemPanel43, ID_CHECKBOX22, _("Escape key closes application"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox47->SetValue(false);
  itemBoxSizer44->Add(itemCheckBox47, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer48 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer44->Add(itemBoxSizer48, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText49 = new wxStaticText( itemPanel43, wxID_STATIC, _("Double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer48->Add(itemStaticText49, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemComboBox50Strings;
  wxComboBox* itemComboBox50 = new wxComboBox( itemPanel43, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox50Strings, wxCB_DROPDOWN );
  itemBoxSizer48->Add(itemComboBox50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer51Static = new wxStaticBox(itemPanel43, wxID_ANY, _("Autotype"));
  wxStaticBoxSizer* itemStaticBoxSizer51 = new wxStaticBoxSizer(itemStaticBoxSizer51Static, wxVERTICAL);
  itemBoxSizer44->Add(itemStaticBoxSizer51, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox52 = new wxCheckBox( itemPanel43, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox52->SetValue(false);
  itemStaticBoxSizer51->Add(itemCheckBox52, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer53 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer51->Add(itemBoxSizer53, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText54 = new wxStaticText( itemPanel43, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer53->Add(itemStaticText54, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl55 = new wxTextCtrl( itemPanel43, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(itemPanel43->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer53->Add(itemTextCtrl55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer56Static = new wxStaticBox(itemPanel43, wxID_ANY, _("Default Username"));
  wxStaticBoxSizer* itemStaticBoxSizer56 = new wxStaticBoxSizer(itemStaticBoxSizer56Static, wxVERTICAL);
  itemBoxSizer44->Add(itemStaticBoxSizer56, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer56->Add(itemBoxSizer57, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox58 = new wxCheckBox( itemPanel43, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox58->SetValue(false);
  itemBoxSizer57->Add(itemCheckBox58, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl59 = new wxTextCtrl( itemPanel43, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxSize(itemPanel43->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer57->Add(itemTextCtrl59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText60 = new wxStaticText( itemPanel43, wxID_STATIC, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer57->Add(itemStaticText60, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox61 = new wxCheckBox( itemPanel43, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox61->SetValue(false);
  itemStaticBoxSizer56->Add(itemCheckBox61, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer62Static = new wxStaticBox(itemPanel43, wxID_ANY, _("Alternate Browser"));
  wxStaticBoxSizer* itemStaticBoxSizer62 = new wxStaticBoxSizer(itemStaticBoxSizer62Static, wxVERTICAL);
  itemBoxSizer44->Add(itemStaticBoxSizer62, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer63 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer62->Add(itemBoxSizer63, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl64 = new wxTextCtrl( itemPanel43, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxSize(itemPanel43->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
  itemBoxSizer63->Add(itemTextCtrl64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton65 = new wxButton( itemPanel43, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer63->Add(itemButton65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer62->Add(itemBoxSizer66, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl67 = new wxTextCtrl( itemPanel43, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(itemPanel43->ConvertDialogToPixels(wxSize(60, -1)).x, -1), 0 );
  itemBoxSizer66->Add(itemTextCtrl67, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText68 = new wxStaticText( itemPanel43, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer66->Add(itemStaticText68, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel43, _("Misc."));

  wxPanel* itemPanel69 = new wxPanel( GetBookCtrl(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxStaticBox* itemStaticBoxSizer70Static = new wxStaticBox(itemPanel69, wxID_ANY, _("Random password generation rules"));
  wxStaticBoxSizer* itemStaticBoxSizer70 = new wxStaticBoxSizer(itemStaticBoxSizer70Static, wxVERTICAL);
  itemPanel69->SetSizer(itemStaticBoxSizer70);

  wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer70->Add(itemBoxSizer71, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText72 = new wxStaticText( itemPanel69, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer71->Add(itemStaticText72, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLenCtrl = new wxSpinCtrl( itemPanel69, ID_SPINCTRL3, _T("8"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 1024, 8 );
  itemBoxSizer71->Add(m_pwpLenCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr = new wxGridSizer(6, 2, 0, 0);
  itemStaticBoxSizer70->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);
  m_pwpUseLowerCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText77 = new wxStaticText( itemPanel69, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText77, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemPanel69, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(itemPanel69->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText79 = new wxStaticText( itemPanel69, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText79, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText82 = new wxStaticText( itemPanel69, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText82, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemPanel69, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(itemPanel69->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText84 = new wxStaticText( itemPanel69, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText84, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText87 = new wxStaticText( itemPanel69, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemPanel69, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(itemPanel69->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText89 = new wxStaticText( itemPanel69, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText89, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText92 = new wxStaticText( itemPanel69, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText92, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemPanel69, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(itemPanel69->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText94 = new wxStaticText( itemPanel69, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText94, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpEasyCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpPronounceCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText99 = new wxStaticText( itemPanel69, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer70->Add(itemStaticText99, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemPanel69, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer70->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel69, _("Password Policy"));

  wxPanel* itemPanel101 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxVERTICAL);
  itemPanel101->SetSizer(itemBoxSizer102);

  wxBoxSizer* itemBoxSizer103 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer102->Add(itemBoxSizer103, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox104 = new wxCheckBox( itemPanel101, ID_CHECKBOX26, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox104->SetValue(false);
  itemBoxSizer103->Add(itemCheckBox104, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl105 = new wxSpinCtrl( itemPanel101, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxSize(itemPanel101->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer103->Add(itemSpinCtrl105, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText106 = new wxStaticText( itemPanel101, wxID_STATIC, _("previous passwords per entry"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer103->Add(itemStaticText106, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer107Static = new wxStaticBox(itemPanel101, wxID_ANY, _("Manage password history of current entries"));
  wxStaticBoxSizer* itemStaticBoxSizer107 = new wxStaticBoxSizer(itemStaticBoxSizer107Static, wxVERTICAL);
  itemBoxSizer102->Add(itemStaticBoxSizer107, 0, wxGROW|wxALL, 5);
  wxRadioButton* itemRadioButton108 = new wxRadioButton( itemPanel101, ID_RADIOBUTTON8, _("No change"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton108->SetValue(false);
  itemStaticBoxSizer107->Add(itemRadioButton108, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton109 = new wxRadioButton( itemPanel101, ID_RADIOBUTTON9, _("Stop saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton109->SetValue(false);
  itemStaticBoxSizer107->Add(itemRadioButton109, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton110 = new wxRadioButton( itemPanel101, ID_RADIOBUTTON10, _("Start saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton110->SetValue(false);
  itemStaticBoxSizer107->Add(itemRadioButton110, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton111 = new wxRadioButton( itemPanel101, ID_RADIOBUTTON11, _("Set maximum number of paswords saved to above value"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton111->SetValue(false);
  itemStaticBoxSizer107->Add(itemRadioButton111, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton112 = new wxButton( itemPanel101, ID_BUTTON9, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer107->Add(itemButton112, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel101, _("Password History"));

  wxPanel* itemPanel113 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer114 = new wxBoxSizer(wxVERTICAL);
  itemPanel113->SetSizer(itemBoxSizer114);

  wxCheckBox* itemCheckBox115 = new wxCheckBox( itemPanel113, ID_CHECKBOX27, _("Clear clipboard upon minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox115->SetValue(false);
  itemBoxSizer114->Add(itemCheckBox115, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox116 = new wxCheckBox( itemPanel113, ID_CHECKBOX, _("Clear clipboard upon exit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox116->SetValue(false);
  itemBoxSizer114->Add(itemCheckBox116, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox117 = new wxCheckBox( itemPanel113, ID_CHECKBOX1, _("Confirm item copy to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox117->SetValue(false);
  itemBoxSizer114->Add(itemCheckBox117, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox118 = new wxCheckBox( itemPanel113, ID_CHECKBOX2, _("Lock password database on minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox118->SetValue(false);
  itemBoxSizer114->Add(itemCheckBox118, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox119 = new wxCheckBox( itemPanel113, ID_CHECKBOX28, _("Lock password database on workstation lock"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox119->SetValue(false);
  itemBoxSizer114->Add(itemCheckBox119, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer120 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer114->Add(itemBoxSizer120, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox121 = new wxCheckBox( itemPanel113, ID_CHECKBOX29, _("Lock password database after"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox121->SetValue(false);
  itemBoxSizer120->Add(itemCheckBox121, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl122 = new wxSpinCtrl( itemPanel113, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxSize(itemPanel113->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer120->Add(itemSpinCtrl122, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText123 = new wxStaticText( itemPanel113, wxID_STATIC, _("minutes idle"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer120->Add(itemStaticText123, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel113, _("Security"));

  wxPanel* itemPanel124 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer125 = new wxBoxSizer(wxVERTICAL);
  itemPanel124->SetSizer(itemBoxSizer125);

  wxStaticBox* itemStaticBoxSizer126Static = new wxStaticBox(itemPanel124, wxID_ANY, _("System Tray"));
  wxStaticBoxSizer* itemStaticBoxSizer126 = new wxStaticBoxSizer(itemStaticBoxSizer126Static, wxVERTICAL);
  itemBoxSizer125->Add(itemStaticBoxSizer126, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox127 = new wxCheckBox( itemPanel124, ID_CHECKBOX30, _("Put icon in System Tray"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox127->SetValue(false);
  itemStaticBoxSizer126->Add(itemCheckBox127, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer128 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer126->Add(itemBoxSizer128, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText129 = new wxStaticText( itemPanel124, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer128->Add(itemStaticText129, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl130 = new wxSpinCtrl( itemPanel124, ID_SPINCTRL13, _T("0"), wxDefaultPosition, wxSize(itemPanel124->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer128->Add(itemSpinCtrl130, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText131 = new wxStaticText( itemPanel124, wxID_STATIC, _("used entries in System Tray menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer128->Add(itemStaticText131, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox132 = new wxCheckBox( itemPanel124, ID_CHECKBOX31, _("Start PasswordSafe at Login"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox132->SetValue(false);
  itemStaticBoxSizer126->Add(itemCheckBox132, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer133Static = new wxStaticBox(itemPanel124, wxID_ANY, _("Recent PasswordSafe Databases"));
  wxStaticBoxSizer* itemStaticBoxSizer133 = new wxStaticBoxSizer(itemStaticBoxSizer133Static, wxVERTICAL);
  itemBoxSizer125->Add(itemStaticBoxSizer133, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer134 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer133->Add(itemBoxSizer134, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText135 = new wxStaticText( itemPanel124, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer134->Add(itemStaticText135, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl136 = new wxSpinCtrl( itemPanel124, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxSize(itemPanel124->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer134->Add(itemSpinCtrl136, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText137 = new wxStaticText( itemPanel124, wxID_STATIC, _("databases"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer134->Add(itemStaticText137, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox138 = new wxCheckBox( itemPanel124, ID_CHECKBOX32, _("Recent Databases on File Menu rather than as a sub-menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox138->SetValue(false);
  itemStaticBoxSizer133->Add(itemCheckBox138, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox139 = new wxCheckBox( itemPanel124, ID_CHECKBOX33, _("Open database as read-only by default"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox139->SetValue(false);
  itemBoxSizer125->Add(itemCheckBox139, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox140 = new wxCheckBox( itemPanel124, ID_CHECKBOX34, _("Allow multiple instances"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox140->SetValue(false);
  itemBoxSizer125->Add(itemCheckBox140, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel124, _("System"));

  wxPanel* itemPanel141 = new wxPanel( GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxGrid* itemGrid142 = new wxGrid( itemPanel141, ID_GRID1, wxDefaultPosition, itemPanel141->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid142->SetDefaultColSize(100);
  itemGrid142->SetDefaultRowSize(25);
  itemGrid142->SetColLabelSize(25);
  itemGrid142->SetRowLabelSize(50);
  itemGrid142->CreateGrid(50, 2, wxGrid::wxGridSelectCells);

  GetBookCtrl()->AddPage(itemPanel141, _("Shortcuts"));

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_saveimmediate) );
  itemCheckBox6->SetValidator( wxGenericValidator(& m_backupb4save) );
  itemCheckBox31->SetValidator( wxGenericValidator(& m_alwaysontop) );
  itemCheckBox32->SetValidator( wxGenericValidator(& m_showusernameintree) );
  // Connect events and objects
  m_usrbuprefixTxt->Connect(ID_TEXTCTRL9, wxEVT_SET_FOCUS, wxFocusEventHandler(COptions::OnBuPrefixTxtSetFocus), NULL, this);
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
  m_backupb4save = prefs->GetPref(PWSprefs::BackupBeforeEverySave);
  wxString buprefixValue = prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
  m_dfltbuprefixRB->SetValue(buprefixValue.empty());
  m_usrbuprefixRB->SetValue(!buprefixValue.empty());
  m_usrbuprefixTxt->SetValue(buprefixValue);
  int suffixIndex = prefs->GetPref(PWSprefs::BackupSuffix);
  m_busuffixCB->SetValue(BUSuffix[suffixIndex]);
  m_bumaxinc->SetValue(prefs->GetPref(PWSprefs::BackupMaxIncremented));
  wxString budirValue = prefs->GetPref(PWSprefs::BackupDir).c_str();
  m_dfltbudirRB->SetValue(budirValue.empty());
  m_usrbudirRB->SetValue(!budirValue.empty());
  m_usrbudirTxt->SetValue(budirValue);

  // display-related preferences
  m_alwaysontop = prefs->GetPref(PWSprefs::AlwaysOnTop);
  m_showusernameintree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  m_showpasswordintreeCB->SetValue(!m_showusernameintree && prefs->
                                   GetPref(PWSprefs::ShowPasswordInTree));
  m_showpasswordintreeCB->Enable(!m_showusernameintree);
}

void COptions::PropSheetToPrefs()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  // Backup-related preferences
  prefs->SetPref(PWSprefs::SaveImmediately, m_saveimmediate);
  prefs->SetPref(PWSprefs::BackupBeforeEverySave, m_backupb4save);
  wxString buprefixValue;
  if (m_usrbuprefixRB->GetValue())
    buprefixValue = m_usrbuprefixTxt->GetValue();
  prefs->SetPref(PWSprefs::BackupPrefixValue, buprefixValue.c_str());
  int suffixIndex = m_busuffixCB->GetCurrentSelection();
  prefs->SetPref(PWSprefs::BackupSuffix, suffixIndex);
  if (suffixIndex == INC_SFX)
    prefs->SetPref(PWSprefs::BackupMaxIncremented, suffixIndex);
  wxString budirValue;
  if (m_usrbudirRB->GetValue())
    budirValue = m_usrbudirTxt->GetValue();
  prefs->SetPref(PWSprefs::BackupDir, budirValue.c_str());

  // display-related preferences
  prefs->SetPref(PWSprefs::AlwaysOnTop, m_alwaysontop);
  // set/clear wxSTAY_ON_TOP flag accrdingly:
  long flags = GetParent()->GetWindowStyleFlag();
  if (m_alwaysontop)
    flags |= wxSTAY_ON_TOP;
  else
    flags &= ~wxSTAY_ON_TOP;
  GetParent()->SetWindowStyleFlag(flags);

  bool oldshowuserpref = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  bool oldshowpswdpref = prefs->GetPref(PWSprefs::ShowPasswordInTree);
  prefs->SetPref(PWSprefs::ShowUsernameInTree, m_showusernameintree);
  prefs->SetPref(PWSprefs::ShowPasswordInTree,
                 m_showpasswordintreeCB->GetValue());

  bool showprefchanged = (oldshowuserpref != prefs->
                          GetPref(PWSprefs::ShowUsernameInTree) ||
                          oldshowpswdpref != prefs->
                          GetPref(PWSprefs::ShowPasswordInTree));
  if (showprefchanged) {
    PasswordSafeFrame *pwsframe = dynamic_cast<PasswordSafeFrame*>(GetParent());
    wxASSERT(pwsframe != NULL);
    if (pwsframe->IsTreeView())
      pwsframe->RefreshView();
  }
}

void COptions::OnOk(wxCommandEvent& event)
{
  if (Validate() && TransferDataFromWindow()) {
    PropSheetToPrefs();
  }
  EndModal(wxID_OK);
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX11
 */

void COptions::OnBackupB4SaveClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    m_dfltbuprefixRB->Enable(m_backupb4save);
    m_usrbuprefixRB->Enable(m_backupb4save);
    m_usrbuprefixTxt->Enable(m_backupb4save);
    m_busuffixCB->Enable(m_backupb4save);
    m_bumaxinc->Enable(m_backupb4save);
  }
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON4
 */

void COptions::OnBuPrefix( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON4 in COptions.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON4 in COptions. 
}


/*!
 * wxEVT_SET_FOCUS event handler for ID_TEXTCTRL9
 */

void COptions::OnBuPrefixTxtSetFocus( wxFocusEvent& event )
{
  m_dfltbuprefixRB->SetValue(false);
  m_usrbuprefixRB->SetValue(true);
}


/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX2
 */

void COptions::OnSuffixCBSet( wxCommandEvent& event )
{
  int suffixIndex = m_busuffixCB->GetCurrentSelection();
  wxString example = m_usrbuprefixTxt->GetValue();

  if (example.empty())
    example = _("pwsafe"); // XXXX get current file's basename!

  m_bumaxinc->Enable(suffixIndex == INC_SFX);
  switch (suffixIndex) {
  case NO_SFX:
    m_suffixExample->SetLabel(_(""));
    break;
  case TS_SFX: {
    time_t now;
    time(&now);
    wxString datetime = PWSUtil::ConvertToDateTimeString(now,
                                                         TMC_EXPORT_IMPORT).c_str();
      example += L"_";
      example = example + datetime.Left(4) +  // YYYY
        datetime.Mid(5,2) +  // MM
        datetime.Mid(8,2) +  // DD
        L"_" +
        datetime.Mid(11,2) +  // HH
        datetime.Mid(14,2) +  // MM
        datetime.Mid(17,2);   // SS
  }
    break;
  case INC_SFX:
    example += L"_001";
    break;
  default:
    break;
  }
  m_suffixExample->SetLabel(example);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void COptions::OnBuDirBrowseClick( wxCommandEvent& event )
{
  wxDirDialog dirdlg(this);
  int status = dirdlg.ShowModal();
  if (status == wxID_OK)
    m_usrbudirTxt->SetValue(dirdlg.GetPath());
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON6
 */

void COptions::OnBuDirRB( wxCommandEvent& event )
{
    bool enable = m_usrbudirRB->GetValue();
    m_usrbudirTxt->Enable(enable);
    m_buDirBN->Enable(enable);
}



/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX13
 */

void COptions::OnShowUsernameInTreeCB( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    if (m_showusernameintree)
      m_showpasswordintreeCB->SetValue(false);
    m_showpasswordintreeCB->Enable(!m_showusernameintree);
  }
}

