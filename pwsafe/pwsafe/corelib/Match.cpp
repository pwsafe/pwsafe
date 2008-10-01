/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file Match.cpp
//-----------------------------------------------------------------------------

#include "Match.h"
#include "ItemData.h"
#include "corelib.h"
#include <time.h>

bool PWSMatch::Match(const StringX &string1, StringX &csValue,
                     int iFunction)
{
  const StringX::size_type str_len = string1.length();
  const StringX::size_type val_len = csValue.length();

  // Negative = Case   Sensitive
  // Positive = Case INsensitive
  switch (iFunction) {
    case -MR_EQUALS:
    case  MR_EQUALS:
      return ((val_len == str_len) &&
             (((iFunction < 0) && (csValue == string1)) ||
              ((iFunction > 0) && CompareNoCase(csValue, string1) == 0)));
    case -MR_NOTEQUAL:
    case  MR_NOTEQUAL:
      return (((iFunction < 0) && (csValue != string1)) ||
              ((iFunction > 0) && CompareNoCase(csValue, string1) != 0));
    case -MR_BEGINS:
    case  MR_BEGINS:
      if (val_len >= str_len) {
        csValue = csValue.substr(0, str_len);
        return (((iFunction < 0) && (string1 == csValue)) ||
                ((iFunction > 0) && CompareNoCase(string1, csValue) == 0));
      } else {
        return false;
      }
    case -MR_NOTBEGIN:
    case  MR_NOTBEGIN:
      if (val_len >= str_len) {
        csValue = csValue.substr(0, str_len);
        return (((iFunction < 0) && (string1 != csValue)) ||
                ((iFunction > 0) && CompareNoCase(string1, csValue) != 0));
      } else {
        return false;
      }
    case -MR_ENDS:
    case  MR_ENDS:
      if (val_len > str_len) {
        csValue = csValue.substr(val_len - str_len);
        return (((iFunction < 0) && (string1 == csValue)) ||
                ((iFunction > 0) && CompareNoCase(string1, csValue) == 0));
      } else {
        return false;
      }
    case -MR_NOTEND:
    case  MR_NOTEND:
      if (val_len > str_len) {
        csValue = csValue.substr(val_len - str_len);
        return (((iFunction < 0) && (string1 != csValue)) ||
                ((iFunction > 0) && CompareNoCase(string1, csValue)!= 0));
      } else
        return true;
    case -MR_CONTAINS:
      return (csValue.find(string1) != StringX::npos);
    case  MR_CONTAINS:
    {
      ToLower(csValue);
      StringX subgroupLC(string1);
      ToLower(subgroupLC);
      return (csValue.find(subgroupLC) != -1);
    }
    case -MR_NOTCONTAIN:
      return (csValue.find(string1)== StringX::npos);
    case  MR_NOTCONTAIN:
    {
      ToLower(csValue);
      StringX subgroupLC(string1);
      ToLower(subgroupLC);
      return (csValue.find(subgroupLC) == StringX::npos);
    }
    default:
      ASSERT(0);
  }

  return true; // should never get here!
}

bool PWSMatch::Match(const bool bValue, int iFunction)
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

char * PWSMatch::GetRuleString(MatchRule rule)
{
  char * pszrule = "  ";
  switch (rule) {
    case MR_INVALID: pszrule = "  "; break;
    case MR_EQUALS: pszrule = "EQ"; break;
    case MR_NOTEQUAL: pszrule = "NE"; break;
    case MR_ACTIVE: pszrule = "AC"; break;
    case MR_INACTIVE: pszrule = "IA"; break;
    case MR_PRESENT: pszrule = "PR"; break;
    case MR_NOTPRESENT: pszrule = "NP"; break;
    case MR_SET: pszrule = "SE"; break;
    case MR_NOTSET: pszrule = "NS"; break;
    case MR_IS: pszrule = "IS"; break;
    case MR_ISNOT: pszrule = "NI"; break;
    case MR_BEGINS: pszrule = "BE"; break;
    case MR_NOTBEGIN: pszrule = "NB"; break;
    case MR_ENDS: pszrule = "EN"; break;
    case MR_NOTEND: pszrule = "ND"; break;
    case MR_CONTAINS: pszrule = "CO"; break;
    case MR_NOTCONTAIN: pszrule = "NC"; break;
    case MR_BETWEEN: pszrule = "BT"; break;
    case MR_LT: pszrule = "LT"; break;
    case MR_LE: pszrule = "LE"; break;
    case MR_GT: pszrule = "GT"; break;
    case MR_GE: pszrule = "GE"; break;
    case MR_BEFORE: pszrule = "BF"; break;
    case MR_AFTER: pszrule = "AF"; break;
    case MR_EXPIRED: pszrule = "EX"; break;  // Special Password rule
    case MR_WILLEXPIRE: pszrule = "WX"; break;  // Special Password rule
    default:
      ASSERT(0);
  }
  return pszrule;
}

UINT PWSMatch::GetRule(MatchRule rule)
{
  UINT id(0);
  switch (rule) {
    case MR_INVALID: id = IDSC_INVALID; break;
    case MR_EQUALS: id = IDSC_EQUALS; break;
    case MR_NOTEQUAL: id = IDSC_DOESNOTEQUAL; break;
    case MR_ACTIVE: id = IDSC_ISACTIVE; break;
    case MR_INACTIVE: id = IDSC_ISINACTIVE; break;
    case MR_PRESENT: id = IDSC_ISPRESENT; break;
    case MR_NOTPRESENT: id = IDSC_ISNOTPRESENT; break;
    case MR_SET: id = IDSC_SET; break;
    case MR_NOTSET: id = IDSC_NOTSET; break;
    case MR_IS: id = IDSC_IS; break;
    case MR_ISNOT: id = IDSC_ISNOT; break;
    case MR_BEGINS: id = IDSC_BEGINSWITH; break;
    case MR_NOTBEGIN: id = IDSC_DOESNOTBEGINSWITH; break;
    case MR_ENDS: id = IDSC_ENDSWITH; break;
    case MR_NOTEND: id = IDSC_DOESNOTENDWITH; break;
    case MR_CONTAINS: id = IDSC_CONTAINS; break;
    case MR_NOTCONTAIN: id = IDSC_DOESNOTCONTAIN; break;
    case MR_BETWEEN: id = IDSC_BETWEEN; break;
    case MR_LT: id = IDSC_LESSTHAN; break;
    case MR_LE: id = IDSC_LESSTHANEQUAL; break;
    case MR_GT: id = IDSC_GREATERTHAN; break;
    case MR_GE: id = IDSC_GREATERTHANEQUAL; break;
    case MR_BEFORE: id = IDSC_BEFORE; break;
    case MR_AFTER: id = IDSC_AFTER; break;
    case MR_EXPIRED: id = IDSC_EXPIRED; break;  // Special Password rule
    case MR_WILLEXPIRE: id = IDSC_WILLEXPIRE; break;  // Special Password rule
    default:
      ASSERT(0);
  }
  return id;
}

void PWSMatch::GetMatchType(MatchType mtype,
                            int fnum1, int fnum2,
                            time_t fdate1, time_t fdate2,
                            const stringT &fstring, int fcase,
                            int etype, bool bBetween,
                            stringT &cs1, stringT &cs2)
{
  cs1 = cs2 = _T("");
  UINT id(0);

  switch (mtype) {
    case MT_INVALID:
      LoadAString(cs1, IDSC_INVALID);
      break;
    case MT_PASSWORD:
      if (fnum1 > 0) {
        Format(cs1, IDSC_EXPIRE_IN_DAYS, fnum1);
        break;
      }
      // Note: purpose drop through to standard 'string' processing
    case MT_STRING:
      cs1 = fstring;
      LoadAString(cs2,
                  fcase == 0 ? IDSC_CASE_INSENSITIVE : IDSC_CASE_SENSITIVE);
      break;
    case MT_INTEGER:
      Format(cs1, _T("%d"), fnum1);
      if (bBetween)
        Format(cs2, _T("%d"), fnum2);
      break;
    case MT_DATE:
      {
        struct tm st_s;
        errno_t err;
        err = _localtime32_s(&st_s, &fdate1);
        ASSERT(err == 0);
        TCHAR tc_buf1[80];
        _tcsftime(tc_buf1, sizeof(tc_buf1) / sizeof(tc_buf1[0]), _T("%x"), &st_s);
        cs1 = tc_buf1;
        if (bBetween) {
          err = _localtime32_s(&st_s, &fdate2);
          ASSERT(err == 0);
          TCHAR tc_buf2[80];
          _tcsftime(tc_buf2, sizeof(tc_buf2) / sizeof(tc_buf2[0]), _T("%x"), &st_s);
          cs2 = tc_buf2;
        }
      }
      break;
    case MT_BOOL:
    case MT_PWHIST:
    case MT_POLICY:
      break;
    case MT_ENTRYTYPE:
      switch (etype) {
        case CItemData::ET_NORMAL: id = IDSC_FNORMAL; break;
        case CItemData::ET_ALIASBASE: id = IDSC_FALIASBASE; break;
        case CItemData::ET_ALIAS: id = IDSC_FALIAS; break;
        case CItemData::ET_SHORTCUTBASE: id = IDSC_FSHORTCUTBASE; break;
        case CItemData::ET_SHORTCUT: id = IDSC_FSHORTCUT; break;
        default:
          ASSERT(0);
          id = IDSC_INVALID;
      }
      LoadAString(cs1, id);
      break;
    default:
      ASSERT(0);
  }
}
