/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/


/** \file DndSupport.cpp
* 
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/file.h>

#include "DnDSupport.h"

#include "core/ItemData.h"
#include "core/ItemAtt.h"
#include "core/PWScore.h"

#include <vector>

using namespace std;

DnDObject::~DnDObject()
{
  // Do not delete m_pbaseitem, as this one is only a pointer into m_core
}

DnDObList::~DnDObList()
{
  // Clear DND objects iterator
  for(DnDIterator pos = m_objects.begin(); pos != m_objects.end(); pos++) {
    delete (*pos);
  }
  m_objects.clear();
  // Clear map of UUIDs
  for(auto iter = m_attrefs.begin(); iter != m_attrefs.end(); iter++) {
    iter->second.clear();
  }
  m_attrefs.clear();
  // Clear list of attachments
  for(CItemAttIterator iter = m_attachments.begin(); iter != m_attachments.end(); iter++) {
    delete (*iter);
  }
  m_attachments.clear();
  m_uuid2atta.clear();
}

void DnDObject::DnDSerializeEntry(wxMemoryBuffer &outDDmem)
{
  // Serialize an entry
  vector<char> v;
  const CItemData *pbci(NULL);
  if (m_item.IsDependent()) {
    pbci = GetBaseItem();
    wxASSERT(pbci != NULL);
  }
 
  m_item.SerializePlainText(v, pbci);
  size_t len = v.size();
  outDDmem.AppendData(&len, sizeof(size_t));
  outDDmem.AppendData(&(*v.begin()), static_cast<size_t>(v.size()));
  trashMemory(&(*v.begin()), v.size());
}

void DnDObject::DnDUnSerializeEntry(wxInputStream &inStream)
{
  // Deserialize an entry
  size_t len = 0;
  inStream.Read(&len, sizeof(size_t));
  wxASSERT((inStream.LastRead() == sizeof(len)) && (len != 0) && (static_cast<wxFileOffset>(len) <= inStream.GetLength()));
  vector<char> v(len);
  inStream.Read(&(*v.begin()), len);
  wxASSERT(inStream.LastRead() == len);
#if wxDEBUG_LEVEL
  bool result =
#endif
    m_item.DeSerializePlainText(v);
  wxASSERT(result);
  trashMemory(&(*v.begin()), v.size());
}

void DnDObList::InsertAttUuid(const pws_os::CUUID attUuid, const pws_os::CUUID baseUuid)
{
  auto search = m_attrefs.find(attUuid);
  if(search == m_attrefs.end()) {
    // new entry
    CUUIDVector baseentry;
    baseentry.push_back(baseUuid);
    m_attrefs.insert(std::make_pair(attUuid, baseentry));
  }
  else {
    // Update of entry, we add only the new base entry to the list of entries related the attachement
    search->second.push_back(baseUuid);
  }
}

void DnDObList::DnDSerialize(wxMemoryBuffer &outDDmem)
{
  // NOTE:  Do not call the base class!
  // Serialize ALL entries
  int nCount;
  DnDIterator pos;

  nCount = (int)GetCount();

  outDDmem.AppendData((void *)&nCount, sizeof(int));
  outDDmem.AppendData((void *)&m_bDragNode, sizeof(bool));

  for(pos = m_objects.begin(); pos != m_objects.end(); pos++) {
    (*pos)->DnDSerializeEntry(outDDmem);
    if((*pos)->HasAttRef()) {
      InsertAttUuid((*pos)->GetUUID(CItemData::ATTREF), (*pos)->GetUUID(CItemData::UUID));
    }
  }
}

void DnDObList::DnDUnSerialize(wxInputStream &inStream)
{
  // Deserialize all entries
  wxASSERT(GetCount() == 0);
  int n, nCount;

  inStream.Read((void *)&nCount, sizeof(int));
  wxASSERT(inStream.LastRead() == sizeof(int));
  inStream.Read((void *)&m_bDragNode, sizeof(bool));
  wxASSERT(inStream.LastRead() == sizeof(bool));

  for (n = 0; n < nCount; n++) {
    DnDObject *pDnDObject = new DnDObject();
    wxASSERT(pDnDObject);
    pDnDObject->DnDUnSerializeEntry(inStream);
    m_objects.push_back(pDnDObject);
  }
}

bool DnDObList::CanFind(pws_os::CUUID &uuid)
{
  DnDIterator pos;
  
  for(pos = m_objects.begin(); pos != m_objects.end(); pos++) {
    if((*pos)->GetUUID() == uuid)
      return true;
  }
  return false;
}


void DnDObList::DnDSerializeAttachments(PWScore &core, wxMemoryBuffer &outDDmem)
{
  size_t natt = m_attrefs.size();
  outDDmem.AppendData((void *)&natt, sizeof(size_t));
  for(AttUuidMapIterator iter = m_attrefs.begin(); iter != m_attrefs.end(); ++iter) {
    // Write Attachement entry first
    pws_os::CUUID uuid = iter->first;
    CItemAtt item = core.GetAtt(uuid);
    std::vector<char> v;
    item.SerializePlainText(v);
    size_t len = v.size();
    outDDmem.AppendData(&len, sizeof(size_t));
    outDDmem.AppendData(&(*v.begin()), len);
    trashMemory(&(*v.begin()), v.size());
    // Write list with depending UUID Base entries
    CUUIDVector vect = iter->second;
    len = vect.size();
    outDDmem.AppendData(&len, sizeof(size_t));
    for(size_t i = 0; i < len; ++i) {
      outDDmem.AppendData(&vect[i], sizeof(pws_os::CUUID));
    }
  }
}

bool DnDObList::DnDSerializeAttachments(PWScore &core, wxFile *outFile)
{
  size_t natt = m_attrefs.size();
  if(outFile->Write((void *)&natt, sizeof(size_t)) != sizeof(size_t))
    return false;
  for(AttUuidMapIterator iter = m_attrefs.begin(); iter != m_attrefs.end(); ++iter) {
    // Write Attachement entry first
    pws_os::CUUID uuid = iter->first;
    CItemAtt item = core.GetAtt(uuid);
    std::vector<char> v;
    item.SerializePlainText(v);
    size_t len = v.size();
    if(outFile->Write(&len, sizeof(size_t)) != sizeof(size_t))
      return false;
    if(outFile->Write(&(*v.begin()), len) != len)
      return false;
    trashMemory(&(*v.begin()), v.size());
    // Write list with depending UUID Base entries
    CUUIDVector vect = iter->second;
    len = vect.size();
    if(outFile->Write(&len, sizeof(size_t)) != sizeof(size_t))
      return false;
    for(size_t i = 0; i < len; ++i) {
      if(outFile->Write(&vect[i], sizeof(pws_os::CUUID)) != sizeof(pws_os::CUUID))
        return false;
    }
  }
  return true;
}

void DnDObList::DnDUnSerializeAttachments(wxInputStream &inStream)
{
  // Deserialize all attachment entries
  size_t n, nCount, length;

  inStream.Read((void *)&nCount, sizeof(size_t));
  wxASSERT(inStream.LastRead() == sizeof(size_t));

  for (n = 0; n < nCount; n++) {
    // Read length
    inStream.Read((void *)&length, sizeof(size_t));
    wxASSERT(inStream.LastRead() == sizeof(size_t));
    // Fill vector with data
    vector<char> v(length);
    inStream.Read(&(*v.begin()), length);
    wxASSERT(inStream.LastRead() == length);
    // Allocate new attachment Item
    CItemAtt *pDnDObject = new CItemAtt();
    wxASSERT(pDnDObject);
    pDnDObject->DeSerializePlainText(v);
    // Store new attachment in list
    m_attachments.push_back(pDnDObject);
    inStream.Read((void *)&length, sizeof(size_t));
    wxASSERT(inStream.LastRead() == sizeof(size_t));
    // Read Attachment UUID
    const pws_os::CUUID attUuid = pDnDObject->GetUUID();
    // Add all base entries related to this one into list
    for(size_t i = 0; i < length; ++i) {
      pws_os::CUUID uuid;
      inStream.Read((void *)&uuid, sizeof(pws_os::CUUID));
      wxASSERT(inStream.LastRead() == sizeof(pws_os::CUUID));
      m_uuid2atta.insert(std::make_pair(uuid, pDnDObject));
    }
  }
}

void DnDObList::UpdateBaseUUIDinDnDEntries(pws_os::CUUID &old_uuid, pws_os::CUUID &new_uuid)
{
  wxASSERT((old_uuid != pws_os::CUUID::NullUUID()) && (new_uuid != pws_os::CUUID::NullUUID()));
  for(DnDIterator pos = m_objects.begin(); pos != m_objects.end(); ++pos) {
    DnDObject *pDnDObject = *pos;
    wxASSERT(pDnDObject);
    pws_os::CUUID uuid = pDnDObject->GetUUID();
    if(pDnDObject->GetBaseUUID() == old_uuid) {
      pDnDObject->SetBaseUUID(new_uuid);
    }
  }
  auto search = m_uuid2atta.find(old_uuid);
  if(search != m_uuid2atta.end()) {
    m_uuid2atta.insert(std::make_pair(new_uuid, search->second));
    m_uuid2atta.erase(old_uuid);
  }
}

const CItemAtt *DnDObList::AttachmentOfBase(pws_os::CUUID &uuid)
{
  auto search = m_uuid2atta.find(uuid);
  if(search != m_uuid2atta.end()) {
    return search->second;
  }
  return nullptr;
}
