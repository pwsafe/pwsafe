/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

struct st_PSWDPolicy {
  PWPolicy pwp;
  StringX symbols;
  size_t usecount;

  st_PSWDPolicy()
  : symbols(_T("")), usecount(0)
  {
    pwp.Empty();
  }

  st_PSWDPolicy(const PWPolicy &in_pwp, const StringX &in_symbols, size_t in_usecount)
  : pwp(in_pwp), symbols(in_symbols), usecount(in_usecount)
  {}

  st_PSWDPolicy(const st_PSWDPolicy &that)
    : pwp(that.pwp), symbols(that.symbols), usecount(that.usecount)
  {}

  st_PSWDPolicy &operator=(const st_PSWDPolicy &that)
  {
    if (this != &that) {
      pwp = that.pwp;
      symbols = that.symbols;
      usecount = that.usecount;
    }
    return *this;
  }

  bool operator==(const st_PSWDPolicy &that) const
  {
    // Need to check all elements are the same for != operator to be correct
    if (this != &that) {
      if (pwp == that.pwp &&
          symbols == that.symbols)
        return true;
    }
    return (this == &that) ? true : false;
  }

  bool operator!=(const st_PSWDPolicy &that) const
  { return !(*this == that);}

  void Empty()
  { 
    pwp.Empty();
    symbols = _T("");
    usecount = 0;
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


typedef std::map<pws_os::CUUID, CItemData, std::less<pws_os::CUUID> > ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;
typedef std::pair<pws_os::CUUID, CItemData> ItemList_Pair;

typedef std::vector<CItemData> OrderedItemList;

typedef std::multimap<pws_os::CUUID, pws_os::CUUID, std::less<pws_os::CUUID> > ItemMMap;
typedef ItemMMap::iterator ItemMMapIter;
typedef ItemMMap::const_iterator ItemMMapConstIter;
typedef std::pair<pws_os::CUUID, pws_os::CUUID> ItemMMap_Pair;

typedef std::map<pws_os::CUUID, pws_os::CUUID, std::less<pws_os::CUUID> > ItemMap;
typedef ItemMap::iterator ItemMapIter;
typedef ItemMap::const_iterator ItemMapConstIter;
typedef std::pair<pws_os::CUUID, pws_os::CUUID> ItemMap_Pair;

typedef std::map<pws_os::CUUID, st_SaveTypePW, std::less<pws_os::CUUID> > SaveTypePWMap;
typedef std::pair<pws_os::CUUID, st_SaveTypePW> SaveTypePWMap_Pair;

typedef std::map<pws_os::CUUID, StringX, std::less<pws_os::CUUID> > SavePWHistoryMap;

typedef std::set<st_GroupTitleUser> GTUSet;
typedef std::pair<GTUSet::iterator, bool > GTUSetPair;

typedef std::set<pws_os::CUUID> UUIDSet;
typedef std::pair<UUIDSet::iterator, bool > UUIDSetPair;

typedef std::list<pws_os::CUUID> UUIDList;
typedef UUIDList::iterator UUIDListIter;
typedef UUIDList::reverse_iterator UUIDListRIter;

typedef std::map<StringX, st_PSWDPolicy> PSWDPolicyMap;
typedef std::map<StringX, st_PSWDPolicy>::iterator PSWDPolicyMapIter;
typedef std::map<StringX, st_PSWDPolicy>::const_iterator PSWDPolicyMapCIter;
typedef std::pair<StringX, st_PSWDPolicy> PSWDPolicyMapPair;

#endif /* __COREDEFS_H */
