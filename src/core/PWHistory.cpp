/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

bool CreatePWHistoryList(const StringX &pwh_str,
                         size_t &pwh_max, size_t &num_err,
                         PWHistList &pwhl, PWSUtil::TMC time_format)
{
  // Return boolean value stating if PWHistory status is active
  pwh_max = num_err = 0;
  pwhl.clear();
  StringX pwh_s = pwh_str;
  const size_t len = pwh_s.length();

  if (len < 5) {
    num_err = len != 0 ? 1 : 0;
    return false;
  }
  bool bStatus = pwh_s[0] != charT('0');

  int n;
  iStringXStream ism(StringX(pwh_s, 1, 2)); // max history 1 byte hex
  ism >> hex >> pwh_max;
  if (!ism)
    return false;

  iStringXStream isn(StringX(pwh_s, 3, 2)); // cur # entries 1 byte hex
  isn >> hex >> n;
  if (!isn)
    return false;

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
      num_err = n;
      return false;
    }
    //number of errors will be counted later
  }

  // Case when password history field is too long and no passwords present
  if (n == 0 && pwh_s.length() != 5) {
    num_err = static_cast<size_t>(-1);
    return bStatus;
  }

  size_t offset = 1 + 2 + 2; // where to extract the next token from pwh_s

  for (int i = 0; i < n; i++) {
    if (offset >= len) {
      // Trying to read past end of buffer!
      num_err++;
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
      num_err++;
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
      num_err++; 
      continue;
    }

    offset += 4;
    const StringX pw(pwh_s, offset, ipwlen);
    pwh_ent.password = pw.c_str();
    offset += ipwlen;
    pwhl.push_back(pwh_ent);
  }

  num_err += n - pwhl.size();
  return bStatus;
}

StringX GetPreviousPassword(const StringX &pwh_str)
{
  if (pwh_str == _T("0") || pwh_str == _T("00000")) {
    return _T("");
  } else {
    // Get all history entries
    size_t num_err, MaxPWHistory;
    PWHistList pwhistlist;
    CreatePWHistoryList(pwh_str, MaxPWHistory, num_err, pwhistlist, PWSUtil::TMC_EXPORT_IMPORT);

    // If none yet saved, then don't return anything
    if (pwhistlist.empty())
      return _T("");

    // Sort in date order and return last saved
    std::sort(pwhistlist.begin(), pwhistlist.end(),
              [](const PWHistEntry &pwhe1, const PWHistEntry &pwhe2) -> bool
              {
                return pwhe1.changetttdate > pwhe2.changetttdate;
              });
    return pwhistlist[0].password;
  }
}

StringX MakePWHistoryHeader(bool status, size_t pwh_max, size_t pwh_num)
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
