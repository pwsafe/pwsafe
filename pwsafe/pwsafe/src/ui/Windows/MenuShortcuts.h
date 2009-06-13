/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file MenuShortcuts.h
//-----------------------------------------------------------------------------

#pragma once

#include "corelib/StringX.h"

#include <map>
#include <vector>

// Structure used for vector of reserved Hotkeys
struct st_MenuShortcut {
  unsigned char cVirtKey;
  unsigned char cModifier;
};

struct st_KeyIDExt {
  unsigned char id;
  bool bExtended;

  bool operator < (const st_KeyIDExt & rhs) const {
    return id < rhs.id;
  }

  bool operator == (const st_KeyIDExt & rhs) const {
    return (id == rhs.id && bExtended == rhs.bExtended) ;
  }
};

class CMenuShortcut;

// Key is the Control ID
typedef std::map<unsigned int, CMenuShortcut> MapMenuShortcuts;
typedef std::pair<unsigned int, CMenuShortcut> MapMenuShortcutsPair;
typedef MapMenuShortcuts::iterator MapMenuShortcutsIter;

// Key is the virtual key
typedef std::map<const st_KeyIDExt, const TCHAR *> MapKeyNameID;
typedef std::pair<const st_KeyIDExt, const TCHAR *> MapKeyNameIDPair;
typedef MapKeyNameID::const_iterator MapKeyNameIDConstIter;
typedef MapKeyNameID::iterator MapKeyNameIDIter;

class CMenuShortcut {
public:
  // Menu item text
  stringT name;
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
    cdefVirtKey = (unsigned char)0;
    cdefModifier = (unsigned char)0;
    cVirtKey = (unsigned char)0;
    cModifier = (unsigned char)0;
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

  static CString FormatShortcut(MapMenuShortcutsIter iter, 
                                MapKeyNameIDConstIter citer);

  static CString FormatShortcut(st_MenuShortcut mst, 
                                MapKeyNameIDConstIter citer);

private:
  static CString CS_CTRLP, CS_ALTP, CS_SHIFTP;
};

