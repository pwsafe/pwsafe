/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// RUEList.h
//-----------------------------------------------------------------------------

#include <deque>
#include <vector>
#include "core/ItemData.h"
#include "core/StringX.h"
#include "core/PWScore.h"
#include "os/UUID.h"
//-----------------------------------------------------------------------------

/*
* CRUEList is a class that contains the recently used entries
*/

struct RUEntryData {
  StringX string;
  int image;
  CItemData *pci;
};

// identifies menu owner-draw data as mine
const unsigned long RUEMENUITEMID = MAKELONG(MAKEWORD('R', 'U'),MAKEWORD('E', 'M'));

// private struct: one of these for each owner-draw menu item
struct CRUEItemData {
  unsigned long magicNum;      // magic number identifying me
  int           nImage;        // index of button image in image list

  CRUEItemData() {magicNum = RUEMENUITEMID;}
  bool IsRUEID() const {return magicNum == RUEMENUITEMID;}
};

typedef std::deque<pws_os::CUUID> RUEList;
typedef RUEList::iterator RUEListIter;
typedef RUEList::const_iterator RUEListConstIter;

class DboxMain;

class CRUEList
{
public:
  // Construction/Destruction/operators
  CRUEList();
  ~CRUEList() {}

  CRUEList& operator=(const CRUEList& second);

  void SetMainWindow(DboxMain *pDbx)
  {m_pDbx = pDbx;}

  // Data retrieval
  size_t GetCount() const {return m_RUEList.size();}
  size_t GetMax() const {return m_maxentries;}
  bool GetAllMenuItemStrings(std::vector<RUEntryData> &) const;
  bool GetPWEntry(size_t, CItemData &); // NOT const!
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
  DboxMain *m_pDbx;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
