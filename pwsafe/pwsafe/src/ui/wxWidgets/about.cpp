/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file about.cpp
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

#include "about.h"

////@begin XPM images
////@end XPM images


/*!
 * CAbout type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAbout, wxDialog )


/*!
 * CAbout event table definition
 */

BEGIN_EVENT_TABLE( CAbout, wxDialog )

////@begin CAbout event table entries
////@end CAbout event table entries

END_EVENT_TABLE()


/*!
 * CAbout constructors
 */

CAbout::CAbout()
{
  Init();
}

CAbout::CAbout( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * CAbout creator
 */

bool CAbout::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAbout creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CAbout creation
  return true;
}


/*!
 * CAbout destructor
 */

CAbout::~CAbout()
{
////@begin CAbout destruction
////@end CAbout destruction
}


/*!
 * Member initialisation
 */

void CAbout::Init()
{
////@begin CAbout member initialisation
////@end CAbout member initialisation
}


/*!
 * Control creation for CAbout
 */

void CAbout::CreateControls()
{    
////@begin CAbout content construction
  CAbout* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticBitmap* itemStaticBitmap3 = new wxStaticBitmap( itemDialog1, wxID_STATIC, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticBitmap3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_VERSIONSTR, _("Password Safe vx.yy (abcd)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please visit the PasswordSafe website"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer4->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CAbout content construction
}


/*!
 * Should we show tooltips?
 */

bool CAbout::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAbout::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CAbout bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end CAbout bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAbout::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CAbout icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CAbout icon retrieval
}
