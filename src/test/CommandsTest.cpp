/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Commands.cpp: Unit test for Commands

#if defined(WIN32) && !defined(__WX__)
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWScore.h"
#include "gtest/gtest.h"

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

  AddEntryCommand *pcmd = AddEntryCommand::Create(&core, di);
  
  core.Execute(pcmd);
  ItemListConstIter iter = core.Find(uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_EQ(di, core.GetEntry(iter));
  core.Undo();
  EXPECT_EQ(0, core.GetNumEntries());

  delete pcmd;
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

  time_t t;
  time(&t);
  si.SetCTime(t);
  si.SetXTime((time_t)0);
  si.SetStatus(CItemData::ES_ADDED);

  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, bi));
  pmulticmds->Add(AddEntryCommand::Create(&core, si, base_uuid));
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
  DeleteEntryCommand *pcmd1 = DeleteEntryCommand::Create(&core, bi2);

  core.Execute(pcmd1);
  EXPECT_EQ(0, core.GetNumEntries());

  core.Undo();
  EXPECT_EQ(2, core.GetNumEntries());

  // Now just delete the shortcut, check that
  // base is left, and that it reverts to a normal entry
  const CItemData si2 = core.GetEntry(core.Find(si.GetUUID())); // si2 has baseUUID set
  DeleteEntryCommand *pcmd2 = DeleteEntryCommand::Create(&core, si2);

  core.Execute(pcmd2);
  ASSERT_EQ(1, core.GetNumEntries());
  EXPECT_TRUE(core.GetEntry(core.Find(base_uuid)).IsNormal());

  // Get core to delete any existing commands
  core.ClearCommands();
}
