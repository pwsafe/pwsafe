/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "NumUtilities.h"

// Utility format functions

// Converts a grouping string returned by
// GetLocaleInfo(LOCALE_SGROUPING) into a UINT understood by NUMBERFMT.
// Steven Engelhardt at http://www.deez.info/sengelha/category/programming/cplusplus/
UINT PWSNumUtil::GroupingStrToUint(wchar_t *wcGrouping)
{
  wchar_t *wcCurr = wcGrouping;
  UINT ret = 0;

  while (true) {
    ret *= 10;
    if (*wcCurr == L'\0')
      break;

    wchar_t *pch;
    ret += wcstol(wcCurr, &pch, 10);

    if (wcscmp(pch, L";0") == 0)
      break;

    wcCurr = pch + 1;
  }
  return ret;
}

// Fills the default NUMBERFMT structure for a given locale.
// pFmt->lpDecimalSep and pFmt->lpThousandSep must point to valid
// buffers of size cchDecimalSep and cchThousandSep respectively.
// Steven Engelhardt at http://www.deez.info/sengelha/category/programming/cplusplus/
BOOL PWSNumUtil::GetDefaultNumberFormat(LCID lcid, NUMBERFMT* pFmt,
                                        int cchDecimalSep, int cchThousandSep)
{
  wchar_t wcBuf[80];

  int ret = ::GetLocaleInfo(lcid, LOCALE_IDIGITS, wcBuf,
                              ARRAYSIZE(wcBuf));
  if (ret == 0)
    return FALSE;
  pFmt->NumDigits = wcstol(wcBuf, NULL, 10);

  ret = ::GetLocaleInfo(lcid, LOCALE_ILZERO, wcBuf, ARRAYSIZE(wcBuf));
  if (ret == 0)
    return FALSE;
  pFmt->LeadingZero = wcstol(wcBuf, NULL, 10);

  ret = ::GetLocaleInfo(lcid, LOCALE_SGROUPING, wcBuf,
                        ARRAYSIZE(wcBuf));
  if (ret == 0)
    return FALSE;
  pFmt->Grouping = GroupingStrToUint(wcBuf);

  ret = ::GetLocaleInfo(lcid, LOCALE_SDECIMAL, pFmt->lpDecimalSep,
                        cchDecimalSep);
  if (ret == 0)
    return FALSE;

  ret = ::GetLocaleInfo(lcid, LOCALE_STHOUSAND, pFmt->lpThousandSep,
                        cchThousandSep);
  if (ret == 0)
    return FALSE;

  ret = ::GetLocaleInfo(lcid, LOCALE_INEGNUMBER, wcBuf,
                        ARRAYSIZE(wcBuf));
  if (ret == 0)
    return FALSE;

  pFmt->NegativeOrder = wcstol(wcBuf, NULL, 10);
  return TRUE;
}

// Steven Engelhardt at http://www.deez.info/sengelha/category/programming/cplusplus/
// DK - Make sure character field is big enough for what the user wants
int PWSNumUtil::GetNumberFormatDouble(const LCID locale, const DWORD dwFlags,
                                      const double value, const NUMBERFMT* lpFormat,
                                      wchar_t *lpNumberStr, int cchNumber)
{
  // DBL_MAX is 1.7976931348623158E+308
  // Max. number of characters required for printf is found from:
  //   (unsigned int)log(DBL_MAX) + 1 + 3 + n
  // (unsigned int)log(DBL_MAX) = 308
  // The '1' = the leading non-zero digit before the decimal point
  // The '3' = [possible minus sign] + [possible decimal point] + terminating null
  // The 'n' = number of digits after the decimal point (supplied by user)
  const int n = lpFormat->NumDigits + 312;
  wchar_t *wcBuf = new wchar_t[n];
  if (swprintf_s(wcBuf, n, L"%f", value) == -1) {
    SetLastError(ERROR_INVALID_PARAMETER);
    delete [] wcBuf;
    return 0;
  }
  int irc = ::GetNumberFormat(locale, dwFlags, wcBuf, lpFormat,
                              lpNumberStr, cchNumber);
  delete [] wcBuf;
  return irc;
}

// Steven Engelhardt at http://www.deez.info/sengelha/category/programming/cplusplus/
// Converts the decimal value to a localized string for the specified
// locale with the given number of fractional digits.
bool PWSNumUtil::DoubleToLocalizedString(const LCID lcid, const double value,
                                         const int nDigits, wchar_t *wcStr, int cchStr)
{
  // Get locale-default NUMBERFMT
  wchar_t wcDecimalSep[5];
  wchar_t wcThousandSep[5];

  NUMBERFMT fmt;
  fmt.lpDecimalSep = wcDecimalSep;
  fmt.lpThousandSep = wcThousandSep;
  if (!GetDefaultNumberFormat(lcid, &fmt, ARRAYSIZE(wcDecimalSep),
                              ARRAYSIZE(wcThousandSep)))
    return false;

  // Override the NumDigits member of NUMBERFMT
  fmt.NumDigits = nDigits;

  // Format the number with the custom NUMBERFMT
  int ret = GetNumberFormatDouble(lcid, 0, value, &fmt, wcStr, cchStr);
  return (ret != 0);
}
