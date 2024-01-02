/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
}

typedef std::vector<PWHistEntry> PWHistVect;

class PWHistList: public PWHistVect
{
private:
    // These values are initialized from the input string parsed by the constructor
    bool m_saveHistory;     // Saving history flag
    size_t m_maxEntries;    // Maximum number of entries to allow
    size_t m_numErr;        // Number of ill-formed entries

    void sortList();

public:
    // Parse a password history string as defined
    // in the format spec to a vector of PWHistEntry
    PWHistList(const StringX &pwh_str, PWSUtil::TMC time_format);

    PWHistList() : m_saveHistory(false), m_maxEntries(0), m_numErr(0) {};
    PWHistList(PWHistList &) = default;
    ~PWHistList() = default;

    // Convert this object to a string in the canonical DB format
    operator StringX();

    bool isSaving() { return m_saveHistory; };
    size_t getMax() { return m_maxEntries; };
    size_t getErr() { return m_numErr; };

    void setMax(size_t x) { m_maxEntries = x; };     // The list will be trimed, if necessary, when a StringX is generated
    void setSaving(bool b) { m_saveHistory = b; };

    void addEntry(const PWHistEntry &pwh_ent) { push_back(pwh_ent); };

    static StringX GetPreviousPassword(const StringX &pwh_str);
    static StringX MakePWHistoryHeader(bool status, size_t pwh_max, size_t pwh_num = 0);

    StringX MakePWHistoryHeader() { return MakePWHistoryHeader(m_saveHistory, m_maxEntries, size()); };
};

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
