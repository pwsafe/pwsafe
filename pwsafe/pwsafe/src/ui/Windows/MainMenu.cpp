/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct st_ShortcutKeyNameID {
  const TCHAR *name;
  unsigned char id;
};

st_ShortcutKeyNameID ShortcutKeys[256];

// Functor for count_if
struct CountShortcuts {
  bool operator()(std::pair<const UINT, CMenuShortcut> &p) {
    return (p.second.cVirtKey != (unsigned char)0);
  }
};

// Functor for for_each
struct CreateAccelTable {
  CreateAccelTable(ACCEL *pacceltbl) : m_pacceltbl(pacceltbl) {}
  void operator()(std::pair<UINT, CMenuShortcut> p)
  {
    if (p.second.cVirtKey != 0) {
      m_pacceltbl->fVirt = FVIRTKEY |
                           ((p.second.cModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL ? FCONTROL : 0) |
                           ((p.second.cModifier & HOTKEYF_ALT)    == HOTKEYF_ALT      ? FALT     : 0) |
                           ((p.second.cModifier & HOTKEYF_SHIFT)  == HOTKEYF_SHIFT    ? FSHIFT   : 0);
      m_pacceltbl->key = (WORD)p.second.cVirtKey;
      m_pacceltbl->cmd = (WORD)p.first;
      m_pacceltbl++;
    }
  }

private:
  ACCEL *m_pacceltbl;
};

bool IsExtended(int code)
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

void DboxMain::SetUpInitialMenuStrings()
{
  CMenu *pMainMenu;
  CMenu *pMenu1, *pMenu2;
  int pos1, pos2, count;

  CHotKeyCtrl cHKC;
  CString sKeyName;
  LPCTSTR ptcKeyName;
  TCHAR *pname;
  st_KeyIDExt st_KIDEx;

  // Add user Excluded Menu Items - anything that is a Popup Menu
  m_ExcludedMenuItems.push_back(ID_FILEMENU);
  m_ExcludedMenuItems.push_back(ID_EXPORTMENU);
  m_ExcludedMenuItems.push_back(ID_IMPORTMENU);
  m_ExcludedMenuItems.push_back(ID_EDITMENU);
  m_ExcludedMenuItems.push_back(ID_VIEWMENU);
  m_ExcludedMenuItems.push_back(ID_FILTERMENU);
  m_ExcludedMenuItems.push_back(ID_CHANGEFONTMENU);
  m_ExcludedMenuItems.push_back(ID_REPORTSMENU);
  m_ExcludedMenuItems.push_back(ID_MANAGEMENU);
  m_ExcludedMenuItems.push_back(ID_HELPMENU);
  m_ExcludedMenuItems.push_back(ID_FINDMENU);

  // Plus Exit (2 shortcuts Ctrl+Q and Alt+F4) and Help (F1)
  m_ExcludedMenuItems.push_back(ID_MENUITEM_EXIT);
  m_ExcludedMenuItems.push_back(ID_HELP);

  // Plus Expand/Collapse group - function of TreeCtrl and is
  // the Enter key, which cannot be assigned via the Shortcut
  // Property Page
  m_ExcludedMenuItems.push_back(ID_MENUITEM_GROUPENTER);

  // The following are only in the Menu to get the correct string
  // for the action.  Add here to stop them being in the Shortcut
  // Options CListCtrl.
  // User can alter the function using the base values
  // ID_MENUITEM_EDIT, ID_MENUITEM_DELETE & ID_MENUITEM_RENAME
  m_ExcludedMenuItems.push_back(ID_MENUITEM_VIEW);

  m_ExcludedMenuItems.push_back(ID_MENUITEM_DELETEENTRY);
  m_ExcludedMenuItems.push_back(ID_MENUITEM_DELETEGROUP);

  m_ExcludedMenuItems.push_back(ID_MENUITEM_RENAMEENTRY);
  m_ExcludedMenuItems.push_back(ID_MENUITEM_RENAMEGROUP);

  // Add in the zero/None entry
  pname = _tcsdup(_T(""));
  st_KIDEx.id = 0;
  st_KIDEx.bExtended = false;
  m_MapKeyNameID.insert(MapKeyNameIDPair(st_KIDEx, pname));

  // Now add in locale key names (note range 0xE0-0xFF not used)
  for (int i = 1; i < 0xE0; i++) {
    // Following are reserved or unassigned by Windows
    if ( i == 0x07 || i == 0x0A || 
         i == 0x0B || i == 0x1B ||
        (i >= 0x3A && i <= 0x40) ||
        (i >= 0x5B && i <= 0x5F) ||
        (i >= 0x88 && i <= 0x8F) ||
        (i >= 0x92 && i <= 0x9F) ||
        (i >= 0xA6 && i <= 0xB9) ||
        (i >= 0xC1 && i <= 0xDA))
      continue;

    st_KIDEx.id = (unsigned char)i;
    st_KIDEx.bExtended = IsExtended(i);
    sKeyName = cHKC.GetKeyName((UINT)i, IsExtended(i) ? TRUE : FALSE);
    if (!sKeyName.IsEmpty()) {
      // Make value into "Sentence Case" e.g. "Enter" or "Num Plus" etc.
      CString cstoken, sKeyName2(_T(""));
      int curPos = 0;
      sKeyName.Trim();
      cstoken = sKeyName.Tokenize(_T(" "), curPos);
      while (!cstoken.IsEmpty()) {
        CString cstemp1 = cstoken.Left(1);
        CString cstemp2 = cstoken.Right(cstoken.GetLength() - 1);
        cstemp1.MakeUpper();
        cstemp2.MakeLower();
        sKeyName2 += cstemp1 + cstemp2 + CString(_T(" "));
        cstoken = sKeyName.Tokenize(_T(" "), curPos);
      };
      sKeyName2.Trim();
      ptcKeyName = sKeyName2.GetBuffer(sKeyName2.GetLength());
      pname = _tcsdup(ptcKeyName);
      sKeyName2.ReleaseBuffer();
      m_MapKeyNameID.insert(MapKeyNameIDPair(st_KIDEx, pname));
    }
  }

  pMainMenu = new CMenu;
  pMainMenu->LoadMenu(IDR_MAINMENU);

  TCHAR tcMenuString[_MAX_PATH + 1];

  MENUINFO minfo;
  memset(&minfo, 0x00, sizeof(minfo));
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;

  MENUITEMINFO miinfo;
  memset(&miinfo, 0x00, sizeof(MENUITEMINFO));
  miinfo.cbSize = sizeof(MENUITEMINFO);
  miinfo.fMask = MIIM_ID | MIIM_STRING;
  miinfo.dwTypeData = tcMenuString;

  count = pMainMenu->GetMenuItemCount();
  // Add reserved shortcuts = Alt+'x' for main menu e.g. Alt+F, E, V, M, H
  // Maybe different in other languages
  st_MenuShortcut st_mst;
  st_mst.cVirtKey;
  st_mst.cModifier = HOTKEYF_ALT;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMainMenu->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      CString csMainMenuItem = tcMenuString;
      int iamp = csMainMenuItem.Find(TCHAR('&'));
      if (iamp >= 0 && iamp < csMainMenuItem.GetLength() - 1) {
        st_mst.cVirtKey = (unsigned char)csMainMenuItem[iamp + 1];
        m_ReservedShortcuts.push_back(st_mst);
      }
    }
  }

  // Add 3 special keys F1 (for Help), Ctrl+Q/Alt+F4 (Exit)
  st_mst.cVirtKey = VK_F1;
  st_mst.cModifier = 0;
  m_ReservedShortcuts.push_back(st_mst);

  st_mst.cVirtKey = 'Q';
  st_mst.cModifier = HOTKEYF_CONTROL;
  m_ReservedShortcuts.push_back(st_mst);

  st_mst.cVirtKey = VK_F4;
  st_mst.cModifier = HOTKEYF_ALT;
  m_ReservedShortcuts.push_back(st_mst);

  // Now get all other Menu items
  CMenuShortcut mst;
  mst.name = tcMenuString;
  mst.iMenuPosition = 0;

  // Do Main Menu (uiParentID == 0)
  count = pMainMenu->GetMenuItemCount();
  mst.uiParentID = 0;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMainMenu->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }
  // Do File Menu
  pos1 = app.FindMenuItem(pMainMenu, ID_FILEMENU);
  ASSERT(pos1 != -1);

  pMenu1 = pMainMenu->GetSubMenu(pos1);
  count = pMenu1->GetMenuItemCount();
  mst.uiParentID = ID_FILEMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu1->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do File Menu Export submenu
  pos2 = app.FindMenuItem(pMenu1, ID_EXPORTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  count = pMenu2->GetMenuItemCount();
  mst.uiParentID = ID_EXPORTMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu2->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do File Menu Import submenu
  pos2 = app.FindMenuItem(pMenu1, ID_IMPORTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  count = pMenu2->GetMenuItemCount();
  mst.uiParentID = ID_IMPORTMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu2->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do Edit Menu
  pos1 = app.FindMenuItem(pMainMenu, ID_EDITMENU);
  ASSERT(pos1 != -1);

  pMenu1 = pMainMenu->GetSubMenu(pos1);
  count = pMenu1->GetMenuItemCount();
  mst.uiParentID = ID_EDITMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu1->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do View Menu
  pos1 = app.FindMenuItem(pMainMenu, ID_VIEWMENU);
  ASSERT(pos1 != -1);

  pMenu1 = pMainMenu->GetSubMenu(pos1);
  count = pMenu1->GetMenuItemCount();
  mst.uiParentID = ID_VIEWMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu1->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do View Menu Filter submenu
  pos2 = app.FindMenuItem(pMenu1, ID_FILTERMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  count = pMenu2->GetMenuItemCount();
  mst.uiParentID = ID_FILTERMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu2->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do View Menu ChangeFont submenu
  pos2 = app.FindMenuItem(pMenu1, ID_CHANGEFONTMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  count = pMenu2->GetMenuItemCount();
  mst.uiParentID = ID_CHANGEFONTMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu2->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do View Menu Reports submenu
  pos2 = app.FindMenuItem(pMenu1, ID_REPORTSMENU);
  ASSERT(pos2 != -1);

  pMenu2 = pMenu1->GetSubMenu(pos2);
  count = pMenu2->GetMenuItemCount();
  mst.uiParentID = ID_REPORTSMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu2->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do Manage Menu
  pos1 = app.FindMenuItem(pMainMenu, ID_MANAGEMENU);
  ASSERT(pos1 != -1);

  pMenu1 = pMainMenu->GetSubMenu(pos1);
  count = pMenu1->GetMenuItemCount();
  mst.uiParentID = ID_MANAGEMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu1->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Do Help Menu
  pos1 = app.FindMenuItem(pMainMenu, ID_HELPMENU);
  ASSERT(pos1 != -1);

  pMenu1 = pMainMenu->GetSubMenu(pos1);
  count = pMenu1->GetMenuItemCount();
  mst.uiParentID = ID_HELPMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu1->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // Don't need main menu again here
  pMainMenu->DestroyMenu();

  // Do Find toolbar menu items not on a menu!
  pMainMenu->LoadMenu(IDR_POPFIND);

  mst.uiParentID = 0;
  ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
  miinfo.cch = _MAX_PATH;
  pMainMenu->GetMenuItemInfo(0, &miinfo, TRUE);
  if (miinfo.wID >= 1) {
    mst.name = _tcsdup(tcMenuString);
    m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
    mst.iMenuPosition++;
    free((void *)mst.name);
    mst.name = NULL;
  }

  pos1 = app.FindMenuItem(pMainMenu, ID_FINDMENU);
  ASSERT(pos1 != -1);

  pMenu1 = pMainMenu->GetSubMenu(pos1);
  count = pMenu1->GetMenuItemCount();
  mst.uiParentID = ID_FINDMENU;

  for (int i = 0; i < count; i++) {
    ZeroMemory(tcMenuString, (_MAX_PATH + 1) * sizeof(TCHAR));
    miinfo.cch = _MAX_PATH;
    pMenu1->GetMenuItemInfo(i, &miinfo, TRUE);
    if (miinfo.wID >= 1) {
      mst.name = _tcsdup(tcMenuString);
      m_MapMenuShortcuts.insert(MapMenuShortcutsPair(miinfo.wID, mst));
      mst.iMenuPosition++;
      free((void *)mst.name);
      mst.name = NULL;
    }
  }

  // No longer need any menus
  pMainMenu->DestroyMenu();
  delete pMainMenu;

  // Now that we have all menu strings - go get current accelerator strings
  // and update Map
  MapMenuShortcutsIter iter, iter_entry, iter_group;
  HACCEL curacctbl = app.m_ghAccelTable;
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
        iter->second.cdefVirtKey = iter->second.cVirtKey = 
          (unsigned char)paccel->key;
          iter->second.cdefModifier = iter->second.cModifier =
          ((paccel->fVirt & FCONTROL) == FCONTROL ? HOTKEYF_CONTROL : 0) |
          ((paccel->fVirt & FALT)     == FALT     ? HOTKEYF_ALT     : 0) |
          ((paccel->fVirt & FSHIFT)   == FSHIFT   ? HOTKEYF_SHIFT   : 0) |
          (IsExtended((int)paccel->key)           ? HOTKEYF_EXT     : 0);
      }
      paccel++;
    }
    // Don't need copy any more
    LocalFree(pacceltbl);
    pacceltbl = paccel = NULL;
  }

  // Now go through the default table and see if the user has changed anything
  // Now change the user's shortcuts as per their preferences
  std::vector<st_prefShortcut> vShortcuts;
  vShortcuts = PWSprefs::GetInstance()->GetPrefShortcuts();

  bool bUserShortcutsChanged(false);
  for (size_t i = 0; i < vShortcuts.size(); i++) {
    st_prefShortcut stxst = vShortcuts[i];
    iter = m_MapMenuShortcuts.find(stxst.id);
    if (iter != m_MapMenuShortcuts.end() ||
        (iter->second.cVirtKey  != stxst.cVirtKey  ||
         iter->second.cModifier != stxst.cModifier)) {
      iter->second.cVirtKey  = stxst.cVirtKey;
      iter->second.cModifier = stxst.cModifier;
    } else {
      // Either unknown Control ID or no different from default
      bUserShortcutsChanged = true;
      vShortcuts.erase(vShortcuts.begin() + i);
    }
    // User should not have these sub-entries in their config file
    if (stxst.id == ID_MENUITEM_GROUPENTER ||
        stxst.id == ID_MENUITEM_VIEW ||
        stxst.id == ID_MENUITEM_DELETEENTRY ||
        stxst.id == ID_MENUITEM_DELETEGROUP ||
        stxst.id == ID_MENUITEM_RENAMEENTRY ||
        stxst.id == ID_MENUITEM_RENAMEGROUP) {
      bUserShortcutsChanged = true;
      vShortcuts.erase(vShortcuts.begin() + i);
    }
  }

  if (bUserShortcutsChanged)
    PWSprefs::GetInstance()->SetPrefShortcuts(vShortcuts);

  // Set up the shortcuts based on the main entry
  // for View, Delete and Rename
  iter = m_MapMenuShortcuts.find(ID_MENUITEM_EDIT);
  iter_entry = m_MapMenuShortcuts.find(ID_MENUITEM_VIEW);
  iter_entry->second.SetKeyFlags(iter->second);

  iter = m_MapMenuShortcuts.find(ID_MENUITEM_DELETE);
  iter_entry = m_MapMenuShortcuts.find(ID_MENUITEM_DELETEENTRY);
  iter_entry->second.SetKeyFlags(iter->second);
  iter_group = m_MapMenuShortcuts.find(ID_MENUITEM_DELETEGROUP);
  iter_group->second.SetKeyFlags(iter->second);

  // Now tell the TreeCtrl the key for Delete
  m_ctlItemTree.SetDeleteKey(iter->second.cVirtKey, iter->second.cModifier);
  m_ctlItemList.SetDeleteKey(iter->second.cVirtKey, iter->second.cModifier);

  iter = m_MapMenuShortcuts.find(ID_MENUITEM_RENAME);
  iter_entry = m_MapMenuShortcuts.find(ID_MENUITEM_RENAMEENTRY);
  iter_entry->second.SetKeyFlags(iter->second);
  iter_group = m_MapMenuShortcuts.find(ID_MENUITEM_RENAMEGROUP);
  iter_group->second.SetKeyFlags(iter->second);

  // Now tell the TreeCtrl the key for Rename
  m_ctlItemTree.SetRenameKey(iter->second.cVirtKey, iter->second.cModifier);
  
  UpdateAccelTable();
}

void DboxMain::UpdateAccelTable()
{
  ACCEL *pacceltbl, *pxatbl;
  int numscs(0);
  CountShortcuts cntscs;

  // Add on space of 3 reserved shortcuts
  numscs = std::count_if(m_MapMenuShortcuts.begin(), m_MapMenuShortcuts.end(),
                         cntscs) + 3;
  if (numscs == 0) {
    // None!
    if (app.m_ghAccelTable != NULL) {
      DestroyAcceleratorTable(app.m_ghAccelTable);
      app.m_ghAccelTable = NULL;
    }
    return;
  }

  // Get new table space
  pacceltbl = (LPACCEL)LocalAlloc(LPTR, numscs * sizeof(ACCEL));
  ASSERT(pacceltbl != NULL);

  // Populate it
  CreateAccelTable create_accel_table(pacceltbl);
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
  MENUITEMINFO miinfo;
  memset(&miinfo, 0x00, sizeof(MENUITEMINFO));
  miinfo.cbSize = sizeof(MENUITEMINFO);
  miinfo.fMask = MIIM_ID | MIIM_STATE;

  MapMenuShortcutsIter iter;
  MapKeyNameIDConstIter citer;
  st_KeyIDExt st_KIDEx;

  int count = pPopupMenu->GetMenuItemCount();
  for (int i = 0; i < count; i++) {
    pPopupMenu->GetMenuItemInfo(i, &miinfo, TRUE);
    // Exit & Help never changed and their shortcuts are in the menu text
    if (miinfo.wID >= 1 &&
        miinfo.wID != ID_MENUITEM_EXIT && miinfo.wID != ID_HELP) {
      iter = m_MapMenuShortcuts.find(miinfo.wID);
      if (iter != m_MapMenuShortcuts.end()) {
        CString str;
        if (iter->second.cVirtKey != 0) {
          st_KIDEx.id = iter->second.cVirtKey;
          st_KIDEx.bExtended = (iter->second.cModifier & HOTKEYF_EXT) == HOTKEYF_EXT;
          citer = m_MapKeyNameID.find(st_KIDEx);
          str.Format(_T("%s\t%s"), iter->second.name, 
                     CMenuShortcut::FormatShortcut(iter, citer));
        } else {
          str = iter->second.name;
        }
        pPopupMenu->ModifyMenu(miinfo.wID, MF_BYCOMMAND | miinfo.fState, miinfo.wID, str);
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
  // It now tailors the Edit menu depending on whether a Group or Entry is selected.
  // "EnableMenuItem" is handled by the UPDATE_UI routines
  bool bGroupSelected(false);
  const bool bTreeView = m_ctlItemTree.IsWindowVisible() == TRUE;
  const bool bItemSelected = (SelItemOk() == TRUE);
  const bool bReadOnly = m_core.IsReadOnly();
  CItemData *pci(NULL);
  CItemData::EntryType etype(CItemData::ET_INVALID);
  const TCHAR *tc_dummy = _T(" ");

  // Except for Edit & View menus, we only get here if the Shortcuts have changed and
  // we need to rebuild menu item text
  if (bDoShortcuts)
    SetUpMenuStrings(pPopupMenu);

  // We have done for all except the Edit and View menus
  if (uiMenuID != ID_EDITMENU && uiMenuID != ID_VIEWMENU)
    return;

  // If View menu selected (contains 'Flattened &List' menu item)
  if (uiMenuID == ID_VIEWMENU) {
    // Delete Show Find Toolbar menu item - don't know if there or previously deleted
    pPopupMenu->RemoveMenu(ID_MENUITEM_SHOWFINDTOOLBAR, MF_BYCOMMAND);
    if (!m_FindToolBar.IsVisible()) {
      // Put it back if not visible - before the separator
      int pos = app.FindMenuItem(pPopupMenu, ID_MENUITEM_SHOWHIDE_DRAGBAR);
      pPopupMenu->InsertMenu(pos + 1,
                             MF_BYPOSITION  | MF_ENABLED | MF_STRING,
                             ID_MENUITEM_SHOWFINDTOOLBAR, tc_dummy);
    }
    pPopupMenu->CheckMenuRadioItem(ID_MENUITEM_LIST_VIEW, ID_MENUITEM_TREE_VIEW,
                                   bTreeView ? ID_MENUITEM_TREE_VIEW : ID_MENUITEM_LIST_VIEW,
                                   MF_BYCOMMAND);

    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_TOOLBAR, MF_BYCOMMAND |
                              m_MainToolBar.IsWindowVisible() ? MF_CHECKED : MF_UNCHECKED);

    bool bDragBarState = PWSprefs::GetInstance()->GetPref(PWSprefs::ShowDragbar);
    pPopupMenu->CheckMenuItem(ID_MENUITEM_SHOWHIDE_DRAGBAR, MF_BYCOMMAND |
                              bDragBarState ? MF_CHECKED : MF_UNCHECKED);

    pPopupMenu->ModifyMenu(ID_MENUITEM_APPLYFILTER, MF_BYCOMMAND |
                           m_bFilterActive ? MF_CHECKED : MF_UNCHECKED,
                           ID_MENUITEM_APPLYFILTER,
                           m_bFilterActive ? CS_CLEARFILTERS : CS_SETFILTERS);

    pPopupMenu->CheckMenuRadioItem(ID_MENUITEM_NEW_TOOLBAR,
                                   ID_MENUITEM_OLD_TOOLBAR,
                                   m_toolbarMode, MF_BYCOMMAND);
    goto exit;
  }  // View menu

  if (bItemSelected) {
    pci = getSelectedItem();
    ASSERT(pci != NULL);

    // Save entry type before changing pci
    etype = pci->GetEntryType();
    if (etype == CItemData::ET_SHORTCUT) {
      // This is an shortcut
      uuid_array_t entry_uuid, base_uuid;
      pci->GetUUID(entry_uuid);
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

      ItemListIter iter = m_core.Find(base_uuid);
      if (iter != End()) {
        pci = &iter->second;
      }
    }
  }

  if (bTreeView) {
    HTREEITEM hi = m_ctlItemTree.GetSelectedItem();
    bGroupSelected = (hi != NULL && !m_ctlItemTree.IsLeaf(hi));
  }

  // If Edit menu selected
  if (uiMenuID == ID_EDITMENU) {
    // Delete all entries and rebuild depending on group/entry selected
    // and, if entry, if fields are non-empty
    int count = pPopupMenu->GetMenuItemCount();
    for (int i = 0; i < count; i++) {
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
          pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_ADDGROUP, tc_dummy);
          pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
        }
      }
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
      goto exit;
    }

    // Scenario 2 - Group selected (Tree view only)
    //   Add Entry, Sep, <Group Items>, Sep, Clear Clipboard
    if (!m_IsListView && bGroupSelected) {
      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_ADD, tc_dummy);
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
      }
      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
      goto exit;
    }

    // Scenario 3 - Entry selected
    //   Tree View - <Entry Items>, Sep, Add Group, Sep, Clear Clipboard, Sep
    //               Entry functions (copy to clipboard, browse, run etc)
    //   List View - <Entry Items>, Sep, Clear Clipboard, Sep
    //               Entry functions (copy to clipboard, browse, run etc)
    if (pci != NULL) {
      // Entry is selected
      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_ADD, tc_dummy);
      }
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             m_core.IsReadOnly() ? ID_MENUITEM_VIEW : ID_MENUITEM_EDIT,
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
      // Only have Find Next/Previous if find still active and entries were found
      if (m_FindToolBar.IsVisible() && m_FindToolBar.EntriesFound()) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FIND, tc_dummy);
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_FINDUP, tc_dummy);
      }
      if (!bReadOnly) {
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_DUPLICATEENTRY, tc_dummy);
        if (!m_IsListView) {
          pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_ADDGROUP, tc_dummy);
        }
      }
      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_COPYPASSWORD, tc_dummy);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_PASSWORDSUBSET, tc_dummy);

      if (!pci->IsUserEmpty())
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYUSERNAME, tc_dummy);

      if (!pci->IsNotesEmpty())
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_COPYNOTESFLD, tc_dummy);

      if (!pci->IsURLEmpty()) {
        const bool bIsEmail = pci->IsURLEmail();
        if (bIsEmail) {
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_COPYEMAIL, tc_dummy);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_SENDEMAIL, tc_dummy);
        } else {
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_COPYURL, tc_dummy);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_BROWSEURL, tc_dummy);
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_BROWSEURLPLUS, tc_dummy);
        }
        UpdateBrowseURLSendEmailButton(bIsEmail);
      }

      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_AUTOTYPE, tc_dummy);

      if (!pci->IsRunCommandEmpty())
        pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                               ID_MENUITEM_RUNCOMMAND, tc_dummy);

      switch (etype) {
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
          // Allow going to the appropriate base entry
          pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                                 ID_MENUITEM_GOTOBASEENTRY, tc_dummy);
          break;
        default:
          ASSERT(0);
      }
    } else {
      // Must be List view with no entry selected
      pPopupMenu->InsertMenu((UINT)-1, MF_SEPARATOR);
      pPopupMenu->AppendMenu(MF_ENABLED | MF_STRING,
                             ID_MENUITEM_CLEARCLIPBOARD, tc_dummy);
    }
  }  // Edit menu

exit:
    SetUpMenuStrings(pPopupMenu);
    return;
}

// helps with MRU by allowing ON_UPDATE_COMMAND_UI
void DboxMain::OnInitMenuPopup(CMenu* pPopupMenu, UINT, BOOL)
{
  // Original OnInitMenu code
  // All main menus are POPUPs (see PasswordSafe2.rc2)

  // Don't do the old OnInitMenu processing if the right-click context menu
  // (IDR_POPEDITMENU or IDR_POPEDITGROUP) is being processed. Only for the Main Menu
  // (ID_EDITMENU).
  ASSERT(pPopupMenu != NULL);

  MENUINFO minfo;
  memset(&minfo, 0x00, sizeof(minfo));
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;
  pPopupMenu->GetMenuInfo(&minfo);

  // Need to update the main menu if the shortcuts have been changed or
  // if the Edit or View menu.  The Edit menu is completely rebuilt each time
  // based on the view and if anything is selected. The View menu has one item
  // (ShowFindToolbar) that is removed if already displayed

  bool bDoShortcuts(false);
  switch (minfo.dwMenuData) {
    case ID_FILEMENU:
      bDoShortcuts = m_bDoShortcuts[FILEMENU];
      m_bDoShortcuts[FILEMENU] = false;
      break;
    case ID_EXPORTMENU:
      bDoShortcuts = m_bDoShortcuts[EXPORTMENU];
      m_bDoShortcuts[EXPORTMENU] = false;
      break;
    case ID_IMPORTMENU:
      bDoShortcuts = m_bDoShortcuts[IMPORTMENU];
      m_bDoShortcuts[IMPORTMENU] = false;
      break;
    case ID_EDITMENU:
      bDoShortcuts = m_bDoShortcuts[EDITMENU];
      m_bDoShortcuts[EDITMENU] = false;
      break;
    case ID_VIEWMENU:
      bDoShortcuts = m_bDoShortcuts[VIEWMENU];
      m_bDoShortcuts[VIEWMENU] = false;
      break;
    case ID_FILTERMENU:
      bDoShortcuts = m_bDoShortcuts[FILTERMENU];
      m_bDoShortcuts[FILTERMENU] = false;
      break;
    case ID_CHANGEFONTMENU:
      bDoShortcuts = m_bDoShortcuts[CHANGEFONTMENU];
      m_bDoShortcuts[CHANGEFONTMENU] = false;
      break;
    case ID_REPORTSMENU:
      bDoShortcuts = m_bDoShortcuts[REPORTSMENU];
      m_bDoShortcuts[REPORTSMENU] = false;
      break;
    case ID_MANAGEMENU:
      bDoShortcuts = m_bDoShortcuts[MANAGEMENU];
      m_bDoShortcuts[MANAGEMENU] = false;
      break;
    case ID_HELPMENU:
      bDoShortcuts = m_bDoShortcuts[HELPMENU];
      m_bDoShortcuts[HELPMENU] = false;
      break;
    default:
      break;
  }

  if (bDoShortcuts || minfo.dwMenuData == ID_EDITMENU || minfo.dwMenuData == ID_VIEWMENU)
    CustomiseMenu(pPopupMenu, minfo.dwMenuData, bDoShortcuts);

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
  if (!m_bImageInLV || minfo.dwMenuData != IDR_POPTRAY)
    return;

  // System Tray Popup menu processing only
  minfo.fMask = MIM_STYLE;
  minfo.dwStyle = MNS_CHECKORBMP | MNS_AUTODISMISS;
  pPopupMenu->SetMenuInfo(&minfo);

  MENUITEMINFO miinfo;
  memset(&miinfo, 0x00, sizeof(miinfo));
  miinfo.cbSize = sizeof(miinfo);
  CRUEItemData *pmd;

  for (UINT pos = 0; pos < pPopupMenu->GetMenuItemCount(); pos++) {
    miinfo.fMask = MIIM_FTYPE | MIIM_DATA;
    pPopupMenu->GetMenuItemInfo(pos, &miinfo, TRUE);
    pmd = (CRUEItemData *)miinfo.dwItemData;

    if (pmd && pmd->IsRUEID() && !(miinfo.fType & MFT_OWNERDRAW) &&
        pmd->nImage >= 0) {
      miinfo.fMask = MIIM_FTYPE | MIIM_BITMAP;
      miinfo.hbmpItem = HBMMENU_CALLBACK;
      miinfo.fType = MFT_STRING;

      pPopupMenu->SetMenuItemInfo(pos, &miinfo, TRUE);
    }
  }
}

// Called when right-click is invoked in the client area of the window.
void DboxMain::OnContextMenu(CWnd* /* pWnd */, CPoint screen)
{
#if defined(POCKET_PC)
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN;
#else
  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;
#endif

  CPoint client;
  int item = -1;
  CItemData *pci = NULL;
  CMenu menu;

  MENUINFO minfo;
  memset(&minfo, 0x00, sizeof(minfo));
  minfo.cbSize = sizeof(MENUINFO);
  minfo.fMask = MIM_MENUDATA;

  // Note if point = (-1, -1) then invoked via keyboard.
  // Need coordinates of current selected itme instead on mouse position when message sent
  bool bKeyboard = (screen.x == -1 && screen.y == -1);

  CPoint mp; // Screen co-ords (from "message point" or via Shift+F10 selected item
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
    screen = mp;  // In screen co-ords
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
      menu.SetMenuInfo(&minfo);
      CMenu* pPopup = menu.GetSubMenu(0);
      ASSERT(pPopup != NULL);
      pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this); // use this window for commands
    }
    return;
  }

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
      if (pci != NULL) {
        // right-click was on an item (LEAF of some kind: normal, alias, shortcut)
        DisplayInfo *di = (DisplayInfo *)pci->GetDisplayInfo();
        ASSERT(di != NULL);
        ASSERT(di->tree_item == ti);
        item = di->list_index;
        m_ctlItemTree.SelectItem(ti); // So that OnEdit gets the right one
      } else {
        // right-click was on a group (NODE)
        m_ctlItemTree.SelectItem(ti);
        if (menu.LoadMenu(IDR_POPEDITGROUP)) {
          minfo.dwMenuData = IDR_POPEDITGROUP;
          menu.SetMenuInfo(&minfo);

          CMenu* pPopup = menu.GetSubMenu(0);
          ASSERT(pPopup != NULL);
          m_TreeViewGroup = m_ctlItemTree.GetGroup(ti);
          // use this DboxMain for commands
          pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
        }
      }
    } else {
      // not over anything
      if (menu.LoadMenu(IDR_POPTREE)) {  // "Add Group"
        minfo.dwMenuData = IDR_POPTREE;
        menu.SetMenuInfo(&minfo);
        CMenu* pPopup = menu.GetSubMenu(0);
        ASSERT(pPopup != NULL);
        // use this DboxMain for commands
        pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
      }
    }
    m_ctlItemTree.SetFocus();
  } // tree view handling

  // RClick over an entry
  if (item >= 0) {
    menu.LoadMenu(IDR_POPEDITMENU);
    minfo.dwMenuData = IDR_POPEDITMENU;
    menu.SetMenuInfo(&minfo);

    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);
    ASSERT(pci != NULL);

    CItemData::EntryType etype = pci->GetEntryType();
    switch (etype) {
      case CItemData::ET_NORMAL:
      case CItemData::ET_SHORTCUTBASE:
        pPopup->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
        break;
      case CItemData::ET_ALIASBASE:
        pPopup->RemoveMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_GOTOBASEENTRY, MF_BYCOMMAND);
        break;
      case CItemData::ET_ALIAS:
      case CItemData::ET_SHORTCUT:
        pPopup->RemoveMenu(ID_MENUITEM_CREATESHORTCUT, MF_BYCOMMAND);
        break;
      default:
        ASSERT(0);
    }

    uuid_array_t entry_uuid, base_uuid;
    if (pci->IsShortcut()) {
      // This is an shortcut
      pci->GetUUID(entry_uuid);
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

      ItemListIter iter = m_core.Find(base_uuid);
      if (iter != End()) {
        pci = &iter->second;
      }
    }

    if (pci->IsURLEmpty()) {
      pPopup->RemoveMenu(ID_MENUITEM_COPYURL, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_COPYEMAIL, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_SENDEMAIL, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_BROWSEURL, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_MENUITEM_BROWSEURLPLUS, MF_BYCOMMAND);
      UpdateBrowseURLSendEmailButton(false);
    } else {
      const bool bIsEmail = pci->IsURLEmail();
      if (bIsEmail) {
        pPopup->RemoveMenu(ID_MENUITEM_COPYURL, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_BROWSEURL, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_BROWSEURLPLUS, MF_BYCOMMAND);
      } else {
        pPopup->RemoveMenu(ID_MENUITEM_COPYEMAIL, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_MENUITEM_SENDEMAIL, MF_BYCOMMAND);
      }
      UpdateBrowseURLSendEmailButton(bIsEmail);
    }
    if (pci->IsRunCommandEmpty()) {
      pPopup->RemoveMenu(ID_MENUITEM_RUNCOMMAND, MF_BYCOMMAND);
    }

    // use this DboxMain for commands
    pPopup->TrackPopupMenu(dwTrackPopupFlags, screen.x, screen.y, this);
  } // if (item >= 0)
}
