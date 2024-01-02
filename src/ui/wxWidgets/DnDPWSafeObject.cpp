/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DndPWSafeObject.cpp
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
////@end includes

#include "DnDPWSafeObject.h"

/*!
 * DnDPWSafeFormatId implementation
 */

static wxString DnDPWSafeFormatId()
{
  return "wxPWSafeDnD40";
}

/*!
 * DnDPWSafeObject implementation
 */

DnDPWSafeObject::DnDPWSafeObject(wxMemoryBuffer *object):wxDataObjectSimple(wxDataFormat(DnDPWSafeFormatId()))
{
  if(object)
  {
    // we need to copy the shape because the one we're handled may be
    // deleted while it's still on the clipboard (for example) - and we
    // reuse the serialisation methods here to copy it
    m_object = new wxMemoryBuffer(*object);
  }
  else // nothing to copy
    m_object = nullptr;
}
