/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
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

namespace {
  // A reusable observer that records every UpdateGUI call so individual
  // tests can assert on which GUI actions were fired and how many times.
  class TestGUIObserver : public Observer {
  public:
    struct CallRecord {
      UpdateGUICommand::GUI_Action ga;
      pws_os::CUUID uuid;
    };

    std::vector<CallRecord> calls;

    void UpdateGUI(const UpdateGUICommand::GUI_Action ga,
                   const pws_os::CUUID &entry_uuid,
                   CItemData::FieldType /* ft */) override {
      calls.push_back({ga, entry_uuid});
    }
  };
}


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
  const auto iter = core.Find(uuid);
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

  // Verify Undo does nothing when core is in read-only mode
  auto delcmd2 = DeleteEntryCommand::Create(&core, ci);
  core.Execute(delcmd2);
  EXPECT_EQ(0U, core.GetNumEntries());
  core.SetReadOnly(true);
  delcmd2->Undo();
  EXPECT_EQ(0U, core.GetNumEntries());
  core.SetReadOnly(false);
  core.ClearCommands();

  // Test undoing deletion of alias base and dependent alias entries
  CItemData abase, al;
  abase.CreateUUID();
  abase.SetTitle(L"alias base");
  abase.SetPassword(L"base password");

  al.SetTitle(L"alias entry");
  al.SetPassword(L"[Alias]");
  al.SetAlias();
  al.CreateUUID();

  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, abase));
  pmulticmds->Add(AddEntryCommand::Create(&core, al, abase.GetUUID()));
  core.Execute(pmulticmds);
  EXPECT_EQ(2U, core.GetNumEntries());

  // Delete the alias base (converts alias dependent to normal entry)
  const CItemData abaseCore = core.GetEntry(core.Find(abase.GetUUID()));
  auto delBaseCmd = DeleteEntryCommand::Create(&core, abaseCore);
  core.Execute(delBaseCmd);
  EXPECT_EQ(1U, core.GetNumEntries());
  EXPECT_TRUE(core.GetEntry(core.Find(al.GetUUID())).IsNormal());

  // Undoing deletion of alias base restores the base and reverts dependent back to an alias
  delBaseCmd->Undo();
  EXPECT_EQ(2U, core.GetNumEntries());
  EXPECT_TRUE(core.GetEntry(core.Find(abase.GetUUID())).IsAliasBase());
  EXPECT_TRUE(core.GetEntry(core.Find(al.GetUUID())).IsAlias());

  // Re-delete alias base
  delBaseCmd = DeleteEntryCommand::Create(&core, core.GetEntry(core.Find(abase.GetUUID())));
  core.Execute(delBaseCmd);
  EXPECT_EQ(1U, core.GetNumEntries());

  // Also delete the alias dependent entry
  const CItemData alCore = core.GetEntry(core.Find(al.GetUUID()));
  auto delAlCmd = DeleteEntryCommand::Create(&core, alCore);
  core.Execute(delAlCmd);
  EXPECT_EQ(0U, core.GetNumEntries());

  // Undoing deletion of alias base when dependent alias is no longer present in core
  delBaseCmd->Undo();
  EXPECT_EQ(1U, core.GetNumEntries());
  EXPECT_NE(core.GetEntryEndIter(), core.Find(abase.GetUUID()));

  // Clean up before next test section
  const CItemData abaseRestored = core.GetEntry(core.Find(abase.GetUUID()));
  core.Execute(DeleteEntryCommand::Create(&core, abaseRestored));
  EXPECT_EQ(0U, core.GetNumEntries());
  core.ClearCommands();

  // Test undoing deletion of dependent entry when the entry is already present in core
  CItemData sbase, sdep;
  sbase.CreateUUID();
  sbase.SetTitle(L"shortcut base");
  sbase.SetPassword(L"base password");

  sdep.SetTitle(L"shortcut entry");
  sdep.SetPassword(L"[Shortcut]");
  sdep.SetShortcut();
  sdep.CreateUUID();

  MultiCommands *pmulticmds2 = MultiCommands::Create(&core);
  pmulticmds2->Add(AddEntryCommand::Create(&core, sbase));
  pmulticmds2->Add(AddEntryCommand::Create(&core, sdep, sbase.GetUUID()));
  core.Execute(pmulticmds2);
  EXPECT_EQ(2U, core.GetNumEntries());

  const CItemData sdepCore = core.GetEntry(core.Find(sdep.GetUUID()));
  auto delDepCmd = DeleteEntryCommand::Create(&core, sdepCore);
  core.Execute(delDepCmd);
  EXPECT_EQ(1U, core.GetNumEntries());

  delDepCmd->Undo();
  EXPECT_EQ(2U, core.GetNumEntries());

  delDepCmd->Redo();
  EXPECT_EQ(1U, core.GetNumEntries());

  // Add sdep back manually into core before calling Undo on delDepCmd
  auto readdDepCmd = AddEntryCommand::Create(&core, sdepCore, sbase.GetUUID());
  core.Execute(readdDepCmd);
  EXPECT_EQ(2U, core.GetNumEntries());

  // Undoing dependent deletion when the dependent entry already exists in core - does nothing
  delDepCmd->Undo();
  EXPECT_EQ(2U, core.GetNumEntries());

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


  core.Execute(AddEntryCommand::Create(&core, ci, pws_os::CUUID::NullUUID(), &ai));
  EXPECT_EQ(1U, core.GetNumEntries());
  EXPECT_EQ(1U, core.GetNumAtts());

  core.Execute(DeleteEntryCommand::Create(&core, ci));
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
  si.SetXTime(0L);
  si.SetStatus(CItemData::ES_ADDED);

  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, bi));
  pmulticmds->Add(AddEntryCommand::Create(&core, si, base_uuid));
  core.Execute(pmulticmds);
  EXPECT_EQ(2U, core.GetNumEntries());
  EXPECT_TRUE(core.HasDBChanged());

  // Check that the base entry is correctly marked
  auto iter = core.Find(base_uuid);
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

  auto iter = core.Find(it.GetUUID());
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
  auto iter = core.Find(uuid);
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

  const std::vector<StringX> eg{L"e1"};
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

  auto iter = core.Find(it.GetUUID());
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

TEST_F(CommandsTest, UpdatePasswordHistory)
{
  PWScore core;
  CItemData item, protectedItem;
  item.CreateUUID();
  item.SetPWHistory(L"");
  protectedItem.CreateUUID();
  protectedItem.SetPWHistory(L"10300");
  protectedItem.SetProtected(true);

  auto addCmd = MultiCommands::Create(&core);
  addCmd->Add(AddEntryCommand::Create(&core, item));
  addCmd->Add(AddEntryCommand::Create(&core, protectedItem));
  core.Execute(addCmd);

  auto getHistory = [&core](const pws_os::CUUID &uuid) {
    return core.GetEntry(core.Find(uuid)).GetPWHistory();
  };

  core.Execute(UpdatePasswordHistoryCommand::Create(&core, PWHist::START_EXCL_PROT, 5));
  EXPECT_EQ(L"10500", getHistory(item.GetUUID()));
  EXPECT_EQ(L"10300", getHistory(protectedItem.GetUUID()));
  core.Undo();
  EXPECT_TRUE(getHistory(item.GetUUID()).empty());
  core.Redo();
  EXPECT_EQ(L"10500", getHistory(item.GetUUID()));

  core.Execute(UpdatePasswordHistoryCommand::Create(&core, PWHist::SETMAX_INCL_PROT, 2));
  EXPECT_EQ(L"10200", getHistory(item.GetUUID()));
  EXPECT_EQ(L"10200", getHistory(protectedItem.GetUUID()));

  core.Execute(UpdatePasswordHistoryCommand::Create(&core, PWHist::STOP_EXCL_PROT, 0));
  EXPECT_EQ(L"00200", getHistory(item.GetUUID()));
  EXPECT_EQ(L"10200", getHistory(protectedItem.GetUUID()));

  core.Execute(UpdatePasswordHistoryCommand::Create(&core, PWHist::CLEAR_INCL_PROT, 0));
  EXPECT_TRUE(getHistory(item.GetUUID()).empty());
  EXPECT_TRUE(getHistory(protectedItem.GetUUID()).empty());

  core.Undo();
  EXPECT_EQ(L"00200", getHistory(item.GetUUID()));
  EXPECT_EQ(L"10200", getHistory(protectedItem.GetUUID()));

  core.ClearCommands();
}

TEST_F(CommandsTest, ChangeDBHeaderCommand)
{
  PWScore core;

  // Initially header fields are empty
  EXPECT_TRUE(core.GetHeaderItem(PWSfile::HDR_DBNAME).empty());
  EXPECT_TRUE(core.GetHeaderItem(PWSfile::HDR_DBDESC).empty());

  // --- HDR_DBNAME ---

  core.Execute(ChangeDBHeaderCommand::Create(&core, L"MyDatabase", PWSfile::HDR_DBNAME));
  EXPECT_EQ(StringX(L"MyDatabase"), core.GetHeaderItem(PWSfile::HDR_DBNAME));
  EXPECT_TRUE(core.HasDBChanged());

  core.Undo();
  EXPECT_TRUE(core.GetHeaderItem(PWSfile::HDR_DBNAME).empty());

  core.Redo();
  EXPECT_EQ(StringX(L"MyDatabase"), core.GetHeaderItem(PWSfile::HDR_DBNAME));

  // --- HDR_DBDESC ---

  core.Execute(ChangeDBHeaderCommand::Create(&core, L"A description", PWSfile::HDR_DBDESC));
  EXPECT_EQ(StringX(L"A description"), core.GetHeaderItem(PWSfile::HDR_DBDESC));

  core.Undo();
  EXPECT_TRUE(core.GetHeaderItem(PWSfile::HDR_DBDESC).empty());

  core.ClearCommands();

  // --- No-op when new value equals the current value ---

  const StringX sxSameName(L"SameName");
  core.Execute(ChangeDBHeaderCommand::Create(&core, sxSameName, PWSfile::HDR_DBNAME));
  EXPECT_EQ(sxSameName, core.GetHeaderItem(PWSfile::HDR_DBNAME));

  core.Execute(ChangeDBHeaderCommand::Create(&core, sxSameName, PWSfile::HDR_DBNAME));
  EXPECT_EQ(sxSameName, core.GetHeaderItem(PWSfile::HDR_DBNAME));

  core.Undo();
  EXPECT_EQ(sxSameName, core.GetHeaderItem(PWSfile::HDR_DBNAME));

  core.ClearCommands();

  // --- No-op when core is read-only ---

  core.SetReadOnly(true);
  core.Execute(ChangeDBHeaderCommand::Create(&core, L"ReadOnlyAttempt", PWSfile::HDR_DBNAME));
  EXPECT_EQ(sxSameName, core.GetHeaderItem(PWSfile::HDR_DBNAME));
  core.SetReadOnly(false);

  core.ClearCommands();
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

  auto iter = core.Find(it.GetUUID());
  ASSERT_NE(core.GetEntryEndIter(), iter);
  EXPECT_EQ(it, CItemData(core.GetEntry(iter)));

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

TEST_F(CommandsTest, MultiCommandsFindCommand)
{
  PWScore core;
  MultiCommands *pmulticmds = MultiCommands::Create(&core);

  EXPECT_EQ(nullptr, pmulticmds->FindCommand(typeid(AddEntryCommand)));

  CItemData ci1;
  ci1.CreateUUID();
  ci1.SetTitle(L"Entry 1");
  AddEntryCommand *addCmd = AddEntryCommand::Create(&core, ci1);
  pmulticmds->Add(addCmd);

  RenameGroupCommand *renameCmd = RenameGroupCommand::Create(&core, L"GroupA", L"GroupB");
  pmulticmds->Add(renameCmd);

  EXPECT_EQ(addCmd, pmulticmds->FindCommand(typeid(AddEntryCommand)));
  EXPECT_EQ(renameCmd, pmulticmds->FindCommand(typeid(RenameGroupCommand)));

  EXPECT_EQ(nullptr, pmulticmds->FindCommand(typeid(DeleteEntryCommand)));

  core.ClearCommands();
}

TEST_F(CommandsTest, MultiCommandsInsert)
{
  PWScore core;
  MultiCommands *pmulticmds = MultiCommands::Create(&core);

  CItemData ci1;
  ci1.CreateUUID();
  ci1.SetTitle(L"First Entry");
  AddEntryCommand *cmd1 = AddEntryCommand::Create(&core, ci1);
  pmulticmds->Insert(cmd1, 0);
  EXPECT_EQ(1U, pmulticmds->GetSize());

  CItemData ci2;
  ci2.CreateUUID();
  ci2.SetTitle(L"Prepended Entry");
  AddEntryCommand *cmd0 = AddEntryCommand::Create(&core, ci2);
  pmulticmds->Insert(cmd0, 0);
  EXPECT_EQ(2U, pmulticmds->GetSize());

  RenameGroupCommand *cmdMid = RenameGroupCommand::Create(&core, L"Group0.Alpha", L"Group0.Beta");
  pmulticmds->Insert(cmdMid, 1);
  EXPECT_EQ(3U, pmulticmds->GetSize());

  CItemData ci3;
  ci3.CreateUUID();
  ci3.SetTitle(L"Appended Entry");
  AddEntryCommand *cmdEnd = AddEntryCommand::Create(&core, ci3);
  pmulticmds->Insert(cmdEnd, pmulticmds->GetSize());
  EXPECT_EQ(4U, pmulticmds->GetSize());

  core.Execute(pmulticmds);
  EXPECT_EQ(3U, core.GetNumEntries());
  EXPECT_NE(core.GetEntryEndIter(), core.Find(ci1.GetUUID()));
  EXPECT_NE(core.GetEntryEndIter(), core.Find(ci2.GetUUID()));
  EXPECT_NE(core.GetEntryEndIter(), core.Find(ci3.GetUUID()));

  core.Undo();
  EXPECT_EQ(0U, core.GetNumEntries());

  core.ClearCommands();
}

TEST_F(CommandsTest, MultiCommandsGetRC)
{
  PWScore core;
  MultiCommands *pmulticmds = MultiCommands::Create(&core);

  CItemData ci1;
  ci1.CreateUUID();
  ci1.SetTitle(L"Entry 1");
  Command *cmd1 = AddEntryCommand::Create(&core, ci1);

  CItemData ci2;
  ci2.CreateUUID();
  ci2.SetTitle(L"Entry 2");
  Command *cmd2 = AddEntryCommand::Create(&core, ci2);

  pmulticmds->Add(cmd1);
  pmulticmds->Add(cmd2);

  // Before Execute, GetRC for non-existent command and invalid indices
  CItemData ci3;
  ci3.CreateUUID();
  Command *cmdUnexecuted = AddEntryCommand::Create(&core, ci3);
  int rc = -999;
  EXPECT_FALSE(pmulticmds->GetRC(cmdUnexecuted, rc));
  EXPECT_EQ(0, rc);
  // have to delete manually since it was never executed, so is not tracked by core
  delete cmdUnexecuted;

  rc = -999;
  EXPECT_FALSE(pmulticmds->GetRC(size_t{0}, rc));
  EXPECT_EQ(0, rc);
  rc = -999;
  EXPECT_FALSE(pmulticmds->GetRC(size_t{1}, rc));
  EXPECT_EQ(0, rc);

  core.Execute(pmulticmds);

  // After Execute, GetRC by command pointer
  rc = -999;
  EXPECT_TRUE(pmulticmds->GetRC(cmd1, rc));
  EXPECT_EQ(0, rc);

  rc = -999;
  EXPECT_TRUE(pmulticmds->GetRC(cmd2, rc));
  EXPECT_EQ(0, rc);

  // GetRC by index
  rc = -999;
  EXPECT_TRUE(pmulticmds->GetRC(size_t{1}, rc));
  EXPECT_EQ(0, rc);

  rc = -999;
  EXPECT_TRUE(pmulticmds->GetRC(size_t{2}, rc));
  EXPECT_EQ(0, rc);

  // Out-of-bounds indices
  rc = -999;
  EXPECT_FALSE(pmulticmds->GetRC(size_t{0}, rc));
  EXPECT_EQ(0, rc);

  rc = -999;
  EXPECT_FALSE(pmulticmds->GetRC(size_t{3}, rc));
  EXPECT_EQ(0, rc);

  core.ClearCommands();
}

TEST_F(CommandsTest, UpdateGUICommand)
{
  PWScore core;
  TestGUIObserver observer;
  core.RegisterObserver(&observer);

  const pws_os::CUUID testUuid;

  // 1. WN_UNDO: Execute does not notify, Undo does notify
  UpdateGUICommand *cmdUndoOnly = UpdateGUICommand::Create(&core, UpdateGUICommand::WN_UNDO,
                                                           UpdateGUICommand::GUI_REFRESH_ENTRY, testUuid);
  // Unlike other tests, we don't use core's Execute/Undo/Redo here because those also trigger a `GUI_UPDATE_STATUSBAR`,
  // and filtering those out would complicate the test. We want to focus just on the command's own functionality,
  // not that of PWScore.
  cmdUndoOnly->Execute();
  EXPECT_TRUE(observer.calls.empty());

  cmdUndoOnly->Undo();
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_REFRESH_ENTRY, observer.calls[0].ga);
  observer.calls.clear();
  delete cmdUndoOnly; // can't use core.ClearCommands() for the cleanup since we didn't use core.Execute()

  // 2. WN_ALL: Both Execute and Undo notify
  UpdateGUICommand *cmdAll = UpdateGUICommand::Create(&core, UpdateGUICommand::WN_ALL,
                                                      UpdateGUICommand::GUI_REFRESH_TREE,
                                                      pws_os::CUUID::NullUUID());
  cmdAll->Execute();
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_REFRESH_TREE, observer.calls[0].ga);
  observer.calls.clear();

  cmdAll->Undo();
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_REFRESH_TREE, observer.calls[0].ga);
  observer.calls.clear();
  delete cmdAll;

  // 3. WN_EXECUTE: Execute notifies, Undo does not notify
  UpdateGUICommand *cmdExecOnly = UpdateGUICommand::Create(&core, UpdateGUICommand::WN_EXECUTE,
                                                           UpdateGUICommand::GUI_ADD_ENTRY, testUuid);
  cmdExecOnly->Execute();
  ASSERT_EQ(1U, observer.calls.size());
  observer.calls.clear();

  cmdExecOnly->Undo();
  EXPECT_TRUE(observer.calls.empty());

  delete cmdExecOnly;
  core.UnregisterObserver(&observer);
  core.ClearCommands();
}

TEST_F(CommandsTest, EditAndDeleteAttachment)
{
  PWScore core;
  CItemAtt ai;
  pws_os::CUUID attUuid;
  time_t cTime = 1665220859L;
  unsigned char content[16] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
  ai.SetUUID(attUuid);
  ai.SetTitle(L"original attachment title");
  ai.SetCTime(cTime);
  ai.SetContent(content, sizeof(content));

  CItemData ci;
  ci.CreateUUID();
  ci.SetTitle(L"entry with attachment");
  ci.SetPassword(L"password123");
  ci.SetAttUUID(attUuid);
  ci.SetNormal();

  auto addcmd = AddEntryCommand::Create(&core, ci, pws_os::CUUID::NullUUID(), &ai);
  core.Execute(addcmd);
  EXPECT_EQ(1U, core.GetNumEntries());
  EXPECT_TRUE(core.HasAtt(attUuid));

  // Edit attachment title operations
  CItemAtt oldAtt = core.GetAtt(attUuid);
  CItemAtt newAtt(oldAtt);
  newAtt.SetTitle(L"updated attachment title");

  auto editAttCmd = EditAttachmentCommand::Create(&core, oldAtt, newAtt);
  core.Execute(editAttCmd);
  EXPECT_EQ(L"updated attachment title", core.GetAtt(attUuid).GetTitle());

  core.Undo();
  EXPECT_EQ(L"original attachment title", core.GetAtt(attUuid).GetTitle());

  core.Redo();
  EXPECT_EQ(L"updated attachment title", core.GetAtt(attUuid).GetTitle());

  // Delete attachment from entry operations
  const CItemData ciInCore = core.GetEntry(core.Find(ci.GetUUID()));
  auto delAttCmd = DeleteAttachmentCommand::Create(&core, ciInCore);
  core.Execute(delAttCmd);
  EXPECT_FALSE(core.HasAtt(attUuid));

  core.Undo();
  EXPECT_TRUE(core.HasAtt(attUuid));
  EXPECT_EQ(L"updated attachment title", core.GetAtt(attUuid).GetTitle());

  core.Redo();
  EXPECT_FALSE(core.HasAtt(attUuid));

  core.ClearCommands();
}

TEST_F(CommandsTest, AddAndRemoveDependentEntry)
{
  PWScore core;

  // Test shortcut entry
  CItemData sbase, sdep;
  sbase.CreateUUID();
  sbase.SetTitle(L"shortcut base entry");
  sbase.SetPassword(L"shortcut base password");
  sbase.SetNormal();

  sdep.CreateUUID();
  sdep.SetTitle(L"shortcut entry");
  sdep.SetPassword(L"[Shortcut]");
  sdep.SetShortcut();

  MultiCommands *pmulticmds1 = MultiCommands::Create(&core);
  pmulticmds1->Add(AddEntryCommand::Create(&core, sbase));
  pmulticmds1->Add(AddEntryCommand::Create(&core, sdep, sbase.GetUUID()));
  core.Execute(pmulticmds1);

  EXPECT_EQ(2U, core.GetNumEntries());
  EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID()));

  auto rmShortcutCmd = RemoveDependentEntryCommand::Create(&core, sbase.GetUUID(), sdep.GetUUID(), CItemData::ET_SHORTCUT);
  core.Execute(rmShortcutCmd);
  EXPECT_EQ(0U, core.NumShortcuts(sbase.GetUUID()));

  core.Undo();
  EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID()));

  core.Redo();
  EXPECT_EQ(0U, core.NumShortcuts(sbase.GetUUID()));

  // Test adding shortcut dependent association back
  auto addShortcutCmd = AddDependentEntryCommand::Create(&core, sbase.GetUUID(), sdep.GetUUID(), CItemData::ET_SHORTCUT);
  core.Execute(addShortcutCmd);
  EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID()));

  core.Undo();
  EXPECT_EQ(0U, core.NumShortcuts(sbase.GetUUID()));

  core.Redo();
  EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID()));

  // Test alias entry
  CItemData abase, adep;
  abase.CreateUUID();
  abase.SetTitle(L"alias base entry");
  abase.SetPassword(L"alias base password");
  abase.SetNormal();

  adep.CreateUUID();
  adep.SetTitle(L"alias entry");
  adep.SetPassword(L"[Alias]");
  adep.SetAlias();

  MultiCommands *pmulticmds2 = MultiCommands::Create(&core);
  pmulticmds2->Add(AddEntryCommand::Create(&core, abase));
  pmulticmds2->Add(AddEntryCommand::Create(&core, adep, abase.GetUUID()));
  core.Execute(pmulticmds2);

  EXPECT_EQ(4U, core.GetNumEntries());
  EXPECT_EQ(1U, core.NumAliases(abase.GetUUID()));

  auto rmAliasCmd = RemoveDependentEntryCommand::Create(&core, abase.GetUUID(), adep.GetUUID(), CItemData::ET_ALIAS);
  core.Execute(rmAliasCmd);
  EXPECT_EQ(0U, core.NumAliases(abase.GetUUID()));

  core.Undo();
  EXPECT_EQ(1U, core.NumAliases(abase.GetUUID()));

  core.Redo();
  EXPECT_EQ(0U, core.NumAliases(abase.GetUUID()));

  // Test adding alias dependent association back
  auto addAliasCmd = AddDependentEntryCommand::Create(&core, abase.GetUUID(), adep.GetUUID(), CItemData::ET_ALIAS);
  core.Execute(addAliasCmd);
  EXPECT_EQ(1U, core.NumAliases(abase.GetUUID()));

  core.Undo();
  EXPECT_EQ(0U, core.NumAliases(abase.GetUUID()));

  core.Redo();
  EXPECT_EQ(1U, core.NumAliases(abase.GetUUID()));

  core.ClearCommands();
}

TEST_F(CommandsTest, AddAndMoveDependentEntries)
{
  PWScore core;

  CItemData base1, base2, base3, dep1, dep2, dep3;
  base1.CreateUUID();
  base1.SetTitle(L"shortcut base 1");
  base1.SetPassword(L"password123");
  base1.SetNormal();

  base2.CreateUUID();
  base2.SetTitle(L"shortcut base 2");
  base2.SetPassword(L"password456");
  base2.SetNormal();

  base3.CreateUUID();
  base3.SetTitle(L"alias base 1");
  base3.SetPassword(L"password456");
  base3.SetNormal();

  dep1.CreateUUID();
  dep1.SetTitle(L"shortcut 1");
  dep1.SetPassword(L"temp password 1");
  dep1.SetNormal();
  dep1.SetBaseUUID(base1.GetUUID());

  dep2.CreateUUID();
  dep2.SetTitle(L"shortcut 2");
  dep2.SetPassword(L"temp password 2");
  dep2.SetNormal();
  dep2.SetBaseUUID(base1.GetUUID());

  dep3.SetTitle(L"alias entry");
  dep3.SetPassword(L"[Alias]");
  dep3.SetNormal();
  dep3.CreateUUID();
  dep3.SetBaseUUID(base3.GetUUID());

  MultiCommands *pmulticmds = MultiCommands::Create(&core);
  pmulticmds->Add(AddEntryCommand::Create(&core, base1));
  pmulticmds->Add(AddEntryCommand::Create(&core, base2));
  pmulticmds->Add(AddEntryCommand::Create(&core, base3));
  pmulticmds->Add(AddEntryCommand::Create(&core, dep1));
  pmulticmds->Add(AddEntryCommand::Create(&core, dep2));
  pmulticmds->Add(AddEntryCommand::Create(&core, dep3));
  core.Execute(pmulticmds);

  EXPECT_EQ(6U, core.GetNumEntries());
  EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
  EXPECT_EQ(0U, core.NumShortcuts(base2.GetUUID()));

  UUIDVector shortcuts{dep1.GetUUID(), dep2.GetUUID()};

  auto addCmdShortcut = AddDependentEntriesCommand::Create(&core, shortcuts, /* pRpt */ nullptr, CItemData::ET_SHORTCUT, CItemData::UUID);
  core.Execute(addCmdShortcut);
  EXPECT_EQ(2U, core.NumShortcuts(base1.GetUUID()));

  core.Undo();
  EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));

  core.Redo();
  EXPECT_EQ(2U, core.NumShortcuts(base1.GetUUID()));

  UUIDVector aliases{dep3.GetUUID()};

  auto addCmdAlias = AddDependentEntriesCommand::Create(&core, aliases, /* pRpt */ nullptr, CItemData::ET_ALIAS, CItemData::UUID);
  core.Execute(addCmdAlias);
  EXPECT_EQ(1U, core.NumAliases(base3.GetUUID()));

  core.Undo();
  EXPECT_EQ(0U, core.NumAliases(base3.GetUUID()));

  core.Redo();
  EXPECT_EQ(1U, core.NumAliases(base3.GetUUID()));

  auto moveCmdShortcut = MoveDependentEntriesCommand::Create(&core, base1.GetUUID(), base2.GetUUID(), CItemData::ET_SHORTCUT);
  core.Execute(moveCmdShortcut);
  EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
  EXPECT_EQ(2U, core.NumShortcuts(base2.GetUUID()));

  core.Undo();
  EXPECT_EQ(2U, core.NumShortcuts(base1.GetUUID()));
  EXPECT_EQ(0U, core.NumShortcuts(base2.GetUUID()));

  core.Redo();
  EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
  EXPECT_EQ(2U, core.NumShortcuts(base2.GetUUID()));

  auto moveCmdAlias = MoveDependentEntriesCommand::Create(&core, base3.GetUUID(), base2.GetUUID(), CItemData::ET_ALIAS);
  core.Execute(moveCmdAlias);
  EXPECT_EQ(0U, core.NumAliases(base3.GetUUID()));
  EXPECT_EQ(1U, core.NumAliases(base2.GetUUID()));

  core.Undo();
  EXPECT_EQ(1U, core.NumAliases(base3.GetUUID()));
  EXPECT_EQ(0U, core.NumAliases(base2.GetUUID()));

  core.Redo();
  EXPECT_EQ(0U, core.NumAliases(base3.GetUUID()));
  EXPECT_EQ(1U, core.NumAliases(base2.GetUUID()));

  core.ClearCommands();
}

TEST_F(CommandsTest, PolicyCommands)
{
  PWScore core;
  PSWDPolicyMap policies;

  PWPolicy initialPolicy;
  initialPolicy.flags = PWPolicy::UseDigits | PWPolicy::UseUppercase;
  initialPolicy.length = 12;

  stringT policyName1 = L"WebPolicy";
  StringX s_policyName1(policyName1);
  auto addCmd = new PolicyCommandAdd(core, policies, policyName1, initialPolicy);
  core.Execute(addCmd);

  EXPECT_EQ(1U, policies.size());
  EXPECT_TRUE(policies.contains(s_policyName1));
  EXPECT_EQ(12, policies[s_policyName1].length);

  core.Undo();
  EXPECT_EQ(0U, policies.size());

  core.Redo();
  EXPECT_EQ(1U, policies.size());

  PWPolicy modifiedPolicy = initialPolicy;
  modifiedPolicy.length = 16;
  modifiedPolicy.flags |= PWPolicy::UseSymbols;

  auto modifyCmd = new PolicyCommandModify<MultiPolicyCollector, PSWDPolicyMap>(core, policies, policyName1, initialPolicy, modifiedPolicy);
  core.Execute(modifyCmd);

  EXPECT_EQ(16, policies[s_policyName1].length);

  core.Undo();
  EXPECT_EQ(12, policies[s_policyName1].length);

  core.Redo();
  EXPECT_EQ(16, policies[s_policyName1].length);

  stringT policyName2 = L"SecureWebPolicy";
  StringX s_policyName2(policyName2);
  PWPolicy renamedPolicy = modifiedPolicy;
  renamedPolicy.length = 20;

  auto renameCmd = new PolicyCommandRename(core, policies, policyName1, policyName2, modifiedPolicy, renamedPolicy);
  core.Execute(renameCmd);

  EXPECT_EQ(1U, policies.size());
  EXPECT_FALSE(policies.contains(s_policyName1));
  EXPECT_TRUE(policies.contains(s_policyName2));
  EXPECT_EQ(20, policies[s_policyName2].length);

  core.Undo();
  EXPECT_EQ(1U, policies.size());
  EXPECT_TRUE(policies.contains(s_policyName1));
  EXPECT_FALSE(policies.contains(s_policyName2));
  EXPECT_EQ(16, policies[s_policyName1].length);

  core.Redo();
  EXPECT_EQ(1U, policies.size());
  EXPECT_FALSE(policies.contains(s_policyName1));
  EXPECT_TRUE(policies.contains(s_policyName2));
  EXPECT_EQ(20, policies[s_policyName2].length);

  // Remove password policy
  auto removeCmd = new PolicyCommandRemove(core, policies, policyName2, renamedPolicy);
  core.Execute(removeCmd);

  EXPECT_EQ(0U, policies.size());

  core.Undo();
  EXPECT_EQ(1U, policies.size());
  EXPECT_TRUE(policies.contains(s_policyName2));

  core.Redo();
  EXPECT_EQ(0U, policies.size());

  core.ClearCommands();
}

TEST_F(CommandsTest, DBPolicyNamesCommand)
{
  PWScore core;

  PWPolicy pol1, pol2, pol3;
  pol1.flags  = PWPolicy::UseDigits | PWPolicy::UseLowercase;
  pol1.length = 12;
  pol2.flags  = PWPolicy::UseUppercase | PWPolicy::UseSymbols;
  pol2.length = 16;
  pol3.flags  = PWPolicy::UseDigits | PWPolicy::UseUppercase;
  pol3.length = 20;

  const StringX name1(L"Policy1");
  const StringX name2(L"Policy2");
  const StringX name3(L"Policy3");

  // --- Single-policy add ---

  core.Execute(DBPolicyNamesCommand::Create(&core, name1, pol1));
  EXPECT_EQ(1U, core.GetPasswordPolicies().size());
  EXPECT_EQ(12, core.GetPasswordPolicies().at(name1).length);
  EXPECT_TRUE(core.HasDBChanged());

  core.Undo();
  EXPECT_EQ(0U, core.GetPasswordPolicies().size());

  core.Redo();
  EXPECT_EQ(1U, core.GetPasswordPolicies().size());

  core.ClearCommands();

  // --- NP_ADDNEW: adds only policies whose names are not already present ---

  PSWDPolicyMap newPolicies;
  newPolicies[name1] = pol1;  // duplicate – should be skipped
  newPolicies[name2] = pol2;  // new – should be added

  core.Execute(DBPolicyNamesCommand::Create(&core, newPolicies, DBPolicyNamesCommand::NP_ADDNEW));
  EXPECT_EQ(2U, core.GetPasswordPolicies().size());
  EXPECT_TRUE(core.GetPasswordPolicies().contains(name1));
  EXPECT_TRUE(core.GetPasswordPolicies().contains(name2));

  core.Undo();
  EXPECT_EQ(1U, core.GetPasswordPolicies().size());
  EXPECT_TRUE(core.GetPasswordPolicies().contains(name1));
  EXPECT_FALSE(core.GetPasswordPolicies().contains(name2));

  core.Redo();
  EXPECT_EQ(2U, core.GetPasswordPolicies().size());

  core.ClearCommands();

  // --- NP_REPLACEALL: replaces the entire policy map ---

  PSWDPolicyMap replacementPolicies;
  replacementPolicies[name3] = pol3;

  core.Execute(DBPolicyNamesCommand::Create(&core, replacementPolicies, DBPolicyNamesCommand::NP_REPLACEALL));
  EXPECT_EQ(1U, core.GetPasswordPolicies().size());
  EXPECT_FALSE(core.GetPasswordPolicies().contains(name1));
  EXPECT_FALSE(core.GetPasswordPolicies().contains(name2));
  EXPECT_TRUE(core.GetPasswordPolicies().contains(name3));
  EXPECT_EQ(20, core.GetPasswordPolicies().at(name3).length);

  core.Undo();
  EXPECT_EQ(2U, core.GetPasswordPolicies().size());
  EXPECT_TRUE(core.GetPasswordPolicies().contains(name1));
  EXPECT_TRUE(core.GetPasswordPolicies().contains(name2));

  core.ClearCommands();

  // --- No-op when core is read-only ---

  const size_t sizeBefore = core.GetPasswordPolicies().size();
  core.SetReadOnly(true);
  core.Execute(DBPolicyNamesCommand::Create(&core, name3, pol3));
  EXPECT_EQ(sizeBefore, core.GetPasswordPolicies().size());
  core.SetReadOnly(false);

  core.ClearCommands();

  // --- GUI notification: observer is called on Execute and on Undo ---

  TestGUIObserver observer;
  core.RegisterObserver(&observer);

  auto pcmd = DBPolicyNamesCommand::Create(&core, name3, pol3);
  // same as in the UpdateGUICommand command test, execute the command directly to avoid the UI notifications
  // generated by core - these are out of scope for this test
  pcmd->Execute();
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_UPDATE_STATUSBAR, observer.calls[0].ga);
  observer.calls.clear();

  pcmd->Undo();
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_UPDATE_STATUSBAR, observer.calls[0].ga);
  observer.calls.clear();
  delete pcmd; // delete manually since we didn't use core.Execute, so core isn't tracking this command

  // --- GUI notification suppressed when SetNoGUINotify() is called ---

  auto silentCmd = DBPolicyNamesCommand::Create(&core, name3, pol3);
  silentCmd->SetNoGUINotify();
  silentCmd->Execute();
  EXPECT_TRUE(observer.calls.empty());

  silentCmd->Undo();
  EXPECT_TRUE(observer.calls.empty());
  delete silentCmd;

  core.UnregisterObserver(&observer);
  core.ClearCommands();
}

TEST_F(CommandsTest, DBEmptyGroupsCommand)
{
  PWScore core;
  TestGUIObserver observer;
  core.RegisterObserver(&observer);

  // initial setup
  const StringX group1(L"Group1");
  const StringX group2(L"Group2");
  const StringX group1a(L"Group1.Subgroup");
  const std::vector initialGroups{group1, group1a};
  core.Execute(DBEmptyGroupsCommand::Create(&core, initialGroups,
                                            DBEmptyGroupsCommand::EG_REPLACEALL));
  core.ClearCommands();
  observer.calls.clear();

  // Single-group add, delete, and rename, including undo.
  auto cmd = DBEmptyGroupsCommand::Create(&core, group2, DBEmptyGroupsCommand::EG_ADD);
  // same as in the UpdateGUICommand command test, execute the command directly to avoid the UI notifications
  // generated by core - these are out of scope for this test
  cmd->Execute();
  EXPECT_EQ((std::vector{group1, group1a, group2}), core.GetEmptyGroups());
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_REFRESH_TREE, observer.calls[0].ga);
  observer.calls.clear();

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_REFRESH_TREE, observer.calls[0].ga);
  observer.calls.clear();
  delete cmd; // delete manually since we didn't use core.Execute, so core isn't tracking this command

  cmd = DBEmptyGroupsCommand::Create(&core, group1, DBEmptyGroupsCommand::EG_DELETE);
  cmd->Execute();
  EXPECT_EQ(std::vector{group1a}, core.GetEmptyGroups());

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  delete cmd;

  const StringX renamedGroup1a(L"Renamed.Subgroup");
  cmd = DBEmptyGroupsCommand::Create(&core, group1a, renamedGroup1a,
                                     DBEmptyGroupsCommand::EG_RENAME);
  cmd->Execute();
  EXPECT_EQ((std::vector{group1, renamedGroup1a}), core.GetEmptyGroups());

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  delete cmd;

  // Add all preserves existing groups; replace all and rename path replace them.
  const StringX group3(L"Group3");
  const std::vector addGroups{group2, group3};
  cmd = DBEmptyGroupsCommand::Create(&core, addGroups, DBEmptyGroupsCommand::EG_ADDALL);
  cmd->Execute();
  EXPECT_EQ((std::vector{group1, group1a, group2, group3}), core.GetEmptyGroups());

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  delete cmd;

  const std::vector replacementGroups{group2};
  cmd = DBEmptyGroupsCommand::Create(&core, replacementGroups,
                                     DBEmptyGroupsCommand::EG_REPLACEALL);
  cmd->Execute();
  EXPECT_EQ(replacementGroups, core.GetEmptyGroups());

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  delete cmd;

  cmd = DBEmptyGroupsCommand::Create(&core, StringX(L"Group1"), StringX(L"NewGroup"),
                                     DBEmptyGroupsCommand::EG_RENAMEPATH);
  cmd->Execute();
  EXPECT_EQ((std::vector{group1, StringX(L"NewGroup.Subgroup")}),
            core.GetEmptyGroups());

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  delete cmd;

  // Duplicate add and identical replacement are no-ops and do not notify.
  observer.calls.clear();
  cmd = DBEmptyGroupsCommand::Create(&core, group1, DBEmptyGroupsCommand::EG_ADD);
  cmd->Execute();
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  cmd = DBEmptyGroupsCommand::Create(&core, initialGroups,
                                     DBEmptyGroupsCommand::EG_REPLACEALL);
  cmd->Execute();
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  observer.calls.clear();
  core.SetReadOnly(true);
  cmd = DBEmptyGroupsCommand::Create(&core, group3, DBEmptyGroupsCommand::EG_ADD);
  cmd->Execute();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  EXPECT_TRUE(observer.calls.empty());

  cmd->Undo();
  EXPECT_EQ(initialGroups, core.GetEmptyGroups());
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  core.SetReadOnly(false);
  cmd = DBEmptyGroupsCommand::Create(&core, group3, DBEmptyGroupsCommand::EG_ADD);
  cmd->SetNoGUINotify();
  cmd->Execute();
  EXPECT_EQ((std::vector{group1, group1a, group3}), core.GetEmptyGroups());
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  core.UnregisterObserver(&observer);
  core.ClearCommands();
}

TEST_F(CommandsTest, DBFiltersCommand)
{
  PWScore core;
  TestGUIObserver observer;
  core.RegisterObserver(&observer);

  st_Filterkey key{FPOOL_DATABASE, L"Database filter"};
  st_filters filter;
  filter.fname = L"Database filter";
  PWSFilters newFilters;
  newFilters[key] = filter;

  auto cmd = DBFiltersCommand::Create(&core, newFilters);
  // same as in the UpdateGUICommand command test, execute the command directly to avoid the UI notifications
  // generated by core - these are out of scope for this test
  cmd->Execute();
  EXPECT_EQ(newFilters, core.GetDBFilters());
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_UPDATE_STATUSBAR, observer.calls[0].ga);
  observer.calls.clear();

  cmd->Undo();
  EXPECT_TRUE(core.GetDBFilters().empty());
  ASSERT_EQ(1U, observer.calls.size());
  EXPECT_EQ(UpdateGUICommand::GUI_UPDATE_STATUSBAR, observer.calls[0].ga);
  observer.calls.clear();
  delete cmd; // delete manually since we didn't use core.Execute, so core isn't tracking this command

  // An unchanged map is a no-op
  cmd = DBFiltersCommand::Create(&core, core.GetDBFilters());
  cmd->Execute();
  EXPECT_TRUE(observer.calls.empty());

  cmd->Undo();
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  core.SetReadOnly(true);
  cmd = DBFiltersCommand::Create(&core, newFilters);
  cmd->Execute();
  EXPECT_TRUE(core.GetDBFilters().empty());
  EXPECT_TRUE(observer.calls.empty());

  cmd->Undo();
  EXPECT_TRUE(core.GetDBFilters().empty());
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  core.SetReadOnly(false);
  cmd = DBFiltersCommand::Create(&core, newFilters);
  cmd->SetNoGUINotify();
  cmd->Execute();
  EXPECT_EQ(newFilters, core.GetDBFilters());
  EXPECT_TRUE(observer.calls.empty());

  cmd->Undo();
  EXPECT_TRUE(core.GetDBFilters().empty());
  EXPECT_TRUE(observer.calls.empty());
  delete cmd;

  core.UnregisterObserver(&observer);
  core.ClearCommands();
}

