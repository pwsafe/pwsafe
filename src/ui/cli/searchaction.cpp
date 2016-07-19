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

#include "./search.h"
#include "./argutils.h"

#include "../../core/PWScore.h"

using namespace std;

inline bool IsNullEntry(const CItemData &ci) { return !ci.HasUUID(); }

struct SearchAndPrint: public SearchAction
{
  virtual void operator()(const pws_os::CUUID &uuid, const CItemData &data) {
    wcout << st_GroupTitleUser{data.GetGroup(), data.GetTitle(), data.GetUser()} << endl;
  }
  virtual int Execute() { return PWScore::SUCCESS; }
};

struct SearchAndDelete: public SearchAction {
  PWScore *core;
  bool confirmed;
  
  SearchAndDelete(PWScore *core, bool conf): core{core}, confirmed{conf} {}
  CItemData found;
  virtual void operator()(const pws_os::CUUID &uuid, const CItemData &data) {
    if (!IsNullEntry(found))
      throw std::domain_error("Search matches multiple arguments. Operation aborted");
    found = data;
  };
  virtual int Execute() {
    if ( !IsNullEntry(found) ) {
      return core->Execute(DeleteEntryCommand::Create(core, found));
    }
    return PWScore::SUCCESS;
  };
};

struct SearchAndUpdate: public SearchAction {
  PWScore *core;
  using FieldValue = std::tuple<CItemData::FieldType, wstring>;
  using FieldUpdates = std::vector< FieldValue >;
  FieldUpdates updates;
  CItemData found;
  bool confirmed;
  FieldUpdates ParseFieldUpdates(const wstring &updates) {
    FieldUpdates u;
    Split(updates, L"[;,]", [&u](const wstring &nameval) {
      std::wsmatch m;
      if (std::regex_match(nameval, m, std::wregex(L"([^=:]+)[=:](.+)"))) {
        u.push_back( std::make_tuple(String2FieldType(m.str(1)), m.str(2)) );
      }
      else {
        wcerr << L"Could not parse field value to be updated: " << nameval << endl;
      }
    });
    return u;
  }
  SearchAndUpdate(PWScore *c, const std::wstring &actArgs, bool conf):
  core{c}, updates{ParseFieldUpdates(actArgs)}, confirmed{conf}
  {}
  int Execute() {
    if ( !IsNullEntry(found) ) {
      MultiCommands *mc = MultiCommands::Create(core);
      for_each(updates.begin(), updates.end(), [mc, this](const FieldValue &fv) {
        mc->Add( UpdateEntryCommand::Create( core, found, std::get<0>(fv), std2stringx(std::get<1>(fv))) );
      });
      return core->Execute(mc);
    }
    return PWScore::SUCCESS;
  }
  
  void operator()(const pws_os::CUUID &uuid, const CItemData &data) {
    if (!IsNullEntry(found))
      throw std::domain_error("Search matches multiple arguments. Operation aborted");
    found = data;
  }
};

SearchAction* CreateSearchAction(int action, PWScore *core, const wstring &actionArgs, bool confirmed)
{
  switch(action) {
    case UserArgs::Print:
      return new SearchAndPrint;
    case UserArgs::Delete:
      return new SearchAndDelete{core, confirmed};
    case UserArgs::Update:
      return new SearchAndUpdate{core, actionArgs, confirmed};
    default:
      throw std::logic_error{"unexpected search action type: " + tostr(action)};
  }
}
