/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/file.h>
////@end includes

#include "core/ItemData.h"
#include "core/ItemAtt.h"
#include "core/PWScore.h"

// Drag & Drop Object written to SMemFile

class DnDObject
{
  // Construction
public:
  DnDObject() {m_pbaseitem = NULL;};
  ~DnDObject();

  void DnDSerializeEntry(wxMemoryBuffer &outDDmem);
  void DnDUnSerializeEntry(wxInputStream &inStream);
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
  bool HasAttRef() { return m_item.HasAttRef(); }
  void SetAttUUID(const pws_os::CUUID &uuid) { m_item.SetAttUUID(uuid); }

private:
  CItemData m_item;
  CItemData *m_pbaseitem;
};

typedef std::vector<DnDObject *>::iterator DnDIterator;
typedef std::vector<pws_os::CUUID>::iterator CUUIDIterator;
typedef std::vector<pws_os::CUUID> CUUIDVector;
typedef std::map<pws_os::CUUID, CUUIDVector>::iterator AttUuidMapIterator;
typedef std::vector<CItemAtt *>::iterator CItemAttIterator;

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
  
  bool HasAttachments() { return !m_attrefs.empty(); }
  
  CItemAttIterator AttachmentsBegin() { return m_attachments.begin(); }
  CItemAttIterator AttachmentsEnd() { return m_attachments.end(); }
  
  void UpdateBaseUUIDinDnDEntries(pws_os::CUUID &old_uuid, pws_os::CUUID &new_uuid);
  
  const CItemAtt *AttachmentOfBase(pws_os::CUUID &uuid);
  const CItemAtt *AttachmentOfItem(CItemData *item) { pws_os::CUUID uuid = item->GetUUID(); return AttachmentOfBase(uuid); }
  
public:
  void DnDSerialize(wxMemoryBuffer &outDDmem);
  void DnDUnSerialize(wxInputStream &inStream);
  void DnDSerializeAttachments(PWScore &core, wxMemoryBuffer &outDDmem);
  bool DnDSerializeAttachments(PWScore &core, wxFile *outFile);
  void DnDUnSerializeAttachments(wxInputStream &inStream);
  
  bool CanFind(pws_os::CUUID &uuid);
  
private:
  bool m_bDragNode;
  std::vector<DnDObject *> m_objects; // List of objects
  size_t m_dragPathParentLen;
  // For Attachment handling (on drag)
  std::map<pws_os::CUUID, CUUIDVector> m_attrefs; // List of attachments UUID
  // For Attachment handling (on drop)
  std::vector<CItemAtt *> m_attachments;
  std::map<pws_os::CUUID, CItemAtt *> m_uuid2atta; // List of attachments UUID for each base entry in m_objects
  
  void InsertAttUuid(const pws_os::CUUID attUuid, const pws_os::CUUID baseUuid);
};

#endif // _DNDSUPPORT_H_
