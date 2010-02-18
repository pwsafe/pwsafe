/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
  friend bool operator< (const st_GroupTitleUser &elem1,
                         const st_GroupTitleUser &elem2)
  {
    if (elem1.group != elem2.group)
      return elem1.group.compare(elem2.group) < 0;
    else if (elem1.title != elem2.title)
      return elem1.title.compare(elem2.title) < 0;
    else
    return elem1.user.compare(elem2.user) < 0;
  }
};

typedef std::map<CUUIDGen, CItemData, CUUIDGen::ltuuid> ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;
typedef std::pair<CUUIDGen, CItemData> ItemList_Pair;

typedef std::vector<CItemData> OrderedItemList;

typedef std::multimap<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> ItemMMap;
typedef ItemMMap::iterator ItemMMapIter;
typedef ItemMMap::const_iterator ItemMMapConstIter;
typedef std::pair<CUUIDGen, CUUIDGen> ItemMMap_Pair;

typedef std::map<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> ItemMap;
typedef ItemMap::iterator ItemMapIter;
typedef ItemMap::const_iterator ItemMapConstIter;
typedef std::pair<CUUIDGen, CUUIDGen> ItemMap_Pair;

typedef std::map<CUUIDGen, st_SaveTypePW, CUUIDGen::ltuuid> SaveTypePWMap;
typedef std::pair<CUUIDGen, st_SaveTypePW> SaveTypePWMap_Pair;

typedef std::map<CUUIDGen, StringX, CUUIDGen::ltuuid> SavePWHistoryMap;

typedef std::set<st_GroupTitleUser> GTUSet;
typedef std::pair< std::set<st_GroupTitleUser>::iterator, bool > GTUSetPair;

#endif
