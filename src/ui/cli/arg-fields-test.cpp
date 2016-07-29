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

}  // namespace
