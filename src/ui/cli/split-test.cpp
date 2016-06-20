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

TEST_F(SplitTest, CountsEmptyTokensBetweenSeparator) {
  vector<wstring> tokens;
  Split( L"Hello,, world!", L",", [&tokens](const wstring &s) {
    tokens.push_back(s);
  });
  EXPECT_EQ(3, tokens.size());
  EXPECT_EQ(L"Hello", tokens[0]);
  EXPECT_TRUE(tokens[1].empty()) << "Includes empty tokens between separators";
  EXPECT_EQ(L" world!", tokens[2]);
}

TEST_F(SplitTest, CountsEmptyTokensBeforeAndAfterSeparator) {
  vector<wstring> tokens;
  Split( L",Hello, world!,", L",", [&tokens](const wstring &s) {
    tokens.push_back(s);
  });
  EXPECT_EQ(4, tokens.size());
  EXPECT_TRUE(tokens[0].empty()) << "Includes empty tokens before separator";
  EXPECT_EQ(L"Hello", tokens[1]);
  EXPECT_EQ(L" world!", tokens[2]);
  EXPECT_TRUE(tokens[3].empty()) << "Includes empty tokens after separator";
}


}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
