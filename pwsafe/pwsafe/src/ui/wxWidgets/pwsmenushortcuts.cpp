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
#include "../../core/PWSprefs.h"

#include <wx/stockitem.h>

#include <wx/grid.h>
#include <algorithm>

#include <iterator>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// MenuItemData
///////////////

MenuItemData::MenuItemData(wxMenuItem* menuItem, 
                           const wxString& label) : m_item(menuItem), 
                                                    m_label(label)
{
  if (!m_origShortcut.FromString(m_item->GetItemLabel()))
    if (wxIsStockID(m_item->GetId()))
      m_origShortcut = wxGetStockAccelerator(m_item->GetId());
}

// used for converting to/from ds used in core::PWSprefs
struct ModifierMap {
  int wxmod;
  int prefsmod;
} static g_modmap[] = {
    {wxACCEL_ALT,    PWS_HOTKEYF_ALT            },
    {wxACCEL_CTRL,   PWS_HOTKEYF_CONTROL        },
    {wxACCEL_SHIFT,  PWS_HOTKEYF_SHIFT          },
#if defined(__WXMAC__) || defined(__WXCOCOA__)
    {wxACCEL_CMD,    PWS_HOTKEYF_CMD            },
#endif

};

st_prefShortcut MenuItemData::ToPrefShortcut() const
{
  const wxAcceleratorEntry ae( GetEffectiveShortcut() );

  st_prefShortcut sc;

  sc.id = m_item->GetId();
  sc.siVirtKey = ae.IsOk()? ae.GetKeyCode(): 0;
  sc.cModifier = 0;
  for (size_t idx = 0; idx < WXSIZEOF(g_modmap); ++idx)
    if ((ae.GetFlags() & g_modmap[idx].wxmod) != 0)
      sc.cModifier |= g_modmap[idx].prefsmod;

  return sc;
}

void MenuItemData::SetUserShortcut(const st_prefShortcut& prefAccel, bool setdirty /* = false */)
{
  int flags = 0;
  for (size_t idx = 0; idx < WXSIZEOF(g_modmap); ++idx)
    if ((prefAccel.cModifier & g_modmap[idx].prefsmod) != 0)
      flags |= g_modmap[idx].wxmod;

  SetUserShortcut( wxAcceleratorEntry(flags, prefAccel.siVirtKey, prefAccel.id), setdirty );
}

void MenuItemData::SetUserShortcut(const wxAcceleratorEntry& userAccel, bool setdirty /* = true */)
{
  if (userAccel.IsOk()) {
    m_userShortcut = userAccel;
    m_status.setchanged();
    if (setdirty)
      m_status.setdirty();
  }
  else if (m_origShortcut.IsOk() || m_userShortcut.IsOk()) {
    // we have a shortcut, but the user doesn't want one for this menu item 
    m_userShortcut = wxAcceleratorEntry();
    m_status.setdeleted();
    if (setdirty)
      m_status.setdirty();
  }
}

bool MenuItemData::IsDirty() const {
  return m_status.isdirty();
}

void MenuItemData::ApplyEffectiveShortcut()
{
  if (HasEffectiveShortcut()) {
    wxAcceleratorEntry ae(GetEffectiveShortcut());
    m_item->SetAccel(&ae);
  }
  else {
    //remove the accelerator
    wxString label = m_item->GetItemLabelText();
    if (label.IsEmpty()) {
      if (wxIsStockID(m_item->GetId()))
        label = wxGetStockLabel(m_item->GetId(), wxSTOCK_WITH_MNEMONIC);
      else {
        wxFAIL_MSG(wxT("empty menu item label, and not a stock item either"));
      }
    }
    m_item->SetItemLabel(label);
  }
}

/*
 * There's an effective shortcut if there's a user shortcut
 * or if there's was an original shortcut which has not been deleted
 */
bool MenuItemData::HasEffectiveShortcut() const
{
  return m_userShortcut.IsOk() || (m_origShortcut.IsOk() && !m_status.isdeleted());
}

/*
 * Save it if there's a user shortcut or if the original shortcut
 * has been deleted
 */
bool MenuItemData::ShouldSave() const
{
  return IsDirty() || (m_userShortcut.IsOk() || (m_status.isdeleted() && m_origShortcut.IsOk()));
}

bool MenuItemData::IsCustom() const
{
  return !m_status.isorig();
}

wxAcceleratorEntry MenuItemData::GetEffectiveShortcut() const
{
  static wxAcceleratorEntry nullAccel;

  if (m_userShortcut.IsOk())
    return m_userShortcut;
  else if (m_status.isdeleted())
    return wxAcceleratorEntry(0, 0, m_item->GetId());
  else
    return m_origShortcut;
}

/*
 * Remove the user shortcut and "shortcut deleted" status
 * if this item was dirty due to previous changes, it now becomes
 * clean.  If it was already clean (just read from config, say),
 * this makes it dirty and needs to be saved/"unsaved"
 */
void MenuItemData::Reset()
{
  if (m_userShortcut.IsOk() || m_status.isdeleted()) {
    m_userShortcut = wxAcceleratorEntry();
    m_status.setorig();
    m_status.flipdirty();
  }
}

void MenuItemData::ClearDirtyFlag()
{
  m_status.cleardirty();
}

/////////////////////////////////////////////////////////////////////////////////
// PWSMenuShortcuts
///////////////////
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
      *cont_itr++ = MenuItemData(item, longLabel);
    }
  }
}

PWSMenuShortcuts::PWSMenuShortcuts(wxMenuBar* menuBar)
{
  /*
  wxFrame* frame = wxDynamicCast(wxGetApp().GetTopWindow(), wxFrame);
  wxCHECK_RET(frame, wxT("Could not get frame window from wxApp"));
  wxMenuBar* menuBar = frame->GetMenuBar();
  wxCHECK_RET(menuBar, wxT("Could not get menu bar from frame"));
  */
  std::back_insert_iterator<MenuItemDataArray> inserter = std::back_inserter(m_midata);
  for( unsigned menuIndex = 0; menuIndex < menuBar->GetMenuCount(); ++menuIndex) {
    GetShortcutsFromMenu(menuBar->GetMenu(menuIndex), inserter, menuBar->GetMenuLabelText(menuIndex));
  }
}

PWSMenuShortcuts::~PWSMenuShortcuts()
{
}

PWSMenuShortcuts* g_pShortcutsManager = 0;

PWSMenuShortcuts* PWSMenuShortcuts::CreateShortcutsManager(wxMenuBar* menubar)
{
  wxCHECK_MSG(g_pShortcutsManager == 0, g_pShortcutsManager, wxT("Shortcuts manager being created multiple times"));
  g_pShortcutsManager = new PWSMenuShortcuts(menubar);
  return g_pShortcutsManager;
}

PWSMenuShortcuts* PWSMenuShortcuts::GetShortcutsManager()
{
  wxASSERT_MSG(g_pShortcutsManager, wxT("Shortcuts manager being used before creation"));;
  return g_pShortcutsManager;
}

void PWSMenuShortcuts::DestroyShortcutsManager()
{
  delete g_pShortcutsManager;
}

wxString PWSMenuShortcuts::MenuLabelAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), wxEmptyString, wxT("Index for menu label exceeds number of menu items retrieved"));
  return m_midata[index].GetLabel();
}

wxAcceleratorEntry PWSMenuShortcuts::EffectiveShortcutAt(size_t index) const
{
  static wxAcceleratorEntry nullAccel;
  wxCHECK_MSG(index < Count(), nullAccel, wxT("Index for old shortcut exceeds number of menu items retrieved"));
  return m_midata[index].GetEffectiveShortcut();
}

wxMenuItem* PWSMenuShortcuts::MenuItemAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), 0, wxT("Index for menuitem exceeds number of menu items retrieved"));
  return m_midata[index].GetMenuItem();
}

void PWSMenuShortcuts::ChangeShortcut(size_t idx, const wxAcceleratorEntry& newEntry)
{
  wxCHECK_RET(idx < Count(), wxT("Index for new shortcut exceeds number of menu items retrieved"));

  m_midata[idx].SetUserShortcut(newEntry);
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


bool PWSMenuShortcuts::IsDirty() const
{
  return std::find_if(m_midata.begin(), m_midata.end(), std::mem_fun_ref(&MenuItemData::IsDirty)) != m_midata.end();
}

struct ApplyEditedShortcuts {
  void operator()(MenuItemData& mi) const {
    if (mi.IsDirty())
      mi.ApplyEffectiveShortcut();
  }
};

// Set the shortcuts of all menuitems to new ones, if modified
void PWSMenuShortcuts::ApplyEditedShortcuts()
{
  std::for_each(m_midata.begin(), m_midata.end(), ::ApplyEditedShortcuts());
}

struct CompShortcuts: public std::binary_function<st_prefShortcut, MenuItemData, bool>
{
  bool operator()(const st_prefShortcut& a, const MenuItemData& b) const {
    return a.id == unsigned(b.GetMenuItem()->GetId());
  }
};

void PWSMenuShortcuts::ReadApplyUserShortcuts()
{
  typedef std::vector<st_prefShortcut> userShortcut_t;
  const std::vector<st_prefShortcut>& userShortcuts = PWSprefs::GetInstance()->GetPrefShortcuts();
  for (userShortcut_t::const_iterator usrItr = userShortcuts.begin(); usrItr != userShortcuts.end(); ++usrItr) {
    MenuItemDataArray::iterator itr = std::find_if(m_midata.begin(), m_midata.end(),
                            std::bind1st(CompShortcuts(), *usrItr));
    if (itr != m_midata.end()) {
      itr->SetUserShortcut(*usrItr);
      itr->ApplyEffectiveShortcut();
    }
  }
}

void PWSMenuShortcuts::SaveUserShortcuts()
{
  typedef std::vector<st_prefShortcut> userShortcut_t;
  std::vector<st_prefShortcut> userShortcuts;
  for (MenuItemDataArray::iterator itr = m_midata.begin(); itr != m_midata.end(); ++itr) {
    if (itr->ShouldSave()) {
      userShortcuts.push_back(itr->ToPrefShortcut());
      wxLogDebug(wxT("Saving shortcut for menu %s [%d]\n"), itr->GetLabel().c_str(),
                                                               itr->GetMenuItem()->GetId());
    }
  }
  PWSprefs::GetInstance()->SetPrefShortcuts(userShortcuts);
  PWSprefs::GetInstance()->SaveShortcuts();
  std::for_each(m_midata.begin(), m_midata.end(), std::mem_fun_ref(&MenuItemData::ClearDirtyFlag));
}

void PWSMenuShortcuts::ResetShortcuts()
{
  std::for_each(m_midata.begin(), m_midata.end(), std::mem_fun_ref(&MenuItemData::Reset));
}

bool PWSMenuShortcuts::IsShortcutCustomizedAt(size_t idx) const
{
  wxCHECK_MSG(idx < Count(), false, wxT("Invalid index for shortcut customization check"));
  return m_midata[idx].IsCustom();
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
      wxAcceleratorEntry newAccel;
      if (newAccel.FromString(wxT('\t')+newStr)) {
        const wxAcceleratorEntry& oldAccel = m_shortcuts.EffectiveShortcutAt(row);
        if (newAccel.GetFlags() != oldAccel.GetFlags() || newAccel.GetKeyCode() != oldAccel.GetKeyCode()) {
          //set the remaining fields in the new accelerator entry, so they get written to pwsafe.cfg
          wxMenuItem* menuitem = m_shortcuts.MenuItemAt(row);
          if (menuitem) {
            newAccel.Set(newAccel.GetFlags(), newAccel.GetKeyCode(), menuitem->GetId(), menuitem);
            m_shortcuts.ChangeShortcut(row, newAccel);
          }
        }
      }
      else {
        success = false;
      }
    }
    else {
      // remove the current user shortcut, if there
      m_shortcuts.ChangeShortcut(row, wxAcceleratorEntry());
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
    const wxAcceleratorEntry& ae = m_shortcuts.EffectiveShortcutAt(row);
    if (ae.IsOk())
      grid->SetCellValue(row, COL_SHORTCUT_KEY, ae.ToString());
    grid->SetCellValue(row, COL_MENU_ITEM, m_shortcuts.MenuLabelAt(row));
    if (m_shortcuts.IsShortcutCustomizedAt(row)) {
      wxFont cellFont = grid->GetCellFont(row, COL_MENU_ITEM);
      cellFont.SetStyle(cellFont.GetStyle()|wxFONTSTYLE_SLANT);
      grid->SetCellFont(row, COL_MENU_ITEM, cellFont);
    }
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
      if (!wxAcceleratorEntry().FromString(wxT('\t')+newStr)) {
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


