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
#include "core/PWHistory.h"
#include "core/core.h"

#include "gtest/gtest.h"

#include <vector>

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

  // A fixture for factoring common code across tests
  class CommandsTest : public testing::Test
  {
  protected:
    PWScore core;
    TestGUIObserver observer;

    void SetUp() override {
      core.RegisterObserver(&observer);
    }

    void TearDown() override {
      core.UnregisterObserver(&observer);
      core.ClearCommands();
    }

    // Tests the full Execute/Undo/Redo cycle of a command.
    // Expects no command-specific GUI actions to be emitted.
    // Will dispose of `cmd` at the end.
    void TestExecute(Command *cmd,
                     const std::function<void()> &assertInitial,
                     const std::function<void()> &assertOnChange) {
      DoTest("Execute", cmd, assertInitial, assertOnChange);
    }

    // Tests the full Execute/Undo/Redo cycle of a command. This version expects a command-specific GUI action
    // to be emitted after each step.
    // Will dispose of `cmd` at the end.
    void TestExecute(Command *cmd,
                     const std::function<void()> &assertInitial,
                     const std::function<void()> &assertOnChange,
                     const UpdateGUICommand::GUI_Action emittedAction) {
      DoTest("Execute", cmd, assertInitial, assertOnChange, {emittedAction}, {emittedAction});
    }

    // Tests the full Execute/Undo/Redo cycle of a command. The version supports a separate set of GUI actions
    // emitted after Execute and Undo.
    // Will dispose of `cmd` at the end.
    void TestExecute(Command *cmd,
                     const std::function<void()> &assertInitial,
                     const std::function<void()> &assertOnChange,
                     const std::vector<UpdateGUICommand::GUI_Action> &actionsOnExecute,
                     const std::vector<UpdateGUICommand::GUI_Action> &actionsOnUndo = {}) {
      DoTest("Execute", cmd, assertInitial, assertOnChange, actionsOnExecute, actionsOnUndo);
    }

    // Tests the full Execute/Undo/Redo cycle of a command when core is set to read-only.
    // There's only one `assertion` which is expected to hold after each operation.
    // Will dispose of `cmd` at the end.
    void TestReadonly(Command *cmd, const std::function<void()> &assertion) {
      core.SetReadOnly(true);
      DoTest("Readonly", cmd, assertion, assertion);
      core.SetReadOnly(false);
    }

    // Tests the full Execute/Undo/Redo cycle of a command with GUI notifications disabled.
    // Will dispose of `cmd` at the end.
    void TestNoGuiNotify(Command *cmd,
                         const std::function<void()> &assertInitial,
                         const std::function<void()> &assertOnChange) {
      cmd->SetNoGUINotify();
      DoTest("No GUI notify", cmd, assertInitial, assertOnChange);
    }

  private:
    // Asserts that the expected command-specific GUI actions and core's default
    // `GUI_UPDATE_STATUSBAR` update (always last) were emitted.
    void AssertGuiCalls(const std::vector<UpdateGUICommand::GUI_Action> &emittedActions) {
      const size_t actionCount = observer.calls.size() - 1;
      EXPECT_EQ(emittedActions.size(), actionCount);
      for (size_t i = 0; i < emittedActions.size() && i < actionCount; ++i) {
        EXPECT_EQ(emittedActions[i], observer.calls[i].ga);
      }
      EXPECT_EQ(UpdateGUICommand::GUI_UPDATE_STATUSBAR, observer.calls.back().ga);
      observer.calls.clear();
    }

    void DoTest(const std::string &phase,
                Command *cmd,
                const std::function<void()> &assertInitial,
                const std::function<void()> &assertOnChange,
                const std::vector<UpdateGUICommand::GUI_Action> &actionsOnExecute = {},
                const std::vector<UpdateGUICommand::GUI_Action> &actionsOnUndo = {}) {
      observer.calls.clear();
      {
        SCOPED_TRACE(phase + " - precondition");
        assertInitial();
      }

      {
        SCOPED_TRACE(phase + " - on change");
        core.Execute(cmd);
        assertOnChange();
        AssertGuiCalls(actionsOnExecute);
      }

      {
        SCOPED_TRACE(phase + " - undo");
        core.Undo();
        assertInitial();
        AssertGuiCalls(actionsOnUndo);
      }

      {
        SCOPED_TRACE(phase + " - redo");
        core.Redo();
        assertOnChange();
        AssertGuiCalls(actionsOnExecute);
      }

      core.ClearCommands();
    }
  };
}

// And now the tests...

TEST_F(CommandsTest, AddItem)
{
  CItemData di;
  di.CreateUUID();
  di.SetTitle(L"a title");
  di.SetPassword(L"a password");
  const pws_os::CUUID uuid = di.GetUUID();

  {
    SCOPED_TRACE("add an entry and undo/redo it");
    auto assertInitial = [this] {
      EXPECT_EQ(0U, core.GetNumEntries());
      EXPECT_FALSE(core.HasDBChanged());
    };
    auto assertOnChange = [this, &di, &uuid] {
      const auto iter = core.Find(uuid);
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(di, core.GetEntry(iter));
      EXPECT_TRUE(core.HasDBChanged());
    };
    TestExecute(AddEntryCommand::Create(&core, di), assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_ADD_ENTRY}, {UpdateGUICommand::GUI_DELETE_ENTRY});
  }

  CItemData readOnlyItem;
  readOnlyItem.CreateUUID();
  readOnlyItem.SetTitle(L"read-only entry");
  {
    SCOPED_TRACE("read-only core ignores entry additions");
    auto assertion = [this] { EXPECT_EQ(1U, core.GetNumEntries()); };
    TestReadonly(AddEntryCommand::Create(&core, readOnlyItem), assertion);
  }

  CItemData noNotifyItem;
  noNotifyItem.CreateUUID();
  noNotifyItem.SetTitle(L"no-notify entry");
  {
    SCOPED_TRACE("entry addition with GUI notifications disabled");
    auto assertInitial = [this] { EXPECT_EQ(1U, core.GetNumEntries()); };
    auto assertOnChange = [this] { EXPECT_EQ(2U, core.GetNumEntries()); };
    TestNoGuiNotify(AddEntryCommand::Create(&core, noNotifyItem), assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, DeleteEntry)
{
  CItemData ci;
  ci.CreateUUID();
  ci.SetTitle(L"blue rabbit");
  ci.SetPassword(L"notagain");
  core.Execute(AddEntryCommand::Create(&core, ci));

  {
    SCOPED_TRACE("delete an entry and undo/redo the deletion");
    auto assertInitial = [this] { EXPECT_EQ(1U, core.GetNumEntries()); };
    auto assertOnChange = [this] { EXPECT_EQ(0U, core.GetNumEntries()); };
    TestExecute(DeleteEntryCommand::Create(&core, ci), assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_DELETE_ENTRY}, {UpdateGUICommand::GUI_ADD_ENTRY});
  }

  core.Execute(AddEntryCommand::Create(&core, ci));
  {
    SCOPED_TRACE("read-only core ignores entry deletion");
    auto assertion = [this] { EXPECT_EQ(1U, core.GetNumEntries()); };
    TestReadonly(DeleteEntryCommand::Create(&core, ci), assertion);
  }

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
  {
    SCOPED_TRACE("deleting an alias base converts its dependent to a normal entry");
    auto assertInitial = [this, &abase, &al] {
      EXPECT_EQ(3U, core.GetNumEntries());
      EXPECT_TRUE(core.GetEntry(core.Find(abase.GetUUID())).IsAliasBase());
      EXPECT_TRUE(core.GetEntry(core.Find(al.GetUUID())).IsAlias());
    };
    auto assertOnChange = [this, &al] {
      EXPECT_EQ(2U, core.GetNumEntries());
      EXPECT_TRUE(core.GetEntry(core.Find(al.GetUUID())).IsNormal());
    };
    TestExecute(DeleteEntryCommand::Create(&core, core.GetEntry(core.Find(abase.GetUUID()))),
                assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_DELETE_ENTRY},
                {UpdateGUICommand::GUI_ADD_ENTRY,
                 UpdateGUICommand::GUI_DELETE_ENTRY,
                 UpdateGUICommand::GUI_ADD_ENTRY});
 }

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
  {
    SCOPED_TRACE("undoing dependent deletion does not duplicate an existing entry");
    const CItemData sdepCore = core.GetEntry(core.Find(sdep.GetUUID()));
    auto deleteCommand = DeleteEntryCommand::Create(&core, sdepCore);
    core.Execute(deleteCommand);
    core.Execute(AddEntryCommand::Create(&core, sdepCore, sbase.GetUUID()));
    EXPECT_EQ(4U, core.GetNumEntries());
    deleteCommand->Undo();
    EXPECT_EQ(4U, core.GetNumEntries());
    EXPECT_NE(core.GetEntryEndIter(), core.Find(sdep.GetUUID()));
    core.ClearCommands();
  }

  {
    SCOPED_TRACE("undoing deletion of base when dependent alias is no longer present only restores the base");
    CItemData missingDependentBase, missingDependentAlias;
    missingDependentBase.CreateUUID();
    missingDependentBase.SetTitle(L"base entry with deleted dependent");
    missingDependentBase.SetPassword(L"base password");
    missingDependentAlias.CreateUUID();
    missingDependentAlias.SetTitle(L"alias deleted before base undo");
    missingDependentAlias.SetPassword(L"[Alias]");
    missingDependentAlias.SetAlias();

    MultiCommands *aliasCommands = MultiCommands::Create(&core);
    aliasCommands->Add(AddEntryCommand::Create(&core, missingDependentBase));
    aliasCommands->Add(AddEntryCommand::Create(&core, missingDependentAlias, missingDependentBase.GetUUID()));
    core.Execute(aliasCommands);

    auto deleteBaseCommand = DeleteEntryCommand::Create(
      &core, core.GetEntry(core.Find(missingDependentBase.GetUUID())));
    core.Execute(deleteBaseCommand);
    ASSERT_NE(core.GetEntryEndIter(), core.Find(missingDependentAlias.GetUUID()));
    EXPECT_TRUE(core.GetEntry(core.Find(missingDependentAlias.GetUUID())).IsNormal());

    core.Execute(DeleteEntryCommand::Create(
      &core, core.GetEntry(core.Find(missingDependentAlias.GetUUID()))));
    EXPECT_EQ(core.GetEntryEndIter(), core.Find(missingDependentAlias.GetUUID()));

    deleteBaseCommand->Undo();
    EXPECT_NE(core.GetEntryEndIter(), core.Find(missingDependentBase.GetUUID()));
    EXPECT_EQ(core.GetEntryEndIter(), core.Find(missingDependentAlias.GetUUID()));
    core.ClearCommands();
  }

  {
    SCOPED_TRACE("delete an entry with GUI notifications disabled");
    auto assertInitial = [this, &ci] { EXPECT_NE(core.GetEntryEndIter(), core.Find(ci.GetUUID())); };
    auto assertOnChange = [this, &ci] { EXPECT_EQ(core.GetEntryEndIter(), core.Find(ci.GetUUID())); };
    TestNoGuiNotify(DeleteEntryCommand::Create(&core, core.GetEntry(core.Find(ci.GetUUID()))),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, DeleteEntryWithAttachment)
{
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
  {
    SCOPED_TRACE("delete an entry and its attachment");
    auto assertInitial = [this] {
      EXPECT_EQ(1U, core.GetNumEntries());
      EXPECT_EQ(1U, core.GetNumAtts());
    };
    auto assertOnChange = [this] {
      EXPECT_EQ(0U, core.GetNumEntries());
      EXPECT_EQ(0U, core.GetNumAtts());
    };
    TestExecute(DeleteEntryCommand::Create(&core, ci), assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_DELETE_ENTRY}, {UpdateGUICommand::GUI_ADD_ENTRY});
  }
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
  const pws_os::CUUID shortcut_uuid = si.GetUUID();

  time_t t;
  time(&t);
  si.SetCTime(t);
  si.SetXTime(time_t{0});
  si.SetStatus(CItemData::ES_ADDED);

  const auto addShortcut = [this, &bi, &si, &base_uuid] {
    const auto commands = MultiCommands::Create(&core);
    commands->Add(AddEntryCommand::Create(&core, bi));
    commands->Add(AddEntryCommand::Create(&core, si, base_uuid));
    return commands;
  };
  {
    SCOPED_TRACE("create a shortcut and mark its base entry");
    auto assertInitial = [this] { EXPECT_EQ(0U, core.GetNumEntries()); };
    auto assertOnChange = [this, &base_uuid, &shortcut_uuid] {
      EXPECT_EQ(2U, core.GetNumEntries());
      EXPECT_TRUE(core.GetEntry(core.Find(base_uuid)).IsShortcutBase());
      EXPECT_TRUE(core.GetEntry(core.Find(shortcut_uuid)).IsShortcut());
      EXPECT_TRUE(core.HasDBChanged());
    };
    TestExecute(addShortcut(), assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_ADD_ENTRY, UpdateGUICommand::GUI_ADD_ENTRY},
                {UpdateGUICommand::GUI_DELETE_ENTRY, UpdateGUICommand::GUI_DELETE_ENTRY,
                 UpdateGUICommand::GUI_REFRESH_TREE});
  }

  {
    SCOPED_TRACE("deleting a shortcut base also deletes its dependent shortcut");
    auto assertInitial = [this] { EXPECT_EQ(2U, core.GetNumEntries()); };
    auto assertOnChange = [this] {
      EXPECT_EQ(0U, core.GetNumEntries());
      EXPECT_TRUE(core.HasDBChanged());
    };
    TestExecute(DeleteEntryCommand::Create(&core, core.GetEntry(core.Find(base_uuid))),
                assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_DELETE_ENTRY},
                {UpdateGUICommand::GUI_ADD_ENTRY, UpdateGUICommand::GUI_ADD_ENTRY});
  }

  {
    SCOPED_TRACE("deleting only a shortcut restores a normal base entry");
    auto assertInitial = [this, &base_uuid, &shortcut_uuid] {
      EXPECT_EQ(2U, core.GetNumEntries());
      EXPECT_TRUE(core.GetEntry(core.Find(base_uuid)).IsShortcutBase());
      EXPECT_NE(core.GetEntryEndIter(), core.Find(shortcut_uuid));
    };
    auto assertOnChange = [this, &base_uuid, &shortcut_uuid] {
      ASSERT_EQ(1U, core.GetNumEntries());
      EXPECT_TRUE(core.GetEntry(core.Find(base_uuid)).IsNormal());
      EXPECT_EQ(core.GetEntryEndIter(), core.Find(shortcut_uuid));
      EXPECT_TRUE(core.HasDBChanged());
    };
    core.Execute(addShortcut());
    TestExecute(DeleteEntryCommand::Create(&core, core.GetEntry(core.Find(shortcut_uuid))),
                assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_DELETE_ENTRY}, {UpdateGUICommand::GUI_ADD_ENTRY});
  }
}

TEST_F(CommandsTest, EditEntry)
{
  CItemData it;
  it.CreateUUID();
  it.SetTitle(L"NoDrama");
  it.SetPassword(L"PolishTrumpetsSq4are");

  core.Execute(AddEntryCommand::Create(&core, it));

  CItemData editedItem(it);
  StringX editedTitle(L"NoDramamine");
  editedItem.SetTitle(editedTitle);
  {
    SCOPED_TRACE("edit an entry and undo/redo the edit");
    auto assertInitial = [this, &it] {
      const auto iter = core.Find(it.GetUUID());
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(it.GetTitle(), core.GetEntry(iter).GetTitle());
    };
    auto assertOnChange = [this, &it, &editedItem] {
      const auto iter = core.Find(it.GetUUID());
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(editedItem.GetTitle(), core.GetEntry(iter).GetTitle());
      EXPECT_TRUE(core.HasDBChanged());
    };
    TestExecute(EditEntryCommand::Create(&core, it, editedItem), assertInitial, assertOnChange,
                UpdateGUICommand::GUI_REFRESH_ENTRY);
  }

  {
    SCOPED_TRACE("read-only core ignores entry edits");
    const CItemData currentItem = core.GetEntry(core.Find(it.GetUUID()));
    CItemData attemptedEdit(currentItem);
    attemptedEdit.SetTitle(L"ReadOnlyEdit");
    auto assertion = [this, &it, &editedTitle] {
      EXPECT_EQ(editedTitle, core.GetEntry(core.Find(it.GetUUID())).GetTitle());
    };
    TestReadonly(EditEntryCommand::Create(&core, currentItem, attemptedEdit), assertion);
  }

  {
    SCOPED_TRACE("entry edit with GUI notifications disabled");
    const CItemData currentItem = core.GetEntry(core.Find(it.GetUUID()));
    CItemData noNotifyEdit(currentItem);
    noNotifyEdit.SetTitle(L"NoNotifyEdit");
    auto assertInitial = [this, &it, &editedTitle] {
      EXPECT_EQ(editedTitle, core.GetEntry(core.Find(it.GetUUID())).GetTitle());
    };
    auto assertOnChange = [this, &it] {
      EXPECT_EQ(L"NoNotifyEdit", core.GetEntry(core.Find(it.GetUUID())).GetTitle());
    };
    TestNoGuiNotify(EditEntryCommand::Create(&core, currentItem, noNotifyEdit),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, RenameGroup)
{
  const StringX groupAlpha(L"Group0.Alpha");
  const StringX groupBeta(L"Group0.Beta");
  const StringX groupGamma(L"Group0.Gamma");
  CItemData di;
  di.CreateUUID();
  di.SetTitle(L"b title");
  di.SetPassword(L"b password");
  di.SetGroup(groupAlpha);
  const pws_os::CUUID uuid = di.GetUUID();

  core.Execute(AddEntryCommand::Create(&core, di));
  {
    SCOPED_TRACE("rename a group and undo/redo the rename");
    auto assertInitial = [this, &uuid, &groupAlpha] {
      const auto iter = core.Find(uuid);
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(groupAlpha, core.GetEntry(iter).GetGroup());
    };
    auto assertOnChange = [this, &uuid, &groupBeta] {
      const auto iter = core.Find(uuid);
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(groupBeta, core.GetEntry(iter).GetGroup());
    };
    TestExecute(RenameGroupCommand::Create(&core, groupAlpha, groupBeta),
                assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_REFRESH_TREE},
                {UpdateGUICommand::GUI_REFRESH_TREE, UpdateGUICommand::GUI_REFRESH_TREE});
  }

  {
    SCOPED_TRACE("read-only core ignores group renames");
    auto assertion = [this, &uuid, &groupBeta] {
      EXPECT_EQ(groupBeta, core.GetEntry(core.Find(uuid)).GetGroup());
    };
    TestReadonly(RenameGroupCommand::Create(&core, groupBeta, groupGamma), assertion);
  }

  {
    SCOPED_TRACE("group rename with GUI notifications disabled");
    auto assertInitial = [this, &uuid, &groupBeta] {
      EXPECT_EQ(groupBeta, core.GetEntry(core.Find(uuid)).GetGroup());
    };
    auto assertOnChange = [this, &uuid, &groupGamma] {
      EXPECT_EQ(groupGamma, core.GetEntry(core.Find(uuid)).GetGroup());
    };
    TestNoGuiNotify(RenameGroupCommand::Create(&core, groupBeta, groupGamma),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, CountGroups)
{
  CItemData di;
  std::vector<stringT> vGroups;

  di.CreateUUID();
  di.SetTitle(L"b title");
  di.SetPassword(L"b password");
  const pws_os::CUUID uuid = di.GetUUID();

  core.Execute(AddEntryCommand::Create(&core, di));
  CItemData groupedItem = di;
  groupedItem.SetGroup(L"g1");
  UpdateGUICommand::GUI_Action guiAction = UpdateGUICommand::GUI_REFRESH_TREE;
  {
    SCOPED_TRACE("add a group to an entry");
    auto assertInitial = [this, &vGroups] {
      core.GetAllGroups(vGroups);
      EXPECT_TRUE(vGroups.empty());
    };
    auto assertOnChange = [this, &vGroups] {
      core.GetAllGroups(vGroups);
      EXPECT_EQ(1U, vGroups.size());
    };
    TestExecute(EditEntryCommand::Create(&core, di, groupedItem), assertInitial, assertOnChange,
                guiAction);
  }

  CItemData subgroupItem = groupedItem;
  subgroupItem.SetGroup(L"g1.g1-1");
  {
    SCOPED_TRACE("add a subgroup to an entry");
    auto assertInitial = [this, &vGroups] {
      core.GetAllGroups(vGroups);
      EXPECT_EQ(1U, vGroups.size());
    };
    auto assertOnChange = [this, &vGroups] {
      core.GetAllGroups(vGroups);
      EXPECT_EQ(2U, vGroups.size());
    };
    TestExecute(EditEntryCommand::Create(&core, groupedItem, subgroupItem), assertInitial, assertOnChange,
                guiAction);
  }

  {
    SCOPED_TRACE("add an empty group");
    const std::vector<StringX> emptyGroups{L"e1"};
    auto assertInitial = [this, &vGroups] {
      core.GetAllGroups(vGroups);
      EXPECT_EQ(2U, vGroups.size());
    };
    auto assertOnChange = [this, &vGroups] {
      core.GetAllGroups(vGroups);
      EXPECT_EQ(3U, vGroups.size());
    };
    TestExecute(DBEmptyGroupsCommand::Create(&core, emptyGroups, DBEmptyGroupsCommand::EG_ADDALL),
                assertInitial, assertOnChange, guiAction);
  }
}

TEST_F(CommandsTest, UpdatePassword)
{
  const int32 i1day = 86400; // 24 * 60 * 60 seconds
  const StringX sxOldPassword(L"MoreWideF1ns");
  const StringX sxNewPassword(L"ManifestQuin1ne");

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

  core.Execute(AddEntryCommand::Create(&core, it));
  {
    SCOPED_TRACE("update password, expiration state, and password history");
    auto assertInitial = [this, &sxOldPassword, &it] {
      const CItemData entry = core.GetEntry(core.Find(it.GetUUID()));
      EXPECT_EQ(sxOldPassword, entry.GetPassword());
      EXPECT_TRUE(entry.IsExpired());
      const PWHistList history(entry.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
      EXPECT_EQ(0U, history.size());
      EXPECT_TRUE(history.isSaving());
      EXPECT_EQ(0U, history.getErr());
      EXPECT_EQ(3U, history.getMax());
    };
    auto assertOnChange = [this, &sxOldPassword, &sxNewPassword, tPMtime, &it] {
      const CItemData entry = core.GetEntry(core.Find(it.GetUUID()));
      EXPECT_EQ(sxNewPassword, entry.GetPassword());
      EXPECT_FALSE(entry.IsExpired());
      time_t modificationTime;
      entry.GetPMTime(modificationTime);
      const PWHistList history(entry.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
      EXPECT_TRUE(history.isSaving());
      ASSERT_EQ(1U, history.size());
      EXPECT_EQ(0U, history.getErr());
      EXPECT_EQ(3U, history.getMax());
      EXPECT_EQ(sxOldPassword, history[0].password);
      // The last entry's modification time is either the previous password modification time (after Execute)
      // or the time when Redo was performed.
      EXPECT_TRUE(tPMtime == history[0].changetttdate || modificationTime == history[0].changetttdate);
    };
    TestExecute(UpdatePasswordCommand::Create(&core, it, sxNewPassword),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD);
    const CItemData &entry = core.GetEntry(core.Find(it.GetUUID()));
    time_t modificationTime;
    entry.GetPMTime(modificationTime);
    PWHistList history(entry.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    // New password change time is that of when Redo is performed & not original time
    EXPECT_EQ(modificationTime, history[0].changetttdate);
  }

  {
    SCOPED_TRACE("read-only core ignores password updates");
    const CItemData currentItem = core.GetEntry(core.Find(it.GetUUID()));
    auto assertion = [this, &it, &sxNewPassword] {
      EXPECT_EQ(sxNewPassword, core.GetEntry(core.Find(it.GetUUID())).GetPassword());
    };
    TestReadonly(UpdatePasswordCommand::Create(&core, currentItem, sxOldPassword), assertion);
  }

  {
    SCOPED_TRACE("password update with GUI notifications disabled");
    const CItemData currentItem = core.GetEntry(core.Find(it.GetUUID()));
    const StringX nextPassword(L"NextPassword");
    auto assertInitial = [this, &it, &sxNewPassword] {
      EXPECT_EQ(sxNewPassword, core.GetEntry(core.Find(it.GetUUID())).GetPassword());
    };
    auto assertOnChange = [this, &it, &nextPassword] {
      EXPECT_EQ(nextPassword, core.GetEntry(core.Find(it.GetUUID())).GetPassword());
    };
    TestNoGuiNotify(UpdatePasswordCommand::Create(&core, currentItem, nextPassword),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, UpdatePasswordHistory)
{
  const StringX historyOnMaxThree(L"10300");
  const StringX historyOnMaxTwo(L"10200");
  const StringX historyOff(L"00200");
  CItemData item, protectedItem;
  item.CreateUUID();
  const pws_os::CUUID itemUuid = item.GetUUID();
  item.SetPWHistory(L"");
  protectedItem.CreateUUID();
  const pws_os::CUUID protectedItemUuid = protectedItem.GetUUID();
  protectedItem.SetPWHistory(historyOnMaxThree);
  protectedItem.SetProtected(true);

  auto addCmd = MultiCommands::Create(&core);
  addCmd->Add(AddEntryCommand::Create(&core, item));
  addCmd->Add(AddEntryCommand::Create(&core, protectedItem));
  core.Execute(addCmd);

  {
    SCOPED_TRACE("reset history for unprotected entries only");
    auto assertInitial = [this, &itemUuid, &protectedItemUuid, &historyOnMaxThree] {
      EXPECT_TRUE(core.GetEntry(core.Find(itemUuid)).GetPWHistory().empty());
      EXPECT_EQ(historyOnMaxThree, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    auto assertOnChange = [this, &itemUuid, &protectedItemUuid, &historyOnMaxThree] {
      EXPECT_EQ(L"10500", core.GetEntry(core.Find(itemUuid)).GetPWHistory());
      EXPECT_EQ(historyOnMaxThree, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    TestExecute(UpdatePasswordHistoryCommand::Create(&core, PWHist::START_EXCL_PROT, 5),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("set the maximum history size including protected entries");
    auto assertInitial = [this, &itemUuid, &protectedItemUuid, &historyOnMaxThree] {
      EXPECT_EQ(L"10500", core.GetEntry(core.Find(itemUuid)).GetPWHistory());
      EXPECT_EQ(historyOnMaxThree, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    auto assertOnChange = [this, &itemUuid, &protectedItemUuid, &historyOnMaxTwo] {
      EXPECT_EQ(historyOnMaxTwo, core.GetEntry(core.Find(itemUuid)).GetPWHistory());
      EXPECT_EQ(historyOnMaxTwo, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    TestExecute(UpdatePasswordHistoryCommand::Create(&core, PWHist::SETMAX_INCL_PROT, 2),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("stop saving history for unprotected entries only");
    auto assertInitial = [this, &itemUuid, &protectedItemUuid, &historyOnMaxTwo] {
      EXPECT_EQ(historyOnMaxTwo, core.GetEntry(core.Find(itemUuid)).GetPWHistory());
      EXPECT_EQ(historyOnMaxTwo, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    auto assertOnChange = [this, &itemUuid, &protectedItemUuid, &historyOff, &historyOnMaxTwo] {
      EXPECT_EQ(historyOff, core.GetEntry(core.Find(itemUuid)).GetPWHistory());
      EXPECT_EQ(historyOnMaxTwo, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    TestExecute(UpdatePasswordHistoryCommand::Create(&core, PWHist::STOP_EXCL_PROT, 0),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("clear history including protected entries");
    auto assertInitial = [this, &itemUuid, &protectedItemUuid, &historyOff, &historyOnMaxTwo] {
      EXPECT_EQ(historyOff, core.GetEntry(core.Find(itemUuid)).GetPWHistory());
      EXPECT_EQ(historyOnMaxTwo, core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory());
    };
    auto assertOnChange = [this, &itemUuid, &protectedItemUuid] {
      EXPECT_TRUE(core.GetEntry(core.Find(itemUuid)).GetPWHistory().empty());
      EXPECT_TRUE(core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory().empty());
    };
    TestExecute(UpdatePasswordHistoryCommand::Create(&core, PWHist::CLEAR_INCL_PROT, 0),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores password history updates");
    auto assertion = [this, &itemUuid, &protectedItemUuid] {
      EXPECT_TRUE(core.GetEntry(core.Find(itemUuid)).GetPWHistory().empty());
      EXPECT_TRUE(core.GetEntry(core.Find(protectedItemUuid)).GetPWHistory().empty());
    };
    TestReadonly(UpdatePasswordHistoryCommand::Create(&core, PWHist::SETMAX_INCL_PROT, 2), assertion);
  }
}

TEST_F(CommandsTest, ChangeDBHeaderCommand)
{
  const StringX sxNewName(L"MyDatabase");
  {
    SCOPED_TRACE("change the database name and undo/redo the change");
    auto assertInitial = [this] { EXPECT_TRUE(core.GetHeaderItem(PWSfile::HDR_DBNAME).empty()); };
    auto assertOnChange = [this, &sxNewName] {
      EXPECT_EQ(sxNewName, core.GetHeaderItem(PWSfile::HDR_DBNAME));
      EXPECT_TRUE(core.HasDBChanged());
    };
    TestExecute(ChangeDBHeaderCommand::Create(&core, sxNewName, PWSfile::HDR_DBNAME),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("change the database description and undo/redo the change");
    auto assertInitial = [this] { EXPECT_TRUE(core.GetHeaderItem(PWSfile::HDR_DBDESC).empty()); };
    auto assertOnChange = [this] { EXPECT_EQ(StringX(L"A description"), core.GetHeaderItem(PWSfile::HDR_DBDESC)); };
    TestExecute(ChangeDBHeaderCommand::Create(&core, L"A description", PWSfile::HDR_DBDESC),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("repeating the same header value is a no-op");
    auto assertion = [this, &sxNewName] { EXPECT_EQ(sxNewName, core.GetHeaderItem(PWSfile::HDR_DBNAME)); };
    TestExecute(ChangeDBHeaderCommand::Create(&core, sxNewName, PWSfile::HDR_DBNAME), assertion, assertion);
  }

  {
    SCOPED_TRACE("read-only core ignores database-header changes");
    auto assertion = [this, &sxNewName] { EXPECT_EQ(sxNewName, core.GetHeaderItem(PWSfile::HDR_DBNAME)); };
    TestReadonly(ChangeDBHeaderCommand::Create(&core, L"ReadOnlyAttempt", PWSfile::HDR_DBNAME), assertion);
  }
}

TEST_F(CommandsTest, UpdateEntry)
{
  CItemData it;
  it.CreateUUID();
  time_t t;
  time(&t);
  it.SetCTime(t);
  it.SetTitle(L"RedC1gar");
  it.SetPassword(L"EarlyR1zer");

  core.Execute(AddEntryCommand::Create(&core, it));
  const StringX newTitle(L"PastaFar1an");
  {
    SCOPED_TRACE("update the entry title");
    auto assertTitle = [this, &it](const StringX &title) {
      const auto iter = core.Find(it.GetUUID());
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(title, core.GetEntry(iter).GetTitle());
    };
    auto assertInitial = [assertTitle, &it] { assertTitle(it.GetTitle()); };
    auto assertOnChange = [assertTitle, &newTitle] { assertTitle(newTitle); };
    TestExecute(UpdateEntryCommand::Create(&core, it, CItem::TITLE, newTitle),
                assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_REFRESH_ENTRYFIELD},
                {UpdateGUICommand::GUI_REFRESH_ENTRYFIELD, UpdateGUICommand::GUI_REFRESH_ENTRY});
  }

  const StringX newPassword(L"F1nallyUpdated");
  it.SetTitle(newTitle);
  {
    SCOPED_TRACE("update the entry password");
    auto assertPassword = [this, &it](const StringX &password) {
      const auto iter = core.Find(it.GetUUID());
      ASSERT_NE(core.GetEntryEndIter(), iter);
      EXPECT_EQ(password, core.GetEntry(iter).GetPassword());
    };
    auto assertInitial = [assertPassword, &it] { assertPassword(it.GetPassword()); };
    auto assertOnChange = [assertPassword, &newPassword] { assertPassword(newPassword); };
    TestExecute(UpdateEntryCommand::Create(&core, it, CItem::PASSWORD, newPassword),
                assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_REFRESH_ENTRYFIELD},
                {UpdateGUICommand::GUI_REFRESH_ENTRYFIELD, UpdateGUICommand::GUI_REFRESH_ENTRY});
  }

  {
    SCOPED_TRACE("read-only core ignores entry-field updates");
    const auto iter = core.Find(it.GetUUID());
    const CItemData currentItem = core.GetEntry(iter);
    auto assertion = [this, &it, &newPassword] {
      EXPECT_EQ(newPassword, core.GetEntry(core.Find(it.GetUUID())).GetPassword());
    };
    TestReadonly(UpdateEntryCommand::Create(&core, currentItem, CItem::PASSWORD, it.GetPassword()), assertion);
  }

  {
    SCOPED_TRACE("entry-field update with GUI notifications disabled");
    const CItemData currentItem = core.GetEntry(core.Find(it.GetUUID()));
    const StringX noNotifyTitle(L"NoNotifyTitle");
    auto assertInitial = [this, &it] {
      EXPECT_EQ(L"PastaFar1an", core.GetEntry(core.Find(it.GetUUID())).GetTitle());
    };
    auto assertOnChange = [this, &it, &noNotifyTitle] {
      EXPECT_EQ(noNotifyTitle, core.GetEntry(core.Find(it.GetUUID())).GetTitle());
    };
    TestNoGuiNotify(UpdateEntryCommand::Create(&core, currentItem, CItem::TITLE, noNotifyTitle),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, MultiCommandsFindCommand)
{
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
}

TEST_F(CommandsTest, MultiCommandsInsert)
{
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

  {
    SCOPED_TRACE("MultiCommands execute, undo, redo");
    auto assertInitial = [this] { EXPECT_EQ(0U, core.GetNumEntries()); };
    auto assertOnChange = [this, &ci1, &ci2, &ci3] {
      EXPECT_EQ(3U, core.GetNumEntries());
      EXPECT_NE(core.GetEntryEndIter(), core.Find(ci1.GetUUID()));
      EXPECT_NE(core.GetEntryEndIter(), core.Find(ci2.GetUUID()));
      EXPECT_NE(core.GetEntryEndIter(), core.Find(ci3.GetUUID()));
    };
    TestExecute(pmulticmds, assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_ADD_ENTRY, UpdateGUICommand::GUI_ADD_ENTRY, UpdateGUICommand::GUI_ADD_ENTRY},
                // The extra GUI_REFRESH_TREE at the end is emitted by MultiCommands::Undo
                // even though there's no corresponding GUI_ADD_ENTRY emitted in MultiCommands::Execute
                {UpdateGUICommand::GUI_DELETE_ENTRY, UpdateGUICommand::GUI_DELETE_ENTRY,
                 UpdateGUICommand::GUI_DELETE_ENTRY, UpdateGUICommand::GUI_REFRESH_TREE});
  }
}

TEST_F(CommandsTest, MultiCommandsGetRC)
{
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
}

TEST_F(CommandsTest, UpdateGUICommand)
{
  const pws_os::CUUID testUuid;

  {
    SCOPED_TRACE("WN_UNDO: execute does not notify, undo does");
    UpdateGUICommand::GUI_Action action = UpdateGUICommand::GUI_REFRESH_ENTRY;
    TestExecute(UpdateGUICommand::Create(&core, UpdateGUICommand::WN_UNDO, action, testUuid),
                [] {}, [] {}, {}, {action});
  }

  {
    SCOPED_TRACE("WN_ALL: execute and undo notify");
    UpdateGUICommand::GUI_Action action = UpdateGUICommand::GUI_REFRESH_TREE;
    TestExecute(UpdateGUICommand::Create(&core, UpdateGUICommand::WN_ALL, action, testUuid),
                [] {}, [] {}, {action}, {action});
  }

  {
    SCOPED_TRACE("WN_EXECUTE: execute notifies, undo does not");
    UpdateGUICommand::GUI_Action action = UpdateGUICommand::GUI_ADD_ENTRY;
    TestExecute(UpdateGUICommand::Create(&core, UpdateGUICommand::WN_EXECUTE, action, testUuid),
                [] {}, [] {}, {action}, {});
  }
}

TEST_F(CommandsTest, EditAttachment)
{
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

  core.Execute(AddEntryCommand::Create(&core, ci, pws_os::CUUID::NullUUID(), &ai));
  CItemAtt oldAtt = core.GetAtt(attUuid);
  CItemAtt newAtt(oldAtt);
  const StringX updatedAttachmentTitle(L"updated attachment title");
  newAtt.SetTitle(updatedAttachmentTitle);
  {
    SCOPED_TRACE("edit an attachment title");
    auto assertInitial = [this, &attUuid] {
      EXPECT_EQ(L"original attachment title", core.GetAtt(attUuid).GetTitle());
    };
    auto assertOnChange = [this, &attUuid, &updatedAttachmentTitle] {
      EXPECT_EQ(updatedAttachmentTitle, core.GetAtt(attUuid).GetTitle());
    };
    TestExecute(EditAttachmentCommand::Create(&core, oldAtt, newAtt), assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores attachment edits");
    auto assertion = [this, &attUuid, &updatedAttachmentTitle] {
      EXPECT_EQ(updatedAttachmentTitle, core.GetAtt(attUuid).GetTitle());
    };
    TestReadonly(EditAttachmentCommand::Create(&core, oldAtt, oldAtt), assertion);
  }

  {
    SCOPED_TRACE("attachment edit with GUI notifications disabled");
    CItemAtt noNotifyAtt(core.GetAtt(attUuid));
    noNotifyAtt.SetTitle(L"no-notify attachment title");
    auto assertInitial = [this, &attUuid, &updatedAttachmentTitle] {
      EXPECT_EQ(updatedAttachmentTitle, core.GetAtt(attUuid).GetTitle());
    };
    auto assertOnChange = [this, &attUuid] {
      EXPECT_EQ(L"no-notify attachment title", core.GetAtt(attUuid).GetTitle());
    };
    TestNoGuiNotify(EditAttachmentCommand::Create(&core, core.GetAtt(attUuid), noNotifyAtt),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, DeleteAttachment)
{
  time_t cTime = 1665220859L;
  const unsigned char content[16] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                      0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
  CItemAtt attachments[3];
  CItemData entries[3];
  for (size_t i = 0; i < 3; ++i) {
    const pws_os::CUUID attUuid;
    attachments[i].SetUUID(attUuid);
    attachments[i].SetTitle(L"attachment title");
    attachments[i].SetCTime(cTime);
    attachments[i].SetContent(content, sizeof(content));

    entries[i].CreateUUID();
    entries[i].SetTitle(L"entry with attachment");
    entries[i].SetPassword(L"password123");
    entries[i].SetAttUUID(attUuid);
    entries[i].SetNormal();
    core.Execute(AddEntryCommand::Create(&core, entries[i], pws_os::CUUID::NullUUID(), &attachments[i]));
  }

  {
    SCOPED_TRACE("delete an entry attachment");
    const pws_os::CUUID attUuid = entries[0].GetAttUUID();
    const CItemData ciInCore = core.GetEntry(core.Find(entries[0].GetUUID()));
    auto assertInitial = [this, &attUuid] {
      EXPECT_TRUE(core.HasAtt(attUuid));
      EXPECT_EQ(L"attachment title", core.GetAtt(attUuid).GetTitle());
    };
    auto assertOnChange = [this, &attUuid] { EXPECT_FALSE(core.HasAtt(attUuid)); };
    TestExecute(DeleteAttachmentCommand::Create(&core, ciInCore), assertInitial, assertOnChange,
                {UpdateGUICommand::GUI_REFRESH_ENTRY}, {});
  }

  {
    SCOPED_TRACE("read-only core ignores attachment deletion");
    const pws_os::CUUID attUuid = entries[1].GetAttUUID();
    const CItemData ciInCore = core.GetEntry(core.Find(entries[1].GetUUID()));
    auto assertion = [this, &attUuid] { EXPECT_TRUE(core.HasAtt(attUuid)); };
    TestReadonly(DeleteAttachmentCommand::Create(&core, ciInCore), assertion);
  }

  {
    SCOPED_TRACE("attachment deletion with GUI notifications disabled");
    const pws_os::CUUID attUuid = entries[2].GetAttUUID();
    const CItemData ciInCore = core.GetEntry(core.Find(entries[2].GetUUID()));
    auto assertInitial = [this, &attUuid] { EXPECT_TRUE(core.HasAtt(attUuid)); };
    auto assertOnChange = [this, &attUuid] { EXPECT_FALSE(core.HasAtt(attUuid)); };
    TestNoGuiNotify(DeleteAttachmentCommand::Create(&core, ciInCore), assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, AddAndRemoveDependentEntry)
{
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

  {
    SCOPED_TRACE("remove a shortcut dependent association");
    auto assertInitial = [this, &sbase] { EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID())); };
    auto assertOnChange = [this, &sbase] { EXPECT_EQ(0U, core.NumShortcuts(sbase.GetUUID())); };
    TestExecute(RemoveDependentEntryCommand::Create(&core, sbase.GetUUID(), sdep.GetUUID(), CItemData::ET_SHORTCUT),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores shortcut dependent additions");
    auto assertion = [this, &sbase] { EXPECT_EQ(0U, core.NumShortcuts(sbase.GetUUID())); };
    TestReadonly(AddDependentEntryCommand::Create(&core, sbase.GetUUID(), sdep.GetUUID(), CItemData::ET_SHORTCUT),
                 assertion);
  }

  {
    SCOPED_TRACE("add a shortcut dependent association");
    auto assertInitial = [this, &sbase] { EXPECT_EQ(0U, core.NumShortcuts(sbase.GetUUID())); };
    auto assertOnChange = [this, &sbase] { EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID())); };
    TestExecute(AddDependentEntryCommand::Create(&core, sbase.GetUUID(), sdep.GetUUID(), CItemData::ET_SHORTCUT),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores shortcut dependent removals");
    auto assertion = [this, &sbase] { EXPECT_EQ(1U, core.NumShortcuts(sbase.GetUUID())); };
    TestReadonly(RemoveDependentEntryCommand::Create(&core, sbase.GetUUID(), sdep.GetUUID(), CItemData::ET_SHORTCUT),
                 assertion);
  }

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

  {
    SCOPED_TRACE("remove an alias dependent association");
    auto assertInitial = [this, &abase] { EXPECT_EQ(1U, core.NumAliases(abase.GetUUID())); };
    auto assertOnChange = [this, &abase] { EXPECT_EQ(0U, core.NumAliases(abase.GetUUID())); };
    TestExecute(RemoveDependentEntryCommand::Create(&core, abase.GetUUID(), adep.GetUUID(), CItemData::ET_ALIAS),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("add an alias dependent association");
    auto assertInitial = [this, &abase] { EXPECT_EQ(0U, core.NumAliases(abase.GetUUID())); };
    auto assertOnChange = [this, &abase] { EXPECT_EQ(1U, core.NumAliases(abase.GetUUID())); };
    TestExecute(AddDependentEntryCommand::Create(&core, abase.GetUUID(), adep.GetUUID(), CItemData::ET_ALIAS),
                assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, AddAndMoveDependentEntries)
{
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

  UUIDVector shortcuts{dep1.GetUUID(), dep2.GetUUID()};
  {
    SCOPED_TRACE("read-only core ignores shortcut association additions");
    auto assertion = [this, &base1, &base2] {
      EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
      EXPECT_EQ(0U, core.NumShortcuts(base2.GetUUID()));
    };
    TestReadonly(AddDependentEntriesCommand::Create(&core, shortcuts, nullptr,
                                                   CItemData::ET_SHORTCUT, CItemData::UUID), assertion);
  }

  {
    SCOPED_TRACE("add shortcut associations");
    auto assertInitial = [this, &base1, &base2] {
      EXPECT_EQ(6U, core.GetNumEntries());
      EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
      EXPECT_EQ(0U, core.NumShortcuts(base2.GetUUID()));
    };
    auto assertOnChange = [this, &base1] { EXPECT_EQ(2U, core.NumShortcuts(base1.GetUUID())); };
    TestExecute(AddDependentEntriesCommand::Create(&core, shortcuts, nullptr,
                                                   CItemData::ET_SHORTCUT, CItemData::UUID),
                assertInitial, assertOnChange);
  }

  UUIDVector aliases{dep3.GetUUID()};
  {
    SCOPED_TRACE("read-only core ignores alias association additions");
    auto assertion = [this, &base3] { EXPECT_EQ(0U, core.NumAliases(base3.GetUUID())); };
    TestReadonly(AddDependentEntriesCommand::Create(&core, aliases, nullptr,
                                                   CItemData::ET_ALIAS, CItemData::UUID), assertion);
  }

  {
    SCOPED_TRACE("add alias associations");
    auto assertInitial = [this, &base3] { EXPECT_EQ(0U, core.NumAliases(base3.GetUUID())); };
    auto assertOnChange = [this, &base3] { EXPECT_EQ(1U, core.NumAliases(base3.GetUUID())); };
    TestExecute(AddDependentEntriesCommand::Create(&core, aliases, nullptr,
                                                   CItemData::ET_ALIAS, CItemData::UUID),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("move shortcut associations between bases");
    auto assertInitial = [this, &base1, &base2] {
      EXPECT_EQ(2U, core.NumShortcuts(base1.GetUUID()));
      EXPECT_EQ(0U, core.NumShortcuts(base2.GetUUID()));
    };
    auto assertOnChange = [this, &base1, &base2] {
      EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
      EXPECT_EQ(2U, core.NumShortcuts(base2.GetUUID()));
    };
    TestExecute(MoveDependentEntriesCommand::Create(&core, base1.GetUUID(), base2.GetUUID(), CItemData::ET_SHORTCUT),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores shortcut association moves");
    auto assertion = [this, &base1, &base2] {
      EXPECT_EQ(0U, core.NumShortcuts(base1.GetUUID()));
      EXPECT_EQ(2U, core.NumShortcuts(base2.GetUUID()));
    };
    TestReadonly(MoveDependentEntriesCommand::Create(&core, base2.GetUUID(), base1.GetUUID(), CItemData::ET_SHORTCUT),
                 assertion);
  }

  {
    SCOPED_TRACE("move alias associations between bases");
    auto assertInitial = [this, &base2, &base3] {
      EXPECT_EQ(1U, core.NumAliases(base3.GetUUID()));
      EXPECT_EQ(0U, core.NumAliases(base2.GetUUID()));
    };
    auto assertOnChange = [this, &base2, &base3] {
      EXPECT_EQ(0U, core.NumAliases(base3.GetUUID()));
      EXPECT_EQ(1U, core.NumAliases(base2.GetUUID()));
    };
    TestExecute(MoveDependentEntriesCommand::Create(&core, base3.GetUUID(), base2.GetUUID(), CItemData::ET_ALIAS),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores alias association moves");
    auto assertion = [this, &base2, &base3] {
      EXPECT_EQ(0U, core.NumAliases(base3.GetUUID()));
      EXPECT_EQ(1U, core.NumAliases(base2.GetUUID()));
    };
    TestReadonly(MoveDependentEntriesCommand::Create(&core, base2.GetUUID(), base3.GetUUID(), CItemData::ET_ALIAS),
                 assertion);
  }
}

TEST_F(CommandsTest, PolicyCommands)
{
  constexpr int initialLength = 12;
  constexpr int modifiedLength = 16;
  PSWDPolicyMap policies;

  PWPolicy initialPolicy;
  initialPolicy.flags = PWPolicy::UseDigits | PWPolicy::UseUppercase;
  initialPolicy.length = initialLength;

  stringT policyName1 = L"WebPolicy";
  StringX s_policyName1(policyName1);
  {
    SCOPED_TRACE("read-only core ignores password policy additions");
    auto assertion = [&policies] { EXPECT_TRUE(policies.empty()); };
    TestReadonly(new PolicyCommandAdd(core, policies, policyName1, initialPolicy), assertion);
  }

  {
    SCOPED_TRACE("add a password policy");
    auto assertInitial = [&policies] { EXPECT_TRUE(policies.empty()); };
    auto assertOnChange = [&policies, &s_policyName1, initialLength] {
      ASSERT_EQ(1U, policies.size());
      EXPECT_TRUE(policies.contains(s_policyName1));
      EXPECT_EQ(initialLength, policies[s_policyName1].length);
    };
    TestExecute(new PolicyCommandAdd(core, policies, policyName1, initialPolicy),
                assertInitial, assertOnChange);
  }

  PWPolicy modifiedPolicy = initialPolicy;
  modifiedPolicy.length = modifiedLength;
  modifiedPolicy.flags |= PWPolicy::UseSymbols;

  {
    SCOPED_TRACE("read-only core ignores password policy modifications");
    auto assertion = [&policies, &s_policyName1, initialLength] {
      ASSERT_EQ(1U, policies.size());
      EXPECT_EQ(initialLength, policies[s_policyName1].length);
    };
    TestReadonly(new PolicyCommandModify<MultiPolicyCollector, PSWDPolicyMap>(
                   core, policies, policyName1, initialPolicy, modifiedPolicy), assertion);
  }

  {
    SCOPED_TRACE("modify a password policy");
    auto assertInitial = [&policies, &s_policyName1, initialLength] { EXPECT_EQ(initialLength, policies[s_policyName1].length); };
    auto assertOnChange = [&policies, &s_policyName1, modifiedLength] { EXPECT_EQ(modifiedLength, policies[s_policyName1].length); };
    TestExecute(new PolicyCommandModify<MultiPolicyCollector, PSWDPolicyMap>(
                  core, policies, policyName1, initialPolicy, modifiedPolicy),
                assertInitial, assertOnChange);
  }

  stringT policyName2 = L"SecureWebPolicy";
  StringX s_policyName2(policyName2);
  PWPolicy renamedPolicy = modifiedPolicy;
  renamedPolicy.length = 20;

  {
    SCOPED_TRACE("read-only core ignores password policy renames");
    auto assertion = [&policies, &s_policyName1, &s_policyName2, modifiedLength] {
      ASSERT_EQ(1U, policies.size());
      EXPECT_TRUE(policies.contains(s_policyName1));
      EXPECT_FALSE(policies.contains(s_policyName2));
      EXPECT_EQ(modifiedLength, policies[s_policyName1].length);
    };
    TestReadonly(new PolicyCommandRename(core, policies, policyName1, policyName2,
                                         modifiedPolicy, renamedPolicy), assertion);
  }

  {
    SCOPED_TRACE("rename a password policy");
    auto assertInitial = [&policies, &s_policyName1, &s_policyName2, modifiedLength] {
      ASSERT_EQ(1U, policies.size());
      EXPECT_TRUE(policies.contains(s_policyName1));
      EXPECT_FALSE(policies.contains(s_policyName2));
      EXPECT_EQ(modifiedLength, policies[s_policyName1].length);
    };
    auto assertOnChange = [&policies, &s_policyName1, &s_policyName2] {
      ASSERT_EQ(1U, policies.size());
      EXPECT_FALSE(policies.contains(s_policyName1));
      EXPECT_TRUE(policies.contains(s_policyName2));
      EXPECT_EQ(20, policies[s_policyName2].length);
    };
    TestExecute(new PolicyCommandRename(core, policies, policyName1, policyName2,
                                        modifiedPolicy, renamedPolicy),
                assertInitial, assertOnChange);
  }

  {
    SCOPED_TRACE("read-only core ignores password policy removals");
    auto assertion = [&policies, &s_policyName2] {
      ASSERT_EQ(1U, policies.size());
      EXPECT_TRUE(policies.contains(s_policyName2));
      EXPECT_EQ(20, policies[s_policyName2].length);
    };
    TestReadonly(new PolicyCommandRemove(core, policies, policyName2, renamedPolicy), assertion);
  }

  {
    SCOPED_TRACE("remove a password policy");
    auto assertInitial = [&policies, &s_policyName2] { EXPECT_TRUE(policies.contains(s_policyName2)); };
    auto assertOnChange = [&policies] { EXPECT_TRUE(policies.empty()); };
    TestExecute(new PolicyCommandRemove(core, policies, policyName2, renamedPolicy),
                assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, DBPolicyNamesCommand)
{
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

  {
    SCOPED_TRACE("add a single password policy");
    auto assertInitial = [this] { EXPECT_TRUE(core.GetPasswordPolicies().empty()); };
    auto assertChanged = [this, &name1] {
      ASSERT_EQ(1U, core.GetPasswordPolicies().size());
      EXPECT_EQ(12, core.GetPasswordPolicies().at(name1).length);
      EXPECT_TRUE(core.HasDBChanged());
    };
    TestExecute(DBPolicyNamesCommand::Create(&core, name1, pol1), assertInitial, assertChanged,
                UpdateGUICommand::GUI_UPDATE_STATUSBAR);
  }

  PSWDPolicyMap newPolicies;
  newPolicies[name1] = pol1;  // duplicate – should be skipped
  newPolicies[name2] = pol2;  // new – should be added

  {
    SCOPED_TRACE("add only policies with new names");
    auto assertInitial = [this, &name1] {
      ASSERT_EQ(1U, core.GetPasswordPolicies().size());
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name1));
    };
    auto assertChanged = [this, &name1, &name2] {
      ASSERT_EQ(2U, core.GetPasswordPolicies().size());
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name1));
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name2));
    };
    TestExecute(DBPolicyNamesCommand::Create(&core, newPolicies, DBPolicyNamesCommand::NP_ADDNEW),
                assertInitial, assertChanged, UpdateGUICommand::GUI_UPDATE_STATUSBAR);
  }

  PSWDPolicyMap replacementPolicies;
  replacementPolicies[name3] = pol3;

  {
    SCOPED_TRACE("replace all password policies");
    auto assertInitial = [this, &name1, &name2] {
      ASSERT_EQ(2U, core.GetPasswordPolicies().size());
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name1));
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name2));
    };
    auto assertChanged = [this, &name1, &name2, &name3] {
      ASSERT_EQ(1U, core.GetPasswordPolicies().size());
      EXPECT_FALSE(core.GetPasswordPolicies().contains(name1));
      EXPECT_FALSE(core.GetPasswordPolicies().contains(name2));
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name3));
      EXPECT_EQ(20, core.GetPasswordPolicies().at(name3).length);
    };
    TestExecute(DBPolicyNamesCommand::Create(&core, replacementPolicies, DBPolicyNamesCommand::NP_REPLACEALL),
                assertInitial, assertChanged, UpdateGUICommand::GUI_UPDATE_STATUSBAR);
  }

  {
    SCOPED_TRACE("read-only core ignores password policy changes");
    auto assertion = [this, &name1, &name3] {
      EXPECT_FALSE(core.GetPasswordPolicies().contains(name1));
      EXPECT_TRUE(core.GetPasswordPolicies().contains(name3));
    };
    TestReadonly(DBPolicyNamesCommand::Create(&core, name1, pol1), assertion);
  }

  {
    SCOPED_TRACE("no-GUI-notify policy command still changes policies");
    auto assertInitial = [this, &name1] { EXPECT_FALSE(core.GetPasswordPolicies().contains(name1)); };
    auto assertChanged = [this, &name1] { EXPECT_TRUE(core.GetPasswordPolicies().contains(name1)); };
    TestNoGuiNotify(DBPolicyNamesCommand::Create(&core, name1, pol1), assertInitial, assertChanged);
  }
}

TEST_F(CommandsTest, EmbedNamedPolicy)
{
  // GH1953: D&D of an entry with a named policy must carry the policy's
  // details, since the drop target recreates the named policy from them
  PWScore core;

  PWPolicy pol;
  pol.flags      = PWPolicy::UseDigits | PWPolicy::UseLowercase | PWPolicy::UseSymbols;
  pol.length     = 14;
  pol.digitminlength = 2;
  pol.symbols    = L"#$%";
  const StringX name(L"NamedPolicy");
  core.Execute(DBPolicyNamesCommand::Create(&core, name, pol));

  CItemData ci;
  ci.CreateUUID();
  ci.SetTitle(L"title");
  ci.SetPassword(L"password");
  ci.SetPolicyName(name);

  // Entry has only the name until embedded
  PWPolicy entry_pol;
  ci.GetPWPolicy(entry_pol);
  EXPECT_EQ(PWPolicy(), entry_pol);

  EXPECT_TRUE(core.EmbedNamedPolicy(ci));

  // Round trip through D&D serialization
  std::vector<char> v;
  ci.SerializePlainText(v);
  CItemData dropped;
  ASSERT_TRUE(dropped.DeSerializePlainText(v));

  EXPECT_EQ(name, dropped.GetPolicyName());
  dropped.GetPWPolicy(entry_pol);
  entry_pol.symbols = dropped.GetSymbols();
  EXPECT_EQ(pol, entry_pol);
  EXPECT_EQ(14, entry_pol.length);
  EXPECT_EQ(pol.flags, entry_pol.flags);

  // No policy name, or name unknown to this core: entry is untouched
  CItemData noname;
  noname.CreateUUID();
  EXPECT_FALSE(core.EmbedNamedPolicy(noname));
  EXPECT_TRUE(noname.GetPWPolicy().empty());

  CItemData unknown;
  unknown.CreateUUID();
  unknown.SetPolicyName(L"NoSuchPolicy");
  EXPECT_FALSE(core.EmbedNamedPolicy(unknown));
  EXPECT_TRUE(unknown.GetPWPolicy().empty());

  core.ClearCommands();
}

namespace {
  // An entry as received by a drop: policy name, plus details if the
  // sender embedded them
  CItemData DroppedEntry(const StringX &sxPolicyName, const PWPolicy *pPolicy)
  {
    CItemData ci;
    ci.CreateUUID();
    ci.SetTitle(L"dropped");
    ci.SetPassword(L"password");
    ci.SetPolicyName(sxPolicyName);
    if (pPolicy != nullptr) {
      ci.SetPWPolicy(*pPolicy);
      ci.SetSymbols(pPolicy->symbols);
    }
    return ci;
  }
}

TEST_F(CommandsTest, ProcessDroppedPolicy)
{
  PWScore core;

  PWPolicy polTarget, polOther;
  polTarget.flags  = PWPolicy::UseDigits | PWPolicy::UseLowercase;
  polTarget.length = 12;
  polOther.flags   = PWPolicy::UseUppercase | PWPolicy::UseSymbols;
  polOther.length  = 20;
  polOther.symbols = L"!@";

  const StringX sxShared(L"Shared");
  core.Execute(DBPolicyNamesCommand::Create(&core, sxShared, polTarget));
  core.ClearCommands();

  std::map<StringX, StringX> mapRenamedPolicies;
  std::vector<StringX> vAddedPolicyNames;
  const StringX sxDateTime(L"2026/10/18 12:00:00");
  Command *pcmd(nullptr);

  // --- No named policy ---
  {
    CItemData ci;
    ci.CreateUUID();
    EXPECT_EQ(PWScore::NamedPolicyOutcome::NONE,
              core.ProcessDroppedPolicy(ci, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    EXPECT_EQ(nullptr, pcmd);
  }

  // --- Same name & settings as target: use it, strip details from entry ---
  {
    CItemData ci = DroppedEntry(sxShared, &polTarget);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::EXISTING,
              core.ProcessDroppedPolicy(ci, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    EXPECT_EQ(nullptr, pcmd);
    EXPECT_EQ(sxShared, ci.GetPolicyName());
    EXPECT_FALSE(ci.IsPasswordPolicySet());
    EXPECT_TRUE(ci.GetSymbols().empty());
  }

  // --- New to target: added once, even if several entries use it ---
  {
    const StringX sxNew(L"New");
    CItemData ci1 = DroppedEntry(sxNew, &polOther);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::ADDED,
              core.ProcessDroppedPolicy(ci1, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    ASSERT_NE(nullptr, pcmd);
    EXPECT_EQ(sxNew, ci1.GetPolicyName());
    EXPECT_FALSE(ci1.IsPasswordPolicySet());
    core.Execute(pcmd);

    PWPolicy added;
    ASSERT_TRUE(core.GetPolicyFromName(sxNew, added));
    EXPECT_EQ(polOther, added);
    EXPECT_EQ(20, added.length);

    CItemData ci2 = DroppedEntry(sxNew, &polOther);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::EXISTING,
              core.ProcessDroppedPolicy(ci2, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    EXPECT_EQ(nullptr, pcmd);
    core.ClearCommands();
  }

  // --- Same name, different settings: added under a new name, once ---
  {
    CItemData ci1 = DroppedEntry(sxShared, &polOther);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::RENAMED,
              core.ProcessDroppedPolicy(ci1, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    ASSERT_NE(nullptr, pcmd);
    const StringX sxRenamed = ci1.GetPolicyName();
    EXPECT_NE(sxShared, sxRenamed);
    EXPECT_FALSE(ci1.IsPasswordPolicySet());

    // Second entry with the same policy maps to the same new name, no new command
    CItemData ci2 = DroppedEntry(sxShared, &polOther);
    Command *pcmd2(nullptr);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::RENAMED,
              core.ProcessDroppedPolicy(ci2, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd2));
    EXPECT_EQ(nullptr, pcmd2);
    EXPECT_EQ(sxRenamed, ci2.GetPolicyName());

    core.Execute(pcmd);
    PWPolicy renamed, original;
    ASSERT_TRUE(core.GetPolicyFromName(sxRenamed, renamed));
    EXPECT_EQ(polOther, renamed);
    ASSERT_TRUE(core.GetPolicyFromName(sxShared, original));
    EXPECT_EQ(polTarget, original); // target's own policy untouched
    core.ClearCommands();
  }

  // --- Sender didn't embed details (older version) ---
  {
    // Name known to target: use target's policy
    CItemData ci1 = DroppedEntry(sxShared, nullptr);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::EXISTING,
              core.ProcessDroppedPolicy(ci1, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    EXPECT_EQ(nullptr, pcmd);
    EXPECT_EQ(sxShared, ci1.GetPolicyName());

    // Name unknown to target: don't create an empty policy - clear the name
    const size_t nPolicies = core.GetPasswordPolicies().size();
    CItemData ci2 = DroppedEntry(L"Unknown", nullptr);
    EXPECT_EQ(PWScore::NamedPolicyOutcome::CLEARED,
              core.ProcessDroppedPolicy(ci2, mapRenamedPolicies, vAddedPolicyNames, sxDateTime, pcmd));
    EXPECT_EQ(nullptr, pcmd);
    EXPECT_FALSE(ci2.IsPolicyNameSet());
    EXPECT_EQ(nPolicies, core.GetPasswordPolicies().size());
  }

  core.ClearCommands();
}

TEST_F(CommandsTest, ProcessPolicyName)
{
  // Merge/Sync/Copy: named policies come from another database
  PWScore core, othercore;

  PWPolicy polSame, polOurs, polTheirs;
  polSame.flags   = PWPolicy::UseDigits | PWPolicy::UseLowercase;
  polSame.length  = 12;
  polOurs.flags   = PWPolicy::UseUppercase;
  polOurs.length  = 10;
  polTheirs.flags = PWPolicy::UseUppercase | PWPolicy::UseSymbols;
  polTheirs.length = 20;

  const StringX sxSame(L"Same"), sxClash(L"Clash"), sxNew(L"New");
  core.Execute(DBPolicyNamesCommand::Create(&core, sxSame, polSame));
  core.Execute(DBPolicyNamesCommand::Create(&core, sxClash, polOurs));
  core.ClearCommands();
  othercore.Execute(DBPolicyNamesCommand::Create(&othercore, sxSame, polSame));
  othercore.Execute(DBPolicyNamesCommand::Create(&othercore, sxClash, polTheirs));
  othercore.Execute(DBPolicyNamesCommand::Create(&othercore, sxNew, polTheirs));
  othercore.ClearCommands();

  std::map<StringX, StringX> mapRenamedPolicies;
  std::vector<StringX> vs_PoliciesAdded;
  const StringX sxDateTime(L"2026/10/19 12:00:00");

  auto Process = [&](CItemData &ci, const StringX &sxOtherPolicyName, bool &bUpdated) {
    return core.ProcessPolicyName(&othercore, ci, mapRenamedPolicies, vs_PoliciesAdded,
                                  sxOtherPolicyName, bUpdated, sxDateTime, IDSC_MERGEPOLICY);
  };

  // --- Same name & settings in both: nothing to do ---
  {
    CItemData ci;
    ci.SetPolicyName(sxSame);
    bool bUpdated(false);
    EXPECT_EQ(nullptr, Process(ci, sxSame, bUpdated));
    EXPECT_FALSE(bUpdated);
    EXPECT_EQ(sxSame, ci.GetPolicyName());
  }

  // --- Not in this database: added once ---
  {
    CItemData ci1, ci2;
    bool bUpdated(false);
    Command *pcmd = Process(ci1, sxNew, bUpdated);
    ASSERT_NE(nullptr, pcmd);
    EXPECT_TRUE(bUpdated);
    EXPECT_EQ(sxNew, ci1.GetPolicyName());

    bUpdated = false;
    EXPECT_EQ(nullptr, Process(ci2, sxNew, bUpdated));
    EXPECT_TRUE(bUpdated);
    EXPECT_EQ(sxNew, ci2.GetPolicyName());

    core.Execute(pcmd);
    PWPolicy added;
    ASSERT_TRUE(core.GetPolicyFromName(sxNew, added));
    EXPECT_EQ(polTheirs, added);
  }

  // --- Same name, different settings: renamed & added once ---
  {
    CItemData ci1, ci2;
    bool bUpdated(false);
    Command *pcmd = Process(ci1, sxClash, bUpdated);
    ASSERT_NE(nullptr, pcmd);
    EXPECT_TRUE(bUpdated);
    const StringX sxRenamed = ci1.GetPolicyName();
    EXPECT_NE(sxClash, sxRenamed);

    bUpdated = false;
    EXPECT_EQ(nullptr, Process(ci2, sxClash, bUpdated)); // no duplicate add
    EXPECT_TRUE(bUpdated);
    EXPECT_EQ(sxRenamed, ci2.GetPolicyName());

    core.Execute(pcmd);
    PWPolicy renamed, ours;
    ASSERT_TRUE(core.GetPolicyFromName(sxRenamed, renamed));
    EXPECT_EQ(polTheirs, renamed);
    ASSERT_TRUE(core.GetPolicyFromName(sxClash, ours));
    EXPECT_EQ(polOurs, ours);
  }

  core.ClearCommands();
}

TEST_F(CommandsTest, DBEmptyGroupsCommand)
{
  const StringX group1(L"Group1");
  const StringX group2(L"Group2");
  const StringX group1a(L"Group1.Subgroup");
  const std::vector initialGroups{group1, group1a};
  const std::vector withGroup2{group1, group1a, group2};
  const auto assertGroups = [this](const std::vector<StringX> &groups) {
    EXPECT_EQ(groups, core.GetEmptyGroups());
  };

  {
    SCOPED_TRACE("create initial empty groups");
    auto assertInitial = [assertGroups] { assertGroups({}); };
    auto assertOnChange = [assertGroups, &initialGroups] { assertGroups(initialGroups); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, initialGroups,
                                             DBEmptyGroupsCommand::EG_REPLACEALL),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  {
    SCOPED_TRACE("add a single empty group");
    auto assertInitial = [assertGroups, &initialGroups] { assertGroups(initialGroups); };
    auto assertOnChange = [assertGroups, &withGroup2] { assertGroups(withGroup2); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, group2, DBEmptyGroupsCommand::EG_ADD),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  {
    SCOPED_TRACE("delete a single empty group");
    auto assertInitial = [assertGroups, &withGroup2] { assertGroups(withGroup2); };
    auto assertOnChange = [assertGroups, &group1a, &group2] { assertGroups({group1a, group2}); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, group1, DBEmptyGroupsCommand::EG_DELETE),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  const StringX renamedGroup1a(L"Renamed.Subgroup");
  {
    SCOPED_TRACE("rename an empty group");
    auto assertInitial = [assertGroups, &group1a, &group2] { assertGroups({group1a, group2}); };
    auto assertOnChange = [assertGroups, &group2, &renamedGroup1a] { assertGroups({group2, renamedGroup1a}); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, group1a, renamedGroup1a,
                                             DBEmptyGroupsCommand::EG_RENAME),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  const StringX group3(L"Group3");
  const std::vector addGroups{group2, group3};
  const std::vector renamedGroups{group2, renamedGroup1a};
  const std::vector addedGroups{group2, group3, renamedGroup1a};
  {
    SCOPED_TRACE("add all groups while preserving existing groups");
    auto assertInitial = [assertGroups, &renamedGroups] { assertGroups(renamedGroups); };
    auto assertOnChange = [assertGroups, &addedGroups] { assertGroups(addedGroups); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, addGroups, DBEmptyGroupsCommand::EG_ADDALL),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  {
    SCOPED_TRACE("replace all empty groups");
    auto assertInitial = [assertGroups, &addedGroups] { assertGroups(addedGroups); };
    auto assertOnChange = [assertGroups, &initialGroups] { assertGroups(initialGroups); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, initialGroups,
                                             DBEmptyGroupsCommand::EG_REPLACEALL),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  const std::vector renamedGroups1{group1, renamedGroup1a};
  {
    SCOPED_TRACE("rename an empty-group path");
    auto assertInitial = [assertGroups, &initialGroups] { assertGroups(initialGroups); };
    auto assertOnChange = [assertGroups, &renamedGroups1] { assertGroups(renamedGroups1); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, group1, StringX(L"Renamed"),
                                             DBEmptyGroupsCommand::EG_RENAMEPATH),
                assertInitial, assertOnChange, UpdateGUICommand::GUI_REFRESH_TREE);
  }

  {
    SCOPED_TRACE("duplicate add is a no-op and doesn't notify");
    auto assertion = [assertGroups, &renamedGroups1] { assertGroups(renamedGroups1); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, renamedGroup1a, DBEmptyGroupsCommand::EG_ADD),
                assertion, assertion);
  }

  {
    SCOPED_TRACE("identical replacement is a no-op");
    auto assertion = [assertGroups, &renamedGroups1] { assertGroups(renamedGroups1); };
    TestExecute(DBEmptyGroupsCommand::Create(&core, renamedGroups1, DBEmptyGroupsCommand::EG_REPLACEALL),
                assertion, assertion);
  }

  {
    SCOPED_TRACE("read-only core ignores empty-group changes");
    auto assertion = [assertGroups, &renamedGroups1] { assertGroups(renamedGroups1); };
    TestReadonly(DBEmptyGroupsCommand::Create(&core, group3, DBEmptyGroupsCommand::EG_ADD), assertion);
  }

  {
    SCOPED_TRACE("no-GUI-notify still modifies empty groups");
    auto assertInitial = [assertGroups, &renamedGroups1] {
      assertGroups(renamedGroups1);
    };
    auto assertOnChange = [assertGroups, &group1, &renamedGroup1a, &group2] {
      assertGroups({group1, group2, renamedGroup1a});
    };
    TestNoGuiNotify(DBEmptyGroupsCommand::Create(&core, group2, DBEmptyGroupsCommand::EG_ADD),
                    assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, DBFiltersCommand)
{
  const PWSFilters originalFilters = core.GetDBFilters();
  const st_Filterkey key{FPOOL_DATABASE, L"Database filter"};
  st_filters filter;
  filter.fname = L"Database filter";
  PWSFilters newFilters;
  newFilters[key] = filter;

  {
    SCOPED_TRACE("execute, undo, and redo filters");
    auto assertInitial = [this, &originalFilters] { EXPECT_EQ(originalFilters, core.GetDBFilters()); };
    auto assertOnChange = [this, &newFilters] { EXPECT_EQ(newFilters, core.GetDBFilters()); };
    TestExecute(DBFiltersCommand::Create(&core, newFilters), assertInitial, assertOnChange, UpdateGUICommand::GUI_UPDATE_STATUSBAR);
  }

  {
    SCOPED_TRACE("unchanged filters are a no-op");
    auto assertion = [this, &newFilters] { EXPECT_EQ(newFilters, core.GetDBFilters()); };
    TestExecute(DBFiltersCommand::Create(&core, core.GetDBFilters()), assertion, assertion);
  }

  {
    SCOPED_TRACE("read-only core ignores filter changes");
    auto assertion = [this, &newFilters] { EXPECT_EQ(newFilters, core.GetDBFilters()); };
    TestReadonly(DBFiltersCommand::Create(&core, originalFilters), assertion);
  }

  {
    SCOPED_TRACE("SetNoGUINotify suppresses command notification");
    auto assertInitial = [this, &newFilters] { EXPECT_EQ(newFilters, core.GetDBFilters()); };
    auto assertOnChange = [this, &originalFilters] { EXPECT_EQ(originalFilters, core.GetDBFilters()); };
    TestNoGuiNotify(DBFiltersCommand::Create(&core, originalFilters), assertInitial, assertOnChange);
  }
}

TEST_F(CommandsTest, DBPrefsCommand)
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  {
    SCOPED_TRACE("change database preferences and hash iterations");
    const uint32 oldHashIters = core.GetHashIters();
    const bool oldSaveHistory = prefs->GetPref(PWSprefs::SavePasswordHistory);
    const bool newSaveHistory = !oldSaveHistory;
    prefs->SetPref(PWSprefs::SavePasswordHistory, newSaveHistory);
    const StringX newPrefs = prefs->Store();
    prefs->SetPref(PWSprefs::SavePasswordHistory, oldSaveHistory);

    auto assertInitial = [this, prefs, oldSaveHistory, oldHashIters] {
      EXPECT_EQ(oldSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(oldHashIters, core.GetHashIters());
    };
    auto assertOnChange = [this, prefs, newSaveHistory, oldHashIters] {
      EXPECT_EQ(newSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(oldHashIters + 1, core.GetHashIters());
    };
    TestExecute(DBPrefsCommand::Create(&core, newPrefs, oldHashIters + 1),
                assertInitial, assertOnChange,
                UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED);
  }

  {
    SCOPED_TRACE("omitting hash iterations leaves them unchanged");
    const bool currentSaveHistory = prefs->GetPref(PWSprefs::SavePasswordHistory);
    const bool newSaveHistory = !currentSaveHistory;
    const uint32 currentHashIters = core.GetHashIters();
    prefs->SetPref(PWSprefs::SavePasswordHistory, newSaveHistory);
    const StringX newPrefs = prefs->Store();
    prefs->SetPref(PWSprefs::SavePasswordHistory, currentSaveHistory);

    auto assertInitial = [this, prefs, currentSaveHistory, currentHashIters] {
      EXPECT_EQ(currentSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(currentHashIters, core.GetHashIters());
    };
    auto assertOnChange = [this, prefs, newSaveHistory, currentHashIters] {
      EXPECT_EQ(newSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(currentHashIters, core.GetHashIters());
    };
    TestExecute(DBPrefsCommand::Create(&core, newPrefs), assertInitial, assertOnChange,
                UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED);
  }

  {
    SCOPED_TRACE("read-only core ignores preference changes");
    const bool currentSaveHistory = prefs->GetPref(PWSprefs::SavePasswordHistory);
    const uint32 currentHashIters = core.GetHashIters();
    const bool newSaveHistory = !currentSaveHistory;
    prefs->SetPref(PWSprefs::SavePasswordHistory, newSaveHistory);
    const StringX newPrefs = prefs->Store();
    prefs->SetPref(PWSprefs::SavePasswordHistory, currentSaveHistory);

    auto assertion = [this, prefs, currentSaveHistory, currentHashIters] {
      EXPECT_EQ(currentSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(currentHashIters, core.GetHashIters());
    };
    TestReadonly(DBPrefsCommand::Create(&core, newPrefs, currentHashIters + 1), assertion);
  }

  {
    SCOPED_TRACE("SetNoGUINotify suppresses command notification");
    const bool currentSaveHistory = prefs->GetPref(PWSprefs::SavePasswordHistory);
    const uint32 currentHashIters = core.GetHashIters();
    const bool newSaveHistory = !currentSaveHistory;
    prefs->SetPref(PWSprefs::SavePasswordHistory, newSaveHistory);
    const StringX newPrefs = prefs->Store();
    prefs->SetPref(PWSprefs::SavePasswordHistory, currentSaveHistory);

    auto assertInitial = [this, prefs, currentSaveHistory, currentHashIters] {
      EXPECT_EQ(currentSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(currentHashIters, core.GetHashIters());
    };
    auto assertOnChange = [this, prefs, newSaveHistory, currentHashIters] {
      EXPECT_EQ(newSaveHistory, prefs->GetPref(PWSprefs::SavePasswordHistory));
      EXPECT_EQ(currentHashIters + 1, core.GetHashIters());
    };
    TestNoGuiNotify(DBPrefsCommand::Create(&core, newPrefs, currentHashIters + 1),
                    assertInitial, assertOnChange);
  }
}
