/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWHistory.cpp
//-----------------------------------------------------------------------------
#include "PWHistory.h"
#include "Util.h"
#include <sstream>
#include <iomanip>
#include "StringXStream.h"

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wistringstream istringstreamT;
typedef std::wostringstream ostringstreamT;
#else
typedef std::istringstream istringstreamT;
typedef std::ostringstream ostringstreamT;
#endif

using namespace std;

bool CreatePWHistoryList(const StringX &pwh_str,
                         size_t &pwh_max, size_t &num_err,
                         PWHistList &pwhl, int time_format)
{
  pwh_max = num_err = 0;

  StringX pwh_s = pwh_str;
  int len = pwh_s.length();

  if (len < 5) {
    num_err = len != 0 ? 1 : 0;
    return false;
  }
  bool retval = pwh_s[0] != charT('0');

  int n;
  iStringXStream ism(StringX(pwh_s, 1, 2)); // max history 1 byte hex
  ism >> hex >> pwh_max;
  if (!ism) return false;
  iStringXStream isn(StringX(pwh_s, 3, 2)); // cur # entries 1 byte hex
  isn >> hex >> n;
  if (!isn) return false;

  // Sanity check: Each entry has at least 12 bytes representing
  // time + pw length
  // so if pwh_s isn't long enough, we bail out pronto.
  if (pwh_s.length() - 5 < unsigned(12 * n)) {
    num_err = n;
    return false;
  }

  size_t offset = 1 + 2 + 2; // where to extract the next token from pwh_s

  for (int i = 0; i < n; i++) {
    PWHistEntry pwh_ent;
    long t;
    iStringXStream ist(StringX(pwh_s, offset, 8)); // time in 4 byte hex
    ist >> hex >> t;
    if (!ist) {num_err++; continue;}
    offset += 8;
    if (offset >= pwh_s.length())
      break;
    pwh_ent.changetttdate = (time_t) t;
    pwh_ent.changedate =
      PWSUtil::ConvertToDateTimeString((time_t) t, time_format);
    if (pwh_ent.changedate.empty()) {
      //                       1234567890123456789
      pwh_ent.changedate = _T("1970-01-01 00:00:00");
    }
    iStringXStream ispwlen(StringX(pwh_s, offset, 4)); // pw length 2 byte hex
    int ipwlen;
    ispwlen >> hex >> ipwlen;
    if (offset + 4 + ipwlen > pwh_s.length())
      break;
    if (!ispwlen) {num_err++; continue;}
    offset += 4;
    const StringX pw(pwh_s, offset, ipwlen);
    pwh_ent.password = pw.c_str();
    offset += ipwlen;
    pwhl.push_back(pwh_ent);
  }

  num_err += n - pwhl.size();
  return retval;
}


StringX MakePWHistoryHeader(BOOL status, size_t pwh_max, size_t pwh_num)
{
  const size_t MAX_PWHISTORY = 255;
  if (pwh_max > MAX_PWHISTORY)
    throw _T("Internal error: max history exceeded");
  if (pwh_num > MAX_PWHISTORY)
    throw _T("Internal error: history list too large");
  ostringstreamT os;
  os.fill(charT('0'));
  os << hex << setw(1) << status
     << setw(2) << pwh_max << setw(2) << pwh_num << ends;
  return os.str().c_str();
}
