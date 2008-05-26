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

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wistringstream istringstreamT;
typedef std::wostringstream ostringstreamT;
#else
typedef std::istringstream istringstreamT;
typedef std::ostringstream ostringstreamT;
#endif

using namespace std;

int CreatePWHistoryList(const CMyString &pwh_str,
                        BOOL &status, size_t &pwh_max, size_t &pwh_num,
                        PWHistList &pwhl, int time_format)
{
  status = FALSE;
  pwh_max = pwh_num = 0;

  stringT pwh_s = pwh_str;
  int len = pwh_s.length();

  if (len < 5)
    return (len != 0 ? 1 : 0);

  BOOL s = pwh_s[0] == charT('0') ? FALSE : TRUE;

  int m, n;
  istringstreamT ism(stringT(pwh_s, 1, 2)); // max history 1 byte hex
  istringstreamT isn(stringT(pwh_s, 3, 2)); // cur # entries 1 byte hex
  ism >> hex >> m;
  if (!ism) return 1;
  isn >> hex >> n;
  if (!isn) return 1;

  // Sanity check: Each entry has at least 12 bytes representing time + pw length
  // so if pwh_s isn't long enough, we bail out pronto.
  if (pwh_s.length() - 5 < unsigned(12 * n)) {
    status = FALSE;
    return n;
  }

  int offset = 1 + 2 + 2; // where to extract the next token from pwh_s
  int i_error = 0;

  for (int i = 0; i < n; i++) {
    PWHistEntry pwh_ent;
    long t;
    istringstreamT ist(stringT(pwh_s, offset, 8)); // time in 4 byte hex
    ist >> hex >> t;
    if (!ist) {i_error++; continue;} // continue or break?
    offset += 8;

    pwh_ent.changetttdate = (time_t) t;
    pwh_ent.changedate =
      PWSUtil::ConvertToDateTimeString((time_t) t, time_format);
    if (pwh_ent.changedate.IsEmpty()) {
      //                       1234567890123456789
      pwh_ent.changedate = _T("1970-01-01 00:00:00");
    }
    istringstreamT ispwlen(stringT(pwh_s, offset, 4)); // pw length 2 byte hex
    int ipwlen;
    ispwlen >> hex >> ipwlen;
    if (!ispwlen) {i_error++; continue;} // continue or break?
    offset += 4;
    const stringT pw(pwh_s, offset, ipwlen);
    pwh_ent.password = pw.c_str();
    offset += ipwlen;
    pwhl.push_back(pwh_ent);
  }

  status = s;
  pwh_max = m;
  pwh_num = n;
  return i_error;
}


CMyString MakePWHistoryHeader(BOOL status, size_t pwh_max, size_t pwh_num)
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
  return CMyString(os.str().c_str());
}
