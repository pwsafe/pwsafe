//
//  searchaction.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__searchaction__
#define __pwsafe_xcode6__searchaction__

#include <vector>

#include "../../core/PWScore.h"
#include "./argutils.h"

using ItemPtrVec = std::vector<const CItemData *>;
using FieldUpdates = UserArgs::FieldUpdates ;

int PrintSearchResults(const ItemPtrVec &items, PWScore &core, const CItemData::FieldBits &ftp, std::wostream &os);
int DeleteSearchResults(const ItemPtrVec &items, PWScore &core);
int UpdateSearchResults(const ItemPtrVec &items, PWScore &core, const FieldUpdates &updates);

template <int action>
struct SearchActionTraits
{};

template <>
struct SearchActionTraits<UserArgs::Print>
{
};

template <>
struct SearchActionTraits<UserArgs::Delete>
{
  static constexpr wchar_t prompt[] = L"Delete Item";
};

template <>
struct SearchActionTraits<UserArgs::Update>
{
  static constexpr wchar_t prompt[] = L"Update Item";
};

#endif /* defined(__pwsafe_xcode6__searchaction__) */
