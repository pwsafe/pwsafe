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
#include "./searchaction.h"

#include <vector>
#include <exception>

#include "../../core/Util.h"
#include "../../core/PWScore.h"

#include <assert.h>

#include "../wxWidgets/SearchUtils.h"

using namespace std;


void SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                      const Restriction &r, const CItemData::FieldBits &fieldsToSearch,
                      SearchAction &cb)
{
  assert( !searchText.empty() );
  
  CItemData::FieldBits fields = fieldsToSearch;
  if (fields.none())
    fields.set();

  ::FindMatches(std2stringx(searchText), ignoreCase, fields, r.valid(), r.value, r.field, r.rule, r.caseSensitive,
                core.GetEntryIter(), core.GetEntryEndIter(), get_second<ItemList>{}, [&cb](ItemListIter itr){
                  cb(itr->first, itr->second);
                });
}

int SaveAfterSearch(PWScore &core, const UserArgs &ua)
{
  if ( (ua.SearchAction == UserArgs::Update ||
        ua.SearchAction == UserArgs::Delete) && core.IsChanged() ) {
    return core.WriteCurFile();
  }
  return PWScore::SUCCESS;
}

int Search(PWScore &core, const UserArgs &ua)
{
  unique_ptr<SearchAction> sa(CreateSearchAction(ua.SearchAction, &core, ua));
  SearchForEntries(core, ua.opArg, ua.ignoreCase, ua.subset, ua.fields, *sa);
  return sa->Execute();
}
