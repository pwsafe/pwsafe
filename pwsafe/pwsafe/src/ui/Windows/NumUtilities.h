/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "StdAfx.h"

// Number Utilities

namespace PWSNumUtil {
  UINT GroupingStrToUint(wchar_t *wcGrouping);
  BOOL GetDefaultNumberFormat(LCID lcid, NUMBERFMT* pFmt, int cchDecimalSep,
                              int cchThousandSep);
  int GetNumberFormatDouble(const LCID locale, const DWORD dwFlags, const double value,
                            const NUMBERFMT* lpFormat, wchar_t *lpNumberStr,
                            int cchNumber);
  bool DoubleToLocalizedString(const LCID lcid, const double value, const int nDigits,
                               wchar_t *wcStr, int cchStr);
}
