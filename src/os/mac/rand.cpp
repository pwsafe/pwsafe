/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of rand.h
 */
#include "../rand.h"
#include <fstream>
#include <cassert>
#include <cstring>
#include <sys/time.h>

using namespace std;

bool pws_os::InitRandomDataFunction()
{
  // For starters, we won't rely on /dev/urandom, only on /dev/random for
  // the seed. Returning false indicates this decision.
  // Perhaps we can check for a hardware rng in future versions, and change
  // the returned value accordingly?
  return false;
}

bool pws_os::GetRandomData(void *p, unsigned long len)
{
  // Return data from /dev/urandom
  // Will not be used by PasswordSafe when InitRandomDataFunction()
  // returns false!

  ifstream is("/dev/urandom");
  if (!is)
    return false;
  return is.read((char *)p, len).good();
}

static void get_failsafe_rnd(char * &p, unsigned &slen)
{
  // This function will be called
  // iff we couldn't get a minimal amount of entropy
  // from the kernel's entropy pool.
  slen = sizeof(suseconds_t);
  p = new char[slen];
  struct timeval tv;
  gettimeofday(&tv, NULL);
  memcpy(p, &tv.tv_usec, slen);
}

void pws_os::GetRandomSeed(void *p, unsigned &slen)
{
  /**
   * Return a cryptographically strong seed
   * from /dev/random, if possible.
   *
   * When called with p == NULL, return number of bytes currently
   * in entropy pool.
   * To minimize TOCTTOU, we also read the data at this time,
   * and deliver it when called with non-NULL p.
   *
   * This implies a strict calling pattern, but hey, it's
   * our application...
   */
  static char *data = NULL;
  if (p == NULL) {
    if (data != NULL) { // just in case...
      delete[] data;
      data = NULL;
    }
    ifstream ent_avail("/proc/sys/kernel/random/entropy_avail");
    if (ent_avail) {
      unsigned ent_bits;
      if (ent_avail >> ent_bits && ent_bits >= 32) {
        slen = ent_bits/8;
        data = new char[slen];
        ifstream rnd;
        rnd.rdbuf()->pubsetbuf(0, 0);
        rnd.open("/dev/random");
        if (rnd.read(data, slen))
          return;
        else { // trouble reading
          delete[] data;
          data = NULL;
          // will get randomness from failsafe.
        }
      }
    }
    // here if we had any trouble getting data from /dev/random
    get_failsafe_rnd(data, slen);
  } else { // called with non-NULL p, just return our hard-earned entropy
    if (data) {
      memcpy(p, data, slen);
      delete[] data;
      data = NULL;
    }
    else {
      assert(false); // MUST call with p == NULL first!
    }
  }
}    
