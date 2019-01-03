/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file MainMenu.cpp
// Menu-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"

#include "HKModifiers.h"

#include "core/PWSprefs.h"

#include "os/file.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Functor for count_if
struct CountShortcuts {
  bool operator()(const std::pair<const UINT, CMenuShortcut> &p) {
    return (p.second.siVirtKey != 0);
  }
};

// Functor for for_each
struct CreateAccelTable {
  CreateAccelTable(ACCEL *pacceltbl, unsigned char ucAutotypeKey) 
    : m_pacceltbl(pacceltbl), m_ucAutotypeKey(ucAutotypeKey) {}

  void operator()(const std::pair<UINT, CMenuShortcut> &p)
  {
    if (p.second.siVirtKey != 0 && p.second.siVirtKey != m_ucAutotypeKey) {
      m_pacceltbl->fVirt = FVIRTKEY |
               ((p.second.cPWSModifier & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL ? FCONTROL : 0) |
               ((p.second.cPWSModifier & PWS_HOTKEYF_ALT)     == PWS_HOTKEYF_ALT     ? FALT     : 0) |
               ((p.second.cPWSModifier & PWS_HOTKEYF_SHIFT)   == PWS_HOTKEYF_SHIFT   ? FSHIFT   : 0);
      m_pacceltbl->key = (WORD)p.second.siVirtKey;
      m_pacceltbl->cmd = (WORD)p.first;
      m_pacceltbl++;
    }
  }

private:
  ACCEL *m_pacceltbl;
  unsigned char m_ucAutotypeKey; // AutoType shortcut key
};

static bool IsExtended(int code)
{
  switch (code) {
    case VK_PRIOR:                       // Page Up
    case VK_NEXT:                        // Page Down
    case VK_END:                         // End
    case VK_HOME:                        // Home
    case VK_LEFT:                        // Left arrow cursor
    case VK_UP:                          // Up arrow cursor
    case VK_RIGHT:                       // Right arrow cursor
    case VK_DOWN:                        // Down arrow cursor
    case VK_SNAPSHOT:                    // Print Screen
    case VK_INSERT:                      // Insert
    case VK_DELETE:                      // Delete
    case VK_LWIN:                        // Left Windows Key
    case VK_RWIN:                        // Right Windows Key
    case VK_APPS:                        // Windows Application Key
    case VK_DIVIDE:                      // Numeric pad '/'
    case VK_NUMLOCK:                     // Numlock
      return true;
    default:
      return false;
  }
}

void InsertShortcuts(CMenu *pMenu, MapMenuShortcuts &mms,
                     const unsigned int parentID)
{
  // if parentID == 0, we're processing a toplevel menu,
  // else, we're passed the menu in which the desired
  // submenu resides.
  wchar_t tcMenuString[_MAX_PATH + 1];
  CMenuShortcut mst;
  MENUITEMINFO miteminfo;
 
  CMenu *pSCTMenu = pMenu;
  ASSERT_VALID(pSCTMenu);
  static int iMenuPos(0);

  if (parentID != 0) {
    int isubmenu_pos = app.FindMenuItem(pSCTMenu, parentID);
    ASSERT(isubmenu_pos != -1);
    pSCTMenu = pSCTMenu->GetSubMenu(isubmenu_pos);
    ASSERT_VALID(pSCTMenu);
  }

  mst.uiParentID = parentID;

  UINT uiCount = pSCTMenu->GetMenuItemCount();
  ASSERT((int)uiCount >= 0);

  for (UINT ui = 0; ui < uiCount; ui++) {
    SecureZeroMemory(tcMenuString, sizeof(tcMenuString));
    SecureZeroMemory(&miteminfo, sizeof(miteminfo));
    miteminfo.cbSize = sizeof(miteminfo);
    miteminfo.fMask = MIIM_ID | MIIM_STRING;
    miteminfo.cch = _MAX_PATH;
    miteminfo.dwTypeData = tcMenuString;

    VERIFY(pSCTMenu->GetMenuItemInfo(ui, &miteminfo, TRUE));

    if (miteminfo.wID >= 1) {
      std::pair< MapMenuShortcutsIter, bool > pr;
      mst.name = tcMenuString;
      mst.iMenuPosition = iMenuPos;
      pr = mms.insert(MapMenuShortcutsPair(miteminfo.wID, mst));
      ASSERT(pr.second == true);
      iMenuPos++;
    }
  }
}

void DboxMain::SetUpInitialMenuStrings()
{
  CMenu *pMainMenu, *pSubMenu;
  int isubmenu_pos;
  UINT uiCount;

  CString sKeyName;

  // Following are excluded from list of user-configurable
  // shortcuts
  UINT excludedMenuItems[] = {
  // Add user Excluded Menu Items - anything that is a Popup Menu
    ID_FILEMENU, ID_EXPORTMENU, ID_IMPORTMENU, ID_EDITMENU,
    ID_VIEWMENU, ID_SUBVIEWMENU, ID_FILTERMENU,
    ID_CHANGEFONTMENU, ID_REPORTSMENU,
    ID_MANAGEMENU, ID_LANGUAGEMENU, ID_HELPMENU, ID_FINDMENU,
    ID_EXPORTENTMENU, ID_EXPORTGROUPMENU,

  // Plus Exit (2 shortcuts Ctrl+Q and Alt+F4) and Help (F1)
    ID_MENUITEM_EXIT, ID_HELP,

  // Plus Expand/Collapse group - function of TreeCtrl and is
  // the Enter key, which cannot be assigned via the Shortcut
  // Property Page
    ID_MENUITEM_GROUPENTER,

  // The following are only in the Menu to get the correct string
  // for the action.  Add here to stop them being in the Shortcut
  // Options CListCtrl.
  // User can alter the function using the base values
  // ID_MENUITEM_EDITENTRY, ID_MENUITEM_DELETE & ID_MENUITEM_RENAME
    ID_MENUITEM_VIEWENTRY,
    ID_MENUITEM_DELETEENTRY, ID_MENUITEM_DELETEGROUP,
    ID_MENUITEM_RENAMEENTRY, ID_MENUITEM_RENAMEGROUP,
  };

  m_ExcludedMenuItems.clear();
  m_ExcludedMenuItems.assign(excludedMenuItems,
                             excludedMenuItems + _countof(excludedMenuItems));

  pMainMenu = new CMenu;
  VERIFY(pMainMenu->LoadMenu(IDR_MAINMENU));

  wchar_t tcMenuString[_MAX_PATH + 1];

  MENUITEMINFO miteminfo = {0};

  uiCount = pMainMenu->GetMenuItemCount();
  ASSERT((int)uiCount >= 0);

  // Add reserved shortcuts = Alt+'x' for main menu e.g. Alt+F, E, V, M, H
  // Maybe different in other languages
  m_ReservedShortcuts.clear();

  st_MenuShortcut st_mst;
  st_mst.siVirtKey;
  st_mst.cPWSModifier = PWS_HOTKEYF_ALT;

  for (UINT ui = 0; ui < uiCount; ui++) {
    SecureZeroMemory(tcMenuString, sizeof(tcMenuString));

    SecureZeroMemory(&miteminfo, sizeof(miteminfo));
    miteminfo.cbSize = sizeof(miteminfo);
    miteminfo.fMask = MIIM_ID | MIIM_STRING;
    miteminfo.dwTypeData = tcMenuString;
    miteminfo.cch = _MAX_PATH;

    VERIFY(pMainMenu->GetMenuItemInfo(ui, &miteminfo, TRUE));

    if (miteminfo.wID >= 1) {
      CString csMainMenuItem = tcMenuString;
      int iamp = csMainMenuItem.Find(L'&');
      if (iamp >= 0 && iamp < csMainMenuItem.GetLength() - 1) {
        st_mst.nControlID = miteminfo.wID;
        st_mst.siVirtKey = csMainMenuItem[iamp + 1];
        m_ReservedShortcuts.push_back(st_mst);
      }
    }
  }

  // Add 3 special keys F1 (for Help), Ctrl+Q/Alt+F4 (Exit)
  st_mst.nControlID = ID_MENUITEM_HELP;
  st_mst.siVirtKey = VK_F1;
  st_mst.cPWSModifier = 0;
  m_ReservedShortcuts.push_back(st_mst);

  st_mst.nControlID = ID_MENUITEM_EXIT;
  st_mst.siVirtKey = 'Q';
  st_mst.cPWSModifier = PWS_HOTKEYF_CONTROL;
  m_ReservedShortcuts.push_back(st_mst);

  st_mst.nControlID = ID_MENUITEM_EXIT;
  st_mst.siVirtKey = VK_F4;
  st_mst.cPWSModifier = PWS_HOTKEYF_ALT;
  m_ReservedShortcuts.push_back(st_mst);

  // Now get all other Menu items
  // (ronys) I think we can do the following properly via recursive descent. Later.

  m_MapMenuShortcuts.clear();

  // Do Main Menu (uiParentID == 0)
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, 0);

  // Do File Menu
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, ID_FILEMENU);

  isubmenu_pos = app.FindMenuItem(pMainMenu, ID_FILEMENU);
  ASSERT(isubmenu_pos != -1);
  pSubMenu = pMainMenu->GetSubMenu(isubmenu_pos);

  // Do File Menu Export submenu
  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_EXPORTMENU);

  // Do File Menu Import submenu
  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_IMPORTMENU);

  // Do Edit Menu
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, ID_EDITMENU);

  // Do Export Entry submenu
  //  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_EXPORTENTMENU);

  // Do Export Group submenu
  //  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_EXPORTGROUPMENU);

  // Do View Menu
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, ID_VIEWMENU);

  isubmenu_pos = app.FindMenuItem(pMainMenu, ID_VIEWMENU);
  ASSERT(isubmenu_pos != -1);
  pSubMenu = pMainMenu->GetSubMenu(isubmenu_pos);

  // Do View Menu Subview submenu
  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_SUBVIEWMENU);

  // Do View Menu Filter submenu
  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_FILTERMENU);

  // Do View Menu ChangeFont submenu
  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_CHANGEFONTMENU);

  // Do View Menu Reports submenu
  InsertShortcuts(pSubMenu, m_MapMenuShortcuts, ID_REPORTSMENU);

  // Do Manage Menu
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, ID_MANAGEMENU);

  // No need to do Manage Menu Languages submenu as rebuilt every time!

  // Do Help Menu
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, ID_HELPMENU);

  // Don't need main menu again here
  VERIFY(pMainMenu->DestroyMenu());

  // Do Find toolbar menu items not on a menu!
  VERIFY(pMainMenu->LoadMenu(IDR_POPFIND));

  // Again a parent menu (uiParentID == 0)
  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, 0);

  InsertShortcuts(pMainMenu, m_MapMenuShortcuts, ID_FINDMENU);

  // No longer need any menus
  VERIFY(pMainMenu->DestroyMenu());

  delete pMainMenu;

  // Now that we have all menu strings - go get current accelerator strings
  // and update Map
  MapMenuShortcutsIter iter, iter_entry, iter_parent, inuse_iter;
  HACCEL curacctbl = app.m_ghAccelTable;
  //if (curacctbl != NULL) {
  //  DestroyAcceleratorTable(app.m_ghAccelTable);
  //  curacctbl = app.m_ghAccelTable = NULL;
  //}

  ACCEL *pacceltbl(NULL), *paccel(NULL);
  int numaccels;

  // Just in case user deletes all shortcuts!
  if (app.m_ghAccelTable != NULL) {
    numaccels = CopyAcceleratorTable(curacctbl, NULL, 0);
    pacceltbl = (LPACCEL)LocalAlloc(LPTR, numaccels * sizeof(ACCEL));
    ASSERT(pacceltbl != NULL);

    CopyAcceleratorTable(curacctbl, pacceltbl, numaccels);

    // Load our map with all the current info
    paccel = pacceltbl;
    for (int i = 0; i < numaccels; i++) {
      /*
        FALT     - The ALT key must be held down when the accelerator key is pressed.
        FCONTROL - The CTRL key must be held down when the accelerator key is pressed.
        FSHIFT   - The SHIFT key must be held down when the accelerator key is pressed.
        FVIRTKEY - The key member specifies a virtual-key code. If this flag is not specified,
                   key is assumed to specify a character code.
      */

      iter = m_MapMenuShortcuts.find((UINT)paccel->cmd);
      if (iter != m_MapMenuShortcuts.end()) {
        iter->second.siDefVirtKey = iter->second.siVirtKey = 
          (unsigned char)paccel->key;
          iter->second.cDefPWSModifier = iter->second.cPWSModifier =
          ((paccel->fVirt & FCONTROL) == FCONTROL ? PWS_HOTKEYF_CONTROL : 0) |
          ((paccel->fVirt & FALT)     == FALT     ? PWS_HOTKEYF_ALT     : 0) |
          ((paccel->fVirt & FSHIFT)   == FSHIFT   ? PWS_HOTKEYF_SHIFT   : 0) |
          (IsExtended((int)paccel->key)           ? PWS_HOTKEYF_EXT     : 0);
      }
      paccel++;
    }

    // Don't need copy any more
    LocalFree(pacceltbl);
    pacceltbl = paccel = NULL;
  }

  // Now go through the default table and see if the user has changed anything,
  // change shortcuts as per preferences
  std::vector<st_prefShortcut> vShortcuts(PWSprefs::GetInstance()->GetPrefShortcuts());

  for (auto &stxst : vShortcuts) {
    // User should not have these sub-entries in their config file
    if (stxst.id == ID_MENUITEM_GROUPENTER  ||
        stxst.id == ID_MENUITEM_VIEWENTRY   ||
        stxst.id == ID_MENUITEM_DELETEENTRY ||
        stxst.id == ID_MENUITEM_DELETEGROUP ||
        stxst.id == ID_MENUITEM_RENAME      ||
        stxst.id == ID_MENUITEM_RENAMEENTRY ||
        stxst.id == ID_MENUITEM_RENAMEGROUP) {
      continue;
    }

    iter = m_MapMenuShortcuts.find(stxst.id);
    if (iter == m_MapMenuShortcuts.end()) {
      // Unknown Control ID - ignore - maybe used by a later version of PWS
      continue;
    }

    // Update Menu name in vector
    iter_parent = m_MapMenuShortcuts.find(iter->second.uiParentID);
    std::wstring name(L"");
    do {
      name = iter_parent->second.name + std::wstring(L"->") + name;
      iter_parent = m_MapMenuShortcuts.find(iter_parent->second.uiParentID);
    } while (iter_parent != m_MapMenuShortcuts.end());

    name += iter->second.name;
    Remove(name, L'&');
    stxst.Menu_Name = name;

    // Check not already in use (ignore if deleting current shortcut)
    if (stxst.siVirtKey != 0) {
      st_mst.siVirtKey = stxst.siVirtKey;
      st_mst.cPWSModifier = stxst.cPWSModifier;
      already_inuse inuse(st_mst);
      inuse_iter = std::find_if(m_MapMenuShortcuts.begin(),
                                m_MapMenuShortcuts.end(),
                                inuse);
      if (inuse_iter != m_MapMenuShortcuts.end() && 
          inuse_iter->first != iter->first) {
        // Shortcut in use - ignore entry - duplicates not allowed!
        continue;
      }
    }

    if ((iter->second.siVirtKey  != stxst.siVirtKey  ||
         iter->second.cPWSModifier != stxst.cPWSModifier)) {
      // User changed or added a shortcut
      iter->second.siVirtKey  = stxst.siVirtKey;
      iter->second.cPWSModifier = stxst.cPWSModifier;
    }
  } // user preference shortcut handling

  // Update Menu names for later XML comment
  PWSprefs::GetInstance()->SetPrefShortcuts(vShortcuts);

  SetupSpecialShortcuts();
  UpdateAccelTable();
}

void DboxMain::UpdateAccelTable()
{
  // m)MapMenuShortcuts -> app.m_ghAccelTable
  ACCEL *pacceltbl, *pxatbl;
  int numscs(0);
  CountShortcuts cntscs;

  // Add on space of 3 reserved shortcuts (Ctrl-Q, F4, F1)
  numscs = (int)std::count_if(m_MapMenuShortcuts.begin(), m_MapMenuShortcuts.end(),
                         cntscs) + 3;
  // But take off 1 if there is a shortcut for AutoType
  if (m_wpAutotypeKey != 0)
    numscs--;

  // Get new table space
  pacceltbl = (LPACCEL)LocalAlloc(LPTR, numscs * sizeof(ACCEL));
  ASSERT(pacceltbl != NULL);

  // Populate it
  CreateAccelTable create_accel_table(pacceltbl, (unsigned char)m_wpAutotypeKey);
  for_each(m_MapMenuShortcuts.begin(), m_MapMenuShortcuts.end(), create_accel_table);

  // Add back in the 3 reserved
  pxatbl = pacceltbl + (numscs - 3);
  pxatbl->fVirt = FVIRTKEY | FCONTROL;
  pxatbl->key = (WORD)'Q';
  pxatbl->cmd = (WORD)ID_MENUITEM_EXIT;
  pxatbl++;
  pxatbl->fVirt = FVIRTKEY | FALT;
  pxatbl->key = (WORD)VK_F4;
  pxatbl->cmd = (WORD)ID_MENUITEM_EXIT;
  pxatbl++;
  pxatbl->fVirt = FVIRTKEY;
  pxatbl->key = (WORD)VK_F1;
  pxatbl->cmd = (WORD)ID_HELP;

  // Replace current one
  DestroyAcceleratorTable(app.m_ghAccelTable);
  app.m_ghAccelTable = CreateAcceleratorTable(pacceltbl, numscs);
  app.SetACCELTableCreated();
}

void DboxMain::SetUpMenuStrings(CMenu *pPopupMenu)
{
  // Can't use GetMenuItemID, as it does not understand that with the MENUEX
  // format, Popup menus can have IDs
  ASSERT_VALID(pPopupMenu);

  MENUITEMINFO miteminfo = {0};

  MapMenuShortcutsIter iter;

  UINT uiCount = pPopupMenu->GetMenuItemCount();
  ASSERT((int)uiCount >= 0);

  for (UINT ui = 0; ui < uiCount; ui++) {
    SecureZeroMemory(&miteminfo, sizeof(miteminfo));
    miteminfo.cbSize = sizeof(miteminfo);
    miteminfo.fMask = MIIM_ID | MIIM_STATE;

    VERIFY(pPopupMenu->GetMenuItemInfo(ui, &miteminfo, TRUE));

    // Exit & Help never changed and their shortcuts are in the menu text
    if (miteminfo.wID >= 1 &&
        miteminfo.wID != ID_MENUITEM_EXIT && miteminfo.wID != ID_HELP) {
      iter = m_MapMenuShortcuts.find(miteminfo.wID);
      if (iter != m_MapMenuShortcuts.end()) {
        CString str;
        if (iter->second.siVirtKey != 0) {
          str.Format(L"%s\t%s", static_cast<LPCWSTR>(iter->second.name.c_str()),
                     static_cast<LPCWSTR>(CMenuShortcut::FormatShortcut(iter)));
        } else {
          str = iter->second.name.c_str();
        }
        pPopupMenu->ModifyMenu(miteminfo.wID, MF_BYCOMMAND | miteminfo.fState, miteminfo.wID, str);
      }
    }
  }
}

void DboxMain::CustomiseMenu(CMenu *pPopupMenu, const UINT uiMenuID, 
                             const bool bDoShortcuts)
{
  // Original OnInitMenu code
  // All main menus are POPUPs (see PasswordSafe2.rc2)

  // This routine changes the text in the menu via "ModifyMenu" and
  // adds the "check" mark via CheckMenuRadioItem for view type and toolbar.
  // It then tailors the Edit menu depending on whether a Group or Entry is selected.
  // "EnableMenuItem" is handled by the UPDATE_UI routines
  bool bGroupSelected(false);
  const bool bTreeView = m_ctlItemTree.IsWindowVisible() == TRUE;
  const bool bItemSelected = (SelItemOk() == TRUE);
  const bool bReadOnly = m_core.IsReadOnly();
  const CItemData *pci(NULL), *pbci(NULL);
  const wchar_t *tc_dummy = L" ";

  ASSERT_VALID(pPopupMenu);

  // Except for Edit & View menus, we only get here if the Shortcuts have changed and
  // we need to rebuild menu item text
  if (bDoShortcuts)
    SetUpMenuStrings(pPopupMenu);

  // Change the 'Change Mode' text as appropriate
  if (uiMenuID == ID_FILEMENU) {
    // Otherwise set the correct menu item text
    pPopupMenu->ModifyMenu(ID_MENUITEM_CHANGEMODE, MF_BYCOMMAND,
      ID_MENUITEM_CHANGEMODE,
      bReadOnly ? CS_READWRITE : CS_READONLY);

    // Disable menu item if no DB or if DB is read-only on disk
    bool bFileIsReadOnly;
    if (!m_bOpen || (pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly) && bFileIsReadOnly)) {
      pPopupMenu->EnableMenuItem(ID_MENUITEM_CHANGEMODE, MF_BYCOMMAND | MF_GRAYED);
    }

    if (app.GetMRU()->GetNumUsed() == 0) {
      pPopupMenu->EnableMenuItem(ID_MENUITEM_CLEAR_MRU, MF_BYCOMMAND | MF_GRAYED);
      pPopupMenu->EnableMenuItem(ID_MENUITEM_MRUENTRY, MF_BYCOMMAND | MF_GRAYED);
    }

    // Remove the corresponding Export V3/V4
    int isubmenu_pos;
    CMenu *pSubMenu;
    isubmenu_pos = app.FindMenuItem(pPopupMenu, ID_EXPORTMENU);
    ASSERT(isubmenu_pos != -1);
    pSubMenu = pPopupMenu->GetSubMenu(isubmenu_pos);
    ASSERT_VALID(pSubMenu);

    const PWSfile::VERSION current_version = m_core.GetReadFileVersion();

    // Delete both menu items and then add the appropriate one
    pSubMenu->DeleteMenu(ID_MENUITEM_EXPORT2V3FORMAT, MF_BYCOMMAND);
    pSubMenu->DeleteMenu(ID_MENUITEM_EXPORT2V4FORMAT, MF_BYCOMMAND);

    switch (current_version) {
      case PWSfile::V30:
      case PWSfile::V40:
      {
        CString tmpStr(MAKEINTRESOURCE((current_version == PWSfile::V40) ?
                         IDS_MENUITEM_EXPORT2V3FORMAT : IDS_MENUITEM_EXPORT2V4FORMAT));
        
        MENUITEMINFO MII = {0};
        MII.cbSize = sizeof(MII);
        MII.fMask = MIIM_STRING | MIIM_ID;
        MII.fType = MFT_STRING;
        MII.fState = MFS_ENABLED;
        MII.wID = (current_version == PWSfile::V40) ? ID_MENUITEM_EXPORT2V3FORMAT : ID_MENUITEM_EXPORT2V4FORMAT;
        MII.hSubMenu = NULL;
        MII.hbmpChecked = NULL;
        MII.hbmpUnchecked = NULL;
        MII.dwTypeData = (LPTSTR)&tmpStr;
        MII.cch = tmpStr.GetLength();

        // Add after export to V2
        pSubMenu->InsertMenuItem(2, &MII, TRUE);
        break;
      }
      default:
        break;
    }

    return;
  }

  // We have done for all except the Edit and View menus
  if (uiMenuID != ID_EDITMENU && uiMenuID != ID_VIEWMENU &&
      uiMenuID != ID_FILTERMENU && uiMenuID != ID_SUBVIEWMENU)
    return;

  // If View menu selected (contains 'Flattened &List' menu item)
  if (uiMenuID == ID_VIEWMENU) {
    pPopupMenu->CheckMenuRadioItem(ID_MENUITEM_LIST_VIEW, ID_MENUITEM_TREE_VIEW,
                                   bTreeView ? ID_MENUITEM_TREE_VIEW : ID_MENUITEM_LIST_VIEW,
                                   MF_BYCOMMAND);

    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_TOOLBAR, MF_BYCOMMAND |
                              m_MainToolBar.IsWindowVisible() ? MF_CHECKED : MF_UNCHECKED);

    bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_DRAGBAR, MF_BYCOMMAND |
                              bDragBarState ? MF_CHECKED : MF_UNCHECKED);

    // Don't show filter menu if "internal" menu active
    pPopupMenu->EnableMenuItem(ID_FILTERMENU, MF_BYCOMMAND |
             (m_bUnsavedDisplayed || m_bExpireDisplayed || m_bFindFilterDisplayed) ? MF_GRAYED : MF_ENABLED);

    pPopupMenu->CheckMenuRadioItem(ID_MENUITEM_NEW_TOOLBAR,
                                   ID_MENUITEM_OLD_TOOLBAR,
                                   m_toolbarMode, MF_BYCOMMAND);

    // Disable View Reports menu if no database open as we need the full path
    // of the database to get to its reports!
    pPopupMenu->EnableMenuItem(ID_REPORTSMENU, MF_BYCOMMAND | (m_bOpen ? MF_ENABLED : MF_GRAYED));
    goto exit;
  }  // View menu

  if (uiMenuID == ID_SUBVIEWMENU) {
    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_UNSAVED, MF_BYCOMMAND |
                              m_bUnsavedDisplayed ? MF_CHECKED : MF_UNCHECKED);

    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOW_ALL_EXPIRY, MF_BYCOMMAND |
                              m_bExpireDisplayed ? MF_CHECKED : MF_UNCHECKED);

    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOW_FOUNDENTRIES, MF_BYCOMMAND |
                              m_bFindFilterDisplayed ? MF_CHECKED : MF_UNCHECKED);
    goto exit;
  } // Subview

  if (uiMenuID == ID_FILTERMENU) {
    pPopupMenu->ModifyMenu(1, MF_BYPOSITION,
                           m_bFilterActive ? ID_MENUITEM_CLEARFILTER : ID_MENUITEM_APPLYFILTER,
                           m_bFilterActive ? CS_CLEARFILTERS : CS_SETFILTERS);
    goto exit;
  }

  if (bItemSelected) {
    pci = getSelectedItem();
    ASSERT(pci != NULL);
    if (pci->IsDependent())
      pbci = m_core.GetBaseEntry(pci);
  }

  // Save original entry type before possibly changing pci
  const CItemData::EntryType etype_original = 
          pci == NULL ? CItemData::ET_INVALID : pci->GetEntryType();

  if (bTreeView) {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    bGroupSelected = (hi != NULL && !m_ctlItemTree.IsLeaf(hi));
  }

  // If Edit menu selected
  if (uiMenuID == ID_EDITMENU) {
    // Delete all entries and rebuild depending on group/entry selected
    // and, if entry, if fields are non-empty
    UINT uiCount = pPopupMenu->GetMenuItemCount();
    ASSERT((int)uiCount >= 0);

    for (UINT ui = 0; ui < uiCount; ui++) {
      pPopupMenu->RemoveMenu(0, MF_BYPOSITION);
    }

    /*
     Three scenarios:
     Note: If the database is read-only, anything modifying it is not added.
     1. Nothing selected
       Tree View - Add Entry, Sep, Add Group, Sep, Clear Clipboard
       List View - Add Entry, Sep, Clear Clipboard
     2. Group selected (Tree view only)
       Add Entry, Sep, <Group Items>, Sep, Clear Clipboard
       Add Group, Duplicate Group, (Un)Protect entries
     3. Entry selected
       Tree View - <Entry Items>, Sep, Add Group, Sep, Clear Clipboard, Sep
                   Entry functions (copy to clipboard, browse, run etc)
       List View - <Entry Items>, Sep, Clear Clipboard, Sep
                   Entry functions (copy to clipboard, browse, run etc)
    */

    // Scenario 1 - Nothing selected
    //   Tree View - Add Entry, Sep, Add Group, Sep, Clear Clipboard
    //   List View - Add Entry, Sep, Clear Clipboard
    if (pci == NULL && !bGroupSelected) {
      if (m_IsListView) {
        if (!bReadOnly) {
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_ADD, tc_dummy);
          pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
        }
      } else {
        if (!bReadOnly) {
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_ADD, tc_dummy);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_ADDGROUP, tc_dummy);
          pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
        }
      }
      
      // Add Find Next/Previous if find entries were found
      if (m_FindToolBar.EntriesFound()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FIND, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FINDUP, tc_dummy);
      } 

      // Only add "Find..." if find filter not active
      if (!(m_bFilterActive && m_bFindFilterDisplayed)) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                ID_MENUITEM_FINDELLIPSIS, tc_dummy);
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }

      if (m_core.AnyToUndo() || m_core.AnyToRedo()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_UNDO, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_REDO, tc_dummy);
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
      goto exit;
    }

    // Scenario 2 - Group selected (Tree view only)
    //   Add Entry, Find, Sep, <Group Items>, Sep, Clear Clipboard
    if (!m_IsListView && bGroupSelected) {
      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_ADD, tc_dummy);
      }

      // Add Find Next/Previous if find entries were found
      if (m_FindToolBar.EntriesFound()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FIND, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FINDUP, tc_dummy);
      } 

      // Only add "Find..." if find filter not active
      if (!(m_bFilterActive && m_bFindFilterDisplayed)) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                ID_MENUITEM_FINDELLIPSIS, tc_dummy);
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_GROUPENTER, tc_dummy);
      
      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_DELETEGROUP, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_RENAMEGROUP, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_ADDGROUP, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_DUPLICATEGROUP, tc_dummy);
        int numProtected, numUnprotected;
        bool bProtect = GetSubtreeEntriesProtectedStatus(numProtected, numUnprotected);
        if (bProtect) {
          if (numUnprotected > 0)
            pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                   ID_MENUITEM_PROTECTGROUP, tc_dummy);
          if (numProtected > 0)
            pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                   ID_MENUITEM_UNPROTECTGROUP, tc_dummy);
        }
      }

      // Only allow export of a group to anything if non-empty
      if (m_ctlItemTree.CountLeafChildren(m_ctlItemTree.GetSelectedItem()) != 0) {
        CMenu GGsubMenu;
        CString GGstr;
        GGsubMenu.CreatePopupMenu();
        // Re-use entry menu texts
        GGstr.LoadString(IDS_EXPORTENT2PLAINTEXT);
        GGsubMenu.AppendMenu(MF_ENABLED | MF_STRING,
          ID_MENUITEM_EXPORTGRP2PLAINTEXT, GGstr);
        GGstr.LoadString(IDS_EXPORTENT2XML);
        GGsubMenu.AppendMenu(MF_ENABLED | MF_STRING,
          ID_MENUITEM_EXPORTGRP2XML, GGstr);
        GGstr.LoadString(IDS_EXPORTENT2DB);
        GGsubMenu.AppendMenu(MF_ENABLED | MF_STRING,
          ID_MENUITEM_EXPORTGRP2DB, GGstr);
        GGstr.LoadString(IDS_EXPORTGRPMENU);
        pPopupMenu->AppendMenu(MF_POPUP, (UINT_PTR)GGsubMenu.Detach(), GGstr);
      }

      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      if (m_core.AnyToUndo() || m_core.AnyToRedo()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_UNDO, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_REDO, tc_dummy);
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING, ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
      goto exit;
    }

    // Scenario 3 - Entry selected
    //   Tree View - <Entry Items>, Sep, Add Entry, Add Group, Sep, Clear Clipboard, Sep,
    //               Entry functions (copy to clipboard, browse, run etc)
    //   List View - <Entry Items>, Sep, Add Entry, Sep, Clear Clipboard, Sep,
    //               Entry functions (copy to clipboard, browse, run etc)
    if (pci != NULL) {
      // Entry is selected

      // Deal with multi-selection
      // More than 2 is meaningless in List view
      if (m_IsListView && m_ctlItemList.GetSelectedCount() > 2)
        return;

      // If exactly 2 selected - show compare entries menu
      if (m_IsListView  && m_ctlItemList.GetSelectedCount() == 2) {
        CString cs_txt(MAKEINTRESOURCE(ID_MENUITEM_COMPARE_ENTRIES));
        cs_txt.TrimLeft();  // Remove leading newline
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COMPARE_ENTRIES, cs_txt);
        return;
      }

      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_ADD, tc_dummy);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ((m_core.IsReadOnly() || pci->IsProtected()) ?
                              ID_MENUITEM_VIEWENTRY : ID_MENUITEM_EDITENTRY),
                             tc_dummy);

      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_DELETEENTRY, tc_dummy);
        if (!m_IsListView) {
          // Rename not valid in List View
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_RENAMEENTRY, tc_dummy);
        }
      }

      // Only have Find Next/Previous if find entries were found
      if (m_FindToolBar.EntriesFound()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FIND, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FINDUP, tc_dummy);
      }

      // Only add "Find..." if find filter not active
      if (!(m_bFilterActive && m_bFindFilterDisplayed)) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                ID_MENUITEM_FINDELLIPSIS, tc_dummy);
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }

      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_DUPLICATEENTRY, tc_dummy);
        if (!m_IsListView) {
          pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_ADDGROUP, tc_dummy);
        }
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }
      
      if (m_core.AnyToUndo() || m_core.AnyToRedo()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_UNDO, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_REDO, tc_dummy);
        pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_COPYPASSWORD, tc_dummy);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_PASSWORDSUBSET, tc_dummy);

      if (!pci->IsFieldValueEmpty(CItemData::USER, pbci))
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYUSERNAME, tc_dummy);

      if (!pci->IsFieldValueEmpty(CItemData::NOTES, pbci))
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYNOTESFLD, tc_dummy);

      /*
      *  Rules:
      *    1. If email field is not empty, add email menuitem.
      *    2. If URL is not empty and is NOT an email address, add browse menuitem
      *    3. If URL is not empty and is an email address, add email menuitem
      *       (if not already added)
      */
      bool bAddCopyEmail = !pci->IsFieldValueEmpty(CItemData::EMAIL, pbci);
      bool bAddSendEmail = bAddCopyEmail || 
               (!pci->IsFieldValueEmpty(CItemData::URL, pbci) && pci->IsURLEmail(pbci));
      bool bAddURL = !pci->IsFieldValueEmpty(CItemData::URL, pbci);

      // Add copies in order
      if (bAddURL) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYURL, tc_dummy);
      }

      if (bAddCopyEmail) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYEMAIL, tc_dummy);
      }

      if (!pci->IsFieldValueEmpty(CItemData::RUNCMD, pbci))
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYRUNCOMMAND, tc_dummy);

      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);

      // Add actions in order
      if (bAddURL && !pci->IsURLEmail(pbci)) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_BROWSEURL, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_BROWSEURLPLUS, tc_dummy);
      }

      if (bAddSendEmail) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_SENDEMAIL, tc_dummy);
      }

      if (!pci->IsFieldValueEmpty(CItemData::RUNCMD, pbci)) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_RUNCOMMAND, tc_dummy);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_AUTOTYPE, tc_dummy);

      switch (etype_original) {
        case CItemData::ET_NORMAL:
        case CItemData::ET_SHORTCUTBASE:
          // Allow creation of a shortcut
          if (!bReadOnly) {
            pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                   ID_MENUITEM_CREATESHORTCUT, tc_dummy);
          }
          break;
        case CItemData::ET_ALIASBASE:
          // Can't have a shortcut to an AliasBase entry + can't goto base
          break;
        case CItemData::ET_ALIAS:
        case CItemData::ET_SHORTCUT:
          // Allow going to/editing the appropriate base entry
          if (m_bFilterActive) {
            // If a filter is active, then might not be able to go to
            // entry's base entry as not in Tree or List view
            pws_os::CUUID uuidBase = pci->GetBaseUUID();
            auto iter = m_MapEntryToGUI.find(uuidBase);
            ASSERT(iter != m_MapEntryToGUI.end());
            if (iter->second.list_index != -1) {
              pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                ID_MENUITEM_GOTOBASEENTRY, tc_dummy);
              pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                ID_MENUITEM_EDITBASEENTRY, tc_dummy);
            } else {
              pPopupMenu->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
              pPopupMenu->RemoveMenu(ID_MENUITEM_EDITBASEENTRY, MF_BYCOMMAND);
            }
          }
         break;
        default:
          ASSERT(0);
      }

      if (pci->IsShortcut() ? pbci->HasAttRef() : pci->HasAttRef()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_VIEWATTACHMENT, tc_dummy);
      }

      CMenu EEsubMenu;
      CString EEstr;
      EEsubMenu.CreatePopupMenu();
      EEstr.LoadString(IDS_EXPORTENT2PLAINTEXT);
      EEsubMenu.AppendMenu(MF_ENABLED | MF_STRING,
                           ID_MENUITEM_EXPORTENT2PLAINTEXT, EEstr); 
      EEstr.LoadString(IDS_EXPORTENT2XML);
      EEsubMenu.AppendMenu(MF_ENABLED | MF_STRING,
                          ID_MENUITEM_EXPORTENT2XML, EEstr);
      EEstr.LoadString(IDS_EXPORTENT2DB);
      EEsubMenu.AppendMenu(MF_ENABLED | MF_STRING,
                           ID_MENUITEM_EXPORTENT2DB, EEstr);
      EEstr.LoadString(IDS_EXPORTENTMENU);
      pPopupMenu->AppendMenu(MF_POPUP, (UINT_PTR)EEsubMenu.Detach(), EEstr);

      if (pci->IsShortcut() ? pbci->HasAttRef() : pci->HasAttRef()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
          ID_MENUITEM_EXPORT_ATTACHMENT, tc_dummy);
      }

      if (!bReadOnly && etype_original != CItemData::ET_SHORTCUT)
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               pci->IsProtected() ? ID_MENUITEM_UNPROTECT : ID_MENUITEM_PROTECT,
                               tc_dummy);

      // Tree view and command flag present only
      if (!m_IsListView && m_bCompareEntries &&
           etype_original != CItemData::ET_SHORTCUT)
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COMPARE_ENTRIES, tc_dummy);
    } else {
      // Must be List view with no entry selected
      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
    }
  }  // Edit menu

exit:
  SetUpMenuStrings(pPopupMenu);
}

// Helps with MRU by allowing ON_UPDATE_COMMAND_UI
void DboxMain::OnInitMenuPopup(CMenu* pPopupMenu, UINT, BOOL)
{
  // Original OnInitMenu code
  // All main menus are POPUPs (see PasswordSafe2.rc2)

  // Don't do the old OnInitMenu processing if the right-click context menu
  // (IDR_POPEDITMENU or IDR_POPEDITGROUP) is being processed. Only for the Main Menu
  // (ID_EDITMENU).
  ASSERT_VALID(pPopupMenu);

  MENUINFO minfo = {0};
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;

  VERIFY(pPopupMenu->GetMenuInfo(&minfo));

  // Need to update the main menu if the shortcuts have been changed or
  // if the Edit or View menu.  The Edit menu is completely rebuilt each time
  // based on the view and if anything is selected. The View menu has one item
  // (ShowFindToolbar) that is removed if already displayed

  bool bDoShortcuts(false);
  switch (minfo.dwMenuData) {
    case ID_FILEMENU:
    case ID_EXPORTMENU:
    case ID_IMPORTMENU:
    case ID_EDITMENU:
    case ID_VIEWMENU:
    case ID_SUBVIEWMENU:
    case ID_FILTERMENU:
    case ID_EXPORTENTMENU:
    case ID_EXPORTGROUPMENU:
    case ID_CHANGEFONTMENU:
    case ID_REPORTSMENU:
    case ID_MANAGEMENU:
    case ID_HELPMENU:  //main menu items' shortcut should be always updated because keyboard layout could be changed
      bDoShortcuts = true;
      break;
    default:
      break;
  }

  if (bDoShortcuts ||
      minfo.dwMenuData == ID_FILEMENU ||
      minfo.dwMenuData == ID_EDITMENU ||
      minfo.dwMenuData == ID_VIEWMENU)
    CustomiseMenu(pPopupMenu, (UINT)minfo.dwMenuData, bDoShortcuts);

  if (minfo.dwMenuData == ID_MANAGEMENU) {
    // Process the Change Language sub-menu
    // First get its position and then build it
    // If no entries added (no language DLLs or back level language DLLs),
    // disable the menu item.
    int iLangPos = app.FindMenuItem(pPopupMenu, ID_LANGUAGEMENU);
    if (iLangPos >= 0) {
      BOOL brc;
      CMenu *pSubMenu = pPopupMenu->GetSubMenu(iLangPos);
      brc = ProcessLanguageMenu(pSubMenu);
      pPopupMenu->EnableMenuItem(iLangPos, MF_BYPOSITION | (brc ? MF_ENABLED : MF_GRAYED));
    }
  }

  // http://www4.ncsu.edu:8030/~jgbishop/codetips/dialog/updatecommandui_menu.html
  // This code comes from the MFC Documentation, and is adapted from
  // CFrameWnd::OnInitMenuPopup() in WinFrm.cpp.
  CCmdUI state; // Check the enabled state of various menu items
  state.m_pMenu = pPopupMenu;

  ASSERT(state.m_pOther == NULL);
  ASSERT(state.m_pParentMenu == NULL);

  // Is the menu in question a popup in the top-level menu? If so, set m_pOther
  // to this menu. Note that m_pParentMenu == NULL indicates that the menu is a
  // secondary popup.
  CMenu *hParentMenu;
  if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu) {
    state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
  } else
  if ((hParentMenu = this->GetMenu()) != NULL) {
    CWnd* pParent = this;
    // Child windows don't have menus--need to go to the top!
    if (pParent != NULL && (hParentMenu = pParent->GetMenu()) != NULL) {
      int nIndexMax = hParentMenu->GetMenuItemCount();
      for (int nIndex = 0; nIndex < nIndexMax; nIndex++) {
        CMenu *submenu = hParentMenu->GetSubMenu(nIndex);
        if (submenu != NULL && submenu->m_hMenu == pPopupMenu->m_hMenu) {
          // When popup is found, m_pParentMenu is containing menu.
          state.m_pParentMenu = CMenu::FromHandle(hParentMenu->GetSafeHmenu());
          break;
        }
      }
    }
  }

  state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
  for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++) {
    state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
    if (state.m_nID == 0)
      continue; // Menu separator or invalid cmd - ignore it.

    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pMenu != NULL);
    if (state.m_nID == (UINT)-1) {
      // Possibly a popup menu, route to first item of that popup.
      state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
      if (state.m_pSubMenu == NULL ||
          (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
          state.m_nID == (UINT)-1) {
        continue; // First item of popup can't be routed to.
      }
      state.DoUpdate(this, TRUE); // Popups are never auto disabled.
    } else {
      // Normal menu item.
      // Auto enable/disable if frame window has m_bAutoMenuEnable
      // set and command is _not_ a system command.
      state.m_pSubMenu = NULL;
      state.DoUpdate(this, FALSE);
    }

    // Adjust for menu deletions and additions.
    UINT nCount = pPopupMenu->GetMenuItemCount();
    if (nCount < state.m_nIndexMax) {
      state.m_nIndex -= (state.m_nIndexMax - nCount);
      while(state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID) {
        state.m_nIndex++;
      }
    }
    state.m_nIndexMax = nCount;
  }

  // Ignore SystemTray popup menu
  if (!m_bImageInLV || 
       (minfo.dwMenuData != IDR_POPTRAY && 
        minfo.dwMenuData != IDR_POPCOPYTOORIGINAL && 
        minfo.dwMenuData != IDR_POPEDITVIEWORIGINAL))
    return;

  // System Tray Popup menu processing only
  SecureZeroMemory(&minfo, sizeof(minfo));
  minfo.cbSize = sizeof(minfo);
  minfo.fMask = MIM_STYLE;
  minfo.dwStyle = MNS_CHECKORBMP | MNS_AUTODISMISS;

  VERIFY(pPopupMenu->SetMenuInfo(&minfo));

  MENUITEMINFO miteminfo = {0};
  CRUEItemData *pmd;

  UINT uiCount = pPopupMenu->GetMenuItemCount();
  ASSERT((int)uiCount >= 0);

  for (UINT pos = 0; pos < uiCount; pos++) {
    SecureZeroMemory(&miteminfo, sizeof(miteminfo));
    miteminfo.cbSize = sizeof(miteminfo);
    miteminfo.fMask = MIIM_FTYPE | MIIM_DATA;

    VERIFY(pPopupMenu->GetMenuItemInfo(pos, &miteminfo, TRUE));

    pmd = (CRUEItemData *)miteminfo.dwItemData;
    if (pmd && pmd->IsRUEID() && !(miteminfo.fType & MFT_OWNERDRAW) &&
        pmd->nImage >= 0) {
      SecureZeroMemory(&miteminfo, sizeof(miteminfo));
      miteminfo.cbSize = sizeof(miteminfo);
      miteminfo.fMask = MIIM_FTYPE | MIIM_BITMAP;
      miteminfo.hbmpItem = HBMMENU_CALLBACK;
      miteminfo.fType = MFT_STRING;

      VERIFY(pPopupMenu->SetMenuItemInfo(pos, &miteminfo, TRUE));
    }
  }
}

// Called when right-click is invoked in the client area of the window.
void DboxMain::OnContextMenu(CWnd * /* pWnd */, CPoint screen)
{
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;

  CPoint client;
  int item = -1;
  const CItemData *pci(NULL), *pbci(NULL);
  CMenu menu;

  MENUINFO minfo ={0};
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;

  // Note if point = (-1, -1) then invoked via keyboard.
  // Need coordinates of current selected item instead of mouse position when message sent
  bool bKeyboard = (screen.x == -1 && screen.y == -1);

  CPoint mp; // Screen coords (from "message point" or via Shift+F10 selected item
  CRect rect, appl_rect;

  // Get client window position
  if (bKeyboard) {
    CRect r;
    if (m_ctlItemList.IsWindowVisible()) {
      POSITION pos = m_ctlItemList.GetFirstSelectedItemPosition();
      if (pos == NULL)
        return;  // Nothing selected!

      int nIndex = m_ctlItemList.GetNextSelectedItem(pos);
      m_ctlItemList.GetItemRect(nIndex, &r, LVIR_LABEL);
      m_ctlItemList.ClientToScreen(&r);
    } else
    if (m_ctlItemTree.IsWindowVisible()) {
      HTREEITEM hItem = m_ctlItemTree.GetSelectedItem();
      if (hItem == NULL)
        return;  // Nothing selected!

      m_ctlItemTree.GetItemRect(hItem, &r, TRUE);
      m_ctlItemTree.ClientToScreen(&r);
    }
    mp.x = (r.left + r.right) / 2;
    mp.y = (r.top + r.bottom) / 2;
    screen = mp;  // In screen coords
  } else {
    mp = ::GetMessagePos();
  }
  GetWindowRect(&appl_rect);
  m_MainToolBar.GetWindowRect(&rect);

  // RClick over Main Toolbar - allow Customize Main toolbar
  if (m_MainToolBar.IsVisible() &&
      mp.x > appl_rect.left && mp.x < appl_rect.right &&
      mp.y > rect.top && mp.y < rect.bottom) {
    if (menu.LoadMenu(IDR_POPCUSTOMIZETOOLBAR)) {
      minfo.dwMenuData = IDR_POPCUSTOMIZETOOLBAR;
      VERIFY(menu.SetMenuInfo(&minfo));

      CMenu *pPopup = menu.GetSubMenu(0);
      ASSERT_VALID(pPopup);

      // Use this DboxMain for commands
      pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
    }
    return;
  }

  if (!m_bOpen)
    return;

  client = screen;
  // RClick over ListView
  if (m_ctlItemList.IsWindowVisible()) {
    // currently in flattened list view.
    m_ctlItemList.GetWindowRect(&rect);
    if (mp.x < rect.left || mp.x > appl_rect.right ||
        mp.y < rect.top || mp.y > rect.bottom) {
      // But not in the window
      return;
    }

    m_ctlItemList.ScreenToClient(&client);
    item = m_ctlItemList.HitTest(client);
    if (item < 0)
      return; // right click on empty list

    pci = (CItemData *)m_ctlItemList.GetItemData(item);
    UpdateToolBarForSelectedItem(pci);
    if (SelectEntry(item) == 0) {
      return;
    }
    m_ctlItemList.SetFocus();
  }

  // RClick over TreeView
  if (m_ctlItemTree.IsWindowVisible()) {
    // currently in tree view
    m_ctlItemTree.GetWindowRect(&rect);
    if (mp.x < rect.left || mp.x > appl_rect.right ||
        mp.y < rect.top || mp.y > rect.bottom) {
      // But not in the window
      return;
    }
    ASSERT(m_ctlItemTree.IsWindowVisible());

    m_ctlItemTree.ScreenToClient(&client);
    HTREEITEM ti = m_ctlItemTree.HitTest(client);
    if (ti != NULL) {
      pci = (CItemData *)m_ctlItemTree.GetItemData(ti);
      UpdateToolBarForSelectedItem(pci);
      if (pci != NULL) {
        // right-click was on an item (LEAF of some kind: normal, alias, shortcut)
        DisplayInfo *pdi = GetEntryGUIInfo(*pci);
        ASSERT(pdi->tree_item == ti);
        item = pdi->list_index;
        m_ctlItemTree.SelectItem(ti); // So that OnEdit gets the right one
      } else {
        // right-click was on a group (GROUP)
        m_ctlItemTree.SelectItem(ti);
        if (menu.LoadMenu(IDR_POPEDITGROUP)) {
          minfo.dwMenuData = IDR_POPEDITGROUP;
          menu.SetMenuInfo(&minfo);

          CMenu *pPopup = menu.GetSubMenu(0);
          ASSERT_VALID(pPopup);
          m_TreeViewGroup = m_mapTreeItemToGroup[ti];

          // Deal with empty groups before removing protect menu items
          if (m_ctlItemTree.CountLeafChildren(ti) == 0) {
            // This is an empty group or, if it has sub-groups, they are
            // all empty too - so remove export to anything
            CMenu *pExportPopup = pPopup->GetSubMenu(7);
            pExportPopup->RemoveMenu(ID_MENUITEM_EXPORTGRP2PLAINTEXT, MF_BYCOMMAND);
            pExportPopup->RemoveMenu(ID_MENUITEM_EXPORTGRP2XML, MF_BYCOMMAND);
            pExportPopup->RemoveMenu(ID_MENUITEM_EXPORTGRP2DB, MF_BYCOMMAND);

            // Since nothing left, remove this pop completely
            pPopup->RemoveMenu(7, MF_BYPOSITION);
          }

          int numProtected, numUnprotected;
          bool bProtect = GetSubtreeEntriesProtectedStatus(numProtected, numUnprotected);

          if (!bProtect || numUnprotected == 0)
            pPopup->RemoveMenu(ID_MENUITEM_PROTECTGROUP, MF_BYCOMMAND);
          if (!bProtect || numProtected == 0)
            pPopup->RemoveMenu(ID_MENUITEM_UNPROTECTGROUP, MF_BYCOMMAND);

          // Use this DboxMain for commands
          pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
        }
      }
    } else {
      // not over anything
      if (menu.LoadMenu(IDR_POPTREE)) {  // "Add Group"
        minfo.dwMenuData = IDR_POPTREE;
        VERIFY(menu.SetMenuInfo(&minfo));
        CMenu *pPopup = menu.GetSubMenu(0);
        ASSERT_VALID(pPopup);
        
        ti = m_ctlItemTree.GetSelectedItem();
        if (ti == NULL || (ti != NULL && (CItemData *)m_ctlItemTree.GetItemData(ti) != NULL))
          pPopup->RemoveMenu(ID_MENUITEM_DUPLICATEGROUP, MF_BYCOMMAND);

        // Use this DboxMain for commands
        m_bWhitespaceRightClick = true;
        pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
      }
    }
    m_ctlItemTree.SetFocus();
  } // tree view handling

  // RClick over an entry
  if (item >= 0) {
    CMenu *pPopup(NULL);

    // More than 2 is meaningless in List view
    if (m_IsListView && m_ctlItemList.GetSelectedCount() > 2)
      return;

    // Only compare entries if 2 entries are selected - List view only
    if (m_IsListView && m_ctlItemList.GetSelectedCount() == 2) {
      VERIFY(menu.LoadMenu(IDR_POPCOMPAREENTRIES));

      minfo.dwMenuData = IDR_POPCOMPAREENTRIES;
      VERIFY(menu.SetMenuInfo(&minfo));

      pPopup = menu.GetSubMenu(0);
      ASSERT_VALID(pPopup);

      // Use this DboxMain for commands
      pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
      return;
    }

    ASSERT(pci != NULL);
    VERIFY(menu.LoadMenu(IDR_POPEDITMENU));

    minfo.dwMenuData = IDR_POPEDITMENU;
    VERIFY(menu.SetMenuInfo(&minfo));

    pPopup = menu.GetSubMenu(0);
    ASSERT_VALID(pPopup);

    if (m_core.IsReadOnly() || pci->IsProtected()) {
      CString csMenu(MAKEINTRESOURCE(ID_MENUITEM_VIEWENTRY));
      pPopup->ModifyMenu(ID_MENUITEM_EDITENTRY, MF_BYCOMMAND, ID_MENUITEM_VIEWENTRY, csMenu);
    } else {
      CString csMenu(MAKEINTRESOURCE(ID_MENUITEM_EDITENTRY));
      pPopup->ModifyMenu(ID_MENUITEM_EDITENTRY, MF_BYCOMMAND, ID_MENUITEM_EDITENTRY, csMenu);
    }

    const CItemData::EntryType etype_original = pci->GetEntryType();

    if (pci->IsDependent()) {
      pbci = m_core.GetBaseEntry(pci);
    }

    // Get list of UUIDs of a base's dependants.
    // Add entry's [g:t:u] and its display position into a map, which will
    // sort the entries.
    UUIDVector tlist;
    std::map<StringX, int> mapDependants;
    int iMaxVisibleEntries(0);

    if (etype_original == CItemData::ET_SHORTCUTBASE || etype_original == CItemData::ET_ALIASBASE) {
      m_core.GetAllDependentEntries(pci->GetUUID(), tlist,
        etype_original == CItemData::ET_SHORTCUTBASE ? CItemData::ET_SHORTCUT : CItemData::ET_ALIAS);

      for (size_t i = 0; i < tlist.size(); i++) {
        ItemListIter iter = Find(tlist[i]);
        DisplayInfo *pdi = GetEntryGUIInfo(iter->second, true);

        // Check in Tree (filter may be active and so may not be there)
        int index = pdi->list_index;
        if (pdi == NULL) {
          index = -1;
        } else {
          iMaxVisibleEntries++;
        }

        StringX sxGroup = iter->second.GetGroup();
        StringX sxTitle = iter->second.GetTitle();
        StringX sxUser = iter->second.GetUser();
        StringX sxEntry;
        Format(sxEntry, L"\xab%s\xbb \xab%s\xbb \xab%s\xbb", sxGroup.c_str(),
          sxTitle.c_str(), sxUser.c_str());

        // Add to map which will sort by entry name
        mapDependants.insert(make_pair(sxEntry, pdi->list_index));
      }
    }

    // Set up "Go to Aliases/Shortcuts"
    int nID = ID_MENUITEM_GOTODEPENDANT1;
    m_vGotoDependants.clear();

    // Find popup menu for this - not can't have a command ID for context menu
    // popups and so must go by position. This is after "Edit Base Entry"
    int isubmenu_pos = app.FindMenuItem(pPopup, ID_MENUITEM_EDITBASEENTRY) + 1;

    // Get pointer to this popup
    CMenu *pGotoDpdPopup = pPopup->GetSubMenu(isubmenu_pos);

    switch (etype_original) {
      case CItemData::ET_SHORTCUTBASE:
      {
        // Need to change menu text
        CString csMenuText1(MAKEINTRESOURCE(IDS_GOTOSHORTCUTS));
        pPopup->ModifyMenu(isubmenu_pos, MF_BYPOSITION, 0, csMenuText1);

        // Remove dummy separator in menu definition
        pGotoDpdPopup->RemoveMenu(0, MF_BYPOSITION);

        // Got through shortcuts and add to popup menu
        // Fill the dependant vector with its corresponding position to be used
        // by the command to select it if the user selects it
        for (auto iter = mapDependants.begin(); iter != mapDependants.end(); iter++) {
          StringX sxEntry = iter->first;

          if (iter->second == -1 || nID > ID_MENUITEM_GOTODEPENDANTMAX) {
            // Dependant is NOT available or reached maximum
            pGotoDpdPopup->InsertMenu((UINT)-1, MF_BYPOSITION | MF_STRING | MF_GRAYED,
                                      0, sxEntry.c_str());
          } else {
            // Dependant is visible
            m_vGotoDependants.push_back(iter->second);
            pGotoDpdPopup->InsertMenu((UINT)-1, MF_BYPOSITION | MF_STRING, nID, sxEntry.c_str());
            nID++;
          }
        }

        // As a base entry - remove menu items for a base
        pPopup->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_EDITBASEENTRY, MF_BYCOMMAND);
        break;
      }
      case CItemData::ET_NORMAL:
        // As a normal entry - remove menu items for a base
        pPopup->RemoveMenu(isubmenu_pos, MF_BYPOSITION);
        pPopup->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_EDITBASEENTRY, MF_BYCOMMAND);
        break;
      case CItemData::ET_ALIASBASE:
        // Remove dummy separator in menu definition
        pGotoDpdPopup->RemoveMenu(0, MF_BYPOSITION);

        // Got throogh aliases and add to popup menu
        // Fill the dependant vector with its corresponding position to be used
        // by the command to select it if the user selects it
        for (auto iter = mapDependants.begin(); iter != mapDependants.end(); iter++) {
          StringX sxEntry = iter->first;
          if (iter->second == -1 || nID > ID_MENUITEM_GOTODEPENDANTMAX) {
            // Dependant is NOT available or reached maximum
            pGotoDpdPopup->InsertMenu((UINT)-1, MF_BYPOSITION | MF_STRING | MF_GRAYED,
              0, sxEntry.c_str());
          } else {
            // Dependant is visible
            m_vGotoDependants.push_back(iter->second);
            pGotoDpdPopup->InsertMenu((UINT)-1, MF_BYPOSITION | MF_STRING, nID, sxEntry.c_str());
            nID++;
          }
        }

        // As a base entry - remove menu items for a base - also can't have a shoretcut to an alias
        pPopup->RemoveMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_EDITBASEENTRY, MF_BYCOMMAND);
        break;
      case CItemData::ET_ALIAS:
      case CItemData::ET_SHORTCUT:
        pPopup->RemoveMenu(isubmenu_pos, MF_BYPOSITION);
        pPopup->RemoveMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND);
        if (m_bFilterActive) {
          // If a filter is active, then might not be able to go to
          // entry's base entry as not in Tree or List view
          pws_os::CUUID uuidBase = pci->GetBaseUUID();
          auto iter = m_MapEntryToGUI.find(uuidBase);
          ASSERT(iter != m_MapEntryToGUI.end());
          if (iter->second.list_index == -1) {
            pPopup->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
            pPopup->RemoveMenu(ID_MENUITEM_EDITBASEENTRY, MF_BYCOMMAND);
          }
        } else {
          if (m_core.IsReadOnly() || pbci->IsProtected()) {
            CString csMenuText(MAKEINTRESOURCE(IDS_VIEWBASEENTRY));
            pPopup->ModifyMenu(ID_MENUITEM_EDITBASEENTRY, MF_BYCOMMAND, 
                               ID_MENUITEM_EDITBASEENTRY, csMenuText);
          }
        }
        break;
      default:
        ASSERT(0);
    }

    // Clear up list of dependant UUIDs and map between the dependant [g:t:u] and
    // display position in list
    tlist.clear();
    mapDependants.clear();

    if (m_core.IsReadOnly() || pci->IsShortcut()) {
      pPopup->RemoveMenu(ID_MENUITEM_PROTECT, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_UNPROTECT, MF_BYCOMMAND);
    } else {
      pPopup->RemoveMenu(pci->IsProtected() ? ID_MENUITEM_PROTECT : ID_MENUITEM_UNPROTECT, MF_BYCOMMAND);
    }

    bool bCopyEmail = !pci->IsFieldValueEmpty(CItemData::EMAIL, pbci);
    bool bSendEmail = bCopyEmail ||
               (!pci->IsFieldValueEmpty(CItemData::URL, pbci) && pci->IsURLEmail(pbci));
    bool bUseURL = !pci->IsFieldValueEmpty(CItemData::URL, pbci) && !pci->IsURLEmail(pbci);

    if (pci->IsFieldValueEmpty(CItemData::USER, pbci))
      pPopup->RemoveMenu(ID_MENUITEM_COPYUSERNAME, MF_BYCOMMAND);

    if (pci->IsFieldValueEmpty(CItemData::NOTES, pbci))
      pPopup->RemoveMenu(ID_MENUITEM_COPYNOTESFLD, MF_BYCOMMAND);

    if (!bCopyEmail)
      pPopup->RemoveMenu(ID_MENUITEM_COPYEMAIL, MF_BYCOMMAND);

    if (!bSendEmail)
      pPopup->RemoveMenu(ID_MENUITEM_SENDEMAIL, MF_BYCOMMAND);

    if (pci->IsFieldValueEmpty(CItemData::URL, pbci))
      pPopup->RemoveMenu(ID_MENUITEM_COPYURL, MF_BYCOMMAND);

    if (!bUseURL) {
      pPopup->RemoveMenu(ID_MENUITEM_BROWSEURL, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_BROWSEURLPLUS, MF_BYCOMMAND);
    }

    if (pci->IsFieldValueEmpty(CItemData::RUNCMD, pbci)) {
      pPopup->RemoveMenu(ID_MENUITEM_COPYRUNCOMMAND, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_RUNCOMMAND, MF_BYCOMMAND);
    }

    if (!pci->HasAttRef()) {
      pPopup->RemoveMenu(ID_MENUITEM_EXPORT_ATTACHMENT, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_VIEWATTACHMENT, MF_BYCOMMAND);
    }

    if (m_IsListView) {
      // Rename not valid in List View
      pPopup->RemoveMenu(ID_MENUITEM_RENAMEENTRY, MF_BYCOMMAND);
    }

    // Since an entry - remove Duplicate Group
    pPopup->RemoveMenu(ID_MENUITEM_DUPLICATEGROUP, MF_BYCOMMAND);

    // Remove if in List View, not allowed in Tree view or a shortcut
    if (m_IsListView || !m_bCompareEntries ||
        etype_original == CItemData::ET_SHORTCUT)
        pPopup->RemoveMenu(ID_MENUITEM_COMPARE_ENTRIES, MF_BYCOMMAND);

    // Use this DboxMain for commands
    pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
  } // if (item >= 0)
}

void DboxMain::SetupSpecialShortcuts()
{
  MapMenuShortcutsIter iter, iter_entry, iter_group;

  // Set up some shortcuts based on the main entry

  iter = m_MapMenuShortcuts.find(ID_MENUITEM_DELETE);

  // Save for CTreeCtrl & CListCtrl PreTranslateMessage
  if (iter != m_MapMenuShortcuts.end()) {
    iter_entry = m_MapMenuShortcuts.find(ID_MENUITEM_DELETEENTRY);
    iter_entry->second.SetKeyFlags(iter->second);
    iter_group = m_MapMenuShortcuts.find(ID_MENUITEM_DELETEGROUP);
    iter_group->second.SetKeyFlags(iter->second);

    m_wpDeleteMsg = ((iter->second.cPWSModifier & PWS_HOTKEYF_ALT) == PWS_HOTKEYF_ALT) ?
                           WM_SYSKEYDOWN : WM_KEYDOWN;
    m_wpDeleteKey = iter->second.siVirtKey;
    m_bDeleteCtrl = (iter->second.cPWSModifier  & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL;
    m_bDeleteShift = (iter->second.cPWSModifier & PWS_HOTKEYF_SHIFT) == PWS_HOTKEYF_SHIFT;
  } else {
    m_wpDeleteKey = 0;
  }

  // Find Rename Shortcut
  iter = m_MapMenuShortcuts.find(ID_MENUITEM_RENAME);
  
  // Save for CTreeCtrl::PreTranslateMessage (not CListCtrl)
  if (iter != m_MapMenuShortcuts.end()) {
    iter_entry = m_MapMenuShortcuts.find(ID_MENUITEM_RENAMEENTRY);
    iter_entry->second.SetKeyFlags(iter->second);
    iter_group = m_MapMenuShortcuts.find(ID_MENUITEM_RENAMEGROUP);
    iter_group->second.SetKeyFlags(iter->second);

    m_wpRenameMsg = ((iter->second.cPWSModifier & PWS_HOTKEYF_ALT) == PWS_HOTKEYF_ALT) ?
                           WM_SYSKEYDOWN : WM_KEYDOWN;
    m_wpRenameKey = iter->second.siVirtKey;
    m_bRenameCtrl = (iter->second.cPWSModifier & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL;
    m_bRenameShift = (iter->second.cPWSModifier & PWS_HOTKEYF_SHIFT) == PWS_HOTKEYF_SHIFT;
  } else {
    m_wpRenameKey = 0;
  }

  // Find Autotype Shortcut
  iter = m_MapMenuShortcuts.find(ID_MENUITEM_AUTOTYPE);
  
  // Save for CTreeCtrl & CListCtrl PreTranslateMessage
  if (iter != m_MapMenuShortcuts.end()) {
    m_wpAutotypeDNMsg = ((iter->second.cPWSModifier & PWS_HOTKEYF_ALT) == PWS_HOTKEYF_ALT) ?
                               WM_SYSKEYDOWN : WM_KEYDOWN;
    m_wpAutotypeUPMsg = ((iter->second.cPWSModifier & PWS_HOTKEYF_ALT) == PWS_HOTKEYF_ALT) ?
                               WM_SYSKEYUP : WM_KEYUP;
    m_wpAutotypeKey = iter->second.siVirtKey;
    m_bAutotypeCtrl = (iter->second.cPWSModifier & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL;
    m_bAutotypeShift = (iter->second.cPWSModifier & PWS_HOTKEYF_SHIFT) == PWS_HOTKEYF_SHIFT;
  } else {
    m_wpAutotypeKey = 0;
  }
}

void DboxMain::UpdateEditViewAccelerator(bool isRO)
{
  // If isRO, remove Ctrl-Enter from ID_MENUITEM_EDITENTRY, set to ID_MENUITEM_VIEWENTRY
  // else, vice-versa
  auto edit_iter = m_MapMenuShortcuts.find(ID_MENUITEM_EDITENTRY);
  ASSERT(edit_iter != m_MapMenuShortcuts.end());
  auto view_iter = m_MapMenuShortcuts.find(ID_MENUITEM_VIEWENTRY);
  ASSERT(view_iter != m_MapMenuShortcuts.end());
  
  if (isRO) {
    view_iter->second.SetKeyFlags(edit_iter->second);
    edit_iter->second.ClearKeyFlags();
  } else { // !isRO
    edit_iter->second.SetKeyFlags(view_iter->second);
    view_iter->second.ClearKeyFlags();
  } // !isRO
  UpdateAccelTable();
}

bool DboxMain::ProcessLanguageMenu(CMenu *pPopupMenu)
{
  app.GetLanguageFiles();

  // Should always be 1 entry (for the embedded resources)
  // Can't do anything if no other DLLs available
  // This can also happen if they are back-level resoure only DLLs who have
  // a language ID the same as the application.
  if (app.m_vlanguagefiles.size() < 2)
    return false;

  size_t i;
  const UINT numItems = pPopupMenu->GetMenuItemCount();

  // Remove any existing menu items in case user has done something!
  for (i = 0; i < numItems; i++) {
    pPopupMenu->RemoveMenu(0, MF_BYPOSITION);
  }

  // Add new ones
  UINT nID = ID_LANGUAGES;

  // Add languages
  for (i = 0; i < app.m_vlanguagefiles.size(); i++) {
    UINT uiFlags = MF_STRING | MF_ENABLED | 
       ((app.m_vlanguagefiles[i].xFlags & 0x80) == 0x80) ? MF_CHECKED : MF_UNCHECKED;
    pPopupMenu->AppendMenu(uiFlags, nID++, app.m_vlanguagefiles[i].wsLanguage.c_str());
  }

  return true;
}

const unsigned int DboxMain::GetMenuShortcut(const unsigned short int &siVirtKey,
                                             const unsigned char &cPWSModifier,
                                             StringX &sxMenuItemName)
{
  unsigned int nControlID(0);
  sxMenuItemName.empty();

  st_MenuShortcut st_mst;
  st_mst.siVirtKey = siVirtKey;
  st_mst.cPWSModifier = cPWSModifier;
  
  auto inuse_iter = std::find_if(m_MapMenuShortcuts.begin(),
                                 m_MapMenuShortcuts.end(),
                                 already_inuse(st_mst));

  if (inuse_iter != m_MapMenuShortcuts.end()) {
    nControlID = inuse_iter->first;
    sxMenuItemName = inuse_iter->second.name.c_str();
  }

  // is it in the reserved shortcuts?
  if (nControlID == 0) {
    auto iter = std::find_if(m_ReservedShortcuts.begin(), m_ReservedShortcuts.end(),
                             reserved(st_mst));

    if (iter != m_ReservedShortcuts.end()) {
      nControlID = iter->nControlID;
      LoadAString(sxMenuItemName, nControlID);
    }
  }

  if (!sxMenuItemName.empty()) {
    // These may have the shortcut hardcoded after a tab character e.g. "\tF1" for Help
    // Remove the tab and shortcut
    size_t found = sxMenuItemName.find_first_of(L"\t");
    if (found != StringX::npos)
      sxMenuItemName.erase(found);
  }

  return nControlID;
}
