/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file createshortcutdlg.cpp
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

#include "createshortcutdlg.h"
#include "core/ItemData.h"
#include "./wxutils.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * CreateShortcutDlg type definition
 */

IMPLEMENT_CLASS( CreateShortcutDlg, wxDialog )

/*!
 * CreateShortcutDlg event table definition
 */

BEGIN_EVENT_TABLE( CreateShortcutDlg, wxDialog )

////@begin CreateShortcutDlg event table entries
  EVT_BUTTON( wxID_OK, CreateShortcutDlg::OnOkClick )

////@end CreateShortcutDlg event table entries

END_EVENT_TABLE()

/*!
 * CreateShortcutDlg constructors
 */

CreateShortcutDlg::CreateShortcutDlg(wxWindow* parent, PWScore &core,
                                     CItemData *base,
                                     wxWindowID id, const wxString& caption,
                                     const wxPoint& pos, const wxSize& size,
                                     long style)
: m_core(core), m_base(base), m_ui(dynamic_cast<UIInterFace *>(parent))
{
  ASSERT(m_base != nullptr);
  ASSERT(m_ui != nullptr);
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * CreateShortcutDlg creator
 */

bool CreateShortcutDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CreateShortcutDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CreateShortcutDlg creation
  ItemFieldsToDialog();
  return true;
}

void CreateShortcutDlg::ItemFieldsToDialog()
{
  // Populate the combo box
  std::vector<stringT> aryGroups;
  m_core.GetAllGroups(aryGroups);
  for (size_t igrp = 0; igrp < aryGroups.size(); igrp++) {
    m_groupCtrl->Append(aryGroups[igrp].c_str());
  }
  // XXX TBD: Determine if there's a current
  // group that we can pre-select for user, e.g.,
  // we're invoked via right-click n a node
}

/*!
 * CreateShortcutDlg destructor
 */

CreateShortcutDlg::~CreateShortcutDlg()
{
////@begin CreateShortcutDlg destruction
////@end CreateShortcutDlg destruction
}

/*!
 * Member initialisation
 */

void CreateShortcutDlg::Init()
{
////@begin CreateShortcutDlg member initialisation
  m_groupCtrl = nullptr;
////@end CreateShortcutDlg member initialisation
}

/*!
 * Control creation for CreateShortcutDlg
 */

void CreateShortcutDlg::CreateControls()
{
////@begin CreateShortcutDlg content construction
  CreateShortcutDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please specify the name & group\n for shortcut to "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

  wxGridSizer* itemGridSizer4 = new wxGridSizer(3, 2, 0, 0);
  itemBoxSizer2->Add(itemGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Group:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_groupCtrlStrings;
  m_groupCtrl = new wxComboBox( itemDialog1, ID_COMBOBOX4, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_groupCtrlStrings, wxCB_DROPDOWN );
  itemGridSizer4->Add(m_groupCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL18, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemTextCtrl8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL19, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemTextCtrl10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer11 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer11, 0, wxGROW|wxALL, 5);
  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer11->AddButton(itemButton12);

  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer11->AddButton(itemButton13);

  wxButton* itemButton14 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer11->AddButton(itemButton14);

  itemStdDialogButtonSizer11->Realize();

  // Set validators
  itemStaticText3->SetValidator( wxGenericValidator(& m_heading) );
  itemTextCtrl8->SetValidator( wxGenericValidator(& m_title) );
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_user) );
////@end CreateShortcutDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool CreateShortcutDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CreateShortcutDlg::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin CreateShortcutDlg bitmap retrieval
  return wxNullBitmap;
////@end CreateShortcutDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CreateShortcutDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin CreateShortcutDlg icon retrieval
  return wxNullIcon;
////@end CreateShortcutDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CreateShortcutDlg::OnOkClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    bool valid = !m_title.empty();

    if (!valid)
      return;

    CItemData shortcut;
    shortcut.SetShortcut();
    shortcut.CreateUUID();
    shortcut.SetPassword(wxT("[Shortcut]"));
    const wxString group = m_groupCtrl->GetValue();

    if (!group.empty())
      shortcut.SetGroup(tostringx(group));
    shortcut.SetTitle(tostringx(m_title));
    if (!m_user.empty())
      shortcut.SetUser(tostringx(m_user));
    time_t t;
    time(&t);
    shortcut.SetCTime(t);
    shortcut.SetXTime(time_t(0));
    shortcut.SetStatus(CItemData::ES_ADDED);

    m_core.Execute(AddEntryCommand::Create(&m_core, shortcut,
					   m_base->GetUUID()));
  }
  EndModal(wxID_OK);
}
