/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManagePwdPolicies.cpp
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

#include "ManagePwdPolicies.h"

////@begin XPM images
////@end XPM images


/*!
 * CManagePasswordPolicies type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CManagePasswordPolicies, wxDialog )


/*!
 * CManagePasswordPolicies event table definition
 */

BEGIN_EVENT_TABLE( CManagePasswordPolicies, wxDialog )

////@begin CManagePasswordPolicies event table entries
  EVT_BUTTON( wxID_NEW, CManagePasswordPolicies::OnNewClick )

  EVT_BUTTON( ID_EDIT_PP, CManagePasswordPolicies::OnEditPpClick )

  EVT_BUTTON( wxID_DELETE, CManagePasswordPolicies::OnDeleteClick )

  EVT_BUTTON( ID_LIST, CManagePasswordPolicies::OnListClick )

  EVT_BUTTON( wxID_UNDO, CManagePasswordPolicies::OnUndoClick )

  EVT_BUTTON( wxID_REDO, CManagePasswordPolicies::OnRedoClick )

  EVT_BUTTON( ID_GENERATE_PASSWORD, CManagePasswordPolicies::OnGeneratePasswordClick )

  EVT_BUTTON( wxID_OK, CManagePasswordPolicies::OnOkClick )

  EVT_BUTTON( wxID_CANCEL, CManagePasswordPolicies::OnCancelClick )

  EVT_BUTTON( wxID_HELP, CManagePasswordPolicies::OnHelpClick )

////@end CManagePasswordPolicies event table entries

END_EVENT_TABLE()


/*!
 * CManagePasswordPolicies constructors
 */

CManagePasswordPolicies::CManagePasswordPolicies()
{
  Init();
}

CManagePasswordPolicies::CManagePasswordPolicies( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * CManagePasswordPolicies creator
 */

bool CManagePasswordPolicies::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CManagePasswordPolicies creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CManagePasswordPolicies creation
  return true;
}


/*!
 * CManagePasswordPolicies destructor
 */

CManagePasswordPolicies::~CManagePasswordPolicies()
{
////@begin CManagePasswordPolicies destruction
////@end CManagePasswordPolicies destruction
}


/*!
 * Member initialisation
 */

void CManagePasswordPolicies::Init()
{
////@begin CManagePasswordPolicies member initialisation
////@end CManagePasswordPolicies member initialisation
}


/*!
 * Control creation for CManagePasswordPolicies
 */

void CManagePasswordPolicies::CreateControls()
{    
////@begin CManagePasswordPolicies content construction
  CManagePasswordPolicies* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Available Password Policies:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGrid* itemGrid5 = new wxGrid( itemDialog1, ID_POLICYLIST, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid5->SetDefaultColSize(50);
  itemGrid5->SetDefaultRowSize(25);
  itemGrid5->SetColLabelSize(25);
  itemGrid5->SetRowLabelSize(50);
  itemGrid5->CreateGrid(10, 2, wxGrid::wxGridSelectRows);
  itemBoxSizer4->Add(itemGrid5, 3, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer4->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxGridSizer* itemGridSizer7 = new wxGridSizer(0, 2, 0, 0);
  itemBoxSizer6->Add(itemGridSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton( itemDialog1, wxID_NEW, _("&New"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, ID_EDIT_PP, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, wxID_DELETE, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_LIST, _("List"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_UNDO, _("&Undo"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_REDO, _("&Redo"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemDialog1, wxID_STATIC, _("Test Selected Policy"));
  wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
  itemBoxSizer6->Add(itemStaticBoxSizer14, 0, wxGROW|wxALL, 5);

  wxButton* itemButton15 = new wxButton( itemDialog1, ID_GENERATE_PASSWORD, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer14->Add(itemBoxSizer16, 0, wxGROW|wxALL, 5);

  wxTextCtrl* itemTextCtrl17 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL21, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer16->Add(itemTextCtrl17, 5, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton18 = new wxBitmapButton( itemDialog1, ID_BITMAPBUTTON, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBoxSizer16->Add(itemBitmapButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText( itemDialog1, wxID_STATIC, _("Selected policy details:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALL, 5);

  wxGrid* itemGrid20 = new wxGrid( itemDialog1, ID_POLICYPROPERTIES, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid20->SetDefaultColSize(50);
  itemGrid20->SetDefaultRowSize(25);
  itemGrid20->SetColLabelSize(25);
  itemGrid20->SetRowLabelSize(50);
  itemGrid20->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemBoxSizer2->Add(itemGrid20, 0, wxGROW|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer21 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton22 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton22);

  wxButton* itemButton23 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton23);

  wxButton* itemButton24 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer21->AddButton(itemButton24);

  itemStdDialogButtonSizer21->Realize();

////@end CManagePasswordPolicies content construction
}


/*!
 * Should we show tooltips?
 */

bool CManagePasswordPolicies::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CManagePasswordPolicies::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CManagePasswordPolicies bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end CManagePasswordPolicies bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CManagePasswordPolicies::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CManagePasswordPolicies icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CManagePasswordPolicies icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEW
 */

void CManagePasswordPolicies::OnNewClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEW in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEW in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT
 */

void CManagePasswordPolicies::OnEditPpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE
 */

void CManagePasswordPolicies::OnDeleteClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LIST
 */

void CManagePasswordPolicies::OnListClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LIST in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LIST in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UNDO
 */

void CManagePasswordPolicies::OnUndoClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UNDO in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UNDO in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO
 */

void CManagePasswordPolicies::OnRedoClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CManagePasswordPolicies::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CManagePasswordPolicies::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void CManagePasswordPolicies::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERATE_PASSWORD
 */

void CManagePasswordPolicies::OnGeneratePasswordClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERATE_PASSWORD in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERATE_PASSWORD in CManagePasswordPolicies. 
}

