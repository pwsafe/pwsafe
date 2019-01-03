/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include <functional>
#include <iterator>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

static wxAcceleratorEntry nullAccel;

// The in-built comparison operator also compares the menu-id, which we don't always have
inline bool IsSameShortcut(const wxAcceleratorEntry& a, const wxAcceleratorEntry& b)
{
  return a.GetKeyCode() == b.GetKeyCode() && a.GetFlags() == b.GetFlags();
}

inline bool IsNull(const wxAcceleratorEntry& acc)
{
  return acc.GetKeyCode() == 0;
}

inline bool IsNotNull(const wxAcceleratorEntry& acc)
{
  return acc.GetKeyCode() != 0;
}

////////////////////////////////////////////////////////////////////////////////
// MenuItemData
///////////////

MenuItemData::MenuItemData(wxMenuItem* menuItem,
                           const wxString& label) : m_item(menuItem),
                                                    m_menuId(menuItem->GetId()),
                                                    m_label(label)
{
  if (!m_origShortcut.FromString(m_item->GetItemLabel()))
    if (wxIsStockID(m_menuId))
      m_origShortcut = wxGetStockAccelerator(m_menuId);
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

  sc.id = m_menuId;
  wxASSERT_MSG(sc.id != 0, wxT("Trying to save shortcut with nullptr menu item id"));
  sc.siVirtKey = ae.GetKeyCode();
  sc.cPWSModifier = 0;
  for (size_t idx = 0; idx < WXSIZEOF(g_modmap); ++idx)
    if ((ae.GetFlags() & g_modmap[idx].wxmod) != 0)
      sc.cPWSModifier |= g_modmap[idx].prefsmod;

  return sc;
}

void MenuItemData::SetUserShortcut(const st_prefShortcut& prefAccel, bool setdirty /* = false */)
{
  int flags = 0;
  for (size_t idx = 0; idx < WXSIZEOF(g_modmap); ++idx)
    if ((prefAccel.cPWSModifier & g_modmap[idx].prefsmod) != 0)
      flags |= g_modmap[idx].wxmod;

  SetUserShortcut( wxAcceleratorEntry(flags, prefAccel.siVirtKey, prefAccel.id), setdirty );
}

/*
 * The only way to change the shortcut.  Specify a nullptr accel to remove the user-specified
 * shortcut, if any, or the original shortcut.
 */
void MenuItemData::SetUserShortcut(const wxAcceleratorEntry& userAccel, bool setdirty /* = true */)
{
  bool modified = false;
  if (IsNotNull(userAccel)) {
    // if the user specified the original shortcut, treat it as unmodified shortcut
    if (IsSameShortcut(userAccel, m_origShortcut)) {
      m_userShortcut = wxAcceleratorEntry();
      m_status.setorig();
      modified = true;
    }
    else if (!IsSameShortcut(m_userShortcut, userAccel)) {
      m_userShortcut = userAccel;
      m_status.setchanged();
      modified = true;
    }
  }
  else if (IsNotNull(m_userShortcut)) {
    // Remove the user shortcut
    m_userShortcut = wxAcceleratorEntry();
    m_status.setorig();
    modified = true;
  }
  else if (IsNotNull(m_origShortcut)) {
    //just mark the original shortcut as deleted
    m_status.setdeleted();
    modified = true;
  }
  if (setdirty && modified)
    m_status.setdirty();
}

bool MenuItemData::IsDirty() const {
  return m_status.isdirty();
}

/*
 * The only reliable way to change the accelerator, at least for 2.8.11, is to
 * set a new and complete label, with full mnemonics + accelerator.   Don't
 * use wxMenuItem::SetAccel(), or do * SetItemLabel(SomeManipulation(GetItemLabelText()))).
 * You will end up with underscores in place where the '&' mnemonic is, and they would multiply
 * each time you call SetItemLabel
 */
void MenuItemData::ApplyEffectiveShortcut()
{
  wxString currentLabel = m_item->GetItemLabel();
  if (currentLabel.IsEmpty()) {
    if (wxIsStockID(m_menuId))
      currentLabel = wxGetStockLabel(m_menuId, wxSTOCK_WITH_MNEMONIC);
    else {
      wxFAIL_MSG(wxT("empty menu item label, and not a stock item either"));
    }
  }

  if (HasEffectiveShortcut()) {
    wxAcceleratorEntry ae(GetEffectiveShortcut());
    m_item->SetItemLabel(currentLabel.BeforeFirst(wxT('\t')) + wxT('\t') + ae.ToString());
  }
  else {
    m_item->SetItemLabel(currentLabel.BeforeFirst(wxT('\t')));
  }
}

/*
 * There's an effective shortcut if there's a user shortcut
 * or if there's was an original shortcut which has not been deleted
 */
bool MenuItemData::HasEffectiveShortcut() const
{
  return IsNotNull(m_userShortcut) || (IsNotNull(m_origShortcut) && !m_status.isdeleted());
}

/*
 * Save it if there's a user shortcut or if the original shortcut
 * has been deleted
 */
bool MenuItemData::ShouldSave() const
{
  return IsNotNull(m_userShortcut) || (m_status.isdeleted() && IsNotNull(m_origShortcut));
}

bool MenuItemData::IsCustom() const
{
  return !m_status.isorig();
}

wxAcceleratorEntry MenuItemData::GetEffectiveShortcut() const
{
  if (m_status.ischanged()) {
    wxASSERT_MSG(IsNotNull(m_userShortcut), wxT("Changed shortcut has invalid user-shortcut"));
    return m_userShortcut;
  }
  else if (m_status.isdeleted()) {
    wxASSERT_MSG(IsNotNull(m_origShortcut), wxT("Deleted shortcuts should have valid original shortcuts"));
    wxASSERT_MSG(IsNull(m_userShortcut), wxT("Deleted shortcuts should invalidate user-shortcuts"));
    return wxAcceleratorEntry(0, 0, m_menuId);
  }
  else {
    wxASSERT_MSG(m_status.isorig(), wxT("Unexpected shortcut status"));
    wxASSERT_MSG(IsNull(m_userShortcut), wxT("Shortcuts not changed or deleted should have invalid user-shortcuts"));
    return m_origShortcut;
  }
}

/*
 * Remove the user shortcut and "shortcut deleted" status
 * if this item was dirty due to previous changes, it now becomes
 * clean.  If it was already clean (just read from config, say),
 * this makes it dirty and needs to be saved/"unsaved"
 */
void MenuItemData::Reset()
{
  if (IsNotNull(m_userShortcut) || m_status.isdeleted()) {
    m_userShortcut = wxAcceleratorEntry();
    m_status.setorig();
    m_status.flipdirty();
  }
}

void MenuItemData::ClearDirtyFlag()
{
  m_status.cleardirty();
}

//////////////////////////////////////////////////////////////////////////
// various helpers/functors used by PWSMenuShortcuts
//
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

struct SameShortcutTarget: public std::binary_function<st_prefShortcut, MenuItemData, bool>
{
  bool operator()(const st_prefShortcut& a, const MenuItemData& b) const {
    return a.id == unsigned(b.GetMenuItem()->GetId());
  }
};

struct ApplyEditedShortcuts {
  void operator()(MenuItemData& mi) const {
    if (mi.IsDirty()) {
      mi.ApplyEffectiveShortcut();
      mi.ClearDirtyFlag();
    }
  }
};

bool IsFunctionKey(int keycode)
{
  return keycode >= WXK_F1 && keycode <= WXK_F24;
}

void SetFont(wxGrid *grid, int row)
{
  wxCHECK_RET(row >= 0 && unsigned(row) < PWSMenuShortcuts::GetShortcutsManager()->Count(), wxT("Invalid row in ShortcutsAttrProvider"));
  MenuItemData::ShortcutStatus status = PWSMenuShortcuts::GetShortcutsManager()->GetGridShortcutStatusAt(unsigned(row));

  struct FontAttr {
    wxFontWeight weight;
    int          style;
  } fattr[3][2] = {  // rows = {orig, custom, deleted}, cols = {shortcut, label}
    { {wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL}, {wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL} },
    { {wxFONTWEIGHT_BOLD,   wxFONTSTYLE_SLANT},  {wxFONTWEIGHT_BOLD,    wxFONTSTYLE_SLANT} },
    { {wxFONTWEIGHT_LIGHT,  wxFONTSTYLE_SLANT},  {wxFONTWEIGHT_BOLD,    wxFONTSTYLE_SLANT} },
  };

  int idx_status;
  if (status.isorig())
    idx_status = 0;
  else if (status.ischanged())
    idx_status = 1;
  else if (status.isdeleted())
    idx_status = 2;
  else {
    wxFAIL_MSG(wxT("Unexpected shortcut status"));
    return;
  }

  for (int col = 0; col < 2; ++col) {
    wxFont ft(grid->GetCellFont(row, col));
    if (ft.IsOk()) {
      ft.SetWeight(fattr[idx_status][col].weight);
      ft.SetStyle(fattr[idx_status][col].style);
      grid->SetCellFont(row, col, ft);
    }
  }

  wxRect rect(grid->CellToRect( row, 0 ));
  rect.x = 0;
  rect.width = grid->GetGridWindow()->GetClientSize().GetWidth();
  int dummy;
  grid->CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
  grid->GetGridWindow()->Refresh( false, &rect );

}

/////////////////////////////////////////////////////////////////////////////////
// PWSMenuShortcuts
///////////////////
PWSMenuShortcuts::PWSMenuShortcuts(wxMenuBar* menuBar): m_shortcutsGrid(0)
{
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
  wxASSERT_MSG(g_pShortcutsManager, wxT("Shortcuts manager being used before creation or after destruction"));
  return g_pShortcutsManager;
}

void PWSMenuShortcuts::DestroyShortcutsManager()
{
  delete g_pShortcutsManager;
  g_pShortcutsManager = 0;
}

wxString PWSMenuShortcuts::MenuLabelAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), wxEmptyString, wxT("Index for menu label exceeds number of menu items retrieved"));
  return m_midata[index].GetLabel();
}

wxAcceleratorEntry PWSMenuShortcuts::EffectiveShortcutAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), nullAccel, wxT("Invalid index for effective shortcut"));
  return m_midata[index].GetEffectiveShortcut();
}

wxAcceleratorEntry PWSMenuShortcuts::OriginalShortcutAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), nullAccel, wxT("Invalid index for original shortcut"));
  return m_midata[index].GetOriginalShortcut();
}

wxMenuItem* PWSMenuShortcuts::MenuItemAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), 0, wxT("Index for menuitem exceeds number of menu items retrieved"));
  return m_midata[index].GetMenuItem();
}

int PWSMenuShortcuts::MenuIdAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), 0, wxT("Index for menuitem exceeds number of menu items retrieved"));
  return m_midata[index].GetMenuId();
}

void PWSMenuShortcuts::ChangeShortcutAt(size_t idx, const wxAcceleratorEntry& newEntry)
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

bool PWSMenuShortcuts::IsDirty() const
{
  return std::find_if(m_midata.begin(), m_midata.end(), std::mem_fun_ref(&MenuItemData::IsDirty)) != m_midata.end();
}

/*
 * Read user-specified shortcuts from config and apply them to the menubar
 */
void PWSMenuShortcuts::ReadApplyUserShortcuts()
{
  typedef std::vector<st_prefShortcut> userShortcut_t;
  const std::vector<st_prefShortcut>& userShortcuts = PWSprefs::GetInstance()->GetPrefShortcuts();
  for (userShortcut_t::const_iterator usrItr = userShortcuts.begin(); usrItr != userShortcuts.end(); ++usrItr) {
    auto itr = std::find_if(m_midata.begin(), m_midata.end(),
                            std::bind1st(SameShortcutTarget(), *usrItr));
    if (itr != m_midata.end()) {
      itr->SetUserShortcut(*usrItr);
      itr->ApplyEffectiveShortcut();
    }
    else {
      pws_os::Trace(L"Could not find menu item id=[%d], for saved shortcut {key=[%d], mods=[%d]}",
                    usrItr->id, usrItr->siVirtKey, usrItr->cPWSModifier);
    }
  }
}

// Set the shortcuts of all menuitems to new ones, if modified
void PWSMenuShortcuts::ApplyEditedShortcuts()
{
  std::for_each(m_midata.begin(), m_midata.end(), ::ApplyEditedShortcuts());
}

/*
 * Save user-specified shortcuts to config file
 */
void PWSMenuShortcuts::SaveUserShortcuts()
{
  std::vector<st_prefShortcut> userShortcuts;
  for (MenuItemDataArray::iterator itr = m_midata.begin(); itr != m_midata.end(); ++itr) {
    if (itr->ShouldSave()) {
      userShortcuts.push_back(itr->ToPrefShortcut());
    }
  }
  PWSprefs::GetInstance()->SetPrefShortcuts(userShortcuts);
  PWSprefs::GetInstance()->SaveShortcuts();
}

/*
 * Reset all shortcuts to their original values in the grid
 */
void PWSMenuShortcuts::GridResetAllShortcuts()
{
  for( ShortcutStatusArray::iterator itr = m_shortcutGridStatus.begin(); itr != m_shortcutGridStatus.end(); ++itr) {
    if (itr->ischanged() || itr->isdeleted()) {
      itr->setorig();
      const size_t index = std::distance(m_shortcutGridStatus.begin(), itr);
      wxCHECK2_MSG(index < m_midata.size(), continue, wxT("ShortcutStatusArray is not the same size as menu item data"));
      wxAcceleratorEntry origShortcut = OriginalShortcutAt(index);
      if (IsNotNull(origShortcut))
        m_shortcutsGrid->SetCellValue(index, COL_SHORTCUT_KEY, origShortcut.ToString());
      else
        m_shortcutsGrid->SetCellValue(index, COL_SHORTCUT_KEY, wxEmptyString);
      SetFont(m_shortcutsGrid, index);
    }
  }
}

/*
 * Reset or remove a shortcut (including the original shortcut) in the grid
 */
void PWSMenuShortcuts::GridResetOrRemoveShortcut(wxGrid* grid, size_t index)
{
  wxCHECK_RET(index < m_shortcutGridStatus.size(), wxT("Invalid index for shortcut reset"));
  MenuItemData::ShortcutStatus& status = m_shortcutGridStatus[index];
  if (status.ischanged() || status.isdeleted()) {
    status.setorig();
    if (IsNotNull(m_midata[index].GetOriginalShortcut()))
      grid->SetCellValue(index, COL_SHORTCUT_KEY, m_midata[index].GetOriginalShortcut().ToString());
    else
      grid->SetCellValue(index, COL_SHORTCUT_KEY, wxEmptyString);
  }
  else if (status.isorig() && IsNotNull(m_midata[index].GetOriginalShortcut())) {
    status.setdeleted();
    grid->SetCellValue(index, COL_SHORTCUT_KEY, wxEmptyString);
  }
  SetFont(grid, index);
}

bool PWSMenuShortcuts::IsShortcutCustomizedAt(size_t idx) const
{
  wxCHECK_MSG(idx < Count(), false, wxT("Invalid index for shortcut customization check"));
  return m_midata[idx].IsCustom();
}

bool PWSMenuShortcuts::HasEffectiveShortcutAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), false, wxT("Invalid index for effective shortcut check"));
  return m_midata[index].HasEffectiveShortcut();
}

MenuItemData::ShortcutStatus PWSMenuShortcuts::GetGridShortcutStatusAt(size_t index) const
{
  wxCHECK_MSG(index < Count(), MenuItemData::ShortcutStatus(), wxT("Invalid index for shortcut status"));
  return m_shortcutGridStatus[index];
}

/*
 * will remove the user-specified shortcut.  If there wasn't any, then it will
 * remove the original shortcut, if any
 */
void PWSMenuShortcuts::RemoveShortcutAt(size_t idx)
{
  wxCHECK_RET(idx < Count(), wxT("Index for deleting shortcut exceeds number of menu items retrieved"));

  m_midata[idx].SetUserShortcut(wxAcceleratorEntry());
}

/*
 * Set ourselves up for handling events from a grid where shortcuts are being edited
 * Also set up some housekeeping data to manage the state of shortcut display in the grid,
 * which will be used by ShortcutsAttrProvider below
 */
void PWSMenuShortcuts::SetShorcutsGridEventHandlers(wxGrid* grid, wxButton* resetAllButton)
{
  m_shortcutGridStatus.resize(m_midata.size());
  std::transform(m_midata.begin(), m_midata.end(), m_shortcutGridStatus.begin(), std::mem_fun_ref(&MenuItemData::GetStatus));

  grid->Connect(wxEVT_GRID_CELL_CHANGED, wxGridEventHandler(PWSMenuShortcuts::OnShortcutChange), nullptr, this);
  grid->Connect(wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler(PWSMenuShortcuts::OnShortcutRightClick), nullptr, this);
  //let's not directly connect to the grid for key events.  We'll only handle what bubbles up to us
  grid->GetGridWindow()->Connect(grid->GetGridWindow()->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(PWSMenuShortcuts::OnShortcutKey), nullptr, this);
  grid->GetGridWindow()->Connect(grid->GetGridWindow()->GetId(), wxEVT_CHAR, wxCharEventHandler(PWSMenuShortcuts::OnKeyChar), nullptr, this);
  resetAllButton->Connect(resetAllButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PWSMenuShortcuts::OnResetAll), nullptr, this);
  m_shortcutsGrid = grid;
}

/*
 * We get here when user edits the contents of the shortcut column in the grid.
 * We need to validate the shortcut entered and provide some feedback if
 * the user-specified text cannot be parsed into a valid shortcut
 */
void PWSMenuShortcuts::OnShortcutChange(wxGridEvent& evt)
{
  wxGrid* grid = wxDynamicCast(evt.GetEventObject(), wxGrid);
  wxCHECK_RET(grid, wxT("Could not get grid from wxGridEvent"));
  wxASSERT_MSG(grid == m_shortcutsGrid, wxT("Events from unexpected grid"));
  wxGridCellCoords cell(evt.GetRow(), evt.GetCol());
  wxString newStr = grid->GetCellValue(cell);
  if (newStr.IsEmpty())
    return;
  wxAcceleratorEntry newAccel;
  if (!newAccel.FromString(wxT('\t') + newStr)) {
    grid->SetCellTextColour(cell.GetRow(), cell.GetCol(), *wxRED);
  }
  else {
    m_shortcutGridStatus[cell.GetRow()].setchanged();
    grid->SetCellValue(cell, newAccel.ToString());
    grid->SetCellTextColour(cell.GetRow(), cell.GetCol(), grid->GetDefaultCellTextColour());
  }
  SetFont(grid, cell.GetRow());
}

/*
 * This is what recognizes the keypress users wants to use as a shortcut
 * We convert the shortcut to appropriate text and put it in the grid
 */
void PWSMenuShortcuts::OnShortcutKey(wxKeyEvent& evt)
{
  wxWindow* gridWindow = wxDynamicCast(evt.GetEventObject(), wxWindow);
  wxGrid* grid = wxDynamicCast(gridWindow->GetParent(), wxGrid);
  wxCHECK_RET(grid, wxT("Could not get grid from wxKeyEvent"));
  wxASSERT_MSG(grid == m_shortcutsGrid, wxT("Events from unexpected grid"));
  const int col = grid->GetGridCursorCol();
  //unless there are modifiers, don't attempt anything
  if ((evt.GetModifiers() || IsFunctionKey(evt.GetKeyCode()))
                && col == COL_SHORTCUT_KEY) {
    wxAcceleratorEntry accel(ModifiersToAccelFlags(evt.GetModifiers()), evt.GetKeyCode(), 0);
    wxCHECK_RET(IsNotNull(accel), wxT("Could not create accelerator from wxKeyEvent"));
    const int row = grid->GetGridCursorRow();
    grid->SetCellValue(row, col, accel.ToString());
    grid->SetCellTextColour(row, col, grid->GetDefaultCellTextColour());
    m_shortcutGridStatus[row].setchanged();
    SetFont(grid, row);
  }
  evt.Skip();
}

/*
 * Handle the DELETE key cycle to reset or remove (if any) the accelerator
 * The cycle being
 *            user-specified shortcut => original shortcut
 *            original shortcut       => empty (no shortcut)
 *            empty (no shortcut)     => original shortcut, if any
 */
void PWSMenuShortcuts::OnKeyChar(wxKeyEvent& evt)
{
  wxWindow* gridWindow = wxDynamicCast(evt.GetEventObject(), wxWindow);
  wxGrid* grid = wxDynamicCast(gridWindow->GetParent(), wxGrid);
  wxCHECK_RET(grid, wxT("Could not get grid from wxKeyEvent"));
  wxASSERT_MSG(grid == m_shortcutsGrid, wxT("Events from unexpected grid"));
  if (!evt.GetModifiers() && evt.GetKeyCode() == WXK_DELETE) {
    const int row = grid->GetGridCursorRow();
    GridResetOrRemoveShortcut(grid, row);
  }
  else
    evt.Skip();
}

struct GridAndIndex
{
  wxGrid* grid;
  size_t  index;
};

/*
 * Display a single-item menu to reset/remove a particular shortcut
 * We don't use a specific menu-item id, just plug-in our handler
 * to the menu after creation
 */
void PWSMenuShortcuts::OnShortcutRightClick( wxGridEvent& evt )
{
  wxMenu shortcutsMenu;
  shortcutsMenu.Append(wxID_ANY, _("&Reset/Remove shortcut\tDel"));

  wxCHECK_RET(evt.GetRow() >= 0 && unsigned(evt.GetRow()) < m_midata.size(),
                wxString::Format(wxT("Invalid index [%d] for reset/remove shortcut"),
                                  evt.GetRow()).c_str());
  GridAndIndex gr = { wxDynamicCast(evt.GetEventObject(), wxGrid), size_t(evt.GetRow()) };
  wxCHECK_RET(gr.grid, wxT("Could not get grid from right-click grid event"));

  shortcutsMenu.SetClientData(reinterpret_cast<void*>(&gr));
  shortcutsMenu.Connect( shortcutsMenu.FindItemByPosition(0)->GetId(),
                          wxEVT_COMMAND_MENU_SELECTED,
                          wxCommandEventHandler(PWSMenuShortcuts::OnResetRemoveShortcut),
                          nullptr, this );
  gr.grid->PopupMenu(&shortcutsMenu);
}

/*
 * Handler for the Reset/Remove context menu displayed on right-clicking
 * the shortcuts grid.
 */
void PWSMenuShortcuts::OnResetRemoveShortcut( wxCommandEvent& evt )
{
  wxMenu* shortcutsMenu = wxDynamicCast(evt.GetEventObject(), wxMenu);
  wxCHECK_RET(shortcutsMenu, wxT("Could not get shortcuts reset/remove menu from event"));
  auto *gr = reinterpret_cast<GridAndIndex*>(shortcutsMenu->GetClientData());
  wxCHECK_RET(gr, wxT("Could not find internal data in reset/remove handler"));
  GridResetOrRemoveShortcut(gr->grid, gr->index);
}

void PWSMenuShortcuts::OnResetAll(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  GridResetAllShortcuts();
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
        if (!IsSameShortcut(newAccel, oldAccel)) {
          //set the remaining fields in the new accelerator entry, so they get written to pwsafe.cfg
          newAccel.Set(newAccel.GetFlags(), newAccel.GetKeyCode(),
                       m_shortcuts.MenuIdAt(row), m_shortcuts.MenuItemAt(row));
          m_shortcuts.ChangeShortcutAt(row, newAccel);
        }
      }
      else {
        success = false;
      }
    }
    else if (m_shortcuts.HasEffectiveShortcutAt(row)) {
      // remove the current user shortcut, if there
      m_shortcuts.RemoveShortcutAt(row);
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
    MenuItemData::ShortcutStatus scs = m_shortcuts.GetGridShortcutStatusAt(row);
    if (scs.ischanged()) {
      wxCHECK2_MSG(m_shortcuts.HasEffectiveShortcutAt(row), continue, wxT("shortcut with status changed has not effective shortcut!"));
      wxAcceleratorEntry ae = m_shortcuts.EffectiveShortcutAt(row);
      wxCHECK2_MSG(IsNotNull(ae), continue, wxT("shortcut with status changed has null keycode!"));
      grid->SetCellValue(row, COL_SHORTCUT_KEY, ae.ToString());
    }
    else if (scs.isorig()){
      wxAcceleratorEntry origShortcut = m_shortcuts.OriginalShortcutAt(row);
      if (IsNotNull(origShortcut))
        grid->SetCellValue(row, COL_SHORTCUT_KEY, origShortcut.ToString());
    }
    grid->SetCellValue(row, COL_MENU_ITEM, m_shortcuts.MenuLabelAt(row));
    SetFont(grid, row);
  }
  grid->AutoSize();
  grid->GetParent()->Layout();
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
        wxString msg(_("Shortcut # "));
        msg << (row + 1) << wxT(" [") << newStr << wxT("] ") << _("is not a valid shortcut");
        wxMessageBox(msg, _("Invalid shortcut"), wxOK | wxICON_ERROR, parent);
        grid->SetGridCursor(row, COL_SHORTCUT_KEY);
        return false;
      }
    }
  }
  return true;
}
