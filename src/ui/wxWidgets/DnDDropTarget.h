/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/


/** \file DndDropTarget.h
 * 
 */

#ifndef _DNDDROPTARGET_H_
#define _DNDDROPTARGET_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>

#include "DnDPWSafeObject.h"
////@end includes

////@begin forward declarations
class TreeCtrl;
////@end forward declarations
#ifndef wxOVERRIDE
#define wxOVERRIDE override
#endif

class DndPWSafeDropTarget : public wxDropTarget
{
  // Construction
public:
  DndPWSafeDropTarget(TreeCtrl *frame) : wxDropTarget(new DnDPWSafeObject)
  {
    m_tree = frame;
    SetDefaultAction(wxDragCopy); // We only do copy, no move
  }

  // Implementation
  virtual bool OnDrop(wxCoord x, wxCoord y) wxOVERRIDE;
  virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) wxOVERRIDE;
  
private:
  TreeCtrl *m_tree;
};

#endif // _DNDDROPTARGET_H_
