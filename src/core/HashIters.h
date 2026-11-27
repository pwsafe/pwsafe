/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __HASHITERS_H
#define __HASHITERS_H

// HashIters.h
// Key stretching iteration limits, and the mapping between hash iterations
// and the "Unlock Difficulty" slider shown by the UI.
//-----------------------------------------------------------------------------

#include "../os/typedefs.h"

// HASH_ITERATIONS is used by the key stretching algorithm.
// MIN_HASH_ITERATIONS is a lower limit - anything lower than this
// is considered inherently insecure.
// Changing MIN_HASH_ITERATIONS causes the database to be re-saved with the new value, if its current value is below it.
#define MIN_HASH_ITERATIONS 0x50000  // 327,680
// MAX_USABLE_HASH_ITERS is a guesstimate on what's acceptable to a user
// with a reasonably powerful CPU. Real limit's 2^32-1.
#define MAX_USABLE_HASH_ITERS (1 << 25)

static_assert(MIN_HASH_ITERATIONS < MAX_USABLE_HASH_ITERS,
              "MIN_HASH_ITERATIONS must be below MAX_USABLE_HASH_ITERS");

/**
 * Hash iterations is the value we use in the database. Unlock Difficulty is what the user sees (as a slider).
 * Here we map between them for all UI platforms.
 * Iterations are between MIN_HASH_ITERATIONS and MAX_USABLE_HASH_ITERS. Both limits are raised
 * periodically to keep up with Moore's law.
 * Unlock Difficulty is between 0 and MAX_UNLOCK_DIFFICULTY.
 * We maintain a linear mapping between the two ranges. -- TODO: Change to logarithmic mapping.
 *
 */

constexpr int MAX_UNLOCK_DIFFICULTY = 31;

// Returns the lowest level whose iteration count is >= iters,
// so that a value between levels is never shown as weaker than it is.
constexpr int HashItersToUnlockDifficulty(uint32 iters)
{
  if (iters <= MIN_HASH_ITERATIONS)
    return 0;
  if (iters >= MAX_USABLE_HASH_ITERS)
    return MAX_UNLOCK_DIFFICULTY;
  // ceil((iters - MIN) * MAX_LEVEL / range), the inverse of
  // UnlockDifficultyToHashIters() rounded up
  const uint64 range = MAX_USABLE_HASH_ITERS - MIN_HASH_ITERATIONS;
  const uint64 delta = iters - MIN_HASH_ITERATIONS;
  return static_cast<int>((delta * MAX_UNLOCK_DIFFICULTY + range - 1) / range);
}

constexpr uint32 UnlockDifficultyToHashIters(int level)
{
  if (level <= 0)
    return MIN_HASH_ITERATIONS;
  if (level >= MAX_UNLOCK_DIFFICULTY)
    return MAX_USABLE_HASH_ITERS;
  const uint64 range = MAX_USABLE_HASH_ITERS - MIN_HASH_ITERATIONS;
  return MIN_HASH_ITERATIONS + static_cast<uint32>(range * level / MAX_UNLOCK_DIFFICULTY);
}

// As above, but if level is what currentIters maps to (i.e., the user didn't change it),
// returns currentIters unchanged, so that it's never modified unintentionally (SF1640).
constexpr uint32 UnlockDifficultyToHashIters(int level, uint32 currentIters)
{
  return (level == HashItersToUnlockDifficulty(currentIters)) ?
    currentIters : UnlockDifficultyToHashIters(level);
}

#endif /* __HASHITERS_H */
