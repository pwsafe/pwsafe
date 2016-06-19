//
//  search.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "./search.h"
#include "./argutils.h"

#include <string>
#include <vector>
#include <exception>
#include <regex>

#include "../../core/Util.h"
#include "../../core/PWScore.h"

#include <assert.h>

#include "../wxWidgets/SearchUtils.h"

using namespace std;

struct Restriction {
  CItemData::FieldType field;
  PWSMatch::MatchRule rule;
  wstring value;
  bool caseSensitive;
};

static std::vector<Restriction> ParseSearchedEntryRestrictions(const wstring &restrictToEntries);
static CItemData::FieldBits ParseFieldsToSearh(const wstring &fieldsToSearch);
static bool CaseSensitive(const wstring &str);

void SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                      const wstring &restrictToEntries, const wstring &fieldsToSearch,
                      SearchAction &cb)
{
  assert( !searchText.empty() );
  
  vector<Restriction> restrictions = ParseSearchedEntryRestrictions(restrictToEntries);
  
  if ( !restrictToEntries.empty() && restrictions.empty() ) {
    throw invalid_argument( "Could not parse [" + toutf8(restrictToEntries) + " ]for restricting searched entries" );
  }
  
  CItemData::FieldBits fields = ParseFieldsToSearh(fieldsToSearch);
  if (fieldsToSearch.empty())
    fields.set();
  if ( !fieldsToSearch.empty() && fields.none() ) {
    throw std::invalid_argument( "Could not parse [" + toutf8(fieldsToSearch) + " ]for restricting searched fields");
  }
  
  const Restriction dummy{ CItem::LAST_DATA, PWSMatch::MR_INVALID, std::wstring{}, true};
  const Restriction r = restrictions.size() > 0? restrictions[0]: dummy;
  
  ::FindMatches(std2stringx(searchText), ignoreCase, fields, restrictions.size() > 0, r.value, r.field, r.rule, r.caseSensitive,
                core.GetEntryIter(), core.GetEntryEndIter(), get_second<ItemList>{}, [&cb](ItemListIter itr){
                  cb(itr->first, itr->second);
                });
}

static std::vector<Restriction> ParseSearchedEntryRestrictions(const wstring &restrictToEntries)
{
  std::vector<Restriction> restrictions;
  if ( !restrictToEntries.empty() ) {
    std::wregex restrictPattern(L"([[:alpha:]-]+)([!]?[=^$~]=)([^;]+?)(/[iI])?(;|$)");
    std::wsregex_iterator pos(restrictToEntries.cbegin(), restrictToEntries.cend(), restrictPattern);
    std::wsregex_iterator end;
    for_each( pos, end, [&restrictions](const wsmatch &m) {
      restrictions.push_back( {String2FieldType(m.str(1)), Str2MatchRule(m.str(2)), m.str(3), CaseSensitive(m.str(4))} );
    });
  }
  return restrictions;
}

static CItemData::FieldBits ParseFieldsToSearh(const wstring &fieldsToSearch)
{
  CItemData::FieldBits fields;
  if ( !fieldsToSearch.empty() ) {
    Split(fieldsToSearch, L",", [&fields](const wstring &field) {
      CItemData::FieldType ft = String2FieldType(field);
      if (ft != CItemData::LAST_DATA)
        fields.set(ft);
      else
        throw std::invalid_argument("Could not parse field name: " + toutf8(field));
    });
  }
  return fields;
}

static bool CaseSensitive(const wstring &str)
{
  assert(str.length() == 0 || (str.length() == 2 && str[0] == '/' && (str[1] == L'i' || str[1] == 'I')));
  return str.length() == 0 || str[0] == L'i';
}

