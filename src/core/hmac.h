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

class HMAC_BASE
{
public:
  HMAC_BASE() {}
  virtual ~HMAC_BASE() {}

  virtual int GetBlockSize() const = 0;
  virtual int GetHashLen() const = 0;

  virtual void Init(const unsigned char *key, unsigned long keylen) = 0;
  virtual void Update(const unsigned char *in, unsigned long inlen) = 0;
  virtual void Final(unsigned char digest[]) = 0;

  void Doit(const unsigned char *key, unsigned long keylen,
            const unsigned char *in, unsigned long inlen,
            unsigned char digest[])
  {Init(key, keylen); Update(in, inlen); Final(digest);}
};

template<class H, int HASHLEN, int BLOCKSIZE>
class HMAC : public HMAC_BASE
{
public:
  HMAC(const unsigned char *key, unsigned long keylen)
    : HMAC_BASE(), Hash(0)
  {
    ASSERT(key != NULL);
    
    memset(K, 0, sizeof(K));
    Init(key, keylen);
  }

  HMAC() : HMAC_BASE(), Hash(0)
  { // Init needs to be called separately
    memset(K, 0, sizeof(K));
  }

  ~HMAC(){delete Hash;}

  int GetBlockSize() const {return BLOCKSIZE;}
  int GetHashLen() const {return HASHLEN;}
  bool IsInited() const {return Hash != NULL;}

  void Init(const unsigned char *key, unsigned long keylen)
  {
    ASSERT(key != NULL);
    ASSERT(Hash == NULL);
    Hash = new H; // to ensure state's cleared.

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
    Hash->Update(k_ipad, BLOCKSIZE);
    memset(k_ipad, 0, BLOCKSIZE);
  }

  void Update(const unsigned char *in, unsigned long inlen)
  {
    ASSERT(Hash != NULL);
    Hash->Update(in, inlen);
  }

  void Final(unsigned char digest[HASHLEN])
  {
    unsigned char d[HASHLEN];
    ASSERT(Hash != NULL);

    Hash->Final(d);
    delete(Hash);
    Hash = NULL;
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

private:
  H *Hash;
  unsigned char K[BLOCKSIZE];
};

#endif /* __HMAC_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

