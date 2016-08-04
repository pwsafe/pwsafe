/*
 * Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file passwordsubset.cpp
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

#include "passwordsubset.h"

////@begin XPM images
////@end XPM images


/*!
 * CPasswordSubset type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPasswordSubset, wxDialog )


/*!
 * CPasswordSubset event table definition
 */

BEGIN_EVENT_TABLE( CPasswordSubset, wxDialog )

////@begin CPasswordSubset event table entries
////@end CPasswordSubset event table entries

END_EVENT_TABLE()


/*!
 * CPasswordSubset constructors
 */

CPasswordSubset::CPasswordSubset()
{
  Init();
}

CPasswordSubset::CPasswordSubset( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * CPasswordSubset creator
 */

bool CPasswordSubset::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPasswordSubset creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CPasswordSubset creation
  return true;
}


/*!
 * CPasswordSubset destructor
 */

CPasswordSubset::~CPasswordSubset()
{
////@begin CPasswordSubset destruction
////@end CPasswordSubset destruction
}


/*!
 * Member initialisation
 */

void CPasswordSubset::Init()
{
////@begin CPasswordSubset member initialisation
////@end CPasswordSubset member initialisation
}


/*!
 * Control creation for CPasswordSubset
 */

void CPasswordSubset::CreateControls()
{    
////@begin CPasswordSubset content construction
  CPasswordSubset* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Enter positions of password characters separated by spaces, commas or semi-colons.\nPosition 1 is the first character, 2 is the second, etc. up to N for the last character of a password of length N.\n-1 is the last character, -2 the next to last, etc. up to -N for the first password character."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGridSizer* itemGridSizer4 = new wxGridSizer(2, 3, 0, 0);
  itemBoxSizer2->Add(itemGridSizer4, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Positions:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemTextCtrl6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemGridSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Values:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton10 = new wxBitmapButton( itemDialog1, ID_BITMAPBUTTON, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemGridSizer4->Add(itemBitmapButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CPasswordSubset content construction
}


/*!
 * Should we show tooltips?
 */

bool CPasswordSubset::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPasswordSubset::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CPasswordSubset bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end CPasswordSubset bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPasswordSubset::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CPasswordSubset icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CPasswordSubset icon retrieval
}
