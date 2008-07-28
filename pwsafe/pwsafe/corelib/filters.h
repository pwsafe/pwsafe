/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#if !defined FILTERS_H
#define FILTERS_H

#include "Match.h"
#include "itemdata.h"
#include "mystring.h"
#include <time.h> // for time_t
#include <string>
#include <vector>
#include <map>


enum FieldType {
  // PWS Test fields
  FT_GROUPTITLE    = CItemData::NAME, // reusing depreciated NAME for Group.Title combination
  FT_GROUP         = CItemData::GROUP,
  FT_TITLE         = CItemData::TITLE,
  FT_USER          = CItemData::USER,
  FT_NOTES         = CItemData::NOTES,
  FT_PASSWORD      = CItemData::PASSWORD,
  FT_CTIME         = CItemData::CTIME,
  FT_PMTIME        = CItemData::PMTIME,
  FT_ATIME         = CItemData::ATIME,
  FT_XTIME         = CItemData::XTIME,
  FT_RMTIME        = CItemData::RMTIME,
  FT_URL           = CItemData::URL,
  FT_AUTOTYPE      = CItemData::AUTOTYPE,
  FT_PWHIST        = CItemData::PWHIST,
  FT_POLICY        = CItemData::POLICY,
  FT_XTIME_INT     = CItemData::XTIME_INT,
  FT_END           = CItemData::END,

  // new fields purely for filters
  FT_ENTRYTYPE     = 0x100,
  FT_UNKNOWNFIELDS = 0x101,

  // Password History Test fields
  HT_PRESENT       = 0x200,
  HT_ACTIVE,
  HT_NUM,
  HT_MAX,
  HT_CHANGEDATE,
  HT_PASSWORDS,
  HT_END,

  // Password Policy Test fields
  PT_PRESENT       = 0x300,
  PT_LENGTH,
  PT_LOWERCASE,
  PT_UPPERCASE,
  PT_DIGITS,
  PT_SYMBOLS,
  PT_EASYVISION,
  PT_PRONOUNCEABLE,
  PT_HEXADECIMAL,
  PT_END,

  FT_INVALID       = 0xffff
};

// Logical connection with previous filter - brackets not allowed
enum LogicConnect {LC_INVALID = 0, LC_AND = 1, LC_OR = 2};

// Filter data
struct st_FilterData {
  bool bFilterActive;
  bool bFilterComplete;
  // Item in ComboBox selected is found via FieldType
  FieldType ftype;
  // and matchtype
  PWSMatch::MatchType mtype;
  // User selected rule
  PWSMatch::MatchRule rule;
  // if filter type is an integer
  int fnum1, fnum2;
  // if filter type is a date
  time_t fdate1, fdate2;
  // if filter type is a string
  CMyString fstring;
  int fcase;
  // if filter type is a entrytype
  CItemData::EntryType etype;
  // Logical connection with previous filter (if present) - brackets not allowed
  LogicConnect ltype;

  st_FilterData()
    : bFilterActive(true), bFilterComplete(false),
    mtype(PWSMatch::MT_INVALID), ftype(FT_INVALID), rule(PWSMatch::MR_INVALID),
    fnum1(0), fnum2(0),
    fdate1(0), fdate2(0),
    fstring(_T("")), fcase(BST_UNCHECKED), etype(CItemData::ET_INVALID),
    ltype(LC_INVALID)
  {}

  st_FilterData(const st_FilterData &that)
    : bFilterActive(that.bFilterActive), bFilterComplete(that.bFilterComplete),
    ftype(that.ftype), mtype(that.mtype), rule(that.rule),
    fnum1(that.fnum1), fnum2(that.fnum2),
    fdate1(that.fdate1), fdate2(that.fdate2), 
    fstring(that.fstring), fcase(that.fcase), etype(that.etype),
    ltype(that.ltype)
  {}

  st_FilterData &operator=(const st_FilterData &that)
  {
    if (this != &that) {
      bFilterActive = that.bFilterActive;
      bFilterComplete = that.bFilterComplete;
      ftype = that.ftype;
      mtype = that.mtype;
      rule = that.rule;
      fnum1 = that.fnum1;
      fnum2 = that.fnum2;
      fdate1 = that.fdate1;
      fdate2 = that.fdate2;
      fstring = that.fstring;
      fcase = that.fcase;
      etype = that.etype;
      ltype = that.ltype;
    }
    return *this;
  }

  void Empty()
  {
    bFilterActive = true;
    bFilterComplete = false;
    ftype = FT_INVALID;
    mtype = PWSMatch::MT_INVALID;
    rule = PWSMatch::MR_INVALID;
    fnum1 = fnum2 = 0;
    fdate1 = fdate2 = (time_t)0;
    fstring = _T("");
    fcase = BST_UNCHECKED;
    etype = CItemData::ET_INVALID;
    ltype = LC_INVALID;
  }
};

// The following structure is needed for entry filtering

typedef std::vector<st_FilterData> vfilterdata;
typedef std::vector<int> vfiltergroup;
typedef std::vector<vfiltergroup> vfiltergroups;

struct st_filters {
  // Filter name
  CString fname;
  // Counters
  int num_Mactive;
  int num_Hactive;
  int num_Pactive;
  // Main filters
  vfilterdata vMfldata;
  // PW history filters
  vfilterdata vHfldata;
  // PW Policy filters
  vfilterdata vPfldata;

  st_filters()
    : fname(_T("")), num_Mactive(0), num_Hactive(0), num_Pactive(0)
  {}

  st_filters(const st_filters &that)
    : fname(that.fname),
    num_Mactive(that.num_Mactive), 
    num_Hactive(that.num_Hactive), num_Pactive(that.num_Pactive),
    vMfldata(that.vMfldata),
    vHfldata(that.vHfldata), vPfldata(that.vPfldata)
  {}

  st_filters &operator=(const st_filters &that)
  {
    if (this != &that) {
      fname = that.fname;
      num_Mactive = that.num_Mactive;
      num_Hactive = that.num_Hactive;
      num_Pactive = that.num_Pactive;
      vMfldata = that.vMfldata;
      vHfldata = that.vHfldata;
      vPfldata = that.vPfldata;
    }
    return *this;
  }

  void Empty()
  {
    fname = _T("");
    num_Mactive = 0;
    num_Hactive = 0;
    num_Pactive = 0;
    vMfldata.clear();
    vHfldata.clear();
    vPfldata.clear();
  }
};

typedef std::map<CString, st_filters> MapFilters;
typedef MapFilters::iterator MapFilters_Iter;
typedef MapFilters::const_iterator MapFilters_cIter;
typedef std::pair<CString, st_filters> MapFilters_Pair;

#endif // FILTERS_H
