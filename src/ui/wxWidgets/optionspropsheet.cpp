/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file optionspropsheet.cpp
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
#include <wx/bookctrl.h>
////@end includes

#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/debug.h>
#include <wx/taskbar.h>

#include "passwordsafeframe.h"
#include "optionspropsheet.h"
#include "core/PWSprefs.h"
#include "core/Util.h" // for datetime string
#include "core/PWSAuxParse.h" // for DEFAULT_AUTOTYPE
#include "core/PWHistory.h" // for history actions
#include "./wxutils.h"
#include "./pwsmenushortcuts.h"
#include "pwsafeapp.h" // for GetHashIters()
#if defined(__X__) || defined(__WXGTK__)
#include "pwsclip.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * COptions type definition
 */

IMPLEMENT_CLASS( COptions, wxPropertySheetDialog )

/*!
 * COptions event table definition
 */

BEGIN_EVENT_TABLE( COptions, wxPropertySheetDialog )
  EVT_BUTTON(      wxID_OK,            COptions::OnOk )

////@begin COptions event table entries
  EVT_CHECKBOX(    ID_CHECKBOX11,      COptions::OnBackupB4SaveClick )
  EVT_RADIOBUTTON( ID_RADIOBUTTON4,    COptions::OnBuPrefix )
  EVT_RADIOBUTTON( ID_RADIOBUTTON5,    COptions::OnBuPrefix )
  EVT_COMBOBOX(    ID_COMBOBOX2,       COptions::OnSuffixCBSet )
  EVT_BUTTON(      ID_BUTTON,          COptions::OnBuDirBrowseClick )
  EVT_CHECKBOX(    ID_CHECKBOX13,      COptions::OnShowUsernameInTreeCB )
  EVT_CHECKBOX(    ID_CHECKBOX19,      COptions::OnPreExpiryWarnClick )
  EVT_CHECKBOX(    ID_CHECKBOX24,      COptions::OnUseDefaultUserClick )
  EVT_BUTTON(      ID_BUTTON8,         COptions::OnBrowseLocationClick )
  EVT_BUTTON(      ID_PWHISTAPPLY,     COptions::OnPWHistApply )
////@end COptions event table entries

  EVT_BOOKCTRL_PAGE_CHANGING(wxID_ANY, COptions::OnPageChanging)
  EVT_BOOKCTRL_PAGE_CHANGING(wxID_ANY, COptions::OnPageChanging)

  EVT_UPDATE_UI(   ID_CHECKBOX10,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_RADIOBUTTON7,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX13,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX14,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX16,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX17,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_RADIOBOX,        COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX21,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_TEXTCTRL11,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX24,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_TEXTCTRL12,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_1,    COptions::OnUpdateUI )

  EVT_UPDATE_UI(   ID_CHECKBOX26,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SPINCTRL11,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_8,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICBOX_1,     COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTNOCHANGE,  COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTSTOP,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTSTART,     COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTSETMAX,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTCLEAR,     COptions::OnUpdateUI )

  EVT_UPDATE_UI(   ID_CHECKBOX29,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SPINCTRL12,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_2,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_3,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SLIDER,          COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_4,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_5,    COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SPINCTRL13,      COptions::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_7,    COptions::OnUpdateUI )
END_EVENT_TABLE()

const wxString BUSuffix[] = {
  wxTRANSLATE("None"),
  wxTRANSLATE("YYYYMMMDD_HHMMSS"),
  wxTRANSLATE("Incremented Number [001-999]"),
};

enum {NO_SFX, TS_SFX, INC_SFX}; // For backup file suffix name

// Following in enum order (see PWSprefs.h)
const wxString DCAStrings[] = {
  wxTRANSLATE("Copy password to clipboard"),
  wxTRANSLATE("Edit/View selected entry"),
  wxTRANSLATE("Autotype"),
  wxTRANSLATE("Browse to URL"),
  wxTRANSLATE("Copy notes to clipboard"),
  wxTRANSLATE("Copy username to clipboard"),
  wxTRANSLATE("Copy password to clipboard, minimize"),
  wxTRANSLATE("Browse to URL + Autotype"),
  wxTRANSLATE("Run Command"),
  wxTRANSLATE("Send email"),
};

/*!
 * COptions constructors
 */

COptions::COptions(PWScore &core) : m_core(core)
{
  Init();
}

COptions::COptions( wxWindow* parent, PWScore &core, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : m_core(core)
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
  m_dfltbuprefixRB = nullptr;
  m_usrbuprefixRB = nullptr;
  m_usrbuprefixTxt = nullptr;
  m_busuffixCB = nullptr;
  m_bumaxinc = nullptr;
  m_suffixExample = nullptr;
  m_dfltbudirRB = nullptr;
  m_usrbudirRB = nullptr;
  m_usrbudirTxt = nullptr;
  m_buDirBN = nullptr;
  m_showpasswordintreeCB = nullptr;
  m_preexpirywarnCB = nullptr;
  m_preexpirywarndaysSB = nullptr;
  m_DCACB = nullptr;
  m_SDCACB = nullptr;
  m_defusernameTXT = nullptr;
  m_defusernameLBL = nullptr;
  m_pwhistsaveCB = nullptr;
  m_pwhistnumdfltSB = nullptr;
  m_pwhdefexpdaysSB = nullptr;
  m_pwhistapplyBN = nullptr;
  m_pwhistnochangeRB = nullptr;
  m_pwhiststopRB = nullptr;
  m_pwhiststartRB = nullptr;
  m_pwhistsetmaxRB = nullptr;
  m_pwhistclearRB = nullptr;
  m_applytoprotectedCB = nullptr;
  m_seclockonidleCB = nullptr;
  m_secidletimeoutSB = nullptr;
  m_sysusesystrayCB = nullptr;
  m_sysmaxREitemsSB = nullptr;
  m_systrayWarning = nullptr;
////@end COptions member initialisation
}

/*!
 * Control creation for COptions
 */

void COptions::CreateControls()
{
////@begin COptions content construction

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Backups"
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), ID_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxCheckBox* itemCheckBox4 = new wxCheckBox( itemPanel2, ID_CHECKBOX10, _("Save database immediately after any change"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox4->SetValue(false);
  itemBoxSizer3->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Intermediate Backups"));
  auto *itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
  itemBoxSizer3->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel2, ID_CHECKBOX11, _("Create intermediate backups (.ibak) before saving"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox6->SetValue(false);
  itemStaticBoxSizer5->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Backup Name"));
  auto *itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
  itemStaticBoxSizer5->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText8 = new wxStaticText( itemPanel2, wxID_STATIC, _("Base:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 5);

  m_dfltbuprefixRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON4, _("Database name"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  m_dfltbuprefixRB->SetValue(false);
  itemStaticBoxSizer7->Add(m_dfltbuprefixRB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);
  m_usrbuprefixRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON5, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_usrbuprefixRB->SetValue(false);
  itemBoxSizer10->Add(m_usrbuprefixRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_usrbuprefixTxt = new wxTextCtrl( itemPanel2, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(100, -1)).x, -1), 0 );
  itemBoxSizer10->Add(m_usrbuprefixTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine13 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer7->Add(itemStaticLine13, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemPanel2, wxID_STATIC, _("Suffix:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer15, 0, wxGROW|wxALL, 0);
  wxArrayString m_busuffixCBStrings;
  for (int i = 0; i < int(sizeof(BUSuffix)/sizeof(BUSuffix[0])); ++i) {
    m_busuffixCBStrings.Add(_(BUSuffix[i]));
  }
  m_busuffixCB = new wxComboBox( itemPanel2, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(140, -1)).x, -1), m_busuffixCBStrings, wxCB_READONLY );
  itemBoxSizer15->Add(m_busuffixCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemPanel2, wxID_STATIC, _("Max."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_bumaxinc = new wxSpinCtrl(
    itemPanel2, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::BackupMaxIncremented),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::BackupMaxIncremented),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::BackupMaxIncremented)
  );

  itemBoxSizer15->Add(m_bumaxinc, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
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

  auto *itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer5->Add(itemBoxSizer25, 0, wxGROW|wxALL, 0);
  m_usrbudirRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON7, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_usrbudirRB->SetValue(false);
  itemBoxSizer25->Add(m_usrbudirRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_usrbudirTxt = new wxTextCtrl( itemPanel2, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(m_usrbudirTxt, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_buDirBN = new wxButton( itemPanel2, ID_BUTTON, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(m_buDirBN, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel2, _("Backups"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Display"
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel29 = new wxPanel( GetBookCtrl(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer30 = new wxBoxSizer(wxVERTICAL);
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

  wxCheckBox* itemCheckBox35 = new wxCheckBox( itemPanel29, ID_CHECKBOX16, _("Show Password in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox35->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox35, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox36 = new wxCheckBox( itemPanel29, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox36->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox36, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox34 = new wxCheckBox( itemPanel29, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox34->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox34, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox37 = new wxCheckBox( itemPanel29, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox37->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox37, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox38 = new wxCheckBox( itemPanel29, ID_CHECKBOX38, _("Put Groups first in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox38->SetValue(false);
  itemBoxSizer30->Add(itemCheckBox38, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer39 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer30->Add(itemBoxSizer39, 0, wxGROW|wxALL, 0);
  m_preexpirywarnCB = new wxCheckBox( itemPanel29, ID_CHECKBOX19, _("Warn"), wxDefaultPosition, wxDefaultSize, 0 );
  m_preexpirywarnCB->SetValue(false);
  itemBoxSizer39->Add(m_preexpirywarnCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_preexpirywarndaysSB = new wxSpinCtrl(
    itemPanel29, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PreExpiryWarnDays),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PreExpiryWarnDays),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PreExpiryWarnDays)
  );

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

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Misc."
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel44 = new wxPanel( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
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

  auto *itemFlexGridSizer50 = new wxFlexGridSizer(0, 2, 0, 0);
  itemBoxSizer45->Add(itemFlexGridSizer50, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText50 = new wxStaticText( itemPanel44, wxID_STATIC, _("Double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer50->Add(itemStaticText50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Prepare strings for DblClick & Shift+DblClick combos
  wxArrayString m_DCACBStrings;
  wxArrayString m_SDCACBStrings;
  for (int i = 0; i < int(sizeof(DCAStrings)/sizeof(DCAStrings[0])); ++i) {
    wxString tmp = _(DCAStrings[i]);
    m_DCACBStrings.Add(tmp);
    m_SDCACBStrings.Add(tmp);
  }

  // This is to avoid a nasty assert on OSX with wx3.0.2
  auto cbStyle = wxCB_READONLY;
#ifndef  __WXMAC__
  cbStyle |= wxCB_SORT;
#endif

  m_DCACB = new wxComboBox( itemPanel44, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_DCACBStrings, cbStyle );
  itemFlexGridSizer50->Add(m_DCACB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText54 = new wxStaticText( itemPanel44, wxID_STATIC, _("Shift double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer50->Add(itemStaticText54, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_SDCACB = new wxComboBox( itemPanel44, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_SDCACBStrings, cbStyle );
  itemFlexGridSizer50->Add(m_SDCACB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer56Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Autotype"));
  auto *itemStaticBoxSizer56 = new wxStaticBoxSizer(itemStaticBoxSizer56Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer56, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox57 = new wxCheckBox( itemPanel44, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox57->SetValue(false);
  itemStaticBoxSizer56->Add(itemCheckBox57, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer56->Add(itemBoxSizer58, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText59 = new wxStaticText( itemPanel44, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemStaticText59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl60 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemTextCtrl60, 2, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  itemBoxSizer58->AddStretchSpacer();

  wxStaticBox* itemStaticBoxSizer61Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Default Username"));
  auto *itemStaticBoxSizer61 = new wxStaticBoxSizer(itemStaticBoxSizer61Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer61, 0, wxGROW|wxALL, 5);
  auto *itemBoxSizer62 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer61->Add(itemBoxSizer62, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox63 = new wxCheckBox( itemPanel44, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox63->SetValue(false);
  itemBoxSizer62->Add(itemCheckBox63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_defusernameTXT = new wxTextCtrl( itemPanel44, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer62->Add(m_defusernameTXT, 2, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_defusernameLBL = new wxStaticText( itemPanel44, ID_STATICTEXT_1, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer62->Add(m_defusernameLBL, 2, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  itemBoxSizer62->AddStretchSpacer();

  wxCheckBox* itemCheckBox66 = new wxCheckBox( itemPanel44, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox66->SetValue(false);
  itemStaticBoxSizer61->Add(itemCheckBox66, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer67Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Alternate Browser"));
  auto *itemStaticBoxSizer67 = new wxStaticBoxSizer(itemStaticBoxSizer67Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer67, 0, wxGROW|wxALL, 5);
  auto *itemBoxSizer68 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer67->Add(itemBoxSizer68, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl69 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer68->Add(itemTextCtrl69, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton70 = new wxButton( itemPanel44, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer68->Add(itemButton70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer67->Add(itemBoxSizer71, 0, wxGROW|wxALL, 0);

  wxStaticText* itemStaticText73 = new wxStaticText( itemPanel44, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer71->Add(itemStaticText73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl72 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer71->Add(itemTextCtrl72, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel44, _("Misc."));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password History"
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel74 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer75 = new wxBoxSizer(wxVERTICAL);
  itemPanel74->SetSizer(itemBoxSizer75);

  auto *itemBoxSizer76 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer75->Add(itemBoxSizer76, 0, wxGROW|wxALL, 5);
  m_pwhistsaveCB = new wxCheckBox( itemPanel74, ID_CHECKBOX26, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhistsaveCB->SetValue(false);
  itemBoxSizer76->Add(m_pwhistsaveCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pwhistnumdfltSB = new wxSpinCtrl(
    itemPanel74, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::NumPWHistoryDefault),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::NumPWHistoryDefault),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::NumPWHistoryDefault)
  );

  itemBoxSizer76->Add(m_pwhistnumdfltSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText79 = new wxStaticText( itemPanel74, ID_STATICTEXT_8, _("previous passwords per entry"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer76->Add(itemStaticText79, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer77 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer75->Add(itemBoxSizer77, 0, wxGROW|wxALL, 5);

  itemBoxSizer77->Add(
    new wxStaticText(itemPanel74, ID_STATICTEXT_9, _("Default password expiration (days)"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );

  m_pwhdefexpdaysSB = new wxSpinCtrl(
    itemPanel74, ID_SPINCTRL14, _T("0"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::DefaultExpiryDays)
  );

  itemBoxSizer77->Add(m_pwhdefexpdaysSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer80Static = new wxStaticBox(itemPanel74, ID_STATICBOX_1, _("Manage password history of current entries"));
  auto *itemStaticBoxSizer80 = new wxStaticBoxSizer(itemStaticBoxSizer80Static, wxVERTICAL);
  itemBoxSizer75->Add(itemStaticBoxSizer80, 0, wxGROW|wxALL, 5);
  m_pwhistnochangeRB = new wxRadioButton( itemPanel74, ID_PWHISTNOCHANGE, _("No change"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhistnochangeRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_pwhistnochangeRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwhiststopRB = new wxRadioButton( itemPanel74, ID_PWHISTSTOP, _("Stop saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhiststopRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_pwhiststopRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwhiststartRB = new wxRadioButton( itemPanel74, ID_PWHISTSTART, _("Start saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhiststartRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_pwhiststartRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwhistsetmaxRB = new wxRadioButton( itemPanel74, ID_PWHISTSETMAX, _("Set maximum number of passwords saved to above value"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhistsetmaxRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_pwhistsetmaxRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_pwhistclearRB = new wxRadioButton( itemPanel74, ID_PWHISTCLEAR, _("Clear password history for ALL entries"), wxDefaultPosition, wxDefaultSize, 0 );
  m_pwhistclearRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_pwhistclearRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_applytoprotectedCB = new wxCheckBox( itemPanel74, ID_APPLYTOPROTECTED, _("Apply these changes to Protected Entries (if required)."), wxDefaultPosition, wxDefaultSize, 0 );
  m_applytoprotectedCB->SetValue(false);
  itemStaticBoxSizer80->Add(m_applytoprotectedCB, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER|wxALL, 5);

  m_pwhistapplyBN = new wxButton( itemPanel74, ID_PWHISTAPPLY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer80->Add(m_pwhistapplyBN, 0, wxALIGN_LEFT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel74, _("Password History"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Security"
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel86 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer87 = new wxBoxSizer(wxVERTICAL);
  itemPanel86->SetSizer(itemBoxSizer87);

  wxCheckBox* itemCheckBox88 = new wxCheckBox( itemPanel86, ID_CHECKBOX27, _("Clear clipboard upon minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox88->SetValue(false);
  itemBoxSizer87->Add(itemCheckBox88, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox89 = new wxCheckBox( itemPanel86, ID_CHECKBOX, _("Clear clipboard upon exit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox89->SetValue(false);
  itemBoxSizer87->Add(itemCheckBox89, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox90 = new wxCheckBox( itemPanel86, ID_CHECKBOX1, _("Confirm item copy to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox90->SetValue(false);
  itemBoxSizer87->Add(itemCheckBox90, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox91 = new wxCheckBox( itemPanel86, ID_CHECKBOX2, _("Lock password database on minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox91->SetValue(false);
  itemBoxSizer87->Add(itemCheckBox91, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox92 = new wxCheckBox( itemPanel86, ID_CHECKBOX28, _("Lock password database on workstation lock"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox92->SetValue(false);
  itemBoxSizer87->Add(itemCheckBox92, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer93 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer87->Add(itemBoxSizer93, 0, wxGROW|wxALL, 0);
  m_seclockonidleCB = new wxCheckBox( itemPanel86, ID_CHECKBOX29, _("Lock password database after"), wxDefaultPosition, wxDefaultSize, 0 );
  m_seclockonidleCB->SetValue(false);
  itemBoxSizer93->Add(m_seclockonidleCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_secidletimeoutSB = new wxSpinCtrl(
    itemPanel86, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::IdleTimeout),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::IdleTimeout),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::IdleTimeout)
  );

  itemBoxSizer93->Add(m_secidletimeoutSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText96 = new wxStaticText( itemPanel86, ID_STATICTEXT_2, _("minutes idle"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer93->Add(itemStaticText96, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer87->Add(itemBoxSizer97, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText98 = new wxStaticText( itemPanel86, ID_STATICTEXT_3, _("Unlock Difficulty:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer97->Add(itemStaticText98, 0, wxALIGN_LEFT|wxALL, 5);

  wxSlider* itemSlider99 = new wxSlider( itemPanel86, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS );
  itemBoxSizer97->Add(itemSlider99, 0, wxGROW|wxALL, 5);

  auto *itemBoxSizer100 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer97->Add(itemBoxSizer100, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText101 = new wxStaticText( itemPanel86, ID_STATICTEXT_4, _("Standard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer100->Add(itemStaticText101, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer100->Add(10, 13, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText103 = new wxStaticText( itemPanel86, ID_STATICTEXT_5, _("Maximum"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer100->Add(itemStaticText103, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel86, _("Security"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Shortcuts"
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel123 = new wxPanel(GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *itemBoxSizer155 = new wxBoxSizer(wxVERTICAL);
  itemPanel123->SetSizer(itemBoxSizer155);

  wxGrid* itemGrid124 = new wxGrid(itemPanel123, ID_GRID1, wxDefaultPosition, itemPanel123->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
  itemGrid124->SetDefaultColSize(150);
  itemGrid124->SetDefaultRowSize(25);
  itemGrid124->SetColLabelSize(25);
  itemGrid124->SetRowLabelSize(50);
  itemGrid124->CreateGrid(50, 2, wxGrid::wxGridSelectCells);

  itemBoxSizer155->Add(itemGrid124, 1, wxGROW|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel123, _("Shortcuts"));

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "System"
  /////////////////////////////////////////////////////////////////////////////

  wxPanel* itemPanel104 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer105 = new wxBoxSizer(wxVERTICAL);
  itemPanel104->SetSizer(itemBoxSizer105);

  wxStaticBox* itemStaticBoxSizer106Static = new wxStaticBox(itemPanel104, wxID_ANY, _("System Tray"));
  auto *itemStaticBoxSizer106 = new wxStaticBoxSizer(itemStaticBoxSizer106Static, wxVERTICAL);
  itemBoxSizer105->Add(itemStaticBoxSizer106, 0, wxGROW|wxALL, 5);
  m_sysusesystrayCB = new wxCheckBox( itemPanel104, ID_CHECKBOX30, _("Put icon in System Tray"), wxDefaultPosition, wxDefaultSize, 0 );
  m_sysusesystrayCB->SetValue(false);
  itemStaticBoxSizer106->Add(m_sysusesystrayCB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer108 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer106->Add(itemBoxSizer108, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText109 = new wxStaticText( itemPanel104, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer108->Add(itemStaticText109, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_sysmaxREitemsSB = new wxSpinCtrl(
    itemPanel104, ID_SPINCTRL13, _T("0"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::MaxREItems),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::MaxREItems),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::MaxREItems)
  );

  itemBoxSizer108->Add(m_sysmaxREitemsSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText111 = new wxStaticText( itemPanel104, ID_STATICTEXT_7, _("used entries in System Tray menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer108->Add(itemStaticText111, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox112 = new wxCheckBox( itemPanel104, ID_CHECKBOX31, _("Start PasswordSafe at Login"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox112->SetValue(false);
  itemStaticBoxSizer106->Add(itemCheckBox112, 0, wxALIGN_LEFT|wxALL, 5);

  m_systrayWarning = new wxStaticText( itemPanel104, wxID_STATIC, _("There appears to be no system tray support in your current environment.\nAny related functionality may not work as expected."), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer106->Add(m_systrayWarning, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  m_systrayWarning->SetForegroundColour(*wxRED);
  m_systrayWarning->Hide();

  wxStaticBox* itemStaticBoxSizer113Static = new wxStaticBox(itemPanel104, wxID_ANY, _("Recent PasswordSafe Databases"));
  auto *itemStaticBoxSizer113 = new wxStaticBoxSizer(itemStaticBoxSizer113Static, wxVERTICAL);
  itemBoxSizer105->Add(itemStaticBoxSizer113, 0, wxGROW|wxALL, 5);
  auto *itemBoxSizer114 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer113->Add(itemBoxSizer114, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText115 = new wxStaticText( itemPanel104, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer114->Add(itemStaticText115, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl116 = new wxSpinCtrl(
    itemPanel104, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::MaxMRUItems),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::MaxMRUItems),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::MaxMRUItems)
  );

  itemBoxSizer114->Add(itemSpinCtrl116, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText117 = new wxStaticText( itemPanel104, wxID_STATIC, _("databases"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer114->Add(itemStaticText117, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox118 = new wxCheckBox( itemPanel104, ID_CHECKBOX32, _("Recent Databases on File Menu rather than as a sub-menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox118->SetValue(false);
  itemStaticBoxSizer113->Add(itemCheckBox118, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox119 = new wxCheckBox( itemPanel104, ID_CHECKBOX33, _("Open database as read-only by default"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox119->SetValue(false);
  itemBoxSizer105->Add(itemCheckBox119, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox120 = new wxCheckBox( itemPanel104, ID_CHECKBOX34, _("Allow multiple instances"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox120->SetValue(false);
  itemBoxSizer105->Add(itemCheckBox120, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXX11__) || defined(__WXGTK__)
  wxCheckBox* itemCheckBox121 = new wxCheckBox( itemPanel104, ID_CHECKBOX39, _("Use Primary Selection for clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox121->SetValue(false);
  itemBoxSizer105->Add(itemCheckBox121, 0, wxALIGN_LEFT|wxALL, 5);
#endif

#if defined(__WXX11__) || defined(__WXGTK__)
  wxCheckBox* itemCheckBox122 = new wxCheckBox( itemPanel104, ID_CHECKBOX40, _("Use alternate AutoType method"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox122->SetValue(false);
  itemCheckBox122->SetHelpText(_("When set, use XTEST for AutoType instead of XSendEvent.\nXSendEvent can handle more control keys, but may be blocked by some applications."));
  if (COptions::ShowToolTips())
    itemCheckBox122->SetToolTip(_("If AutoType doesn't work, setting this may help."));
  itemBoxSizer105->Add(itemCheckBox122, 0, wxALIGN_LEFT|wxALL, 5);
#endif

  GetBookCtrl()->AddPage(itemPanel104, _("System"));

  /////////////////////////////////////////////////////////////////////////////
  // End of Tab Creation
  /////////////////////////////////////////////////////////////////////////////

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
  itemCheckBox88->SetValidator( wxGenericValidator(& m_secclrclponmin) );
  itemCheckBox89->SetValidator( wxGenericValidator(& m_secclrclponexit) );
  itemCheckBox90->SetValidator( wxGenericValidator(& m_secconfrmcpy) );
  itemCheckBox91->SetValidator( wxGenericValidator(& m_seclockonmin) );
  itemCheckBox92->SetValidator( wxGenericValidator(& m_seclockonwinlock) );
  itemSlider99->SetValidator( wxGenericValidator(& m_hashIterSlider) );
  itemCheckBox112->SetValidator( wxGenericValidator(& m_sysstartup) );
  itemSpinCtrl116->SetValidator( wxGenericValidator(& m_sysmaxmru) );
  itemCheckBox118->SetValidator( wxGenericValidator(& m_sysmruonfilemenu) );
  itemCheckBox119->SetValidator( wxGenericValidator(& m_sysdefopenro) );
  itemCheckBox120->SetValidator( wxGenericValidator(& m_sysmultinst) );
  m_pwhistsaveCB->SetValidator( wxGenericValidator(& m_pwhistsave) );
  m_pwhistnumdfltSB->SetValidator( wxGenericValidator(& m_pwhistnumdflt) );
  m_pwhdefexpdaysSB->SetValidator( wxGenericValidator(& m_pwhdefexpdays) );
#if defined(__WXX11__) || defined(__WXGTK__)
  itemCheckBox121->SetValidator( wxGenericValidator(& m_usePrimarySelection) );
#endif
#if defined(__WXX11__) || defined(__WXGTK__)
  itemCheckBox122->SetValidator( wxGenericValidator(& m_useAltAutoType) );
#endif
  // Connect events and objects
  m_usrbuprefixTxt->Connect(ID_TEXTCTRL9, wxEVT_SET_FOCUS, wxFocusEventHandler(COptions::OnBuPrefixTxtSetFocus), nullptr, this);
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

wxBitmap COptions::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin COptions bitmap retrieval
  return wxNullBitmap;
////@end COptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon COptions::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin COptions icon retrieval
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
  m_DCACB->SetValue(_(DCAStrings[m_doubleclickaction]));
  m_shiftdoubleclickaction = prefs->GetPref(PWSprefs::ShiftDoubleClickAction);
  if (m_shiftdoubleclickaction < 0 ||
      m_shiftdoubleclickaction >= int(sizeof(DCAStrings)/sizeof(DCAStrings[0])))
    m_shiftdoubleclickaction = 0;
  m_SDCACB->SetValue(_(DCAStrings[m_shiftdoubleclickaction]));
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

  // Password History preferences
  m_pwhistsave = prefs->GetPref(PWSprefs::SavePasswordHistory);
  m_pwhistnumdflt = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
  m_pwhistnumdfltSB->Enable(m_pwhistsave);
  m_pwhdefexpdays = prefs->GetPref(PWSprefs::DefaultExpiryDays);

  // Security preferences
  m_secclrclponmin = prefs->GetPref(PWSprefs::ClearClipboardOnMinimize);
  m_secclrclponexit = prefs->GetPref(PWSprefs::ClearClipboardOnExit);
  m_seclockonmin = prefs->GetPref(PWSprefs::DatabaseClear);
  m_secconfrmcpy = prefs->GetPref(PWSprefs::DontAskQuestion);
  m_seclockonwinlock = prefs->GetPref(PWSprefs::LockOnWindowLock);
  m_seclockonidleCB->SetValue(prefs->GetPref(PWSprefs::LockDBOnIdleTimeout));
  m_secidletimeoutSB->SetValue(prefs->GetPref(PWSprefs::IdleTimeout));
  auto *app = dynamic_cast<PwsafeApp *>(wxTheApp);
  uint32 hashIters = app->GetHashIters();
  if (hashIters <= MIN_HASH_ITERATIONS) {
    m_hashIterSlider = 0;
  } else {
    const int step = MAX_USABLE_HASH_ITERS/100;
    m_hashIterSlider = uint32(hashIters/step);
  }

  // System preferences
  m_sysmaxREitemsSB->SetValue(prefs->GetPref(PWSprefs::MaxREItems));
  m_sysusesystrayCB->SetValue(prefs->GetPref(PWSprefs::UseSystemTray));
  if (!IsTaskBarIconAvailable()) {
    m_systrayWarning->Show();
    Layout();
  }
  m_sysstartup = false; // XXX TBD
  m_sysmaxmru = prefs->GetPref(PWSprefs::MaxMRUItems);
  m_sysmruonfilemenu = prefs->GetPref(PWSprefs::MRUOnFileMenu);
  m_sysdefopenro = prefs->GetPref(PWSprefs::DefaultOpenRO);
  m_sysmultinst = prefs->GetPref(PWSprefs::MultipleInstances);
#if defined(__X__) || defined(__WXGTK__)
  m_usePrimarySelection = prefs->GetPref(PWSprefs::UsePrimarySelectionForClipboard);
  m_useAltAutoType = prefs->GetPref(PWSprefs::UseAltAutoType);
#endif
}

static int DCAStr2Int(const wxString &str)
{
  for (int i = 0; i < int(sizeof(DCAStrings)/sizeof(DCAStrings[0])); ++i)
    if (str == _(DCAStrings[i])) {
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
  // set/clear wxSTAY_ON_TOP flag accordingly:
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

  // Password History preferences
  prefs->SetPref(PWSprefs::SavePasswordHistory, m_pwhistsave);
  prefs->SetPref(PWSprefs::NumPWHistoryDefault, m_pwhistnumdflt);
  prefs->SetPref(PWSprefs::DefaultExpiryDays, m_pwhdefexpdays);

  // Security preferences
  prefs->SetPref(PWSprefs::ClearClipboardOnMinimize, m_secclrclponmin);
  prefs->SetPref(PWSprefs::ClearClipboardOnExit, m_secclrclponexit);
  prefs->SetPref(PWSprefs::DatabaseClear, m_seclockonmin);
  prefs->SetPref(PWSprefs::DontAskQuestion, m_secconfrmcpy);
  prefs->SetPref(PWSprefs::LockOnWindowLock, m_seclockonwinlock);

  m_hashIterValue = MIN_HASH_ITERATIONS;
  if (m_hashIterSlider > 0) {
    const int step = MAX_USABLE_HASH_ITERS/100;
    m_hashIterValue = uint32(m_hashIterSlider*step);
  }

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
  PWSclipboard::GetInstance()->UsePrimarySelection(m_usePrimarySelection);
  prefs->SetPref(PWSprefs::UseAltAutoType, m_useAltAutoType);
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
      prefs->SetPref(PWSprefs::DefaultAutotypeString, wxEmptyString, true);
  else
    prefs->SetPref(PWSprefs::DefaultAutotypeString, tostringx(m_autotypeStr), true);

  prefs->SetPref(PWSprefs::LockDBOnIdleTimeout, m_seclockonidleCB->GetValue(), true);
  prefs->SetPref(PWSprefs::IdleTimeout, m_secidletimeoutSB->GetValue(), true);
  wxGetApp().ConfigureIdleTimer();
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
    example = wxT("pwsafe"); // XXXX get current file's basename!

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
      example += wxT("_");
      example = example + datetime.Left(4) +  // YYYY
        datetime.Mid(5,2) +  // MM
        datetime.Mid(8,2) +  // DD
        wxT("_") +
        datetime.Mid(11,2) +  // HH
        datetime.Mid(14,2) +  // MM
        datetime.Mid(17,2);   // SS
  }
    break;
  case INC_SFX:
    example += wxT("_001");
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

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PWHISTAPPLY
 */

void COptions::OnPWHistApply( wxCommandEvent& evt )
{
  int applytoprotected = m_applytoprotectedCB->GetValue();
  int pwhistaction = 0;
  int pwhistnum = m_pwhistnumdfltSB->GetValue();
  wxString resultmsg;

  if (m_pwhiststopRB->GetValue()) {
    // Reset entries to HISTORY OFF
    pwhistaction = (applytoprotected) ? PWHist::STOP_INCL_PROT : PWHist::STOP_EXCL_PROT;
    resultmsg = _("Number of entries that had their settings changed to not save password history was: %d");

  } else if (m_pwhiststartRB->GetValue()) {
    // Reset entries to HISTORY ON
    pwhistaction = (applytoprotected) ? PWHist::START_INCL_PROT : PWHist::START_EXCL_PROT;
    resultmsg = _("Number of entries that had their settings changed to save password history was: %d");

  } else if (m_pwhistsetmaxRB->GetValue()) {
    // Don't reset history setting, but set history number
    pwhistaction = (applytoprotected) ? PWHist::SETMAX_INCL_PROT : PWHist::SETMAX_EXCL_PROT;
    resultmsg = _("Number of entries that had their 'maximum saved passwords' changed to the new default was %d");

  } else if (m_pwhistclearRB->GetValue()) {
    // Don't reset history setting, but clear all history items
    pwhistaction = (applytoprotected) ? PWHist::CLEAR_INCL_PROT : PWHist::CLEAR_EXCL_PROT;
    resultmsg = _("Number of entries that had their password history removed was %d");

  } else {
    assert(0);
  }


  if (pwhistaction != 0) {
    Command *pcmd = UpdatePasswordHistoryCommand::Create(&m_core,
                                                                pwhistaction,
                                                                pwhistnum);
    int num_altered = pcmd->Execute();

    wxMessageBox( wxString::Format(resultmsg, num_altered), _("Password Safe"), wxOK, this);

  }
}

/*!
 * wxEVT_UPDATE_UI event handler for all command ids
 */

void COptions::OnUpdateUI(wxUpdateUIEvent& evt)
{
  bool dbIsReadOnly = m_core.IsReadOnly();

  switch (evt.GetId()) {
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Backups"
  /////////////////////////////////////////////////////////////////////////////
    case ID_CHECKBOX10:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_RADIOBUTTON7:
      m_usrbudirTxt->Enable(m_usrbudirRB->GetValue());
      m_buDirBN->Enable(m_usrbudirRB->GetValue());
      break;
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Display"
  /////////////////////////////////////////////////////////////////////////////
    case ID_CHECKBOX13:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_CHECKBOX14:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_CHECKBOX16:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_CHECKBOX17:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_RADIOBOX:
      evt.Enable(!dbIsReadOnly);
      break;
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Misc."
  /////////////////////////////////////////////////////////////////////////////
    case ID_CHECKBOX21:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_TEXTCTRL11:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_CHECKBOX24:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_TEXTCTRL12:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_STATICTEXT_1:
      evt.Enable(!dbIsReadOnly);
      break;
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password History"
  /////////////////////////////////////////////////////////////////////////////
    case ID_CHECKBOX26:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_SPINCTRL11:
      evt.Enable(!dbIsReadOnly && m_pwhistsaveCB->GetValue());
      break;
    case ID_STATICTEXT_8:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_STATICBOX_1:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_PWHISTNOCHANGE:
      m_applytoprotectedCB->Enable(!dbIsReadOnly && !m_pwhistnochangeRB->GetValue());
      m_pwhistapplyBN->Enable(!dbIsReadOnly && !m_pwhistnochangeRB->GetValue());
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_PWHISTSTOP:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_PWHISTSTART:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_PWHISTSETMAX:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_PWHISTCLEAR:
      evt.Enable(!dbIsReadOnly);
      break;
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Security"
  /////////////////////////////////////////////////////////////////////////////
    case ID_CHECKBOX29:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_SPINCTRL12:
      evt.Enable(!dbIsReadOnly && m_seclockonidleCB->GetValue());
      break;
    case ID_STATICTEXT_2:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_STATICTEXT_3:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_SLIDER:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_STATICTEXT_4:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_STATICTEXT_5:
      evt.Enable(!dbIsReadOnly);
      break;
  /////////////////////////////////////////////////////////////////////////////
  // Tab: "System"
  /////////////////////////////////////////////////////////////////////////////
    case ID_SPINCTRL13:
      evt.Enable(m_sysusesystrayCB->GetValue());
      break;
    case ID_STATICTEXT_7:
      evt.Enable(m_sysusesystrayCB->GetValue());
      break;
    default:
      break;
  }
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
