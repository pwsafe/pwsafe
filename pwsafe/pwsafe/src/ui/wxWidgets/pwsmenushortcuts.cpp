/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "pwsmenushortcuts.h"

#include "./RecentDBList.h"
#include "./pwsafeapp.h"
#include <wx/stockitem.h>

#include <wx/grid.h>
#include <algorithm>

#include <iterator>

template <class Iter>
void GetShortcutsFromMenu(wxMenu* menu, Iter cont_itr, const wxString& menuLabel)
{
  wxMenuItemList& items = menu->GetMenuItems();
  const CRecentDBList& rdb = wxGetApp().recentDatabases();
  for (wxMenuItemList::iterator itr = items.begin(); itr != items.end(); ++itr) {
    wxMenuItem* item = *itr;
    if (item->IsSeparator())
      continue;
    //skip Recently-Used items from file menu
    if (item->GetId() >= rdb.GetBaseId() && item->GetId() <= (rdb.GetBaseId() + rdb.GetMaxFiles()-1))
      continue;
    wxString menuItemLabel = item->GetItemLabelText();
    if (menuItemLabel.IsEmpty()) {
      wxASSERT(wxIsStockID(item->GetId()));
      menuItemLabel = wxGetStockLabel(item->GetId(), wxSTOCK_NOFLAGS);
    }
    const wxString longLabel = menuLabel + wxT(" \xbb ") + menuItemLabel;
    if (item->IsSubMenu()) {
      GetShortcutsFromMenu(item->GetSubMenu(), cont_itr, longLabel);
    }
    else {
      MenuItemData mid = {0};

      mid.label = longLabel;
      mid.item = item;

      wxAcceleratorEntry* accel = item->GetAccel();
      if (accel) {
        mid.oldShortcut = *accel;
      }
      else if (wxIsStockID(item->GetId())) {
        mid.oldShortcut = wxGetStockAccelerator(item->GetId());
      }
      *cont_itr++ = mid;
    }
  }
}

PWSMenuShortcuts::PWSMenuShortcuts()
{
  wxFrame* frame = wxDynamicCast(wxGetApp().GetTopWindow(), wxFrame);
  wxCHECK_RET(frame, wxT("Could not get frame window from wxApp"));
  wxMenuBar* menuBar = frame->GetMenuBar();
  wxCHECK_RET(menuBar, wxT("Could not get menu bar from frame"));

  std::back_insert_iterator<MenuItemDataArray> inserter = std::back_inserter(m_midata);
  for( unsigned menuIndex = 0; menuIndex < menuBar->GetMenuCount(); ++menuIndex) {
    GetShortcutsFromMenu(menuBar->GetMenu(menuIndex), inserter, menuBar->GetMenuLabelText(menuIndex));
  }
}

PWSMenuShortcuts::~PWSMenuShortcuts()
{
}

wxString PWSMenuShortcuts::MenuLabelAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), wxEmptyString, wxT("Index for menu label exceeds number of menu items retrieved"));
  return m_midata[index].label;
}

wxAcceleratorEntry PWSMenuShortcuts::OldShortcutAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), wxAcceleratorEntry(), wxT("Index for old shortcut exceeds number of menu items retrieved"));
  return m_midata[index].oldShortcut;
}

wxAcceleratorEntry PWSMenuShortcuts::NewShortcutAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), wxAcceleratorEntry(), wxT("Index for new shortcut exceeds number of menu items retrieved"));
  return m_midata[index].newShortcut;
}

void PWSMenuShortcuts::ChangeShortcut(size_t idx, const wxAcceleratorEntry* newEntry)
{
  wxCHECK_RET(idx < Count(), wxT("Index for new shortcut exceeds number of menu items retrieved"));

  m_midata[idx].newShortcut = *newEntry;
}

int ModifiersToAccelFlags(int mods)
{
  struct mod_accel_map_t {
    int modifier;
    int accelerator;
  } mod_accel_map[] = {
        {wxMOD_ALT,          wxACCEL_ALT   },
        {wxMOD_CONTROL,      wxACCEL_CTRL  },
        {wxMOD_SHIFT,        wxACCEL_SHIFT },
#if defined(__WXMAC__) || defined(__WXCOCOA__)
        {wxMOD_CMD,          wxACCEL_CMD   },
#endif
  };

  int flags = wxACCEL_NORMAL; //no modifiers
  for (size_t idx = 0; idx < WXSIZEOF(mod_accel_map); ++idx) {
    if (mods & mod_accel_map[idx].modifier)
      flags |= mod_accel_map[idx].accelerator;
  }
  return flags;
}

wxAcceleratorEntry* PWSMenuShortcuts::CreateShortcut(const wxKeyEvent& evt)
{
  return new wxAcceleratorEntry(ModifiersToAccelFlags(evt.GetModifiers()), evt.GetKeyCode(), 0);
}

wxAcceleratorEntry* PWSMenuShortcuts::CreateShortcut(const wxString& str)
{
  //The parser expects a full menuitem string, with menu text and accel separated by TAB
  return wxAcceleratorEntry::Create(wxT('\t') + str);
}

struct ShortcutChanged{
  bool operator()(const MenuItemData& m) const {
    return m.newShortcut.IsOk() && m.newShortcut != m.oldShortcut;
  }
};

bool PWSMenuShortcuts::IsDirty() const
{
  return std::find_if(m_midata.begin(), m_midata.end(), ShortcutChanged()) != m_midata.end();
}

struct ApplyNewShortcut
{
  void operator()(MenuItemData& m) const {
    if (m.newShortcut.IsOk() && m.newShortcut != m.oldShortcut)
      m.item->SetAccel(&m.newShortcut);
  }
};

// Set the shortcuts of all menuitems to new ones, if modified
void PWSMenuShortcuts::ApplyAll()
{
  std::for_each(m_midata.begin(), m_midata.end(), ApplyNewShortcut());
}


//////////////////////////////////////////////////////////////////
// ShortcutsGridValidator
//

bool ShortcutsGridValidator::TransferFromWindow()
{
  bool success = true;

  wxGrid* grid = wxDynamicCast(GetWindow(), wxGrid);
  wxCHECK_MSG(grid, false, wxT("ShortcutsGridValidator attached to a non-wxGrid derived window?"));

  for( unsigned row = 0; row < m_shortcuts.Count(); ++row) {
    wxString newStr = grid->GetCellValue(row, COL_SHORTCUT_KEY);
    if (!newStr.IsEmpty()) {
      std::auto_ptr<wxAcceleratorEntry> newAccel(wxAcceleratorEntry::Create(wxT('\t')+newStr));
      if (newAccel.get() && newAccel->IsOk()) {
        if (*newAccel != m_shortcuts.OldShortcutAt(row)) {
          m_shortcuts.ChangeShortcut(row, newAccel.get());
        }
      }
      else {
        success = false;
      }
    }
  }
  return success;
}

bool ShortcutsGridValidator::TransferToWindow()
{
  wxGrid* grid = wxDynamicCast(GetWindow(), wxGrid);
  wxCHECK_MSG(grid, false, wxT("ShortcutsGridValidator attached to a non-wxGrid derived window?"));

  wxFrame* frame = wxDynamicCast(wxGetApp().GetTopWindow(), wxFrame);
  wxCHECK_MSG(frame, false, wxT("Could not get frame window from wxApp"));
  wxMenuBar* menuBar = frame->GetMenuBar();
  wxCHECK_MSG(menuBar, false, wxT("Could not get menu bar from frame"));

  if (m_shortcuts.Count() > unsigned(grid->GetNumberRows())) {
    grid->AppendRows(m_shortcuts.Count() - grid->GetNumberRows());
  }
  else if (m_shortcuts.Count() < unsigned(grid->GetNumberRows())) {
    grid->DeleteRows(grid->GetNumberRows() - m_shortcuts.Count());
  }

  for( unsigned row = 0; row < m_shortcuts.Count(); ++row) {
    if (m_shortcuts.OldShortcutAt(row).IsOk())
      grid->SetCellValue(row, COL_SHORTCUT_KEY, m_shortcuts.OldShortcutAt(row).ToString());
    grid->SetCellValue(row, COL_MENU_ITEM, m_shortcuts.MenuLabelAt(row));
  }
  grid->AutoSize();
  return true;
}

bool ShortcutsGridValidator::Validate(wxWindow* parent)
{
  wxGrid* grid = wxDynamicCast(GetWindow(), wxGrid);
  wxCHECK_MSG(grid, false, wxT("ShortcutsGridValidator attached to a non-wxGrid derived window?"));

  for( unsigned row = 0; row < m_shortcuts.Count(); ++row) {
    wxString newStr = grid->GetCellValue(row, COL_SHORTCUT_KEY);
    if (!newStr.IsEmpty()) {
      std::auto_ptr<wxAcceleratorEntry> newAccel(wxAcceleratorEntry::Create(wxT('\t')+newStr));
      if (newAccel.get() == 0 || !newAccel->IsOk()) {
        wxString msg(wxT("Shortcut # "));
        msg << (row + 1) << wxT(" [") << newStr << wxT("] is not a valid shortcut");
        wxMessageBox(msg, wxT("Invalid shortcut"), wxOK | wxICON_ERROR, parent);
        grid->SetGridCursor(row, COL_SHORTCUT_KEY);
        return false;
      }
    }
  }
  return true;
}


