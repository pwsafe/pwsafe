/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PropertiesDlg.cpp
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

////@begin includes
#include <wx/grid.h>
#include <wx/textdlg.h>
////@end includes

#include "PropertiesDlg.h"
#include "wxUtilities.h"

#include <vector>

////@begin XPM images
////@end XPM images

/*!
 * PropertiesDlg type definition
 */

IMPLEMENT_CLASS( PropertiesDlg, wxDialog )

/*!
 * PropertiesDlg event table definition
 */

BEGIN_EVENT_TABLE( PropertiesDlg, wxDialog )

////@begin PropertiesDlg event table entries
  EVT_BUTTON( wxID_CLOSE,                  PropertiesDlg::OnCloseClick )
  EVT_BUTTON( wxID_CHANGE_NAME,         PropertiesDlg::OnEditName )
  EVT_BUTTON( wxID_CHANGE_DESCRIPTION,  PropertiesDlg::OnEditDescription )

////@end PropertiesDlg event table entries

END_EVENT_TABLE()

/*!
 * PropertiesDlg constructors
 */

PropertiesDlg::PropertiesDlg(wxWindow *parent, const PWScore &core,
                         wxWindowID id, const wxString& caption,
                         const wxPoint& pos, const wxSize& size, long style)
  : m_core(core)
{
  wxASSERT(!parent || parent->IsTopLevel());

  Init();

////@begin PropertiesDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end PropertiesDlg creation
}

PropertiesDlg* PropertiesDlg::Create(wxWindow *parent, const PWScore &core,
                         wxWindowID id, const wxString& caption,
                         const wxPoint& pos, const wxSize& size, long style)
{
  return new PropertiesDlg(parent, core, id, caption, pos, size, style);
}

/*!
 * Member initialisation
 */

void PropertiesDlg::Init()
{
////@begin PropertiesDlg member initialisation
////@end PropertiesDlg member initialisation

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database file name
  /////////////////////////////////////////////////////////////////////////////

  m_database = m_core.GetCurFile().c_str();


  /////////////////////////////////////////////////////////////////////////////
  // Property: Database format
  /////////////////////////////////////////////////////////////////////////////

  m_databaseformat = wxString::Format(_T("%d.%02d"),
                                      m_core.GetHeader().m_nCurrentMajorVersion,
                                      m_core.GetHeader().m_nCurrentMinorVersion);


  /////////////////////////////////////////////////////////////////////////////
  // Property: Number of groups
  /////////////////////////////////////////////////////////////////////////////

  std::vector<stringT> aryGroups;
  m_core.GetAllGroups(aryGroups);

  auto nEmptyGroups = m_core.GetEmptyGroups().size();

  m_numgroups << aryGroups.size()
              << wxT(" (") << nEmptyGroups << _(" empty)");


  /////////////////////////////////////////////////////////////////////////////
  // Property: Number of entries
  /////////////////////////////////////////////////////////////////////////////

  m_numentries << m_core.GetNumEntries();


  /////////////////////////////////////////////////////////////////////////////
  // Property: Number of attachments
  /////////////////////////////////////////////////////////////////////////////

  m_numattachments << m_core.GetNumAtts();


  /////////////////////////////////////////////////////////////////////////////
  // Property: Date when database was last saved
  /////////////////////////////////////////////////////////////////////////////

  time_t twls = m_core.GetHeader().m_whenlastsaved;

  if (twls == 0) {
    m_whenlastsaved = _("Unknown");
  } else {
    m_whenlastsaved = PWSUtil::ConvertToDateTimeString(twls,
                                                       PWSUtil::TMC_EXPORT_IMPORT).c_str();
  }


  /////////////////////////////////////////////////////////////////////////////
  // Property: Database last saved by user on host
  /////////////////////////////////////////////////////////////////////////////

  if (m_core.GetHeader().m_lastsavedby.empty() &&
      m_core.GetHeader().m_lastsavedon.empty()) {
    m_wholastsaved = _("Unknown");
  }
  else {
    wxString user = m_core.GetHeader().m_lastsavedby.empty() ?
      _T("?") : m_core.GetHeader().m_lastsavedby.c_str();

    wxString host = m_core.GetHeader().m_lastsavedon.empty() ?
      _T("?") : m_core.GetHeader().m_lastsavedon.c_str();

    m_wholastsaved = wxString::Format(_("%ls on %ls"), user.c_str(), host.c_str());
  }


  /////////////////////////////////////////////////////////////////////////////
  // Property: Database saved by application
  /////////////////////////////////////////////////////////////////////////////

  wxString wls = m_core.GetHeader().m_whatlastsaved.c_str();

  m_whatlastsaved = wls.empty() ? _("Unknown") : wls;


  /////////////////////////////////////////////////////////////////////////////
  // Property: Date when master password last changed
  /////////////////////////////////////////////////////////////////////////////

  time_t twplc = m_core.GetHeader().m_whenpwdlastchanged;

  if (twplc == 0) {
    m_whenpwdlastchanged = _("Unknown");
  } else {
    m_whenpwdlastchanged = PWSUtil::ConvertToDateTimeString(twplc, PWSUtil::TMC_EXPORT_IMPORT).c_str();
  }


  /////////////////////////////////////////////////////////////////////////////
  // Property: Database Unique ID
  /////////////////////////////////////////////////////////////////////////////

  pws_os::CUUID file_uuid = m_core.GetFileUUID();

  if (file_uuid == pws_os::CUUID::NullUUID()) {
    m_file_uuid = _T("N/A");
  }
  else {
    ostringstreamT os;
    pws_os::CUUID huuid(*file_uuid.GetARep(), true); // true for canonical format
    os << huuid;
    m_file_uuid = os.str().c_str();
  }


  /////////////////////////////////////////////////////////////////////////////
  // Property: Unknown Fields
  /////////////////////////////////////////////////////////////////////////////

  int num = m_core.GetNumRecordsWithUnknownFields();

  if (num != 0 || m_core.HasHeaderUnknownFields()) {
    const wxString cs_HdrYesNo = m_core.HasHeaderUnknownFields() ? _("Yes") : _("No");

    m_unknownfields = wxString::Format(_("In Headers(%ls)/In Entries("), cs_HdrYesNo.c_str());

    if (num == 0) {
      m_unknownfields += _("No)");
    }
    else {
      wls = wxString::Format(wxT("%d)"), num);
      m_unknownfields += wls;
    }
  }
  else {
    m_unknownfields = _("None");
  }


  /////////////////////////////////////////////////////////////////////////////
  // Property: Database Name
  /////////////////////////////////////////////////////////////////////////////

  wxString dbName = m_core.GetHeader().m_DB_Name.c_str();

  m_DbName = dbName.empty() ? _("N/A") : dbName;
  m_NewDbName = m_core.GetHeader().m_DB_Name;


  /////////////////////////////////////////////////////////////////////////////
  // Property: Database Description
  /////////////////////////////////////////////////////////////////////////////

  wxString dbDescription = m_core.GetHeader().m_DB_Description.c_str();

  m_DbDescription = dbDescription.empty() ? _("N/A") : dbDescription;
  m_NewDbDescription = m_core.GetHeader().m_DB_Description;
}

/*!
 * Control creation for PropertiesDlg
 */

void PropertiesDlg::CreateControls()
{
////@begin PropertiesDlg content construction
  auto mainSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(mainSizer);

  auto dbPathText = new wxStaticText( this, wxID_DATABASE, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  mainSizer->Add(dbPathText, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 10);

  auto flexGridSizer = new wxFlexGridSizer(3 /*cols*/, 0 /*vgap*/, 0 /*hgap*/);
  flexGridSizer->AddGrowableCol(1);
  mainSizer->Add(flexGridSizer, 1, wxALL|wxEXPAND, 5);

  auto itemStaticText6 = new wxStaticText( this, wxID_STATIC, _("Database format:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto dbFormatText = new wxStaticText( this, wxID_DATABASEFORMAT, wxT("9.99"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(dbFormatText,    1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText7 = new wxStaticText( this, wxID_STATIC, _("Number of Groups:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto numGroupsText = new wxStaticText( this, wxID_NUMGROUPS, wxT("999"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(numGroupsText  , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText8 = new wxStaticText( this, wxID_STATIC, _("Number of Entries:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto entriesText = new wxStaticText( this, wxID_NUMENTRIES, wxT("999"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(entriesText    , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText14 = new wxStaticText( this, wxID_STATIC, _("Number of Attachments:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto attachmentsText = new wxStaticText( this, wxID_NUMATTACHMENTS, wxT("999"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(attachmentsText , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText9 = new wxStaticText( this, wxID_STATIC, _("Last saved by:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastSavedUserText = new wxStaticText( this, wxID_WHOLASTSAVED, wxT("user on host"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText9  , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastSavedUserText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText10 = new wxStaticText( this, wxID_STATIC, _("Last saved on:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastSavedDateText = new wxStaticText( this, wxID_WHENLASTSAVED, wxT("dd.mm.yyyy"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText10 , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastSavedDateText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText11 = new wxStaticText( this, wxID_STATIC, _("Using application:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastSavedAppText = new wxStaticText( this, wxID_WHATLASTSAVED, wxT("application & version"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastSavedAppText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText15 = new wxStaticText( this, wxID_STATIC, _("Master Password last set on:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastChangedPwdDateText = new wxStaticText( this, wxID_PWDLASTCHANGED, wxT("dd.mm.yyyy"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText15      , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastChangedPwdDateText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText12 = new wxStaticText( this, wxID_STATIC, _("Database Unique ID:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto uuidText = new wxStaticText( this, wxID_FILEUUID,
                                            wxT("12345678-90AB-CDEF-1234-567890ABCDEF"), // need to use different digits/letters to correctly calculate size because of kerning
                                            wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(uuidText        , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText13 = new wxStaticText( this, wxID_STATIC, _("Unknown fields:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto unknownFieldsText = new wxStaticText( this, wxID_UNKNOWFIELDS, wxT("x"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText13 , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(unknownFieldsText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->AddStretchSpacer(); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText16 = new wxStaticText( this, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto dbNameText = new wxStaticText( this, wxID_DBNAME, wxT("database name"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
  auto onEditNameButton = new wxButton( this, wxID_CHANGE_NAME, wxT("..."), wxDefaultPosition, wxSize(35, 25), 0 );
  flexGridSizer->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(dbNameText      , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->Add(onEditNameButton, 0, wxALIGN_LEFT|wxALL         , 5); // Item for 3rd column of wxFlexGridSizer

  auto itemStaticText17 = new wxStaticText( this, wxID_STATIC, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto dbDescriptionText = new wxStaticText( this, wxID_DBDESCRIPTION, wxT("database description\n\n"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
  auto onEditDescriptionButton = new wxButton( this, wxID_CHANGE_DESCRIPTION, wxT("..."), wxDefaultPosition, wxSize(35, 25), 0 );
  flexGridSizer->Add(itemStaticText17       , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(dbDescriptionText      , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  flexGridSizer->Add(onEditDescriptionButton, 0, wxALIGN_LEFT|wxALL         , 5); // Item for 3rd column of wxFlexGridSizer


  auto buttonsSizer = new wxStdDialogButtonSizer;

  mainSizer->Add(buttonsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  auto okButton = new wxButton( this, wxID_CLOSE);
  okButton->SetDefault();
  buttonsSizer->AddButton(okButton);
  buttonsSizer->Realize();

  if (m_core.IsReadOnly()) {
    onEditNameButton->Enable(false);
    onEditDescriptionButton->Enable(false);
  }

  // Set validators
  dbPathText->SetValidator( wxGenericValidator(& m_database) );
  dbFormatText->SetValidator( wxGenericValidator(& m_databaseformat) );
  numGroupsText->SetValidator( wxGenericValidator(& m_numgroups) );
  entriesText->SetValidator( wxGenericValidator(& m_numentries) );
  attachmentsText->SetValidator( wxGenericValidator(& m_numattachments) );
  lastSavedUserText->SetValidator( wxGenericValidator(& m_wholastsaved) );
  lastSavedDateText->SetValidator( wxGenericValidator(& m_whenlastsaved) );
  lastSavedAppText->SetValidator( wxGenericValidator(& m_whatlastsaved) );
  lastChangedPwdDateText->SetValidator( wxGenericValidator(& m_whenpwdlastchanged) );
  uuidText->SetValidator( wxGenericValidator(& m_file_uuid) );
  unknownFieldsText->SetValidator( wxGenericValidator(& m_unknownfields) );
  dbNameText->SetValidator( wxGenericValidator(& m_DbName) );
  dbDescriptionText->SetValidator( wxGenericValidator(& m_DbDescription) );

////@end PropertiesDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool PropertiesDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap PropertiesDlg::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin PropertiesDlg bitmap retrieval
  return wxNullBitmap;
////@end PropertiesDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon PropertiesDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin PropertiesDlg icon retrieval
  return wxNullIcon;
////@end PropertiesDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void PropertiesDlg::OnCloseClick( wxCommandEvent& WXUNUSED(evt) )
{
  EndModal(wxID_CLOSE);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CHANGE_NAME
 */
void PropertiesDlg::OnEditName( wxCommandEvent& WXUNUSED(evt) )
{
  CallAfter(&PropertiesDlg::DoEditName);
}

void PropertiesDlg::DoEditName() {
  auto* inputDialog = new wxTextEntryDialog(this, _("Name:"), _("Name"), m_NewDbName.c_str(), wxOK|wxCANCEL);

  inputDialog->SetSize(550, -1);

  if (inputDialog->ShowModal() == wxID_OK) {
    auto newDbName = inputDialog->GetValue();

    if (newDbName.IsEmpty()) {
      m_DbName  = _("N/A");     // Show 'N/A' on the UI in case of an empty string,
      newDbName = _T("");       // but use the empty string as new DB name.
    }
    else {
      m_DbName = newDbName;
    }

    if (Validate() && TransferDataToWindow()) {
      m_NewDbName = tostringx(newDbName);
    }
  }

  inputDialog->Destroy();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CHANGE_DESCRIPTION
 */
void PropertiesDlg::OnEditDescription( wxCommandEvent& WXUNUSED(evt) )
{
  CallAfter(&PropertiesDlg::DoEditDescription);
}

void PropertiesDlg::DoEditDescription() {
  auto* inputDialog = new wxTextEntryDialog(this, _("Description:"), _("Description"), m_NewDbDescription.c_str(), wxOK|wxCANCEL|wxTE_MULTILINE);

  inputDialog->SetSize(550, 300);

  if (inputDialog->ShowModal() == wxID_OK) {
    auto newDbDescription = inputDialog->GetValue();

    if (newDbDescription.IsEmpty()) {
      m_DbDescription = _("N/A");     // Show 'N/A' on the UI in case of an empty string,
      newDbDescription = _T("");      // but use the empty string as new DB description.
    }
    else {
      m_DbDescription = newDbDescription;
    }

    if (Validate() && TransferDataToWindow()) {
      m_NewDbDescription = tostringx(newDbDescription);
    }
  }

  inputDialog->Destroy();
}
