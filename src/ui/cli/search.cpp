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


using CbType = function<void(const pws_os::CUUID &, const CItemData &, bool *)>;
void SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                      const Restriction &r, const CItemData::FieldBits &fieldsToSearch,
                      CbType cb)
{
  assert( !searchText.empty() );
  
  CItemData::FieldBits fields = fieldsToSearch;
  if (fields.none())
    fields.set();

  ::FindMatches(std2stringx(searchText), ignoreCase, fields, r.valid(), r.value, r.field, r.rule, r.caseSensitive,
                core.GetEntryIter(), core.GetEntryEndIter(), get_second<ItemList>{},
                  [&cb](ItemListIter itr, bool *keep_going){
                  cb(itr->first, itr->second, keep_going);
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


wchar_t Confirm(const wstring &prompt, const wstring &ops,
            const wstring &help, const CItemData &item)
{
  wstring options{ops};
  options += L"p?";
  wchar_t choice{};
  do {
    wcout << st_GroupTitleUser{item.GetGroup(), item.GetTitle(), item.GetUser()} << endl;
    wcout << prompt << L" [" << options << L"]? ";
    wcin >> choice;
    switch( choice ) {
      case L'p':
      {
        auto fields = {CItem::GROUP, CItem::TITLE, CItem::USER,
                       CItem::EMAIL, CItem::URL, CItem::AUTOTYPE};
        for(auto f: fields)
          wcout << item.FieldName(f) << L": " << item.GetFieldValue(f) << endl;
        choice = 0;
        break;
      }
      case L'?':
        wcout << help << L"[p]rint - print all fields for this item" << endl
                      << L"[?}     - print this help message" << endl;
        choice = 0;
        break;
      default:
        if (ops.find(choice) != wstring::npos) return choice;
        wcerr << L"Huh (" << choice << L")?" << endl;
        choice = 0;
        break;
    }
  } while( !choice );
  return choice;
}

int Search(PWScore &core, const UserArgs &ua)
{
  unique_ptr<SearchAction> sa(CreateSearchAction(ua.SearchAction, &core, ua));

  const wchar_t help[] = L"[y]es   - yes for this item\n"
                          "[n]o    - no for this item\n"
                          "[a]ll   - yes for this item and all remaining items\n"
                          "[q]uit  - no for this item all remaining items\n"
                          "a[b]ort - abort operation, even for previous items\n";
  wchar_t choice{sa->confirmed? L'a': 0};
  SearchForEntries(core, ua.opArg, ua.ignoreCase, ua.subset, ua.fields,
      [&sa, &choice, help](const pws_os::CUUID &uuid,
                           const CItemData &data,
                           bool *keep_going) {

    if( choice != L'a' )
      choice = Confirm(L"Update this record", L"ynaqb", help, data);

    switch(choice) {
      case L'y': case L'a':
        sa->OnMatch(uuid, data);
        break;
      case L'n':
        break;
      case L'q': case L'b':
        *keep_going = false;
        break;
      default:
        assert(false);
        break;
    }

  });

  if (choice != L'b')
    return sa->Execute();

  return PWScore::SUCCESS;
}
