/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "./argutils.h"
#include <gtest/gtest.h>

#include <string>
#include <vector>

using std::wstring;
using std::vector;

#include "../../core/PWScore.h"

namespace {

TEST(ParseFieldArgsTest, ParsesSingleField) {
  const CItemData::FieldBits fields = ParseFields( L"Group");
  EXPECT_EQ(1, fields.count());
  EXPECT_TRUE(fields.test(CItemData::GROUP));
}

TEST(ParseFieldArgsTest, ParsesAllFields) {
  const CItemData::FieldBits fields = ParseFields( L"Title,Username");
  EXPECT_EQ(2, fields.count());
  EXPECT_TRUE(fields.test(CItemData::TITLE));
  EXPECT_TRUE(fields.test(CItemData::USER));
}

TEST(ParseFieldArgsTest, ThrowsOnInvalidField) {
  bool thrown = false;
  try {
    ParseFields( L"InvalidField");
  }
  catch(const std::exception& /*e*/) {
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

TEST(ParsesFieldValue, ParsesSingleFieldValue) {
  UserArgs::FieldUpdates upd = ParseFieldValues(L"Username=example");
  EXPECT_EQ(upd.size(), 1);
  EXPECT_EQ(std::get<0>(upd[0]), CItemData::USER);
  EXPECT_EQ(std::get<1>(upd[0]), L"example");
}

TEST(ParsesFieldValue, ParsesMultipleFieldValue) {
  UserArgs::FieldUpdates upd = ParseFieldValues(L"Group=NewGroup,URL=example.com,Username=example");
  EXPECT_EQ(upd.size(), 3);
  EXPECT_EQ(std::get<0>(upd[0]), CItemData::GROUP);
  EXPECT_EQ(std::get<1>(upd[0]), L"NewGroup");
  EXPECT_EQ(std::get<0>(upd[1]), CItemData::URL);
  EXPECT_EQ(std::get<1>(upd[1]), L"example.com");
  EXPECT_EQ(std::get<0>(upd[2]), CItemData::USER);
  EXPECT_EQ(std::get<1>(upd[2]), L"example");
}

TEST(ParsesFieldValue, ParsesSingleFieldValueWithSpacesAfterSeparator) {
  UserArgs::FieldUpdates upd;
  ASSERT_NO_THROW(upd = ParseFieldValues(L"Group= NewGroup"));
  EXPECT_EQ(upd.size(), 1);
  EXPECT_EQ(std::get<0>(upd[0]), CItemData::GROUP);
  EXPECT_EQ(std::get<1>(upd[0]), L" NewGroup");
}

TEST(ParsesFieldValue, ParsesSingleFieldValueWithSpacesBeforeSeparator) {
  UserArgs::FieldUpdates upd;
  ASSERT_NO_THROW(upd = ParseFieldValues(L"Group =NewGroup"));
  EXPECT_EQ(upd.size(), 1);
  EXPECT_EQ(std::get<0>(upd[0]), CItemData::GROUP);
  EXPECT_EQ(std::get<1>(upd[0]), L"NewGroup");
}

}  // namespace
