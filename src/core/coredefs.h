/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __COREDEFS_H
#define __COREDEFS_H

#include <map>
#include <vector>
#include <set>
#include <list>

#include "os/UUID.h"
#include "ItemData.h"
#include "ItemAtt.h"

struct st_SaveTypePW {
  CItemData::EntryType et;
  StringX sxpw;

  st_SaveTypePW()
  : et(CItemData::ET_INVALID), sxpw(_T(""))
  {}

  st_SaveTypePW(const st_SaveTypePW &that)
    : et(that.et), sxpw(that.sxpw)
  {}

  st_SaveTypePW &operator=(const st_SaveTypePW &that)
  {
    if (this != &that) {
      et = that.et;
      sxpw = that.sxpw;
    }
    return *this;
  }  
};

// Used to verify uniqueness of GTU using std::set and
// retrieving all entries using a given Named Password Policy
struct st_GroupTitleUser {
  StringX group;
  StringX title;
  StringX user;

  st_GroupTitleUser() {}

  st_GroupTitleUser(const StringX &g, const StringX &t, const StringX &u)
  : group(g), title(t), user(u) {}

  st_GroupTitleUser(const st_GroupTitleUser &other)
  : group(other.group), title(other.title), user(other.user) {}
  
  st_GroupTitleUser &operator=(const st_GroupTitleUser &that) {
    if (this != &that) {
      group = that.group; title = that.title; user = that.user;
    }
    return *this;
  }

  friend bool operator< (const st_GroupTitleUser &gtu1,
                         const st_GroupTitleUser &gtu2)
  {
    if (gtu1.group != gtu2.group)
      return gtu1.group.compare(gtu2.group) < 0;
    else if (gtu1.title != gtu2.title)
      return gtu1.title.compare(gtu2.title) < 0;
    else
      return gtu1.user.compare(gtu2.user) < 0;
  }
};

struct st_PWH_status {
  StringX pwh;
  CItemData::EntryStatus es;
};

typedef std::map<pws_os::CUUID, CItemData, std::less<pws_os::CUUID> > ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;
typedef std::pair<pws_os::CUUID, CItemData> ItemList_Pair;

typedef std::map<pws_os::CUUID, CItemAtt, std::less<pws_os::CUUID> > AttList;
typedef AttList::iterator AttListIter;
typedef AttList::const_iterator AttListConstIter;
typedef std::pair<pws_os::CUUID, CItemAtt> AttList_Pair;

typedef std::vector<CItemData> OrderedItemList;

typedef std::multimap<pws_os::CUUID, pws_os::CUUID, std::less<pws_os::CUUID> > ItemMMap;
typedef ItemMMap::iterator ItemMMapIter;
typedef ItemMMap::const_iterator ItemMMapConstIter;
typedef std::pair<pws_os::CUUID, pws_os::CUUID> ItemMMap_Pair;

typedef std::map<pws_os::CUUID, st_SaveTypePW, std::less<pws_os::CUUID> > SaveTypePWMap;
typedef std::pair<pws_os::CUUID, st_SaveTypePW> SaveTypePWMap_Pair;

typedef std::map<pws_os::CUUID, st_PWH_status, std::less<pws_os::CUUID> > SavePWHistoryMap;

typedef std::set<st_GroupTitleUser> GTUSet;
typedef std::pair<GTUSet::iterator, bool > GTUSetPair;

typedef std::set<pws_os::CUUID> UUIDSet;
typedef std::pair<UUIDSet::iterator, bool > UUIDSetPair;

typedef std::list<pws_os::CUUID> UUIDList;
typedef UUIDList::iterator UUIDListIter;
typedef UUIDList::reverse_iterator UUIDListRIter;

typedef std::map<StringX, PWPolicy> PSWDPolicyMap;
typedef std::map<StringX, PWPolicy>::iterator PSWDPolicyMapIter;
typedef std::map<StringX, PWPolicy>::const_iterator PSWDPolicyMapCIter;
typedef std::pair<StringX, PWPolicy> PSWDPolicyMapPair;

typedef std::map<int32, pws_os::CUUID> KBShortcutMap;
typedef KBShortcutMap::iterator KBShortcutMapIter;
typedef KBShortcutMap::const_iterator KBShortcutMapConstIter;
typedef std::pair<int32, pws_os::CUUID> KBShortcutMapPair;

struct PopulatePWPVector {
  PopulatePWPVector(std::vector<StringX> *pvPWPolicies) :
    m_pvPWPolicies(pvPWPolicies) {}

  // operator for OrderedItemList
  void operator()(const CItemData &item) {
    StringX sx_pwpname = item.GetPolicyName();
    if (!sx_pwpname.empty())
      m_pvPWPolicies->push_back(sx_pwpname);
  }

private:
  PopulatePWPVector& operator=(const PopulatePWPVector&); // Do not implement
  std::vector<StringX> *m_pvPWPolicies;
};

#endif /* __COREDEFS_H */
