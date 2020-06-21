/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file OptionsPropertySheetDlg.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin includes
#include <wx/bookctrl.h>
////@end includes

#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/debug.h>
#include <wx/taskbar.h>

#include "core/PWSprefs.h"
#include "core/Util.h" // for datetime string
#include "core/PWSAuxParse.h" // for DEFAULT_AUTOTYPE
#include "core/PWHistory.h" // for history actions

#if defined(__X__) || defined(__WXGTK__)
#include "Clipboard.h"
#endif
#include "GridShortcutsValidator.h"
#include "OptionsPropertySheetDlg.h"
#include "PasswordSafeFrame.h"
#include "PWSafeApp.h" // for GetHashIters()
#include "wxUtilities.h"

////@begin XPM images
#include "graphics/OptionsPropertySheetDlg/backups.xpm"
#include "graphics/OptionsPropertySheetDlg/display.xpm"
#include "graphics/OptionsPropertySheetDlg/miscellaneous.xpm"
#include "graphics/OptionsPropertySheetDlg/history.xpm"
#include "graphics/OptionsPropertySheetDlg/shortcuts.xpm"
#include "graphics/OptionsPropertySheetDlg/security.xpm"
#include "graphics/OptionsPropertySheetDlg/system.xpm"
////@end XPM images

/*!
 * OptionsPropertySheetDlg type definition
 */

IMPLEMENT_CLASS( OptionsPropertySheetDlg, wxPropertySheetDialog )

/*!
 * OptionsPropertySheetDlg event table definition
 */

BEGIN_EVENT_TABLE( OptionsPropertySheetDlg, wxPropertySheetDialog )
  EVT_BUTTON(      wxID_OK,            OptionsPropertySheetDlg::OnOk )

////@begin OptionsPropertySheetDlg event table entries
  EVT_CHECKBOX(    ID_CHECKBOX11,      OptionsPropertySheetDlg::OnBackupB4SaveClick )
  EVT_RADIOBUTTON( ID_RADIOBUTTON4,    OptionsPropertySheetDlg::OnBuPrefix )
  EVT_RADIOBUTTON( ID_RADIOBUTTON5,    OptionsPropertySheetDlg::OnBuPrefix )
  EVT_COMBOBOX(    ID_COMBOBOX2,       OptionsPropertySheetDlg::OnSuffixCBSet )
  EVT_BUTTON(      ID_BUTTON,          OptionsPropertySheetDlg::OnBuDirBrowseClick )
  EVT_CHECKBOX(    ID_CHECKBOX13,      OptionsPropertySheetDlg::OnShowUsernameInTreeCB )
  EVT_CHECKBOX(    ID_CHECKBOX19,      OptionsPropertySheetDlg::OnPreExpiryWarnClick )
  EVT_CHECKBOX(    ID_CHECKBOX24,      OptionsPropertySheetDlg::OnUseDefaultUserClick )
  EVT_BUTTON(      ID_BUTTON8,         OptionsPropertySheetDlg::OnBrowseLocationClick )
  EVT_BUTTON(      ID_PWHISTAPPLY,     OptionsPropertySheetDlg::OnPWHistApply )
////@end OptionsPropertySheetDlg event table entries

  EVT_BOOKCTRL_PAGE_CHANGING(wxID_ANY, OptionsPropertySheetDlg::OnPageChanging)
  EVT_BOOKCTRL_PAGE_CHANGING(wxID_ANY, OptionsPropertySheetDlg::OnPageChanging)

  EVT_UPDATE_UI(   ID_CHECKBOX10,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_RADIOBUTTON7,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX13,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX14,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX16,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX17,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_RADIOBOX,        OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX21,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_TEXTCTRL11,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX24,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_TEXTCTRL12,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_1,    OptionsPropertySheetDlg::OnUpdateUI )

  EVT_UPDATE_UI(   ID_CHECKBOX26,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SPINCTRL11,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_8,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICBOX_1,     OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTNOCHANGE,  OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTSTOP,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTSTART,     OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTSETMAX,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_PWHISTCLEAR,     OptionsPropertySheetDlg::OnUpdateUI )

  EVT_UPDATE_UI(   ID_CHECKBOX35,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_CHECKBOX29,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SPINCTRL12,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_2,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_3,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SLIDER,          OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_4,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_5,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_SPINCTRL13,      OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_7,    OptionsPropertySheetDlg::OnUpdateUI )
  EVT_UPDATE_UI(   ID_STATICTEXT_10,   OptionsPropertySheetDlg::OnUpdateUI )
END_EVENT_TABLE()

const wxString BACKUP_SUFFIX[] = {
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
 * OptionsPropertySheetDlg constructors
 */

OptionsPropertySheetDlg::OptionsPropertySheetDlg(PWScore &core) : m_core(core)
{
  Init();
}

OptionsPropertySheetDlg::OptionsPropertySheetDlg( wxWindow* parent, PWScore &core, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : m_core(core)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * OptionsPropertySheetDlg creator
 */

bool OptionsPropertySheetDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin OptionsPropertySheetDlg creation
  SetSheetStyle(wxPROPSHEET_LISTBOOK);
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  PrefsToPropSheet();
  LayoutDialog();
  Centre();
////@end OptionsPropertySheetDlg creation
  wxCommandEvent dummyEv;
  OnSuffixCBSet(dummyEv);
  return true;
}

/*!
 * OptionsPropertySheetDlg destructor
 */

OptionsPropertySheetDlg::~OptionsPropertySheetDlg()
{
////@begin OptionsPropertySheetDlg destruction
////@end OptionsPropertySheetDlg destruction
  delete m_ImageList;
}

/*!
 * Member initialisation
 */

void OptionsPropertySheetDlg::Init()
{
////@begin OptionsPropertySheetDlg member initialisation
  m_Backup_Panel = nullptr;
  m_Backup_DefaultPrefixRB = nullptr;
  m_Backup_UserPrefixRB = nullptr;
  m_Backup_UserPrefixTXT = nullptr;
  m_Backup_SuffixCB = nullptr;
  m_Backup_MaxIncrSB = nullptr;
  m_Backup_SuffixExampleST = nullptr;
  m_Backup_DefaultDirRB = nullptr;
  m_Backup_UserDirRB = nullptr;
  m_Backup_UserDirTXT = nullptr;
  m_Backup_DirBN = nullptr;

  m_Display_Panel = nullptr;
  m_Display_ShowPasswordInTreeCB = nullptr;
  m_Display_PreExpiryWarnCB = nullptr;
  m_Display_PreExpiryWarnDaysSB = nullptr;

  m_Misc_Panel = nullptr;
  m_Misc_DoubleClickActionCB = nullptr;
  m_Misc_ShiftDoubleClickActionCB = nullptr;
  m_Misc_DefaultUsernameTXT = nullptr;
  m_Misc_DefaultUsernameLBL = nullptr;

  m_PasswordHistory_Panel = nullptr;
  m_PasswordHistory_SaveCB = nullptr;
  m_PasswordHistory_NumDefaultSB = nullptr;
  m_PasswordHistory_DefaultExpiryDaysSB = nullptr;
  m_PasswordHistory_ApplyBN = nullptr;
  m_PasswordHistory_NoChangeRB = nullptr;
  m_PasswordHistory_StopRB = nullptr;
  m_PasswordHistory_StartRB = nullptr;
  m_PasswordHistory_SetMaxRB = nullptr;
  m_PasswordHistory_ClearRB = nullptr;
  m_PasswordHistory_Apply2ProtectedCB = nullptr;

  m_Security_Panel = nullptr;
  m_Security_LockOnIdleTimeoutCB = nullptr;
  m_Security_IdleTimeoutSB = nullptr;

  m_Shortcuts_Panel = nullptr;

  m_System_Panel = nullptr;
  m_System_UseSystemTrayCB = nullptr;
  m_System_MaxREItemsSB = nullptr;
  m_System_SystemTrayWarningST = nullptr;
////@end OptionsPropertySheetDlg member initialisation

  const wxSize imageSize(64, 64);

  m_ImageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
  m_ImageList->Add(wxIcon(backups_xpm));        // https://www.pngrepo.com/svg/224774/database
  m_ImageList->Add(wxIcon(display_xpm));        // https://www.pngrepo.com/svg/131986/computer
  m_ImageList->Add(wxIcon(miscellaneous_xpm));  // https://www.pngrepo.com/svg/230756/test
  m_ImageList->Add(wxIcon(history_xpm));        // https://www.pngrepo.com/svg/95031/file
  m_ImageList->Add(wxIcon(security_xpm));       // https://www.pngrepo.com/svg/219291/security-shield
  m_ImageList->Add(wxIcon(shortcuts_xpm));      // https://www.pngrepo.com/svg/153991/keyboard
  m_ImageList->Add(wxIcon(system_xpm));         // https://www.pngrepo.com/svg/230728/settings-gear
}

/*!
 * Control creation for OptionsPropertySheetDlg
 */

void OptionsPropertySheetDlg::CreateControls()
{
  GetBookCtrl()->SetImageList(m_ImageList);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Backups"
  /////////////////////////////////////////////////////////////////////////////

  m_Backup_Panel = CreateBackupsPanel();

  GetBookCtrl()->AddPage(m_Backup_Panel, _("Backups"), false, 0);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Display"
  /////////////////////////////////////////////////////////////////////////////

  m_Display_Panel = CreateDisplayPanel();

  GetBookCtrl()->AddPage(m_Display_Panel, _("Display"), false, 1);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Miscellaneous"
  /////////////////////////////////////////////////////////////////////////////

  m_Misc_Panel = CreateMiscellaneousPanel();

  GetBookCtrl()->AddPage(m_Misc_Panel, _("Miscellaneous"), false, 2);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Password History"
  /////////////////////////////////////////////////////////////////////////////

  m_PasswordHistory_Panel = CreatePasswordHistoryPanel();

  GetBookCtrl()->AddPage(m_PasswordHistory_Panel, _("Password History"), false, 3);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Security"
  /////////////////////////////////////////////////////////////////////////////

  m_Security_Panel = CreateSecurityPanel();

  GetBookCtrl()->AddPage(m_Security_Panel, _("Security"), false, 4);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "Shortcuts"
  /////////////////////////////////////////////////////////////////////////////

  m_Shortcuts_Panel = CreateShortcutsPanel();

  GetBookCtrl()->AddPage(m_Shortcuts_Panel, _("Shortcuts"), false, 5);

  /////////////////////////////////////////////////////////////////////////////
  // Tab: "System"
  /////////////////////////////////////////////////////////////////////////////

  m_Backup_Panel = CreateSystemPanel();

  GetBookCtrl()->AddPage(m_Backup_Panel, _("System"), false, 6);

  /////////////////////////////////////////////////////////////////////////////
  // End of Tab Creation
  /////////////////////////////////////////////////////////////////////////////
}

wxPanel* OptionsPropertySheetDlg::CreateBackupsPanel()
{
  wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), ID_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxCheckBox* backup_SaveImmediatelyCB = new wxCheckBox( itemPanel2, ID_CHECKBOX10, _("Save database immediately after any change"), wxDefaultPosition, wxDefaultSize, 0 );
  backup_SaveImmediatelyCB->SetValue(false);
  itemBoxSizer3->Add(backup_SaveImmediatelyCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Intermediate Backups"));
  auto *itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
  itemBoxSizer3->Add(itemStaticBoxSizer5, 0, wxEXPAND|wxALL, 5);
  wxCheckBox* backup_BackupBeforeSaveCB = new wxCheckBox( itemPanel2, ID_CHECKBOX11, _("Create intermediate backups (.ibak) before saving"), wxDefaultPosition, wxDefaultSize, 0 );
  backup_BackupBeforeSaveCB->SetValue(false);
  itemStaticBoxSizer5->Add(backup_BackupBeforeSaveCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Backup Name"));
  auto *itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
  itemStaticBoxSizer5->Add(itemStaticBoxSizer7, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText8 = new wxStaticText( itemPanel2, wxID_STATIC, _("Base:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 5);

  m_Backup_DefaultPrefixRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON4, _("Database name"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  m_Backup_DefaultPrefixRB->SetValue(false);
  itemStaticBoxSizer7->Add(m_Backup_DefaultPrefixRB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer10, 0, wxEXPAND|wxALL, 0);
  m_Backup_UserPrefixRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON5, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_Backup_UserPrefixRB->SetValue(false);
  itemBoxSizer10->Add(m_Backup_UserPrefixRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Backup_UserPrefixTXT = new wxTextCtrl( itemPanel2, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(100, -1)).x, -1), 0 );
  itemBoxSizer10->Add(m_Backup_UserPrefixTXT, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine13 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer7->Add(itemStaticLine13, 0, wxEXPAND|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemPanel2, wxID_STATIC, _("Suffix:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer15, 0, wxEXPAND|wxALL, 0);
  wxArrayString Backup_SuffixCBStrings;
  for (int i = 0; i < int(sizeof(BACKUP_SUFFIX)/sizeof(BACKUP_SUFFIX[0])); ++i) {
    Backup_SuffixCBStrings.Add(_(BACKUP_SUFFIX[i]));
  }
  m_Backup_SuffixCB = new wxComboBox( itemPanel2, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(140, -1)).x, -1), Backup_SuffixCBStrings, wxCB_READONLY );
  itemBoxSizer15->Add(m_Backup_SuffixCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemPanel2, wxID_STATIC, _("Max."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Backup_MaxIncrSB = new wxSpinCtrl(
    itemPanel2, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::BackupMaxIncremented),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::BackupMaxIncremented),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::BackupMaxIncremented)
  );

  FixInitialSpinnerSize(m_Backup_MaxIncrSB);

  itemBoxSizer15->Add(m_Backup_MaxIncrSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer19, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText20 = new wxStaticText( itemPanel2, wxID_STATIC, _("Example:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer19->Add(itemStaticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Backup_SuffixExampleST = new wxStaticText( itemPanel2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(160, -1)).x, -1), 0 );
  itemBoxSizer19->Add(m_Backup_SuffixExampleST, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine22 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer5->Add(itemStaticLine22, 0, wxEXPAND|wxALL, 5);

  wxStaticText* itemStaticText23 = new wxStaticText( itemPanel2, wxID_STATIC, _("Backup directory:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer5->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALL, 5);

  m_Backup_DefaultDirRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON6, _("Same as database's"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  m_Backup_DefaultDirRB->SetValue(false);
  itemStaticBoxSizer5->Add(m_Backup_DefaultDirRB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer5->Add(itemBoxSizer25, 0, wxEXPAND|wxALL, 0);
  m_Backup_UserDirRB = new wxRadioButton( itemPanel2, ID_RADIOBUTTON7, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_Backup_UserDirRB->SetValue(false);
  itemBoxSizer25->Add(m_Backup_UserDirRB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Backup_UserDirTXT = new wxTextCtrl( itemPanel2, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(m_Backup_UserDirTXT, 1, wxEXPAND|wxALL, 5);

  m_Backup_DirBN = new wxButton( itemPanel2, ID_BUTTON, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(m_Backup_DirBN, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Backups Preferences
  backup_SaveImmediatelyCB->SetValidator( wxGenericValidator(& m_Backup_SaveImmediately) );
  backup_BackupBeforeSaveCB->SetValidator( wxGenericValidator(& m_Backup_BackupBeforeSave) );

  // Connect events and objects
  m_Backup_UserPrefixTXT->Connect(ID_TEXTCTRL9, wxEVT_SET_FOCUS, wxFocusEventHandler(OptionsPropertySheetDlg::OnBuPrefixTxtSetFocus), nullptr, this);

  return itemPanel2;
}

wxPanel* OptionsPropertySheetDlg::CreateDisplayPanel()
{
  wxPanel* itemPanel29 = new wxPanel( GetBookCtrl(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer30 = new wxBoxSizer(wxVERTICAL);
  itemPanel29->SetSizer(itemBoxSizer30);

  wxCheckBox* display_AlwaysOnTopCB = new wxCheckBox( itemPanel29, ID_CHECKBOX12, _("Always keep Password Safe on top"), wxDefaultPosition, wxDefaultSize, 0 );
  display_AlwaysOnTopCB->SetValue(false);
  itemBoxSizer30->Add(display_AlwaysOnTopCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_ShowUsernameInTreeCB = new wxCheckBox( itemPanel29, ID_CHECKBOX13, _("Show Username in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  display_ShowUsernameInTreeCB->SetValue(false);
  itemBoxSizer30->Add(display_ShowUsernameInTreeCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_ShowPasswordInTreeCB = new wxCheckBox( itemPanel29, ID_CHECKBOX14, _("Show Password in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  display_ShowPasswordInTreeCB->SetValue(false);
  itemBoxSizer30->Add(display_ShowPasswordInTreeCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_ShowPasswordInEditCB = new wxCheckBox( itemPanel29, ID_CHECKBOX16, _("Show Password in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  display_ShowPasswordInEditCB->SetValue(false);
  itemBoxSizer30->Add(display_ShowPasswordInEditCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_ShowNotesInEditCB = new wxCheckBox( itemPanel29, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  display_ShowNotesInEditCB->SetValue(false);
  itemBoxSizer30->Add(display_ShowNotesInEditCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_ShowNotesAsTipsInViewsCB = new wxCheckBox( itemPanel29, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  display_ShowNotesAsTipsInViewsCB->SetValue(false);
  itemBoxSizer30->Add(display_ShowNotesAsTipsInViewsCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_WordWrapNotesCB = new wxCheckBox( itemPanel29, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  display_WordWrapNotesCB->SetValue(false);
  itemBoxSizer30->Add(display_WordWrapNotesCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* display_GroupsFirstCB = new wxCheckBox( itemPanel29, ID_CHECKBOX38, _("Put Groups first in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  display_GroupsFirstCB->SetValue(false);
  itemBoxSizer30->Add(display_GroupsFirstCB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer39 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer30->Add(itemBoxSizer39, 0, wxEXPAND|wxALL, 0);
  wxCheckBox* display_PreExpiryWarnCB = new wxCheckBox( itemPanel29, ID_CHECKBOX19, _("Warn"), wxDefaultPosition, wxDefaultSize, 0 );
  display_PreExpiryWarnCB->SetValue(false);
  itemBoxSizer39->Add(display_PreExpiryWarnCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* display_PreExpiryWarnDaysSB = new wxSpinCtrl(
    itemPanel29, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PreExpiryWarnDays),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PreExpiryWarnDays),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::PreExpiryWarnDays)
  );

  FixInitialSpinnerSize(display_PreExpiryWarnDaysSB);

  itemBoxSizer39->Add(display_PreExpiryWarnDaysSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText42 = new wxStaticText( itemPanel29, wxID_STATIC, _("days before passwords expire"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer39->Add(itemStaticText42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemRadioBox43Strings;
  itemRadioBox43Strings.Add(_("&Fully collapsed"));
  itemRadioBox43Strings.Add(_("&Fully expanded"));
  itemRadioBox43Strings.Add(_("&Same as when last saved"));
  wxRadioBox* display_TreeDisplayStatusAtOpenRB = new wxRadioBox( itemPanel29, ID_RADIOBOX, _("Initial Tree View"), wxDefaultPosition, wxDefaultSize, itemRadioBox43Strings, 1, wxRA_SPECIFY_COLS );
  display_TreeDisplayStatusAtOpenRB->SetSelection(0);
  itemBoxSizer30->Add(display_TreeDisplayStatusAtOpenRB, 0, wxEXPAND|wxALL, 5);

  // Display Preferences
  display_AlwaysOnTopCB->SetValidator( wxGenericValidator(& m_Display_AlwaysOnTop) );
  display_ShowUsernameInTreeCB->SetValidator( wxGenericValidator(& m_Display_ShowUsernameInTree) );
  display_ShowNotesAsTipsInViewsCB->SetValidator( wxGenericValidator(& m_Display_ShowNotesAsTipsInViews) );
  display_ShowPasswordInEditCB->SetValidator( wxGenericValidator(& m_Display_ShowPasswordInEdit) );
  display_ShowNotesInEditCB->SetValidator( wxGenericValidator(& m_Display_ShowNotesInEdit) );
  display_WordWrapNotesCB->SetValidator( wxGenericValidator(& m_Display_WordWrapNotes) );
  display_GroupsFirstCB->SetValidator( wxGenericValidator(& m_Display_GroupsFirst) );
  display_PreExpiryWarnCB->SetValidator( wxGenericValidator(& m_Display_PreExpiryWarn) );
  display_TreeDisplayStatusAtOpenRB->SetValidator( wxGenericValidator(& m_Display_TreeDisplayStatusAtOpen) );

  return itemPanel29;
}

wxPanel* OptionsPropertySheetDlg::CreateMiscellaneousPanel()
{
  wxPanel* itemPanel44 = new wxPanel( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
  itemPanel44->SetSizer(itemBoxSizer45);

  wxCheckBox* misc_ConfirmDeleteCB = new wxCheckBox( itemPanel44, ID_CHECKBOX20, _("Confirm deletion of items"), wxDefaultPosition, wxDefaultSize, 0 );
  misc_ConfirmDeleteCB->SetValue(false);
  itemBoxSizer45->Add(misc_ConfirmDeleteCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* misc_MaintainDatetimeStampsCB = new wxCheckBox( itemPanel44, ID_CHECKBOX21, _("Record last access times"), wxDefaultPosition, wxDefaultSize, 0 );
  misc_MaintainDatetimeStampsCB->SetValue(false);
  itemBoxSizer45->Add(misc_MaintainDatetimeStampsCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* misc_EscExitsCB = new wxCheckBox( itemPanel44, ID_CHECKBOX22, _("Escape key closes application"), wxDefaultPosition, wxDefaultSize, 0 );
  misc_EscExitsCB->SetValue(false);
  itemBoxSizer45->Add(misc_EscExitsCB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemFlexGridSizer50 = new wxFlexGridSizer(0, 2, 0, 0);
  itemBoxSizer45->Add(itemFlexGridSizer50, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText50 = new wxStaticText( itemPanel44, wxID_STATIC, _("Double-Click Action:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer50->Add(itemStaticText50, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5);

  // Prepare strings for DblClick & Shift+DblClick combos
  wxArrayString m_Misc_DoubleClickActionCBStrings;
  wxArrayString m_Misc_ShiftDoubleClickActionCBStrings;
  for (int i = 0; i < int(sizeof(DCAStrings)/sizeof(DCAStrings[0])); ++i) {
    wxString tmp = _(DCAStrings[i]);
    m_Misc_DoubleClickActionCBStrings.Add(tmp);
    m_Misc_ShiftDoubleClickActionCBStrings.Add(tmp);
  }

  // This is to avoid a nasty assert on OSX with wx3.0.2
  auto cbStyle = wxCB_READONLY;
#ifndef  __WXMAC__
  cbStyle |= wxCB_SORT;
#endif

  m_Misc_DoubleClickActionCB = new wxComboBox( itemPanel44, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Misc_DoubleClickActionCBStrings, cbStyle );
  itemFlexGridSizer50->Add(m_Misc_DoubleClickActionCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText54 = new wxStaticText( itemPanel44, wxID_STATIC, _("Shift-Double-Click Action:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer50->Add(itemStaticText54, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5);

  m_Misc_ShiftDoubleClickActionCB = new wxComboBox( itemPanel44, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Misc_ShiftDoubleClickActionCBStrings, cbStyle );
  itemFlexGridSizer50->Add(m_Misc_ShiftDoubleClickActionCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer56Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Autotype"));
  auto *itemStaticBoxSizer56 = new wxStaticBoxSizer(itemStaticBoxSizer56Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer56, 0, wxEXPAND|wxALL, 5);
  wxCheckBox* misc_AutotypeMinimizeCB = new wxCheckBox( itemPanel44, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  misc_AutotypeMinimizeCB->SetValue(false);
  itemStaticBoxSizer56->Add(misc_AutotypeMinimizeCB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer56->Add(itemBoxSizer58, 0, wxEXPAND|wxALL, 0);
  wxStaticText* itemStaticText59 = new wxStaticText( itemPanel44, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemStaticText59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* misc_AutotypeStringTXT = new wxTextCtrl( itemPanel44, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(misc_AutotypeStringTXT, 2, wxEXPAND|wxALL, 5);
  itemBoxSizer58->AddStretchSpacer();

  wxStaticBox* itemStaticBoxSizer61Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Default Username"));
  auto *itemStaticBoxSizer61 = new wxStaticBoxSizer(itemStaticBoxSizer61Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer61, 0, wxEXPAND|wxALL, 5);
  auto *itemBoxSizer62 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer61->Add(itemBoxSizer62, 0, wxEXPAND|wxALL, 0);
  wxCheckBox* misc_UseDefUsernameCB = new wxCheckBox( itemPanel44, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  misc_UseDefUsernameCB->SetValue(false);
  itemBoxSizer62->Add(misc_UseDefUsernameCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Misc_DefaultUsernameTXT = new wxTextCtrl( itemPanel44, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer62->Add(m_Misc_DefaultUsernameTXT, 1, wxEXPAND|wxALL, 5);

  m_Misc_DefaultUsernameLBL = new wxStaticText( itemPanel44, ID_STATICTEXT_1, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer62->Add(m_Misc_DefaultUsernameLBL, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  itemBoxSizer62->AddStretchSpacer();

  wxCheckBox* misc_QuerySetDefUsernameCB = new wxCheckBox( itemPanel44, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  misc_QuerySetDefUsernameCB->SetValue(false);
  itemStaticBoxSizer61->Add(misc_QuerySetDefUsernameCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer67Static = new wxStaticBox(itemPanel44, wxID_ANY, _("Alternate Browser"));
  auto *itemStaticBoxSizer67 = new wxStaticBoxSizer(itemStaticBoxSizer67Static, wxVERTICAL);
  itemBoxSizer45->Add(itemStaticBoxSizer67, 0, wxEXPAND|wxALL, 5);
  auto *itemBoxSizer68 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer67->Add(itemBoxSizer68, 0, wxEXPAND|wxALL, 0);
  wxTextCtrl* misc_OtherBrowserLocationTXT = new wxTextCtrl( itemPanel44, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer68->Add(misc_OtherBrowserLocationTXT, 1, wxEXPAND|wxALL, 5);

  wxButton* itemButton70 = new wxButton( itemPanel44, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer68->Add(itemButton70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer67->Add(itemBoxSizer71, 0, wxEXPAND|wxALL, 0);

  wxStaticText* itemStaticText73 = new wxStaticText( itemPanel44, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer71->Add(itemStaticText73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl72 = new wxTextCtrl( itemPanel44, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer71->Add(itemTextCtrl72, 1, wxEXPAND|wxALL, 5);

  // Miscellaneous Preferences
  misc_ConfirmDeleteCB->SetValidator( wxGenericValidator(& m_Misc_ConfirmDelete) );
  misc_MaintainDatetimeStampsCB->SetValidator( wxGenericValidator(& m_Misc_MaintainDatetimeStamps) );
  misc_EscExitsCB->SetValidator( wxGenericValidator(& m_Misc_EscExits) );
  misc_AutotypeMinimizeCB->SetValidator( wxGenericValidator(& m_Misc_AutotypeMinimize) );
  misc_AutotypeStringTXT->SetValidator( wxGenericValidator(& m_Misc_AutotypeString) );
  misc_UseDefUsernameCB->SetValidator( wxGenericValidator(& m_Misc_UseDefUsername) );
  misc_QuerySetDefUsernameCB->SetValidator( wxGenericValidator(& m_Misc_QuerySetDefUsername) );
  misc_OtherBrowserLocationTXT->SetValidator( wxGenericValidator(& m_Misc_OtherBrowserLocation) );

  return itemPanel44;
}

wxPanel* OptionsPropertySheetDlg::CreatePasswordHistoryPanel()
{
  wxPanel* itemPanel74 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer75 = new wxBoxSizer(wxVERTICAL);
  itemPanel74->SetSizer(itemBoxSizer75);

  auto *itemBoxSizer76 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer75->Add(itemBoxSizer76, 0, wxEXPAND|wxALL, 5);
  m_PasswordHistory_SaveCB = new wxCheckBox( itemPanel74, ID_CHECKBOX26, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_SaveCB->SetValue(false);
  itemBoxSizer76->Add(m_PasswordHistory_SaveCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_PasswordHistory_NumDefaultSB = new wxSpinCtrl(
    itemPanel74, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::NumPWHistoryDefault),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::NumPWHistoryDefault),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::NumPWHistoryDefault)
  );

  FixInitialSpinnerSize(m_PasswordHistory_NumDefaultSB);

  itemBoxSizer76->Add(m_PasswordHistory_NumDefaultSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText79 = new wxStaticText( itemPanel74, ID_STATICTEXT_8, _("previous passwords per entry"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer76->Add(itemStaticText79, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer77 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer75->Add(itemBoxSizer77, 0, wxEXPAND|wxALL, 5);

  itemBoxSizer77->Add(
    new wxStaticText(itemPanel74, ID_STATICTEXT_9, _("Default password expiration (days)"), wxDefaultPosition, wxDefaultSize, 0),
    0, wxALIGN_CENTER_VERTICAL|wxALL, 5
  );

  m_PasswordHistory_DefaultExpiryDaysSB = new wxSpinCtrl(
    itemPanel74, ID_SPINCTRL14, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::DefaultExpiryDays)
  );

  FixInitialSpinnerSize(m_PasswordHistory_DefaultExpiryDaysSB);

  itemBoxSizer77->Add(m_PasswordHistory_DefaultExpiryDaysSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer80Static = new wxStaticBox(itemPanel74, ID_STATICBOX_1, _("Manage password history of current entries"));
  auto *itemStaticBoxSizer80 = new wxStaticBoxSizer(itemStaticBoxSizer80Static, wxVERTICAL);
  itemBoxSizer75->Add(itemStaticBoxSizer80, 0, wxEXPAND|wxALL, 5);
  m_PasswordHistory_NoChangeRB = new wxRadioButton( itemPanel74, ID_PWHISTNOCHANGE, _("No change"), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_NoChangeRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_PasswordHistory_NoChangeRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_PasswordHistory_StopRB = new wxRadioButton( itemPanel74, ID_PWHISTSTOP, _("Stop saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_StopRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_PasswordHistory_StopRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_PasswordHistory_StartRB = new wxRadioButton( itemPanel74, ID_PWHISTSTART, _("Start saving previous passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_StartRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_PasswordHistory_StartRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_PasswordHistory_SetMaxRB = new wxRadioButton( itemPanel74, ID_PWHISTSETMAX, _("Set maximum number of passwords saved to above value"), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_SetMaxRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_PasswordHistory_SetMaxRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_PasswordHistory_ClearRB = new wxRadioButton( itemPanel74, ID_PWHISTCLEAR, _("Clear password history for ALL entries"), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_ClearRB->SetValue(false);
  itemStaticBoxSizer80->Add(m_PasswordHistory_ClearRB, 0, wxALIGN_LEFT|wxALL, 5);

  m_PasswordHistory_Apply2ProtectedCB = new wxCheckBox( itemPanel74, ID_APPLYTOPROTECTED, _("Apply these changes to Protected Entries (if required)."), wxDefaultPosition, wxDefaultSize, 0 );
  m_PasswordHistory_Apply2ProtectedCB->SetValue(false);
  itemStaticBoxSizer80->Add(m_PasswordHistory_Apply2ProtectedCB, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER|wxALL, 5);

  m_PasswordHistory_ApplyBN = new wxButton( itemPanel74, ID_PWHISTAPPLY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer80->Add(m_PasswordHistory_ApplyBN, 0, wxALIGN_LEFT|wxALL, 5);

  // Password History Preferences
  m_PasswordHistory_SaveCB->SetValidator( wxGenericValidator(& m_PasswordHistory_Save) );
  m_PasswordHistory_NumDefaultSB->SetValidator( wxGenericValidator(& m_PasswordHistory_NumDefault) );
  m_PasswordHistory_DefaultExpiryDaysSB->SetValidator( wxGenericValidator(& m_PasswordHistory_DefaultExpiryDays) );

  return itemPanel74;
}

wxPanel* OptionsPropertySheetDlg::CreateSecurityPanel()
{
  wxPanel* itemPanel86 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer87 = new wxBoxSizer(wxVERTICAL);
  itemPanel86->SetSizer(itemBoxSizer87);

  wxCheckBox* security_ClearClipboardOnMinimizeCB = new wxCheckBox( itemPanel86, ID_CHECKBOX27, _("Clear clipboard upon minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  security_ClearClipboardOnMinimizeCB->SetValue(false);
  itemBoxSizer87->Add(security_ClearClipboardOnMinimizeCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* security_ClearClipboardOnExitCB = new wxCheckBox( itemPanel86, ID_CHECKBOX, _("Clear clipboard upon exit"), wxDefaultPosition, wxDefaultSize, 0 );
  security_ClearClipboardOnExitCB->SetValue(false);
  itemBoxSizer87->Add(security_ClearClipboardOnExitCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* security_ConfirmCopyCB = new wxCheckBox( itemPanel86, ID_CHECKBOX1, _("Confirm copy of password to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  security_ConfirmCopyCB->SetValue(false);
  itemBoxSizer87->Add(security_ConfirmCopyCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* security_CopyPswdBrowseURLCB = new wxCheckBox( itemPanel86, ID_CHECKBOX35, _("'Browse to URL' copies password to clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  security_CopyPswdBrowseURLCB->SetValue(false);
  itemBoxSizer87->Add(security_CopyPswdBrowseURLCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* security_LockOnMinimizeCB = new wxCheckBox( itemPanel86, ID_CHECKBOX2, _("Lock password database on minimize"), wxDefaultPosition, wxDefaultSize, 0 );
  security_LockOnMinimizeCB->SetValue(false);
  itemBoxSizer87->Add(security_LockOnMinimizeCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* security_LockOnWindowLockCB = new wxCheckBox( itemPanel86, ID_CHECKBOX28, _("Lock password database on workstation lock"), wxDefaultPosition, wxDefaultSize, 0 );
  security_LockOnWindowLockCB->SetValue(false);
  itemBoxSizer87->Add(security_LockOnWindowLockCB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer93 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer87->Add(itemBoxSizer93, 0, wxEXPAND|wxALL, 0);
  m_Security_LockOnIdleTimeoutCB = new wxCheckBox( itemPanel86, ID_CHECKBOX29, _("Lock password database after"), wxDefaultPosition, wxDefaultSize, 0 );
  m_Security_LockOnIdleTimeoutCB->SetValue(false);
  itemBoxSizer93->Add(m_Security_LockOnIdleTimeoutCB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Security_IdleTimeoutSB = new wxSpinCtrl(
    itemPanel86, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::IdleTimeout),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::IdleTimeout),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::IdleTimeout)
  );

  FixInitialSpinnerSize(m_Security_IdleTimeoutSB);

  itemBoxSizer93->Add(m_Security_IdleTimeoutSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText96 = new wxStaticText( itemPanel86, ID_STATICTEXT_2, _("minutes idle"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer93->Add(itemStaticText96, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer87->Add(itemBoxSizer97, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText98 = new wxStaticText( itemPanel86, ID_STATICTEXT_3, _("Unlock Difficulty:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer97->Add(itemStaticText98, 0, wxALIGN_LEFT|wxALL, 5);

  wxSlider* security_HashIterSliderSL = new wxSlider( itemPanel86, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS );
  itemBoxSizer97->Add(security_HashIterSliderSL, 0, wxEXPAND|wxALL, 5);

  auto *itemBoxSizer100 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer97->Add(itemBoxSizer100, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText101 = new wxStaticText( itemPanel86, ID_STATICTEXT_4, _("Standard"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer100->Add(itemStaticText101, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer100->Add(10, 13, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText103 = new wxStaticText( itemPanel86, ID_STATICTEXT_5, _("Maximum"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer100->Add(itemStaticText103, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Security Preferences
  security_ClearClipboardOnMinimizeCB->SetValidator( wxGenericValidator(& m_Security_ClearClipboardOnMinimize) );
  security_ClearClipboardOnExitCB->SetValidator( wxGenericValidator(& m_Security_ClearClipboardOnExit) );
  security_ConfirmCopyCB->SetValidator( wxGenericValidator(& m_Security_ConfirmCopy) );
  security_CopyPswdBrowseURLCB->SetValidator( wxGenericValidator(& m_Security_CopyPswdBrowseURL) );
  security_LockOnMinimizeCB->SetValidator( wxGenericValidator(& m_Security_LockOnMinimize) );
  security_LockOnWindowLockCB->SetValidator( wxGenericValidator(& m_Security_LockOnWindowLock) );
  m_Security_LockOnIdleTimeoutCB->SetValidator( wxGenericValidator(& m_Security_LockOnIdleTimeout) );
  security_HashIterSliderSL->SetValidator( wxGenericValidator(& m_Security_HashIterSlider) );

  return itemPanel86;
}

wxPanel* OptionsPropertySheetDlg::CreateShortcutsPanel()
{
  wxPanel* itemPanel123 = new wxPanel(GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto *itemBoxSizer155 = new wxBoxSizer(wxVERTICAL);
  itemPanel123->SetSizer(itemBoxSizer155);

  wxGrid* itemGrid124 = new wxGrid(itemPanel123, ID_GRID1, wxDefaultPosition, itemPanel123->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
  itemGrid124->SetDefaultColSize(150);
  itemGrid124->SetDefaultRowSize(25);
  itemGrid124->SetColLabelSize(25);
  itemGrid124->SetRowLabelSize(50);
  itemGrid124->CreateGrid(50, 2, wxGrid::wxGridSelectCells);

  itemBoxSizer155->Add(itemGrid124, 1, wxEXPAND|wxALL, 5);

  return itemPanel123;
}

wxPanel* OptionsPropertySheetDlg::CreateSystemPanel()
{
  wxPanel* itemPanel104 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
  auto *itemBoxSizer105 = new wxBoxSizer(wxVERTICAL);
  itemPanel104->SetSizer(itemBoxSizer105);

  wxStaticBox* itemStaticBoxSizer106Static = new wxStaticBox(itemPanel104, wxID_ANY, _("System Tray"));
  auto *itemStaticBoxSizer106 = new wxStaticBoxSizer(itemStaticBoxSizer106Static, wxVERTICAL);
  itemBoxSizer105->Add(itemStaticBoxSizer106, 0, wxEXPAND|wxALL, 5);
  m_System_UseSystemTrayCB = new wxCheckBox( itemPanel104, ID_CHECKBOX30, _("Put icon in System Tray"), wxDefaultPosition, wxDefaultSize, 0 );
  m_System_UseSystemTrayCB->SetValue(false);
  itemStaticBoxSizer106->Add(m_System_UseSystemTrayCB, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer108 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer106->Add(itemBoxSizer108, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText109 = new wxStaticText( itemPanel104, ID_STATICTEXT_10, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer108->Add(itemStaticText109, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_System_MaxREItemsSB = new wxSpinCtrl(
    itemPanel104, ID_SPINCTRL13, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::MaxREItems),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::MaxREItems),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::MaxREItems)
  );

  FixInitialSpinnerSize(m_System_MaxREItemsSB);

  itemBoxSizer108->Add(m_System_MaxREItemsSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText111 = new wxStaticText( itemPanel104, ID_STATICTEXT_7, _("used entries in System Tray menu"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer108->Add(itemStaticText111, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* system_StartupCB = new wxCheckBox( itemPanel104, ID_CHECKBOX31, _("Start PasswordSafe at Login"), wxDefaultPosition, wxDefaultSize, 0 );
  system_StartupCB->SetValue(false);
  itemStaticBoxSizer106->Add(system_StartupCB, 0, wxALIGN_LEFT|wxALL, 5);

  m_System_SystemTrayWarningST = new wxStaticText( itemPanel104, wxID_STATIC, _("There appears to be no system tray support in your current environment.\nAny related functionality may not work as expected."), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer106->Add(m_System_SystemTrayWarningST, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  m_System_SystemTrayWarningST->SetForegroundColour(*wxRED);
  m_System_SystemTrayWarningST->Hide();

  wxStaticBox* itemStaticBoxSizer113Static = new wxStaticBox(itemPanel104, wxID_ANY, _("Recent PasswordSafe Databases"));
  auto *itemStaticBoxSizer113 = new wxStaticBoxSizer(itemStaticBoxSizer113Static, wxVERTICAL);
  itemBoxSizer105->Add(itemStaticBoxSizer113, 0, wxEXPAND|wxALL, 5);
  auto *itemBoxSizer114 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer113->Add(itemBoxSizer114, 0, wxEXPAND|wxALL, 5);
  wxStaticText* itemStaticText115 = new wxStaticText( itemPanel104, wxID_STATIC, _("  Remember last"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer114->Add(itemStaticText115, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* system_MaxMRUItemsSB = new wxSpinCtrl(
    itemPanel104, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
    PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::MaxMRUItems),
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::MaxMRUItems),
    PWSprefs::GetInstance()->GetPrefDefVal(PWSprefs::MaxMRUItems)
  );

  FixInitialSpinnerSize(system_MaxMRUItemsSB);

  itemBoxSizer114->Add(system_MaxMRUItemsSB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText117 = new wxStaticText( itemPanel104, wxID_STATIC, _("databases"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer114->Add(itemStaticText117, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* system_MRUOnFileMenuCB = new wxCheckBox( itemPanel104, ID_CHECKBOX32, _("Recent Databases on File Menu rather than as a sub-menu"), wxDefaultPosition, wxDefaultSize, 0 );
  system_MRUOnFileMenuCB->SetValue(false);
  itemStaticBoxSizer113->Add(system_MRUOnFileMenuCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* system_DefaultOpenROCB = new wxCheckBox( itemPanel104, ID_CHECKBOX33, _("Open database as read-only by default"), wxDefaultPosition, wxDefaultSize, 0 );
  system_DefaultOpenROCB->SetValue(false);
  itemBoxSizer105->Add(system_DefaultOpenROCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* system_MultipleInstancesCB = new wxCheckBox( itemPanel104, ID_CHECKBOX34, _("Allow multiple instances"), wxDefaultPosition, wxDefaultSize, 0 );
  system_MultipleInstancesCB->SetValue(false);
  itemBoxSizer105->Add(system_MultipleInstancesCB, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXX11__) || defined(__WXGTK__)
  wxCheckBox* system_UsePrimarySelectionCB = new wxCheckBox( itemPanel104, ID_CHECKBOX39, _("Use Primary Selection for clipboard"), wxDefaultPosition, wxDefaultSize, 0 );
  system_UsePrimarySelectionCB->SetValue(false);
  itemBoxSizer105->Add(system_UsePrimarySelectionCB, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* system_UseAltAutoTypeCB = new wxCheckBox( itemPanel104, ID_CHECKBOX40, _("Use alternate AutoType method"), wxDefaultPosition, wxDefaultSize, 0 );
  system_UseAltAutoTypeCB->SetValue(false);
  system_UseAltAutoTypeCB->SetHelpText(_("When set, use XTEST for AutoType instead of XSendEvent.\nXSendEvent can handle more control keys, but may be blocked by some applications."));
  if (OptionsPropertySheetDlg::ShowToolTips())
    system_UseAltAutoTypeCB->SetToolTip(_("If AutoType doesn't work, setting this may help."));
  itemBoxSizer105->Add(system_UseAltAutoTypeCB, 0, wxALIGN_LEFT|wxALL, 5);
#endif

  // System Preferences
  system_StartupCB->SetValidator( wxGenericValidator(& m_System_Startup) );
  system_MaxMRUItemsSB->SetValidator( wxGenericValidator(& m_System_MaxMRUItems) );
  system_MRUOnFileMenuCB->SetValidator( wxGenericValidator(& m_System_MRUOnFileMenu) );
  system_DefaultOpenROCB->SetValidator( wxGenericValidator(& m_System_DefaultOpenRO) );
  system_MultipleInstancesCB->SetValidator( wxGenericValidator(& m_System_MultipleInstances) );
#if defined(__WXX11__) || defined(__WXGTK__)
  system_UsePrimarySelectionCB->SetValidator( wxGenericValidator(& m_System_UsePrimarySelection) );
  system_UseAltAutoTypeCB->SetValidator( wxGenericValidator(& m_System_UseAltAutoType) );
#endif

  return itemPanel104;
}

/*!
 * Should we show tooltips?
 */

bool OptionsPropertySheetDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap OptionsPropertySheetDlg::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin OptionsPropertySheetDlg bitmap retrieval
  return wxNullBitmap;
////@end OptionsPropertySheetDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon OptionsPropertySheetDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin OptionsPropertySheetDlg icon retrieval
  return wxNullIcon;
////@end OptionsPropertySheetDlg icon retrieval
}

void OptionsPropertySheetDlg::PrefsToPropSheet()
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Backup-related preferences
  m_Backup_SaveImmediately = prefs->GetPref(PWSprefs::SaveImmediately);
  m_Backup_BackupBeforeSave = prefs->GetPref(PWSprefs::BackupBeforeEverySave);
  wxString buprefixValue = prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
  m_Backup_DefaultPrefixRB->SetValue(buprefixValue.empty());
  m_Backup_UserPrefixRB->SetValue(!buprefixValue.empty());
  m_Backup_UserPrefixTXT->SetValue(buprefixValue);
  int suffixIndex = prefs->GetPref(PWSprefs::BackupSuffix);
  m_Backup_SuffixCB->Select(suffixIndex);
  m_Backup_MaxIncrSB->SetValue(prefs->GetPref(PWSprefs::BackupMaxIncremented));
  wxString budirValue = prefs->GetPref(PWSprefs::BackupDir).c_str();
  m_Backup_DefaultDirRB->SetValue(budirValue.empty());
  m_Backup_UserDirRB->SetValue(!budirValue.empty());
  m_Backup_UserDirTXT->SetValue(budirValue);

  // display-related preferences
  m_Display_AlwaysOnTop = prefs->GetPref(PWSprefs::AlwaysOnTop);
  m_Display_ShowUsernameInTree = prefs->GetPref(PWSprefs::ShowUsernameInTree);
  m_Display_ShowPasswordInTreeCB->SetValue(m_Display_ShowUsernameInTree && prefs->
                                   GetPref(PWSprefs::ShowPasswordInTree));
  m_Display_ShowPasswordInTreeCB->Enable(m_Display_ShowUsernameInTree);
  m_Display_ShowNotesAsTipsInViews = prefs->
    GetPref(PWSprefs::ShowNotesAsTooltipsInViews);
  m_Display_ShowPasswordInEdit = prefs->GetPref(PWSprefs::ShowPWDefault);
  m_Display_ShowNotesInEdit = prefs->GetPref(PWSprefs::ShowNotesDefault);
  m_Display_WordWrapNotes = prefs->GetPref(PWSprefs::NotesWordWrap);
  m_Display_GroupsFirst = prefs->GetPref(PWSprefs::ExplorerTypeTree);
  m_Display_PreExpiryWarn = prefs->GetPref(PWSprefs::PreExpiryWarn);
  m_Display_PreExpiryWarnDaysSB->SetValue(prefs->GetPref(PWSprefs::PreExpiryWarnDays));
  m_Display_PreExpiryWarnDaysSB->Enable(m_Display_PreExpiryWarn);
  m_Display_TreeDisplayStatusAtOpen = prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen);

  // Misc. preferences
  m_Misc_ConfirmDelete = !prefs->GetPref(PWSprefs::DeleteQuestion);
  m_Misc_MaintainDatetimeStamps = prefs->GetPref(PWSprefs::MaintainDateTimeStamps);
  m_Misc_EscExits = prefs->GetPref(PWSprefs::EscExits);
  m_DoubleClickAction = prefs->GetPref(PWSprefs::DoubleClickAction);
  if (m_DoubleClickAction < 0 ||
      m_DoubleClickAction >= int(sizeof(DCAStrings)/sizeof(DCAStrings[0])))
    m_DoubleClickAction = 0;
  m_Misc_DoubleClickActionCB->SetValue(_(DCAStrings[m_DoubleClickAction]));
  m_ShiftDoubleClickAction = prefs->GetPref(PWSprefs::ShiftDoubleClickAction);
  if (m_ShiftDoubleClickAction < 0 ||
      m_ShiftDoubleClickAction >= int(sizeof(DCAStrings)/sizeof(DCAStrings[0])))
    m_ShiftDoubleClickAction = 0;
  m_Misc_ShiftDoubleClickActionCB->SetValue(_(DCAStrings[m_ShiftDoubleClickAction]));
  m_Misc_AutotypeMinimize = prefs->GetPref(PWSprefs::MinimizeOnAutotype);
  m_Misc_AutotypeString = prefs->GetPref(PWSprefs::DefaultAutotypeString).c_str();
  if (m_Misc_AutotypeString.empty())
    m_Misc_AutotypeString = DEFAULT_AUTOTYPE;
  m_Misc_UseDefUsername = prefs->GetPref(PWSprefs::UseDefaultUser);
  m_Misc_DefaultUsernameTXT->SetValue(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
  m_Misc_DefaultUsernameTXT->Enable(m_Misc_UseDefUsername);
  m_Misc_DefaultUsernameLBL->Enable(m_Misc_UseDefUsername);
  m_Misc_QuerySetDefUsername = prefs->GetPref(PWSprefs::QuerySetDef);
  m_Misc_OtherBrowserLocation = prefs->GetPref(PWSprefs::AltBrowser).c_str();
  m_Misc_OtherBrowserLocationparams = prefs->GetPref(PWSprefs::AltBrowserCmdLineParms).c_str();

  // Password History preferences
  m_PasswordHistory_Save = prefs->GetPref(PWSprefs::SavePasswordHistory);
  m_PasswordHistory_NumDefault = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
  m_PasswordHistory_NumDefaultSB->Enable(m_PasswordHistory_Save);
  m_PasswordHistory_DefaultExpiryDays = prefs->GetPref(PWSprefs::DefaultExpiryDays);

  // Security Preferences
  m_Security_ClearClipboardOnMinimize   = prefs->GetPref(PWSprefs::ClearClipboardOnMinimize);
  m_Security_ClearClipboardOnExit       = prefs->GetPref(PWSprefs::ClearClipboardOnExit);
  m_Security_ConfirmCopy                = prefs->GetPref(PWSprefs::DontAskQuestion);
  m_Security_CopyPswdBrowseURL          = prefs->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL);
  m_Security_LockOnMinimize             = prefs->GetPref(PWSprefs::DatabaseClear);
  m_Security_LockOnWindowLock           = prefs->GetPref(PWSprefs::LockOnWindowLock);
  m_Security_LockOnIdleTimeout          = prefs->GetPref(PWSprefs::LockDBOnIdleTimeout);
  m_Security_IdleTimeoutSB->SetValue(     prefs->GetPref(PWSprefs::IdleTimeout));

  auto *app = dynamic_cast<PWSafeApp *>(wxTheApp);
  uint32 hashIters = app->GetHashIters();
  if (hashIters <= MIN_HASH_ITERATIONS) {
    m_Security_HashIterSlider = 0;
  } else {
    const int step = MAX_USABLE_HASH_ITERS/100;
    m_Security_HashIterSlider = uint32(hashIters/step);
  }

  // System preferences
  m_System_MaxREItemsSB->SetValue(prefs->GetPref(PWSprefs::MaxREItems));
  m_System_UseSystemTrayCB->SetValue(prefs->GetPref(PWSprefs::UseSystemTray));
  if (!IsTaskBarIconAvailable()) {
    m_System_SystemTrayWarningST->Show();
  }
  m_System_Startup = false; // XXX TBD
  m_System_MaxMRUItems = prefs->GetPref(PWSprefs::MaxMRUItems);
  m_System_MRUOnFileMenu = prefs->GetPref(PWSprefs::MRUOnFileMenu);
  m_System_DefaultOpenRO = prefs->GetPref(PWSprefs::DefaultOpenRO);
  m_System_MultipleInstances = prefs->GetPref(PWSprefs::MultipleInstances);
#if defined(__X__) || defined(__WXGTK__)
  m_System_UsePrimarySelection = prefs->GetPref(PWSprefs::UsePrimarySelectionForClipboard);
  m_System_UseAltAutoType = prefs->GetPref(PWSprefs::UseAltAutoType);
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

void OptionsPropertySheetDlg::PropSheetToPrefs()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  // Backup-related preferences
  prefs->SetPref(PWSprefs::BackupBeforeEverySave, m_Backup_BackupBeforeSave);
  wxString buprefixValue;
  if (m_Backup_UserPrefixRB->GetValue())
    buprefixValue = m_Backup_UserPrefixTXT->GetValue();
  prefs->SetPref(PWSprefs::BackupPrefixValue, tostringx(buprefixValue));
  int suffixIndex = m_Backup_SuffixCB->GetCurrentSelection();
  prefs->SetPref(PWSprefs::BackupSuffix, suffixIndex);
  if (suffixIndex == INC_SFX)
    prefs->SetPref(PWSprefs::BackupMaxIncremented, m_Backup_MaxIncrSB->GetValue());
  wxString budirValue;
  if (m_Backup_UserDirRB->GetValue())
    budirValue = m_Backup_UserDirTXT->GetValue();
  prefs->SetPref(PWSprefs::BackupDir, tostringx(budirValue));

  // display-related preferences
  prefs->SetPref(PWSprefs::AlwaysOnTop, m_Display_AlwaysOnTop);
  // set/clear wxSTAY_ON_TOP flag accordingly:
  long flags = GetParent()->GetWindowStyleFlag();
  if (m_Display_AlwaysOnTop)
    flags |= wxSTAY_ON_TOP;
  else
    flags &= ~wxSTAY_ON_TOP;
  GetParent()->SetWindowStyleFlag(flags);

  prefs->SetPref(PWSprefs::ShowNotesAsTooltipsInViews,
                 m_Display_ShowNotesAsTipsInViews);
  prefs->SetPref(PWSprefs::NotesWordWrap, m_Display_WordWrapNotes);
  prefs->SetPref(PWSprefs::ExplorerTypeTree, m_Display_GroupsFirst);
  prefs->SetPref(PWSprefs::PreExpiryWarn, m_Display_PreExpiryWarn);
  if (m_Display_PreExpiryWarn)
    prefs->SetPref(PWSprefs::PreExpiryWarnDays,
                   m_Display_PreExpiryWarnDaysSB->GetValue());

  // Misc. preferences
  prefs->SetPref(PWSprefs::DeleteQuestion, !m_Misc_ConfirmDelete);
  prefs->SetPref(PWSprefs::EscExits, m_Misc_EscExits);
  m_DoubleClickAction = DCAStr2Int(m_Misc_DoubleClickActionCB->GetValue());
  prefs->SetPref(PWSprefs::DoubleClickAction, m_DoubleClickAction);
  m_ShiftDoubleClickAction = DCAStr2Int(m_Misc_ShiftDoubleClickActionCB->GetValue());
  prefs->SetPref(PWSprefs::ShiftDoubleClickAction, m_ShiftDoubleClickAction);
  prefs->SetPref(PWSprefs::MinimizeOnAutotype, m_Misc_AutotypeMinimize);
  prefs->SetPref(PWSprefs::QuerySetDef, m_Misc_QuerySetDefUsername);
  prefs->SetPref(PWSprefs::AltBrowser, tostringx(m_Misc_OtherBrowserLocation));
  prefs->SetPref(PWSprefs::AltBrowserCmdLineParms,
                 tostringx(m_Misc_OtherBrowserLocationparams));

  // Password History preferences
  prefs->SetPref(PWSprefs::SavePasswordHistory, m_PasswordHistory_Save);
  prefs->SetPref(PWSprefs::NumPWHistoryDefault, m_PasswordHistory_NumDefault);
  prefs->SetPref(PWSprefs::DefaultExpiryDays, m_PasswordHistory_DefaultExpiryDays);

  // Security Preferences
  prefs->SetPref(PWSprefs::ClearClipboardOnMinimize   , m_Security_ClearClipboardOnMinimize);
  prefs->SetPref(PWSprefs::ClearClipboardOnExit       , m_Security_ClearClipboardOnExit);
  prefs->SetPref(PWSprefs::DontAskQuestion            , m_Security_ConfirmCopy);
  prefs->SetPref(PWSprefs::CopyPasswordWhenBrowseToURL, m_Security_CopyPswdBrowseURL);
  prefs->SetPref(PWSprefs::DatabaseClear              , m_Security_LockOnMinimize);
  prefs->SetPref(PWSprefs::LockOnWindowLock           , m_Security_LockOnWindowLock);

  m_hashIterValue = MIN_HASH_ITERATIONS;
  if (m_Security_HashIterSlider > 0) {
    const int step = MAX_USABLE_HASH_ITERS/100;
    m_hashIterValue = uint32(m_Security_HashIterSlider*step);
  }

  // System preferences
  prefs->SetPref(PWSprefs::MaxREItems, m_System_MaxREItemsSB->GetValue());
  prefs->SetPref(PWSprefs::UseSystemTray, m_System_UseSystemTrayCB->GetValue());
  m_System_Startup = false; // XXX TBD
  prefs->SetPref(PWSprefs::MaxMRUItems, m_System_MaxMRUItems);
  prefs->SetPref(PWSprefs::MRUOnFileMenu, m_System_MRUOnFileMenu);
  prefs->SetPref(PWSprefs::DefaultOpenRO, m_System_DefaultOpenRO);
  prefs->SetPref(PWSprefs::MultipleInstances, m_System_MultipleInstances);
#if defined(__X__) || defined(__WXGTK__)
  prefs->SetPref(PWSprefs::UsePrimarySelectionForClipboard, m_System_UsePrimarySelection);
  Clipboard::GetInstance()->UsePrimarySelection(m_System_UsePrimarySelection);
  prefs->SetPref(PWSprefs::UseAltAutoType, m_System_UseAltAutoType);
#endif

  // Now do a bit of trickery to find the new preferences to be stored in
  // the database as a string but without updating the actual preferences
  // which needs to be done via a Command so that it can be Undone & Redone

  // Initialise a copy of the DB preferences
  prefs->SetupCopyPrefs();

  // Update them - last parameter of SetPref and Store is: "bUseCopy = true"
  // In PropertyPage alphabetic order
  prefs->SetPref(PWSprefs::SaveImmediately, m_Backup_SaveImmediately, true);
  prefs->SetPref(PWSprefs::ShowPWDefault, m_Display_ShowPasswordInEdit, true);

  prefs->SetPref(PWSprefs::ShowUsernameInTree, m_Display_ShowUsernameInTree, true);
  prefs->SetPref(PWSprefs::ShowPasswordInTree, m_Display_ShowPasswordInTreeCB->GetValue(), true);

  prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, m_Display_TreeDisplayStatusAtOpen, true);
  prefs->SetPref(PWSprefs::ShowNotesDefault, m_Display_ShowNotesInEdit, true);
  prefs->SetPref(PWSprefs::MaintainDateTimeStamps, m_Misc_MaintainDatetimeStamps, true);
  prefs->SetPref(PWSprefs::UseDefaultUser, m_Misc_UseDefUsername, true);
  prefs->SetPref(PWSprefs::DefaultUsername, tostringx(m_Misc_DefaultUsernameTXT->GetValue()), true);

  if (m_Misc_AutotypeString.empty() || m_Misc_AutotypeString == DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString, wxEmptyString, true);
  else
    prefs->SetPref(PWSprefs::DefaultAutotypeString, tostringx(m_Misc_AutotypeString), true);

  prefs->SetPref(PWSprefs::LockDBOnIdleTimeout, m_Security_LockOnIdleTimeout, true);
  prefs->SetPref(PWSprefs::IdleTimeout, m_Security_IdleTimeoutSB->GetValue(), true);
  wxGetApp().ConfigureIdleTimer();
}

void OptionsPropertySheetDlg::OnOk(wxCommandEvent& WXUNUSED(evt))
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

void OptionsPropertySheetDlg::OnBackupB4SaveClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    m_Backup_DefaultPrefixRB->Enable(m_Backup_BackupBeforeSave);
    m_Backup_UserPrefixRB->Enable(m_Backup_BackupBeforeSave);
    m_Backup_UserPrefixTXT->Enable(m_Backup_BackupBeforeSave);
    m_Backup_SuffixCB->Enable(m_Backup_BackupBeforeSave);
    m_Backup_MaxIncrSB->Enable(m_Backup_BackupBeforeSave);
  }
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON4
 */

void OptionsPropertySheetDlg::OnBuPrefix( wxCommandEvent& evt )
{
  evt.Skip();
}

/*!
 * wxEVT_SET_FOCUS event handler for ID_TEXTCTRL9
 */

void OptionsPropertySheetDlg::OnBuPrefixTxtSetFocus(wxFocusEvent& WXUNUSED(evt))
{
  m_Backup_DefaultPrefixRB->SetValue(false);
  m_Backup_UserPrefixRB->SetValue(true);
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX2
 */

void OptionsPropertySheetDlg::OnSuffixCBSet(wxCommandEvent& WXUNUSED(evt))
{
  int suffixIndex = m_Backup_SuffixCB->GetCurrentSelection();
  wxString example = m_Backup_UserPrefixTXT->GetValue();

  if (example.empty())
    example = wxT("pwsafe"); // XXXX get current file's basename!

  m_Backup_MaxIncrSB->Enable(suffixIndex == INC_SFX);
  switch (suffixIndex) {
  case NO_SFX:
    m_Backup_SuffixExampleST->SetLabel(wxEmptyString);
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
  m_Backup_SuffixExampleST->SetLabel(example);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void OptionsPropertySheetDlg::OnBuDirBrowseClick(wxCommandEvent& WXUNUSED(evt))
{
  wxDirDialog dirdlg(this);
  int status = dirdlg.ShowModal();
  if (status == wxID_OK)
    m_Backup_UserDirTXT->SetValue(dirdlg.GetPath());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX13
 */

void OptionsPropertySheetDlg::OnShowUsernameInTreeCB(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    if (!m_Display_ShowUsernameInTree)
      m_Display_ShowPasswordInTreeCB->SetValue(false);
    m_Display_ShowPasswordInTreeCB->Enable(m_Display_ShowUsernameInTree);
  }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX19
 */

void OptionsPropertySheetDlg::OnPreExpiryWarnClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    m_Display_PreExpiryWarnDaysSB->Enable(m_Display_PreExpiryWarn);
  }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX24
 */

void OptionsPropertySheetDlg::OnUseDefaultUserClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    m_Misc_DefaultUsernameTXT->Enable(m_Misc_UseDefUsername);
    m_Misc_DefaultUsernameLBL->Enable(m_Misc_UseDefUsername);
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON8
 */

void OptionsPropertySheetDlg::OnBrowseLocationClick(wxCommandEvent& WXUNUSED(evt))
{
  wxFileDialog fd(this, _("Select a Browser"));
  if (Validate() && TransferDataFromWindow()) {
    fd.SetPath(m_Misc_OtherBrowserLocation);
  }
  if (fd.ShowModal() == wxID_OK) {
    m_Misc_OtherBrowserLocation = fd.GetPath();
    Validate() && TransferDataToWindow();
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PWHISTAPPLY
 */

void OptionsPropertySheetDlg::OnPWHistApply(wxCommandEvent& WXUNUSED(evt))
{
  int applytoprotected = m_PasswordHistory_Apply2ProtectedCB->GetValue();
  int pwhistaction = 0;
  int pwhistnum = m_PasswordHistory_NumDefaultSB->GetValue();
  wxString resultmsg;

  if (m_PasswordHistory_StopRB->GetValue()) {
    // Reset entries to HISTORY OFF
    pwhistaction = (applytoprotected) ? PWHist::STOP_INCL_PROT : PWHist::STOP_EXCL_PROT;
    resultmsg = _("Number of entries that had their settings changed to not save password history was: %d");

  } else if (m_PasswordHistory_StartRB->GetValue()) {
    // Reset entries to HISTORY ON
    pwhistaction = (applytoprotected) ? PWHist::START_INCL_PROT : PWHist::START_EXCL_PROT;
    resultmsg = _("Number of entries that had their settings changed to save password history was: %d");

  } else if (m_PasswordHistory_SetMaxRB->GetValue()) {
    // Don't reset history setting, but set history number
    pwhistaction = (applytoprotected) ? PWHist::SETMAX_INCL_PROT : PWHist::SETMAX_EXCL_PROT;
    resultmsg = _("Number of entries that had their 'maximum saved passwords' changed to the new default was %d");

  } else if (m_PasswordHistory_ClearRB->GetValue()) {
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

void OptionsPropertySheetDlg::OnUpdateUI(wxUpdateUIEvent& evt)
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
      m_Backup_UserDirTXT->Enable(m_Backup_UserDirRB->GetValue());
      m_Backup_DirBN->Enable(m_Backup_UserDirRB->GetValue());
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
      evt.Enable(!dbIsReadOnly && m_PasswordHistory_SaveCB->GetValue());
      break;
    case ID_STATICTEXT_8:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_STATICBOX_1:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_PWHISTNOCHANGE:
      m_PasswordHistory_Apply2ProtectedCB->Enable(!dbIsReadOnly && !m_PasswordHistory_NoChangeRB->GetValue());
      m_PasswordHistory_ApplyBN->Enable(!dbIsReadOnly && !m_PasswordHistory_NoChangeRB->GetValue());
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
    case ID_CHECKBOX35:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_CHECKBOX29:
      evt.Enable(!dbIsReadOnly);
      break;
    case ID_SPINCTRL12:
      evt.Enable(!dbIsReadOnly && m_Security_LockOnIdleTimeoutCB->GetValue());
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
    case ID_STATICTEXT_7:
    case ID_STATICTEXT_10:
      evt.Enable(m_System_UseSystemTrayCB->GetValue());
      break;
    default:
      break;
  }
}

void OptionsPropertySheetDlg::OnPageChanging(wxBookCtrlEvent& evt)
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
