/*
 * Created by Saurav Ghosh on 19/06/16.
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "stdafx.h"

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
  CItemData::TWOFACTORKEY,
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
  CItemData::KBSHORTCUT,
  CItemData::TOTPCONFIG,
  CItemData::TOTPLENGTH,
  CItemData::TOTPTIMESTEP,
  CItemData::TOTPSTARTTIME
};

int PrintSearchResults(const ItemPtrVec &items, PWScore &, const CItemData::FieldBits &ftp,
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

int GenerateTotpCodeForSearchResults(const ItemPtrVec& items, PWScore&, std::wostream& os, int verbosity_level) 
{
  PWScore::Status result = PWScore::SUCCESS;
  for_each(items.begin(), items.end(), [&os, &result, verbosity_level](const CItemData* p) {
    const CItemData& data = *p;
    if (data.GetTwoFactorKeyLength() == 0) {
      os << "TOTP key not found. Entry not configured for TOTP." << endl;
      result = PWScore::FAILURE;
      return;
    }
    uint8_t totp_time_step_seconds = data.GetTotpTimeStepSecondsAsByte();
    time_t totp_start_time = data.GetTotpStartTimeAsTimeT();
    time_t totp_time_now;
    uint32_t totp_auth_code;
    PWSTotp::TOTP_Result totpResult = PWSTotp::GetNextTotpAuthCode(data, totp_auth_code, &totp_time_now);
    if (verbosity_level > 0) {
      os << "TOTP Config: " << (int)data.GetTotpConfigAsByte() << endl;
      os << "TOTP Auth Code Length: " << (int)data.GetTotpLengthAsByte() << endl;
      os << "TOTP Time Step Seconds: " << (int)data.GetTotpTimeStepSecondsAsByte() << endl;
      os << "TOTP Start Time: " << data.GetTotpStartTimeAsTimeT() << endl;
    }
    if (totpResult != PWSTotp::Success) {
      os << "TOTP authentication code generation error." << endl
        << PWSTotp::GetTotpErrorString(totpResult) << " (TOTP Error=" << totpResult << ")" << endl;
      result = PWScore::FAILURE;
      return;
    }
    uint64_t seconds_remaining = totp_time_step_seconds - ((totp_time_now - totp_start_time) % totp_time_step_seconds);
    StringX totp_auth_code_str = PWSTotp::TotpCodeToString(data, totp_auth_code);
    os << "Authentication Code: " << totp_auth_code_str
       << " valid for approximately " << seconds_remaining
       << (seconds_remaining > 1 ? " seconds." : " second.")
       << endl;
   });

  return result;
}

constexpr const wchar_t *SearchActionTraits<UserArgs::Delete>::prompt;
constexpr const wchar_t *SearchActionTraits<UserArgs::Update>::prompt;
constexpr const wchar_t *SearchActionTraits<UserArgs::ClearFields>::prompt;
