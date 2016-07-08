//
//  argutils.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "argutils.h"
#include "./strutils.h"

#include <map>
#include <regex>

#include "../../core/PWScore.h"
#include "../../core/core.h"

using namespace std;

using String2FieldTypeMap = std::map<stringT, CItemData::FieldType>;

// Reverse of CItemData::FieldName
static String2FieldTypeMap  InitFieldTypeMap()
{
  String2FieldTypeMap ftmap;
  stringT retval;
  LoadAString(retval, IDSC_FLDNMGROUPTITLE);      ftmap[retval] = CItem::GROUPTITLE;
  LoadAString(retval, IDSC_FLDNMUUID);            ftmap[retval] = CItem::UUID;
  LoadAString(retval, IDSC_FLDNMGROUP);           ftmap[retval] = CItem::GROUP;
  LoadAString(retval, IDSC_FLDNMTITLE);           ftmap[retval] = CItem::TITLE;
  LoadAString(retval, IDSC_FLDNMUSERNAME);        ftmap[retval] = CItem::USER;
  LoadAString(retval, IDSC_FLDNMNOTES);           ftmap[retval] = CItem::NOTES;
  LoadAString(retval, IDSC_FLDNMPASSWORD);        ftmap[retval] = CItem::PASSWORD;
#undef CTIME
  LoadAString(retval, IDSC_FLDNMCTIME);           ftmap[retval] = CItem::CTIME;
  LoadAString(retval, IDSC_FLDNMPMTIME);          ftmap[retval] = CItem::PMTIME;
  LoadAString(retval, IDSC_FLDNMATIME);           ftmap[retval] = CItem::ATIME;
  LoadAString(retval, IDSC_FLDNMXTIME);           ftmap[retval] = CItem::XTIME;
  LoadAString(retval, IDSC_FLDNMRMTIME);          ftmap[retval] = CItem::RMTIME;
  LoadAString(retval, IDSC_FLDNMURL);             ftmap[retval] = CItem::URL;
  LoadAString(retval, IDSC_FLDNMAUTOTYPE);        ftmap[retval] = CItem::AUTOTYPE;
  LoadAString(retval, IDSC_FLDNMPWHISTORY);       ftmap[retval] = CItem::PWHIST;
  LoadAString(retval, IDSC_FLDNMPWPOLICY);        ftmap[retval] = CItem::POLICY;
  LoadAString(retval, IDSC_FLDNMXTIMEINT);        ftmap[retval] = CItem::XTIME_INT;
  LoadAString(retval, IDSC_FLDNMRUNCOMMAND);      ftmap[retval] = CItem::RUNCMD;
  LoadAString(retval, IDSC_FLDNMDCA);             ftmap[retval] = CItem::DCA;
  LoadAString(retval, IDSC_FLDNMSHIFTDCA);        ftmap[retval] = CItem::SHIFTDCA;
  LoadAString(retval, IDSC_FLDNMEMAIL);           ftmap[retval] = CItem::EMAIL;
  LoadAString(retval, IDSC_FLDNMPROTECTED);       ftmap[retval] = CItem::PROTECTED;
  LoadAString(retval, IDSC_FLDNMSYMBOLS);         ftmap[retval] = CItem::SYMBOLS;
  LoadAString(retval, IDSC_FLDNMPWPOLICYNAME);    ftmap[retval] = CItem::POLICYNAME;
  LoadAString(retval, IDSC_FLDNMKBSHORTCUT);      ftmap[retval] = CItem::KBSHORTCUT;
  return ftmap;;
}

CItemData::FieldType String2FieldType(const stringT& str)
{
  static const String2FieldTypeMap ftmap = InitFieldTypeMap();
  auto itr = ftmap.find(str);
  if (itr != ftmap.end())
    return itr->second;
  throw std::invalid_argument("Invalid field: " + toutf8(str));
}

PWSMatch::MatchRule Str2MatchRule( const wstring &s)
{
  static const std::map<wstring, PWSMatch::MatchRule> rulemap{
    {L"==", PWSMatch::MR_EQUALS},
    {L"!==", PWSMatch::MR_NOTEQUAL},
    {L"^=", PWSMatch::MR_BEGINS},
    {L"!^=", PWSMatch::MR_NOTBEGIN},
    {L"$=", PWSMatch::MR_ENDS},
    {L"!$=", PWSMatch::MR_NOTEND},
    {L"~=", PWSMatch::MR_CONTAINS},
    {L"!~=", PWSMatch::MR_NOTCONTAIN}
  };
  const auto itr = rulemap.find(s);
  if ( itr != rulemap.end() )
    return itr->second;
  return PWSMatch::MR_INVALID;
}

void UserArgs::SetFields(const wstring &f)
{
  fields.reset();
  Split(f, L",", [this](const wstring &field) {
    CItemData::FieldType ft = String2FieldType(field);
    fields.set(ft);
  });
}

inline bool CaseSensitive(const wstring &str)
{
  assert(str.length() == 0 || (str.length() == 2 && str[0] == '/' && (str[1] == L'i' || str[1] == 'I')));
  return str.length() == 0 || str[0] == L'i';
}


void UserArgs::SetSubset(const std::wstring &s)
{
  std::wregex restrictPattern(L"([[:alpha:]-]+)([!]?[=^$~]=)([^;]+?)(/[iI])?(;|$)");
  std::wsregex_iterator pos(s.cbegin(), s.cend(), restrictPattern);
  std::wsregex_iterator end;
  for_each( pos, end, [this](const wsmatch &m) {
    subset.push_back( {String2FieldType(m.str(1)), Str2MatchRule(m.str(2)), m.str(3), CaseSensitive(m.str(4))} );
  });
}
