/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file RUEList.cpp
//-----------------------------------------------------------------------------

#include <algorithm> // for find_if

#include "RUEList.h"
#include "PWScore.h"

using namespace std;

//-----------------------------------------------------------------------------

// Accessors

void CRUEList::SetMax(size_t newmax)
{
  m_maxentries = newmax;

  size_t numentries = m_RUEList.size();

  if (newmax < numentries)
    m_RUEList.resize(newmax);
}

bool CRUEList::GetAllMenuItemStrings(vector<RUEntryData> &ListofAllMenuStrings) const
{
  RUEntryData ruentrydata;
  bool retval = false;

  for (auto iter = m_RUEList.begin(); iter != m_RUEList.end(); iter++) {
    ItemListConstIter pw_listpos = m_core.Find(*iter);
    if (pw_listpos == m_core.GetEntryEndIter()) {
      ruentrydata.string = L"";
      ruentrydata.pci = nullptr;
    } else {
      const CItemData &ci = m_core.GetEntry(pw_listpos);
      StringX group = ci.GetGroup();
      StringX title = ci.GetTitle();
      StringX user = ci.GetUser();

      if (group.empty())
        group = L" ";

      if (title.empty())
        title = L" ";

      if (user.empty())
        user = L" ";

      // Looks similar to <g><t><u>
      ruentrydata.string = L"\xab" + group + L"\xbb " + 
                           L"\xab" + title + L"\xbb " + 
                           L"\xab" + user  + L"\xbb";
      ruentrydata.pci = (CItemData *)&ci;
    }
    ListofAllMenuStrings.push_back(ruentrydata);
    retval = true;
  }
  return retval;
}

bool CRUEList::AddRUEntry(const pws_os::CUUID &RUEuuid)
{
  /*
  * If the entry's already there, do nothing, return true.
  * Otherwise, add it, removing last entry if needed to
  * maintain size() <= m_maxentries invariant
  */
  if (m_maxentries == 0) return false;

  auto iter = find(m_RUEList.begin(), m_RUEList.end(), RUEuuid);

  if (iter == m_RUEList.end()) {
    if (m_RUEList.size() == m_maxentries)  // if already maxed out - delete oldest entry
      m_RUEList.pop_back();
  } else {
    m_RUEList.erase(iter);  // take it out
  }
  m_RUEList.push_front(RUEuuid);  // put it at the top
  return true;
}

bool CRUEList::DeleteRUEntry(size_t index)
{
  if ((m_maxentries == 0) ||
    m_RUEList.empty() ||
    (index > (m_maxentries - 1)) ||
    (index > (m_RUEList.size() - 1))) return false;

  m_RUEList.erase(m_RUEList.begin() + index);
  return true;
}

bool CRUEList::DeleteRUEntry(const pws_os::CUUID &RUEuuid)
{
  if ((m_maxentries == 0) || m_RUEList.empty())
    return false;

  auto iter = find(m_RUEList.begin(), m_RUEList.end(), RUEuuid);

  if (iter != m_RUEList.end()) {
    m_RUEList.erase(iter);
  }
  return true;
}

bool CRUEList::GetPWEntry(size_t index, CItemData &ci) const
{
  if ((m_maxentries == 0) || m_RUEList.empty() ||
     (index > (m_maxentries - 1)) ||
     (index > (m_RUEList.size() - 1)))
    return false;

  const pws_os::CUUID &re_FoundEntry = m_RUEList[index];

  ItemListConstIter pw_listpos = m_core.Find(re_FoundEntry);
  if (pw_listpos == m_core.GetEntryEndIter()) {
    // Entry does not exist anymore!
    // We break constness here since this is "housekeeping", and
    // doesn't affect the object's semantics
    RUEList::iterator iter = const_cast<CRUEList *>(this)->m_RUEList.begin() + index;
    const_cast<CRUEList *>(this)->m_RUEList.erase(iter);
    return false;
  } else { // valid pw_listpos
    ci = m_core.GetEntry(pw_listpos);
    ASSERT(ci.GetUUID() == re_FoundEntry);
    return true;
  }
}

void CRUEList::GetRUEList(UUIDList &RUElist) const
{
  RUElist.clear();
  RUElist.resize(m_RUEList.size());
  std::copy(m_RUEList.begin(), m_RUEList.end(), RUElist.begin());
}

void CRUEList::SetRUEList(const UUIDList &RUElist)
{
  m_RUEList.clear();
  m_RUEList.resize(RUElist.size());
  std::copy(RUElist.rbegin(), RUElist.rend(), m_RUEList.begin());
  if (m_RUEList.size() > m_maxentries)
    m_RUEList.resize(m_maxentries);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
