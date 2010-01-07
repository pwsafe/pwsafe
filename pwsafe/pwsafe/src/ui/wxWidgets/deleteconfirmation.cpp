/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file deleteconfirmation.cpp
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

#include "deleteconfirmation.h"
#include "corelib/PWSprefs.h"

////@begin XPM images
////@end XPM images


/*!
 * DeleteConfirmation type definition
 */

IMPLEMENT_CLASS( DeleteConfirmation, wxDialog )


/*!
 * DeleteConfirmation event table definition
 */

BEGIN_EVENT_TABLE( DeleteConfirmation, wxDialog )

////@begin DeleteConfirmation event table entries
  EVT_BUTTON( wxID_YES, DeleteConfirmation::OnYesClick )

  EVT_BUTTON( wxID_NO, DeleteConfirmation::OnNoClick )

////@end DeleteConfirmation event table entries

END_EVENT_TABLE()


/*!
 * DeleteConfirmation constructors
 */

DeleteConfirmation::DeleteConfirmation(int num_children)
: m_numchildren(num_children)
{
  Init();
}

DeleteConfirmation::DeleteConfirmation( wxWindow* parent, int num_children, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : m_numchildren(num_children)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * DeleteConfirmation creator
 */

bool DeleteConfirmation::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DeleteConfirmation creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end DeleteConfirmation creation
  return true;
}


/*!
 * DeleteConfirmation destructor
 */

DeleteConfirmation::~DeleteConfirmation()
{
////@begin DeleteConfirmation destruction
////@end DeleteConfirmation destruction
}


/*!
 * Member initialisation
 */

void DeleteConfirmation::Init()
{
////@begin DeleteConfirmation member initialisation
  m_areyousure = NULL;
////@end DeleteConfirmation member initialisation
}


/*!
 * Control creation for DeleteConfirmation
 */

void DeleteConfirmation::CreateControls()
{    
////@begin DeleteConfirmation content construction
  DeleteConfirmation* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  m_areyousure = new wxStaticText( itemDialog1, wxID_STATIC, _("Are you sure you want to delete the selected entry?"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(m_areyousure, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxCheckBox* itemCheckBox4 = new wxCheckBox( itemDialog1, ID_CHECKBOX37, _("Don't ask me again"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox4->SetValue(false);
  itemBoxSizer2->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton6 = new wxButton( itemDialog1, wxID_YES, _("&Yes"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer5->AddButton(itemButton6);

  wxButton* itemButton7 = new wxButton( itemDialog1, wxID_NO, _("&No"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton7->SetDefault();
  itemStdDialogButtonSizer5->AddButton(itemButton7);

  itemStdDialogButtonSizer5->Realize();

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_confirmdelete) );
////@end DeleteConfirmation content construction
}


/*!
 * Should we show tooltips?
 */

bool DeleteConfirmation::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap DeleteConfirmation::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin DeleteConfirmation bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end DeleteConfirmation bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DeleteConfirmation::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin DeleteConfirmation icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end DeleteConfirmation icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NO
 */

void DeleteConfirmation::OnNoClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::DeleteQuestion,
                                     m_confirmdelete);
    
  }
  EndModal(wxID_NO);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_YES
 */

void DeleteConfirmation::OnYesClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::DeleteQuestion,
                                     !m_confirmdelete);
    
  }
  EndModal(wxID_YES);
}

