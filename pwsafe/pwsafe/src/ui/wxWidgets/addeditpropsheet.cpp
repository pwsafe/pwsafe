/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file addeditpropsheet.cpp
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
#include "wx/bookctrl.h"
////@end includes

#include "addeditpropsheet.h"

////@begin XPM images
////@end XPM images


/*!
 * AddEditPropSheet type definition
 */

IMPLEMENT_DYNAMIC_CLASS( AddEditPropSheet, wxPropertySheetDialog )


/*!
 * AddEditPropSheet event table definition
 */

BEGIN_EVENT_TABLE( AddEditPropSheet, wxPropertySheetDialog )

////@begin AddEditPropSheet event table entries
////@end AddEditPropSheet event table entries

END_EVENT_TABLE()


/*!
 * AddEditPropSheet constructors
 */

AddEditPropSheet::AddEditPropSheet()
{
  Init();
}

AddEditPropSheet::AddEditPropSheet( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * AddEditPropSheet creator
 */

bool AddEditPropSheet::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin AddEditPropSheet creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  LayoutDialog();
  Centre();
////@end AddEditPropSheet creation
  return true;
}


/*!
 * AddEditPropSheet destructor
 */

AddEditPropSheet::~AddEditPropSheet()
{
////@begin AddEditPropSheet destruction
////@end AddEditPropSheet destruction
}


/*!
 * Member initialisation
 */

void AddEditPropSheet::Init()
{
////@begin AddEditPropSheet member initialisation
////@end AddEditPropSheet member initialisation
}


/*!
 * Control creation for AddEditPropSheet
 */

void AddEditPropSheet::CreateControls()
{    
////@begin AddEditPropSheet content construction
  AddEditPropSheet* itemPropertySheetDialog1 = this;

  wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), ID_PANEL_BASIC, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxStaticText* itemStaticText4 = new wxStaticText( itemPanel2, wxID_STATIC, _("To add a new entry, simply fill in the fields below. At least a title and a\npassword are required. If you have set a default username, it will appear in the\nusername field."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText6 = new wxStaticText( itemPanel2, wxID_STATIC, _("Group:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer5->Add(10, 10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemComboBox8Strings;
  wxComboBox* itemComboBox8 = new wxComboBox( itemPanel2, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox8Strings, wxCB_DROPDOWN );
  itemBoxSizer5->Add(itemComboBox8, 3, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer5->Add(10, 10, 3, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer10, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText11 = new wxStaticText( itemPanel2, wxID_STATIC, _("Title:   "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer10->Add(10, 10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer10->Add(itemTextCtrl13, 3, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer10->Add(10, 10, 3, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer15, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText16 = new wxStaticText( itemPanel2, wxID_STATIC, _("Username: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer15->Add(10, 10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl18 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemTextCtrl18, 6, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer15->Add(10, 10, 6, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer20, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText21 = new wxStaticText( itemPanel2, wxID_STATIC, _("Password: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemStaticText21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer20->Add(10, 10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl23 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemTextCtrl23, 5, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton24 = new wxButton( itemPanel2, ID_BUTTON, _("Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxButton* itemButton25 = new wxButton( itemPanel2, ID_BUTTON1, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemButton25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer20->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer27, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText28 = new wxStaticText( itemPanel2, wxID_STATIC, _("Confirm\nPassword:  "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemStaticText28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  itemBoxSizer27->Add(10, 10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl30 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemTextCtrl30, 5, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer27->Add(10, 10, 5, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel2, _("Basic"));

  wxPanel* itemPanel32 = new wxPanel( GetBookCtrl(), ID_PANEL_ADDITIONAL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel32, _("Additional"));

  wxPanel* itemPanel33 = new wxPanel( GetBookCtrl(), ID_PANEL_DTIME, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel33, _("Dates and Times"));

  wxPanel* itemPanel34 = new wxPanel( GetBookCtrl(), ID_PANEL_PPOLICY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel34, _("Password Policy"));

////@end AddEditPropSheet content construction
}


/*!
 * Should we show tooltips?
 */

bool AddEditPropSheet::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap AddEditPropSheet::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin AddEditPropSheet bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end AddEditPropSheet bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AddEditPropSheet::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin AddEditPropSheet icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end AddEditPropSheet icon retrieval
}
