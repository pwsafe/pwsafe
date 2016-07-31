#include "./strutils.h"
#include <gtest/gtest.h>

namespace {

using std::wstring;

TEST(TrimLeft, SpaceOnLeft) {
  EXPECT_EQ(trimleft(wstring{L" abc"}), L"abc");
}

TEST(TrimLeft, SpaceOnRight) {
  EXPECT_EQ(trimleft(wstring{L"abc "}), L"abc ");
}

TEST(TrimLeft, SpaceOnBothSides) {
  EXPECT_EQ(trimleft(wstring{L" abc "}), L"abc ");
}


TEST(TrimLeft, SpaceInMiddle) {
  EXPECT_EQ(trimleft(wstring{L"ab cd"}), L"ab cd");
}


TEST(TrimLeft, SpaceInMiddleAndLeft) {
  EXPECT_EQ(trimleft(wstring{L" ab cd"}), L"ab cd");
}


TEST(TrimLeft, SpaceInMiddleAndRight) {
  EXPECT_EQ(trimleft(wstring{L"ab cd "}), L"ab cd ");
}

TEST(TrimLeft, SpaceInLeftMiddleAndRight) {
  EXPECT_EQ(trimleft(wstring{L" ab cd "}), L"ab cd ");
}

TEST(TrimLeft, AllSpaces) {
  EXPECT_EQ(trimleft(wstring{L" "}), L"");
}

TEST(TrimLeft, EmptyString) {
  EXPECT_EQ(trimleft(wstring{L""}), L"");
}

/////////////// TrimRight ////////////////////

TEST(TrimRight, SpaceOnRight) {
  EXPECT_EQ(trimright(wstring{L"abc "}), L"abc");
}

TEST(TrimRight, SpaceOnBothSides) {
  EXPECT_EQ(trimright(wstring{L" abc "}), L" abc");
}

TEST(TrimRight, SpaceInMiddle) {
  EXPECT_EQ(trimright(wstring{L"ab cd"}), L"ab cd");
}

TEST(TrimRight, SpaceInMiddleAndLeft) {
  EXPECT_EQ(trimright(wstring{L" ab cd"}), L" ab cd");
}

TEST(TrimRight, SpaceInMiddleAndRight) {
  EXPECT_EQ(trimright(wstring{L"ab cd "}), L"ab cd");
}

TEST(TrimRight, AllSpaces) {
  EXPECT_EQ(trimright(wstring{L" "}), L"");
}

TEST(TrimRight, EmptyString) {
  EXPECT_EQ(trimright(wstring{L""}), L"");
}

///////////// trim //////////////////////

TEST(trim, SpaceOnBothSides) {
  EXPECT_EQ(trim(wstring{L" abcd "}), L"abcd");
}

TEST(trim, SpaceOnLeft) {
  EXPECT_EQ(trim(wstring{L" abc"}), L"abc");
}

TEST(trim, SpaceOnRight) {
  EXPECT_EQ(trim(wstring{L"abc "}), L"abc");
}

TEST(trim, SpaceInMiddle) {
  EXPECT_EQ(trim(wstring{L"ab cd"}), L"ab cd");
}

TEST(trim, AllSpaces) {
  EXPECT_EQ(trim(wstring{L" "}), L"");
}

TEST(trim, EmptyString) {
  EXPECT_EQ(trim(wstring{L""}), L"");
}


}  // namespace
