/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/**
 *  @file PWHistory.h
 *
 * Password History (PWH):
 * Password history is represented in the entry record as a textual field
 * with the following semantics:
 *
 * Password History Header: 
 * %01x - status for saving PWH for this entry (0 = no; 1 = yes) 
 * %02x - maximum number of entries in this entry 
 * %02x - number of entries currently saved 
 *
 * Each Password History Entry: 
 * %08x - time of this old password was set (time_t) 
 * %04x - length of old password (in TCHAR)
 * %s   - old password 
 *
 * No history being kept for a record can be represented either by the lack
 * of the PWH field (preferred), or by a header of _T("00000"):
 * status = 0, max = 00, num = 00 
 *
 * Note that 0aabb where bb <= aa is possible if password history was enabled in the past
 * but has been disabled and the history hasn't been cleared.
 *
 *
 * This file declares structures and utilities for manipulating entry's password
 * history data per the above format.
 */

//-----------------------------------------------------------------------------

#if !defined PWHistory_h
#define PWHistory_h

#include <time.h> // for time_t
#include <vector>
#include "StringX.h"
#include "Util.h"

struct PWHistEntry {
  time_t changetttdate;
  // "yyyy/mm/dd hh:mm:ss" - format used in ListCtrl & copied to clipboard (best for sorting)
  // "yyyy-mm-ddThh:mm:ss" - format used in XML
  StringX changedate;
  StringX password;

PWHistEntry() : changetttdate(0), changedate(), password() {}
  // copy c'tor and assignment operator, standard idioms
PWHistEntry(const PWHistEntry &that) :
  changetttdate(that.changetttdate), changedate(that.changedate),
    password(that.password) {}

  PWHistEntry &operator=(const PWHistEntry &that)
  { if (this != &that) {
      changetttdate = that.changetttdate;
      changedate = that.changedate;
      password = that.password;
    }
    return *this;
  }
};

namespace PWHist {
  // Following values are used in the Manage Options
  // to bulk modify the password histories.
  // The origin of these values is in how they were
  // implemented in Windows - do not change.
  enum Action {NOCHANGE = 0,
               STOP_INCL_PROT = -1,
               STOP_EXCL_PROT  = 1,
               START_INCL_PROT = -2,
               START_EXCL_PROT  = 2,
               SETMAX_INCL_PROT = -3,
               SETMAX_EXCL_PROT  = 3,
               CLEAR_INCL_PROT = -4,
               CLEAR_EXCL_PROT  = 4,
  };
};

typedef std::vector<PWHistEntry> PWHistList;

// Parses a password history string as defined
// in format spec to a vector of PWHistEntry
// returns true iff password storing flag in string is set.
// pwh_max is the max number passwords the string may have.
// num_err will have the number of ill-formed entries.

bool CreatePWHistoryList(const StringX &pwh_str,
                         size_t &pwh_max, size_t &num_err,
                         PWHistList &pwhl, PWSUtil::TMC time_format);

StringX GetPreviousPassword(const StringX &pwh_str);

StringX MakePWHistoryHeader(bool status, size_t pwh_max, size_t pwh_num);

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
