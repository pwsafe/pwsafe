/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "./wxutils.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * EditShortcut type definition
 */

IMPLEMENT_CLASS( EditShortcut, wxDialog )

/*!
 * EditShortcut event table definition
 */

BEGIN_EVENT_TABLE( EditShortcut, wxDialog )

////@begin EditShortcut event table entries
  EVT_BUTTON( wxID_OK, EditShortcut::OnOkClick )

////@end EditShortcut event table entries

END_EVENT_TABLE()

/*!
 * EditShortcut constructors
 */

EditShortcut::EditShortcut(wxWindow* parent,
                           PWScore &core, CItemData *item,
                           wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style)
: m_core(core), m_item(item)
{
  ASSERT(m_item != nullptr);
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
  ItemFieldsToDialog();
  return true;
}

void EditShortcut::ItemFieldsToDialog()
{
  // Populate the combo box
  std::vector<stringT> aryGroups;
  m_core.GetAllGroups(aryGroups);
  for (size_t igrp = 0; igrp < aryGroups.size(); igrp++) {
    m_groupCtrl->Append(aryGroups[igrp].c_str());
  }
  // select relevant group
  const StringX group = m_item->GetGroup();
  if (!group.empty())
    for (size_t igrp = 0; igrp < aryGroups.size(); igrp++)
      if (group == aryGroups[igrp].c_str()) {
        m_groupCtrl->SetSelection(reinterpret_cast<int &>(igrp));
        break;
      }

  m_title = m_item->GetTitle().c_str();
  m_user = m_item->GetUser().c_str();
  m_created = m_item->GetCTimeL().c_str();
  m_lastAccess = m_item->GetATimeL().c_str();
  m_lastAny = m_item->GetRMTimeL().c_str();
  const CItemData *base = m_core.GetBaseEntry(m_item);
  if (base != nullptr) {
    m_lastChanged = base->GetRMTimeL().c_str();
  } else {
    m_lastChanged = _("Unknown"); // Internal error
  }
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
  m_groupCtrl = nullptr;
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

  wxArrayString m_groupCtrlStrings;
  m_groupCtrl = new wxComboBox( itemDialog1, ID_SC_GROUP, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_groupCtrlStrings, wxCB_DROPDOWN );
  itemGridSizer4->Add(m_groupCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL16, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemTextCtrl8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL17, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemTextCtrl10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Date/Time Information"));
  wxStaticBoxSizer* itemStaticBoxSizer11 = new wxStaticBoxSizer(itemStaticBoxSizer11Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGridSizer* itemGridSizer12 = new wxGridSizer(4, 2, 0, 0);
  itemStaticBoxSizer11->Add(itemGridSizer12, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("Created on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemDialog1, wxID_STATIC, _("DDD DD MMM YYYY HH:MM:SS XM ZZZ"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Target last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText16 = new wxStaticText( itemDialog1, wxID_STATIC, _("DDD DD MMM YYYY HH:MM:SS XM ZZZ"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_STATIC, _("Last accessed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_STATIC, _("DDD DD MMM YYYY HH:MM:SS XM ZZZ"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText( itemDialog1, wxID_STATIC, _("Any field last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText20 = new wxStaticText( itemDialog1, wxID_STATIC, _("DDD DD MMM YYYY HH:MM:SS XM ZZZ"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer21 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer21, 0, wxGROW|wxALL, 5);
  wxButton* itemButton22 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton22);

  wxButton* itemButton23 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton23);

  wxButton* itemButton24 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton24);

  itemStdDialogButtonSizer21->Realize();

  // Set validators
  itemTextCtrl8->SetValidator( wxGenericValidator(& m_title) );
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_user) );
  itemStaticText14->SetValidator( wxGenericValidator(& m_created) );
  itemStaticText16->SetValidator( wxGenericValidator(& m_lastChanged) );
  itemStaticText18->SetValidator( wxGenericValidator(& m_lastAccess) );
  itemStaticText20->SetValidator( wxGenericValidator(& m_lastAny) );
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

wxBitmap EditShortcut::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin EditShortcut bitmap retrieval
  return wxNullBitmap;
////@end EditShortcut bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon EditShortcut::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin EditShortcut icon retrieval
  return wxNullIcon;
////@end EditShortcut icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void EditShortcut::OnOkClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    bool modified = false;
    CItemData modified_item(*m_item);
    const wxString group = m_groupCtrl->GetValue();
    if (group != m_item->GetGroup().c_str()) {
      modified = true;
      modified_item.SetGroup(tostringx(group));
    }
    if (m_title != m_item->GetTitle().c_str()) {
      modified = true;
      modified_item.SetTitle(tostringx(m_title));
    }
    if (m_user != m_item->GetUser().c_str()) {
      modified = true;
      modified_item.SetUser(tostringx(m_user));
    }
    if (modified) {
      time_t t;
      time(&t);
      modified_item.SetRMTime(t);
      m_core.Execute(EditEntryCommand::Create(&m_core,*m_item,
                                              modified_item));
    }
  }
  EndModal(wxID_OK);
}
