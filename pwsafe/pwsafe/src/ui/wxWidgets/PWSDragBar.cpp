/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWSDragBar.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

////@begin includes
#include "PWSDragBar.h"
#include "passwordsafeframe.h"
#include "../../os/pws_tchar.h"
#include "../../corelib/PwsPlatform.h"
#include "../../corelib/ItemData.h"
#include "./wxutils.h"
////@end includes

////@begin XPM images
#include "../graphics/wxWidgets/dragbar/Email.xpm"
#include "../graphics/wxWidgets/dragbar/EmailX.xpm"
#include "../graphics/wxWidgets/dragbar/Group.xpm"
#include "../graphics/wxWidgets/dragbar/GroupX.xpm"
#include "../graphics/wxWidgets/dragbar/Notes.xpm"
#include "../graphics/wxWidgets/dragbar/NotesX.xpm"
#include "../graphics/wxWidgets/dragbar/Password.xpm"
#include "../graphics/wxWidgets/dragbar/PasswordX.xpm"
#include "../graphics/wxWidgets/dragbar/Title.xpm"
#include "../graphics/wxWidgets/dragbar/TitleX.xpm"
#include "../graphics/wxWidgets/dragbar/URL.xpm"
#include "../graphics/wxWidgets/dragbar/URLX.xpm"
#include "../graphics/wxWidgets/dragbar/User.xpm"
#include "../graphics/wxWidgets/dragbar/UserX.xpm"
////@end XPM images

IMPLEMENT_CLASS( PWSDragBar, CDragBar )

enum { DRAGBAR_TOOLID_BASE = 100 };

#define TOOLINFO(t, f) { wxSTRINGIZE_T(t), t, wxCONCAT(t, X), CItemData::f}

struct _DragbarElementInfo {
  const TCHAR* name;
  const char** bitmap;
  const char** bitmap_disabled;
  CItemData::FieldType ft;
} DragbarElements[] = { TOOLINFO(Group,     GROUP), 
                        TOOLINFO(Title,     TITLE), 
                        TOOLINFO(User,      USER), 
                        TOOLINFO(Password,  PASSWORD), 
                        TOOLINFO(Notes,     NOTES), 
                        TOOLINFO(URL,       URL), 
                        TOOLINFO(Email,     EMAIL)
                      };

PWSDragBar::PWSDragBar(PasswordSafeFrame* frame) : CDragBar(frame, this), m_frame(frame)
{
  for (int idx = 0; size_t(idx) < NumberOf(DragbarElements); ++idx) {
    AddTool(idx + DRAGBAR_TOOLID_BASE, wxBitmap(DragbarElements[idx].bitmap),
              wxString(_("Drag this image onto another window to paste the '"))
                      << DragbarElements[idx].name << _("' field."),
              wxBitmap(DragbarElements[idx].bitmap_disabled));
  }

}

PWSDragBar::~PWSDragBar()
{
}

wxString PWSDragBar::GetText(int id) const
{
  CItemData* item = m_frame->GetSelectedEntry();
  if (item) {
    return towxstring(item->GetFieldValue(DragbarElements[id-DRAGBAR_TOOLID_BASE].ft));
  }
  return wxEmptyString;
}

bool PWSDragBar::IsEnabled(int id) const
{
  const int idx = id - DRAGBAR_TOOLID_BASE;
  wxASSERT( idx >= 0 && size_t(idx) < NumberOf(DragbarElements));
  
  CItemData* item = m_frame->GetSelectedEntry();
  return item != 0 && item->GetFieldValue(DragbarElements[idx].ft).empty() == false;
}
