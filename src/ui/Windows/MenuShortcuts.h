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
  unsigned char cVirtKey;
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
  unsigned char cdefVirtKey;
  unsigned char cdefModifier;
  // new User values
  unsigned char cVirtKey;
  unsigned char cModifier;

  CMenuShortcut()
  : uiParentID(0), iMenuPosition(0),
    cdefVirtKey(0), cdefModifier(0), cVirtKey(0), cModifier(0)
  {}

  CMenuShortcut(const CMenuShortcut &that)
  : name(that.name), uiParentID(that.uiParentID),
    iMenuPosition(that.iMenuPosition),
    cdefVirtKey(that.cdefVirtKey), cdefModifier(that.cdefModifier),
    cVirtKey(that.cVirtKey), cModifier(that.cModifier)
  {}

  ~CMenuShortcut()
  {
  }

  void ClearKeyFlags()
  {
    // Only clear Key/flags - not name, parent ID or menu position
    cdefVirtKey = cdefModifier = cVirtKey = cModifier = (unsigned char)0;
  }

  void SetKeyFlags(const CMenuShortcut &that)
  {
    // Only set Key/flags - not name, parent ID or menu position
    cdefVirtKey = that.cdefVirtKey;
    cdefModifier = that.cdefModifier;
    cVirtKey = that.cVirtKey;
    cModifier = that.cModifier;
  }

  CMenuShortcut &operator=(const CMenuShortcut &that)
  {
    if (this != &that) {
      name = that.name;
      uiParentID = that.uiParentID;
      iMenuPosition = that.iMenuPosition;
      cdefVirtKey = that.cdefVirtKey;
      cdefModifier = that.cdefModifier;
      cVirtKey = that.cVirtKey;
      cModifier = that.cModifier;
    }
    return *this;
  }

  static void InitStrings();

  static CString FormatShortcut(MapMenuShortcutsIter iter)
  {return FormatShortcut(iter->second.cModifier, iter->second.cVirtKey);}

  static CString FormatShortcut(const st_MenuShortcut &mst)
  {return FormatShortcut(mst.cModifier, mst.cVirtKey);}
  static bool IsNormalShortcut(const st_MenuShortcut &mst)
  {return IsNormalShortcut(mst.cModifier, mst.cVirtKey);}

private:
  static bool IsNormalShortcut(unsigned char cModifier,
                                unsigned char cVirtKey);  
  static CString FormatShortcut(unsigned char cModifier,
                                unsigned char cVirtKey);
  static CString CS_CTRLP, CS_ALTP, CS_SHIFTP;
};

// Functor for find_if to see if shortcut is already in use
struct already_inuse {
  already_inuse(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(MapMenuShortcutsPair const & p) const
  {
    return (p.second.cVirtKey  == m_st_mst.cVirtKey &&
            p.second.cModifier == m_st_mst.cModifier);
  }

  st_MenuShortcut m_st_mst;
};
