/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file Comp.cpp
//-----------------------------------------------------------------------------

#include "Match.h"
#include <time.h>

bool PWSMatch::Matches(CMyString string1, CMyString &csValue,
                      const int &iFunction)
{
  const int str_len = string1.GetLength();
  const int val_len = csValue.GetLength();

  // Negative = Case   Sensitive
  // Positive = Case INsensitive
  switch (iFunction) {
    case -MR_EQUALS:
    case  MR_EQUALS:
      return ((val_len == str_len) &&
             (((iFunction < 0) && (csValue.Compare((LPCTSTR)string1) == 0)) ||
              ((iFunction > 0) && (csValue.CompareNoCase((LPCTSTR)string1) == 0))));
    case -MR_NOTEQUAL:
    case  MR_NOTEQUAL:
      return (((iFunction < 0) && (csValue.Compare((LPCTSTR)string1) != 0)) ||
              ((iFunction > 0) && (csValue.CompareNoCase((LPCTSTR)string1) != 0)));
    case -MR_BEGINS:
    case  MR_BEGINS:
      if (val_len >= str_len) {
        csValue = csValue.Left(str_len);
        return (((iFunction < 0) && (string1.Compare((LPCTSTR)csValue) == 0)) ||
                ((iFunction > 0) && (string1.CompareNoCase((LPCTSTR)csValue) == 0)));
      } else {
        return false;
      }
    case -MR_NOTBEGIN:
    case  MR_NOTBEGIN:
      if (val_len >= str_len) {
        csValue = csValue.Left(str_len);
        return (((iFunction < 0) && (string1.Compare((LPCTSTR)csValue) != 0)) ||
                ((iFunction > 0) && (string1.CompareNoCase((LPCTSTR)csValue) != 0)));
      } else {
        return false;
      }
    case -MR_ENDS:
    case  MR_ENDS:
      if (val_len > str_len) {
        csValue = csValue.Right(str_len);
        return (((iFunction < 0) && (string1.Compare((LPCTSTR)csValue) == 0)) ||
                ((iFunction > 0) && (string1.CompareNoCase((LPCTSTR)csValue) == 0)));
      } else {
        return false;
      }
    case -MR_NOTEND:
    case  MR_NOTEND:
      if (val_len > str_len) {
        csValue = csValue.Right(str_len);
        return (((iFunction < 0) && (string1.Compare((LPCTSTR)csValue) != 0)) ||
                ((iFunction > 0) && (string1.CompareNoCase((LPCTSTR)csValue) != 0)));
      } else
        return true;
    case -MR_CONTAINS:
      return (csValue.Find((LPCTSTR)string1) != -1);
    case  MR_CONTAINS:
    {
      csValue.MakeLower();
      CString subgroupLC(string1);
      subgroupLC.MakeLower();
      return (csValue.Find((LPCTSTR)subgroupLC) != -1);
    }
    case -MR_NOTCONTAIN:
      return (csValue.Find((LPCTSTR)string1)== -1);
    case  MR_NOTCONTAIN:
    {
      csValue.MakeLower();
      CString subgroupLC(string1);
      subgroupLC.MakeLower();
      return (csValue.Find((LPCTSTR)subgroupLC) == -1);
    }
    default:
      ASSERT(0);
  }

  return true; // should never get here!
}

bool PWSMatch::Matches(const int &num1, const int &num2, const int &iValue,
                      const int &iFunction)
{
  switch (iFunction) {
    case MR_EQUALS:
      return iValue == num1;
    case MR_NOTEQUAL:
      return iValue != num1;
    case MR_BETWEEN:
      return iValue >= num1 && iValue <= num2;
    case MR_LT:
      return iValue < num1;
    case MR_LE:
      return iValue <= num1;
    case MR_GT:
      return iValue > num1;
    case MR_GE:
      return iValue >= num1;
    default:
      ASSERT(0);
  }
  return false;
}

bool PWSMatch::Matches(const time_t &time1, const time_t &time2, const time_t &tValue,
                      const int &iFunction)
{
  switch (iFunction) {
    case MR_EQUALS:
      return tValue == time1;
    case MR_NOTEQUAL:
      return tValue != time1;
    case MR_BETWEEN:
      return tValue >= time1 && tValue <= time2;
    case MR_BEFORE:
      return tValue < time1;
    case MR_AFTER:
      return tValue > time1;
    default:
      ASSERT(0);
  }
  return false;
}

bool PWSMatch::Matches(const bool bValue, const int &iFunction)
{
  bool rc;

  if (bValue) {
    if (iFunction == MR_EQUALS ||
        iFunction == MR_ACTIVE ||
        iFunction == MR_PRESENT)
      rc = true;
    else
      rc = false;
  } else {
    if (iFunction == MR_NOTEQUAL ||
        iFunction == MR_INACTIVE ||
        iFunction == MR_NOTPRESENT)
      rc = true;
    else
      rc = false;
  }
  return rc;
}
