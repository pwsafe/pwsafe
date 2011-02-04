/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __DBCOMPAREDATA_H
#define __DBCOMPAREDATA_H

/// DBCompareData.h
//-----------------------------------------------------------------------------

#include "core/ItemData.h"
#include "core/StringX.h"
#include "core/UUIDGen.h"

enum {BOTH = -1 , CURRENT = 0, COMPARE = 1};

// The following structure is needed for compare when record is in
// both databases (indatabase = -1) but there are differences
// Subset used when record is in only one (indatabase = 0 or 1)
// If entries made equal by copying, indatabase set to -1.
struct st_CompareData {
  st_CompareData()
    : bsDiffs(0), group(L""), title(L""), user(L""),
    id(0), indatabase(0), listindex(0),
    unknflds0(false), unknflds1(false), bIsProtected0(false)
  {
    memset(uuid0, 0, sizeof(uuid0));
    memset(uuid1, 0, sizeof(uuid1));
  }

  st_CompareData(const st_CompareData &that)
    : bsDiffs(that.bsDiffs), group(that.group), title(that.title), user(that.user),
    id(that.id), indatabase(that.indatabase), listindex(that.listindex),
    unknflds0(that.unknflds0), unknflds1(that.unknflds1),
    bIsProtected0(that.bIsProtected0)
  {
    memcpy(uuid0, that.uuid0, sizeof(uuid0));
    memcpy(uuid1, that.uuid1, sizeof(uuid1));
  }

  st_CompareData &operator=(const st_CompareData &that)
  {
    if (this != &that) {
      memcpy(uuid0, that.uuid0, sizeof(uuid0));
      memcpy(uuid1, that.uuid1, sizeof(uuid1));
      bsDiffs = that.bsDiffs;
      group = that.group;
      title = that.title;
      user = that.user;
      id = that.id;
      indatabase = that.indatabase;
      listindex = that.listindex;
      unknflds0 = that.unknflds0;
      unknflds1 = that.unknflds1;
      bIsProtected0 = that.bIsProtected0;
    }
    return *this;
  }

  uuid_array_t uuid0;  // original DB
  uuid_array_t uuid1;  // comparison DB
  CItemData::FieldBits bsDiffs;  // list of items compared
  StringX group;
  StringX title;
  StringX user;
  int id;  // # in the appropriate list: "Only in Original", "Only in Comparison" or in "Both with Differences"
  int indatabase;    // see enum above
  int listindex;     // Used in the UI Compare results dialog
  bool unknflds0;    // original DB
  bool unknflds1;    // comparison DB
  bool bIsProtected0;
};

struct equal_id {
  equal_id(int const& id) : m_id(id) {}

  bool operator()(st_CompareData const& rdata) const
  { return (rdata.id == m_id); }

  int m_id;
};

// Vector of entries passed from DboxMain::Compare to CompareResultsDlg
// Used for "Only in Original DB", "Only in Comparison DB" and
// in "Both with Differences"
typedef std::vector<st_CompareData> CompareData;

#endif /* __DBCOMPAREDATA_H */
