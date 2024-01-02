/*
 * Initial version created as 'SearchUtils.h'
 * by Saurav Ghosh on 07/06/16.
 * 
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SearchUtils.h
*
*/

#ifndef __SearchUtils_H
#define __SearchUtils_H

#include "ItemData.h"
#include "PWHistory.h"


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
      PWHistList pwhistlist(afn(itr).GetPWHistory(), PWSUtil::TMC_XML);
      for (PWHistList::iterator iter = pwhistlist.begin(); iter != pwhistlist.end(); iter++) {
        PWHistEntry pwshe = *iter;
        found = fCaseSensitive ? pwshe.password.find(searchText) != StringX::npos : FindNoCase(searchText, pwshe.password );
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


#endif /* defined(__SearchUtils_H) */
