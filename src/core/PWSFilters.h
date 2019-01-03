/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "StringX.h"
#include "StringXStream.h"
#include "Match.h"
#include "ItemData.h"
#include "ItemAtt.h"
#include "Proxy.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <time.h> // for time_t

enum FilterType {DFTYPE_INVALID = 0,
                 DFTYPE_MAIN,
                 DFTYPE_PWHISTORY, 
                 DFTYPE_PWPOLICY,
                 DFTYPE_ATTACHMENT
};

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
  FT_RUNCMD        = CItemData::RUNCMD,
  FT_DCA           = CItemData::DCA,
  FT_SHIFTDCA      = CItemData::SHIFTDCA,
  FT_EMAIL         = CItemData::EMAIL,
  FT_PROTECTED     = CItemData::PROTECTED,
  FT_SYMBOLS       = CItemData::SYMBOLS,
  FT_POLICYNAME    = CItemData::POLICYNAME,
  FT_KBSHORTCUT    = CItemData::KBSHORTCUT,
  FT_END           = CItemData::END,

  // Internal fields purely for filters
  FT_ENTRYSIZE     = CItemData::ENTRYSIZE,     // 0x100
  FT_ENTRYTYPE     = CItemData::ENTRYTYPE,     // 0x101,
  FT_ENTRYSTATUS   = CItemData::ENTRYSTATUS,   // 0x102,
  FT_UNKNOWNFIELDS = CItemData::UNKNOWNFIELDS, // 0x103,
  FT_PASSWORDLEN   = CItemData::PASSWORDLEN,   // 0x104

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

  // Attachment Test fields
  AT_PRESENT       = CItemAtt::START_ATT,
  AT_TITLE         = CItemAtt::ATTTITLE,
  AT_CTIME         = CItemAtt::ATTCTIME,
  AT_MEDIATYPE     = CItemAtt::MEDIATYPE,
  AT_FILENAME      = CItemAtt::FILENAME,
  AT_FILEPATH      = CItemAtt::FILEPATH,
  AT_FILECTIME     = CItemAtt::FILECTIME,
  AT_FILEMTIME     = CItemAtt::FILEMTIME,
  AT_FILEATIME     = CItemAtt::FILEATIME,
  AT_END           = CItemAtt::LAST_SEARCHABLE,
  FT_ATTACHMENT    = CItemAtt::LAST_SEARCHABLE + 1, // not searchable

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

  // if filter type is a 'relative date', integer or size
  int fnum1, fnum2;
  // if filter type is an 'absolute date'
  time_t fdate1, fdate2;
  // fdatetype = 0 for absolute date (using fdate1 & 2) "the default" or
  // fdatetype = 1 for relative date (using fnum1 & 2)
  // if relative, values can be between -3650 & +3650 (i.e. 10 years either side of now)
  int fdatetype;
  // if filter type is a string
  StringX fstring;
  bool fcase; // case sensitive?
  // if filter type is Double-Click Action or Shift-Double-Click Action
  short fdca;
  // if filter type is a entrytype
  CItemData::EntryType etype;
  // if filter type is a entrystatus
  CItemData::EntryStatus estatus;
  // if filter type is a entrysize
  int funit;  // 0 = Bytes, 1 = KBytes, 2 = MBytes
  
  // Logical connection with previous filter (if any). Brackets unsupported
  LogicConnect ltype;

  st_FilterRow()
    : bFilterActive(true), bFilterComplete(false),
    ftype(FT_INVALID), mtype(PWSMatch::MT_INVALID), rule(PWSMatch::MR_INVALID),
    fnum1(0), fnum2(0),
    fdate1(time_t(0)), fdate2(time_t(0)), fdatetype(0),
    fstring(_T("")), fcase(false), fdca(-2),
    etype(CItemData::ET_INVALID),
    estatus(CItemData::ES_INVALID), funit(-1),
    ltype(LC_INVALID)
  {}

  st_FilterRow(const st_FilterRow &that)
    : bFilterActive(that.bFilterActive), bFilterComplete(that.bFilterComplete),
    ftype(that.ftype), mtype(that.mtype), rule(that.rule),
    fnum1(that.fnum1), fnum2(that.fnum2),
    fdate1(that.fdate1), fdate2(that.fdate2), fdatetype(that.fdatetype),
    fstring(that.fstring), fcase(that.fcase), fdca(that.fdca),
    etype(that.etype), estatus(that.estatus), funit(that.funit),
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
      fdatetype = that.fdatetype;
      fstring = that.fstring;
      fcase = that.fcase;
      fdca = that.fdca;
      etype = that.etype;
      estatus = that.estatus;
      funit = that.funit;
      ltype = that.ltype;
    }
    return *this;
  }

  bool operator==(const st_FilterRow &that) const
  {
    if (this != &that) {
      if (bFilterActive != that.bFilterActive ||
          bFilterComplete != that.bFilterComplete ||
          ftype != that.ftype ||
          mtype != that.mtype ||
          rule != that.rule ||
          fnum1 != that.fnum1 ||
          fnum2 != that.fnum2 ||
          fdate1 != that.fdate1 ||
          fdate2 != that.fdate2 ||
          fdatetype != that.fdatetype ||
          fstring != that.fstring ||
          fcase != that.fcase ||
          fdca != that.fdca ||
          etype != that.etype ||
          estatus != that.estatus ||
          funit != that.funit ||
          ltype != that.ltype)
      return false;
    }
    return true;
  }

  bool operator!=(const st_FilterRow &that) const
  {return !(*this == that);}

  void Empty()
  {
    bFilterActive = true;
    bFilterComplete = false;
    ftype = FT_INVALID;
    mtype = PWSMatch::MT_INVALID;
    rule = PWSMatch::MR_INVALID;
    fnum1 = fnum2 = 0;
    fdate1 = fdate2 = time_t(0);
    fdatetype = 0;
    fstring = _T("");
    fcase = false;
    fdca = -2;
    etype = CItemData::ET_INVALID;
    estatus = CItemData::ES_INVALID;
    funit = -1;
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
  int num_Mactive, num_Hactive, num_Pactive, num_Aactive;
  // Main filters
  vFilterRows vMfldata;
  // PW history filters
  vFilterRows vHfldata;
  // PW Policy filters
  vFilterRows vPfldata;
  // Attachment filters
  vFilterRows vAfldata;

  st_filters()
    : fname(_T("")), num_Mactive(0), num_Hactive(0), num_Pactive(0), num_Aactive(0)
  {}

  st_filters(const st_filters &that)
    : fname(that.fname),
    num_Mactive(that.num_Mactive), 
    num_Hactive(that.num_Hactive), num_Pactive(that.num_Pactive), num_Aactive(that.num_Aactive),
    vMfldata(that.vMfldata), vHfldata(that.vHfldata),
    vPfldata(that.vPfldata), vAfldata(that.vAfldata)
  {}

  st_filters &operator=(const st_filters &that)
  {
    if (this != &that) {
      fname = that.fname;
      num_Mactive = that.num_Mactive;
      num_Hactive = that.num_Hactive;
      num_Pactive = that.num_Pactive;
      num_Aactive = that.num_Aactive;
      vMfldata = that.vMfldata;
      vHfldata = that.vHfldata;
      vPfldata = that.vPfldata;
      vAfldata = that.vAfldata;
    }
    return *this;
  }

  bool operator==(const st_filters &that) const
  {
    if (this != &that) {
      if (fname != that.fname ||
          num_Mactive != that.num_Mactive ||
          num_Hactive != that.num_Hactive ||
          num_Pactive != that.num_Pactive ||
          num_Aactive != that.num_Aactive ||
          vMfldata != that.vMfldata ||
          vHfldata != that.vHfldata ||
          vPfldata != that.vPfldata ||
          vAfldata != that.vAfldata)
      return false;
    }
    return true;
  }

  bool operator!=(const st_filters &that) const
  {return !(*this == that);}

  bool IsActive() const {return (num_Mactive + num_Hactive +
                                 num_Pactive + num_Aactive) != 0;}

  void Empty()
  {
    fname = _T("");
    num_Mactive = num_Hactive = num_Pactive = num_Aactive = 0;
    vMfldata.clear();
    vHfldata.clear();
    vPfldata.clear();
    vAfldata.clear();
  }
};

enum FilterPool {FPOOL_DATABASE = 1, FPOOL_AUTOLOAD, FPOOL_IMPORTED, FPOOL_SESSION,
                 FPOOL_LAST};

struct st_Filterkey {
  FilterPool fpool;
  stringT cs_filtername;

  bool operator==(const st_Filterkey& that) const
  {
    if (this != &that) {
      if (fpool != that.fpool &&
          cs_filtername != that.cs_filtername)
        return false;
    }
    return true;
  }

  bool operator!=(const st_Filterkey& that) const
  {
    return !(*this == that);
  }
};

// Following is for map<> compare function
struct ltfk {
  bool operator()(const st_Filterkey &fk1, const st_Filterkey &fk2) const
  {
    if (fk1.fpool != fk2.fpool)
      return static_cast<int>(fk1.fpool) < static_cast<int>(fk2.fpool);

    return fk1.cs_filtername.compare(fk2.cs_filtername) < 0;
  }
};

struct PWSfileHeader;
class PWScore;

class PWSFilters : public std::map<st_Filterkey, st_filters, ltfk> {
 public:
  typedef std::pair<st_Filterkey, st_filters> Pair;
  
  int WriteFilterXMLFile(const StringX &filename, const PWSfileHeader &hdr,
                         const StringX &currentfile);
  int WriteFilterXMLFile(coStringXStream &os, const PWSfileHeader &hdr,
                         const StringX &currentfile, const bool bWithFormatting = false);
  int ImportFilterXMLFile(const FilterPool fpool,
                          const StringX &strXMLData,
                          const stringT &strXMLFileName,
                          const stringT &strXSDFileName, stringT &strErrors,
                          Asker *pAsker);

  static stringT GetFilterDescription(const st_FilterRow &st_fldata);
 private:
  std::string GetFilterXMLHeader(const StringX &currentfile,
                                 const PWSfileHeader &hdr);
};

class PWSFilterManager {
 public:
  PWSFilterManager();
  void CreateGroups();
  bool PassesFiltering(const CItemData &ci, const PWScore &core);
  bool PassesEmptyGroupFiltering(const StringX &sxGroup);
  void SetFindFilter(const bool &bFilter) { m_bFindFilterActive = bFilter; }
  void SetFilterFindEntries(std::vector<pws_os::CUUID> *pvFoundUUIDs);

  // predefined filters accessors, use by assigning to m_currentfilter
  const st_filters &GetExpireFilter() const {return m_expirefilter;}
  const st_filters &GetUnsavedFilter() const {return m_unsavedfilter;}
  const st_filters &GetFoundFilter() const { return m_lastfoundfilter; }

  st_filters m_currentfilter;
  size_t GetFindFilterSize() { return m_vFltrFoundUUIDs.size(); }
  
 private:
   bool PassesPWHFiltering(const CItemData *pci) const;
   bool PassesPWPFiltering(const CItemData *pci) const;
   bool PassesAttFiltering(const CItemData *pci, const PWScore &core) const;

   vfiltergroups m_vMflgroups, m_vHflgroups, m_vPflgroups, m_vAflgroups;

   // predefined filters, set up at c'tor
   st_filters m_expirefilter, m_unsavedfilter, m_lastfoundfilter;

   // Filter on Find results
   bool m_bFindFilterActive;
   // Vector of found entries' UUID for advance search to display only those
   // entries satisfying a search
   std::vector<pws_os::CUUID> m_vFltrFoundUUIDs;
};

#endif  /* __PWSFILTERS_H */
