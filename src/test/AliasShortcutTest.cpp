/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

class AliasShortcutTest : public ::testing::Test
{
protected:
  AliasShortcutTest() {}
  PWScore core;
  CItemData base;
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
  al.SetAlias();

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
  EXPECT_TRUE(sx_totpauthcode.empty());
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
  EXPECT_TRUE(sx_totpauthcode.empty());
}
