/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#pragma once
#include "corelib/ItemData.h"

// Drag & Drop Object written to SMemFile

// Note: If the class CDDObject or CDDObList is changed, increase the Version
// and add code to CDDObject::Serialize and CDDObList::Serialize to handle the changes

// NOTE: If these classes change, user CANNOT drag to a release with a lower version!!!

#define CDDObject_Version 0x100

class CDDObject : public CObject
{
// Construction
public:
  CDDObject()
    : m_nVersion(CDDObject_Version) {};

  virtual void Serialize(CArchive& ar);
  void FromItem(const CItemData &item) {m_item = item;}
  void ToItem(CItemData &item) const {item = m_item;}

 private:
  CItemData m_item;
  int m_nVersion;

  DECLARE_SERIAL(CDDObject)
};

// A list of Drag & Drop Objects

class CDDObList : public CObList
{
// Construction
public:
  CDDObList() {};

// Implementation
public:
  virtual void Serialize(CArchive& ar);

public:
  bool m_bDragNode;
};
