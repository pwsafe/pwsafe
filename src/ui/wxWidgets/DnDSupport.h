/*
* Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/


/** \file DndSupport.h
 * 
 */

#ifndef _DNDSUPPORT_H_
#define _DNDSUPPORT_H_

/*!
 * Includes
 */

////@begin includes
#include <vector>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/mstream.h>
////@end includes

#include "core/ItemData.h"

// Drag & Drop Object written to SMemFile

class DnDObject
{
  // Construction
public:
  DnDObject() {m_pbaseitem = NULL;};
  ~DnDObject();

  void DnDSerializeEntry(wxMemoryBuffer &outDDmem);
  void DnDUnSerializeEntry(wxMemoryInputStream &inDDmem);
  void FromItem(const CItemData &item) { m_item = item; }
  void ToItem(CItemData &item) const { item = m_item; }

  void SetBaseItem(CItemData *item) { m_pbaseitem = item; }
  const CItemData *GetBaseItem() const { return m_pbaseitem; }
  bool IsAlias() const { return (m_pbaseitem != NULL); }
  
  const pws_os::CUUID GetUUID(CItemData::FieldType ft = CItemData::END) { return m_item.GetUUID(ft); }
  void CreateUUID(CItemData::FieldType ft = CItemData::END) { m_item.CreateUUID(ft); }
  const pws_os::CUUID GetBaseUUID()  { return m_item.GetBaseUUID(); }
  void SetBaseUUID(const pws_os::CUUID &uuid)  {
    m_item.SetBaseUUID(uuid);
    if(m_pbaseitem) m_pbaseitem->SetUUID(uuid);
  }

private:
  CItemData m_item;
  CItemData *m_pbaseitem;
};

typedef std::vector<DnDObject *>::iterator DnDIterator;

// A list of Drag & Drop Objects

class DnDObList
{
  // Construction
public:
  DnDObList() { m_dragPathParentLen = 0; }
  DnDObList(size_t l) { m_dragPathParentLen = l; }
  ~DnDObList();

  // Implementation
  size_t GetCount() { return m_objects.size(); }
  bool IsEmpty() { return m_objects.empty(); }
  void AddTail(DnDObject *o) { m_objects.push_back(o); }
  DnDIterator ObjectsBegin() { return m_objects.begin(); }
  DnDIterator ObjectsEnd() { return m_objects.end(); }

  size_t DragPathParentLen() { return m_dragPathParentLen; }
  void SetDragPathParentLen(size_t l) { m_dragPathParentLen = l; }
  
  void SetDragNode(bool cutGroupPath) { m_bDragNode = cutGroupPath; }
  bool CutGroupPath() { return m_bDragNode; }
  
public:
  void DnDSerialize(wxMemoryBuffer &outDDmem);
  void DnDUnSerialize(wxMemoryInputStream &inDDmem);
  
  bool CanFind(pws_os::CUUID &uuid);
  
private:
  bool m_bDragNode;
  std::vector<DnDObject *> m_objects; // List of objects
  size_t m_dragPathParentLen;
};

#endif // _DNDSUPPORT_H_
