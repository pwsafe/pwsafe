/*
 * Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file properties.cpp
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

#include <vector>
#include "properties.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * CProperties type definition
 */

IMPLEMENT_CLASS( CProperties, wxDialog )

/*!
 * CProperties event table definition
 */

BEGIN_EVENT_TABLE( CProperties, wxDialog )

////@begin CProperties event table entries
  EVT_BUTTON( wxID_OK, CProperties::OnOkClick )

////@end CProperties event table entries

END_EVENT_TABLE()

/*!
 * CProperties constructors
 */

CProperties::CProperties(wxWindow* parent, const PWScore &core,
                         wxWindowID id, const wxString& caption,
                         const wxPoint& pos, const wxSize& size, long style)
  : m_core(core)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * CProperties creator
 */

bool CProperties::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProperties creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CProperties creation
  return true;
}

/*!
 * CProperties destructor
 */

CProperties::~CProperties()
{
////@begin CProperties destruction
////@end CProperties destruction
}

/*!
 * Member initialisation
 */

void CProperties::Init()
{
////@begin CProperties member initialisation
////@end CProperties member initialisation
  m_database = m_core.GetCurFile().c_str();
  m_databaseformat = wxString::Format(L"%d.%02d",
                                      m_core.GetHeader().m_nCurrentMajorVersion,
                                      m_core.GetHeader().m_nCurrentMinorVersion);
  std::vector<stringT> aryGroups;
  m_core.GetAllGroups(aryGroups);
  auto nEmptyGroups = m_core.GetEmptyGroups().size();
  m_numgroups << aryGroups.size()
              << L" (" << nEmptyGroups << _(" empty)");

  m_numentries << m_core.GetNumEntries();

  time_t twls = m_core.GetHeader().m_whenlastsaved;
  if (twls == 0) {
    m_whenlastsaved = _("Unknown");
  } else {
    m_whenlastsaved = PWSUtil::ConvertToDateTimeString(twls,
                                                       PWSUtil::TMC_EXPORT_IMPORT).c_str();
  }

  if (m_core.GetHeader().m_lastsavedby.empty() &&
      m_core.GetHeader().m_lastsavedon.empty()) {
    m_wholastsaved = _("Unknown");
  } else {
    wxString user = m_core.GetHeader().m_lastsavedby.empty() ?
      L"?" : m_core.GetHeader().m_lastsavedby.c_str();
    wxString host = m_core.GetHeader().m_lastsavedon.empty() ?
      L"?" : m_core.GetHeader().m_lastsavedon.c_str();
    m_wholastsaved = wxString::Format(_("%ls on %ls"), user.c_str(), host.c_str());
  }

  wxString wls = m_core.GetHeader().m_whatlastsaved.c_str();
  if (wls.empty()) {
    m_whatlastsaved = _("Unknown");
  } else
    m_whatlastsaved = wls;

  pws_os::CUUID file_uuid = m_core.GetFileUUID();
  if (file_uuid == pws_os::CUUID::NullUUID())
    m_file_uuid = _("N/A");
  else {
    ostringstreamT os;
    pws_os::CUUID huuid(*file_uuid.GetARep(),
      true); // true for canonical format
    os << huuid;
    m_file_uuid = os.str().c_str();
  }

  int num = m_core.GetNumRecordsWithUnknownFields();
  if (num != 0 || m_core.HasHeaderUnknownFields()) {
    const wxString cs_HdrYesNo = m_core.HasHeaderUnknownFields() ? _("Yes") : _("No");

    m_unknownfields = wxString::Format(_("In Headers(%ls)/In Entries("),
                                       cs_HdrYesNo.c_str());
    if (num == 0)
      m_unknownfields += _("No)");
    else {
      wls = wxString::Format(L"%d", num);
      m_unknownfields += wls;
    }
  } else {
    m_unknownfields = _("None");
  }
}

/*!
 * Control creation for CProperties
 */

void CProperties::CreateControls()
{
////@begin CProperties content construction
  CProperties* currDialog = this;

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  currDialog->SetSizer(mainSizer);

  wxStaticText* dbPathText = new wxStaticText( currDialog, wxID_DATABASE, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  mainSizer->Add(dbPathText, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(horizSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* staticVertSizer = new wxBoxSizer(wxVERTICAL);
  horizSizer->Add(staticVertSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( currDialog, wxID_STATIC, _("Database format:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( currDialog, wxID_STATIC, _("Number of Groups:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( currDialog, wxID_STATIC, _("Number of Entries:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( currDialog, wxID_STATIC, _("Last saved by:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText10 = new wxStaticText( currDialog, wxID_STATIC, _("Last saved on:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText11 = new wxStaticText( currDialog, wxID_STATIC, _("Using application:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText12 = new wxStaticText( currDialog, wxID_STATIC, _("Database Unique ID:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText13 = new wxStaticText( currDialog, wxID_STATIC, _("Unknown fields:"), wxDefaultPosition, wxDefaultSize, 0 );
  staticVertSizer->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* dataVertSizer = new wxBoxSizer(wxVERTICAL);
  horizSizer->Add(dataVertSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* dbFormatText = new wxStaticText( currDialog, wxID_DATABASEFORMAT, L"9.99", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(dbFormatText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* numGroupsText = new wxStaticText( currDialog, wxID_NUMGROUPS, L"999", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(numGroupsText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* entriesText = new wxStaticText( currDialog, wxID_NUMENTRIES, L"999", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(entriesText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* lastSavedUserText = new wxStaticText( currDialog, wxID_WHOLASTSAVED, L"user on host", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(lastSavedUserText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* lastSavedDateText = new wxStaticText( currDialog, wxID_WHENLASTSAVED, L"dd.mm.yyyy", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(lastSavedDateText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* lastSavedAppText = new wxStaticText( currDialog, wxID_WHATLASTSAVED, L"application & version", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(lastSavedAppText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* uuidText = new wxStaticText( currDialog, wxID_FILEUUID,
                                            L"12345678-90AB-CDEF-1234-567890ABCDEF", // need to use different digits/letters to correctly calculate size because of kerning
                                            wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(uuidText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* unknownFieldsText = new wxStaticText( currDialog, wxID_UNKNOWFIELDS, L"x", wxDefaultPosition, wxDefaultSize, 0 );
  dataVertSizer->Add(unknownFieldsText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStdDialogButtonSizer* buttonsSizer = new wxStdDialogButtonSizer;

  mainSizer->Add(buttonsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* okButton = new wxButton( currDialog, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  okButton->SetDefault();
  buttonsSizer->AddButton(okButton);
  buttonsSizer->Realize();

  // Set validators
  dbPathText->SetValidator( wxGenericValidator(& m_database) );
  dbFormatText->SetValidator( wxGenericValidator(& m_databaseformat) );
  numGroupsText->SetValidator( wxGenericValidator(& m_numgroups) );
  entriesText->SetValidator( wxGenericValidator(& m_numentries) );
  lastSavedUserText->SetValidator( wxGenericValidator(& m_wholastsaved) );
  lastSavedDateText->SetValidator( wxGenericValidator(& m_whenlastsaved) );
  lastSavedAppText->SetValidator( wxGenericValidator(& m_whatlastsaved) );
  uuidText->SetValidator( wxGenericValidator(& m_file_uuid) );
  unknownFieldsText->SetValidator( wxGenericValidator(& m_unknownfields) );

////@end CProperties content construction
}

/*!
 * Should we show tooltips?
 */

bool CProperties::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CProperties::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin CProperties bitmap retrieval
  return wxNullBitmap;
////@end CProperties bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CProperties::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin CProperties icon retrieval
  return wxNullIcon;
////@end CProperties icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CProperties::OnOkClick( wxCommandEvent& /* evt */ )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CProperties.
  // Before editing this code, remove the block markers.
  EndModal(wxID_OK);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CProperties.
}
