/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MATCH_H
#define __MATCH_H

// Match.h
//-----------------------------------------------------------------------------

#include "StringX.h"
#include "ItemData.h"
//#include "PWSFilters.h"  // For DateType

namespace PWSMatch {
  // namespace of common utility functions

  // For  any comparing functions
  // SubGroup Function - if value used is negative, string compare IS case sensitive
  enum MatchRule {
    MR_INVALID = 0,  // Not valid value
    // For string, integer & date comparisons/filtering
    MR_EQUALS, MR_NOTEQUAL,
    MR_ACTIVE, MR_INACTIVE,
    MR_PRESENT, MR_NOTPRESENT,
    MR_SET, MR_NOTSET,
    // For entrytype & DCA & Attributes comparisons/filters
    MR_IS, MR_ISNOT,
    // For string comparisons/filters
    MR_BEGINS, MR_NOTBEGIN,
    MR_ENDS, MR_NOTEND,
    MR_CONTAINS, MR_NOTCONTAIN,
    MR_CNTNANY, MR_NOTCNTNANY,
    MR_CNTNALL, MR_NOTCNTNALL,
    // For integer and date comparisons/filtering
    MR_BETWEEN,
    // For integer comparisons/filtering
    MR_LT, MR_LE, MR_GT, MR_GE,
    // For date comparisons/filtering
    MR_BEFORE, MR_AFTER,
    // Special rules for Passwords
    MR_EXPIRED, MR_WILLEXPIRE,
    MR_LAST // MUST be last entry
  };

  enum MatchType {MT_INVALID = 0,
                  MT_STRING, MT_PASSWORD, MT_INTEGER, MT_DATE,
                  MT_BOOL, MT_PWHIST, MT_POLICY, MT_ENTRYTYPE,
                  MT_DCA, MT_SHIFTDCA, MT_ENTRYSTATUS, MT_ENTRYSIZE,
                  MT_ATTACHMENT, MT_MEDIATYPE
  };

  // Generalised checking
  bool Match(const StringX &stValue, StringX sx_Object, const int &iFunction);

  template<typename T> bool Match(T v1, T v2, T value, int iFunction)
  {
    switch (iFunction) {
      case MR_EQUALS: return value == v1;
      case MR_NOTEQUAL: return value != v1;
      case MR_BETWEEN: return value >= v1 && value <= v2;
      case MR_LT: return value < v1;
      case MR_LE: return value <= v1;
      case MR_GT: return value > v1;
      case MR_GE: return value >= v1;
      case MR_BEFORE: return value < v1;
      case MR_AFTER: return value > v1;
      default: ASSERT(0);
    }
    return false; // keep compiler happy
  }

  bool Match(bool bValue, int iFunction);  // bool - if field present or not

  UINT GetRule(MatchRule rule);
  MatchRule GetRule(const StringX &sx_mnemonic);
  const char *GetRuleString(const MatchRule rule);
  void GetMatchType(MatchType mtype,
                    int fnum1, int fnum2,
                    time_t fdate1, time_t fdate2, int fdatetype,
                    const stringT &fstring, bool fcase,
                    short fdca, int etype, int estatus, int funit,
                    bool bBetween, stringT &cs1, stringT &cs2);
}
#endif /* __MATCH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
