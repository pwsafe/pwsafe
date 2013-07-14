/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file hmac.h
// HMAC for PasswordSafe

//-----------------------------------------------------------------------------
#ifndef __HMAC_H
#define __HMAC_H

// HMAC algorithm as per RFC2104

#include "Util.h" // for ASSERT

template<class H, int HASHLEN, int BLOCKSIZE>
class HMAC
{
public:
  HMAC(const unsigned char *key, unsigned long keylen)
  {
    ASSERT(key != NULL);
    
    memset(K, 0, sizeof(K));
    Init(key, keylen);
  }

  HMAC()
  { // Init needs to be called separately
    memset(K, 0, sizeof(K));
  }

  ~HMAC(){/* cleaned up in Final */}

  int GetBlockSize() const {return BLOCKSIZE;}
  int GetHashLen() const {return HASHLEN;}

  void Init(const unsigned char *key, unsigned long keylen)
  {
    ASSERT(key != NULL);

    if (keylen > BLOCKSIZE) {
      H H0;
      H0.Update(key, keylen);
      H0.Final(K);
    } else {
      ASSERT(keylen <= sizeof(K));
      memcpy(K, key, keylen);
    }

    unsigned char k_ipad[BLOCKSIZE];
    for (int i = 0; i < BLOCKSIZE; i++)
      k_ipad[i] = K[i] ^ 0x36;
    Hash.Update(k_ipad, BLOCKSIZE);
    memset(k_ipad, 0, BLOCKSIZE);
  }

  void Update(const unsigned char *in, unsigned long inlen)
  {
    Hash.Update(in, inlen);
  }

  void Final(unsigned char digest[HASHLEN])
  {
    unsigned char d[HASHLEN];

    Hash.Final(d);
    unsigned char k_opad[BLOCKSIZE];
    for (int i = 0; i < BLOCKSIZE; i++)
      k_opad[i] = K[i] ^ 0x5c;

    memset(K, 0, BLOCKSIZE);

    H H1;
    H1.Update(k_opad, BLOCKSIZE);
    memset(k_opad, 0, BLOCKSIZE);
    H1.Update(d, HASHLEN);
    memset(d, 0, HASHLEN);
    H1.Final(digest);
  }

  void Doit(const unsigned char *key, unsigned long keylen,
            const unsigned char *in, unsigned long inlen,
            unsigned char digest[HASHLEN])
  {Init(key, keylen); Update(in, inlen); Final(digest);}

private:
  H Hash;
  unsigned char K[BLOCKSIZE];
};

class HMAC_SHA256
{
public:
  enum {HASHLEN = 32};
  HMAC_SHA256(const unsigned char *key, unsigned long keylen); // Calls Init
  HMAC_SHA256(); // Init needs to be called separately
  ~HMAC_SHA256();
  void Init(const unsigned char *key, unsigned long keylen);
  void Update(const unsigned char *in, unsigned long inlen);
  void Final(unsigned char digest[HASHLEN]);

private:
  SHA256 H;
  /* for SHA256 hashsize(L) = 32, blocksize(B) = 64 */
  enum {L = 32, B = 64};
  unsigned char K[B];
};

#endif /* __HMAC_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

