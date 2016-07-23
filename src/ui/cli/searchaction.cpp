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

void SearchAction::OnMatch(const pws_os::CUUID &uuid, const CItemData &data)
{
  itemids.push_back(&data);
}

struct SearchAndPrint: public SearchAction
{
  CItemData::FieldBits ftp; // fields to print
  SearchAndPrint(const wstring &f): SearchAction(true), ftp{ParseFields(f)}
  {}
  virtual int Execute() {
    for_each( itemids.begin(), itemids.end(), [this](const CItemData *p) {
      const CItemData &data = *p;
      wcout << st_GroupTitleUser{data.GetGroup(), data.GetTitle(), data.GetUser()} << endl;
      for (auto ft : known_fields) {
        if (ftp.test(ft))
          wcout << data.FieldName(ft) << L": " << data.GetFieldValue(ft) << endl;
      }
    });
    return PWScore::SUCCESS;
  }
};

struct SearchAndDelete: public SearchAction {
  PWScore *core;
  
  SearchAndDelete(PWScore *core, bool conf): SearchAction(conf), core{core}
  {}
  virtual int Execute() {
    if ( !itemids.empty() ) {
      MultiCommands *mc = MultiCommands::Create(core);
      for_each(itemids.begin(), itemids.end(), [this, mc](const CItemData *p) {
        mc->Add(DeleteEntryCommand::Create(core, *p));
      });
      return core->Execute(mc);
    }
    return PWScore::SUCCESS;
  };
};

struct SearchAndUpdate: public SearchAction {
  using FieldUpdates = UserArgs::FieldUpdates ;
  using FieldValue = UserArgs::FieldValue;

  PWScore *core;
  FieldUpdates updates;
  SearchAndUpdate(PWScore *c, const FieldUpdates &u, bool conf):
  SearchAction(conf), core{c}, updates{u}
  {}
  int Execute() {
    if ( !itemids.empty() ) {
      MultiCommands *mc{MultiCommands::Create(core)};
      for_each(itemids.begin(), itemids.end(), [this, mc](const CItemData *p) {
        for_each(updates.begin(), updates.end(), [mc, p, this](const FieldValue &fv) {
          mc->Add( UpdateEntryCommand::Create(core, *p, std::get<0>(fv), std::get<1>(fv)) );
        });
      });
      return core->Execute(mc);
    }
    return PWScore::SUCCESS;
  }
};

SearchAction* CreateSearchAction(int action, PWScore *core, const UserArgs &ua)
{
  switch(action) {
    case UserArgs::Print:
      return new SearchAndPrint(ua.opArg2);
    case UserArgs::Delete:
      return new SearchAndDelete{core, ua.confirmed};
    case UserArgs::Update:
      return new SearchAndUpdate{core, ua.fieldValues, ua.confirmed};
    default:
      throw std::logic_error{"unexpected search action type: " + tostr(action)};
  }
}
