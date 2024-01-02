/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWHistory.cpp
//-----------------------------------------------------------------------------
#include "PWHistory.h"
#include "StringXStream.h"

#include <sstream>
#include <iomanip>
#include <algorithm> // for sort

using namespace std;

// This is the canonical sort order for storing and comparing the history list
void PWHistList::sortList()
{
  // Make sure entries are sorted oldest first.  This is consistent with
  // other routines that expect the newest entries to be at the end.
  std::sort( begin(), end(),
            [](const PWHistEntry& first, const PWHistEntry& second) -> bool {
                return first.changetttdate < second.changetttdate;
              }
  );
}

// Parse a string in the canonical history format and build an object
PWHistList::PWHistList(const StringX &pwh_str, PWSUtil::TMC time_format)
{
  // Return boolean value stating if PWHistory status is active
  m_saveHistory = false;
  m_maxEntries = m_numErr = 0;
  StringX pwh_s = pwh_str;
  const size_t len = pwh_s.length();

  if (len < 5) {
    m_numErr = len != 0 ? 1 : 0;
    return;
  }
  bool bStatus = pwh_s[0] != charT('0');

  int n;
  iStringXStream ism(StringX(pwh_s, 1, 2)); // max history 1 byte hex
  ism >> hex >> m_maxEntries;
  if (!ism)
    return;

  iStringXStream isn(StringX(pwh_s, 3, 2)); // cur # entries 1 byte hex
  isn >> hex >> n;
  if (!isn)
    return;

  // Sanity check: Each entry has at least 12 bytes representing
  // time + pw length
  // so if pwh_s isn't long enough check if it contains integral number of history records
  if (len - 5 < unsigned(12 * n)) {
    size_t offset = 5;
    bool err=false;
    while (offset < len) {
      offset += 8; //date
      if (offset + 4 >= len) { //passwd len
        err = true;
        break;
      }
      iStringXStream ispwlen(StringX(pwh_s, offset, 4)); // pw length 2 byte hex
      if (!ispwlen){
        err = true;
        break;
      }
      int ipwlen = 0;
      ispwlen >> hex >> ipwlen;
      if ( (ipwlen <= 0) || (offset + 4 + ipwlen > len) ) {
        err = true;
        break;
      }
      offset += 4 + ipwlen;
    }
    if ( err || (offset != len) ) {
      m_numErr = n;
      return;
    }
    //number of errors will be counted later
  }

  // Case when password history field is too long and no passwords present
  if (n == 0 && pwh_s.length() != 5) {
    m_numErr = static_cast<size_t>(-1);
    m_saveHistory = bStatus;
    return;
  }

  size_t offset = 1 + 2 + 2; // where to extract the next token from pwh_s

  for (int i = 0; i < n; i++) {
    if (offset >= len) {
      // Trying to read past end of buffer!
      m_numErr++;
      break;
    }

    PWHistEntry pwh_ent;
    long t = 0L;
    iStringXStream ist(StringX(pwh_s, offset, 8)); // time in 4 byte hex
    ist >> hex >> t;
    // Note: t == 0 - means time is unknown - quite possible for the
    // oldest saved password
    if (!ist) {
      // Invalid time of password change
      m_numErr++;
      continue;
    }

    offset += 8;
    if (offset >= pwh_s.length())
      break;

    pwh_ent.changetttdate = static_cast<time_t>(t);
    pwh_ent.changedate =
      PWSUtil::ConvertToDateTimeString(static_cast<time_t>(t), time_format);
    if (pwh_ent.changedate.empty()) {
      //                       1234567890123456789
      pwh_ent.changedate = _T("1970-01-01 00:00:00");
    }

    iStringXStream ispwlen(StringX(pwh_s, offset, 4)); // pw length 2 byte hex
    int ipwlen = 0;
    ispwlen >> hex >> ipwlen;
    if (offset + 4 + ipwlen > pwh_s.length())
      break;

    if (!ispwlen || ipwlen == 0) {
      // Invalid password length of zero
      m_numErr++;
      continue;
    }

    offset += 4;
    const StringX pw(pwh_s, offset, ipwlen);
    pwh_ent.password = pw.c_str();
    offset += ipwlen;
    addEntry(pwh_ent);
  }
  sortList();   // Old DB entries might not be in the "proper" order

  m_numErr += n - size();
  m_saveHistory = bStatus;
}

StringX PWHistList::GetPreviousPassword(const StringX &pwh_str)
{
  if (pwh_str == _T("0") || pwh_str == _T("00000")) {
    return _T("");
  } else {
    // Get all history entries
    PWHistList pwhistlist(pwh_str, PWSUtil::TMC_EXPORT_IMPORT);

    // If none yet saved, then don't return anything
    if (pwhistlist.empty())
      return _T("");

    // Sort in date order and return last saved
    pwhistlist.sortList();
    return pwhistlist.back().password;
  }
}

StringX PWHistList::MakePWHistoryHeader(bool status, size_t pwh_max, size_t pwh_num)
{
  const size_t MAX_PWHISTORY = 255;
  if (pwh_max > MAX_PWHISTORY)
    throw _T("Internal error: max history exceeded");
  if (pwh_num > MAX_PWHISTORY)
    throw _T("Internal error: history list too large");
  ostringstreamT os;
  os.fill(charT('0'));
  os << hex << setw(1) << status
     << setw(2) << pwh_max << setw(2) << pwh_num;
  return os.str().c_str();
}

// Sort the list, trim it to the maximum length if necessary, and format as a StringX.
// This code was factored from CItemData::UpdatePasswordHistory and AddEditPropSheetDlg::PreparePasswordHistory
// and is called from several places.
PWHistList::operator StringX() {

  // Make sure the list is sorted in the proper order
  sortList();

  // If the number of entries is greater than the max allowed,
  // trim the list from the front, discarding the oldest.
  size_t num = size();
  if (num > m_maxEntries) {
    PWHistVect hv(begin() + (num - m_maxEntries), end());
    ASSERT(hv.size() == m_maxEntries);
    assign(hv.begin(), hv.end());
  }

  // Now create the string version, starting with a header...
  StringX new_PWHistory, buffer;
  new_PWHistory = MakePWHistoryHeader();

  // Encode each of the history entries into the string format
  PWHistList::iterator iter;
  for (iter = begin(); iter != end(); iter++) {
    Format(buffer, L"%08x%04x%ls",
           static_cast<long>(iter->changetttdate), iter->password.length(),
           iter->password.c_str());
    new_PWHistory += buffer;
  }
  return new_PWHistory;
}
