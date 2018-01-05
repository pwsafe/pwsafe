/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "core/ItemData.h"
#include "core/coredefs.h"

#include "SMemFile.h"

// Drag & Drop Object written to SMemFile

class CDDObject : public CObject
{
  // Construction
public:
  CDDObject() {m_pbaseitem = NULL;};

  void DDSerializeEntry(CSMemFile &outDDmemfile);
  void DDUnSerializeEntry(CSMemFile &inDDmemfile);
  void FromItem(const CItemData &item) {m_item = item;}
  void ToItem(CItemData &item) const {item = m_item;}

  void SetBaseItem(const CItemData *item) {m_pbaseitem = item;}
  const CItemData *GetBaseItem() const {return m_pbaseitem;}
  bool IsAlias() const {return (m_pbaseitem != NULL);}

private:
  CItemData m_item;
  const CItemData *m_pbaseitem;
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
