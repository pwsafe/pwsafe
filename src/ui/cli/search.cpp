//
//  search.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "./search.h"
#include "./argutils.h"
#include "./strutils.h"

#include <string>
#include <vector>
#include <exception>

#include "../../core/Util.h"
#include "../../core/PWScore.h"

#include <assert.h>

#include "../wxWidgets/SearchUtils.h"

using namespace std;


void SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                      const std::vector<Restriction> &restrictions, const CItemData::FieldBits &fieldsToSearch,
                      SearchAction &cb)
{
  assert( !searchText.empty() );
  
  CItemData::FieldBits fields = fieldsToSearch;
  if (fields.none())
    fields.set();

  const Restriction dummy{ CItem::LAST_DATA, PWSMatch::MR_INVALID, std::wstring{}, true};
  const Restriction r = restrictions.size() > 0? restrictions[0]: dummy;
  
  ::FindMatches(std2stringx(searchText), ignoreCase, fields, restrictions.size() > 0, r.value, r.field, r.rule, r.caseSensitive,
                core.GetEntryIter(), core.GetEntryEndIter(), get_second<ItemList>{}, [&cb](ItemListIter itr){
                  cb(itr->first, itr->second);
                });
}
