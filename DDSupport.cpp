/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#include "DDSupport.h"
#include "corelib/MyString.h"
#include "corelib/util.h"
#include "corelib/UUIDGen.h"

IMPLEMENT_SERIAL(CDDObject, CObject, VERSIONABLE_SCHEMA | CDDObject_Version)

void CDDObject::Serialize(CArchive& ar)
{
  if (ar.IsStoring()) {
    ar << m_nVersion;

    ar.Write(m_DD_UUID, sizeof(uuid_array_t));

    ar << (CString)m_DD_Group;
    ar << (CString)m_DD_Title;
    ar << (CString)m_DD_User;
    ar << (CString)m_DD_Notes;
    ar << (CString)m_DD_Password;
    ar << (CString)m_DD_URL;
    ar << (CString)m_DD_AutoType;
    ar << (CString)m_DD_PWHistory;

    ar << m_DD_CTime;
    ar << m_DD_PMTime;
    ar << m_DD_ATime;
    ar << m_DD_LTime;
    ar << m_DD_RMTime;
  } else {
    int iVersion;
    ar >> iVersion;

    switch(iVersion) {
      case 0x100:
        {
        CString cs_Group, cs_Title, cs_User, cs_Notes, cs_Password, 
                cs_URL, cs_AutoType, cs_PWHistory;

        ar.Read(m_DD_UUID, sizeof(uuid_array_t));

        ar >> cs_Group;
        ar >> cs_Title;
        ar >> cs_User;
        ar >> cs_Notes;
        ar >> cs_Password;
        ar >> cs_URL;
        ar >> cs_AutoType;
        ar >> cs_PWHistory;

        ar >> m_DD_CTime;
        ar >> m_DD_PMTime;
        ar >> m_DD_ATime;
        ar >> m_DD_LTime;
        ar >> m_DD_RMTime;

        m_DD_Group = CMyString(cs_Group);
        m_DD_Title = CMyString(cs_Title);
        m_DD_User = CMyString(cs_User);
        m_DD_Notes = CMyString(cs_Notes);
        m_DD_Password = CMyString(cs_Password);
        m_DD_URL = CMyString(cs_URL);
        m_DD_AutoType = CMyString(cs_AutoType);
        m_DD_PWHistory = CMyString(cs_PWHistory);

        trashStringMemory(cs_Group);
        trashStringMemory(cs_Title);
        trashStringMemory(cs_User);
        trashStringMemory(cs_Notes);
        trashStringMemory(cs_Password);
        trashStringMemory(cs_URL);
        trashStringMemory(cs_AutoType);
        trashStringMemory(cs_PWHistory);
        }

        break;
      default:
        break;
    }
  }
  CObject::Serialize(ar);
}

void CDDObList::Serialize(CArchive& ar)
{
  // NOTE:  Do not call the base class!
  int n, nCount;
  POSITION Pos;
  CDDObject* pDDObject;

  if (ar.IsStoring()) {
    nCount = GetCount();

    ar << nCount;
    ar << m_bDragNode;

    Pos = GetHeadPosition();
    while (Pos != NULL) {
      pDDObject = (CDDObject *)GetNext(Pos);
      pDDObject->Serialize(ar);
    }
  } else {
    ASSERT(GetCount() == 0);

    ar >> nCount;
    ar >> m_bDragNode;

    for (n = 0; n < nCount; n++) {
      pDDObject = new CDDObject();
      pDDObject->Serialize(ar);
      AddTail(pDDObject);
    }
  }
}
