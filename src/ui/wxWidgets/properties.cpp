/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file properties.cpp
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
////@end includes

#include <vector>
#include "properties.h"

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
  m_databaseformat = wxString::Format("%d.%02d",
                                      m_core.GetHeader().m_nCurrentMajorVersion,
                                      m_core.GetHeader().m_nCurrentMinorVersion);
  std::vector<stringT> aryGroups;
  m_core.GetUniqueGroups(aryGroups);
  m_numgroups = wxString::Format(_T("%d"), aryGroups.size());

  m_numentries = wxString::Format(_T("%d"), m_core.GetNumEntries());

  time_t twls = m_core.GetHeader().m_whenlastsaved;
  if (twls == 0) {
    m_whenlastsaved = _("Unknown");
  } else {
    m_whenlastsaved = PWSUtil::ConvertToDateTimeString(twls,
                                                       TMC_EXPORT_IMPORT).c_str();
  }

  if (m_core.GetHeader().m_lastsavedby.empty() &&
      m_core.GetHeader().m_lastsavedon.empty()) {
    m_wholastsaved = _("Unknown");
  } else {
    wxString user = m_core.GetHeader().m_lastsavedby.empty() ?
      "?" : m_core.GetHeader().m_lastsavedby.c_str();
    wxString host = m_core.GetHeader().m_lastsavedon.empty() ?
      "?" : m_core.GetHeader().m_lastsavedon.c_str();
    m_wholastsaved = wxString::Format(_("%s on %s"), user.c_str(), host.c_str());
  }

  wxString wls = m_core.GetHeader().m_whatlastsaved.c_str();
  if (wls.empty()) {
    m_whatlastsaved = _("Unknown");
  } else
    m_whatlastsaved = wls;

  uuid_array_t file_uuid_array, ref_uuid_array;
  memset(ref_uuid_array, 0x00, sizeof(ref_uuid_array));
  m_core.GetFileUUID(file_uuid_array);

  if (memcmp(file_uuid_array, ref_uuid_array, sizeof(file_uuid_array)) == 0)
    m_file_uuid = _T("N/A");
  else {
    ostringstreamT os;
    CUUIDGen huuid(file_uuid_array, true); // true for canonical format
    os << huuid;
    m_file_uuid = os.str().c_str();
  }

  int num = m_core.GetNumRecordsWithUnknownFields();
  if (num != 0 || m_core.HasHeaderUnknownFields()) {
    const wxString cs_HdrYesNo = m_core.HasHeaderUnknownFields() ? _("Yes") : _("No");

    m_unknownfields = wxString::Format(_("In Headers(%s)/In Entries("),
                                       cs_HdrYesNo.c_str());
    if (num == 0)
      m_unknownfields += _("No)");
    else {
      wls = wxString::Format("%d)", num);
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
  CProperties* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_DATABASE, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Database format:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Number of Groups:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Number of Entries:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Last saved by:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Last saved on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Using application:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Database Unique ID:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("Unknown fields:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer4->Add(itemBoxSizer14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_DATABASEFORMAT, _("x.y"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText16 = new wxStaticText( itemDialog1, wxID_NUMGROUPS, _("N"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText16, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_NUMENTRIES, _("N"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText17, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_WHOLASTSAVED, _("U"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText18, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText( itemDialog1, wxID_WHENLASTSAVED, _("h"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText20 = new wxStaticText( itemDialog1, wxID_WHATLASTSAVED, _("a"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText21 = new wxStaticText( itemDialog1, wxID_FILEUUID,
                                                     _("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"),
                                                     wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText21, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText22 = new wxStaticText( itemDialog1, wxID_UNKNOWFIELDS, _("x"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer23 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer23, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton24 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton24->SetDefault();
  itemStdDialogButtonSizer23->AddButton(itemButton24);

  itemStdDialogButtonSizer23->Realize();

  // Set validators
  itemStaticText3->SetValidator( wxGenericValidator(& m_database) );
  itemStaticText15->SetValidator( wxGenericValidator(& m_databaseformat) );
  itemStaticText16->SetValidator( wxGenericValidator(& m_numgroups) );
  itemStaticText17->SetValidator( wxGenericValidator(& m_numentries) );
  itemStaticText18->SetValidator( wxGenericValidator(& m_wholastsaved) );
  itemStaticText19->SetValidator( wxGenericValidator(& m_whenlastsaved) );
  itemStaticText20->SetValidator( wxGenericValidator(& m_whatlastsaved) );
  itemStaticText21->SetValidator( wxGenericValidator(& m_file_uuid) );
  itemStaticText22->SetValidator( wxGenericValidator(& m_unknownfields) );
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

wxBitmap CProperties::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CProperties bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end CProperties bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CProperties::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CProperties icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CProperties icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CProperties::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CProperties.
  // Before editing this code, remove the block markers.
  EndModal(wxID_OK);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CProperties. 
}

