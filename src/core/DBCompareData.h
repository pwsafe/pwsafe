/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "../os/UUID.h"

enum {BOTH = -1 , CURRENT = 0, COMPARE = 1};

// The following structure is needed for compare when record is in
// both databases (indatabase = -1) but there are differences
// Subset used when record is in only one (indatabase = 0 or 1)
// If entries made equal by copying, indatabase set to -1.
struct st_CompareData {
  st_CompareData()
    : uuid0(pws_os::CUUID::NullUUID()), uuid1(pws_os::CUUID::NullUUID()),
    bsDiffs(0), group(_T("")), title(_T("")), user(_T("")),
    id(0), indatabase(0), listindex(0),
    unknflds0(false), unknflds1(false), bIsProtected0(false), 
    bHasAttachment0(false), bHasAttachment1(false)
  {
  }

  st_CompareData(const st_CompareData &that)
    : uuid0(that.uuid0), uuid1(that.uuid1), bsDiffs(that.bsDiffs),
    group(that.group), title(that.title), user(that.user),
    id(that.id), indatabase(that.indatabase), listindex(that.listindex),
    unknflds0(that.unknflds0), unknflds1(that.unknflds1),
    bIsProtected0(that.bIsProtected0),
    bHasAttachment0(that.bHasAttachment0), bHasAttachment1(that.bHasAttachment1)
  {
  }

  st_CompareData &operator=(const st_CompareData &that)
  {
    if (this != &that) {
      uuid0 = that.uuid0;
      uuid1 = that.uuid1;
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
      bHasAttachment0 = that.bHasAttachment0;
      bHasAttachment1 = that.bHasAttachment1;
    }
    return *this;
  }

  void Empty() {
    uuid0 = uuid1 = pws_os::CUUID::NullUUID();
    bsDiffs = 0;
    group = title = user = _T("");
    id = indatabase = listindex =0;
    unknflds0 = unknflds1 = bIsProtected0 = bHasAttachment0 = bHasAttachment1 = false;
  }

  operator int() {return id;}
    
  pws_os::CUUID uuid0;  // original DB
  pws_os::CUUID uuid1;  // comparison DB
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
  bool bHasAttachment0, bHasAttachment1;
};

// Vector of entries passed from DboxMain::Compare to CompareResultsDlg
// Used for "Only in Original DB", "Only in Comparison DB" and
// in "Both with Differences"
typedef std::vector<st_CompareData> CompareData;

#endif /* __DBCOMPAREDATA_H */
