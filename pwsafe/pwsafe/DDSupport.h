/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#pragma once

#include "corelib/MyString.h"
#include "corelib/UUIDGen.h"

// Drag & Drop Object written to SMemFile

// Note: If the class CDDObject or CDDObList is changed, increase the Version
// and add code to CDDObject::Serialize and CDDObList::Serialize to handle the changes

// NOTE: If these classes change, user CANNOT drag to a release with a lower version!!!

#define CDDObject_Version 0x100

class CDDObject : public CObject
{
// Construction
public:
  CDDObject()
    : m_nVersion(CDDObject_Version) {};

// Implementation
public:
  virtual void Serialize(CArchive& ar);

public:
  uuid_array_t m_DD_UUID;
  CMyString m_DD_Group;
  CMyString m_DD_Title;
  CMyString m_DD_User;
  CMyString m_DD_Notes;
  CMyString m_DD_Password;
  CMyString m_DD_URL;
  CMyString m_DD_AutoType;
  CMyString m_DD_PWHistory;
  long m_DD_CTime;
  long m_DD_PMTime;
  long m_DD_ATime;
  long m_DD_LTime;
  long m_DD_RMTime;
  int m_nVersion;

  DECLARE_SERIAL(CDDObject)
};

// List of Drag & Drop Object written to DDMemFile

class CDDObList : public CObList
{
// Construction
public:
  CDDObList() {};

// Implementation
public:
  virtual void Serialize(CArchive& ar);

public:
  bool m_bDragNode;
};
