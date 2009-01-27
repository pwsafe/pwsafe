/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWSTreeCtrl.cpp
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

#include "PWStree.h"

////@begin XPM images
////@end XPM images


/*!
 * PWSTreeCtrl type definition
 */

IMPLEMENT_DYNAMIC_CLASS( PWSTreeCtrl, wxTreeCtrl )


/*!
 * PWSTreeCtrl event table definition
 */

BEGIN_EVENT_TABLE( PWSTreeCtrl, wxTreeCtrl )

////@begin PWSTreeCtrl event table entries
////@end PWSTreeCtrl event table entries

END_EVENT_TABLE()


/*!
 * PWSTreeCtrl constructors
 */

PWSTreeCtrl::PWSTreeCtrl()
{
  Init();
}

PWSTreeCtrl::PWSTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
  Init();
  Create(parent, id, pos, size, style);
}


/*!
 * PWSTreeCtrl creator
 */

bool PWSTreeCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin PWSTreeCtrl creation
  wxTreeCtrl::Create(parent, id, pos, size, style);
  CreateControls();
////@end PWSTreeCtrl creation
  return true;
}


/*!
 * PWSTreeCtrl destructor
 */

PWSTreeCtrl::~PWSTreeCtrl()
{
////@begin PWSTreeCtrl destruction
////@end PWSTreeCtrl destruction
}


/*!
 * Member initialisation
 */

void PWSTreeCtrl::Init()
{
////@begin PWSTreeCtrl member initialisation
////@end PWSTreeCtrl member initialisation
}


/*!
 * Control creation for PWSTreeCtrl
 */

void PWSTreeCtrl::CreateControls()
{    
////@begin PWSTreeCtrl content construction
////@end PWSTreeCtrl content construction
}

void PWSTreeCtrl::AddItem(const CItemData &item)
{
  wxString group = item.GetGroup().c_str();
  wxString title = item.GetTitle().c_str();
  wxString user = item.GetUser().c_str();
  wxString disp = title;
  if (!group.empty())
    disp = group + "." + disp;
  if (!user.empty())
    disp += " [" + user + "]";
  AppendItem(GetRootItem(), disp);
}

