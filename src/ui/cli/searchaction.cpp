//
//  searchaction.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include <string>

#include "./searchaction.h"
#include "./strutils.h"

#include "./argutils.h"

#include "../../core/PWScore.h"

using namespace std;

constexpr auto known_fields = {
  CItemData::GROUP,
  CItemData::TITLE,
  CItemData::USER,
  CItemData::NOTES,
  CItemData::PASSWORD,
  CItemData::CTIME,
  CItemData::PMTIME,
  CItemData::ATIME,
  CItemData::XTIME,
  CItemData::RMTIME,
  CItemData::URL,
  CItemData::PWHIST,
  CItemData::POLICY,
  CItemData::XTIME_INT,
  CItemData::RUNCMD,
  CItemData::DCA,
  CItemData::EMAIL,
  CItemData::PROTECTED,
  CItemData::SYMBOLS,
  CItemData::SHIFTDCA,
  CItemData::POLICYNAME,
  CItemData::KBSHORTCUT
};

int PrintSearchResults(const ItemPtrVec &items, PWScore &core, const CItemData::FieldBits &ftp,
                            std::wostream &os) {
  for_each( items.begin(), items.end(), [&ftp, &os](const CItemData *p) {
    const CItemData &data = *p;
    os << st_GroupTitleUser{data.GetGroup(), data.GetTitle(), data.GetUser()} << endl;
    for (auto ft : known_fields) {
      if (ftp.test(ft))
        os << data.FieldName(ft) << L": " << data.GetFieldValue(ft) << endl;
    }
  });
  return PWScore::SUCCESS;
}

int DeleteSearchResults(const ItemPtrVec &items, PWScore &core)
{
  if ( !items.empty() ) {
    MultiCommands *mc = MultiCommands::Create(&core);
    for_each(items.begin(), items.end(), [mc, &core](const CItemData *p) {
      mc->Add(DeleteEntryCommand::Create(&core, *p));
    });
    return core.Execute(mc);
  }
  return PWScore::SUCCESS;
};

using FieldValue = UserArgs::FieldValue;

int UpdateSearchResults(const ItemPtrVec &items, PWScore &core, const FieldUpdates &updates) {
  if ( !items.empty() ) {
    MultiCommands *mc{MultiCommands::Create(&core)};
    for_each(items.begin(), items.end(), [&core, mc, &updates](const CItemData *p) {
      for_each(updates.begin(), updates.end(), [mc, p, &core](const FieldValue &fv) {
        mc->Add( UpdateEntryCommand::Create(&core, *p, std::get<0>(fv), std::get<1>(fv)) );
      });
    });
    return core.Execute(mc);
  }
  return PWScore::SUCCESS;
}

constexpr wchar_t SearchActionTraits<UserArgs::Delete>::prompt[];
constexpr wchar_t SearchActionTraits<UserArgs::Update>::prompt[];
