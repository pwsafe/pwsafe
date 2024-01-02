/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
  EXPECT_EQ(0U, core.GetNumEntries());

  // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest, DeleteEntry)
{
  PWScore core;
  CItemData ci;
  ci.CreateUUID();
  ci.SetTitle(L"blue rabbit");
  ci.SetPassword(L"notagain");
  auto addcmd = AddEntryCommand::Create(&core, ci);
  core.Execute(addcmd);

  auto delcmd = DeleteEntryCommand::Create(&core, ci);
  core.Execute(delcmd);
  EXPECT_EQ(0U, core.GetNumEntries());
  core.Undo();
  EXPECT_EQ(1U, core.GetNumEntries());
    // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest,DeleteEntryWithAttachment)
{
  PWScore core;
  CItemAtt ai;
  pws_os::CUUID attUuid;
  time_t cTime = 1665220859;
  unsigned char content[122] = { 0xff, 0x00, 0xb4, 0x65, 0xfc, 0x91, 0xfb, 0xbf,
                                0xe0, 0x8f, 0xea, 0x6b, 0xf9, 0x9f, 0xde, 0x1f,
                                0x62, 0xd6, 0xbf, 0xe8, 0x2f, 0xfb, 0x2c, 0xff,
                                0x00, 0xe0, 0xf3, 0xe1, 0xff, 0x00, 0xff, 0x00,
                                0x1d, 0xa5, 0x5b, 0x2d, 0x67, 0xbe, 0xaf, 0xfb,
                                0x2c, 0xff, 0x00, 0xe0, 0xf3, 0xc0, 0x1f, 0xfc,
                                0x76, 0x8a, 0x29, 0x7f, 0x68, 0x4b, 0xf9, 0x23,
                                0xf7, 0x7f, 0xc1, 0x1f, 0xd4, 0xd7, 0xf3, 0x3f,
                                0xbc, 0x5f, 0xb1, 0x6b, 0x1f, 0xf4, 0x17, 0xfd,
                                0x96, 0x7f, 0xf0, 0x79, 0xe0, 0x0f, 0xfe, 0x3b,
                                0x47, 0xd8, 0xb5, 0x8f, 0xfa, 0x0b, 0xfe, 0xcb,
                                0x3f, 0xf8, 0x3c, 0xf0, 0x07, 0xff, 0x00, 0x1d,
                                0xa2, 0x8a, 0x3f, 0xb4, 0x25, 0xfc, 0x91, 0xfb,
                                0xbf, 0xe0, 0x8b, 0xea, 0x6b, 0xf9, 0x9f, 0xde,
                                0xb0, 0xf1, 0xa7, 0xef, 0xa6, 0xdb, 0xf3, 0x3f,
                                0xff, 0xd9 };
  ai.SetUUID(attUuid);
  ai.SetTitle(L"magnum chocolate");
  ai.SetCTime(cTime);
  ai.SetContent(content, sizeof(content));

  CItemData ci;
  ci.CreateUUID();
  ci.SetTitle(L"red osprey");
  ci.SetPassword(L"sundown jelly");
  ci.SetAttUUID(attUuid);


  auto addcmd = AddEntryCommand::Create(&core, ci, pws_os::CUUID::NullUUID(), &ai);
  core.Execute(addcmd);
  EXPECT_EQ(1U, core.GetNumEntries());
  EXPECT_EQ(1U, core.GetNumAtts());

  auto delcmd = DeleteEntryCommand::Create(&core, ci);
  core.Execute(delcmd);
  EXPECT_EQ(0U, core.GetNumEntries());
  EXPECT_EQ(0U, core.GetNumAtts());
  core.Undo();
  EXPECT_EQ(1U, core.GetNumEntries());
  EXPECT_EQ(1U, core.GetNumAtts());
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
  EXPECT_EQ(2U, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  // Check that the base entry is correctly marked
  ItemListConstIter iter = core.Find(base_uuid);
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_TRUE(core.GetEntry(iter).IsShortcutBase());

  core.Undo();
  EXPECT_EQ(0U, core.GetNumEntries());
  EXPECT_FALSE(core.HasDBChanged());

  core.Redo();
  EXPECT_EQ(2U, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  // Delete base, expect both to be gone
  // Get base from core for correct type
  const CItemData bi2 = core.GetEntry(core.Find(base_uuid));
  DeleteEntryCommand *pcmd1 = DeleteEntryCommand::Create(&core, bi2);

  core.Execute(pcmd1);
  EXPECT_EQ(0U, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  core.Undo();
  EXPECT_EQ(2U, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  // Now just delete the shortcut, check that
  // base is left, and that it reverts to a normal entry
  const CItemData si2 = core.GetEntry(core.Find(si.GetUUID())); // si2 has baseUUID set
  DeleteEntryCommand *pcmd2 = DeleteEntryCommand::Create(&core, si2);

  core.Execute(pcmd2);
  ASSERT_EQ(1U, core.GetNumEntries());
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
  EXPECT_EQ(0U, core.GetNumEntries());
  EXPECT_FALSE(core.HasDBChanged());

  core.Redo();
  EXPECT_TRUE(core.HasDBChanged());
  EXPECT_EQ(1U, core.GetNumEntries());

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
  EXPECT_EQ(1U, vGroups.size());

  iter = core.Find(di.GetUUID());
  di = core.GetEntry(iter);
  di.SetGroup(L"g1.g1-1");
  pcmd = EditEntryCommand::Create(&core, di2, di);
  core.Execute(pcmd);

  core.GetAllGroups(vGroups);
  EXPECT_EQ(2U, vGroups.size());

  std::vector<StringX> eg;
  eg.push_back(L"e1");
  pcmd = DBEmptyGroupsCommand::Create(&core, eg, DBEmptyGroupsCommand::EG_ADDALL);
  core.Execute(pcmd);

  core.GetAllGroups(vGroups);
  EXPECT_EQ(3U, vGroups.size());

  // Get core to delete any existing commands
  core.ClearCommands();
}

TEST_F(CommandsTest, UpdatePassword)
{
  PWScore core;

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

  {
    PWHistList pwhl(it3.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(1U, pwhl.size());
    EXPECT_EQ(sxOldPassword, pwhl[0].password);
    EXPECT_EQ(tPMtime, pwhl[0].changetttdate);
  }

  core.Undo();
  EXPECT_FALSE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  CItemData it4(core.GetEntry(iter));
  EXPECT_EQ(it4.GetPassword(), sxOldPassword);

  {
    PWHistList pwhl(it4.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(0U, pwhl.size());
  }

  core.Redo();
  EXPECT_TRUE(core.HasDBChanged());

  iter = core.Find(it.GetUUID());
  CItemData it5(core.GetEntry(iter));
  EXPECT_EQ(it5.GetPassword(), sxNewPassword);

  // New password change time is that of when Redo is performed & not original time
  it5.GetPMTime(tPMtime);

  {
    PWHistList pwhl(it5.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(1U, pwhl.size());
    EXPECT_EQ(sxOldPassword, pwhl[0].password);
    EXPECT_EQ(tPMtime, pwhl[0].changetttdate);
  }

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
