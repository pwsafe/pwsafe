/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "DDSupport.h"
#include "SecString.h"
#include "core/util.h"

#include <vector>

using namespace std;

void CDDObject::DDSerializeEntry(CSMemFile &outDDmemfile)
{
  // Serialize an entry
  vector<char> v;
  const CItemData *pbci(NULL);
  if (m_item.IsDependent()) {
    pbci = GetBaseItem();
    ASSERT(pbci != NULL);
  }
 
  m_item.SerializePlainText(v, pbci);
  size_t len = v.size();
  outDDmemfile.Write(&len, sizeof(len));
  outDDmemfile.Write(&(*v.begin()), (UINT)v.size());
  trashMemory(&(*v.begin()), v.size());
}

void CDDObject::DDUnSerializeEntry(CSMemFile &inDDmemfile)
{
  // Deserialize an entry
  size_t len = 0;
  VERIFY(inDDmemfile.Read(&len, sizeof(len)) == sizeof(len) && len != 0);
  vector<char> v(len);
  VERIFY(inDDmemfile.Read(&(*v.begin()), (UINT)len) == len);
  VERIFY(m_item.DeSerializePlainText(v));
  trashMemory(&(*v.begin()), v.size());
}

void CDDObList::DDSerialize(CSMemFile &outDDmemfile)
{
  // NOTE:  Do not call the base class!
  // Serialize ALL entries
  int nCount;
  POSITION pos;

  nCount = (int)GetCount();

  outDDmemfile.Write((void *)&nCount, sizeof(nCount));
  outDDmemfile.Write((void *)&m_bDragNode, sizeof(bool));

  pos = GetHeadPosition();
  while (pos != NULL) {
    CDDObject *pDDObject = (CDDObject *)GetNext(pos);
    pDDObject->DDSerializeEntry(outDDmemfile);
  }
}

void CDDObList::DDUnSerialize(CSMemFile &inDDmemfile)
{
  // Deserialize all entries
  ASSERT(GetCount() == 0);
  int n, nCount;

  inDDmemfile.Read((void *)&nCount, sizeof(nCount));
  inDDmemfile.Read((void *)&m_bDragNode, sizeof(bool));

  for (n = 0; n < nCount; n++) {
    CDDObject *pDDObject = new CDDObject();
    pDDObject->DDUnSerializeEntry(inDDmemfile);
    AddTail(pDDObject);
  }
}
