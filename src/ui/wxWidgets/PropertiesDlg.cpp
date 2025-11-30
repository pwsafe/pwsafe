/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
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

PropertiesModel::PropertiesModel(PropertiesDlg &view, PWScore &core) : m_View(view), m_Core(core)
{
}

/// Retrieves the stored properties from the database,
/// stores them internally, and uses them to initialize the view.
void PropertiesModel::Init()
{
  /////////////////////////////////////////////////////////////////////////////
  // Property: Database file name

  m_PropertiesDb.database = m_Core.GetCurFile();
  m_View.SetDatabase(towxstring(m_PropertiesDb.database));

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database format

  auto dbFormat = wxString::Format(wxT("%d.%02d"),
    m_Core.GetHeader().m_nCurrentMajorVersion,
    m_Core.GetHeader().m_nCurrentMinorVersion);

  m_View.SetDatabaseformat(dbFormat);
  m_PropertiesDb.databaseformat = tostringx(dbFormat);

  /////////////////////////////////////////////////////////////////////////////
  // Property: Number of groups

  std::vector<stringT> allGroups;
  m_Core.GetAllGroups(allGroups);

  auto numgroups = wxString::Format(_("%d"),
    static_cast<int>(allGroups.size()));
  m_PropertiesDb.numgroups = tostringx(numgroups);

  auto numemptygroups = wxString::Format(_("%d"),
    static_cast<int>(m_Core.GetEmptyGroups().size()));
  m_PropertiesDb.numemptygroups = tostringx(numemptygroups);

  m_View.SetNumOfGroups(wxString::Format(_("%ls (%ls empty)"),
    numgroups, numemptygroups));

  /////////////////////////////////////////////////////////////////////////////
  // Property: Number of entries

  auto numEntries = wxString::Format(wxT("%d"),
    static_cast<int>(m_Core.GetNumEntries()));

  m_View.SetNumOfEntries(numEntries);
  m_PropertiesDb.numentries = tostringx(numEntries);

  /////////////////////////////////////////////////////////////////////////////
  // Property: Number of attachments

  auto numAttachements = wxString::Format(wxT("%i"),
    static_cast<int>(m_Core.GetNumAtts()));
  m_View.SetNumOfAttachments(numAttachements);
  m_PropertiesDb.numattachments = tostringx(numAttachements);

  /////////////////////////////////////////////////////////////////////////////
  // Property: Date when database was last saved

  time_t twls = m_Core.GetHeader().m_whenlastsaved;

  if (twls == 0) {
    m_PropertiesDb.whenlastsaved = tostringx(_("Unknown"));
  } else {
    m_PropertiesDb.whenlastsaved = PWSUtil::ConvertToDateTimeString(
      twls, PWSUtil::TMC_LOCALE_SIMPLIFIED);
  }
  m_View.SetWhenLastSaved(towxstring(m_PropertiesDb.whenlastsaved));

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database last saved by user on host

  if (m_Core.GetHeader().m_lastsavedby.empty() &&
      m_Core.GetHeader().m_lastsavedon.empty()) {
    m_View.SetWhoLastSaved(_("Unknown"));
    m_PropertiesDb.wholastsaved = tostringx(m_View.GetWhoLastSaved());
  }
  else {
    wxString user = m_Core.GetHeader().m_lastsavedby.empty() ?
      _T("?") : m_Core.GetHeader().m_lastsavedby.c_str();

    wxString host = m_Core.GetHeader().m_lastsavedon.empty() ?
      _T("?") : m_Core.GetHeader().m_lastsavedon.c_str();

    auto whoLastsaved = wxString::Format(_("%ls on %ls"), user.c_str(), host.c_str());
    m_View.SetWhoLastSaved(whoLastsaved);
    m_PropertiesDb.wholastsaved = tostringx(whoLastsaved);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database saved by application

  wxString wls = m_Core.GetHeader().m_whatlastsaved.c_str();

  m_View.SetWhatLastSaved(wls.empty() ? _("Unknown") : wls);
  m_PropertiesDb.whatlastsaved = tostringx(m_View.GetWhatLastSaved());

  /////////////////////////////////////////////////////////////////////////////
  // Property: Date when master password last changed

  time_t twplc = m_Core.GetHeader().m_whenpwdlastchanged;

  if (twplc == 0) {
    m_PropertiesDb.whenpwdlastchanged = tostringx(_("Unknown"));
  } else {
    m_PropertiesDb.whenpwdlastchanged = PWSUtil::ConvertToDateTimeString(
      twplc, PWSUtil::TMC_LOCALE_SIMPLIFIED).c_str();
  }
  m_View.SetWhenPwdLastChanged(towxstring(m_PropertiesDb.whenpwdlastchanged));

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database Unique ID

  pws_os::CUUID file_uuid = m_Core.GetFileUUID();

  if (file_uuid == pws_os::CUUID::NullUUID()) {
    m_PropertiesDb.file_uuid = tostringx(wxT("N/A"));
  }
  else {
    ostringstreamT os;
    pws_os::CUUID huuid(*file_uuid.GetARep(), true); // true for canonical format
    os << huuid;
    m_PropertiesDb.file_uuid = tostringx(os.str().c_str());
  }
  m_View.SetFileUuid(towxstring(m_PropertiesDb.file_uuid));

  /////////////////////////////////////////////////////////////////////////////
  // Property: Unknown Fields

  int num = m_Core.GetNumRecordsWithUnknownFields();

  if (num != 0 || m_Core.HasHeaderUnknownFields()) {
    const wxString cs_HdrYesNo = m_Core.HasHeaderUnknownFields() ? _("Yes") : _("No");

    auto unknownFields = wxString::Format(_("In Headers(%ls)/In Entries("), cs_HdrYesNo.c_str());

    if (num == 0) {
      unknownFields += _("No)");
    }
    else {
      wls = wxString::Format(wxT("%d)"), num);
      unknownFields += wls;
    }
    m_View.SetUnknownFields(unknownFields);
    m_PropertiesDb.unknownfields = tostringx(unknownFields);
  }
  else {
    m_View.SetUnknownFields(_("None"));
    m_PropertiesDb.unknownfields = tostringx(m_View.GetUnknownFields());
  }

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database Name

  m_PropertiesDb.db_name = m_Core.GetHeader().m_DB_Name;
  m_View.SetDatabaseName(towxstring(m_PropertiesDb.db_name));

  /////////////////////////////////////////////////////////////////////////////
  // Property: Database Description

  m_PropertiesDb.db_description = m_Core.GetHeader().m_DB_Description;
  m_View.SetDatabaseDescription(towxstring(m_PropertiesDb.db_description));

  /////////////////////////////////////////////////////////////////////////////
  // All user operations are performed on the copy of the properties

  m_PropertiesNew = m_PropertiesDb;
}

/// Instructs the application to save the database properties
/// if there are any changes.
void PropertiesModel::Save()
{
  if (HasChanges()) {

    auto multiCommands = MultiCommands::Create(&m_Core);

    if (HasDatabaseNameChanged()) {

      multiCommands->Add(
        ChangeDBHeaderCommand::Create(
          &m_Core, GetDatabaseName(), PWSfile::HDR_DBNAME
        )
      );
    }

    if (HasDatabaseDescriptionChanged()) {

      multiCommands->Add(
        ChangeDBHeaderCommand::Create(
          &m_Core, GetDatabaseDescription(), PWSfile::HDR_DBDESC
        )
      );
    }

    if (!multiCommands->IsEmpty()) {
      m_Core.Execute(multiCommands);
    }
    else {
      delete multiCommands;
    }
  }
}

/*!
 * PropertiesDlg type definition
 */

IMPLEMENT_CLASS( PropertiesDlg, wxDialog )

/*!
 * PropertiesDlg event table definition
 */

BEGIN_EVENT_TABLE( PropertiesDlg, wxDialog )

////@begin PropertiesDlg event table entries
  EVT_BUTTON( wxID_CLOSE, PropertiesDlg::OnCloseClick )
  EVT_BUTTON( wxID_SAVE,  PropertiesDlg::OnSaveClick  )
////@end PropertiesDlg event table entries

END_EVENT_TABLE()

/*!
 * PropertiesDlg constructors
 */

PropertiesDlg::PropertiesDlg(wxWindow *parent, PWScore &core,
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

PropertiesDlg* PropertiesDlg::Create(wxWindow *parent, PWScore &core,
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
  m_Model = new PropertiesModel(*this, m_core);
  m_Model->Init();
////@end PropertiesDlg member initialisation
}

/*!
 * Control creation for PropertiesDlg
 */

void PropertiesDlg::CreateControls()
{
////@begin PropertiesDlg content construction
  auto *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  auto flexGridSizer = new wxFlexGridSizer(2 /*cols*/, 0 /*vgap*/, 0 /*hgap*/);
  flexGridSizer->AddGrowableCol(1);   // For second column with database properties
  flexGridSizer->AddGrowableRow(12);  // For multiline description field
  mainSizer->Add(flexGridSizer, 1, wxALL|wxEXPAND, 12);

  auto itemStaticText5 = new wxStaticText( this, wxID_STATIC, _("Password Database:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto dbPathText = new wxStaticText( this, wxID_DATABASE, _T("Static text"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE );
  flexGridSizer->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(dbPathText,      1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText6 = new wxStaticText( this, wxID_STATIC, _("Database Format:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto dbFormatText = new wxStaticText( this, wxID_DATABASEFORMAT, wxT("9.99"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(dbFormatText,    1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText7 = new wxStaticText( this, wxID_STATIC, _("Number of Groups:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto numGroupsText = new wxStaticText( this, wxID_NUMGROUPS, wxT("999"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(numGroupsText  , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText8 = new wxStaticText( this, wxID_STATIC, _("Number of Entries:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto entriesText = new wxStaticText( this, wxID_NUMENTRIES, wxT("999"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(entriesText    , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText14 = new wxStaticText( this, wxID_STATIC, _("Number of Attachments:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto attachmentsText = new wxStaticText( this, wxID_NUMATTACHMENTS, wxT("999"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(attachmentsText , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText9 = new wxStaticText( this, wxID_STATIC, _("Last saved by:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastSavedUserText = new wxStaticText( this, wxID_WHOLASTSAVED, wxT("user on host"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText9  , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastSavedUserText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText10 = new wxStaticText( this, wxID_STATIC, _("Last saved on:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastSavedDateText = new wxStaticText( this, wxID_WHENLASTSAVED, wxT("dd.mm.yyyy"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText10 , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastSavedDateText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText11 = new wxStaticText( this, wxID_STATIC, _("Using application:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastSavedAppText = new wxStaticText( this, wxID_WHATLASTSAVED, wxT("application & version"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastSavedAppText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText15 = new wxStaticText( this, wxID_STATIC, _("Master Password last set on:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto lastChangedPwdDateText = new wxStaticText( this, wxID_PWDLASTCHANGED, wxT("dd.mm.yyyy"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText15      , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(lastChangedPwdDateText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText12 = new wxStaticText( this, wxID_STATIC, _("Database Unique ID:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto uuidText = new wxStaticText( this, wxID_FILEUUID,
                                    wxT("12345678-90AB-CDEF-1234-567890ABCDEF"), // need to use different digits/letters to correctly calculate size because of kerning
                                    wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(uuidText        , 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText13 = new wxStaticText( this, wxID_STATIC, _("Unknown fields:"), wxDefaultPosition, wxDefaultSize, 0 );
  auto unknownFieldsText = new wxStaticText( this, wxID_UNKNOWFIELDS, wxT("x"), wxDefaultPosition, wxDefaultSize, 0 );
  flexGridSizer->Add(itemStaticText13 , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(unknownFieldsText, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  auto itemStaticText16 = new wxStaticText( this, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_dbNameTextCtrl = new wxTextCtrl( this, wxID_DBNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  m_dbNameTextCtrl->SetBackgroundColour(GetBackgroundColour());
  if (m_core.IsReadOnly()) {
    m_dbNameTextCtrl->SetToolTip(_("The database name (not file name)"));
  }
  else {
    m_dbNameTextCtrl->SetToolTip(_("Double click to edit the database name (which is not the file name)"));
    m_dbNameTextCtrl->Bind(wxEVT_LEFT_DCLICK, &PropertiesDlg::OnDoubleClickNameTextCtrl, this);
  }
  flexGridSizer->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL        , 5);
  flexGridSizer->Add(m_dbNameTextCtrl, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5);

  auto itemStaticText17 = new wxStaticText( this, wxID_STATIC, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
  m_dbDescriptionTextCtrl = new wxTextCtrl( this, wxID_DBDESCRIPTION, wxEmptyString, wxDefaultPosition, wxSize(-1, 100), wxTE_READONLY|wxTE_MULTILINE );
  m_dbDescriptionTextCtrl->SetBackgroundColour(GetBackgroundColour());
  if (m_core.IsReadOnly()) {
    m_dbDescriptionTextCtrl->SetToolTip(_("The database description"));
  }
  else {
    m_dbDescriptionTextCtrl->SetToolTip(_("Double click to edit the database description"));
    m_dbDescriptionTextCtrl->Bind(wxEVT_LEFT_DCLICK, &PropertiesDlg::OnDoubleClickDescriptionTextCtrl, this);
  }
  flexGridSizer->Add(itemStaticText17       , 0, wxALIGN_RIGHT|wxALL        , 5);
  flexGridSizer->Add(m_dbDescriptionTextCtrl, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);


  auto buttonsSizer = new wxStdDialogButtonSizer;
  mainSizer->Add(buttonsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_saveButton = new wxButton(this, wxID_SAVE);
  m_saveButton->Disable();
  buttonsSizer->AddButton(m_saveButton);

  m_closeButton = new wxButton(this, wxID_CLOSE);
  m_closeButton->SetDefault();
  m_closeButton->SetFocus();

  buttonsSizer->AddButton(m_closeButton);
  buttonsSizer->Realize();

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
  m_dbNameTextCtrl->SetValidator( wxGenericValidator(& m_DbName) );
  m_dbDescriptionTextCtrl->SetValidator( wxGenericValidator(& m_DbDescription) );
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
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void PropertiesDlg::OnCloseClick( wxCommandEvent& WXUNUSED(evt) )
{
  if (m_Model->HasChanges()) {
    wxGenericMessageDialog dialog(this,
      _("One or more values have been changed.\nDo you want to discard the changes?"), _("Warning"),
      wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_EXCLAMATION
    );
    dialog.SetOKLabel(_("Discard"));

    if (dialog.ShowModal() == wxID_CANCEL) {
      return;
    }
  }
  EndModal(wxID_CLOSE);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_SAVE
 */

void PropertiesDlg::OnSaveClick( wxCommandEvent& WXUNUSED(evt) )
{
  m_Model->Save();
  EndModal(wxID_SAVE);
}

/*!
 * wxEVT_LEFT_DCLICK event handler for wxID_DBNAME
 */

void PropertiesDlg::OnDoubleClickNameTextCtrl(wxMouseEvent& WXUNUSED(event))
{
  m_dbNameTextCtrl->SetEditable(true);
  m_dbNameTextCtrl->SelectNone();
  m_dbNameTextCtrl->SetToolTip(_("Edit the database name (not file name)"));
  m_dbNameTextCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
  m_dbNameTextCtrl->Unbind(wxEVT_LEFT_DCLICK, &PropertiesDlg::OnDoubleClickNameTextCtrl, this);
  m_dbNameTextCtrl->Bind(wxEVT_TEXT, &PropertiesDlg::OnNameOrDescriptionChanged, this);
}

/*!
 * wxEVT_LEFT_DCLICK event handler for wxID_DBDESCRIPTION
 */

void PropertiesDlg::OnDoubleClickDescriptionTextCtrl(wxMouseEvent& WXUNUSED(event))
{
  m_dbDescriptionTextCtrl->SetEditable(true);
  m_dbDescriptionTextCtrl->SelectNone();
  m_dbDescriptionTextCtrl->SetToolTip(_("Edit the database description"));
  m_dbDescriptionTextCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
  m_dbDescriptionTextCtrl->Unbind(wxEVT_LEFT_DCLICK, &PropertiesDlg::OnDoubleClickDescriptionTextCtrl, this);
  m_dbDescriptionTextCtrl->Bind(wxEVT_TEXT, &PropertiesDlg::OnNameOrDescriptionChanged, this);
}

/*!
 * wxEVT_TEXT event handler for wxID_CHANGE_NAME and wxID_CHANGE_DESCRIPTION
 */

void PropertiesDlg::OnNameOrDescriptionChanged(wxCommandEvent& evt)
{
  if (evt.GetId() == wxID_DBNAME) {
    m_Model->SetDatabaseName(
      m_dbNameTextCtrl->GetValue()
    );
  }
  else if (evt.GetId() == wxID_DBDESCRIPTION) {
    m_Model->SetDatabaseDescription(
      m_dbDescriptionTextCtrl->GetValue()
    );
  }

  m_saveButton->Enable(
    m_Model->HasChanges()
  );
}
