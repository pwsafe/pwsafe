//
//  SearchUtils.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 07/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__SearchUtils__
#define __pwsafe_xcode6__SearchUtils__

#include "../../core/ItemData.h"
#include "../../core/PWHistory.h"

// THIS CODE MUST BE KEPT FREE OF WXWIDGETS EVEN THOUGH ITS IN THE WXWIDGETS FOLDER
// BECAUSE IT'S ALSO USED BY PWSAFE-CLI WHICH DOES NOT USE WXWIDGETS


template <class Iter, class Accessor, class Callback>
void FindMatches(const StringX& searchText, bool fCaseSensitive,
                 const CItemData::FieldBits& bsFields, bool fUseSubgroups, const stringT& subgroupText,
                 CItemData::FieldType subgroupObject, PWSMatch::MatchRule subgroupFunction,
                 bool subgroupFunctionCaseSensitive, Iter begin, Iter end, Accessor afn, Callback cb)
{
  if (searchText.empty())
    return;

  typedef StringX (CItemData::*ItemDataFuncT)() const;

  struct {
    CItemData::FieldType type;
    ItemDataFuncT        func;
  } ItemDataFields[] = {  {CItemData::GROUP,     &CItemData::GetGroup},
    {CItemData::TITLE,     &CItemData::GetTitle},
    {CItemData::USER,      &CItemData::GetUser},
    {CItemData::PASSWORD,  &CItemData::GetPassword},
    //                        {CItemData::NOTES,     &CItemData::GetNotes},
    {CItemData::URL,       &CItemData::GetURL},
    {CItemData::EMAIL,     &CItemData::GetEmail},
    {CItemData::RUNCMD,    &CItemData::GetRunCommand},
    {CItemData::AUTOTYPE,  &CItemData::GetAutoType},
    {CItemData::XTIME_INT, &CItemData::GetXTimeInt},
  };

  bool keep_going = true;
  for ( Iter itr = begin; itr != end && keep_going; ++itr) {
    const int fn = (subgroupFunctionCaseSensitive? -subgroupFunction: subgroupFunction);
    if (fUseSubgroups && !afn(itr).Matches(stringT(subgroupText.c_str()), subgroupObject, fn))
      continue;

    bool found = false;
    for (size_t idx = 0; idx < NumberOf(ItemDataFields) && !found; ++idx) {
      if (bsFields.test(ItemDataFields[idx].type)) {
        const StringX str = (afn(itr).*ItemDataFields[idx].func)();
        found = fCaseSensitive? str.find(searchText) != StringX::npos: FindNoCase(searchText, str);
      }
    }

    if (!found && bsFields.test(CItemData::NOTES)) {
      StringX str = afn(itr).GetNotes();
      found = fCaseSensitive? str.find(searchText) != StringX::npos: FindNoCase(searchText, str);
    }

    if (!found && bsFields.test(CItemData::PWHIST)) {
      size_t pwh_max, err_num;
      PWHistList pwhistlist;
      CreatePWHistoryList(afn(itr).GetPWHistory(), pwh_max, err_num,
                          pwhistlist, PWSUtil::TMC_XML);
      for (PWHistList::iterator iter = pwhistlist.begin(); iter != pwhistlist.end(); iter++) {
        PWHistEntry pwshe = *iter;
        found = fCaseSensitive? pwshe.password.find(searchText) != StringX::npos: FindNoCase(searchText, pwshe.password );
        if (found)
          break;  // break out of for loop
      }
      pwhistlist.clear();
    }

    if (found) {
      cb(itr, &keep_going);
    }
  }
}


#endif /* defined(__pwsafe_xcode6__SearchUtils__) */
