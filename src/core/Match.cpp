/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file Match.cpp
//-----------------------------------------------------------------------------

#include "Match.h"
#include "ItemData.h"
#include "core.h"

#include "os/pws_tchar.h"

#include <time.h>

bool PWSMatch::Match(const StringX &stValue, StringX sx_Object,
                     const int &iFunction)
{
  // Note: sx_Object may be changed within this routine.

  const StringX::size_type val_len = stValue.length();
  const StringX::size_type obj_len = sx_Object.length();

  StringX sx_Object_lc(sx_Object);
  ToLower(sx_Object_lc);

  StringX stValue_lc(stValue);
  ToLower(stValue_lc);

  // Negative = Case   Sensitive
  // Positive = Case INsensitive
  switch (iFunction) {
    case -MR_EQUALS:
    case  MR_EQUALS:
      return ((obj_len == val_len) &&
             (((iFunction < 0) && (sx_Object == stValue)) ||
              ((iFunction > 0) && CompareNoCase(sx_Object, stValue) == 0)));
    case -MR_NOTEQUAL:
    case  MR_NOTEQUAL:
      return (((iFunction < 0) && (sx_Object != stValue)) ||
              ((iFunction > 0) && CompareNoCase(sx_Object, stValue) != 0));
    case -MR_BEGINS:
    case  MR_BEGINS:
      if (obj_len >= val_len) {
        StringX sx_Subset = sx_Object.substr(0, val_len);
        return (((iFunction < 0) && (stValue == sx_Subset)) ||
                ((iFunction > 0) && CompareNoCase(stValue, sx_Subset) == 0));
      } else {
        return false;
      }
    case -MR_NOTBEGIN:
    case  MR_NOTBEGIN:
      if (obj_len >= val_len) {
        StringX sx_Subset = sx_Object.substr(0, val_len);
        return (((iFunction < 0) && (stValue != sx_Subset)) ||
                ((iFunction > 0) && CompareNoCase(stValue, sx_Subset) != 0));
      } else {
        return true;
      }
    case -MR_ENDS:
    case  MR_ENDS:
      if (obj_len > val_len) {
        StringX sx_Subset = sx_Object.substr(obj_len - val_len);
        return (((iFunction < 0) && (stValue == sx_Subset)) ||
                ((iFunction > 0) && CompareNoCase(stValue, sx_Subset) == 0));
      } else {
        return false;
      }
    case -MR_NOTEND:
    case  MR_NOTEND:
      if (obj_len > val_len) {
        StringX sx_Subset = sx_Object.substr(obj_len - val_len);
        return (((iFunction < 0) && (stValue != sx_Subset)) ||
                ((iFunction > 0) && CompareNoCase(stValue, sx_Subset) != 0));
      } else
        return true;
    case -MR_CONTAINS:
      return (sx_Object.find(stValue) != StringX::npos);
    case  MR_CONTAINS:
      return (sx_Object_lc.find(stValue_lc) != StringX::npos);
    case -MR_NOTCONTAIN:
      return (sx_Object.find(stValue) == StringX::npos);
    case  MR_NOTCONTAIN:
      return (sx_Object_lc.find(stValue_lc) == StringX::npos);
    case -MR_CNTNANY:
    {
      const charT *c = stValue.c_str();
      for (size_t i = 0; i < stValue.length(); i++) {
        if (sx_Object.find(c, 0, 1) != StringX::npos)
          return true;
        c++;
      }
      return false;
    }
    case  MR_CNTNANY:
    {
      const charT *c = stValue_lc.c_str();
      for (size_t i = 0; i < stValue_lc.length(); i++) {
        if (sx_Object_lc.find(c, 0, 1) != StringX::npos)
          return true;
        c++;
      }
      return false;
    }
    case -MR_NOTCNTNANY:
    {
      const charT *c = stValue.c_str();
      for (size_t i = 0; i < stValue.length(); i++) {
        if (sx_Object.find(c, 0, 1) != StringX::npos)
          return false;
        c++;
      }
      return true;
    }
    case  MR_NOTCNTNANY:
    {
      const charT *c = stValue_lc.c_str();
      for (size_t i = 0; i < stValue_lc.length(); i++) {
        if (sx_Object_lc.find(c, 0, 1) != StringX::npos)
          return false;
        c++;
      }
      return true;
    }
    case -MR_CNTNALL:
    {
      const charT *c = stValue.c_str();
      for (size_t i = 0; i < stValue.length(); i++) {
        if (sx_Object.find(c, 0, 1) == StringX::npos)
          return false;
        c++;
      }
      return true;
    }
    case  MR_CNTNALL:
    {
      const charT *c = stValue_lc.c_str();
      for (size_t i = 0; i < stValue_lc.length(); i++) {
        if (sx_Object_lc.find(c, 0, 1) == StringX::npos)
          return false;
        c++;
      }
      return true;
    }
    case -MR_NOTCNTNALL:
    {
      const charT *c = stValue.c_str();
      for (size_t i = 0; i < stValue.length(); i++) {
        if (sx_Object.find(c, 0, 1) != StringX::npos)
          return false;
        c++;
      }
      return true;
    }
    case  MR_NOTCNTNALL:
    {
      const charT *c = stValue_lc.c_str();
      for (size_t i = 0; i < stValue_lc.length(); i++) {
        if (sx_Object_lc.find(c, 0, 1) != StringX::npos)
          return false;
        c++;
      }
      return true;
    }
    default:
      ASSERT(0);
  }

  return true; // should never get here!
}

bool PWSMatch::Match(const bool bValue, int iFunction)
{
  if (bValue) {
    return (iFunction == MR_EQUALS     ||
            iFunction == MR_ACTIVE     ||
            iFunction == MR_PRESENT    ||
            iFunction == MR_IS);
  } else {
    return (iFunction == MR_NOTEQUAL   ||
            iFunction == MR_INACTIVE   ||
            iFunction == MR_NOTPRESENT ||
            iFunction == MR_ISNOT);
  }
}

const char *PWSMatch::GetRuleString(const MatchRule rule)
{
  const char *pszrule = "  ";
  switch (rule) {
    case MR_INVALID:    pszrule = "  "; break;
    case MR_EQUALS:     pszrule = "EQ"; break;
    case MR_NOTEQUAL:   pszrule = "NE"; break;
    case MR_ACTIVE:     pszrule = "AC"; break;
    case MR_INACTIVE:   pszrule = "IA"; break;
    case MR_PRESENT:    pszrule = "PR"; break;
    case MR_NOTPRESENT: pszrule = "NP"; break;
    case MR_SET:        pszrule = "SE"; break;
    case MR_NOTSET:     pszrule = "NS"; break;
    case MR_IS:         pszrule = "IS"; break;
    case MR_ISNOT:      pszrule = "NI"; break;
    case MR_BEGINS:     pszrule = "BE"; break;
    case MR_NOTBEGIN:   pszrule = "NB"; break;
    case MR_ENDS:       pszrule = "EN"; break;
    case MR_NOTEND:     pszrule = "ND"; break;
    case MR_CONTAINS:   pszrule = "CO"; break;
    case MR_NOTCONTAIN: pszrule = "NC"; break;
    case MR_CNTNANY:    pszrule = "CY"; break;
    case MR_NOTCNTNANY: pszrule = "NY"; break;
    case MR_CNTNALL:    pszrule = "CA"; break;
    case MR_NOTCNTNALL: pszrule = "NA"; break;
    case MR_BETWEEN:    pszrule = "BT"; break;
    case MR_LT:         pszrule = "LT"; break;
    case MR_LE:         pszrule = "LE"; break;
    case MR_GT:         pszrule = "GT"; break;
    case MR_GE:         pszrule = "GE"; break;
    case MR_BEFORE:     pszrule = "BF"; break;
    case MR_AFTER:      pszrule = "AF"; break;
    case MR_EXPIRED:    pszrule = "EX"; break;  // Special Password rule
    case MR_WILLEXPIRE: pszrule = "WX"; break;  // Special Password rule
    default:
      ASSERT(0);
  }
  return pszrule;
}

UINT PWSMatch::GetRule(MatchRule rule)
{
  UINT id(0);
  if (rule < 0)
    rule = MatchRule(-rule);

  switch (rule) {
    case MR_INVALID:    id = IDSC_INVALID; break;
    case MR_EQUALS:     id = IDSC_EQUALS; break;
    case MR_NOTEQUAL:   id = IDSC_DOESNOTEQUAL; break;
    case MR_ACTIVE:     id = IDSC_ISACTIVE; break;
    case MR_INACTIVE:   id = IDSC_ISINACTIVE; break;
    case MR_PRESENT:    id = IDSC_ISPRESENT; break;
    case MR_NOTPRESENT: id = IDSC_ISNOTPRESENT; break;
    case MR_SET:        id = IDSC_SET; break;
    case MR_NOTSET:     id = IDSC_NOTSET; break;
    case MR_IS:         id = IDSC_IS; break;
    case MR_ISNOT:      id = IDSC_ISNOT; break;
    case MR_BEGINS:     id = IDSC_BEGINSWITH; break;
    case MR_NOTBEGIN:   id = IDSC_DOESNOTBEGINSWITH; break;
    case MR_ENDS:       id = IDSC_ENDSWITH; break;
    case MR_NOTEND:     id = IDSC_DOESNOTENDWITH; break;
    case MR_CONTAINS:   id = IDSC_CONTAINS; break;
    case MR_NOTCONTAIN: id = IDSC_DOESNOTCONTAIN; break;
    case MR_CNTNANY:    id = IDSC_CONTAINSANY; break;
    case MR_NOTCNTNANY: id = IDSC_DOESNOTCONTAINANY; break;
    case MR_CNTNALL:    id = IDSC_CONTAINSALL; break;
    case MR_NOTCNTNALL: id = IDSC_DOESNOTCONTAINALL; break;
    case MR_BETWEEN:    id = IDSC_BETWEEN; break;
    case MR_LT:         id = IDSC_LESSTHAN; break;
    case MR_LE:         id = IDSC_LESSTHANEQUAL; break;
    case MR_GT:         id = IDSC_GREATERTHAN; break;
    case MR_GE:         id = IDSC_GREATERTHANEQUAL; break;
    case MR_BEFORE:     id = IDSC_BEFORE; break;
    case MR_AFTER:      id = IDSC_AFTER; break;
    case MR_EXPIRED:    id = IDSC_EXPIRED; break;     // Special Password rule
    case MR_WILLEXPIRE: id = IDSC_WILLEXPIRE; break;  // Special Password rule
    default:
      ASSERT(0);
  }
  return id;
}

PWSMatch::MatchRule PWSMatch::GetRule(const StringX &sx_mnemonic)
{
  static const struct {
    const charT *mnemonic; PWSMatch::MatchRule mr;
  } table[] = {
    {_T("  "), MR_INVALID},
    {_T("EQ"), MR_EQUALS},
    {_T("NE"), MR_NOTEQUAL},
    {_T("AC"), MR_ACTIVE},
    {_T("IA"), MR_INACTIVE},
    {_T("PR"), MR_PRESENT},
    {_T("NP"), MR_NOTPRESENT},
    {_T("SE"), MR_SET},
    {_T("NS"), MR_NOTSET},
    {_T("IS"), MR_IS},
    {_T("NI"), MR_ISNOT},
    {_T("BE"), MR_BEGINS},
    {_T("NB"), MR_NOTBEGIN},
    {_T("EN"), MR_ENDS},
    {_T("ND"), MR_NOTEND},
    {_T("CO"), MR_CONTAINS},
    {_T("NC"), MR_NOTCONTAIN},
    {_T("CY"), MR_CNTNANY},
    {_T("NY"), MR_NOTCNTNANY},
    {_T("CA"), MR_CNTNALL},
    {_T("NA"), MR_NOTCNTNALL},
    {_T("BT"), MR_BETWEEN},
    {_T("LT"), MR_LT},
    {_T("LE"), MR_LE},
    {_T("GT"), MR_GT},
    {_T("GE"), MR_GE},
    {_T("BF"), MR_BEFORE},
    {_T("AF"), MR_AFTER},
    {_T("EX"), MR_EXPIRED},
    {_T("WX"), MR_WILLEXPIRE},
    {nullptr, MR_INVALID}
  };

  for (size_t i = 0; table[i].mnemonic != nullptr; i++)
    if (sx_mnemonic == table[i].mnemonic)
      return table[i].mr;

  ASSERT(0);
  return MR_INVALID;
}

void PWSMatch::GetMatchType(MatchType mtype,
                            int fnum1, int fnum2,
                            time_t fdate1, time_t fdate2, int fdatetype,
                            const stringT &fstring, bool fcase,
                            short fdca, int etype, int estatus, int funit,
                            bool bBetween, stringT &cs1, stringT &cs2)
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
      LoadAString(cs2, fcase ? IDSC_CASE_SENSITIVE : IDSC_CASE_INSENSITIVE);
      break;
    case MT_INTEGER:
      Format(cs1, L"%d", fnum1);
      if (bBetween)
        Format(cs2, L"%d", fnum2);
      break;
    case MT_DATE:
    {
      if (fdatetype == 1 /* Relative */) {
        Format(cs1, L"%d", fnum1);
        if (bBetween)
          Format(cs2, L"%d", fnum2);
      } else {
        struct tm st_s;
        errno_t err;
        err = localtime_s(&st_s, &fdate1);
        ASSERT(err == 0);
        if (!err) {
          TCHAR tc_buf1[80];
          _tcsftime(tc_buf1, sizeof(tc_buf1) / sizeof(tc_buf1[0]), _T("%x"), &st_s);
          cs1 = tc_buf1;
        }
        if (bBetween) {
          err = localtime_s(&st_s, &fdate2);
          ASSERT(err == 0);
          if (!err) {
            TCHAR tc_buf2[80];
            _tcsftime(tc_buf2, sizeof(tc_buf2) / sizeof(tc_buf2[0]), _T("%x"), &st_s);
            cs2 = tc_buf2;
          }
        }
      }
      break;
    }
    case MT_BOOL:
    case MT_PWHIST:
    case MT_POLICY:
    case MT_ATTACHMENT:
      break;
    case MT_ENTRYTYPE:
      switch (etype) {
        case CItemData::ET_NORMAL:       id = IDSC_FNORMAL;       break;
        case CItemData::ET_ALIASBASE:    id = IDSC_FALIASBASE;    break;
        case CItemData::ET_ALIAS:        id = IDSC_FALIAS;        break;
        case CItemData::ET_SHORTCUTBASE: id = IDSC_FSHORTCUTBASE; break;
        case CItemData::ET_SHORTCUT:     id = IDSC_FSHORTCUT;     break;
        default:
          ASSERT(0);
          id = IDSC_INVALID;
      }
      LoadAString(cs1, id);
      break;
    case MT_DCA:
    case MT_SHIFTDCA:
      switch (fdca) {
        case -1:                                        id = IDSC_CURRENTDEFAULTDCA;  break;
        case PWSprefs::DoubleClickCopyPassword:         id = IDSC_DCACOPYPASSWORD;    break;
        case PWSprefs::DoubleClickViewEdit:             id = IDSC_DCAVIEWEDIT;        break;
        case PWSprefs::DoubleClickAutoType:             id = IDSC_DCAAUTOTYPE;        break;
        case PWSprefs::DoubleClickBrowse:               id = IDSC_DCABROWSE;          break;
        case PWSprefs::DoubleClickCopyNotes:            id = IDSC_DCACOPYNOTES;       break;
        case PWSprefs::DoubleClickCopyUsername:         id = IDSC_DCACOPYUSERNAME;    break;
        case PWSprefs::DoubleClickCopyPasswordMinimize: id = IDSC_DCACOPYPASSWORDMIN; break;
        case PWSprefs::DoubleClickBrowsePlus:           id = IDSC_DCABROWSEPLUS;      break;
        case PWSprefs::DoubleClickRun:                  id = IDSC_DCARUN;             break;
        case PWSprefs::DoubleClickSendEmail:            id = IDSC_DCASENDEMAIL;       break;
        default:
          ASSERT(0);
          id = IDSC_INVALID;
      }
      LoadAString(cs1, id);
      if (fdca == -1) {
        // Fill in the current message with the default action
        short iDCA = (short)PWSprefs::GetInstance()->GetPref(mtype == MT_SHIFTDCA ?
          PWSprefs::ShiftDoubleClickAction : PWSprefs::DoubleClickAction);
        UINT id2;
        switch (iDCA) {
          case PWSprefs::DoubleClickCopyPassword:         id2 = IDSC_DCACOPYPASSWORD;    break;
          case PWSprefs::DoubleClickViewEdit:             id2 = IDSC_DCAVIEWEDIT;        break;
          case PWSprefs::DoubleClickAutoType:             id2 = IDSC_DCAAUTOTYPE;        break;
          case PWSprefs::DoubleClickBrowse:               id2 = IDSC_DCABROWSE;          break;
          case PWSprefs::DoubleClickCopyNotes:            id2 = IDSC_DCACOPYNOTES;       break;
          case PWSprefs::DoubleClickCopyUsername:         id2 = IDSC_DCACOPYUSERNAME;    break;
          case PWSprefs::DoubleClickCopyPasswordMinimize: id2 = IDSC_DCACOPYPASSWORDMIN; break;
          case PWSprefs::DoubleClickBrowsePlus:           id2 = IDSC_DCABROWSEPLUS;      break;
          case PWSprefs::DoubleClickRun:                  id2 = IDSC_DCARUN;             break;
          case PWSprefs::DoubleClickSendEmail:            id2 = IDSC_DCASENDEMAIL;       break;
          default:
            ASSERT(0);
            id2 = IDSC_INVALID;
        }
        stringT cs3;
        LoadAString(cs3, id2);
        Format(cs1, cs1.c_str(), cs3.c_str());
      }
      break;
    case MT_ENTRYSTATUS:
      switch (estatus) {
        case CItemData::ES_CLEAN:        id = IDSC_FSCLEAN;        break;
        case CItemData::ES_ADDED :       id = IDSC_FSADDED;        break;
        case CItemData::ES_MODIFIED:     id = IDSC_FSMODIFIED;     break;
        default:
          ASSERT(0);
          id = IDSC_INVALID;
      }
      LoadAString(cs1, id);
      break;
    case MT_ENTRYSIZE:
      {
        Format(cs1, L"%d", fnum1 >> (funit * 10));
        if (bBetween)
          Format(cs2, L"%d", fnum2 >> (funit * 10));
      }
      break;
    case MT_MEDIATYPE:
      cs1 = fstring;
      break;
    default:
      ASSERT(0);
  }
}
