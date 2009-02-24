/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// hmac.h
// HMAC for PasswordSafe
//-----------------------------------------------------------------------------
#ifndef _HMAC_H_
#define _HMAC_H_

// HMAC algorithm as per RFC2104

// Implemented as a template class to save overhead of virtual function calls
// to an abstract Hash base class.
// Works with SHA256 (for pwsafe file format v3) and SHA1 (for Yubikey)

#include "Util.h" // for ASSERT
#include <cstring>

template<class Hash>
class HMAC {
public:
  enum {HASHLEN = Hash::HASHLEN};
  HMAC(const unsigned char *key, unsigned long keylen)
  {
    ASSERT(key != NULL);

    std::memset(K, 0, sizeof(K));
    Init(key, keylen);
  }
  HMAC()
  {
    std::memset(K, 0, sizeof(K));
  }
  ~HMAC() {} // cleanup in Final()

  void Init(const unsigned char *key, unsigned long keylen)
  {
    ASSERT(key != NULL);

    if (keylen > B) {
      Hash H0;
      H0.Update(key, keylen);
      H0.Final(K);
    } else {
      ASSERT(keylen <= sizeof(K));
      std::memcpy(K, key, keylen);
    }

    unsigned char k_ipad[B];
    for (int i = 0; i < B; i++)
      k_ipad[i] = K[i] ^ 0x36;
    H.Update(k_ipad, B);
    std::memset(k_ipad, 0, B);
  }
  void Update(const unsigned char *in, unsigned long inlen)
  {H.Update(in, inlen);}

  void Final(unsigned char digest[HASHLEN])
  {
    unsigned char d[HASHLEN];

    H.Final(d);
    unsigned char k_opad[B];
    for (int i = 0; i < B; i++)
      k_opad[i] = K[i] ^ 0x5c;

    std::memset(K, 0, B);

    Hash H1;
    H1.Update(k_opad, B);
    std::memset(k_opad, 0, B);
    H1.Update(d, HASHLEN);
    std::memset(d, 0, HASHLEN);
    H1.Final(digest);
  }

private:
  Hash H;
  enum {L = HASHLEN, B = Hash::BLOCKSIZE};
  unsigned char K[B];
};

#endif /* _HMAC_H_ */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

