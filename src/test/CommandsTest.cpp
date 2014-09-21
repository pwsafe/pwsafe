/*
* Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
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

// Since we use CItemData, we assume CItemData::SetSessionKey()
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

