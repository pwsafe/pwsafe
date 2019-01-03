/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "VerifyFormat.h"
#include "core.h"
#include "StringXStream.h"

#include <ctime>
#include <vector>

static const TCHAR *sHex = _T("0123456789abcdefABCDEF");

// Our own mktime, differs from libc's via argument overloading
static time_t mktime(int yyyy, int mon, int dd,
                     int hh = 0, int min = 0, int sec = 0, int *dow = nullptr)
{
  struct tm xtm;
  memset(&xtm, 0, sizeof(tm));
  xtm.tm_year = yyyy - 1900;
  xtm.tm_mon = mon - 1;
  xtm.tm_mday = dd;
  xtm.tm_hour = hh;
  xtm.tm_min = min;
  xtm.tm_sec = sec;
  xtm.tm_isdst = -1;
  time_t retval = std::mktime(&xtm);
  if (dow != nullptr)
    *dow = xtm.tm_wday + 1;
  return retval;
}

bool verifyDTvalues(int yyyy, int mon, int dd,
                    int hh, int min, int ss)
{
  const int month_lengths[12] = {31, 28, 31, 30, 31, 30,
                                 31, 31, 30, 31, 30, 31};
  // Built-in obsolescence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon == 2 && (yyyy % 4) == 0) {
    // Feb and a leap year
    if (dd > 29)
      return false;
  } else {
    // Either (Not Feb) or (Is Feb but not a leap-year)
    if (dd > month_lengths[mon - 1])
      return false;
  }

  if ((hh < 0 || hh > 23) ||
      (min < 0 || min > 59) ||
      (ss < 0 || ss > 59))
    return false;

  return true;
}

bool VerifyImportDateTimeString(const stringT &time_str, time_t &t)
{
  //  String format must be "yyyy/mm/dd hh:mm:ss"
  //                        "0123456789012345678"

  const int ndigits = 14;
  const int idigits[ndigits] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18};
  int yyyy, mon, dd, hh, min, ss;

  t = time_t(-1);

  if (time_str.length() != 19)
    return false;

  // Validate time_str
  if (time_str[4] != TCHAR('/') ||
      time_str[7] != TCHAR('/') ||
      (time_str[10] != TCHAR(' ') && time_str[10] != TCHAR('T')) ||
      time_str[13] != TCHAR(':') ||
      time_str[16] != TCHAR(':'))
    return false;

  for (int i = 0;  i < ndigits; i++)
    if (!isdigit(time_str[idigits[i]]))
      return false;

  istringstreamT is(time_str);
  TCHAR dummy;

  is >> yyyy >> dummy >> mon >> dummy >> dd
     >> hh >> dummy >> min >> dummy >> ss;

  if (!verifyDTvalues(yyyy, mon, dd, hh, min, ss))
    return false;

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with mktime after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = time_t(0);
    return true;
  }

  t = mktime(yyyy, mon, dd, hh, min, ss);
  return true;
}

bool VerifyASCDateTimeString(const stringT &time_str, time_t &t)
{
  //  String format must be "ddd MMM dd hh:mm:ss yyyy"
  //                        "012345678901234567890123"
  // e.g.,                  "Wed Oct 06 21:02:38 2008"

  const stringT str_months = _T("JanFebMarAprMayJunJulAugSepOctNovDec");
  const stringT str_days = _T("SunMonTueWedThuFriSat");
  const int ndigits = 12;
  const int idigits[ndigits] = {8, 9, 11, 12, 14, 15, 17, 18, 20, 21, 22, 23};
  stringT::size_type iMON, iDOW;
  int yyyy, mon, dd, hh, min, ss;

  t = time_t(-1);

  if (time_str.length() != 24)
    return false;

  // Validate time_str
  if (time_str[13] != TCHAR(':') ||
      time_str[16] != TCHAR(':'))
    return false;

  for (int i = 0; i < ndigits; i++)
    if (!isdigit(time_str[idigits[i]]))
      return false;

  istringstreamT is(time_str);
  stringT dow, mon_str;
  TCHAR dummy;
  is >> dow >> mon_str >> dd >> hh >> dummy >> min >> dummy >> ss >> yyyy;

  iMON = str_months.find(mon_str);
  if (iMON == stringT::npos)
    return false;

  mon = ((reinterpret_cast<int &>(iMON) / 3) + 1);

  if (!verifyDTvalues(yyyy, mon, dd, hh, min, ss))
    return false;

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with mktime after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = time_t(0);
    return true;
  }

  int  mktime_dow;
  time_t xt = mktime(yyyy, mon, dd, hh, min, ss, &mktime_dow);

  iDOW = str_days.find(dow);
  if (iDOW == stringT::npos)
    return false;

  iDOW = (iDOW / 3) + 1;
  if (iDOW != stringT::size_type(mktime_dow))
    return false;

  t = xt;
  return true;
}

bool VerifyXMLDateTimeString(const stringT &time_str, time_t &t)
{
  //  String format must be "yyyy-mm-ddThh:mm:ss"
  //                    or  "yyyy-mm-ddThh:mm:ssZ"
  //                    or  "yyyy-mm-ddThh:mm:ss+hh:mm"
  //                    or  "yyyy-mm-ddThh:mm:ss-hh:mm"
  //                        "0123456789012345678901234"

  // e.g.,                  "2008-10-06T21:20:56"
  //                        "2008-10-06T21:20:56Z"
  //                        "2008-10-06T21:20:56+01:00"
  //                        "2008-10-06T21:20:56-01:00"

  int ndigits(14);
  const int idigits[18] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18, 20, 21, 23, 24};
  int yyyy, mon, dd, hh, min, ss, tz_hh(0), tz_mm(0);

  t = time_t(-1);

  const size_t len = time_str.length();
  if (len < 19)
    return false;

  // Validate time_str
  if (time_str[4] != TCHAR('-')  ||
      time_str[7] != TCHAR('-')  ||
      time_str[10] != TCHAR('T') ||
      time_str[13] != TCHAR(':') ||
      time_str[16] != TCHAR(':'))
    return false;

  switch (len) {
    case 19:
      break;
    case 20:
      if (time_str[19] != TCHAR('Z'))
        return false;
      break;
    case 25:
      if (time_str[22] != TCHAR(':')  &&
          (time_str[19] != TCHAR('+') &&
           time_str[19] != TCHAR('-')))
        return false;
      ndigits = 18;
      break;
    default:
      return false;
  }

  for (int i = 0; i < ndigits; i++) {
    if (!isdigit(time_str[idigits[i]]))
      return false;
  }

  istringstreamT is(time_str);
  TCHAR dummy;
  is >> yyyy >> dummy >> mon >> dummy >> dd >> dummy
     >>  hh  >> dummy >> min >> dummy >> ss;

  if (len == 25) {
    is >> tz_hh >> dummy >> tz_mm;

    if (tz_mm > 59 || abs(tz_hh) > 14 ||
        (abs(tz_hh) == 14 && tz_mm != 0))
      tz_hh = tz_mm = 0;
  }

  if (!verifyDTvalues(yyyy, mon, dd, hh, min, ss))
    return false;

  // Accept 1970-01-01 as a special 'unset' value, otherwise there can be
  // issues with mktime after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = time_t(0);
    return true;
  }

  t = mktime(yyyy, mon, dd, hh, min, ss);

  // Add timezone offsets
  if (tz_hh != 0 || tz_mm != 0) {
    t -= (tz_hh * 60 + tz_mm) * 60;
  }

  return true;
}

bool VerifyXMLDateString(const stringT &time_str, time_t &t)
{
  //  String format must be "yyyy-mm-dd"
  //                        "0123456789"

  const int ndigits = 8;
  const int idigits[ndigits] = {0, 1, 2, 3, 5, 6, 8, 9};
  int yyyy, mon, dd;

  t = time_t(-1);

  if (time_str.length() != 10)
    return false;

  // Validate time_str
  if (time_str.substr(4,1) != _S("-") ||
      time_str.substr(7,1) != _S("-"))
    return false;

  for (int i = 0; i < ndigits; i++) {
    if (!isdigit(time_str[idigits[i]]))
      return false;
  }

  istringstreamT is(time_str);
  TCHAR dummy;

  is >> yyyy >> dummy >> mon >> dummy >> dd;

  if (!verifyDTvalues(yyyy, mon, dd, 1, 2, 3))
    return false;

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with mktime after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = time_t(0);
    return true;
  }

  t = mktime(yyyy, mon, dd);
  return true;
}

int VerifyTextImportPWHistoryString(const StringX &PWHistory,
                                    StringX &newPWHistory, stringT &strErrors)
{
  // Format is (! == mandatory blank, unless at the end of the record):
  //    sxx00
  // or
  //    sxxnn!yyyy/mm/dd!hh:mm:ss!llll!pppp...pppp!yyyy/mm/dd!hh:mm:ss!llll!pppp...pppp!.........
  // Note:
  //    !yyyy/mm/dd!hh:mm:ss! may be !1970-01-01 00:00:00! meaning unknown

  StringX sxBuffer;
  size_t ipwlen, pwleft = 0;
  int s = -1, m = -1, n = -1;
  int rc = PWH_OK;
  time_t t;

  newPWHistory = _T("");
  strErrors = _T("");

  if (PWHistory.empty())
    return PWH_OK;

  StringX pwh(PWHistory);
  StringX tmp;
  const TCHAR *lpszPWHistory = nullptr;
  size_t len = pwh.length();

  if (len < 5) {
    rc = PWH_INVALID_HDR;
    goto exit;
  }

  if (pwh[0] == TCHAR('0')) s = 0;
  else if (pwh[0] == TCHAR('1')) s = 1;
  else {
    rc = PWH_INVALID_STATUS;
    goto exit;
  }

  if (PWHistory.substr(0, 4).find_first_not_of(sHex) != StringX::npos) {
    // Header not hex!
    rc = PWH_INVALID_HDR;
    goto exit;
  }

  {
    StringX s1 (pwh.substr(1, 2));
    StringX s2 (pwh.substr(3, 4));
    iStringXStream is1(s1), is2(s2);
    is1 >> std::hex >> m;
    is2 >> std::hex >> n;
  }

  if (n > m) {
    rc = PWH_INVALID_NUM;
    goto exit;
  }

  lpszPWHistory = pwh.c_str() + 5;
  pwleft = len - 5;

  if (pwleft == 0 && s == 0 && m == 0 && n == 0) {
    rc = PWH_IGNORE;
    goto exit;
  }

  Format(sxBuffer, L"%01d%02x%02x", s, m, n);
  newPWHistory = sxBuffer;

  for (int i = 0; i < n; i++) {
    if (pwleft < 26) {  //  blank + date(10) + blank + time(8) + blank + pw_length(4) + blank
      rc = PWH_TOO_SHORT;
      goto exit;
    }

    if (lpszPWHistory[0] != _T(' ')) {
      rc = PWH_INVALID_CHARACTER;
      goto exit;
    }

    lpszPWHistory++;
    pwleft--;

    tmp = StringX(lpszPWHistory, 19);

    if (tmp.substr(0, 10) == _T("1970-01-01"))
      t = 0;
    else {
      if (!VerifyImportDateTimeString(tmp.c_str(), t) ||
          (t == time_t(-1))) {
        rc = PWH_INVALID_DATETIME;
        goto exit;
      }
    }

    lpszPWHistory += 19;
    pwleft -= 19;

    if (lpszPWHistory[0] != _T(' ')) {
      rc = PWH_INVALID_CHARACTER;
      goto exit;
    }

    lpszPWHistory++;
    pwleft--;
    {
      StringX s3(lpszPWHistory, 4);
      iStringXStream is3(s3);
      is3 >> std::hex >> ipwlen;
    }
    lpszPWHistory += 4;
    pwleft -= 4;

    if (lpszPWHistory[0] != _T(' ')) {
      rc = PWH_INVALID_CHARACTER;
      goto exit;
    }

    lpszPWHistory += 1;
    pwleft -= 1;

    if (pwleft < ipwlen) {
      rc = PWH_INVALID_PSWD_LENGTH;
      goto exit;
    }

    tmp = StringX(lpszPWHistory, ipwlen);
    Format(sxBuffer, L"%08x%04x%ls", static_cast<long>(t), ipwlen, tmp.c_str());
    newPWHistory += sxBuffer;
    sxBuffer = L"";
    lpszPWHistory += ipwlen;
    pwleft -= ipwlen;
  }

  if (pwleft > 0)
    rc = PWH_TOO_LONG;

exit:
  stringT buffer, temp(_T(""));
  Format(temp, IDSC_PWHERRORTEXT, buffer.c_str(), len - pwleft + 1);
  Format(buffer, IDSC_PWHERROR, temp.c_str());
  switch (rc) {
    case PWH_OK:
    case PWH_IGNORE:
      temp.clear();
      buffer.clear();
      break;
    case PWH_INVALID_HDR:
      Format(temp, IDSC_INVALIDHEADER, PWHistory.c_str());
      break;
    case PWH_INVALID_STATUS:
      Format(temp, IDSC_INVALIDPWHSTATUS, s);
      break;
    case PWH_INVALID_NUM:
      Format(temp, IDSC_INVALIDNUMOLDPW, n, m);
      break;
    case PWH_INVALID_DATETIME:
      LoadAString(temp, IDSC_INVALIDDATETIME);
      break;
    case PWH_INVALID_PSWD_LENGTH:
      LoadAString(temp, IDSC_INVALIDPWLENGTH);
      break;
    case PWH_TOO_SHORT:
      LoadAString(temp, IDSC_FIELDTOOSHORT);
      break;
    case PWH_TOO_LONG:
      LoadAString(temp, IDSC_FIELDTOOLONG);
      break;
    case PWH_INVALID_CHARACTER:
      LoadAString(temp, IDSC_INVALIDSEPARATER);
      break;
    default:
      ASSERT(0);
  }
  strErrors = buffer + temp;
  if (rc != PWH_OK)
    newPWHistory = _T("");

  return rc;
}

int VerifyXMLImportPWHistoryString(const StringX &PWHistory,
                                   StringX &newPWHistory, stringT &strErrors)
{
  // Format is (! == mandatory blank, unless at the end of the record):
  //    sxx00
  // or
  //    sxxnn!<time field>!llll!pppp...pppp!<time field>!llll!pppp...pppp!.........
  //
  // Note:
  //   For Plain text input the <time field> is fixed as "yyyy/mm/dd!hh:mm:ss"
  //   For XML input the <time field> is defined by the W3C xs:dateTime specification
  //   of: "yyyy-mm-ddThh:mm:ss", "yyyy-mm-ddThh:mm:ssZ",
  //       "yyyy-mm-ddThh:mm:ss+hh:mm" or "yyyy-mm-ddThh:mm:ss-hh:mm"
  //
  // A date value of '1970-01-01' (irrespective of the time value) is interpreted
  // as 'unknown'.

  StringX sxBuffer, tmp;
  std::vector<StringX> in_tokens, out_entries;
  int s = -1;
  size_t nerror(size_t(-1));
  unsigned int ipwlen, m = 0, n = 0; // using uint instead of size_t to use 'x' format spec instead of complier-dependent z/I
  int rc = PWH_OK;
  time_t t;

  newPWHistory = _T("");
  strErrors = _T("");

  if (PWHistory.empty())
    return PWH_OK;

  const size_t len = PWHistory.length();

  if (len < 5) {
    rc = PWH_INVALID_HDR;
    goto exit;
  }

  if (PWHistory.substr(0, 4).find_first_not_of(sHex) != StringX::npos) {
    // Header not hex!
    rc = PWH_INVALID_HDR;
    goto exit;
  }

  if (PWHistory[0] == TCHAR('0')) s = 0;
  else if (PWHistory[0] == TCHAR('1')) s = 1;
  else {
    rc = PWH_INVALID_STATUS;
    goto exit;
  }

  if (PWHistory.substr(1, 4).find_first_not_of(sHex) != StringX::npos) {
    // Password length not hex!
    rc = PWH_PSWD_LENGTH_NOTHEX;
    goto exit;
  } else {
    StringX s1(PWHistory.substr(1, 2));
    StringX s2(PWHistory.substr(3, 4));
    iStringXStream is1(s1), is2(s2);
    is1 >> std::hex >> m;
    is2 >> std::hex >> n;
  }

  if (n > m) {
    rc = PWH_INVALID_NUM;
    goto exit;
  }

  if (len == 5 && s == 0 && m == 0 && n == 0) {
    rc = PWH_IGNORE;
    goto exit;
  }

  if (len <= 6) {
    // Really invalid but just set n == 0 and return
    Format(sxBuffer, L"%01d%02x00", s, m);
    newPWHistory = sxBuffer;
    return PWH_OK;
  }

  // Now tokenize the rest using a blank as a delimiter
  {
    StringX item(PWHistory.substr(6));
    StringXStream ss(item);
    while (getline(ss, item, _T('\xff'))) {
      in_tokens.push_back(item);
    }
  }

  // We need to handle these in 3s or 4s depending on whether we are doing it for
  // plain text input (4 - date, time, length, password) or
  // XML input (3 - datetime, length, password).

  // Check we have enough
  if (in_tokens.size() != n * 3) {
    // too few or too many - set number to number of complete entries
    n = (in_tokens.size() % 3);
  }

  // Now only verify them and create out_tokens for processing
  size_t it;
  it = 0;  // token counter
  size_t ie;      // entry counter
  for (ie = 0; ie < n; ie++) {
    StringX sxDatetime, sxPWLen, sxPassword;
    // Check datetime and password length fields
    size_t idtlen = in_tokens[it].length();
    if ((idtlen != 19 && idtlen != 20 && idtlen != 25) ||
        in_tokens[it + 1].length() != 4) {
      // Bad lengths
      rc = PWH_INVALID_FIELD_LENGTH;
      break;
    }
    sxDatetime = in_tokens[it];
    sxPWLen = in_tokens[it + 1];
    sxPassword = in_tokens[it + 2];

    // Get password length
    if (sxPWLen.find_first_not_of(sHex) != StringX::npos) {
      // Password length not hex!
      rc = PWH_PSWD_LENGTH_NOTHEX;
      break;
    } else {
      iStringXStream iss_pwlen(sxPWLen.c_str());
      iss_pwlen >> std::hex >> ipwlen;
    }

    // Check password field length
    if (sxPassword.length() != ipwlen) {
      // Bad entry - stop with what we have
      rc = PWH_INVALID_PSWD_LENGTH;
      break;
    }

    // Verify datetime field
    if (!VerifyXMLDateTimeString(sxDatetime.c_str(), t) ||
        (t == time_t(-1))) {
       rc = PWH_INVALID_DATETIME;
      break;
    }

    Format(sxBuffer, L"%08lx%04x%ls", static_cast<long>(t), ipwlen,
                 sxPassword.c_str());
    out_entries.push_back(sxBuffer);
    sxBuffer = _T("");
    it += 3;
  }

  if (rc != PWH_OK)
    nerror = ie;

  n = (unsigned int)out_entries.size();
  Format(sxBuffer, L"%01d%02x%02x", s, m, n);
  newPWHistory = sxBuffer;

  for (size_t i = 0; i < out_entries.size(); i++)
    newPWHistory += out_entries[i];

 exit:
  stringT buffer, temp(_T(""));
  if (nerror != size_t(-1)) {
    // Need to add information about which PWH entry is in error
    LoadAString(buffer, IDSC_ENTRY);
    Format(temp, L"%ls %d", buffer.c_str(), nerror);
  }
  Format(buffer, IDSC_PWHERROR, temp.c_str());
  switch (rc) {
    case PWH_OK:
    case PWH_IGNORE:
      temp.clear();
      buffer.clear();
      break;
    case PWH_INVALID_HDR:
      Format(temp, IDSC_INVALIDHEADER, PWHistory.substr(0, 5).c_str());
      break;
    case PWH_INVALID_STATUS:
      Format(temp, IDSC_INVALIDPWHSTATUS, s);
      break;
    case PWH_INVALID_NUM:
      Format(temp, IDSC_INVALIDNUMOLDPW, n, m);
      break;
    case PWH_INVALID_DATETIME:
      LoadAString(temp, IDSC_INVALIDDATETIME);
      break;
    case PWH_INVALID_PSWD_LENGTH:
      LoadAString(temp, IDSC_INVALIDPWLENGTH);
      break;
    case PWH_PSWD_LENGTH_NOTHEX:
      LoadAString(temp, IDSC_INVALIDPWLENGTHX);
      break;
    case PWH_INVALID_FIELD_LENGTH:
      LoadAString(temp, IDSC_INVALIDFIELDLENGTH);
      break;
    default:
      ASSERT(0);
  }

  if (rc != PWH_OK) {
    strErrors = buffer.c_str();
    strErrors += temp;
    newPWHistory = _T("");
  }

  return rc;
}
