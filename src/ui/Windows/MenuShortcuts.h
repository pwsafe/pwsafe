/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file MenuShortcuts.h
//-----------------------------------------------------------------------------

#pragma once

#include "core/StringX.h"

#include <map>
#include <vector>

// Structure used for vector of reserved Hotkeys
struct st_MenuShortcut {
  unsigned int nControlID;
  unsigned short int siVirtKey;
  unsigned char cPWSModifier;
};

class CMenuShortcut;

// Key is the Control ID
typedef std::map<unsigned int, CMenuShortcut> MapMenuShortcuts;
typedef std::pair<unsigned int, CMenuShortcut> MapMenuShortcutsPair;
typedef MapMenuShortcuts::iterator MapMenuShortcutsIter;

class CMenuShortcut
{
public:
  // Menu item text
  std::wstring name;
  // Menu item's parent or zero
  unsigned int uiParentID;
  // Sequential menu item position (i.e. in going through the menus)
  // Used to sort shortcuts in the CListCtrl in OptionsShortcuts
  int iMenuPosition;
  // Default values
  unsigned short int siDefVirtKey;
  unsigned char cDefPWSModifier;
  // new User values
  unsigned short int siVirtKey;
  unsigned char cPWSModifier;

  CMenuShortcut()
  : uiParentID(0), iMenuPosition(0),
    siDefVirtKey(0), cDefPWSModifier(0), siVirtKey(0), cPWSModifier(0)
  {}

  CMenuShortcut(const CMenuShortcut &that)
  : name(that.name), uiParentID(that.uiParentID),
    iMenuPosition(that.iMenuPosition),
    siDefVirtKey(that.siDefVirtKey), cDefPWSModifier(that.cDefPWSModifier),
    siVirtKey(that.siVirtKey), cPWSModifier(that.cPWSModifier)
  {}

  ~CMenuShortcut()
  {
  }

  void ClearKeyFlags()
  {
    // Only clear Key/flags - not name, parent ID or menu position
    siDefVirtKey = siVirtKey = 0;
    cDefPWSModifier = cPWSModifier = 0;
  }

  void SetKeyFlags(const CMenuShortcut &that)
  {
    // Only set Key/flags - not name, parent ID or menu position
    siDefVirtKey = that.siDefVirtKey;
    cDefPWSModifier = that.cDefPWSModifier;
    siVirtKey = that.siVirtKey;
    cPWSModifier = that.cPWSModifier;
  }

  CMenuShortcut &operator=(const CMenuShortcut &that)
  {
    if (this != &that) {
      name = that.name;
      uiParentID = that.uiParentID;
      iMenuPosition = that.iMenuPosition;
      siDefVirtKey = that.siDefVirtKey;
      cDefPWSModifier = that.cDefPWSModifier;
      siVirtKey = that.siVirtKey;
      cPWSModifier = that.cPWSModifier;
    }
    return *this;
  }

  static void InitStrings();

  static CString FormatShortcut(MapMenuShortcutsIter iter)
  {return FormatShortcut(iter->second.cPWSModifier, iter->second.siVirtKey);}
  static CString FormatShortcut(const st_MenuShortcut &mst)
  {return FormatShortcut(mst.cPWSModifier, mst.siVirtKey);}
  static bool IsNormalShortcut(const st_MenuShortcut &mst)
  {return IsNormalShortcut(mst.siVirtKey);}
  static CString FormatShortcut(WORD wPWSModifiers, WORD wVirtualKeyCode);
  
private:
  static bool IsNormalShortcut(WORD wVirtualKeyCode);  
  static CString CS_CTRLP, CS_ALTP, CS_SHIFTP;
};

// Functor for find_if to see if shortcut is already in use
struct already_inuse {
  already_inuse(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(MapMenuShortcutsPair const & p) const
  {
    return (p.second.siVirtKey  == m_st_mst.siVirtKey &&
            p.second.cPWSModifier == m_st_mst.cPWSModifier);
  }

  st_MenuShortcut m_st_mst;
};

// Functor for find_if to see if shortcut is reserved
struct reserved {
  reserved(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(st_MenuShortcut const& rdata) const
  {
    return (m_st_mst.siVirtKey == rdata.siVirtKey &&
            m_st_mst.cPWSModifier == rdata.cPWSModifier);
  }

  st_MenuShortcut m_st_mst;
};
