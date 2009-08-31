/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file optionspropsheet.cpp
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

#include "optionspropsheet.h"

////@begin XPM images

////@end XPM images


/*!
 * COptions type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COptions, wxPropertySheetDialog )


/*!
 * COptions event table definition
 */

BEGIN_EVENT_TABLE( COptions, wxPropertySheetDialog )

////@begin COptions event table entries
////@end COptions event table entries

END_EVENT_TABLE()


/*!
 * COptions constructors
 */

COptions::COptions()
{
  Init();
}

COptions::COptions( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * COptions creator
 */

bool COptions::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COptions creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  LayoutDialog();
  Centre();
////@end COptions creation
  return true;
}


/*!
 * COptions destructor
 */

COptions::~COptions()
{
////@begin COptions destruction
////@end COptions destruction
}


/*!
 * Member initialisation
 */

void COptions::Init()
{
////@begin COptions member initialisation
////@end COptions member initialisation
}


/*!
 * Control creation for COptions
 */

void COptions::CreateControls()
{    
////@begin COptions content construction
  COptions* itemPropertySheetDialog1 = this;

////@end COptions content construction
}


/*!
 * Should we show tooltips?
 */

bool COptions::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap COptions::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin COptions bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end COptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon COptions::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin COptions icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end COptions icon retrieval
}
