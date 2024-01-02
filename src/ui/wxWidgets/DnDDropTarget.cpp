/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DndDropTarget.cpp
* 
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin includes
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>

#include "DnDDropTarget.h"

#include "TreeCtrl.h"
////@end includes


/*!
 * OnDrop implementation
 */

bool DndPWSafeDropTarget::OnDrop(wxCoord x, wxCoord y)
{
  return m_tree->IsShown() && !m_tree->IsReadOnly() && (m_tree->IsSortingGroup() || ::wxGetKeyState(WXK_SHIFT));
}

/*!
 * OnData implementation
 */

wxDragResult DndPWSafeDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
  if (!GetData())
  {
    pws_os::Trace(L"Failed to get drag and drop data");
    return wxDragNone;
  }
  DnDPWSafeObject *obj = (DnDPWSafeObject *) GetDataObject();
  wxASSERT(obj);
  return m_tree->OnDrop(x, y, obj->GetDnDObject()); // DnDobject is not released by Data Object, must be done later
}

