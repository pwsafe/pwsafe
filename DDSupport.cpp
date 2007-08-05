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

    uuid_array_t uuid;
    m_item.GetUUID(uuid);
    ar.Write(uuid, sizeof(uuid_array_t));

    ar << (CString)m_item.GetGroup();
    ar << (CString)m_item.GetTitle();
    ar << (CString)m_item.GetUser();
    ar << (CString)m_item.GetNotes();
    ar << (CString)m_item.GetPassword();
    ar << (CString)m_item.GetURL();
    ar << (CString)m_item.GetAutoType();
    ar << (CString)m_item.GetPWHistory();

    time_t t;
    m_item.GetCTime(t);  ar << t;
    m_item.GetPMTime(t); ar << t;
    m_item.GetATime(t);  ar << t;
    m_item.GetLTime(t);  ar << t;
    m_item.GetRMTime(t); ar << t;
    // XXX TBD - Unknown fields
  } else { // !Storing
    int iVersion;
    ar >> iVersion;

    switch(iVersion) {
    case 0x100:
      {
        CString cs;
        uuid_array_t uuid;
        ar.Read(uuid, sizeof(uuid_array_t));
        m_item.SetUUID(uuid);
        ar >> cs; m_item.SetGroup(cs);
        ar >> cs; m_item.SetTitle(cs);
        ar >> cs; m_item.SetUser(cs);
        ar >> cs; m_item.SetNotes(cs);
        ar >> cs; m_item.SetPassword(cs);
        ar >> cs; m_item.SetURL(cs);
        ar >> cs; m_item.SetAutoType(cs);
        ar >> cs; m_item.SetPWHistory(cs);
        trashMemory(cs);

        time_t t;
        ar >> t; m_item.SetCTime(t);
        ar >> t; m_item.SetPMTime(t);
        ar >> t; m_item.SetATime(t);
        ar >> t; m_item.SetLTime(t);
        ar >> t; m_item.SetRMTime(t);
      }
      break;
    default:
      TRACE(_T("CDDObject::Serialize() - unsupported version %x\n"), iVersion);
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
  } else { // !IsStoring
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
