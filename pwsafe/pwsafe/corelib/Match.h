/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MATCH_H
#define __MATCH_H

// Match.h
//-----------------------------------------------------------------------------

#include "MyString.h"
#include "ItemData.h"

namespace PWSMatch {
  // namespace of common utility functions

  // For  any comparing functions
  // SubGroup Function - if value used is negative, string compare IS case sensitive
  enum MatchRule {
    MR_INVALID = 0,  // Not valid value
    // For string, integer & date comparisons/filtering
    MR_EQUALS = 1, MR_NOTEQUAL,
    MR_ACTIVE, MR_INACTIVE,
    MR_PRESENT, MR_NOTPRESENT,
    MR_SET, MR_NOTSET,
    // For entrytype comparisons/filters
    MR_IS, MR_ISNOT,
    // For string comparisons/filters
    MR_BEGINS, MR_NOTBEGIN, 
    MR_ENDS, MR_NOTEND, 
    MR_CONTAINS, MR_NOTCONTAIN, 
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
    MT_STRING, MT_PASSWORD, MT_INTEGER, MT_DATE, MT_BOOL, MT_PWHIST, MT_POLICY, MT_ENTRYTYPE};

  // Generalised checking
  bool Match(CMyString string1, CMyString &csValue,
             const int &iFunction);
  bool Match(const int &num1, const int &num2, const int &iValue,
             const int &iFunction);
  bool Match(const time_t &time1, const time_t &time2, const time_t &tValue,
             const int &iFunction);
  bool Match(const bool bValue,
             const int &iFunction);  // bool - if field present or not

  UINT GetRule(MatchRule rule);
  char * GetRuleString(MatchRule rule);
  void GetMatchType(const MatchType &mtype,
                    const int &fnum1, const int &fnum2,
                    const time_t &fdate1, const time_t &fdate2,
                    const CString &fstring, const int &fcase,
                    const int &etype, const bool &bBetween,
                    CString &cs1, CString &cs2);
};
#endif /* __MATCH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
