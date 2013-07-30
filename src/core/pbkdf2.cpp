/*
* Copyright (c) 2013 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Implementation of PBKDF2 (RFC2898 Section 5.2)
// Based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org

/*
 * TEST_HMAC_SHA1 is used to test against test vectors of PBKDF2
 * from RFC6070. Can't find test vectors for SHA256.
 * The #define is the lesser of evils. Trust me...
 */

//#define TEST_HMAC_SHA1

#include "PwsPlatform.h"
#include "hmac.h"

#include <cstring>

#ifdef TEST_HMAC_SHA1
#include "sha1.h"
#else
#include "sha256.h"
#endif
#include "Util.h"


#ifdef TEST_HMAC_SHA1
typedef HMAC<SHA1, SHA1::HASHLEN, SHA1::BLOCKSIZE> HmacT;
#else
typedef HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> HmacT;
#endif


/**
   @param password          The input password (or key)
   @param password_len      The length of the password (octets)
   @param salt              The salt (or nonce)
   @param salt_len          The length of the salt (octets)
   @param iteration_count   # of iterations desired for LTC_PKCS #5 v2 [read specs for more]
   @param out               [out] The destination for this algorithm
   @param outlen            [in/out] The max size and resulting size of the algorithm output
*/
void pbkdf2(const unsigned char *password, unsigned long password_len, 
            const unsigned char *salt,     unsigned long salt_len,
            int iteration_count,
            unsigned char *out,            unsigned long *outlen)
{
  int itts;
  ulong32  blkno;
  unsigned long stored, left, x, y;
  unsigned char *buf[2];
  HmacT hmac;

  ASSERT(password != NULL);
  ASSERT(salt     != NULL);
  ASSERT(out      != NULL);
  ASSERT(outlen   != NULL);


  buf[0] = new unsigned char[hmac.GetBlockSize() * 2];
  if (buf[0] == NULL) {
    ASSERT(0);
    return;
  }
  /* buf[1] points to the second block of MAXBLOCKSIZE bytes */
  buf[1] = buf[0] + hmac.GetBlockSize();

  left   = *outlen;
  blkno  = 1;
  stored = 0;
  x = hmac->GetHashLen();

  while (left != 0) {
    /* process block number blkno */
    memset(buf[0], 0, hmac.GetBlockSize() * 2);
       
    /* store current block number and increment for next pass */
    STORE32H(blkno, buf[1]);
    ++blkno;

    /* get PRF(P, S||int(blkno)) */
    hmac.Init(password, password_len);
    hmac.Update(salt, salt_len);
    hmac.Update(buf[1], 4);
    hmac.Final(buf[0]);
    x = hmac.GetBlockSize();

    /* now compute repeated and XOR it in buf[1] */
    memcpy(buf[1], buf[0], x);
    for (itts = 1; itts < iteration_count; ++itts) {
      hmac.Doit(password, password_len, buf[0], x, buf[0]);
      for (y = 0; y < x; y++) {
        buf[1][y] ^= buf[0][y];
      }
    }

    /* now emit upto x bytes of buf[1] to output */
    for (y = 0; y < x && left != 0; ++y) {
      out[stored++] = buf[1][y];
      --left;
    }
  }
  *outlen = stored;

  std::memset(buf[0], 0, hmac.GetBlockSize() * 2);

  delete[] buf[0];
}
