/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

enum
{
  ID_DRAGBAR_GROUP = wxID_HIGHEST + 1,
  ID_DRAGBAR_TITLE,
  ID_DRAGBAR_USER,
  ID_DRAGBAR_PASSWORD,
  ID_DRAGBAR_NOTES,
  ID_DRAGBAR_URL,
  ID_DRAGBAR_EMAIL,
  ID_DRAGBAR_DND
};

#define PWS_DRAGBAR_BITMAPS(t)            \
  wxCONCAT(t, _xpm),                      \
  wxCONCAT(t, X_xpm),                     \
  wxCONCAT(wxCONCAT(classic_, t), _xpm),  \
  wxCONCAT(wxCONCAT(classic_, t), X_xpm)

struct DragbarToolInfo {
  const wxWindowID id;
  const wxString name;
  const char* const* const bitmap;
  const char* const* const bitmap_disabled;
  const char* const* const classic_bitmap;
  const char* const* const classic_bitmap_disabled;
  const CItemData::FieldType field_type;

  DragbarToolInfo() :
    id(0), name(wxEmptyString),
    bitmap(nullptr), bitmap_disabled(nullptr),
    classic_bitmap(nullptr), classic_bitmap_disabled(nullptr),
    field_type(CItem::FieldType::UNKNOWNFIELDS) {}

  DragbarToolInfo(
    wxWindowID id, const wxString &name,
    const char* const* bitmap, const char* const* bitmap_disabled,
    const char* const* classic_bitmap, const char* const* classic_bitmap_disabled,
    CItemData::FieldType field_type
  ) :
    id(id), name(name),
    bitmap(bitmap), bitmap_disabled(bitmap_disabled),
    classic_bitmap(classic_bitmap), classic_bitmap_disabled(classic_bitmap_disabled),
    field_type(field_type) {}

  bool UseNewToolbarStyle() const
  {
    return PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar);
  }

  /**
   * Provides the bitmap that represents an enabled toolbar item in the new or classic style, depending on user preferences.
   * @return toolbar item bitmap
   */
  wxBitmap GetBitmapForEnabledButton() const
  {
    return UseNewToolbarStyle() ? bitmap : classic_bitmap;
  };

  /**
   * Provides the bitmap that represents an disabled toolbar item in the new or classic style, depending on user preferences.
   * @return toolbar item bitmap
   */
  wxBitmap GetBitmapForDisabledButton() const
  {
    return UseNewToolbarStyle() ? bitmap_disabled : classic_bitmap_disabled;
  }

  /**
   * Provides the toolbar item tooltip.
   * @return the tooltip string
   */
  wxString GetTooltipForButton() const
  {
    return (id == ID_DRAGBAR_DND) ?
      _("Drag this image onto another window to paste the selected element or tree.") :
      wxString::Format(_("Drag this image onto another window to paste the '%s' field."), _(name));
  }
};

std::vector<DragbarToolInfo> DragbarToolInfos =
  {
    { ID_DRAGBAR_GROUP,     _("Group"),     PWS_DRAGBAR_BITMAPS(Group),     CItemData::FieldType::GROUP           },
    { ID_DRAGBAR_TITLE,     _("Title"),     PWS_DRAGBAR_BITMAPS(Title),     CItemData::FieldType::TITLE           },
    { ID_DRAGBAR_USER,      _("User"),      PWS_DRAGBAR_BITMAPS(User),      CItemData::FieldType::USER            },
    { ID_DRAGBAR_PASSWORD,  _("Password"),  PWS_DRAGBAR_BITMAPS(Password),  CItemData::FieldType::PASSWORD        },
    { ID_DRAGBAR_NOTES,     _("Notes"),     PWS_DRAGBAR_BITMAPS(Notes),     CItemData::FieldType::NOTES           },
    { ID_DRAGBAR_URL,       _("Url"),       PWS_DRAGBAR_BITMAPS(URL),       CItemData::FieldType::URL             },
    { ID_DRAGBAR_EMAIL,     _("Email"),     PWS_DRAGBAR_BITMAPS(Email),     CItemData::FieldType::EMAIL           },
    { ID_DRAGBAR_DND,       _("Dnd"),       PWS_DRAGBAR_BITMAPS(Dnd),       CItemData::FieldType::UNKNOWNFIELDS   }
  };

DragBarCtrl::DragBarCtrl(wxWindow *parent, wxWindowID id, const wxPoint &position, const wxSize &size, long style) : wxAuiToolBar(parent, id, position, size, style)
{
  CreateToolbar();
  CalculateToolsWidth();
  Bind(wxEVT_AUITOOLBAR_BEGIN_DRAG, &DragBarCtrl::OnDrag, this);
  Bind(wxEVT_UPDATE_UI, &DragBarCtrl::OnUpdateUI, this, ID_DRAGBAR_GROUP, ID_DRAGBAR_DND);
}

DragBarCtrl::~DragBarCtrl()
{
  Unbind(wxEVT_UPDATE_UI, &DragBarCtrl::OnUpdateUI, this, ID_DRAGBAR_GROUP, ID_DRAGBAR_DND);
  Unbind(wxEVT_AUITOOLBAR_BEGIN_DRAG, &DragBarCtrl::OnDrag, this);
}

/**
 * Creates the toolbar items. Existing tools will be deleted beforehand.
 */
void DragBarCtrl::CreateToolbar()
{
  ClearTools();

  for (const auto & toolInfo : DragbarToolInfos)
  {
    AddTool(
      toolInfo.id,
      toolInfo.GetBitmapForEnabledButton(),
      toolInfo.GetBitmapForDisabledButton(),
      false, nullptr,
      toolInfo.GetTooltipForButton()
    );
  }

  Realize();
}

void DragBarCtrl::CalculateToolsWidth()
{
  size_t width = 0;

  for (const auto & toolInfo : DragbarToolInfos)
  {
    auto tool = FindTool(toolInfo.id);
    if (tool) {
      width += (tool->GetMinSize()).GetWidth();
    }
  }

  SetMinSize(wxSize(static_cast<int>(width), -1));
}

/**
 * Updates the bitmaps of the tool elements after the user changed the icon style.
 */
void DragBarCtrl::UpdateBitmaps()
{
  if (HasTools()) {
    for (const auto & toolInfo : DragbarToolInfos)
    {
      auto tool = FindTool(toolInfo.id);
      if (tool) {
        tool->SetBitmap(toolInfo.GetBitmapForEnabledButton());
        tool->SetDisabledBitmap(toolInfo.GetBitmapForDisabledButton());
      }
    }

    Realize();
  }
}

/**
 * Updates the tooltips of the tool elements after the user changed the language.
 */
void DragBarCtrl::UpdateTooltips()
{
  if (HasTools()) {
    for (const auto & toolInfo : DragbarToolInfos)
    {
      auto tool = FindTool(toolInfo.id);
      if (tool) {
        tool->SetShortHelp(toolInfo.GetTooltipForButton());
      }
    }

    Realize();
  }
}

/**
 * Provides the string of the items data field for the drag and drop procedure.
 * @param toolId the toolbar item's id.
 * @return the items data as string
 */
wxString DragBarCtrl::GetText(int toolId) const
{
  const CItemData *pci(nullptr), *pbci(nullptr);
  auto mainFrame = wxGetApp().GetPasswordSafeFrame();
  wxASSERT(mainFrame);
  pci = mainFrame->GetSelectedEntry();
  pbci = mainFrame->GetBaseEntry(pci);

  if (!pci) {
    return wxEmptyString;
  }

  for (const auto & toolInfo : DragbarToolInfos) {
    if (toolId == toolInfo.id) {
      return towxstring(pci->GetEffectiveFieldValue(toolInfo.field_type, pbci));
    }
  }

  return wxEmptyString;
}

/**
 * The event handler when dragging of a tool item starts.
 * @param event the <code>wxAuiToolBarEvent</code> event
 */
void DragBarCtrl::OnDrag(wxAuiToolBarEvent& event)
{
  auto toolId = event.GetToolId();

  if (toolId == ID_DRAGBAR_DND) {
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

/**
 * Updates continuously the toolbar items state (enabled/disabled).
 * @param event the <code>wxUpdateUIEvent</code> event
 */
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
    case ID_DRAGBAR_GROUP:
      event.Enable(hasGroup);
      break;
    case ID_DRAGBAR_TITLE:
      event.Enable(hasTitle);
      break;
    case ID_DRAGBAR_USER:
      event.Enable(hasUser);
      break;
    case ID_DRAGBAR_PASSWORD:
      event.Enable(hasPassword);
      break;
    case ID_DRAGBAR_NOTES:
      event.Enable(hasNotes);
      break;
    case ID_DRAGBAR_URL:
      event.Enable(hasURL);
      break;
    case ID_DRAGBAR_EMAIL:
      event.Enable(hasEmail);
      break;
    case ID_DRAGBAR_DND:
      event.Enable(isTreeView && hasAnySelection);
      break;
    default:
      ;
  }
}
