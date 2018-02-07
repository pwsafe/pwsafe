/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __PWSMENUSHORTCUTS_H__
#define __PWSMENUSHORTCUTS_H__

#include <vector>

struct st_prefShortcut;
class wxGrid;
class wxGridEvent;

/*
 * data we need to track for managing menuitem shortcuts
 */
class MenuItemData{
  wxMenuItem*         m_item;
  int                 m_menuId;
  wxString            m_label;
  wxAcceleratorEntry  m_origShortcut;
  wxAcceleratorEntry  m_userShortcut;

public:
  class ShortcutStatus {
  
    enum {
      SC_NORMAL    = 1,      /* unmodified.  Might have user, original, both or none */
      SC_CHANGED   = 2,      /* user shortcut has been added/changed */
      SC_DELETED   = 4,      /* shortcut deleted.  neither user nor original available */
      SC_DIRTY     = 8       /* something has changed, and this shortcut should be saved or removed from config */
    };
    int flags;
  public:
    ShortcutStatus(): flags(SC_NORMAL) {}
    void setorig()    { flags |= SC_NORMAL;  flags &= ~(SC_CHANGED | SC_DELETED); }
    void setchanged() { flags |= SC_CHANGED; flags &= ~(SC_NORMAL  | SC_DELETED); }
    void setdeleted() { flags |= SC_DELETED; flags &= ~(SC_NORMAL  | SC_CHANGED); }

    bool isorig()     const { return (flags & SC_NORMAL)  == SC_NORMAL;  }
    bool ischanged()  const { return (flags & SC_CHANGED) == SC_CHANGED; }
    bool isdeleted()  const { return (flags & SC_DELETED) == SC_DELETED; }

    void setdirty()   { flags |= SC_DIRTY; }
    void flipdirty()  { flags ^= SC_DIRTY; }
    void cleardirty() { flags &= ~SC_DIRTY; }
    bool isdirty()    const  { return (flags & SC_DIRTY) == SC_DIRTY; }
  };

private:
  ShortcutStatus      m_status;
  
public:
  MenuItemData(wxMenuItem* menuItem, const wxString& label);

  wxMenuItem*                  GetMenuItem()         const { return m_item;         }
  int                          GetMenuId()           const { return m_menuId;       }
  wxString                     GetLabel()            const { return m_label;        }
  const wxAcceleratorEntry&    GetOriginalShortcut() const { return m_origShortcut; }
  const wxAcceleratorEntry&    GetUserShortcut()     const { return m_userShortcut; }
  ShortcutStatus               GetStatus()           const { return m_status;       }

  // Setting the user shortcut from preferences doesn't make it dirty
  void SetUserShortcut(const st_prefShortcut& prefShortcut, bool setdirty = false);
  // But setting it from a shortcut specified in the GUI does.
  void SetUserShortcut(const wxAcceleratorEntry& userAccel, bool setdirty = true);

  // converts user shortcut to prefs shortcut.  Doesn't look at original shortcut
  st_prefShortcut ToPrefShortcut() const;

  // Remove the user shortcut, and clear the "shortcut deleted" flag
  void Reset();

  // This tells whether the shortcut should be written to prefs file
  // Has nothing to do with whether it has been modified
  bool ShouldSave() const;

  // returns true if the effective shortcut is not in any way the same as original shortcut
  bool IsCustom() const;

  bool HasEffectiveShortcut() const;
  wxAcceleratorEntry GetEffectiveShortcut() const;

  void  ApplyEffectiveShortcut();

  bool IsDirty() const;
  void ClearDirtyFlag();
};

/*
 * This class provides convenient access to all the shortcuts used by menuitems in a menubar
 * The access is index-based.  You give it a menubar or a string (from prefs), and access
 * all the menu items and their accelerators by index
 */
class PWSMenuShortcuts : public wxEvtHandler {

  typedef std::vector<MenuItemData> MenuItemDataArray;

  MenuItemDataArray m_midata;

  //used for tracking the status of shortcuts while editing them in the options grid
  //we don't modify the actual shortcuts unless the user clicks Ok
  typedef std::vector<MenuItemData::ShortcutStatus> ShortcutStatusArray;
  ShortcutStatusArray m_shortcutGridStatus;

  // Walks the menubar and collect all shortcuts
  PWSMenuShortcuts(wxMenuBar* menubar);

  // Note: the dtor doesn't save all shortcuts automatically to prefs. You have to do it
  // yourself using SaveUserShortcuts()
  ~PWSMenuShortcuts();

public:

  size_t Count() const { return m_midata.size(); }

  wxString MenuLabelAt(size_t index) const;
  wxAcceleratorEntry EffectiveShortcutAt(size_t index) const;
  wxAcceleratorEntry OriginalShortcutAt(size_t index) const;
  wxMenuItem* MenuItemAt(size_t index) const;
  int MenuIdAt(size_t index) const;
  bool IsShortcutCustomizedAt(size_t index) const;
  bool HasEffectiveShortcutAt(size_t index) const;

  // change or remove the user shortcut
  void ChangeShortcutAt(size_t idx, const wxAcceleratorEntry& newEntry);
  void RemoveShortcutAt(size_t idx);

  // Did the user change any of the shortcuts that aren't reflected in the menubar yet?
  bool IsDirty() const;

  // Set the shortcuts of all menuitems to new ones, if modified
  void ApplyEditedShortcuts();

  // Call this the menubar has been fully created, to read user shortcuts from prefs & apply them
  void ReadApplyUserShortcuts();

  // Save User shortcuts to prefs file
  void SaveUserShortcuts();

  // Remove or reset all user shortcuts in grid.  They don't take effect until
  // user clicks Ok
  void GridResetAllShortcuts();
  void GridResetOrRemoveShortcut(wxGrid* grid, size_t index);

  MenuItemData::ShortcutStatus GetGridShortcutStatusAt(size_t index) const;
  
  void SetShorcutsGridEventHandlers(wxGrid* grid, wxButton* resetAllButton);
  void OnShortcutChange(wxGridEvent& evt);
  void OnShortcutKey(wxKeyEvent& evt);
  void OnKeyChar(wxKeyEvent& evt);
  void OnShortcutRightClick( wxGridEvent& evt );
  void OnResetRemoveShortcut( wxCommandEvent& evt );
  void OnResetAll(wxCommandEvent& evt);

  static PWSMenuShortcuts* CreateShortcutsManager(wxMenuBar* menubar);
  static PWSMenuShortcuts* GetShortcutsManager();
  static void DestroyShortcutsManager();

private:
  // The grid we are handling events for
  wxGrid* m_shortcutsGrid;
};

enum {COL_SHORTCUT_KEY, COL_MENU_ITEM}; //For shortcuts page

/*
 * A validator for shortcuts displayed & edited in a wxGrid
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
