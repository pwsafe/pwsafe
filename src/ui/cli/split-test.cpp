/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "./strutils.h"
#include <gtest/gtest.h>

#include <string>
#include <vector>

using std::wstring;
using std::vector;

namespace {

// The fixture for testing Split function.
class SplitTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  SplitTest() {
    // You can do set-up work for each test here.
  }

  virtual ~SplitTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for SplitTest.
};

// Tests that the Split works correctly for single-char separators
TEST_F(SplitTest, SplitsWithSingleCharSeparator) {
  vector<wstring> tokens;
  Split( L"Hello, world!", L",", [&tokens](const wstring &s) {
    tokens.push_back(s);
  });
  EXPECT_EQ(2, tokens.size());
  EXPECT_EQ(L"Hello", tokens[0]);
  EXPECT_EQ(L" world!", tokens[1]);
}

TEST_F(SplitTest, ReturnEntireStringIfNoSeparatorMatch) {
  vector<wstring> tokens;
  Split( L"Hello, world!", L";", [&tokens](const wstring &s) {
    tokens.push_back(s);
  });
  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(L"Hello, world!", tokens[0]);
}

TEST_F(SplitTest, IgnoreEmptyTokensBetweenSeparator) {
  vector<wstring> tokens;
  Split( L"Hello,, world!", L",", [&tokens](const wstring &s) {
    tokens.push_back(s);
  });
  EXPECT_EQ(2, tokens.size());
  EXPECT_EQ(L"Hello", tokens[0]);
  EXPECT_EQ(L" world!", tokens[1]);
}

TEST_F(SplitTest, IgnoresEmptyTokensBeforeAndAfterSeparator) {
  vector<wstring> tokens;
  Split( L",Hello, world!,", L",", [&tokens](const wstring &s) {
    tokens.push_back(s);
  });
  EXPECT_EQ(2, tokens.size());
  EXPECT_EQ(L"Hello", tokens[0]);
  EXPECT_EQ(L" world!", tokens[1]);
}


}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
