/*
 * Created by Saurav Ghosh on 19/06/16.
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

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
int ClearFieldsOfSearchResults(const ItemPtrVec &items, PWScore &core, const CItemData::FieldBits &ftp);
int ChangePasswordOfSearchResults(const ItemPtrVec &items, PWScore &core);

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

template <>
struct SearchActionTraits<UserArgs::ClearFields>
{
  static constexpr wchar_t prompt[] = L"Clear files of item";
};

template <>
struct SearchActionTraits<UserArgs::ChangePassword>
{
  static constexpr wchar_t prompt[] = L"Change password of item";
};

#endif /* defined(__pwsafe_xcode6__searchaction__) */
