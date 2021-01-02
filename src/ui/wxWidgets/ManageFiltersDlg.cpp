/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file managefilters.cpp
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

#include "ManageFiltersDlg.h"

////@begin XPM images
////@end XPM images


/*!
 * ManageFilters type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ManageFilters, wxDialog )


/*!
 * ManageFilters event table definition
 */

BEGIN_EVENT_TABLE( ManageFilters, wxDialog )

////@begin ManageFilters event table entries
  EVT_GRID_CELL_LEFT_CLICK( ManageFilters::OnCellLeftClick )
  EVT_BUTTON( wxID_NEW, ManageFilters::OnNewClick )
  EVT_BUTTON( ID_EDIT, ManageFilters::OnEditClick )
  EVT_BUTTON( wxID_COPY, ManageFilters::OnCopyClick )
  EVT_BUTTON( wxID_DELETE, ManageFilters::OnDeleteClick )
  EVT_BUTTON( ID_IMPORT, ManageFilters::OnImportClick )
  EVT_BUTTON( ID_EXPORT, ManageFilters::OnExportClick )
  EVT_BUTTON( wxID_HELP, ManageFilters::OnHelpClick )
  EVT_BUTTON( wxID_CLOSE, ManageFilters::OnCloseClick )
////@end ManageFilters event table entries

END_EVENT_TABLE()


/*!
 * ManageFilters constructors
 */

ManageFilters::ManageFilters()
{
  Init();
}

ManageFilters::ManageFilters( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * ManageFilters creator
 */

bool ManageFilters::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ManageFilters creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end ManageFilters creation
  return true;
}


/*!
 * ManageFilters destructor
 */

ManageFilters::~ManageFilters()
{
////@begin ManageFilters destruction
////@end ManageFilters destruction
}


/*!
 * Member initialisation
 */

void ManageFilters::Init()
{
////@begin ManageFilters member initialisation
////@end ManageFilters member initialisation
}


/*!
 * Control creation for ManageFilters
 */

void ManageFilters::CreateControls()
{    
////@begin ManageFilters content construction
  ManageFilters* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Available Filters:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGrid* itemGrid4 = new wxGrid( itemDialog1, ID_FILTERSGRID, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid4->SetDefaultColSize(50);
  itemGrid4->SetDefaultRowSize(25);
  itemGrid4->SetColLabelSize(25);
  itemGrid4->SetRowLabelSize(50);
  itemGrid4->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemBoxSizer3->Add(itemGrid4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxGridSizer* itemGridSizer5 = new wxGridSizer(0, 2, 0, 0);
  itemBoxSizer3->Add(itemGridSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton6 = new wxButton( itemDialog1, wxID_NEW, _("&New"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton7 = new wxButton( itemDialog1, ID_EDIT, _("&Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton( itemDialog1, wxID_COPY, _("&Copy"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, wxID_DELETE, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, ID_IMPORT, _("&Import"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_EXPORT, _("E&xport"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemDialog1, wxID_STATIC, _("Selected Filter Details:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  wxGrid* itemGrid15 = new wxGrid( itemDialog1, ID_GRID, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid15->SetDefaultColSize(50);
  itemGrid15->SetDefaultRowSize(25);
  itemGrid15->SetColLabelSize(25);
  itemGrid15->SetRowLabelSize(50);
  itemGrid15->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
  itemBoxSizer2->Add(itemGrid15, 0, wxGROW|wxALL, 5);

////@end ManageFilters content construction
}


/*!
 * Should we show tooltips?
 */

bool ManageFilters::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ManageFilters::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin ManageFilters bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end ManageFilters bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ManageFilters::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin ManageFilters icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end ManageFilters icon retrieval
}


/*!
 * wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERSGRID
 */

void ManageFilters::OnCellLeftClick( wxGridEvent& event )
{
////@begin wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERSGRID in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERSGRID in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
 */

void ManageFilters::OnNewClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT
 */

void ManageFilters::OnEditClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_COPY
 */

void ManageFilters::OnCopyClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_COPY in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_COPY in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
 */

void ManageFilters::OnDeleteClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT
 */

void ManageFilters::OnImportClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT
 */

void ManageFilters::OnExportClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void ManageFilters::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in ManageFilters. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void ManageFilters::OnCloseClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in ManageFilters.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in ManageFilters. 
}

