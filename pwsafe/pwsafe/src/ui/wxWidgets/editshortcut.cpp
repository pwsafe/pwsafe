/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file editshortcut.cpp
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

#include "editshortcut.h"

////@begin XPM images
////@end XPM images


/*!
 * EditShortcut type definition
 */

IMPLEMENT_DYNAMIC_CLASS( EditShortcut, wxDialog )


/*!
 * EditShortcut event table definition
 */

BEGIN_EVENT_TABLE( EditShortcut, wxDialog )

////@begin EditShortcut event table entries
////@end EditShortcut event table entries

END_EVENT_TABLE()


/*!
 * EditShortcut constructors
 */

EditShortcut::EditShortcut()
{
  Init();
}

EditShortcut::EditShortcut( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * EditShortcut creator
 */

bool EditShortcut::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin EditShortcut creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end EditShortcut creation
  return true;
}


/*!
 * EditShortcut destructor
 */

EditShortcut::~EditShortcut()
{
////@begin EditShortcut destruction
////@end EditShortcut destruction
}


/*!
 * Member initialisation
 */

void EditShortcut::Init()
{
////@begin EditShortcut member initialisation
  m_group = NULL;
  m_title = NULL;
  m_user = NULL;
  m_created = NULL;
  m_lastChanged = NULL;
  m_lastAccess = NULL;
  m_lastAny = NULL;
////@end EditShortcut member initialisation
}


/*!
 * Control creation for EditShortcut
 */

void EditShortcut::CreateControls()
{    
////@begin EditShortcut content construction
  EditShortcut* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, ID_SC_DISP, _("Please specify the name and group for this shortcut\nto the base entry "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGridSizer* itemGridSizer4 = new wxGridSizer(3, 2, 0, 0);
  itemBoxSizer2->Add(itemGridSizer4, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Group:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_group = new wxComboCtrl( itemDialog1, ID_SC_GROUP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(m_group, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_title = new wxTextCtrl( itemDialog1, ID_TEXTCTRL16, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(m_title, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_user = new wxTextCtrl( itemDialog1, ID_TEXTCTRL17, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(m_user, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Date/Time Information"));
  wxStaticBoxSizer* itemStaticBoxSizer11 = new wxStaticBoxSizer(itemStaticBoxSizer11Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGridSizer* itemGridSizer12 = new wxGridSizer(4, 2, 0, 0);
  itemStaticBoxSizer11->Add(itemGridSizer12, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("Created on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_created = new wxStaticText( itemDialog1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(m_created, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Target last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_lastChanged = new wxStaticText( itemDialog1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(m_lastChanged, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_STATIC, _("Last accessed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_lastAccess = new wxStaticText( itemDialog1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(m_lastAccess, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText( itemDialog1, wxID_STATIC, _("Any field last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_lastAny = new wxStaticText( itemDialog1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(m_lastAny, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer21 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer21, 0, wxGROW|wxALL, 5);
  wxButton* itemButton22 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton22);

  wxButton* itemButton23 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton23);

  wxButton* itemButton24 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton24);

  itemStdDialogButtonSizer21->Realize();

////@end EditShortcut content construction
}


/*!
 * Should we show tooltips?
 */

bool EditShortcut::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap EditShortcut::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin EditShortcut bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end EditShortcut bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon EditShortcut::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin EditShortcut icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end EditShortcut icon retrieval
}
