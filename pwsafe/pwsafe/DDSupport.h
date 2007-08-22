/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#pragma once
#include "corelib/ItemData.h"
#include "corelib/SMemFile.h"

// Drag & Drop Object written to SMemFile

class CDDObject : public CObject
{
// Construction
public:
  CDDObject() {};

  void DDSerialize(CSMemFile &outDDmemfile);
  void DDUnSerialize(CSMemFile &inDDmemfile);
  void FromItem(const CItemData &item) {m_item = item;}
  void ToItem(CItemData &item) const {item = m_item;}

 private:
  CItemData m_item;
};

// A list of Drag & Drop Objects

class CDDObList : public CObList
{
// Construction
public:
  CDDObList() {};

// Implementation
public:
  void DDSerialize(CSMemFile &outDDmemfile);
  void DDUnSerialize(CSMemFile &inDDmemfile);

public:
  bool m_bDragNode;
};
