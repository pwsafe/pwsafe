/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file search.cpp
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

#include "search.h"

////@begin XPM images
////@end XPM images


/*!
 * AdvancedSearchOptionsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( AdvancedSearchOptionsDlg, wxDialog )


/*!
 * AdvancedSearchOptionsDlg event table definition
 */

BEGIN_EVENT_TABLE( AdvancedSearchOptionsDlg, wxDialog )

////@begin AdvancedSearchOptionsDlg event table entries
  EVT_BUTTON( ID_BUTTON10, AdvancedSearchOptionsDlg::OnSelOneButtonClick )

  EVT_BUTTON( ID_BUTTON11, AdvancedSearchOptionsDlg::OnSelAllButtonClick )

  EVT_BUTTON( ID_BUTTON12, AdvancedSearchOptionsDlg::OnDeSelOneButtonClick )

  EVT_BUTTON( ID_BUTTON13, AdvancedSearchOptionsDlg::OnDeSelAllButtonClick )

////@end AdvancedSearchOptionsDlg event table entries

END_EVENT_TABLE()


/*!
 * AdvancedSearchOptionsDlg constructors
 */

AdvancedSearchOptionsDlg::AdvancedSearchOptionsDlg()
{
  Init();
}

AdvancedSearchOptionsDlg::AdvancedSearchOptionsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * AdvancedSearchOptionsDlg creator
 */

bool AdvancedSearchOptionsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin AdvancedSearchOptionsDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end AdvancedSearchOptionsDlg creation
  return true;
}


/*!
 * AdvancedSearchOptionsDlg destructor
 */

AdvancedSearchOptionsDlg::~AdvancedSearchOptionsDlg()
{
////@begin AdvancedSearchOptionsDlg destruction
////@end AdvancedSearchOptionsDlg destruction
}


/*!
 * Member initialisation
 */

void AdvancedSearchOptionsDlg::Init()
{
////@begin AdvancedSearchOptionsDlg member initialisation
  m_restrict2subset = false;
  m_caseSensitive = false;
  m_fieldsCombo = NULL;
  m_relationCombo = NULL;
  m_availFields = NULL;
  m_selectedFields = NULL;
////@end AdvancedSearchOptionsDlg member initialisation
}


/*!
 * Control creation for AdvancedSearchOptionsDlg
 */

void AdvancedSearchOptionsDlg::CreateControls()
{    
////@begin AdvancedSearchOptionsDlg content construction
  AdvancedSearchOptionsDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, wxEmptyString);
  wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxCheckBox* itemCheckBox4 = new wxCheckBox( itemDialog1, ID_CHECKBOX35, _("Restrict to a subset of entries:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox4->SetValue(false);
  itemStaticBoxSizer3->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, 0);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer3->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("where"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_fieldsCombo = new wxComboCtrl( itemDialog1, ID_COMBOCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(m_fieldsCombo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_relationCombo = new wxComboCtrl( itemDialog1, ID_COMBOCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(m_relationCombo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("the following text:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer3->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL15, wxEmptyString, wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(80, -1)).x, -1), 0 );
  itemStaticBoxSizer3->Add(itemTextCtrl10, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox11 = new wxCheckBox( itemDialog1, ID_CHECKBOX36, _("Case Sensitive"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox11->SetValue(false);
  itemStaticBoxSizer3->Add(itemCheckBox11, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer12, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer12->Add(itemBoxSizer13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemDialog1, wxID_STATIC, _("Available Fields:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString m_availFieldsStrings;
  m_availFields = new wxListBox( itemDialog1, ID_LISTBOX1, wxDefaultPosition, wxSize(-1, itemDialog1->ConvertDialogToPixels(wxSize(-1, 80)).y), m_availFieldsStrings, wxLB_SINGLE );
  itemBoxSizer13->Add(m_availFields, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer12->Add(itemBoxSizer16, 0, wxGROW|wxALL, 5);

  itemBoxSizer16->Add(10, 21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton18 = new wxButton( itemDialog1, ID_BUTTON10, _(">"), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(20, 10)), 0 );
  itemBoxSizer16->Add(itemButton18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton19 = new wxButton( itemDialog1, ID_BUTTON11, _(">>"), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(20, 10)), 0 );
  itemBoxSizer16->Add(itemButton19, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer16->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton21 = new wxButton( itemDialog1, ID_BUTTON12, _("<"), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(20, 10)), 0 );
  itemBoxSizer16->Add(itemButton21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton22 = new wxButton( itemDialog1, ID_BUTTON13, _("<<"), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(20, 10)), 0 );
  itemBoxSizer16->Add(itemButton22, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer16->Add(10, 21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer12->Add(itemBoxSizer24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText25 = new wxStaticText( itemDialog1, wxID_STATIC, _("Selected Fields:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString m_selectedFieldsStrings;
  m_selectedFields = new wxListBox( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxSize(-1, itemDialog1->ConvertDialogToPixels(wxSize(-1, 80)).y), m_selectedFieldsStrings, wxLB_SINGLE );
  itemBoxSizer24->Add(m_selectedFields, 0, wxGROW|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer27 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer27, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton28 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer27->AddButton(itemButton28);

  wxButton* itemButton29 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer27->AddButton(itemButton29);

  wxButton* itemButton30 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer27->AddButton(itemButton30);

  itemStdDialogButtonSizer27->Realize();

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_restrict2subset) );
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_fieldText) );
  itemCheckBox11->SetValidator( wxGenericValidator(& m_caseSensitive) );
////@end AdvancedSearchOptionsDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool AdvancedSearchOptionsDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap AdvancedSearchOptionsDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin AdvancedSearchOptionsDlg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end AdvancedSearchOptionsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AdvancedSearchOptionsDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin AdvancedSearchOptionsDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end AdvancedSearchOptionsDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON10
 */

void AdvancedSearchOptionsDlg::OnSelOneButtonClick( wxCommandEvent& evt )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON10 in AdvancedSearchOptionsDlg.
  // Before editing this code, remove the block markers.
  evt.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON10 in AdvancedSearchOptionsDlg. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
 */

void AdvancedSearchOptionsDlg::OnSelAllButtonClick( wxCommandEvent& evt )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11 in AdvancedSearchOptionsDlg.
  // Before editing this code, remove the block markers.
  evt.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11 in AdvancedSearchOptionsDlg. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
 */

void AdvancedSearchOptionsDlg::OnDeSelOneButtonClick( wxCommandEvent& evt )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12 in AdvancedSearchOptionsDlg.
  // Before editing this code, remove the block markers.
  evt.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12 in AdvancedSearchOptionsDlg. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13
 */

void AdvancedSearchOptionsDlg::OnDeSelAllButtonClick( wxCommandEvent& evt )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13 in AdvancedSearchOptionsDlg.
  // Before editing this code, remove the block markers.
  evt.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13 in AdvancedSearchOptionsDlg. 
}

