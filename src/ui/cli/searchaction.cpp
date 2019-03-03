/*
 * Created by Saurav Ghosh on 19/06/16.
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "stdafx.h"
#include <string>

#include "./searchaction.h"
#include "./strutils.h"
#include "./safeutils.h"

#include "./argutils.h"

#include "../../core/PWScore.h"

using namespace std;

constexpr CItemData::FieldType known_fields[] = {
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

int ClearFieldsOfSearchResults(const ItemPtrVec &items, PWScore &core,
                                  const CItemData::FieldBits &ftp) {
  for( auto p: items ) {
    ItemListIter it = core.Find(p->GetUUID());
    for (auto ft : known_fields)
      if (ftp.test(ft)) it->second.ClearField(ft);
  }
  return PWScore::SUCCESS;
}

int ChangePasswordOfSearchResults(const ItemPtrVec &items, PWScore &core)
{
  // Helper class to (1) find the correct policy for an entry and (2) to
  // initialize the default policy only once & only if required
  class PWPolicyForEntry
  {
    PWScore *core;
    std::unique_ptr<PWPolicy> defpwp;

  public:
    PWPolicyForEntry(PWScore *c): core{c} {}
    PWPolicy Get(const CItemData *entry)
    {
      PWPolicy pwp;
      if ( entry->IsPolicyNameSet() ) {
        if ( core->GetPolicyFromName( entry->GetPolicyName(), pwp ) )
          return pwp;
      }

      if ( entry->IsPasswordPolicySet() ) {
        entry->GetPWPolicy(pwp);
        return pwp;
      }

      if ( !defpwp ) {
        defpwp.reset( new PWPolicy );
        auto initfn = [this]() {
          if ( InitPWPolicy(*defpwp, *core) != PWScore::SUCCESS )
              throw std::logic_error("Could not initialize default password policy");
        };
        initfn();
      };

      return *defpwp;
    }
  };

  PWPolicyForEntry pol(&core);
  for( auto p: items ) {
    auto it = core.Find(p->GetUUID());
    it->second.SetPassword( pol.Get(p).MakeRandomPassword() );
  }
  return PWScore::SUCCESS;
}

constexpr wchar_t *SearchActionTraits<UserArgs::Delete>::prompt;
constexpr wchar_t *SearchActionTraits<UserArgs::Update>::prompt;
constexpr wchar_t *SearchActionTraits<UserArgs::ClearFields>::prompt;
