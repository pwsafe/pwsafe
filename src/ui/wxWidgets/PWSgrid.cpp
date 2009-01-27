/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwsgrid.cpp
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

#include "PWSgrid.h"

////@begin XPM images
////@end XPM images


/*!
 * PWSGrid type definition
 */

IMPLEMENT_DYNAMIC_CLASS( PWSGrid, wxGrid )


/*!
 * PWSGrid event table definition
 */

BEGIN_EVENT_TABLE( PWSGrid, wxGrid )

////@begin PWSGrid event table entries
////@end PWSGrid event table entries

END_EVENT_TABLE()


/*!
 * PWSGrid constructors
 */

PWSGrid::PWSGrid()
{
  Init();
}

PWSGrid::PWSGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
  Init();
  Create(parent, id, pos, size, style);
}


/*!
 * PWSGrid creator
 */

bool PWSGrid::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin PWSGrid creation
  wxGrid::Create(parent, id, pos, size, style);
  CreateControls();
////@end PWSGrid creation
  return true;
}


/*!
 * PWSGrid destructor
 */

PWSGrid::~PWSGrid()
{
////@begin PWSGrid destruction
////@end PWSGrid destruction
}


/*!
 * Member initialisation
 */

void PWSGrid::Init()
{
////@begin PWSGrid member initialisation
////@end PWSGrid member initialisation
}


/*!
 * Control creation for PWSGrid
 */

void PWSGrid::CreateControls()
{    
////@begin PWSGrid content construction
////@end PWSGrid content construction
}


/*!
 * Should we show tooltips?
 */

bool PWSGrid::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap PWSGrid::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin PWSGrid bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end PWSGrid bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon PWSGrid::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin PWSGrid icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end PWSGrid icon retrieval
}

void PWSGrid::Clear()
{
  int N = GetNumberRows();
  if (N > 0)
    DeleteRows(0, N);
}

void PWSGrid::AddItem(const CItemData &item, int row)
{
  wxString title = item.GetTitle().c_str();
  wxString user = item.GetUser().c_str();
  SetCellValue(row, 0, title);
  SetCellValue(row, 1, user);
}
