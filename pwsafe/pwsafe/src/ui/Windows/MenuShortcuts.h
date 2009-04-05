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

#include <map>
#include <vector>
#include <algorithm>

// Structure used for vector of reserved Hotkeys
struct st_MenuShortcut {
	unsigned char cVirtKey;
	bool bCtrl;
	bool bAlt;
	bool bShift;
  bool bExtended;
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

class CMenuShortcut {
public:
  // Menu item text
	const TCHAR *name;
  // Menu item's parent or zero
  UINT uiParentID;
  // Sequential menu item position (i.e. in going through the menus)
  // Used to sort shortcuts in the CListCtrl in OptionsShortcuts
  int iMenuPosition;
  // Default values
	unsigned char cdefVirtKey;
	bool bdefCtrl;
	bool bdefAlt;
	bool bdefShift;
  bool bdefExtended;
  // new User values
	unsigned char cVirtKey;
	bool bCtrl;
	bool bAlt;
	bool bShift;
  bool bExtended;

  CMenuShortcut()
  : name(NULL), uiParentID(0), iMenuPosition(0),
  cdefVirtKey(0), bdefCtrl(false), bdefAlt(false), bdefShift(false), bdefExtended(false),
  cVirtKey(0), bCtrl(false), bAlt(false), bShift(false), bExtended(false)
  {}

  CMenuShortcut(const CMenuShortcut &that)
  : name(_tcsdup(that.name)), uiParentID(that.uiParentID),
  iMenuPosition(that.iMenuPosition),
  cdefVirtKey(that.cdefVirtKey),
  bdefCtrl(that.bdefCtrl), bdefAlt(that.bdefAlt), bdefShift(that.bdefShift),
  bdefExtended(that.bdefExtended),
  cVirtKey(that.cVirtKey),
  bCtrl(that.bCtrl), bAlt(that.bAlt), bShift(that.bShift), bExtended(that.bExtended)
  {}

  ~CMenuShortcut()
  {
    free((void *)name);
  }

  void ClearKeyFlags()
  {
    // Only clear Key/flags - not menu position or name
    cdefVirtKey = (unsigned char)0;
    bdefCtrl = false;
    bdefAlt = false;
    bdefShift = false;
    bdefExtended = false;
    cVirtKey = (unsigned char)0;
    bCtrl = false;
    bAlt = false;
    bShift = false;
    bExtended = false;
  }

  void SetKeyFlags(const CMenuShortcut &that)
  {
    // Only set Key/flags - not menu position or name
    cdefVirtKey = that.cdefVirtKey;
    bdefCtrl = that.bdefCtrl;
    bdefAlt = that.bdefAlt;
    bdefShift = that.bdefShift;
    bdefExtended = that.bdefExtended;
    cVirtKey = that.cVirtKey;
    bCtrl = that.bCtrl;
    bAlt = that.bAlt;
    bShift = that.bShift;
    bExtended = that.bExtended;
  }

  CMenuShortcut &operator=(const CMenuShortcut &that)
  {
    if (this != &that) {
      free((void *)name);
      name = _tcsdup(that.name);
      uiParentID = that.uiParentID;
      iMenuPosition = that.iMenuPosition;
      cdefVirtKey = that.cdefVirtKey;
      bdefCtrl = that.bdefCtrl;
      bdefAlt = that.bdefAlt;
      bdefShift = that.bdefShift;
      bdefExtended = that.bdefExtended;
      cVirtKey = that.cVirtKey;
      bCtrl = that.bCtrl;
      bAlt = that.bAlt;
      bShift = that.bShift;
      bExtended = that.bExtended;
    }
    return *this;
  }
};

// Key is the Control ID
typedef std::map<UINT, CMenuShortcut> MapMenuShortcuts;
typedef std::pair<UINT, CMenuShortcut> MapMenuShortcutsPair;
typedef MapMenuShortcuts::iterator MapMenuShortcutsIter;

// Key is the virtual key
typedef std::map<const st_KeyIDExt, const TCHAR *> MapKeyNameID;
typedef std::pair<const st_KeyIDExt, const TCHAR *> MapKeyNameIDPair;
typedef MapKeyNameID::const_iterator MapKeyNameIDConstIter;
typedef MapKeyNameID::iterator MapKeyNameIDIter;
