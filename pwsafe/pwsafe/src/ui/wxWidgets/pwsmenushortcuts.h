/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __PWSMENUSHORTCUTS_H__
#define __PWSMENUSHORTCUTS_H__

#include <vector>

/*
 * data we need to track for managing menuitem shortcuts
 */
struct MenuItemData{
  wxMenuItem*         item;
  wxString            label;
  wxAcceleratorEntry  oldShortcut;
  wxAcceleratorEntry  newShortcut;
};

/*
 * This class provides convenient access to all the shortcuts used by menuitems in a menubar
 * The access is index-based.  You give it a menubar or a string (from prefs), and access
 * all the menu items and their accelerators by index
 */
class PWSMenuShortcuts {

  typedef std::vector<MenuItemData> MenuItemDataArray;

  MenuItemDataArray m_midata;

public:
  // Walk the menubar and collect all shortcuts
  PWSMenuShortcuts();

  // Note: this doesn't save all shortcuts automatically to prefs. You have to do it
  // yourself using ToString()
  ~PWSMenuShortcuts();

  size_t Count() const { return m_midata.size(); }

  wxString MenuLabelAt(size_t index) const;
  wxAcceleratorEntry OldShortcutAt(size_t index) const;
  wxAcceleratorEntry NewShortcutAt(size_t index) const;

  void ChangeShortcut(size_t idx, const wxAcceleratorEntry& newEntry);

  bool IsDirty() const;

  // Set the shortcuts of all menuitems to new ones, if modified
  void ApplyAll();

  wxString ToString() const;
};

enum {COL_SHORTCUT_KEY, COL_MENU_ITEM}; //For shortcuts page

/*
 * A validator for shortcuts displayed in a grid
 */
struct ShortcutsGridValidator: public wxValidator
{
  ShortcutsGridValidator(PWSMenuShortcuts& shortcuts) : m_shortcuts(shortcuts) {}

  virtual wxObject* Clone() const { return new ShortcutsGridValidator(m_shortcuts); }
  virtual bool TransferFromWindow();
  virtual bool TransferToWindow();
  virtual bool Validate (wxWindow* parent);
  
private:
  PWSMenuShortcuts& m_shortcuts;
  DECLARE_NO_COPY_CLASS(ShortcutsGridValidator)
};

#endif // __PWSMENUSHORTCUTS_H__
