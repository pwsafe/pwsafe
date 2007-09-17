/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "hmac.h"
#include "Util.h" // for ASSERT
#include <string.h>

HMAC_SHA256::HMAC_SHA256(const unsigned char *key, unsigned long keylen)
{
  ASSERT(key != NULL);

  memset(K, 0, sizeof(K));
  Init(key, keylen);
}

HMAC_SHA256::HMAC_SHA256()
{
  memset(K, 0, sizeof(K));
}

void
HMAC_SHA256::Init(const unsigned char *key, unsigned long keylen)
{
  ASSERT(key != NULL);

  if (keylen > B) {
    SHA256 H0;
    H0.Update(key, keylen);
    H0.Final(K);
  } else {
    ASSERT(keylen <= sizeof(K));
    memcpy(K, key, keylen);
  }

  unsigned char k_ipad[B];
  for (int i = 0; i < B; i++)
    k_ipad[i] = K[i] ^ 0x36;
  H.Update(k_ipad, B);
  memset(k_ipad, 0, B);
}

HMAC_SHA256::~HMAC_SHA256()
{
  // cleaned up in Final
}

void HMAC_SHA256::Update(const unsigned char *in, unsigned long inlen)
{
  H.Update(in, inlen);
}

void HMAC_SHA256::Final(unsigned char digest[SHA256::HASHLEN])
{
    unsigned char d[HASHLEN];

  H.Final(d);
  unsigned char k_opad[B];
  for (int i = 0; i < B; i++)
    k_opad[i] = K[i] ^ 0x5c;
  memset(K, 0, B);

  SHA256 H1;
  H1.Update(k_opad, B);
  memset(k_opad, 0, B);
  H1.Update(d, HASHLEN);
  memset(d, 0, HASHLEN);
  H1.Final(digest);
}
