/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// HashItersTest.cpp: Unit test for the Unlock Difficulty <-> hash iterations mapping (SF1640)

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/HashIters.h"

#include "gtest/gtest.h"

constexpr uint32 kMinIters = MIN_HASH_ITERATIONS;
constexpr uint32 kMaxIters = MAX_USABLE_HASH_ITERS;

// Compile-time sanity checks
static_assert(UnlockDifficultyToHashIters(0) == kMinIters, "level 0 must be the minimum");
static_assert(UnlockDifficultyToHashIters(MAX_UNLOCK_DIFFICULTY) == kMaxIters,
              "top level must be the maximum");

TEST(HashItersTest, Endpoints)
{
  EXPECT_EQ(kMinIters, UnlockDifficultyToHashIters(0));
  EXPECT_EQ(kMaxIters, UnlockDifficultyToHashIters(MAX_UNLOCK_DIFFICULTY));
  // out of range levels are clamped
  EXPECT_EQ(kMinIters, UnlockDifficultyToHashIters(-1));
  EXPECT_EQ(kMaxIters, UnlockDifficultyToHashIters(MAX_UNLOCK_DIFFICULTY + 1));

  EXPECT_EQ(0, HashItersToUnlockDifficulty(0));
  EXPECT_EQ(0, HashItersToUnlockDifficulty(kMinIters - 1));
  EXPECT_EQ(0, HashItersToUnlockDifficulty(kMinIters));
  EXPECT_EQ(MAX_UNLOCK_DIFFICULTY, HashItersToUnlockDifficulty(kMaxIters));
  EXPECT_EQ(MAX_UNLOCK_DIFFICULTY, HashItersToUnlockDifficulty(0xffffffff));
}

TEST(HashItersTest, StrictlyIncreasingAndRoundTrips)
{
  uint32 prev = 0;
  for (int level = 0; level <= MAX_UNLOCK_DIFFICULTY; level++) {
    const uint32 iters = UnlockDifficultyToHashIters(level);
    EXPECT_GE(iters, kMinIters);
    if (level > 0) {
      EXPECT_GT(iters, prev) << "level " << level;
    }
    EXPECT_EQ(level, HashItersToUnlockDifficulty(iters)) << "level " << level;
    prev = iters;
  }
}

TEST(HashItersTest, InBetweenValuesRoundUp)
{
  // A value between two levels is shown at the higher one,
  // so it's never displayed as weaker than it is.
  for (int level = 1; level <= MAX_UNLOCK_DIFFICULTY; level++) {
    const uint32 lo = UnlockDifficultyToHashIters(level - 1);
    const uint32 hi = UnlockDifficultyToHashIters(level);
    EXPECT_EQ(level, HashItersToUnlockDifficulty(lo + 1)) << "level " << level;
    EXPECT_EQ(level, HashItersToUnlockDifficulty(hi - 1)) << "level " << level;
    EXPECT_GE(UnlockDifficultyToHashIters(HashItersToUnlockDifficulty(lo + 1)), lo + 1);
  }
}

// If the slider isn't moved, v is kept as is; moving it one step either way
// yields that level's value.
static void CheckUnchangedLevelKeepsValue(uint32 v)
{
  const int level = HashItersToUnlockDifficulty(v);
  EXPECT_EQ(v, UnlockDifficultyToHashIters(level, v)) << v;
  if (level > 0) {
    EXPECT_EQ(UnlockDifficultyToHashIters(level - 1),
              UnlockDifficultyToHashIters(level - 1, v)) << v;
  }
  if (level < MAX_UNLOCK_DIFFICULTY) {
    EXPECT_EQ(UnlockDifficultyToHashIters(level + 1),
              UnlockDifficultyToHashIters(level + 1, v)) << v;
  }
}

TEST(HashItersTest, UnchangedLevelKeepsCurrentValue)
{
  // Values that aren't on a level, e.g., with a lower MIN_HASH_ITERATIONS
  const uint32 values[] = {kMinIters - 1,
                           UnlockDifficultyToHashIters(3) + 12345,
                           kMaxIters + 1};
  for (uint32 v : values)
    CheckUnchangedLevelKeepsValue(v);
}

TEST(HashItersTest, LegacyWindowsValuesKept)
{
  // Before SF1640, the Windows slider (0..31) stored level * (MAX_USABLE_HASH_ITERS / 31),
  // with level 0 storing MIN_HASH_ITERATIONS
  const uint32 step = kMaxIters / 31;
  for (uint32 k = 1; k <= 31; k++)
    CheckUnchangedLevelKeepsValue(k * step);
}

TEST(HashItersTest, LegacyWxValuesKept)
{
  // Before SF1640, the wx slider (0..100) stored level * (MAX_USABLE_HASH_ITERS / 100),
  // with level 0 storing MIN_HASH_ITERATIONS
  const uint32 step = kMaxIters / 100;
  for (uint32 k = 1; k <= 100; k++)
    CheckUnchangedLevelKeepsValue(k * step);
}
