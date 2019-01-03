/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
  EXPECT_EQ(2, core.GetNumEntries());

  const CItemData al2 = core.GetEntry(core.Find(al.GetUUID()));

  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd;

  bool status = PWSAuxParse::GetEffectiveValues(&al2, &base,
                                               sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes,
                                               sx_url, sx_email, sx_autotype, sx_runcmd);
  EXPECT_TRUE(status);
  // Password should be from base:
  EXPECT_EQ(sx_pswd, base.GetPassword());
  // All the rest should be from alias:
  EXPECT_EQ(sx_group, al.GetGroup());
  EXPECT_EQ(sx_title, al.GetTitle());
  EXPECT_EQ(sx_user, al.GetUser());
  EXPECT_EQ(sx_lastpswd, L"");
  EXPECT_EQ(sx_notes, al.GetNotes());
  EXPECT_EQ(sx_url, al.GetURL());
  EXPECT_EQ(sx_email, al.GetEmail());
  EXPECT_EQ(sx_autotype, al.GetAutoType());
  EXPECT_EQ(sx_runcmd, al.GetRunCommand());
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
  EXPECT_EQ(2, core.GetNumEntries());

  const CItemData sc2 = core.GetEntry(core.Find(sc.GetUUID()));

  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd;

  bool status = PWSAuxParse::GetEffectiveValues(&sc2, &base,
                                               sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes,
                                               sx_url, sx_email, sx_autotype, sx_runcmd);
  EXPECT_TRUE(status);
  // Group, title and user should all be from sc:
  EXPECT_EQ(sx_group, sc.GetGroup());
  EXPECT_EQ(sx_title, sc.GetTitle());
  EXPECT_EQ(sx_user, sc.GetUser());
  // All the rest should be from base:
  EXPECT_EQ(sx_pswd, base.GetPassword());
  EXPECT_EQ(sx_lastpswd, L"");
  EXPECT_EQ(sx_notes, base.GetNotes());
  EXPECT_EQ(sx_url, base.GetURL());
  EXPECT_EQ(sx_email, base.GetEmail());
  EXPECT_EQ(sx_autotype, base.GetAutoType());
  EXPECT_EQ(sx_runcmd, base.GetRunCommand());
}
