/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// RUEList.h
// Utility class for managing recently used entries
//-----------------------------------------------------------------------------

#ifndef __RUELIST_H
#define __RUELIST_H

#include <deque>
#include <vector>
#include "ItemData.h"
#include "StringX.h"
#include "PWScore.h"
#include "../os/UUID.h"
//-----------------------------------------------------------------------------

/*
* CRUEList is a class that contains the recently used entries
*/

struct RUEntryData {
  StringX string;
  CItemData *pci;
};

// private struct: one of these for each owner-draw menu item
struct CRUEItemData {
  unsigned long magicNum;      // magic number identifying me
  int           nImage;        // index of button image in image list

  CRUEItemData() : magicNum(RUEMENUITEMID), nImage(-1) {}
  bool IsRUEID() const {return magicNum == RUEMENUITEMID;}
private:
  // identifies menu owner-draw data as mine
  const unsigned long RUEMENUITEMID = MAKELONG(MAKEWORD('R', 'U'),MAKEWORD('E', 'M'));
};

typedef std::deque<pws_os::CUUID> RUEList;

class CRUEList
{
public:
  // Construction/Destruction/operators
  CRUEList(PWScore &core) : m_core(core), m_maxentries(0) {}
  CRUEList(const CRUEList &other) = delete;
  ~CRUEList() {}

  CRUEList& operator=(const CRUEList& second) = delete;

  // Data retrieval
  size_t GetCount() const {return m_RUEList.size();}
  bool IsEmpty() const {return m_RUEList.empty();}
  size_t GetMax() const {return m_maxentries;}
  bool GetAllMenuItemStrings(std::vector<RUEntryData> &) const;
  bool GetPWEntry(size_t, CItemData &) const; // "logically" const!
  void GetRUEList(UUIDList &RUElist) const;

  // Data setting
  void SetMax(size_t);
  void ClearEntries() {m_RUEList.clear();}
  bool AddRUEntry(const pws_os::CUUID &);
  bool DeleteRUEntry(size_t);
  bool DeleteRUEntry(const pws_os::CUUID &);
  void SetRUEList(const UUIDList &RUElist);

private:
  PWScore &m_core;    // Dboxmain's m_core (which = app.m_core!)
  size_t m_maxentries;
  RUEList m_RUEList;  // Recently Used Entry History List
};
#endif /* __RUELIST_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
