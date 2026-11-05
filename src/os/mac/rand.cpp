/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of rand.h
 */
#include "../rand.h"
#include <cassert>
#include <cstdlib>  // for std::abort()
#include <CommonCrypto/CommonRandom.h>

bool pws_os::InitRandomDataFunction()
{
  // Use CommonCrypto RNG
  return true;
}

bool pws_os::GetRandomData(void *p, unsigned long len)
{
    CCRNGStatus err = CCRandomGenerateBytes(p, len);
    if (err != kCCSuccess)
        std::abort();  // PWSrand::GetRandomData does not handle failure
    return true;
}

void pws_os::GetRandomSeed(void *p, unsigned &slen)
{
  /**
   * Provide a cryptographically strong seed from CommonCrypto.
   *
   * When p is NULL, set slen to the required buffer size. Otherwise,
   * fill p with slen seed bytes.
   *
   * The seed initializes PWSrand's internal generator. PWSrand also mixes
   * fresh CommonCrypto bytes into each random fill. Seed generation aborts
   * if CommonCrypto fails.
   */
  const unsigned SEED_LEN = 256/8;
  if (p == NULL) {
    slen = SEED_LEN;
  } else {
    assert(slen == SEED_LEN); // MUST call with p == NULL first!
    if (!GetRandomData(p, slen))
        std::abort(); // no usable entropy source - refuse to run with a guessable seed
  }
}
