/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "ItemData.h"

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

// Used to verify uniqueness of GTU using std::set
struct st_GroupTitleUser {
  StringX group;
  StringX title;
  StringX user;

  st_GroupTitleUser() {}

  st_GroupTitleUser(const StringX &g, const StringX &t, const StringX &u)
  : group(g), title(t), user(u) {}

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

// Used to verify uniqueness of UUID using std::set
struct st_UUID {
  uuid_array_t uuid;

  st_UUID() {}

  st_UUID(const uuid_array_t &u)
  {memcpy(uuid, u, sizeof(uuid_array_t));}

  st_UUID &operator=(const st_UUID &that) {
    if (this != &that) {
      memcpy(uuid, that.uuid, sizeof(uuid_array_t));
    }
    return *this;
  }

  friend bool operator< (const st_UUID &uuid1,
                         const st_UUID &uuid2)
  {
    return (memcmp(uuid1.uuid, uuid2.uuid, sizeof(uuid_array_t)) < 0);
  }
};

typedef std::map<CUUIDGen, CItemData, std::less<CUUIDGen> > ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;
typedef std::pair<CUUIDGen, CItemData> ItemList_Pair;

typedef std::vector<CItemData> OrderedItemList;

typedef std::multimap<CUUIDGen, CUUIDGen, std::less<CUUIDGen> > ItemMMap;
typedef ItemMMap::iterator ItemMMapIter;
typedef ItemMMap::const_iterator ItemMMapConstIter;
typedef std::pair<CUUIDGen, CUUIDGen> ItemMMap_Pair;

typedef std::map<CUUIDGen, CUUIDGen, std::less<CUUIDGen> > ItemMap;
typedef ItemMap::iterator ItemMapIter;
typedef ItemMap::const_iterator ItemMapConstIter;
typedef std::pair<CUUIDGen, CUUIDGen> ItemMap_Pair;

typedef std::map<CUUIDGen, st_SaveTypePW, std::less<CUUIDGen> > SaveTypePWMap;
typedef std::pair<CUUIDGen, st_SaveTypePW> SaveTypePWMap_Pair;

typedef std::map<CUUIDGen, StringX, std::less<CUUIDGen> > SavePWHistoryMap;

typedef std::set<st_GroupTitleUser> GTUSet;
typedef std::pair< std::set<st_GroupTitleUser>::iterator, bool > GTUSetPair;

typedef std::set<st_UUID> UUIDSet;
typedef std::pair< std::set<st_UUID>::iterator, bool > UUIDSetPair;

typedef std::list<st_UUID> UUIDList;
typedef UUIDList::iterator UUIDListIter;
typedef UUIDList::reverse_iterator UUIDListRIter;

#endif /* __COREDEFS_H */
