/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "./safeutils-internal.h"
#include <gtest/gtest.h>

namespace {

TEST(AddEntryTest, AddEntryWithTitleOnly) {
  PWScore core;
  std::wostringstream os;
  int ret = AddEntryWithFields(core, {std::make_tuple(CItemData::TITLE, L"TitleOnlyItem")}, os);
  EXPECT_EQ(ret, PWScore::SUCCESS);
  EXPECT_EQ(core.GetNumEntries(), 1);
  const CItemData &entry = core.GetEntryIter()->second;
  EXPECT_FALSE(entry.GetPassword().empty()) << "Password should be auto-generated";
}

TEST(AddEntryTest, AddEntryWithoutTitle) {
  PWScore core;
  std::wostringstream os;
  int ret = AddEntryWithFields(core, {}, os);
  EXPECT_NE(ret, PWScore::SUCCESS);
  EXPECT_EQ(core.GetNumEntries(), 0);
}

TEST(AddEntryTest, AddEntryWithPassword) {
  PWScore core;
  std::wostringstream os;
  int ret = AddEntryWithFields(core, {std::make_tuple(CItemData::TITLE, L"SomeTitle"),
                                      std::make_tuple(CItemData::PASSWORD, L"randompass")}, os);
  EXPECT_EQ(ret, PWScore::SUCCESS);
  EXPECT_EQ(core.GetNumEntries(), 1);
  const CItemData &entry = core.GetEntryIter()->second;
  EXPECT_EQ(entry.GetPassword(), L"randompass") << "Password should NOT be auto-generated if specified";
}

TEST(AddEntryTest, AddEntryWithFields) {
  PWScore core;
  std::wostringstream os;
  int ret = AddEntryWithFields(core, {std::make_tuple(CItemData::TITLE, L"SomeTitle"),
                                      std::make_tuple(CItemData::EMAIL, L"test@example.com"),
                                      std::make_tuple(CItemData::URL,   L"http://example.com"),
                                      std::make_tuple(CItemData::GROUP, L"Example.Group"),
                                      std::make_tuple(CItemData::USER,  L"ExampleUser"),
                                      std::make_tuple(CItemData::NOTES, L"Line1\nLine2\nLine3"),
                                      }, os);
  EXPECT_EQ(ret, PWScore::SUCCESS);
  EXPECT_EQ(core.GetNumEntries(), 1);
  const CItemData &entry = core.GetEntryIter()->second;
  EXPECT_FALSE(entry.GetPassword().empty()) << "Password should be auto-generated even if other fields are specified";
  EXPECT_EQ(entry.GetGroup(), L"Example.Group");
  EXPECT_EQ(entry.GetTitle(), L"SomeTitle");
  EXPECT_EQ(entry.GetEmail(), L"test@example.com");
  EXPECT_EQ(entry.GetURL(), L"http://example.com");
  EXPECT_EQ(entry.GetUser(), L"ExampleUser");
  EXPECT_EQ(entry.GetNotes(), L"Line1\nLine2\nLine3");
}

}  // namespace
