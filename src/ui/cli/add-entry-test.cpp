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

}  // namespace
