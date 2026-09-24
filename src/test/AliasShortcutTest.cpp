/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AliasShortcutTest.cpp: Unit test for verifying semantics of alias and shortcut entries.

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWScore.h"
#include "core/PWSAuxParse.h"
#include "gtest/gtest.h"

#include <vector>

class AliasShortcutTest : public ::testing::Test
{
protected:
  AliasShortcutTest() {}
  PWScore core;
  CItemData base;
  const std::vector<CItemData::FieldType> passkey_fields = {
    CItemData::PASSKEY_CRED_ID,
    CItemData::PASSKEY_RP_ID,
    CItemData::PASSKEY_USER_HANDLE,
    CItemData::PASSKEY_ALGO_ID,
    CItemData::PASSKEY_PRIVATE_KEY,
    CItemData::PASSKEY_SIGN_COUNT
  };
  void SetUp();
  void TearDown();
};

void AliasShortcutTest::TearDown()
{
  // Get core to delete any existing commands
  core.ClearCommands();
}

void AliasShortcutTest::SetUp()
{
  base.CreateUUID();
  base.SetTitle(L"base");
  base.SetPassword(L"base-password");
  base.SetUser(L"base-user");
  base.SetNotes(L"base-notes");
  base.SetGroup(L"G");
  base.SetURL(L"http://base-url.com");
  base.SetAutoType(L"base-autotype");
  base.SetEmail(L"email@base.com");
  base.SetRunCommand(L"Run base, run");
  base.SetPasskeyCredentialID(VectorX<unsigned char>(64, 1));
  base.SetPasskeyRelyingPartyID(L"base-relying-party");
  base.SetPasskeyUserHandle(VectorX<unsigned char>(32, 2));
  base.SetPasskeyAlgorithmID(1);
  base.SetPasskeyPrivateKey(VectorX<unsigned char>(512, 3));
  base.SetPasskeySignCount(4);
  CustomField baseCustom;
  baseCustom.SetName(L"PIN");
  baseCustom.SetValue(L"base-pin");
  ASSERT_TRUE(base.AddCustomField(baseCustom));

  base.SetTwoFactorKey(L"YRUTW6JLVKRXEC7ZA7QMPXCGBSOO6HHT"); // valid secret
  base.SetTotpConfig(L"0");
  base.SetTotpStartTime(time_t(1000000));
  base.SetTotpTimeStep(L"30");
  base.SetTotpLength(L"6");
}

TEST_F(AliasShortcutTest, Alias)
{
  CItemData al;

  al.CreateUUID();
  al.SetTitle(L"alias");
  al.SetPassword(L"alias-password-not-used");
  al.SetUser(L"alias-user");
  al.SetNotes(L"alias-notes");
  al.SetGroup(L"Galias");
  al.SetURL(L"http://alias-url.com");
  al.SetAutoType(L"alias-autotype");
  al.SetEmail(L"email@alias.com");
  al.SetRunCommand(L"Run alias, run");
  al.SetPasskeyCredentialID(VectorX<unsigned char>(64, 10));
  al.SetPasskeyRelyingPartyID(L"alias-relying-party");
  al.SetPasskeyUserHandle(VectorX<unsigned char>(32, 11));
  al.SetPasskeyAlgorithmID(10);
  al.SetPasskeyPrivateKey(VectorX<unsigned char>(512, 12));
  al.SetPasskeySignCount(13);
  CustomField aliasCustom;
  aliasCustom.SetName(L"PIN");
  aliasCustom.SetValue(L"alias-pin");
  ASSERT_TRUE(al.AddCustomField(aliasCustom));
  al.SetAlias();
  al.SetTwoFactorKey(L"ODAVH3CHB2ZBAVON"); // different from base
  al.SetTotpConfig(L"0"); // must stay a valid (HMAC-SHA1) algorithm id
  al.SetTotpStartTime(time_t(2000000));
  al.SetTotpTimeStep(L"60");
  al.SetTotpLength(L"8");

  const pws_os::CUUID base_uuid = base.GetUUID();
  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, base));
  pmulticmds->Add(AddEntryCommand::Create(&core, al, base_uuid));
  core.Execute(pmulticmds);
  EXPECT_EQ(2U, core.GetNumEntries());

  const CItemData al2 = core.GetEntry(core.Find(al.GetUUID()));

  CItemData effci;
  StringX sx_lastpswd, sx_totpauthcode;

  PWSAuxParse::GetEffectiveValues(&al2, &base, effci, sx_lastpswd, sx_totpauthcode);

  // Password should be from base:
  EXPECT_EQ(effci.GetPassword(), base.GetPassword());
  // All the rest should be from alias:
  EXPECT_EQ(effci.GetGroup(), al.GetGroup());
  EXPECT_EQ(effci.GetTitle(), al.GetTitle());
  EXPECT_EQ(effci.GetUser(), al.GetUser());
  EXPECT_TRUE(sx_lastpswd.empty());
  EXPECT_EQ(effci.GetNotes(), al.GetNotes());
  EXPECT_EQ(effci.GetURL(), al.GetURL());
  EXPECT_EQ(effci.GetEmail(), al.GetEmail());
  EXPECT_EQ(effci.GetAutoType(), al.GetAutoType());
  EXPECT_EQ(effci.GetRunCommand(), al.GetRunCommand());
  EXPECT_EQ(effci.GetCustomFieldsRaw(), al.GetCustomFieldsRaw());
  // Alias has its own TOTP configuration, so it (not base's) is used:
  EXPECT_FALSE(sx_totpauthcode.empty());

  for (const CItemData::FieldType ft : passkey_fields) {
    EXPECT_EQ(base.GetEffectiveFieldValue(ft, nullptr),
              al2.GetEffectiveFieldValue(ft, &base));
  }

  // Alias has its own TOTP secret, so the credential entry used to
  // generate its auth code must be that, not the base's.
  const CItemData *totpItem = core.GetCredentialEntry(&al2);
  ASSERT_TRUE(totpItem != nullptr);
  EXPECT_EQ(totpItem->GetUUID(), al2.GetUUID());
  EXPECT_EQ(totpItem->GetTwoFactorKey(), al.GetTwoFactorKey());
}

TEST_F(AliasShortcutTest, AliasInheritsBaseTotpByDefault)
{
  CItemData al;

  al.CreateUUID();
  al.SetTitle(L"alias-no-own-totp");
  al.SetPassword(L"alias-password-not-used");
  al.SetAlias();

  const pws_os::CUUID base_uuid = base.GetUUID();
  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, base));
  pmulticmds->Add(AddEntryCommand::Create(&core, al, base_uuid));
  core.Execute(pmulticmds);
  EXPECT_EQ(2U, core.GetNumEntries());

  const CItemData al2 = core.GetEntry(core.Find(al.GetUUID()));
  ASSERT_FALSE(al2.HasTwoFactorKey());

  CItemData effci;
  StringX sx_lastpswd, sx_totpauthcode;

  PWSAuxParse::GetEffectiveValues(&al2, &base, effci, sx_lastpswd, sx_totpauthcode);

  // Alias has no TOTP configuration of its own, so base's is used:
  EXPECT_FALSE(sx_totpauthcode.empty());

  const CItemData *totpItem = core.GetCredentialEntry(&al2);
  ASSERT_TRUE(totpItem != nullptr);
  EXPECT_EQ(totpItem->GetUUID(), base.GetUUID());
  EXPECT_EQ(totpItem->GetTwoFactorKey(), base.GetTwoFactorKey());
}

TEST_F(AliasShortcutTest, Shortcut)
{
  CItemData sc;

  sc.SetTitle(L"shortcut");
  sc.SetUser(L"sc-user");
  sc.SetGroup(L"sc-group");
  sc.SetPassword(L"[Shortcut]");
  sc.SetShortcut();
  sc.CreateUUID(); // call after setting to shortcut!

  const pws_os::CUUID base_uuid = base.GetUUID();
  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, base));
  pmulticmds->Add(AddEntryCommand::Create(&core, sc, base_uuid));
  core.Execute(pmulticmds);
  EXPECT_EQ(2U, core.GetNumEntries());

  const CItemData sc2 = core.GetEntry(core.Find(sc.GetUUID()));

  CItemData effci;
  StringX sx_lastpswd, sx_totpauthcode;

  PWSAuxParse::GetEffectiveValues(&sc2, &base, effci, sx_lastpswd, sx_totpauthcode);

  // Group, title and user should all be from sc:
  EXPECT_EQ(effci.GetGroup(), sc.GetGroup());
  EXPECT_EQ(effci.GetTitle(), sc.GetTitle());
  EXPECT_EQ(effci.GetUser(), sc.GetUser());
  // All the rest should be from base:
  EXPECT_EQ(effci.GetPassword(), base.GetPassword());
  EXPECT_TRUE(sx_lastpswd.empty());
  EXPECT_EQ(effci.GetNotes(), base.GetNotes());
  EXPECT_EQ(effci.GetURL(), base.GetURL());
  EXPECT_EQ(effci.GetEmail(), base.GetEmail());
  EXPECT_EQ(effci.GetAutoType(), base.GetAutoType());
  EXPECT_EQ(effci.GetRunCommand(), base.GetRunCommand());
  EXPECT_EQ(effci.GetCustomFieldsRaw(), base.GetCustomFieldsRaw());

  for (const CItemData::FieldType ft : passkey_fields) {
    EXPECT_EQ(base.GetEffectiveFieldValue(ft, nullptr),
              sc2.GetEffectiveFieldValue(ft, &base));
  }

  // A shortcut always uses base's TOTP configuration:
  EXPECT_FALSE(sx_totpauthcode.empty());
  const CItemData *totpItem = core.GetCredentialEntry(&sc2);
  ASSERT_TRUE(totpItem != nullptr);
  EXPECT_EQ(totpItem->GetUUID(), base.GetUUID());
  EXPECT_EQ(totpItem->GetTwoFactorKey(), base.GetTwoFactorKey());
}
