/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSFILTERS_H
#define __PWSFILTERS_H

/**
 * \file PWSFilters.h
 * types used for defining and working with filters of pwsafe
 * entries. Note that the structures are st_*, and they currently
 * have a wrapper class PWSFilters.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <time.h> // for time_t
#include "StringX.h"
#include "PWSfile.h"
#include "Match.h"
#include "ItemData.h"
#include "Proxy.h"

enum FilterType {DFTYPE_INVALID = 0,
                 DFTYPE_MAIN,
                 DFTYPE_PWHISTORY, 
                 DFTYPE_PWPOLICY};

// All the fields that we can use for filtering entries:

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

// This defines a single row of a filter
// Filters consist of one or more rows. The relation with the previous
// row is defined by LogicConnect ltype.
struct st_FilterRow {
  bool bFilterActive;
  bool bFilterComplete;
  // Item in ComboBox selected is found via FieldType
  FieldType ftype;
  // and matchtype
  PWSMatch::MatchType mtype;
  // User selected rule
  PWSMatch::MatchRule rule;
  // Following should really be a union or subclassed, since
  // only one is used, and which is defined uniquely by the ftype.
  // if filter type is an integer
  int fnum1, fnum2;
  // if filter type is a date
  time_t fdate1, fdate2;
  // if filter type is a string
  StringX fstring;
  bool fcase; // case sensitive?
  // if filter type is a entrytype
  CItemData::EntryType etype;
  
  // Logical connection with previous filter (if any). Brackets unsupported
  LogicConnect ltype;

  st_FilterRow()
    : bFilterActive(true), bFilterComplete(false),
    ftype(FT_INVALID), mtype(PWSMatch::MT_INVALID), rule(PWSMatch::MR_INVALID),
    fnum1(0), fnum2(0),
    fdate1(0), fdate2(0),
    fstring(_T("")), fcase(false), etype(CItemData::ET_INVALID),
    ltype(LC_INVALID)
  {}

  st_FilterRow(const st_FilterRow &that)
    : bFilterActive(that.bFilterActive), bFilterComplete(that.bFilterComplete),
    ftype(that.ftype), mtype(that.mtype), rule(that.rule),
    fnum1(that.fnum1), fnum2(that.fnum2),
    fdate1(that.fdate1), fdate2(that.fdate2), 
    fstring(that.fstring), fcase(that.fcase), etype(that.etype),
    ltype(that.ltype)
  {}

  st_FilterRow &operator=(const st_FilterRow &that)
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
    fcase = false;
    etype = CItemData::ET_INVALID;
    ltype = LC_INVALID;
  }
  void SetFilterComplete() {bFilterComplete = true;}
  void ClearFilterComplete() {bFilterComplete = false;}
};

// The following structure is needed for entry filtering
// Basically a named vector of filter rows, along with
// some accounting information

typedef std::vector<st_FilterRow> vFilterRows;
typedef std::vector<int> vfiltergroup;
typedef std::vector<vfiltergroup> vfiltergroups;

struct st_filters {
  // Filter name
  stringT fname;
  // Counters
  int num_Mactive;
  int num_Hactive;
  int num_Pactive;
  // Main filters
  vFilterRows vMfldata;
  // PW history filters
  vFilterRows vHfldata;
  // PW Policy filters
  vFilterRows vPfldata;

  st_filters()
  : fname(_T("")), num_Mactive(0), num_Hactive(0), num_Pactive(0)
  {}

  st_filters(const st_filters &that)
    : fname(that.fname),
    num_Mactive(that.num_Mactive), 
    num_Hactive(that.num_Hactive), num_Pactive(that.num_Pactive),
    vMfldata(that.vMfldata), vHfldata(that.vHfldata),
    vPfldata(that.vPfldata)
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
    num_Mactive = num_Hactive = num_Pactive = 0;
    vMfldata.clear();
    vHfldata.clear();
    vPfldata.clear();
  }
};

enum FilterPool {FPOOL_DATABASE = 1, FPOOL_AUTOLOAD, FPOOL_IMPORTED, FPOOL_SESSION,
                 FPOOL_LAST};

struct st_Filterkey {
  FilterPool fpool;
  stringT cs_filtername;
};

// Following is for map<> compare function
struct ltfk {
  bool operator()(const st_Filterkey &fk1, const st_Filterkey &fk2) const
  {
    if (fk1.fpool != fk2.fpool)
      return (int)fk1.fpool < (int)fk2.fpool;

    return fk1.cs_filtername.compare(fk2.cs_filtername) < 0;
  }
};

class PWSFilters : public std::map<st_Filterkey, st_filters, ltfk> {
 public:
  typedef std::pair<st_Filterkey, st_filters> Pair;
  
  std::string GetFilterXMLHeader(const StringX &currentfile,
                                 const PWSfile::HeaderRecord &hdr);

  int WriteFilterXMLFile(const StringX &filename, const PWSfile::HeaderRecord hdr,
                         const StringX &currentfile);
  int WriteFilterXMLFile(std::ostream &os, PWSfile::HeaderRecord hdr,
                         const StringX &currentfile, const bool bWithFormatting = false);
  int ImportFilterXMLFile(const FilterPool fpool,
                          const StringX &strXMLData,
                          const stringT &strXMLFileName,
                          const stringT &strXSDFileName, stringT &strErrors,
                          Asker *pAsker);

  static stringT GetFilterDescription(const st_FilterRow &st_fldata);
};

#endif  // __PWSFILTERS_H
