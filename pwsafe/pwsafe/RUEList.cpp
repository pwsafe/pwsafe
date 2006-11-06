/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file RUEList.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "RUEList.h"
#include "corelib/PWScore.h"
#include "resource3.h"  // String resources

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// Constructors

CRUEList::CRUEList() : m_core(app.m_core), m_maxentries(0)
{
}

CRUEList::~CRUEList()
{
}

//-----------------------------------------------------------------------------
// Accessors

bool
CRUEList::SetMax(const int &newmax)
{
  if (newmax < 0) return false;

  m_maxentries = newmax;

  int numentries = m_RUEList.GetCount();

  if (newmax > numentries) return true;

  for (int i = numentries; i > newmax; i--)
    m_RUEList.RemoveTail();

  return true;
}

int
CRUEList::GetMax() const
{
  return m_maxentries;
}

int
CRUEList::GetCount() const
{
  return m_RUEList.GetCount();
}

bool
CRUEList::GetAllMenuItemStrings(CList<CMyString, CMyString&> &ListofAllMenuStrings) const
{
  CMyString itemstring;
  POSITION ruel_pos = m_RUEList.GetHeadPosition();
  bool retval = false;

  while (ruel_pos != NULL) {
    const RUEntry m_ruentry = m_RUEList.GetNext(ruel_pos);
    POSITION pw_listpos = m_core.Find(m_ruentry.RUEuuid);
    if (pw_listpos == NULL) {
      itemstring = _T("");
    } else {
      const CItemData &ci = m_core.GetEntryAt(pw_listpos);
      CMyString group = ci.GetGroup();
      CMyString title = ci.GetTitle();
      CMyString user = ci.GetUser();

      if (group.IsEmpty())
        group = _T("*");

      if (title.IsEmpty())
        title = _T("*");

      if (user.IsEmpty())
        user = _T("*");

      itemstring = MRE_FS + group + MRE_FS + title + MRE_FS + user + MRE_FS;
    }
    ListofAllMenuStrings.AddTail(itemstring);
    retval = true;
  }
  return retval;
}

bool
CRUEList::GetMenuItemString(const int &index, CMyString &itemstring) const
{
  if (m_RUEList.GetCount() == 0  || index > m_RUEList.GetCount()) {
    itemstring = _T("");
    return false;
  }

  POSITION re_listpos = m_RUEList.FindIndex(index);
  RUEntry m_ruentry = m_RUEList.GetAt(re_listpos);

  POSITION pw_listpos = m_core.Find(m_ruentry.RUEuuid);
  if (pw_listpos == NULL) {
    itemstring = _T("");
    return false;
  }

  const CItemData ci = m_core.GetEntryAt(pw_listpos);
  CMyString group = ci.GetGroup();
  CMyString title = ci.GetTitle();
  CMyString user = ci.GetUser();

  if (group.IsEmpty())
    group = _T("*");

  if (title.IsEmpty())
    title = _T("*");

  if (user.IsEmpty())
    user = _T("*");

  itemstring = MRE_FS + group + MRE_FS + title + MRE_FS + user + MRE_FS;
  return true;
}

bool
CRUEList::GetMenuItemString(const uuid_array_t &RUEuuid, CMyString &itemstring) const
{
  if (m_RUEList.GetCount() == 0) {
    itemstring = _T("");
    return false;
  }

  POSITION pw_listpos = m_core.Find(RUEuuid);
  if (pw_listpos == NULL) {
    itemstring = _T("");
    return false;
  }

  const CItemData ci = m_core.GetEntryAt(pw_listpos);
  CMyString group = ci.GetGroup();
  CMyString title = ci.GetTitle();
  CMyString user = ci.GetUser();

  if (group.IsEmpty())
    group = _T("*");

  if (title.IsEmpty())
    title = _T("*");

  if (user.IsEmpty())
    user = _T("*");

  itemstring = MRE_FS + group + MRE_FS + title + MRE_FS + user + MRE_FS;
  return true;
}

void
CRUEList::ClearEntries()
{
  m_RUEList.RemoveAll();
}

bool
CRUEList::AddRUEntry(const uuid_array_t &RUEuuid)
{
  if (m_maxentries == 0) return false;

  RUEntry re_newEntry;
  memcpy(re_newEntry.RUEuuid, RUEuuid, sizeof(uuid_array_t));

  POSITION re_listpos = m_RUEList.GetHeadPosition();
  while (re_listpos != NULL) {
    const RUEntry &re_FoundEntry = m_RUEList.GetAt(re_listpos);
    if (memcmp(re_FoundEntry.RUEuuid, re_newEntry.RUEuuid, sizeof(uuid_array_t)) == 0)
      break;  // found it already there
    else
      m_RUEList.GetNext(re_listpos);
  }

  if (re_listpos == NULL) {
    if (m_RUEList.GetCount() == m_maxentries)  // if already maxed out - delete oldest entry
      m_RUEList.RemoveTail();

    m_RUEList.AddHead(re_newEntry);  // put it at the top
  } else {
    m_RUEList.RemoveAt(re_listpos);  // take it out
    m_RUEList.AddHead(re_newEntry);  // put it at the top
  }
  return true;
}

bool
CRUEList::DeleteRUEntry(const int &index)
{
  if ((m_maxentries == 0) ||
      (m_RUEList.GetCount() == 0) ||
      (index > (m_maxentries - 1)) ||
      (index < 0) ||
      (index > (m_RUEList.GetCount() - 1))) return false;

  POSITION re_listpos = m_RUEList.FindIndex(index);
  ASSERT(re_listpos != NULL);

  m_RUEList.RemoveAt(re_listpos);
  return true;
}

bool
CRUEList::DeleteRUEntry(const uuid_array_t &RUEuuid)
{
  if ((m_maxentries == 0) ||
      (m_RUEList.GetCount() == 0)) return false;

  POSITION re_listpos = m_RUEList.GetHeadPosition();
  RUEntry re_oldEntry;
  memcpy(re_oldEntry.RUEuuid, RUEuuid, sizeof(uuid_array_t));

  while (re_listpos != NULL) {
    const RUEntry &re_FoundEntry = m_RUEList.GetAt(re_listpos);
    if (memcmp(re_FoundEntry.RUEuuid, re_oldEntry.RUEuuid, sizeof(uuid_array_t)) == 0) {
      m_RUEList.RemoveAt(re_listpos);
      break;
    }
    else
      m_RUEList.GetNext(re_listpos);
  }
  return true;
}

bool
CRUEList::GetPWEntry(const int &index, CItemData &ci)
{
  if ((m_maxentries == 0) ||
      (m_RUEList.GetCount() == 0) ||
      (index > (m_maxentries - 1)) ||
      (index < 0) ||
      (index > (m_RUEList.GetCount() - 1))) return false;

  POSITION re_listpos = m_RUEList.FindIndex(index);
  ASSERT(re_listpos != NULL);
  const RUEntry &re_FoundEntry = m_RUEList.GetAt(re_listpos);

  POSITION pw_listpos = m_core.Find(re_FoundEntry.RUEuuid);
  if (pw_listpos == NULL) {
    // Entry does not exist anymore!
    m_RUEList.RemoveAt(re_listpos);
    AfxMessageBox(IDS_CANTPROCESSENTRY);
  }
  if (pw_listpos == NULL)
    return false;

  ci = m_core.GetEntryAt(pw_listpos);
  return true;
}

CRUEList&
CRUEList::operator=(const CRUEList &that)
{
  if (this != &that) {
    m_maxentries = that.m_maxentries;
    POSITION that_pos = that.m_RUEList.GetHeadPosition();
    while (that_pos != NULL) {
      RUEntry ruentry = that.m_RUEList.GetNext(that_pos);
      m_RUEList.AddTail(ruentry);
    }
  }
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
