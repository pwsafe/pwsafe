/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

#ifdef _WIN32
#include <afx.h>
#endif
#include <time.h> // for time_t
#include <vector>
#include "StringX.h"

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

typedef std::vector<PWHistEntry> PWHistList;

// Parses a password history string as defined
// in format spec to a vector of PWHistEntry
// returns true iff password storing flag in string is set.
// pwh_max is the max number passwords the string may have.
// num_err will have the number of ill-formed entries.

bool CreatePWHistoryList(const StringX &pwh_str,
                        size_t &pwh_max, size_t &num_err,
                        PWHistList &pwhl, int time_format);

StringX MakePWHistoryHeader(BOOL status, size_t pwh_max, size_t pwh_num);

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

