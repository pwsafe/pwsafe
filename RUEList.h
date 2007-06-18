/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// RUEList.h
//-----------------------------------------------------------------------------

#include <deque>
#include <vector>
#include "corelib/ItemData.h"
#include "corelib/MyString.h"
#include "corelib/PWScore.h"
#include "corelib/UUIDGen.h"

//-----------------------------------------------------------------------------

/*
 * CRUEList is a class that contains the recently used entries
 *
 */

// Following is Most Recent Entry field separator for dynamic menu:
#define MRE_FS _T("\xbb")

// Recent Entry structure for m_RUEList
struct RUEntry {
  RUEntry() {}
  RUEntry(const uuid_array_t &RUEuuid);
  bool operator() (const RUEntry &); // for find_if
  uuid_array_t RUEuuid;
};

typedef std::deque<RUEntry> RUEList;
typedef RUEList::iterator RUEListIter;
typedef RUEList::const_iterator RUEListConstIter;

class CRUEList
{
 public:
  // Construction/Destruction/operators
  CRUEList();
  ~CRUEList() {}

  CRUEList& operator=(const CRUEList& second);

  // Data retrieval
  size_t GetCount() const {return m_RUEList.size();}
  size_t GetMax() const {return m_maxentries;}
  bool GetAllMenuItemStrings(std::vector<CMyString> &) const;
  bool GetMenuItemString(size_t, CMyString &) const;
  bool GetMenuItemString(const uuid_array_t &, CMyString &) const;
  bool GetPWEntry(size_t, CItemData &); // NOT const!

  // Data setting
  void SetMax(size_t);
  void ClearEntries() {m_RUEList.clear();}
  bool AddRUEntry(const uuid_array_t &);
  bool DeleteRUEntry(size_t);
  bool DeleteRUEntry(const uuid_array_t &);

 private:
  PWScore &m_core;    // Dboxmain's m_core (which = app.m_core!)
  size_t m_maxentries;
  RUEList m_RUEList;  // Recently Used Entry History List
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
