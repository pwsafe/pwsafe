/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "DDSupport.h"
#include "SecString.h"
#include "corelib/util.h"
#include "corelib/UUIDGen.h"

#include <vector>

using namespace std;

void CDDObject::DDSerialize(CSMemFile &outDDmemfile)
{
  vector<char> v;
  CItemData *cibase(NULL);
  if (m_item.IsAlias() || m_item.IsShortcut()) {
    cibase = GetBaseItem();
  }
  m_item.SerializePlainText(v, cibase);
  size_t len = v.size();
  outDDmemfile.Write(&len, sizeof(len));
  outDDmemfile.Write(&(*v.begin()), (UINT)v.size());
  trashMemory(&(*v.begin()), v.size());
}

void CDDObject::DDUnSerialize(CSMemFile &inDDmemfile)
{
  size_t len = 0;
  size_t lenlen = inDDmemfile.Read(&len, sizeof(len));
  ASSERT(lenlen == sizeof(len) && len != 0);
  vector<char> v(len);
  size_t lenRead = inDDmemfile.Read(&(*v.begin()), (UINT)len);
  ASSERT(lenRead == len);
  bool status = m_item.DeserializePlainText(v);
  ASSERT(status);
  trashMemory(&(*v.begin()), v.size());
}

void CDDObList::DDSerialize(CSMemFile &outDDmemfile)
{
  // NOTE:  Do not call the base class!
  int nCount;
  POSITION Pos;
  CDDObject* pDDObject;

  nCount = (int)GetCount();

  outDDmemfile.Write((void *)&nCount, sizeof(nCount));
  outDDmemfile.Write((void *)&m_bDragNode, sizeof(bool));

  Pos = GetHeadPosition();
  while (Pos != NULL) {
    pDDObject = (CDDObject *)GetNext(Pos);
    pDDObject->DDSerialize(outDDmemfile);
  }
}

void CDDObList::DDUnSerialize(CSMemFile &inDDmemfile)
{
  ASSERT(GetCount() == 0);
  int n, nCount;
  CDDObject* pDDObject;

  inDDmemfile.Read((void *)&nCount, sizeof(nCount));
  inDDmemfile.Read((void *)&m_bDragNode, sizeof(bool));

  for (n = 0; n < nCount; n++) {
    pDDObject = new CDDObject();
    pDDObject->DDUnSerialize(inDDmemfile);
    AddTail(pDDObject);
  }
}
