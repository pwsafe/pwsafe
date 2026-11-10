/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of rand.h
 */
#include "../rand.h"
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <unistd.h> // getentropy()

using namespace std;

bool pws_os::InitRandomDataFunction()
{
  // Return false so PWSrand relies on its internal PRNG.
  // GetRandomSeed() supplies its seed using getentropy().
  return false;
}

bool pws_os::GetRandomData(void *p, unsigned long len)
{
  // Return data from /dev/urandom.
  // PWSrand skips this when InitRandomDataFunction() returns false, but
  // the YubiKey configuration dialog still calls it directly.

  ifstream is("/dev/urandom");
  if (!is)
    return false;
  return is.read(static_cast<char *>(p), len).good();
}

void pws_os::GetRandomSeed(void *p, unsigned &slen)
{
  /**
   * Return a cryptographically strong seed from the kernel's CSPRNG.
   *
   * When called with p == nullptr, return the number of seed bytes
   * we'll provide. When called with non-nullptr p, fill p with
   * that many bytes.
   *
   * getentropy() only blocks until the kernel's pool has been
   * initialized at boot, so unlike reading /dev/random there's no
   * need to consult /proc/sys/kernel/random/entropy_avail first
   * (which doesn't exist outside Linux anyway).
   *
   * There is deliberately NO fallback to the clock or similar: the seed
   * is the only thing that makes the salts and keys we generate
   * unpredictable, so a guessable seed is worse than no seed at all.
   */
  const unsigned SEED_LEN = 256/8;
  if (p == nullptr) {
    slen = SEED_LEN;
  } else {
    assert(slen == SEED_LEN); // MUST call with p == nullptr first!
    if (getentropy(p, slen) != 0)
      abort(); // no usable entropy source - refuse to run with a guessable seed
  }
}
