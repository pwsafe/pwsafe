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
  // new User values
	unsigned char cVirtKey;
	bool bCtrl;
	bool bAlt;
	bool bShift;

  CMenuShortcut()
  : name(NULL), uiParentID(0), iMenuPosition(0),
  cdefVirtKey(0), bdefCtrl(false), bdefAlt(false), bdefShift(false),
  cVirtKey(0), bCtrl(false), bAlt(false), bShift(false)
  {}

  CMenuShortcut(const CMenuShortcut &that)
  : name(_tcsdup(that.name)), uiParentID(that.uiParentID),
  iMenuPosition(that.iMenuPosition),
  cdefVirtKey(that.cdefVirtKey),
  bdefCtrl(that.bdefCtrl), bdefAlt(that.bdefAlt), bdefShift(that.bdefShift),
  cVirtKey(that.cVirtKey),
  bCtrl(that.bCtrl), bAlt(that.bAlt), bShift(that.bShift)
  {}

  ~CMenuShortcut()
  {
    free((void *)name);
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
      cVirtKey = that.cVirtKey;
      bCtrl = that.bCtrl;
      bAlt = that.bAlt;
      bShift = that.bShift;
    }
    return *this;
  }
};

// Key is the Control ID
typedef std::map<UINT, CMenuShortcut> MapMenuShortcuts;
typedef std::pair<UINT, CMenuShortcut> MapMenuShortcutsPair;
typedef MapMenuShortcuts::iterator MapMenuShortcutsIter;

// Key is the virtual key
typedef std::map<const unsigned char, const TCHAR *> MapKeyNameID;
typedef std::pair<const unsigned char, const TCHAR *> MapKeyNameIDPair;
typedef MapKeyNameID::const_iterator MapKeyNameIDConstIter;
typedef MapKeyNameID::iterator MapKeyNameIDIter;
