/*
* Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
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

#include "DnDSupport.h"

#include <vector>

using namespace std;

DnDObject::~DnDObject()
{
  // Do not delete m_pbaseitem, as this one is only a pointer into m_core
}

DnDObList::~DnDObList()
{
  DnDIterator pos;

  for(pos = m_objects.begin(); pos != m_objects.end(); pos++) {
    delete (*pos);
  }
  m_objects.clear();
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

void DnDObject::DnDUnSerializeEntry(wxMemoryInputStream &inDDmem)
{
  // Deserialize an entry
  size_t len = 0;
  inDDmem.Read(&len, sizeof(size_t));
  wxASSERT((inDDmem.LastRead() == sizeof(len)) && (len != 0) && (wxFileOffset(len) <= inDDmem.GetLength()));
  vector<char> v(len);
  inDDmem.Read(&(*v.begin()), len);
  wxASSERT(inDDmem.LastRead() == len);
#if wxDEBUG_LEVEL
  bool result =
#endif
    m_item.DeSerializePlainText(v);
  wxASSERT(result);
  trashMemory(&(*v.begin()), v.size());
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
  }
}

void DnDObList::DnDUnSerialize(wxMemoryInputStream &inDDmem)
{
  // Deserialize all entries
  wxASSERT(GetCount() == 0);
  int n, nCount;

  inDDmem.Read((void *)&nCount, sizeof(int));
  wxASSERT(inDDmem.LastRead() == sizeof(int));
  inDDmem.Read((void *)&m_bDragNode, sizeof(bool));
  wxASSERT(inDDmem.LastRead() == sizeof(bool));

  for (n = 0; n < nCount; n++) {
    DnDObject *pDnDObject = new DnDObject();
    wxASSERT(pDnDObject);
    pDnDObject->DnDUnSerializeEntry(inDDmem);
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
