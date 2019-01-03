/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "core/PWSfileV3.h"
#include "core/PWHistory.h"
#include "os/file.h"

#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class CommandsTest : public ::testing::Test
{
protected:
  CommandsTest() {}
};

// And now the tests...

TEST_F(CommandsTest, AddItem)
{
  PWScore core;
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
  EXPECT_TRUE(core.HasDBChanged());

  core.Undo();
  EXPECT_FALSE(core.HasDBChanged());
  EXPECT_EQ(0, core.GetNumEntries());

  // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest, CreateShortcutEntry)
{
  PWScore core;
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
  EXPECT_TRUE(core.HasDBChanged());

  // Check that the base entry is correctly marked
  ItemListConstIter iter = core.Find(base_uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_TRUE(core.GetEntry(iter).IsShortcutBase());

  core.Undo();
  EXPECT_EQ(0, core.GetNumEntries());
  EXPECT_FALSE(core.HasDBChanged());

  core.Redo();
  EXPECT_EQ(2, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  // Delete base, expect both to be gone
  // Get base from core for correct type
  const CItemData bi2 = core.GetEntry(core.Find(base_uuid));
  DeleteEntryCommand *pcmd1 = DeleteEntryCommand::Create(&core, bi2);

  core.Execute(pcmd1);
  EXPECT_EQ(0, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  core.Undo();
  EXPECT_EQ(2, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  // Now just delete the shortcut, check that
  // base is left, and that it reverts to a normal entry
  const CItemData si2 = core.GetEntry(core.Find(si.GetUUID())); // si2 has baseUUID set
  DeleteEntryCommand *pcmd2 = DeleteEntryCommand::Create(&core, si2);

  core.Execute(pcmd2);
  ASSERT_EQ(1, core.GetNumEntries());
  EXPECT_TRUE(core.GetEntry(core.Find(base_uuid)).IsNormal());
  EXPECT_TRUE(core.HasDBChanged());

  // Get core to delete any existing commands
  core.ClearCommands();
  EXPECT_TRUE(core.HasDBChanged());
}

TEST_F(CommandsTest, EditEntry)
{
  PWScore core;
  CItemData it;
  it.CreateUUID();
  it.SetTitle(L"NoDrama");
  it.SetPassword(L"PolishTrumpetsSq4are");

  Command *pcmd = AddEntryCommand::Create(&core, it);
  core.Execute(pcmd);
  EXPECT_TRUE(core.HasDBChanged());

  ItemListConstIter iter = core.Find(it.GetUUID());
  ASSERT_NE(core.GetEntryEndIter(), iter);
  CItemData it2(core.GetEntry(iter));
  EXPECT_EQ(it, it2);

  it2.SetTitle(L"NoDramamine");
  pcmd = EditEntryCommand::Create(&core, it, it2);
  core.Execute(pcmd);
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  EXPECT_EQ(core.GetEntry(iter).GetTitle(), it2.GetTitle());
  core.Undo();
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  EXPECT_EQ(core.GetEntry(iter).GetTitle(), it.GetTitle());
  core.Undo();
  EXPECT_EQ(core.GetNumEntries(), 0);
  EXPECT_FALSE(core.HasDBChanged());

  core.Redo();
  EXPECT_TRUE(core.HasDBChanged());
  EXPECT_EQ(core.GetNumEntries(), 1);

  // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest, RenameGroup)
{
  PWScore core;
  CItemData di;
  di.CreateUUID();
  di.SetTitle(L"b title");
  di.SetPassword(L"b password");
  di.SetGroup(L"Group0.Alpha");
  const pws_os::CUUID uuid = di.GetUUID();

  Command *pcmd = AddEntryCommand::Create(&core, di);
  
  core.Execute(pcmd);
  ItemListConstIter iter = core.Find(uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_EQ(di, core.GetEntry(iter));
  EXPECT_TRUE(core.HasDBChanged());
  
  pcmd = RenameGroupCommand::Create(&core, L"Group0.Alpha", L"Group0.Beta");
  core.Execute(pcmd);

  iter = core.Find(uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_EQ(core.GetEntry(iter).GetGroup(), L"Group0.Beta");
  core.Undo();

  iter = core.Find(uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_EQ(core.GetEntry(iter).GetGroup(), L"Group0.Alpha");

  // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest, CountGroups)
{
  PWScore core;
  CItemData di;
  std::vector<stringT> vGroups;

  di.CreateUUID();
  di.SetTitle(L"b title");
  di.SetPassword(L"b password");
  const pws_os::CUUID uuid = di.GetUUID();

  Command *pcmd = AddEntryCommand::Create(&core, di);  
  core.Execute(pcmd);

  core.GetAllGroups(vGroups);
  EXPECT_TRUE(vGroups.empty());

  auto iter = core.Find(di.GetUUID());
  CItemData di2 = core.GetEntry(iter);
  di2.SetGroup(L"g1");
  pcmd = EditEntryCommand::Create(&core, di, di2);
  core.Execute(pcmd);

  core.GetAllGroups(vGroups);
  EXPECT_EQ(1, vGroups.size());

  iter = core.Find(di.GetUUID());
  di = core.GetEntry(iter);
  di.SetGroup(L"g1.g1-1");
  pcmd = EditEntryCommand::Create(&core, di2, di);
  core.Execute(pcmd);

  core.GetAllGroups(vGroups);
  EXPECT_EQ(2, vGroups.size());

  std::vector<StringX> eg;
  eg.push_back(L"e1");
  pcmd = DBEmptyGroupsCommand::Create(&core, eg, DBEmptyGroupsCommand::EG_ADDALL);
  core.Execute(pcmd);

  core.GetAllGroups(vGroups);
  EXPECT_EQ(3, vGroups.size());

  // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest, UpdatePassword)
{
  PWScore core;
  size_t pwh_max, num_err;
  PWHistList pwhl;

  const stringT fname(L"UpdPWTest.psafe3");
  const StringX passphrase(L"WhyAmIDoingThis?");
  const int32 i1day = 86400; // 24 * 60 * 60 seconds
  const StringX sxOldPassword(L"MoreWideF1ns");
  const StringX sxNewPassword(L"ManifestQuin1ne");

  core.SetCurFile(fname.c_str());
  core.NewFile(passphrase);

  CItemData it;
  it.CreateUUID();
  time_t t, tPMtime;
  time(&t);
  tPMtime = t - i1day;
  it.SetCTime(t);
  it.SetTitle(L"KarmaKiller");
  it.SetPassword(sxOldPassword);
  it.SetPWHistory(L"10300");  // On and save 3
  it.SetPMTime(tPMtime);       // Say password set yesterday
  it.SetXTimeInt(10);
  it.SetXTime(t - i1day * 2); // Say expired 2 days ago

  Command *pcmd = AddEntryCommand::Create(&core, it);
  core.Execute(pcmd);
  EXPECT_TRUE(core.HasDBChanged());
  EXPECT_TRUE(it.IsExpired());

  ItemListConstIter iter = core.Find(it.GetUUID());
  ASSERT_NE(core.GetEntryEndIter(), iter);
  CItemData it2(core.GetEntry(iter));
  EXPECT_EQ(it, it2);

  core.WriteCurFile();
  EXPECT_FALSE(core.HasDBChanged());

  pcmd = UpdatePasswordCommand::Create(&core, it, sxNewPassword);
  core.Execute(pcmd);
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  CItemData it3(core.GetEntry(iter));
  EXPECT_EQ(it3.GetPassword(), sxNewPassword);
  ASSERT_FALSE(it3.IsExpired());

  EXPECT_TRUE(CreatePWHistoryList(it3.GetPWHistory(), pwh_max, num_err,
                                  pwhl, PWSUtil::TMC_ASC_UNKNOWN));

  EXPECT_EQ(0, num_err);
  EXPECT_EQ(3, pwh_max);
  EXPECT_EQ(1, pwhl.size());
  EXPECT_EQ(sxOldPassword, pwhl[0].password);
  EXPECT_EQ(tPMtime, pwhl[0].changetttdate);

  core.Undo();
  EXPECT_FALSE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  CItemData it4(core.GetEntry(iter));
  EXPECT_EQ(it4.GetPassword(), sxOldPassword);

  EXPECT_TRUE(CreatePWHistoryList(it4.GetPWHistory(), pwh_max, num_err,
                                  pwhl, PWSUtil::TMC_ASC_UNKNOWN));

  EXPECT_EQ(0, num_err);
  EXPECT_EQ(3, pwh_max);
  EXPECT_EQ(0, pwhl.size());

  core.Redo();
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  CItemData it5(core.GetEntry(iter));
  EXPECT_EQ(it5.GetPassword(), sxNewPassword);

  // New password change time is that of when Redo is performed & not original time
  it5.GetPMTime(tPMtime);

  EXPECT_TRUE(CreatePWHistoryList(it5.GetPWHistory(), pwh_max, num_err,
                                  pwhl, PWSUtil::TMC_ASC_UNKNOWN));
  EXPECT_EQ(0, num_err);
  EXPECT_EQ(3, pwh_max);
  EXPECT_EQ(1, pwhl.size());
  EXPECT_EQ(sxOldPassword, pwhl[0].password);
  EXPECT_EQ(tPMtime, pwhl[0].changetttdate);

  // Get core to delete any existing commands
  core.ClearCommands();

  // Delete file
  pws_os::DeleteAFile(fname);
}

TEST_F(CommandsTest, UpdateEntry)
{
  PWScore core;
  CItemData it;
  it.CreateUUID();
  time_t t;
  time(&t);
  it.SetCTime(t);
  it.SetTitle(L"RedC1gar");
  it.SetPassword(L"EarlyR1zer");

  Command *pcmd = AddEntryCommand::Create(&core, it);
  core.Execute(pcmd);
  EXPECT_TRUE(core.HasDBChanged());

  ItemListConstIter iter = core.Find(it.GetUUID());
  ASSERT_NE(core.GetEntryEndIter(), iter);
  CItemData it2(core.GetEntry(iter));
  EXPECT_EQ(it, it2);

  const StringX newTitle(L"PastaFar1an");
  pcmd = UpdateEntryCommand::Create(&core, it, CItem::TITLE, newTitle);
  core.Execute(pcmd);
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  EXPECT_EQ(core.GetEntry(iter).GetTitle(), newTitle);
  core.Undo();
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  EXPECT_EQ(core.GetEntry(iter).GetTitle(), it.GetTitle());

  // Get core to delete any existing commands
  core.ClearCommands();
}
