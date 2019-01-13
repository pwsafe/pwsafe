/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "../../core/PwsPlatform.h"
#include "../../core/ItemData.h"
#include "./wxutils.h"
////@end includes

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
#include "./graphics/dragbar/new/Email.xpm"
#include "./graphics/dragbar/new/EmailX.xpm"
#include "./graphics/dragbar/new/Group.xpm"
#include "./graphics/dragbar/new/GroupX.xpm"
#include "./graphics/dragbar/new/Notes.xpm"
#include "./graphics/dragbar/new/NotesX.xpm"
#include "./graphics/dragbar/new/Password.xpm"
#include "./graphics/dragbar/new/PasswordX.xpm"
#include "./graphics/dragbar/new/Title.xpm"
#include "./graphics/dragbar/new/TitleX.xpm"
#include "./graphics/dragbar/new/URL.xpm"
#include "./graphics/dragbar/new/URLX.xpm"
#include "./graphics/dragbar/new/User.xpm"
#include "./graphics/dragbar/new/UserX.xpm"
//-- classic bitmaps...
#include "./graphics/dragbar/classic/email.xpm"
#include "./graphics/dragbar/classic/emailx.xpm"
#include "./graphics/dragbar/classic/Group.xpm"
#include "./graphics/dragbar/classic/GroupX.xpm"
#include "./graphics/dragbar/classic/Notes.xpm"
#include "./graphics/dragbar/classic/NotesX.xpm"
#include "./graphics/dragbar/classic/Password.xpm"
#include "./graphics/dragbar/classic/PasswordX.xpm"
#include "./graphics/dragbar/classic/Title.xpm"
#include "./graphics/dragbar/classic/TitleX.xpm"
#include "./graphics/dragbar/classic/URL.xpm"
#include "./graphics/dragbar/classic/URLX.xpm"
#include "./graphics/dragbar/classic/User.xpm"
#include "./graphics/dragbar/classic/UserX.xpm"

////@end XPM images

IMPLEMENT_CLASS( PWSDragBar, CDragBar )

enum { DRAGBAR_TOOLID_BASE = 100 };

#define PWS_TOOLINFO(t, f) {  wxSTRINGIZE_T(t),                                       \
                              wxCONCAT(t, _xpm),                                      \
                              wxCONCAT(t, X_xpm),                                     \
                              wxCONCAT(wxCONCAT(classic_, t), _xpm),                  \
                              wxCONCAT(wxCONCAT(classic_, t), X_xpm),                 \
                              CItemData::f  }

struct _DragbarElementInfo {
  const TCHAR* name;
  const char** bitmap;
  const char** bitmap_disabled;
  const char** classic_bitmap;
  const char** classic_bitmap_disabled;
  CItemData::FieldType ft;
} DragbarElements[] = { PWS_TOOLINFO(Group,     GROUP),
                        PWS_TOOLINFO(Title,     TITLE),
                        PWS_TOOLINFO(User,      USER),
                        PWS_TOOLINFO(Password,  PASSWORD),
                        PWS_TOOLINFO(Notes,     NOTES),
                        PWS_TOOLINFO(URL,       URL),
                        PWS_TOOLINFO(Email,     EMAIL)
                      };

PWSDragBar::PWSDragBar(PasswordSafeFrame* frame) : CDragBar(frame, this), m_frame(frame)
{
  RefreshButtons();
}

void PWSDragBar::RefreshButtons()
{
  const bool newButtons = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar);

#define BTN newButtons? wxBitmap(DragbarElements[idx].bitmap) : wxBitmap(DragbarElements[idx].classic_bitmap)
#define BTN_DISABLED newButtons? wxBitmap(DragbarElements[idx].bitmap_disabled): wxBitmap(DragbarElements[idx].classic_bitmap_disabled)

  if (GetToolsCount() == 0) {  //being created?
    for (int idx = 0; size_t(idx) < NumberOf(DragbarElements); ++idx) {
      AddTool(idx + DRAGBAR_TOOLID_BASE, BTN,
                wxString(_("Drag this image onto another window to paste the '"))
                        << _(DragbarElements[idx].name) << _("' field."), BTN_DISABLED );
    }
  }
  else {
    for (int idx = 0; size_t(idx) < NumberOf(DragbarElements); ++idx) {
      SetToolBitmaps(idx + DRAGBAR_TOOLID_BASE, BTN, BTN_DISABLED);
    }
  }

#undef BTN
#undef BTN_DISABLED
}

PWSDragBar::~PWSDragBar()
{
}

wxString PWSDragBar::GetText(int id) const
{
  const int idx = id - DRAGBAR_TOOLID_BASE;
  wxASSERT( idx >= 0 && size_t(idx) < NumberOf(DragbarElements));

  const CItemData *pci(nullptr), *pbci(nullptr);
  pci = m_frame->GetSelectedEntry();
  pbci = m_frame->GetBaseEntry(pci);

  return pci ?
    towxstring(pci->GetEffectiveFieldValue(DragbarElements[idx].ft, pbci)) : wxString(wxEmptyString);
}

bool PWSDragBar::IsEnabled(int id) const
{
  const int idx = id - DRAGBAR_TOOLID_BASE;
  wxASSERT( idx >= 0 && size_t(idx) < NumberOf(DragbarElements));

  const CItemData *pci(nullptr), *pbci(nullptr);
  pci = m_frame->GetSelectedEntry();
  pbci = m_frame->GetBaseEntry(pci);

  return pci && !pci->IsFieldValueEmpty(DragbarElements[idx].ft, pbci);
}
