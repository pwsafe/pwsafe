/*
 * Copyright (c) 2003-2013 Rony Shapiro <ronys@users.sourceforge.net>.
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
#if defined(__X__) || defined(__WXGTK__)
#include <wx/clipbrd.h>
#endif

#include "passwordsafeframe.h"
#include "optionspropsheet.h"
#include "core/PWSprefs.h"
#include "core/Util.h" // for datetime string
#include "core/PWSAuxParse.h" // for DEFAULT_AUTOTYPE
#include "./wxutils.h"
#include "./pwsmenushortcuts.h"
#include "pwsafeapp.h" // for set/get hashIter

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

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
  EVT_CHECKBOX( ID_CHECKBOX26, COptions::OnPWHistSaveClick )
  EVT_RADIOBUTTON( ID_PWHISTNOCHANGE, COptions::OnPWHistRB )
  EVT_RADIOBUTTON( ID_PWHISTSTOP, COptions::OnPWHistRB )
  EVT_RADIOBUTTON( ID_PWHISTSTART, COptions::OnPWHistRB )
  EVT_RADIOBUTTON( ID_PWHISTSETMAX, COptions::OnPWHistRB )
  EVT_BUTTON( ID_PWHISTNOCHANGE, COptions::OnPWHistApply )
  EVT_CHECKBOX( ID_CHECKBOX29, COptions::OnLockOnIdleClick )
  EVT_CHECKBOX( ID_CHECKBOX30, COptions::OnUseSystrayClick )
////@end COptions event table entries

  EVT_BOOKCTRL_PAGE_CHANGING(wxID_ANY, COptions::OnPageChanging)
  EVT_BOOKCTRL_PAGE_CHANGING(wxID_ANY, COptions::OnPageChanging)
  EVT_SPINCTRL(ID_SPINCTRL5, COptions::OnAtLeastChars)
  EVT_SPINCTRL(ID_SPINCTRL6, COptions::OnAtLeastChars)
  EVT_SPINCTRL(ID_SPINCTRL7, COptions::OnAtLeastChars)
  EVT_SPINCTRL(ID_SPINCTRL8, COptions::OnAtLeastChars)
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
  _("Edit/View selected entry"),
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
  OnPWHistSaveClick(dummyEv);
  m_pwhistapplyBN->Enable(false);
  OnLockOnIdleClick(dummyEv);
  OnUseSystrayClick(dummyEv);
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
  m_SDCACB = NULL;
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
  m_pwhistsaveCB = NULL;
  m_pwhistnumdfltSB = NULL;
  m_pwhistapplyBN = NULL;
  m_seclockonidleCB = NULL;
  m_secidletimeoutSB = NULL;
  m_sysusesystrayCB = NULL;
  m_sysmaxREitemsSB = NULL;
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

  wxCheckBox* itemCheckBox34 = new wxCheckBox( itemPanel29, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox34->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox34, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox35 = new wxCheckBox( itemPanel29, ID_CHECKBOX16, _("Show Password in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox35->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox35, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox36 = new wxCheckBox( itemPanel29, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox36->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox36, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox37 = new wxCheckBox( itemPanel29, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox37->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox37, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox38 = new wxCheckBox( itemPanel29, ID_CHECKBOX38, _("Put Groups first in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
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

  itemBoxSizer49->Add(20, 13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

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
  m_DCACBStrings.Add(_("Edit/View selected entry"));
  m_DCACB = new wxComboBox( itemPanel44, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_DCACBStrings, wxCB_READONLY );
  itemBoxSizer49->Add(m_DCACB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer53 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer45->Add(itemBoxSizer53, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText54 = new wxStaticText( itemPanel44, wxID_STATIC, _("Shift double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer53->Add(itemStaticText54, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_SDCACBStrings;
  m_SDCACBStrings.Add(_("Autotype"));
  m_SDCACBStrings.Add(_("Browse to URL"));
  m_SDCACBStrings.Add(_("Browse to URL + Autotype"));
  m_SDCACBStrings.Add(_("Copy notes to clipboard"));
  m_SDCACBStrings.Add(_("Copy password to clipboard"));
  m_SDCACBStrings.Add(_("Copy password to clipboard, minimize"));
  m_SDCACBStrings.Add(_("Copy username to clipboard"));
  m_SDCACBStrings.Add(_("Run Command"));
  m_SDCACBStrings.Add(_("Send email"));
  m_SDCACBStrings.Add(_("Edit/View selected entry"));
  m_SDCACB = new wxComboBox( itemPanel44, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_SDCACBStrings, wxCB_READONLY );
  itemBoxSizer53->Add(m_SDCACB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer56Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Autotype"));
  wxStaticBoxSizer* itemStaticBoxSizer56 = new wxStaticBoxSizer(itemStaticBoxSizer56Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer56, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox57 = new wxCheckBox( itemPanel44, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox57->SetValue(false);
  itemStaticBoxSizer56->Add(itemCheckBox57, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer56->Add(itemBoxSizer58, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText59 = new wxStaticText( itemPanel44, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemStaticText59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl60 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer58->Add(itemTextCtrl60, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer61Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Default Username"));
  wxStaticBoxSizer* itemStaticBoxSizer61 = new wxStaticBoxSizer(itemStaticBoxSizer61Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer61, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer62 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer61->Add(itemBoxSizer62, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox63 = new wxCheckBox( itemPanel44, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox63->SetValue(false);
  itemBoxSizer62->Add(itemCheckBox63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_defusernameTXT = new wxTextCtrl( itemPanel44, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer62->Add(m_defusernameTXT, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_defusernameLBL = new wxStaticText( itemPanel44, wxID_STATIC, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer62->Add(m_defusernameLBL, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox66 = new wxCheckBox( itemPanel44, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox66->SetValue(false);
  itemStaticBoxSizer61->Add(itemCheckBox66, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer67Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Alternate Browser"));
  wxStaticBoxSizer* itemStaticBoxSizer67 = new wxStaticBoxSizer(itemStaticBoxSizer67Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer67, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer68 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer67->Add(itemBoxSizer68, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl69 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
  itemBoxSizer68->Add(itemTextCtrl69, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton70 = new wxButton( itemPanel44, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer68->Add(itemButton70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer67->Add(itemBoxSizer71, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl72 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(itemPanel44->ConvertDialogToPixels(wxSize(60, -1)).x, -1), 0 );
  itemBoxSizer71->Add(itemTextCtrl72, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText73 = new wxStaticText( itemPanel44, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer71->Add(itemStaticText73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel44, _("Misc."));

  wxPanel* itemPanel74 = new wxPanel( GetBookCtrl(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxStaticBox* itemStaticBoxSizer75Static = new wxStaticBox(itemPanel74, wxID_ANY, _("Random password generation rules"));
  wxStaticBoxSizer* itemStaticBoxSizer75 = new wxStaticBoxSizer(itemStaticBoxSizer75Static, wxVERTICAL);
  itemPanel74->SetSizer(itemStaticBoxSizer75);

  wxBoxSizer* itemBoxSizer76 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer75->Add(itemBoxSizer76, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText77 = new wxStaticText( itemPanel74, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer76->Add(itemStaticText77, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl78 = new wxSpinCtrl( itemPanel74, ID_SPINCTRL3, _T("8"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 1024, 8 );
  itemBoxSizer76->Add(itemSpinCtrl78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwMinsGSzr = new wxGridSizer(6, 2, 0, 0);
  itemStaticBoxSizer75->Add(m_pwMinsGSzr, 0, wxALIGN_LEFT|wxALL, 5);
  m_pwpUseLowerCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseLowerCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseLowerCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumLCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumLCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText82 = new wxStaticText( itemPanel74, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText82, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpLCSpin = new wxSpinCtrl( itemPanel74, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(itemPanel74->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumLCbox->Add(m_pwpLCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText84 = new wxStaticText( itemPanel74, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumLCbox->Add(itemStaticText84, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseUpperCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseUpperCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseUpperCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumUCbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumUCbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText87 = new wxStaticText( itemPanel74, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUCSpin = new wxSpinCtrl( itemPanel74, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(itemPanel74->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumUCbox->Add(m_pwpUCSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText89 = new wxStaticText( itemPanel74, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumUCbox->Add(itemStaticText89, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpUseDigitsCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpUseDigitsCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpUseDigitsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumDigbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumDigbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText92 = new wxStaticText( itemPanel74, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText92, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpDigSpin = new wxSpinCtrl( itemPanel74, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(itemPanel74->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumDigbox->Add(m_pwpDigSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText94 = new wxStaticText( itemPanel74, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumDigbox->Add(itemStaticText94, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpSymCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpSymCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwNumSymbox = new wxBoxSizer(wxHORIZONTAL);
  m_pwMinsGSzr->Add(m_pwNumSymbox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxStaticText* itemStaticText97 = new wxStaticText( itemPanel74, wxID_STATIC, _("(At least "), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText97, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpSymSpin = new wxSpinCtrl( itemPanel74, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(itemPanel74->ConvertDialogToPixels(wxSize(20, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  m_pwNumSymbox->Add(m_pwpSymSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText99 = new wxStaticText( itemPanel74, wxID_STATIC, _(")"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwNumSymbox->Add(itemStaticText99, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwpEasyCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX7, _("Use only easy-to-read characters\n(i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpEasyCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpEasyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwpPronounceCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpPronounceCtrl->SetValue(false);
  m_pwMinsGSzr->Add(m_pwpPronounceCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  m_pwMinsGSzr->Add(10, 13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText104 = new wxStaticText( itemPanel74, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer75->Add(itemStaticText104, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwpHexCtrl = new wxCheckBox( itemPanel74, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwpHexCtrl->SetValue(false);
  itemStaticBoxSizer75->Add(m_pwpHexCtrl, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel74, _("Password Policy"));

  wxPanel* itemPanel106 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer107 = new wxBoxSizer(wxVERTICAL);
  itemPanel106->SetSizer(itemBoxSizer107);

  wxBoxSizer* itemBoxSizer108 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer107->Add(itemBoxSizer108, 0, wxGROW|wxALL, 5);
  m_pwhistsaveCB = new wxCheckBox( itemPanel106, ID_CHECKBOX26, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhistsaveCB->SetValue(false);
  itemBoxSizer108->Add(m_pwhistsaveCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwhistnumdfltSB = new wxSpinCtrl( itemPanel106, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxSize(itemPanel106->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer108->Add(m_pwhistnumdfltSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText111 = new wxStaticText( itemPanel106, wxID_STATIC, _("previous passwords per entry"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer108->Add(itemStaticText111, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer112Static = new wxStaticBox(itemPanel106, wxID_ANY, _("Manage password history of current entries"));
  wxStaticBoxSizer* itemStaticBoxSizer112 = new wxStaticBoxSizer(itemStaticBoxSizer112Static, wxVERTICAL);
  itemBoxSizer107->Add(itemStaticBoxSizer112, 0, wxGROW|wxALL, 5);
  wxRadioButton* itemRadioButton113 = new wxRadioButton( itemPanel106, ID_PWHISTNOCHANGE, _("No change"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton113->SetValue(false);
  itemStaticBoxSizer112->Add(itemRadioButton113, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton114 = new wxRadioButton( itemPanel106, ID_PWHISTSTOP, _("Stop saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton114->SetValue(false);
  itemStaticBoxSizer112->Add(itemRadioButton114, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton115 = new wxRadioButton( itemPanel106, ID_PWHISTSTART, _("Start saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton115->SetValue(false);
  itemStaticBoxSizer112->Add(itemRadioButton115, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton116 = new wxRadioButton( itemPanel106, ID_PWHISTSETMAX, _("Set maximum number of paswords saved to above value"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton116->SetValue(false);
  itemStaticBoxSizer112->Add(itemRadioButton116, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwhistapplyBN = new wxButton( itemPanel106, ID_PWHISTNOCHANGE, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer112->Add(m_pwhistapplyBN, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel106, _("Password History"));

  wxPanel* itemPanel118 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer119 = new wxBoxSizer(wxVERTICAL);
  itemPanel118->SetSizer(itemBoxSizer119);

  wxCheckBox* itemCheckBox120 = new wxCheckBox( itemPanel118, ID_CHECKBOX27, _("Clear clipboard upon minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox120->SetValue(false);
  itemBoxSizer119->Add(itemCheckBox120, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox121 = new wxCheckBox( itemPanel118, ID_CHECKBOX, _("Clear clipboard upon exit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox121->SetValue(false);
  itemBoxSizer119->Add(itemCheckBox121, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox122 = new wxCheckBox( itemPanel118, ID_CHECKBOX1, _("Confirm item copy to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox122->SetValue(false);
  itemBoxSizer119->Add(itemCheckBox122, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox123 = new wxCheckBox( itemPanel118, ID_CHECKBOX2, _("Lock password database on minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox123->SetValue(false);
  itemBoxSizer119->Add(itemCheckBox123, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox124 = new wxCheckBox( itemPanel118, ID_CHECKBOX28, _("Lock password database on workstation lock"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox124->SetValue(false);
  itemBoxSizer119->Add(itemCheckBox124, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer125 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer119->Add(itemBoxSizer125, 0, wxGROW|wxALL, 0);
  m_seclockonidleCB = new wxCheckBox( itemPanel118, ID_CHECKBOX29, _("Lock password database after"), wxDefaultPosition, wxDefaultSize, 0 );
  m_seclockonidleCB->SetValue(false);
  itemBoxSizer125->Add(m_seclockonidleCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_secidletimeoutSB = new wxSpinCtrl( itemPanel118, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxSize(itemPanel118->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer125->Add(m_secidletimeoutSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText128 = new wxStaticText( itemPanel118, wxID_STATIC, _("minutes idle"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer125->Add(itemStaticText128, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer129 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer119->Add(itemBoxSizer129, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText130 = new wxStaticText( itemPanel118, wxID_STATIC, _("Unlock Difficulty:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer129->Add(itemStaticText130, 0, wxALIGN_LEFT|wxALL, 5);

  wxSlider* itemSlider131 = new wxSlider( itemPanel118, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS );
  itemBoxSizer129->Add(itemSlider131, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer132 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer129->Add(itemBoxSizer132, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText133 = new wxStaticText( itemPanel118, wxID_STATIC, _("Standard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer132->Add(itemStaticText133, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer132->Add(10, 13, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText135 = new wxStaticText( itemPanel118, wxID_STATIC, _("Maximum"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer132->Add(itemStaticText135, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel118, _("Security"));

  wxPanel* itemPanel136 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer137 = new wxBoxSizer(wxVERTICAL);
  itemPanel136->SetSizer(itemBoxSizer137);

  wxStaticBox* itemStaticBoxSizer138Static = new wxStaticBox(itemPanel136, wxID_ANY, _("System Tray"));
  wxStaticBoxSizer* itemStaticBoxSizer138 = new wxStaticBoxSizer(itemStaticBoxSizer138Static, wxVERTICAL);
  itemBoxSizer137->Add(itemStaticBoxSizer138, 0, wxGROW|wxALL, 5);
  m_sysusesystrayCB = new wxCheckBox( itemPanel136, ID_CHECKBOX30, _("Put icon in System Tray"), wxDefaultPosition, wxDefaultSize, 0 );
  m_sysusesystrayCB->SetValue(false);
  itemStaticBoxSizer138->Add(m_sysusesystrayCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer140 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer138->Add(itemBoxSizer140, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText141 = new wxStaticText( itemPanel136, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer140->Add(itemStaticText141, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_sysmaxREitemsSB = new wxSpinCtrl( itemPanel136, ID_SPINCTRL13, _T("0"), wxDefaultPosition, wxSize(itemPanel136->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer140->Add(m_sysmaxREitemsSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText143 = new wxStaticText( itemPanel136, wxID_STATIC, _("used entries in System Tray menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer140->Add(itemStaticText143, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox144 = new wxCheckBox( itemPanel136, ID_CHECKBOX31, _("Start PasswordSafe at Login"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox144->SetValue(false);
  itemStaticBoxSizer138->Add(itemCheckBox144, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer145Static = new wxStaticBox(itemPanel136, wxID_ANY, _("Recent PasswordSafe Databases"));
  wxStaticBoxSizer* itemStaticBoxSizer145 = new wxStaticBoxSizer(itemStaticBoxSizer145Static, wxVERTICAL);
  itemBoxSizer137->Add(itemStaticBoxSizer145, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer146 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer145->Add(itemBoxSizer146, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText147 = new wxStaticText( itemPanel136, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer146->Add(itemStaticText147, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl148 = new wxSpinCtrl( itemPanel136, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxSize(itemPanel136->ConvertDialogToPixels(wxSize(30, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer146->Add(itemSpinCtrl148, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText149 = new wxStaticText( itemPanel136, wxID_STATIC, _("databases"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer146->Add(itemStaticText149, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox150 = new wxCheckBox( itemPanel136, ID_CHECKBOX32, _("Recent Databases on File Menu rather than as a sub-menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox150->SetValue(false);
  itemStaticBoxSizer145->Add(itemCheckBox150, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox151 = new wxCheckBox( itemPanel136, ID_CHECKBOX33, _("Open database as read-only by default"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox151->SetValue(false);
  itemBoxSizer137->Add(itemCheckBox151, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox152 = new wxCheckBox( itemPanel136, ID_CHECKBOX34, _("Allow multiple instances"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox152->SetValue(false);
  itemBoxSizer137->Add(itemCheckBox152, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXX11__) || defined(__WXGTK__)
  wxCheckBox* itemCheckBox153 = new wxCheckBox( itemPanel136, ID_CHECKBOX39, _("Use Primary Selection for clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox153->SetValue(false);
  itemBoxSizer137->Add(itemCheckBox153, 0, wxALIGN_LEFT|wxALL, 5);
#endif

  GetBookCtrl()->AddPage(itemPanel136, _("System"));

  wxPanel* itemPanel154 = new wxPanel( GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxGrid* itemGrid155 = new wxGrid( itemPanel154, ID_GRID1, wxDefaultPosition, itemPanel154->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid155->SetDefaultColSize(100);
  itemGrid155->SetDefaultRowSize(25);
  itemGrid155->SetColLabelSize(25);
  itemGrid155->SetRowLabelSize(50);
  itemGrid155->CreateGrid(50, 2, wxGrid::wxGridSelectCells);

  GetBookCtrl()->AddPage(itemPanel154, _("Shortcuts"));

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_saveimmediate) );
  itemCheckBox6->SetValidator( wxGenericValidator(& m_backupb4save) );
  itemCheckBox31->SetValidator( wxGenericValidator(& m_alwaysontop) );
  itemCheckBox32->SetValidator( wxGenericValidator(& m_showusernameintree) );
  itemCheckBox34->SetValidator( wxGenericValidator(& m_shownotesastipsinviews) );
  itemCheckBox35->SetValidator( wxGenericValidator(& m_pwshowinedit) );
  itemCheckBox36->SetValidator( wxGenericValidator(& m_notesshowinedit) );
  itemCheckBox37->SetValidator( wxGenericValidator(& m_wordwrapnotes) );
  itemCheckBox38->SetValidator( wxGenericValidator(& m_putgroups1st) );
  m_preexpirywarnCB->SetValidator( wxGenericValidator(& m_preexpirywarn) );
  itemRadioBox43->SetValidator( wxGenericValidator(& m_inittreeview) );
  itemCheckBox46->SetValidator( wxGenericValidator(& m_confirmdelete) );
  itemCheckBox47->SetValidator( wxGenericValidator(& m_maintaindatetimestamps) );
  itemCheckBox48->SetValidator( wxGenericValidator(& m_escexits) );
  itemCheckBox57->SetValidator( wxGenericValidator(& m_minauto) );
  itemTextCtrl60->SetValidator( wxGenericValidator(& m_autotypeStr) );
  itemCheckBox63->SetValidator( wxGenericValidator(& m_usedefuser) );
  itemCheckBox66->SetValidator( wxGenericValidator(& m_querysetdef) );
  itemTextCtrl69->SetValidator( wxGenericValidator(& m_otherbrowser) );
  itemSpinCtrl78->SetValidator( wxGenericValidator(& m_pwdefaultlength) );
  itemCheckBox120->SetValidator( wxGenericValidator(& m_secclrclponmin) );
  itemCheckBox121->SetValidator( wxGenericValidator(& m_secclrclponexit) );
  itemCheckBox122->SetValidator( wxGenericValidator(& m_secconfrmcpy) );
  itemCheckBox123->SetValidator( wxGenericValidator(& m_seclockonwinlock) );
  itemCheckBox124->SetValidator( wxGenericValidator(& m_seclockonwinlock) );
  itemSlider131->SetValidator( wxGenericValidator(& m_hashIterSlider) );
  itemCheckBox144->SetValidator( wxGenericValidator(& m_sysstartup) );
  itemSpinCtrl148->SetValidator( wxGenericValidator(& m_sysmaxmru) );
  itemCheckBox150->SetValidator( wxGenericValidator(& m_sysmruonfilemenu) );
  itemCheckBox151->SetValidator( wxGenericValidator(& m_sysdefopenro) );
  itemCheckBox152->SetValidator( wxGenericValidator(& m_sysmultinst) );
#if defined(__WXX11__) || defined(__WXGTK__)
  itemCheckBox153->SetValidator( wxGenericValidator(& m_usePrimarySelection) );
#endif
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
  m_busuffixCB->Select(suffixIndex);
  m_bumaxinc->SetValue(prefs->GetPref(PWSprefs::BackupMaxIncremented));
  wxString budirValue = prefs->GetPref(PWSprefs::BackupDir).c_str();
  m_dfltbudirRB->SetValue(budirValue.empty());
  m_usrbudirRB->SetValue(!budirValue.empty());
  m_usrbudirTxt->SetValue(budirValue);

  // display-related preferences
  m_alwaysontop = prefs->GetPref(PWSprefs::AlwaysOnTop);
  m_showusernameintree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  m_showpasswordintreeCB->SetValue(m_showusernameintree && prefs->
                                   GetPref(PWSprefs::ShowPasswordInTree));
  m_showpasswordintreeCB->Enable(m_showusernameintree);
  m_shownotesastipsinviews = prefs->
    GetPref(PWSprefs::ShowNotesAsTooltipsInViews);
  m_pwshowinedit = prefs->GetPref(PWSprefs::ShowPWDefault);
  m_notesshowinedit = prefs->GetPref(PWSprefs::ShowNotesDefault);
  m_wordwrapnotes = prefs->GetPref(PWSprefs::NotesWordWrap);
  m_putgroups1st = prefs->GetPref(PWSprefs::ExplorerTypeTree);
  m_preexpirywarn = prefs->GetPref(PWSprefs::PreExpiryWarn);
  m_preexpirywarndaysSB->SetValue(prefs->GetPref(PWSprefs::PreExpiryWarnDays));
  m_preexpirywarndaysSB->Enable(m_preexpirywarn);
  m_inittreeview = prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen);

  // Misc. preferences
  m_confirmdelete = !prefs->GetPref(PWSprefs::DeleteQuestion);
  m_maintaindatetimestamps = prefs->GetPref(PWSprefs::MaintainDateTimeStamps);
  m_escexits = prefs->GetPref(PWSprefs::EscExits);
  m_doubleclickaction = prefs->GetPref(PWSprefs::DoubleClickAction);
  if (m_doubleclickaction < 0 ||
      m_doubleclickaction >= int(sizeof(DCAStrings)/sizeof(DCAStrings[0])))
    m_doubleclickaction = 0;
  m_DCACB->SetValue(DCAStrings[m_doubleclickaction]);
  m_shiftdoubleclickaction = prefs->GetPref(PWSprefs::ShiftDoubleClickAction);
  if (m_shiftdoubleclickaction < 0 ||
      m_shiftdoubleclickaction >= int(sizeof(DCAStrings)/sizeof(DCAStrings[0])))
    m_shiftdoubleclickaction = 0;
  m_SDCACB->SetValue(DCAStrings[m_shiftdoubleclickaction]);
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

  // Password History preferences
  m_pwhistsaveCB->SetValue(prefs->GetPref(PWSprefs::SavePasswordHistory));
  m_pwhistnumdfltSB->SetValue(prefs->GetPref(PWSprefs::NumPWHistoryDefault));

  // Security preferences
  m_secclrclponmin = prefs->GetPref(PWSprefs::ClearClipboardOnMinimize);
  m_secclrclponexit = prefs->GetPref(PWSprefs::ClearClipboardOnExit);
  m_seclockonmin = prefs->GetPref(PWSprefs::DatabaseClear);
  m_secconfrmcpy = prefs->GetPref(PWSprefs::DontAskQuestion);
  m_seclockonwinlock = prefs->GetPref(PWSprefs::LockOnWindowLock);
  m_seclockonidleCB->SetValue(prefs->GetPref(PWSprefs::LockDBOnIdleTimeout));
  m_secidletimeoutSB->SetValue(prefs->GetPref(PWSprefs::IdleTimeout));
  PwsafeApp *app = dynamic_cast<PwsafeApp *>(wxTheApp);
  uint32 hashIters = app->GetHashIters();
  if (hashIters <= MIN_HASH_ITERATIONS) {
    m_hashIterSlider = 0;
  } else {
    const int step = MAX_USABLE_HASH_ITERS/31;
    m_hashIterSlider = uint32(hashIters/step);
  }

  // System preferences
  m_sysmaxREitemsSB->SetValue(prefs->GetPref(PWSprefs::MaxREItems));
  m_sysusesystrayCB->SetValue(prefs->GetPref(PWSprefs::UseSystemTray));
  m_sysstartup = false; // XXX TBD
  m_sysmaxmru = prefs->GetPref(PWSprefs::MaxMRUItems);
  m_sysmruonfilemenu = prefs->GetPref(PWSprefs::MRUOnFileMenu);
  m_sysdefopenro = prefs->GetPref(PWSprefs::DefaultOpenRO);
  m_sysmultinst = prefs->GetPref(PWSprefs::MultipleInstances);
#if defined(__X__) || defined(__WXGTK__)
  m_usePrimarySelection = prefs->GetPref(PWSprefs::UsePrimarySelectionForClipboard);
#endif
}

static int DCAStr2Int(const wxString &str)
{
  for (int i = 0; i < int(sizeof(DCAStrings)/sizeof(DCAStrings[0])); ++i)
    if (str == DCAStrings[i]) {
      return i;
    }
  ASSERT(0);
  return -1;
}

void COptions::PropSheetToPrefs()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  // Backup-related preferences
  prefs->SetPref(PWSprefs::BackupBeforeEverySave, m_backupb4save);
  wxString buprefixValue;
  if (m_usrbuprefixRB->GetValue())
    buprefixValue = m_usrbuprefixTxt->GetValue();
  prefs->SetPref(PWSprefs::BackupPrefixValue, tostringx(buprefixValue));
  int suffixIndex = m_busuffixCB->GetCurrentSelection();
  prefs->SetPref(PWSprefs::BackupSuffix, suffixIndex);
  if (suffixIndex == INC_SFX)
    prefs->SetPref(PWSprefs::BackupMaxIncremented, m_bumaxinc->GetValue());
  wxString budirValue;
  if (m_usrbudirRB->GetValue())
    budirValue = m_usrbudirTxt->GetValue();
  prefs->SetPref(PWSprefs::BackupDir, tostringx(budirValue));

  // display-related preferences
  prefs->SetPref(PWSprefs::AlwaysOnTop, m_alwaysontop);
  // set/clear wxSTAY_ON_TOP flag accrdingly:
  long flags = GetParent()->GetWindowStyleFlag();
  if (m_alwaysontop)
    flags |= wxSTAY_ON_TOP;
  else
    flags &= ~wxSTAY_ON_TOP;
  GetParent()->SetWindowStyleFlag(flags);

  prefs->SetPref(PWSprefs::ShowNotesAsTooltipsInViews,
                 m_shownotesastipsinviews);
  prefs->SetPref(PWSprefs::NotesWordWrap, m_wordwrapnotes);
  prefs->SetPref(PWSprefs::ExplorerTypeTree, m_putgroups1st);
  prefs->SetPref(PWSprefs::PreExpiryWarn, m_preexpirywarn);
  if (m_preexpirywarn)
    prefs->SetPref(PWSprefs::PreExpiryWarnDays,
                   m_preexpirywarndaysSB->GetValue());

  // Misc. preferences
  prefs->SetPref(PWSprefs::DeleteQuestion, !m_confirmdelete);
  prefs->SetPref(PWSprefs::EscExits, m_escexits);
  m_doubleclickaction = DCAStr2Int(m_DCACB->GetValue());
  prefs->SetPref(PWSprefs::DoubleClickAction, m_doubleclickaction);
  m_shiftdoubleclickaction = DCAStr2Int(m_SDCACB->GetValue());
  prefs->SetPref(PWSprefs::ShiftDoubleClickAction, m_shiftdoubleclickaction);
  prefs->SetPref(PWSprefs::MinimizeOnAutotype, m_minauto);
  prefs->SetPref(PWSprefs::QuerySetDef, m_querysetdef);
  prefs->SetPref(PWSprefs::AltBrowser, tostringx(m_otherbrowser));
  prefs->SetPref(PWSprefs::AltBrowserCmdLineParms,
                 tostringx(m_otherbrowserparams));

  // Password Policy preferences:

  // Password History preferences

  // Security preferences
  prefs->SetPref(PWSprefs::ClearClipboardOnMinimize, m_secclrclponmin);
  prefs->SetPref(PWSprefs::ClearClipboardOnExit, m_secclrclponexit);
  prefs->SetPref(PWSprefs::DatabaseClear, m_seclockonmin);
  prefs->SetPref(PWSprefs::DontAskQuestion, m_secconfrmcpy);
  prefs->SetPref(PWSprefs::LockOnWindowLock, m_seclockonwinlock);
  PwsafeApp *app = dynamic_cast<PwsafeApp *>(wxTheApp);
  uint32 value = MIN_HASH_ITERATIONS;
  if (m_hashIterSlider > 0) {
    const int step = MAX_USABLE_HASH_ITERS/31;
    value = uint32(m_hashIterSlider*step);
  }
  app->SetHashIters(value);

  // System preferences
  prefs->SetPref(PWSprefs::MaxREItems, m_sysmaxREitemsSB->GetValue());
  prefs->SetPref(PWSprefs::UseSystemTray, m_sysusesystrayCB->GetValue());
  m_sysstartup = false; // XXX TBD
  prefs->SetPref(PWSprefs::MaxMRUItems, m_sysmaxmru);
  prefs->SetPref(PWSprefs::MRUOnFileMenu, m_sysmruonfilemenu);
  prefs->SetPref(PWSprefs::DefaultOpenRO, m_sysdefopenro);
  prefs->SetPref(PWSprefs::MultipleInstances, m_sysmultinst);
#if defined(__X__) || defined(__WXGTK__)
  prefs->SetPref(PWSprefs::UsePrimarySelectionForClipboard, m_usePrimarySelection);
  wxTheClipboard->UsePrimarySelection(m_usePrimarySelection);
#endif

  // Now do a bit of trickery to find the new preferences to be stored in
  // the database as a string but without updating the actual preferences
  // which needs to be done via a Command so that it can be Undone & Redone

  // Initialise a copy of the DB preferences
  prefs->SetupCopyPrefs();

  // Update them - last parameter of SetPref and Store is: "bUseCopy = true"
  // In PropertyPage alphabetic order
  prefs->SetPref(PWSprefs::SaveImmediately, m_saveimmediate, true);
  prefs->SetPref(PWSprefs::ShowPWDefault, m_pwshowinedit, true);

  prefs->SetPref(PWSprefs::ShowUsernameInTree, m_showusernameintree, true);
  prefs->SetPref(PWSprefs::ShowPasswordInTree, m_showpasswordintreeCB->GetValue(), true);

  prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, m_inittreeview, true);
  prefs->SetPref(PWSprefs::ShowNotesDefault, m_notesshowinedit, true);
  prefs->SetPref(PWSprefs::MaintainDateTimeStamps, m_maintaindatetimestamps, true);
  prefs->SetPref(PWSprefs::UseDefaultUser, m_usedefuser, true);
  prefs->SetPref(PWSprefs::DefaultUsername, tostringx(m_defusernameTXT->GetValue()), true);

  if (m_autotypeStr.empty() || m_autotypeStr == DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString, L"", true);
  else
    prefs->SetPref(PWSprefs::DefaultAutotypeString, tostringx(m_autotypeStr), true);

  const bool usehex = m_pwpHexCtrl->GetValue();
  prefs->SetPref(PWSprefs::SavePasswordHistory, m_pwhistsaveCB->GetValue(), true);
  prefs->SetPref(PWSprefs::NumPWHistoryDefault, m_pwhistnumdfltSB->GetValue(), true);
  int pwlenRequired = GetRequiredPWLength();
  prefs->SetPref(PWSprefs::PWDefaultLength, wxMax(pwlenRequired, m_pwdefaultlength), true);
  prefs->SetPref(PWSprefs::PWUseLowercase, m_pwpUseLowerCtrl->GetValue() && !usehex, true);
  prefs->SetPref(PWSprefs::PWUseUppercase, m_pwpUseUpperCtrl->GetValue() && !usehex, true);
  prefs->SetPref(PWSprefs::PWUseDigits, m_pwpUseDigitsCtrl->GetValue() && !usehex, true);
  prefs->SetPref(PWSprefs::PWUseSymbols, m_pwpSymCtrl->GetValue() && !usehex, true);
  prefs->SetPref(PWSprefs::PWUseHexDigits, usehex, true);
  prefs->SetPref(PWSprefs::PWUseEasyVision, m_pwpEasyCtrl->GetValue() && !usehex, true);
  prefs->SetPref(PWSprefs::PWMakePronounceable, m_pwpPronounceCtrl->GetValue() && !usehex, true);
  prefs->SetPref(PWSprefs::PWLowercaseMinLength, m_pwpLCSpin->GetValue(), true);
  prefs->SetPref(PWSprefs::PWUppercaseMinLength, m_pwpUCSpin->GetValue(), true);
  prefs->SetPref(PWSprefs::PWDigitMinLength, m_pwpDigSpin->GetValue(), true);
  prefs->SetPref(PWSprefs::PWSymbolMinLength, m_pwpSymSpin->GetValue(), true);

  prefs->SetPref(PWSprefs::LockDBOnIdleTimeout, m_seclockonidleCB->GetValue(), true);
  prefs->SetPref(PWSprefs::IdleTimeout, m_secidletimeoutSB->GetValue(), true);
}

void COptions::OnOk(wxCommandEvent& /* evt */)
{
  if (Validate() && TransferDataFromWindow()) {
    PropSheetToPrefs();
    if (PWSMenuShortcuts::GetShortcutsManager()->IsDirty()) {
      PWSMenuShortcuts::GetShortcutsManager()->ApplyEditedShortcuts();
    }
    EndModal(wxID_OK);
  }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX11
 */

void COptions::OnBackupB4SaveClick( wxCommandEvent& /* evt */ )
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

void COptions::OnBuPrefix( wxCommandEvent& evt )
{
  evt.Skip();
}


/*!
 * wxEVT_SET_FOCUS event handler for ID_TEXTCTRL9
 */

void COptions::OnBuPrefixTxtSetFocus( wxFocusEvent& /* evt */ )
{
  m_dfltbuprefixRB->SetValue(false);
  m_usrbuprefixRB->SetValue(true);
}


/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX2
 */

void COptions::OnSuffixCBSet( wxCommandEvent& /* evt */ )
{
  int suffixIndex = m_busuffixCB->GetCurrentSelection();
  wxString example = m_usrbuprefixTxt->GetValue();

  if (example.empty())
    example = _("pwsafe"); // XXXX get current file's basename!

  m_bumaxinc->Enable(suffixIndex == INC_SFX);
  switch (suffixIndex) {
  case NO_SFX:
    m_suffixExample->SetLabel(wxEmptyString);
    break;
  case TS_SFX: {
    time_t now;
    time(&now);
    wxString datetime = PWSUtil::ConvertToDateTimeString(now,
                                                         PWSUtil::TMC_EXPORT_IMPORT).c_str();
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

void COptions::OnBuDirBrowseClick( wxCommandEvent& /* evt */ )
{
  wxDirDialog dirdlg(this);
  int status = dirdlg.ShowModal();
  if (status == wxID_OK)
    m_usrbudirTxt->SetValue(dirdlg.GetPath());
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON6
 */

void COptions::OnBuDirRB( wxCommandEvent& /* evt */ )
{
    bool enable = m_usrbudirRB->GetValue();
    m_usrbudirTxt->Enable(enable);
    m_buDirBN->Enable(enable);
}



/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX13
 */

void COptions::OnShowUsernameInTreeCB( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    if (!m_showusernameintree)
      m_showpasswordintreeCB->SetValue(false);
    m_showpasswordintreeCB->Enable(m_showusernameintree);
  }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX19
 */

void COptions::OnPreExpiryWarnClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    m_preexpirywarndaysSB->Enable(m_preexpirywarn);
  }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX24
 */

void COptions::OnUseDefaultUserClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    m_defusernameTXT->Enable(m_usedefuser);
    m_defusernameLBL->Enable(m_usedefuser);
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON8
 */

void COptions::OnBrowseLocationClick( wxCommandEvent& /* evt */ )
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

void COptions::OnPwPolUseClick( wxCommandEvent& evt )
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

  m_pwMinsGSzr->Layout();

  m_pwpUseLowerCtrl->Enable(!useHex);
  m_pwpUseUpperCtrl->Enable(!useHex);
  m_pwpUseDigitsCtrl->Enable(!useHex);
  m_pwpSymCtrl->Enable(!useHex);
  m_pwpEasyCtrl->Enable(!useHex);
  m_pwpPronounceCtrl->Enable(!useHex);

  if (m_pwpEasyCtrl->GetValue() && m_pwpPronounceCtrl->GetValue()) {
    // we don't support both - notify user, reset caller:
    wxMessageDialog msg(this, _("Sorry, 'pronounceable' and 'easy-to-read' are not supported together"),
                        _("Password Safe"), wxOK | wxICON_EXCLAMATION);
    msg.ShowModal();
    if (evt.GetEventObject() == m_pwpPronounceCtrl)
      m_pwpPronounceCtrl->SetValue(false);
    else
      m_pwpEasyCtrl->SetValue(false);
  }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX26
 */

void COptions::OnPWHistSaveClick( wxCommandEvent& /* evt */ )
{
  m_pwhistnumdfltSB->Enable(m_pwhistsaveCB->GetValue());
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PWHISTNOCHANGE
 */

void COptions::OnPWHistApply( wxCommandEvent& evt )
{
  // XXX TBD - send this to someone who knows how to deal with it!

  evt.Skip();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON8
 */

void COptions::OnPWHistRB( wxCommandEvent& evt )
{
  int id = evt.GetId();
  m_pwhistapplyBN->Enable(id != ID_PWHISTNOCHANGE);
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX29
 */

void COptions::OnLockOnIdleClick( wxCommandEvent& /* evt */)
{
  m_secidletimeoutSB->Enable(m_seclockonidleCB->GetValue());
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX30
 */

void COptions::OnUseSystrayClick( wxCommandEvent& /* evt */)
{
  m_sysmaxREitemsSB->Enable(m_sysusesystrayCB->GetValue());
}

void COptions::OnPageChanging(wxBookCtrlEvent& evt)
{
  const int from = evt.GetOldSelection();
  if (from != -1) {
    wxWindow* page = GetBookCtrl()->GetPage(from);
    //note that wxWindow::Validate() validates child windows
    //we need to validate the page itself, so we call its Validator directly
    wxValidator* validator = page->GetValidator();
    if (validator && !validator->Validate(this))
      evt.Veto();
  }
}

/*
 * Just trying to give the user some visual indication that
 * the password length has to be bigger than the sum of all
 * "at least" lengths.  This is not comprehensive & foolproof
 * since there are far too many ways to make the password length
 * smaller than the sum of "at least" lengths, to even think of.
 *
 * In OnOk(), we just ensure the password length is greater than
 * the sum of all enabled "at least" lengths.  We have to do this in the
 * UI, or else password generation crashes
 */
void COptions::OnAtLeastChars(wxSpinEvent& /*evt*/)
{
  const int min = GetRequiredPWLength();
  wxSpinCtrl* pwlenCtrl = wxDynamicCast(FindWindow(ID_SPINCTRL3), wxSpinCtrl);
  //pwlenCtrl->SetRange(min, pwlenCtrl->GetMax());
  if (min > pwlenCtrl->GetValue())
    pwlenCtrl->SetValue(min);
}

int COptions::GetRequiredPWLength() const {
  wxSpinCtrl* spinCtrls[] = {m_pwpUCSpin, m_pwpLCSpin, m_pwpDigSpin, m_pwpSymSpin};
  int total = 0;
  for (size_t idx = 0; idx < WXSIZEOF(spinCtrls); ++idx) {
    if (spinCtrls[idx]->IsEnabled())
      total += spinCtrls[idx]->GetValue();
  }
  return total;
}

