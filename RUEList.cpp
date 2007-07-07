/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file RUEList.cpp
//-----------------------------------------------------------------------------

#include <algorithm> // for find_if

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "RUEList.h"
#include "corelib/PWScore.h"
#include "resource3.h"  // String resources

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

RUEntry::RUEntry(const uuid_array_t &aRUEuuid)
{
  ::memcpy(RUEuuid, aRUEuuid, sizeof(uuid_array_t));
}

bool RUEntry::operator()(const RUEntry &re)
{
  return ::memcmp(RUEuuid, re.RUEuuid,
                  sizeof(uuid_array_t)) == 0;
}

//-----------------------------------------------------------------------------

CRUEList::CRUEList() : m_core(app.m_core), m_maxentries(0)
{}

// Accessors

void
CRUEList::SetMax(size_t newmax)
{
  m_maxentries = newmax;

  size_t numentries = m_RUEList.size();

  if (newmax < numentries)
    m_RUEList.resize(newmax);
}

bool
CRUEList::GetAllMenuItemStrings(vector<CMyString> &ListofAllMenuStrings) const
{
  CMyString itemstring;
  bool retval = false;

  RUEListConstIter iter;

  for (iter = m_RUEList.begin(); iter != m_RUEList.end(); iter++) {
    ItemListConstIter pw_listpos = m_core.Find(iter->RUEuuid);
    if (pw_listpos == m_core.GetEntryEndIter()) {
      itemstring = _T("");
    } else {
      const CItemData &ci = m_core.GetEntry(pw_listpos);
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
    ListofAllMenuStrings.push_back(itemstring);
    retval = true;
  }
  return retval;
}

bool
CRUEList::GetMenuItemString(size_t index, CMyString &itemstring) const
{
  if (m_RUEList.empty()  || index > m_RUEList.size()) {
    itemstring = _T("");
    return false;
  }

  RUEntry m_ruentry = m_RUEList[index];

  ItemListConstIter pw_listpos = m_core.Find(m_ruentry.RUEuuid);
  if (pw_listpos == m_core.GetEntryEndIter()) {
    itemstring = _T("");
    return false;
  }

  const CItemData ci = m_core.GetEntry(pw_listpos);
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
CRUEList::GetMenuItemString(const uuid_array_t &RUEuuid,
                            CMyString &itemstring) const
{
  if (m_RUEList.empty()) {
    itemstring = _T("");
    return false;
  }

  ItemListConstIter pw_listpos = m_core.Find(RUEuuid);
  if (pw_listpos == m_core.GetEntryEndIter()) {
    itemstring = _T("");
    return false;
  }

  const CItemData ci = m_core.GetEntry(pw_listpos);
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
CRUEList::AddRUEntry(const uuid_array_t &RUEuuid)
{
  /*
   * If the entry's already there, do nothing, return true.
   * Otherwise, add it, removing last entry if needed to
   * maintain size() <= m_maxentries invariant
   */
  if (m_maxentries == 0) return false;

  RUEntry match_uuid(RUEuuid);

  RUEListIter iter = find_if(m_RUEList.begin(), m_RUEList.end(), match_uuid);

  if (iter == m_RUEList.end()) {
    if (m_RUEList.size() == m_maxentries)  // if already maxed out - delete oldest entry
      m_RUEList.pop_back();

    m_RUEList.push_front(match_uuid);  // put it at the top
  } else {
    m_RUEList.erase(iter);  // take it out
    m_RUEList.push_front(match_uuid);  // put it at the top
  }
  return true;
}

bool
CRUEList::DeleteRUEntry(size_t index)
{
  if ((m_maxentries == 0) ||
      m_RUEList.empty() ||
      (index > (m_maxentries - 1)) ||
      (index > (m_RUEList.size() - 1))) return false;

  m_RUEList.erase(m_RUEList.begin() + index);
  return true;
}

bool
CRUEList::DeleteRUEntry(const uuid_array_t &RUEuuid)
{
  if ((m_maxentries == 0) || m_RUEList.empty())
    return false;

  RUEntry match_uuid(RUEuuid);

  RUEListIter iter = find_if(m_RUEList.begin(), m_RUEList.end(), match_uuid);

  if (iter != m_RUEList.end()) {
    m_RUEList.erase(iter);
  }
  return true;
}

bool
CRUEList::GetPWEntry(size_t index, CItemData &ci){
  if ((m_maxentries == 0) ||
      m_RUEList.empty() ||
      (index > (m_maxentries - 1)) ||
      (index > (m_RUEList.size() - 1))) return false;

  const RUEntry &re_FoundEntry = m_RUEList[index];

  ItemListConstIter pw_listpos = m_core.Find(re_FoundEntry.RUEuuid);
  if (pw_listpos == m_core.GetEntryEndIter()) {
    // Entry does not exist anymore!
    m_RUEList.erase(m_RUEList.begin() + index);
    AfxMessageBox(IDS_CANTPROCESSENTRY);
  }
  if (pw_listpos == m_core.GetEntryEndIter())
    return false;

  ci = m_core.GetEntry(pw_listpos);
  return true;
}

CRUEList&
CRUEList::operator=(const CRUEList &that)
{
  if (this != &that) {
    m_maxentries = that.m_maxentries;
    m_RUEList = that.m_RUEList;
  }
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
