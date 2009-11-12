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
#include "wx/msgdlg.h"
#include "wx/debug.h"

#include "passwordsafeframe.h"
#include "optionspropsheet.h"
#include "corelib/PWSprefs.h"
#include "corelib/Util.h" // for datetime string
#include "corelib/PWSAuxParse.h" // for DEFAULT_AUTOTYPE

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

  EVT_CHECKBOX( ID_CHECKBOX19, COptions::OnPreExpiryWarnClick )

  EVT_CHECKBOX( ID_CHECKBOX24, COptions::OnUseDefaultUserClick )

  EVT_BUTTON( ID_BUTTON8, COptions::OnBrowseLocationClick )

  EVT_CHECKBOX( ID_CHECKBOX3, COptions::OnPwPolUseClick )

  EVT_CHECKBOX( ID_CHECKBOX4, COptions::OnPwPolUseClick )

  EVT_CHECKBOX( ID_CHECKBOX5, COptions::OnPwPolUseClick )

  EVT_CHECKBOX( ID_CHECKBOX6, COptions::OnPwPolUseClick )

  EVT_CHECKBOX( ID_CHECKBOX7, COptions::OnPwPolUseClick )

  EVT_CHECKBOX( ID_CHECKBOX8, COptions::OnPwPolUseClick )

  EVT_CHECKBOX( ID_CHECKBOX9, COptions::OnPwPolUseClick )

////@end COptions event table entries

END_EVENT_TABLE()

const wxChar *BUSuffix[] = {
  _("None"),
  _("YYYYMMMDD_HHMMSS"),
  _("Incremented Number [001-999]"),
};

enum {NO_SFX, TS_SFX, INC_SFX}; // For backup file suffix name

// Following in enum order (see PWSprefs.h)
const wxChar *DCAStrings[] = {
  _("Copy password to clipboard"),
  _("View/Edit selected entry"),
  _("Autotype"),
  _("Browse to URL"),
  _("Copy notes to clipboard"),
  _("Copy username to clipboard"),
  _("Copy password to clipboard, minimize"),
  _("Browse to URL + Autotype"),
  _("Run Command"),
  _("Send email"),
};

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
  OnPwPolUseClick(dummyEv);
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
  m_preexpirywarnCB = NULL;
  m_preexpirywarndaysSB = NULL;
  m_DCACB = NULL;
  m_defusernameTXT = NULL;
  m_defusernameLBL = NULL;
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
  wxArrayString m_busuffixCBStrings;
  m_busuffixCBStrings.Add(_("None"));
  m_busuffixCBStrings.Add(_("YYYYMMMDD_HHMMSS"));
  m_busuffixCBStrings.Add(_("Incremented Number [001-999]"));
  m_busuffixCB = new wxComboBox( itemPanel2, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(140, -1)).x, -1), m_busuffixCBStrings, wxCB_READONLY );
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

  wxStaticText* itemStaticText34 = new wxStaticText( itemPanel29, wxID_STATIC, _("Example:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(itemStaticText34, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox35 = new wxCheckBox( itemPanel29, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox35->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox35, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox36 = new wxCheckBox( itemPanel29, ID_CHECKBOX16, _("Show Password in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox36->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox36, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox37 = new wxCheckBox( itemPanel29, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox37->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox37, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox38 = new wxCheckBox( itemPanel29, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox38->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox38, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer39 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer30->Add(itemBoxSizer39, 0, wxGROW|wxALL, 0);
  m_preexpirywarnCB = new wxCheckBox( itemPanel29, ID_CHECKBOX19, _("Warn"), wxDefaultPosition, wxDefaultSize, 0 );
  m_preexpirywarnCB->SetValue(false);
  itemBoxSizer39->Add(m_preexpirywarnCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_preexpirywarndaysSB = new wxSpinCtrl( itemPanel29, ID_SPINCTRL10, _T("1"), wxDefaultPosition, wxSize(itemPanel29->ConvertDialogToPixels(wxSize(25, -1)).x, -1), wxSP_ARROW_KEYS, 1, 30, 1 );
  itemBoxSizer39->Add(m_preexpirywarndaysSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText42 = new wxStaticText( itemPanel29, wxID_STATIC, _("days before passwords expire"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer39->Add(itemStaticText42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemRadioBox43Strings;
  itemRadioBox43Strings.Add(_("&Fully collapsed"));
  itemRadioBox43Strings.Add(_("&Fully expanded"));
  itemRadioBox43Strings.Add(_("&Same as when last saved"));
  wxRadioBox* itemRadioBox43 = new wxRadioBox( itemPanel29, ID_RADIOBOX, _("Initial Tree View"), wxDefaultPosition, wxDefaultSize, itemRadioBox43Strings, 1, wxRA_SPECIFY_COLS );
  itemRadioBox43->SetSelection(0);
  itemBoxSizer30->Add(itemRadioBox43, 0, wxGROW|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel29, _("Display"));

  wxPanel* itemPanel44 = new wxPanel( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
  itemPanel44->SetSizer(itemBoxSizer45);

  wxCheckBox* itemCheckBox46 = new wxCheckBox( itemPanel44, ID_CHECKBOX20, _("Confirm deletion of items"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox46->SetValue(false);
  itemBoxSizer45->Add(itemCheckBox46, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox47 = new wxCheckBox( itemPanel44, ID_CHECKBOX21, _("Record last access times"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox47->SetValue(false);
  itemBoxSizer45->Add(itemCheckBox47, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox48 = new wxCheckBox( itemPanel44, ID_CHECKBOX22, _("Escape key closes application"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox48->SetValue(false);
  itemBoxSizer45->Add(itemCheckBox48, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer45->Add(itemBoxSizer49, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText50 = new wxStaticText( itemPanel44, wxID_STATIC, _("Double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer49->Add(itemStaticText50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_DCACBStrings;
  m_DCACBStrings.Add(_("Autotype"));
  m_DCACBStrings.Add(_("Browse to URL"));
  m_DCACBStrings.Add(_("Browse to URL + Autotype"));
  m_DCACBStrings.Add(_("Copy notes to clipboard"));
  m_DCACBStrings.Add(_("Copy password to clipboard"));
  m_DCACBStrings.Add(_("Copy password to clipboard, minimize"));
  m_DCACBStrings.Add(_("Copy username to clipboard"));
  m_DCACBStrings.Add(_("Run Command"));
  m_DCACBStrings.Add(_("Send email"));
  m_DCACBStrings.Add(_("View/Edit selected entry"));
  m_DCACB = new wxComboBox( itemPanel44, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_DCACBStrings, wxCB_READONLY );
  itemBoxSizer49->Add(m_DCACB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer52Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Autotype"));
  wxStaticBoxSizer* itemStaticBoxSizer52 = new wxStaticBoxSizer(itemStaticBoxSizer52Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer52, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox53 = new wxCheckBox( itemPanel44, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox53->SetValue(false);
  itemStaticBoxSizer52->Add(itemCheckBox53, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer54 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer52->Add(itemBoxSizer54, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText55 = new wxStaticText( itemPanel44, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer54->Add(itemStaticText55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl56 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer54->Add(itemTextCtrl56, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer57Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Default Username"));
  wxStaticBoxSizer* itemStaticBoxSizer57 = new wxStaticBoxSizer(itemStaticBoxSizer57Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer57, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer57->Add(itemBoxSizer58, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox59 = new wxCheckBox( itemPanel44, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox59->SetValue(false);
  itemBoxSizer58->Add(itemCheckBox59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_defusernameTXT = new wxTextCtrl( itemPanel44, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer58->Add(m_defusernameTXT, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_defusernameLBL = new wxStaticText( itemPanel44, wxID_STATIC, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(m_defusernameLBL, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox62 = new wxCheckBox( itemPanel44, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox62->SetValue(false);
  itemStaticBoxSizer57->Add(itemCheckBox62, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer63Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Alternate Browser"));
  wxStaticBoxSizer* itemStaticBoxSizer63 = new wxStaticBoxSizer(itemStaticBoxSizer63Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer63, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer63->Add(itemBoxSizer64, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl65 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
  itemBoxSizer64->Add(itemTextCtrl65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton66 = new wxButton( itemPanel44, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer64->Add(itemButton66, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer63->Add(itemBoxSizer67, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl68 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(60, -1)).x, -1), 0 );
  itemBoxSizer67->Add(itemTextCtrl68, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText69 = new wxStaticText( itemPanel44, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer67->Add(itemStaticText69, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel44, _("Misc."));

  wxPanel* itemPanel70 = new wxPanel( GetBookCtrl(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxStaticBox* itemStaticBoxSizer71Static = new wxStaticBox(itemPanel70, wxID_ANY, _("Random password generation rules"));
  wxStaticBoxSizer* itemStaticBoxSizer71 = new wxStaticBoxSizer(itemStaticBoxSizer71Static, wxVERTICAL);
  itemPanel70->SetSizer(itemStaticBoxSizer71);

  wxBoxSizer* itemBoxSizer72 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer71->Add(itemBoxSizer72, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText73 = new wxStaticText( itemPanel70, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer72->Add(itemStaticText73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl74 = new wxSpinCtrl( itemPanel70, ID_SPINCTRL3, _T("8"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 1024, 8 );
  itemBoxSizer72->Add(itemSpinCtrl74, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr = new wxGridSizer(6, 2, 0, 0);
  itemStaticBoxSizer71->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);
  m_pwpUseLowerCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText78 = new wxStaticText( itemPanel70, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemPanel70, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(itemPanel70->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText80 = new wxStaticText( itemPanel70, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText80, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText83 = new wxStaticText( itemPanel70, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText83, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemPanel70, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(itemPanel70->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText85 = new wxStaticText( itemPanel70, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText85, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText88 = new wxStaticText( itemPanel70, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText88, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemPanel70, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(itemPanel70->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText90 = new wxStaticText( itemPanel70, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText90, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText93 = new wxStaticText( itemPanel70, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText93, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemPanel70, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(itemPanel70->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText95 = new wxStaticText( itemPanel70, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText95, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpEasyCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpPronounceCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText100 = new wxStaticText( itemPanel70, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer71->Add(itemStaticText100, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemPanel70, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer71->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel70, _("Password Policy"));

  wxPanel* itemPanel102 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer103 = new wxBoxSizer(wxVERTICAL);
  itemPanel102->SetSizer(itemBoxSizer103);

  wxBoxSizer* itemBoxSizer104 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer103->Add(itemBoxSizer104, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox105 = new wxCheckBox( itemPanel102, ID_CHECKBOX26, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox105->SetValue(false);
  itemBoxSizer104->Add(itemCheckBox105, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl106 = new wxSpinCtrl( itemPanel102, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxSize(itemPanel102->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer104->Add(itemSpinCtrl106, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText107 = new wxStaticText( itemPanel102, wxID_STATIC, _("previous passwords per entry"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer104->Add(itemStaticText107, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer108Static = new wxStaticBox(itemPanel102, wxID_ANY, _("Manage password history of current entries"));
  wxStaticBoxSizer* itemStaticBoxSizer108 = new wxStaticBoxSizer(itemStaticBoxSizer108Static, wxVERTICAL);
  itemBoxSizer103->Add(itemStaticBoxSizer108, 0, wxGROW|wxALL, 5);
  wxRadioButton* itemRadioButton109 = new wxRadioButton( itemPanel102, ID_RADIOBUTTON8, _("No change"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton109->SetValue(false);
  itemStaticBoxSizer108->Add(itemRadioButton109, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton110 = new wxRadioButton( itemPanel102, ID_RADIOBUTTON9, _("Stop saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton110->SetValue(false);
  itemStaticBoxSizer108->Add(itemRadioButton110, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton111 = new wxRadioButton( itemPanel102, ID_RADIOBUTTON10, _("Start saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton111->SetValue(false);
  itemStaticBoxSizer108->Add(itemRadioButton111, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton112 = new wxRadioButton( itemPanel102, ID_RADIOBUTTON11, _("Set maximum number of paswords saved to above value"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton112->SetValue(false);
  itemStaticBoxSizer108->Add(itemRadioButton112, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton113 = new wxButton( itemPanel102, ID_BUTTON9, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer108->Add(itemButton113, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel102, _("Password History"));

  wxPanel* itemPanel114 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer115 = new wxBoxSizer(wxVERTICAL);
  itemPanel114->SetSizer(itemBoxSizer115);

  wxCheckBox* itemCheckBox116 = new wxCheckBox( itemPanel114, ID_CHECKBOX27, _("Clear clipboard upon minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox116->SetValue(false);
  itemBoxSizer115->Add(itemCheckBox116, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox117 = new wxCheckBox( itemPanel114, ID_CHECKBOX, _("Clear clipboard upon exit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox117->SetValue(false);
  itemBoxSizer115->Add(itemCheckBox117, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox118 = new wxCheckBox( itemPanel114, ID_CHECKBOX1, _("Confirm item copy to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox118->SetValue(false);
  itemBoxSizer115->Add(itemCheckBox118, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox119 = new wxCheckBox( itemPanel114, ID_CHECKBOX2, _("Lock password database on minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox119->SetValue(false);
  itemBoxSizer115->Add(itemCheckBox119, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox120 = new wxCheckBox( itemPanel114, ID_CHECKBOX28, _("Lock password database on workstation lock"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox120->SetValue(false);
  itemBoxSizer115->Add(itemCheckBox120, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer121 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer115->Add(itemBoxSizer121, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox122 = new wxCheckBox( itemPanel114, ID_CHECKBOX29, _("Lock password database after"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox122->SetValue(false);
  itemBoxSizer121->Add(itemCheckBox122, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl123 = new wxSpinCtrl( itemPanel114, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxSize(itemPanel114->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer121->Add(itemSpinCtrl123, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText124 = new wxStaticText( itemPanel114, wxID_STATIC, _("minutes idle"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer121->Add(itemStaticText124, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel114, _("Security"));

  wxPanel* itemPanel125 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer126 = new wxBoxSizer(wxVERTICAL);
  itemPanel125->SetSizer(itemBoxSizer126);

  wxStaticBox* itemStaticBoxSizer127Static = new wxStaticBox(itemPanel125, wxID_ANY, _("System Tray"));
  wxStaticBoxSizer* itemStaticBoxSizer127 = new wxStaticBoxSizer(itemStaticBoxSizer127Static, wxVERTICAL);
  itemBoxSizer126->Add(itemStaticBoxSizer127, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox128 = new wxCheckBox( itemPanel125, ID_CHECKBOX30, _("Put icon in System Tray"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox128->SetValue(false);
  itemStaticBoxSizer127->Add(itemCheckBox128, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer129 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer127->Add(itemBoxSizer129, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText130 = new wxStaticText( itemPanel125, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer129->Add(itemStaticText130, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl131 = new wxSpinCtrl( itemPanel125, ID_SPINCTRL13, _T("0"), wxDefaultPosition, wxSize(itemPanel125->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer129->Add(itemSpinCtrl131, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText132 = new wxStaticText( itemPanel125, wxID_STATIC, _("used entries in System Tray menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer129->Add(itemStaticText132, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox133 = new wxCheckBox( itemPanel125, ID_CHECKBOX31, _("Start PasswordSafe at Login"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox133->SetValue(false);
  itemStaticBoxSizer127->Add(itemCheckBox133, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer134Static = new wxStaticBox(itemPanel125, wxID_ANY, _("Recent PasswordSafe Databases"));
  wxStaticBoxSizer* itemStaticBoxSizer134 = new wxStaticBoxSizer(itemStaticBoxSizer134Static, wxVERTICAL);
  itemBoxSizer126->Add(itemStaticBoxSizer134, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer135 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer134->Add(itemBoxSizer135, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText136 = new wxStaticText( itemPanel125, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer135->Add(itemStaticText136, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl137 = new wxSpinCtrl( itemPanel125, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxSize(itemPanel125->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer135->Add(itemSpinCtrl137, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText138 = new wxStaticText( itemPanel125, wxID_STATIC, _("databases"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer135->Add(itemStaticText138, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox139 = new wxCheckBox( itemPanel125, ID_CHECKBOX32, _("Recent Databases on File Menu rather than as a sub-menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox139->SetValue(false);
  itemStaticBoxSizer134->Add(itemCheckBox139, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox140 = new wxCheckBox( itemPanel125, ID_CHECKBOX33, _("Open database as read-only by default"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox140->SetValue(false);
  itemBoxSizer126->Add(itemCheckBox140, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox141 = new wxCheckBox( itemPanel125, ID_CHECKBOX34, _("Allow multiple instances"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox141->SetValue(false);
  itemBoxSizer126->Add(itemCheckBox141, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel125, _("System"));

  wxPanel* itemPanel142 = new wxPanel( GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxGrid* itemGrid143 = new wxGrid( itemPanel142, ID_GRID1, wxDefaultPosition, itemPanel142->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid143->SetDefaultColSize(100);
  itemGrid143->SetDefaultRowSize(25);
  itemGrid143->SetColLabelSize(25);
  itemGrid143->SetRowLabelSize(50);
  itemGrid143->CreateGrid(50, 2, wxGrid::wxGridSelectCells);

  GetBookCtrl()->AddPage(itemPanel142, _("Shortcuts"));

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_saveimmediate) );
  itemCheckBox6->SetValidator( wxGenericValidator(& m_backupb4save) );
  itemCheckBox31->SetValidator( wxGenericValidator(& m_alwaysontop) );
  itemCheckBox32->SetValidator( wxGenericValidator(& m_showusernameintree) );
  itemCheckBox35->SetValidator( wxGenericValidator(& m_shownotesastipsinviews) );
  itemCheckBox36->SetValidator( wxGenericValidator(& m_pwshowinedit) );
  itemCheckBox37->SetValidator( wxGenericValidator(& m_notesshowinedit) );
  itemCheckBox38->SetValidator( wxGenericValidator(& m_wordwrapnotes) );
  m_preexpirywarnCB->SetValidator( wxGenericValidator(& m_preexpirywarn) );
  itemRadioBox43->SetValidator( wxGenericValidator(& m_inittreeview) );
  itemCheckBox46->SetValidator( wxGenericValidator(& m_confirmdelete) );
  itemCheckBox47->SetValidator( wxGenericValidator(& m_maintaindatetimestamps) );
  itemCheckBox48->SetValidator( wxGenericValidator(& m_escexits) );
  itemCheckBox53->SetValidator( wxGenericValidator(& m_minauto) );
  itemTextCtrl56->SetValidator( wxGenericValidator(& m_autotypeStr) );
  itemCheckBox59->SetValidator( wxGenericValidator(& m_usedefuser) );
  itemCheckBox62->SetValidator( wxGenericValidator(& m_querysetdef) );
  itemTextCtrl65->SetValidator( wxGenericValidator(& m_otherbrowser) );
  itemSpinCtrl74->SetValidator( wxGenericValidator(& m_pwdefaultlength) );
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
  m_shownotesastipsinviews = prefs->
    GetPref(PWSprefs::ShowNotesAsTooltipsInViews);
  m_pwshowinedit = prefs->GetPref(PWSprefs::ShowPWDefault);
  m_notesshowinedit = prefs->GetPref(PWSprefs::ShowNotesDefault);
  m_wordwrapnotes = prefs->GetPref(PWSprefs::NotesWordWrap);
  m_preexpirywarn = prefs->GetPref(PWSprefs::PreExpiryWarn);
  m_preexpirywarndaysSB->SetValue(prefs->GetPref(PWSprefs::PreExpiryWarnDays));
  m_preexpirywarndaysSB->Enable(m_preexpirywarn);
  m_inittreeview = prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen);

  // Misc. preferences
  m_confirmdelete = prefs->GetPref(PWSprefs::DeleteQuestion);
  m_maintaindatetimestamps = prefs->GetPref(PWSprefs::MaintainDateTimeStamps);
  m_escexits = prefs->GetPref(PWSprefs::EscExits);
  m_doubleclickaction = prefs->GetPref(PWSprefs::DoubleClickAction);
  wxASSERT(m_doubleclickaction >= 0 &&
           m_doubleclickaction < int(sizeof(DCAStrings)/sizeof(DCAStrings[0])));
  if (m_doubleclickaction < 0 ||
      m_doubleclickaction >= int(sizeof(DCAStrings)/sizeof(DCAStrings[0])))
    m_doubleclickaction = 0;
  m_DCACB->SetValue(DCAStrings[m_doubleclickaction]);
  m_minauto = prefs->GetPref(PWSprefs::MinimizeOnAutotype);
  m_autotypeStr = prefs->GetPref(PWSprefs::DefaultAutotypeString).c_str();
  if (m_autotypeStr.empty())
    m_autotypeStr = DEFAULT_AUTOTYPE;
  m_usedefuser = prefs->GetPref(PWSprefs::UseDefaultUser);
  m_defusernameTXT->SetValue(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
  m_defusernameTXT->Enable(m_usedefuser);
  m_defusernameLBL->Enable(m_usedefuser);
  m_querysetdef = prefs->GetPref(PWSprefs::QuerySetDef);
  m_otherbrowser = prefs->GetPref(PWSprefs::AltBrowser).c_str();
  m_otherbrowserparams = prefs->GetPref(PWSprefs::AltBrowserCmdLineParms).c_str();

  // Password Policy preferences
  m_pwdefaultlength = prefs->GetPref(PWSprefs::PWDefaultLength);
  m_pwpUseLowerCtrl->SetValue(prefs->GetPref(PWSprefs::PWUseLowercase));
  m_pwpUseUpperCtrl->SetValue(prefs->GetPref(PWSprefs::PWUseUppercase));
  m_pwpUseDigitsCtrl->SetValue(prefs->GetPref(PWSprefs::PWUseDigits));
  m_pwpSymCtrl->SetValue(prefs->GetPref(PWSprefs::PWUseSymbols));
  m_pwpHexCtrl->SetValue(prefs->GetPref(PWSprefs::PWUseHexDigits));
  m_pwpEasyCtrl->SetValue(prefs->GetPref(PWSprefs::PWUseEasyVision));
  m_pwpPronounceCtrl->SetValue(prefs->GetPref(PWSprefs::PWMakePronounceable));
  m_pwpLCSpin->SetValue(prefs->GetPref(PWSprefs::PWLowercaseMinLength));
  m_pwpUCSpin->SetValue(prefs->GetPref(PWSprefs::PWUppercaseMinLength));
  m_pwpDigSpin->SetValue(prefs->GetPref(PWSprefs::PWDigitMinLength));
  m_pwpSymSpin->SetValue(prefs->GetPref(PWSprefs::PWSymbolMinLength));
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

  prefs->SetPref(PWSprefs::ShowNotesAsTooltipsInViews,
                 m_shownotesastipsinviews);
  prefs->SetPref(PWSprefs::ShowPWDefault, m_pwshowinedit);
  prefs->SetPref(PWSprefs::ShowNotesDefault, m_notesshowinedit);
  prefs->SetPref(PWSprefs::NotesWordWrap, m_wordwrapnotes);
  prefs->SetPref(PWSprefs::PreExpiryWarn, m_preexpirywarn);
  if (m_preexpirywarn)
    prefs->SetPref(PWSprefs::PreExpiryWarnDays,
                   m_preexpirywarndaysSB->GetValue());
  prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, m_inittreeview);

  // Misc. preferences
  prefs->SetPref(PWSprefs::DeleteQuestion, m_confirmdelete);
  prefs->SetPref(PWSprefs::MaintainDateTimeStamps, m_maintaindatetimestamps);
  prefs->SetPref(PWSprefs::EscExits, m_escexits);
  const wxString dcaStr = m_DCACB->GetValue();
  for (int i = 0; i < int(sizeof(DCAStrings)/sizeof(DCAStrings[0])); ++i)
    if (dcaStr == DCAStrings[i]) {
      m_doubleclickaction = i;
      break;
    }

  prefs->SetPref(PWSprefs::DoubleClickAction, m_doubleclickaction);
  prefs->SetPref(PWSprefs::MinimizeOnAutotype, m_minauto);
  if (m_autotypeStr.empty() || m_autotypeStr == DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString, L"");
  else prefs->SetPref(PWSprefs::DefaultAutotypeString, m_autotypeStr.c_str());
  prefs->SetPref(PWSprefs::UseDefaultUser, m_usedefuser);
  prefs->SetPref(PWSprefs::DefaultUsername,
                 m_defusernameTXT->GetValue().c_str());
  prefs->SetPref(PWSprefs::QuerySetDef, m_querysetdef);
  prefs->SetPref(PWSprefs::AltBrowser, m_otherbrowser.c_str());
  prefs->SetPref(PWSprefs::AltBrowserCmdLineParms,
                 m_otherbrowserparams.c_str());

  // Password Policy preferences:
  prefs->SetPref(PWSprefs::PWDefaultLength, m_pwdefaultlength);
  prefs->SetPref(PWSprefs::PWUseLowercase, m_pwpUseLowerCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWUseUppercase, m_pwpUseUpperCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWUseDigits, m_pwpUseDigitsCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWUseSymbols, m_pwpSymCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWUseHexDigits, m_pwpHexCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWUseEasyVision, m_pwpEasyCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWMakePronounceable, m_pwpPronounceCtrl->GetValue());
  prefs->SetPref(PWSprefs::PWLowercaseMinLength, m_pwpLCSpin->GetValue());
  prefs->SetPref(PWSprefs::PWUppercaseMinLength, m_pwpUCSpin->GetValue());
  prefs->SetPref(PWSprefs::PWDigitMinLength, m_pwpDigSpin->GetValue());
  prefs->SetPref(PWSprefs::PWSymbolMinLength, m_pwpSymSpin->GetValue());
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


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX19
 */

void COptions::OnPreExpiryWarnClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    m_preexpirywarndaysSB->Enable(m_preexpirywarn);
  }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX24
 */

void COptions::OnUseDefaultUserClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    m_defusernameTXT->Enable(m_usedefuser);
    m_defusernameLBL->Enable(m_usedefuser);
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON8
 */

void COptions::OnBrowseLocationClick( wxCommandEvent& event )
{
  wxFileDialog fd(this, _("Select a Browser"));
  if (Validate() && TransferDataFromWindow()) {
    fd.SetPath(m_otherbrowser);
  }
  if (fd.ShowModal() == wxID_OK) {
    m_otherbrowser = fd.GetPath();
    Validate() && TransferDataToWindow();
  }
}

static void EnableSizerChildren(wxSizer *sz, bool enable)
{
  wxSizerItemList &clist = sz->GetChildren();
  wxSizerItemList::iterator iter;

  for (iter = clist.begin(); iter != clist.end(); iter++) {
    wxWindow *w = (*iter)->GetWindow();
    if (w != NULL)
      w->Enable(enable);
  }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX3
 */

void COptions::OnPwPolUseClick( wxCommandEvent& event )
{
  bool useHex = m_pwpHexCtrl->GetValue();

  EnableSizerChildren(m_pwNumLCbox, m_pwpUseLowerCtrl->GetValue() && !useHex);
  EnableSizerChildren(m_pwNumUCbox, m_pwpUseUpperCtrl->GetValue() && !useHex);
  EnableSizerChildren(m_pwNumDigbox, m_pwpUseDigitsCtrl->GetValue() && !useHex);
  EnableSizerChildren(m_pwNumSymbox, m_pwpSymCtrl->GetValue() && !useHex);

  bool showAtLeasts = !(m_pwpEasyCtrl->GetValue() ||
                        m_pwpPronounceCtrl->GetValue());
  m_pwNumLCbox->Show(showAtLeasts);
  m_pwNumUCbox->Show(showAtLeasts);
  m_pwNumDigbox->Show(showAtLeasts);
  m_pwNumSymbox->Show(showAtLeasts);

  m_pwpUseLowerCtrl->Enable(!useHex);
  m_pwpUseUpperCtrl->Enable(!useHex);
  m_pwpUseDigitsCtrl->Enable(!useHex);
  m_pwpSymCtrl->Enable(!useHex);
  m_pwpEasyCtrl->Enable(!useHex);
  m_pwpPronounceCtrl->Enable(!useHex);

  if (m_pwpEasyCtrl->GetValue() && m_pwpPronounceCtrl->GetValue()) {
    // we don't support both - notify user, reset caller:
    wxMessageDialog msg(this, _("Sorry, 'pronounceable' and 'easy-to-read'"
                                " are not supported together"),
                        _("Password Safe"), wxOK | wxICON_EXCLAMATION);
    msg.ShowModal();
    if (event.GetEventObject() == m_pwpPronounceCtrl)
      m_pwpPronounceCtrl->SetValue(false);
    else
      m_pwpEasyCtrl->SetValue(false);
  }
}
