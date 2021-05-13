/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DragBarCtrl.cpp
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
#include "core/ItemData.h"

#include "DragBarCtrl.h"
#include "PasswordSafeFrame.h"
#include "PWSafeApp.h"
////@end includes

////@begin XPM images
#include "graphics/dragbar/new/Email.xpm"
#include "graphics/dragbar/new/EmailX.xpm"
#include "graphics/dragbar/new/Group.xpm"
#include "graphics/dragbar/new/GroupX.xpm"
#include "graphics/dragbar/new/Notes.xpm"
#include "graphics/dragbar/new/NotesX.xpm"
#include "graphics/dragbar/new/Password.xpm"
#include "graphics/dragbar/new/PasswordX.xpm"
#include "graphics/dragbar/new/Title.xpm"
#include "graphics/dragbar/new/TitleX.xpm"
#include "graphics/dragbar/new/URL.xpm"
#include "graphics/dragbar/new/URLX.xpm"
#include "graphics/dragbar/new/User.xpm"
#include "graphics/dragbar/new/UserX.xpm"
#include "graphics/dragbar/new/Dnd.xpm"
#include "graphics/dragbar/new/DndX.xpm"
//-- classic bitmaps...
#include "graphics/dragbar/classic/email.xpm"
#include "graphics/dragbar/classic/emailx.xpm"
#include "graphics/dragbar/classic/Group.xpm"
#include "graphics/dragbar/classic/GroupX.xpm"
#include "graphics/dragbar/classic/Notes.xpm"
#include "graphics/dragbar/classic/NotesX.xpm"
#include "graphics/dragbar/classic/Password.xpm"
#include "graphics/dragbar/classic/PasswordX.xpm"
#include "graphics/dragbar/classic/Title.xpm"
#include "graphics/dragbar/classic/TitleX.xpm"
#include "graphics/dragbar/classic/URL.xpm"
#include "graphics/dragbar/classic/URLX.xpm"
#include "graphics/dragbar/classic/User.xpm"
#include "graphics/dragbar/classic/UserX.xpm"
#include "graphics/dragbar/classic/Dnd.xpm"
#include "graphics/dragbar/classic/DndX.xpm"

////@end XPM images

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
                        PWS_TOOLINFO(Email,     EMAIL),
                        PWS_TOOLINFO(Dnd,       UNKNOWNFIELDS) // Must be last entry
                      };

// Drag and drop tree or item is last element in drag bar
#define DND_IDX (NumberOf(DragbarElements) - 1)    // Last entry is DnD

DragBarCtrl::DragBarCtrl(wxWindow *parent, wxWindowID id, const wxPoint &position, const wxSize &size, long style) : wxAuiToolBar(parent, id, position, size, style)
{
  CreateToolbar();
  Bind(wxEVT_AUITOOLBAR_BEGIN_DRAG, &DragBarCtrl::OnDrag, this);
  Bind(wxEVT_UPDATE_UI, &DragBarCtrl::OnUpdateUI, this, DRAGBAR_TOOLID_BASE, DRAGBAR_TOOLID_BASE + DND_IDX);
}

DragBarCtrl::~DragBarCtrl()
{
  Unbind(wxEVT_UPDATE_UI, &DragBarCtrl::OnUpdateUI, this, DRAGBAR_TOOLID_BASE, DRAGBAR_TOOLID_BASE + DND_IDX);
  Unbind(wxEVT_AUITOOLBAR_BEGIN_DRAG, &DragBarCtrl::OnDrag, this);
}

void DragBarCtrl::CreateToolbar()
{
  const bool newButtons = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar);

#define BUTTON_ENABLED newButtons ? \
  wxBitmap(DragbarElements[idx].bitmap) : \
  wxBitmap(DragbarElements[idx].classic_bitmap)
#define BUTTON_DISABLED newButtons ? \
  wxBitmap(DragbarElements[idx].bitmap_disabled) : \
  wxBitmap(DragbarElements[idx].classic_bitmap_disabled)
#define BUTTON_TOOLTIP ((idx == DND_IDX) ? \
  wxString(_("Drag this image onto another window to paste the selected element or tree.")) : \
  wxString(_("Drag this image onto another window to paste the '")) << _(DragbarElements[idx].name) << _("' field."))

  ClearTools();

  for (int idx = 0; size_t(idx) < NumberOf(DragbarElements); ++idx) {
    AddTool(
      idx + DRAGBAR_TOOLID_BASE,
      BUTTON_ENABLED, BUTTON_DISABLED,
      false, nullptr,
      BUTTON_TOOLTIP
    );
  }

  Realize();

#undef BUTTON_ENABLED
#undef BUTTON_DISABLED
#undef BUTTON_TOOLTIP
}

void DragBarCtrl::UpdateBitmaps()
{
  const bool newButtons = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar);

#define BUTTON_ENABLED newButtons ? \
  wxBitmap(DragbarElements[idx].bitmap) : \
  wxBitmap(DragbarElements[idx].classic_bitmap)
#define BUTTON_DISABLED newButtons ? \
  wxBitmap(DragbarElements[idx].bitmap_disabled) : \
  wxBitmap(DragbarElements[idx].classic_bitmap_disabled)

  if (GetToolCount() > 0) {
    for (int idx = 0; size_t(idx) < NumberOf(DragbarElements); ++idx) {
      auto tool = FindToolByIndex(idx);
      if (tool) {
        tool->SetBitmap(BUTTON_ENABLED);
        tool->SetDisabledBitmap(BUTTON_DISABLED);
      }
    }

    Realize();
  }

#undef BUTTON_ENABLED
#undef BUTTON_DISABLED
}

void DragBarCtrl::UpdateTooltips()
{
#define BUTTON_TOOLTIP ((idx == DND_IDX) ? \
  wxString(_("Drag this image onto another window to paste the selected element or tree.")) : \
  wxString(_("Drag this image onto another window to paste the '")) << _(DragbarElements[idx].name) << _("' field."))

  if (GetToolCount() > 0) {
    for (int idx = 0; size_t(idx) < NumberOf(DragbarElements); ++idx) {
      auto tool = FindToolByIndex(idx);
      if (tool) {
        tool->SetShortHelp(BUTTON_TOOLTIP);
      }
    }

    Realize();
  }

#undef BUTTON_TOOLTIP
}

wxString DragBarCtrl::GetText(int idx) const
{
  wxASSERT( idx >= 0 && size_t(idx) < NumberOf(DragbarElements));

  if(idx == DND_IDX) {
    return wxString(wxEmptyString);
  }

  const CItemData *pci(nullptr), *pbci(nullptr);
  auto mainFrame = wxGetApp().GetPasswordSafeFrame();
  wxASSERT(mainFrame);
  pci = mainFrame->GetSelectedEntry();
  pbci = mainFrame->GetBaseEntry(pci);

  return pci ?
    towxstring(pci->GetEffectiveFieldValue(DragbarElements[idx].ft, pbci)) : wxString(wxEmptyString);
}

void DragBarCtrl::OnDrag(wxAuiToolBarEvent& event)
{
  auto toolId = static_cast<size_t>(event.GetToolId()) - DRAGBAR_TOOLID_BASE;
  wxASSERT(toolId >= 0 && toolId <= NumberOf(DragbarElements));

  // Separators have the index -1 (ID_SEPARATOR)
  if (toolId < 0) {
    return;
  }

  // The last dragbar item is addressing drag & drop for tree element(s)
  if(toolId == DND_IDX) {
#if wxUSE_DRAG_AND_DROP && (wxVERSION_NUMBER != 3104) // 3.1.4 is crashing in Drop, use 3.1.5 instead
    auto mainFrame = wxGetApp().GetPasswordSafeFrame();
    wxASSERT(mainFrame && mainFrame->m_tree);
    if (mainFrame->IsEntryMarked() && PWSprefs::GetInstance()->GetPref(PWSprefs::MultipleInstances)) {
      mainFrame->m_tree->OnDrag(event);
    }
#endif
    return;
  }

  wxString text = GetText(toolId);
  if (!text.IsEmpty())
  {
    wxTextDataObjectEx dataObj(text);
    wxDropSource source(dataObj, this);
    switch (source.DoDragDrop())
    {
    case wxDragError:
      wxLogDebug(_("Error dragging"));
      break;
    case wxDragNone:
      wxLogDebug(_("Nothing happened dragging"));
      break;
    case wxDragCopy:
      wxLogDebug(_("Copied successfully"));
      break;
    case wxDragMove:
      wxLogDebug(_("Moved successfully"));
      break;
    case wxDragCancel:
      wxLogDebug(_("Dragging cancelled"));
      break;
    default:
      wxLogDebug(_("Unexpected result dragging"));
      break;
    }
  }
}

void DragBarCtrl::OnUpdateUI(wxUpdateUIEvent& event)
{
  const auto mainFrame = wxGetApp().GetPasswordSafeFrame();
  wxASSERT(mainFrame && mainFrame->m_tree);

  const auto selection         = mainFrame->GetSelectedEntry();
  const auto isTreeView        = mainFrame->IsTreeView();
  const auto hasGroupSelection = mainFrame->m_tree->IsGroupSelected();
  const auto hasItemSelection  = selection != nullptr;
  const auto hasAnySelection   = hasItemSelection || hasGroupSelection;
  const auto hasGroup          = hasItemSelection && (selection->IsGroupSet());
  const auto hasTitle          = hasItemSelection && (selection->IsTitleSet());
  const auto hasUser           = hasItemSelection && (selection->IsUserSet());
  const auto hasPassword       = hasItemSelection && (selection->IsPasswordSet());
  const auto hasNotes          = hasItemSelection && (selection->IsNotesSet());
  const auto hasURL            = hasItemSelection && (selection->IsURLSet());
  const auto hasEmail          = hasItemSelection && (selection->IsEmailSet());

  switch (event.GetId()) {
    case DRAGBAR_TOOLID_BASE:     // the 'Group' tool item
      event.Enable(hasGroup);
      break;
    case DRAGBAR_TOOLID_BASE + 1: // the 'Title' tool item
      event.Enable(hasTitle);
      break;
    case DRAGBAR_TOOLID_BASE + 2: // the 'User' tool item
      event.Enable(hasUser);
      break;
    case DRAGBAR_TOOLID_BASE + 3: // the 'Password' tool item
      event.Enable(hasPassword);
      break;
    case DRAGBAR_TOOLID_BASE + 4: // the 'Notes' tool item
      event.Enable(hasNotes);
      break;
    case DRAGBAR_TOOLID_BASE + 5: // the 'URL' tool item
      event.Enable(hasURL);
      break;
    case DRAGBAR_TOOLID_BASE + 6: // the 'Email' tool item
      event.Enable(hasEmail);
      break;
    case DRAGBAR_TOOLID_BASE + 7: // the 'DnD' tool item
      event.Enable(isTreeView && hasAnySelection);
      break;
    default:
      ;
  }
}
