/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DndFile.h
* 
*/

#ifndef _PWSAFE_DNDFILE_H_
#define _PWSAFE_DNDFILE_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dnd.h>
////@end includes

#include "PasswordSafeFrame.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class PasswordSafeFrame;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

class DnDFile : public wxFileDropTarget
{
public:
  DnDFile(PasswordSafeFrame *parent) { m_pOwner = parent; }

  virtual bool OnDropFiles(wxCoord x, wxCoord y,
                           const wxArrayString& filenames) wxOVERRIDE;

private:
  PasswordSafeFrame *m_pOwner;
};

#endif // _PWSAFE_DNDFILE_H_
