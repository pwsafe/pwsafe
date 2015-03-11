/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Commands.cpp: Unit test for Commands

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWScore.h"
#include "gtest/gtest.h"

// Since we use CItemData, we assume CItem::SetSessionKey()
// was called in the ItemData test environment setup

// A fixture for factoring common code across tests
class CommandsTest : public ::testing::Test
{
protected:
  CommandsTest() {}
  void SetUp() {}
  PWScore core;
};


// And now the tests...

TEST_F(CommandsTest, AddItem)
{
  CItemData di;
  di.CreateUUID();
  di.SetTitle(L"a title");
  di.SetPassword(L"a password");
  const pws_os::CUUID uuid = di.GetUUID();
  
  core.Execute(AddEntryCommand::Create(&core, di));
  ItemListConstIter iter = core.Find(uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_EQ(di, core.GetEntry(iter));
  core.Undo();
  EXPECT_EQ(0, core.GetNumEntries());
}

TEST_F(CommandsTest, CreateShortcutEntry)
{
  CItemData bi, si;
  bi.CreateUUID();
  bi.SetTitle(L"base entry");
  bi.SetPassword(L"base password");
  const pws_os::CUUID base_uuid = bi.GetUUID();

  si.SetTitle(L"shortcut to base");
  si.SetPassword(L"[Shortcut]");
  si.SetShortcut();
  si.CreateUUID(); // call after setting to shortcut!
  si.SetBaseUUID(base_uuid);

  time_t t;
  time(&t);
  si.SetCTime(t);
  si.SetXTime((time_t)0);
  si.SetStatus(CItemData::ES_ADDED);

  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, bi));
  pmulticmds->Add(AddEntryCommand::Create(&core, si));
  core.Execute(pmulticmds);
  EXPECT_EQ(2, core.GetNumEntries());

  // Check that the base entry is correctly marked
  ItemListConstIter iter = core.Find(base_uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_TRUE(core.GetEntry(iter).IsShortcutBase());

  core.Undo();
  EXPECT_EQ(0, core.GetNumEntries());
  core.Redo();
  EXPECT_EQ(2, core.GetNumEntries());

  // Delete base, expect both to be gone
  // Get base from core for correct type
  const CItemData bi2 = core.GetEntry(core.Find(base_uuid));
  core.Execute(DeleteEntryCommand::Create(&core, bi2));
  EXPECT_EQ(0, core.GetNumEntries());
  core.Undo();
  EXPECT_EQ(2, core.GetNumEntries());

  // Now just delete the shortcut, check that
  // base is left, and that it reverts to a normal entry
  core.Execute(DeleteEntryCommand::Create(&core, si));
  ASSERT_EQ(1, core.GetNumEntries());
  EXPECT_TRUE(core.GetEntry(core.Find(base_uuid)).IsNormal());
}
