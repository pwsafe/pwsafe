/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
  unsigned short int siVirtKey;
  unsigned char cModifier;
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
  unsigned char cDefModifier;
  // new User values
  unsigned short int siVirtKey;
  unsigned char cModifier;

  CMenuShortcut()
  : uiParentID(0), iMenuPosition(0),
    siDefVirtKey(0), cDefModifier(0), siVirtKey(0), cModifier(0)
  {}

  CMenuShortcut(const CMenuShortcut &that)
  : name(that.name), uiParentID(that.uiParentID),
    iMenuPosition(that.iMenuPosition),
    siDefVirtKey(that.siDefVirtKey), cDefModifier(that.cDefModifier),
    siVirtKey(that.siVirtKey), cModifier(that.cModifier)
  {}

  ~CMenuShortcut()
  {
  }

  void ClearKeyFlags()
  {
    // Only clear Key/flags - not name, parent ID or menu position
    siDefVirtKey = siVirtKey = 0;
    cDefModifier = cModifier = 0;
  }

  void SetKeyFlags(const CMenuShortcut &that)
  {
    // Only set Key/flags - not name, parent ID or menu position
    siDefVirtKey = that.siDefVirtKey;
    cDefModifier = that.cDefModifier;
    siVirtKey = that.siVirtKey;
    cModifier = that.cModifier;
  }

  CMenuShortcut &operator=(const CMenuShortcut &that)
  {
    if (this != &that) {
      name = that.name;
      uiParentID = that.uiParentID;
      iMenuPosition = that.iMenuPosition;
      siDefVirtKey = that.siDefVirtKey;
      cDefModifier = that.cDefModifier;
      siVirtKey = that.siVirtKey;
      cModifier = that.cModifier;
    }
    return *this;
  }

  static void InitStrings();

  static CString FormatShortcut(MapMenuShortcutsIter iter)
  {return FormatShortcut(iter->second.cModifier, iter->second.siVirtKey);}

  static CString FormatShortcut(const st_MenuShortcut &mst)
  {return FormatShortcut(mst.cModifier, mst.siVirtKey);}
  static bool IsNormalShortcut(const st_MenuShortcut &mst)
  {return IsNormalShortcut(mst.cModifier, mst.siVirtKey);}

private:
  static bool IsNormalShortcut(unsigned short int cModifier,
                               unsigned short int siVirtKey);  
  static CString FormatShortcut(unsigned short int cModifier,
                                unsigned short int siVirtKey);
  static CString CS_CTRLP, CS_ALTP, CS_SHIFTP;
};

// Functor for find_if to see if shortcut is already in use
struct already_inuse {
  already_inuse(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(MapMenuShortcutsPair const & p) const
  {
    return (p.second.siVirtKey  == m_st_mst.siVirtKey &&
            p.second.cModifier == m_st_mst.cModifier);
  }

  st_MenuShortcut m_st_mst;
};
