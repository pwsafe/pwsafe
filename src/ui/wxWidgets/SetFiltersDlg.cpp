/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SetFiltersDlg.cpp
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

#include "SetFiltersDlg.h"

////@begin XPM images
////@end XPM images


/*!
 * SetFilters type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SetFilters, wxDialog )


/*!
 * SetFilters event table definition
 */

BEGIN_EVENT_TABLE( SetFilters, wxDialog )

////@begin SetFilters event table entries
  EVT_GRID_CELL_LEFT_CLICK( SetFilters::OnCellLeftClick )
  EVT_BUTTON( wxID_APPLY, SetFilters::OnApplyClick )
  EVT_BUTTON( wxID_OK, SetFilters::OnOkClick )
  EVT_BUTTON( wxID_CANCEL, SetFilters::OnCancelClick )
  EVT_BUTTON( wxID_HELP, SetFilters::OnHelpClick )
////@end SetFilters event table entries

END_EVENT_TABLE()


/*!
 * SetFilters constructors
 */

SetFilters::SetFilters()
{
  Init();
}

SetFilters::SetFilters( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * SetFilters creator
 */

bool SetFilters::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin SetFilters creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end SetFilters creation
  return true;
}


/*!
 * SetFilters destructor
 */

SetFilters::~SetFilters()
{
////@begin SetFilters destruction
////@end SetFilters destruction
}


/*!
 * Member initialisation
 */

void SetFilters::Init()
{
////@begin SetFilters member initialisation
  m_filterGrid = NULL;
////@end SetFilters member initialisation
}


/*!
 * Control creation for SetFilters
 */

void SetFilters::CreateControls()
{    
////@begin SetFilters content construction
  SetFilters* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Filter name:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl3 = new wxTextCtrl( itemDialog1, ID_FilterName, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer1->Add(itemTextCtrl3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_filterGrid = new wxGrid( itemDialog1, ID_FILTERGRID, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_filterGrid->SetDefaultColSize(50);
  m_filterGrid->SetDefaultRowSize(25);
  m_filterGrid->SetColLabelSize(25);
  m_filterGrid->SetRowLabelSize(50);
  m_filterGrid->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
  itemBoxSizer2->Add(m_filterGrid, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton6 = new wxButton( itemDialog1, wxID_APPLY, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Set validators
  itemTextCtrl3->SetValidator( wxTextValidator(wxFILTER_NONE, & m_filterName) );
////@end SetFilters content construction
}


/*!
 * wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERGRID
 */

void SetFilters::OnCellLeftClick( wxGridEvent& event )
{
////@begin wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERGRID in SetFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERGRID in SetFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
 */

void SetFilters::OnApplyClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY in SetFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY in SetFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SetFilters::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in SetFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in SetFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void SetFilters::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in SetFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in SetFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void SetFilters::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in SetFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in SetFilters. 
}


/*!
 * Should we show tooltips?
 */

bool SetFilters::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap SetFilters::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin SetFilters bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end SetFilters bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon SetFilters::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin SetFilters icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end SetFilters icon retrieval
}
