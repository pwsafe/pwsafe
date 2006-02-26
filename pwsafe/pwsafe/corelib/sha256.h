// sha256.h
// SHA256 for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef _SHA256_H_
#define _SHA256_H_

#include "Util.h" // for typedefs

class SHA256
{
public:
  enum {HASHLEN = 32};
  SHA256();
  ~SHA256();
  void Update(const unsigned char *in, unsigned long inlen);
  void Final(unsigned char digest[HASHLEN]);
private:
  ulong64 length;
  ulong32 state[8], curlen;
  unsigned char buf[64];
};

// HMAC algorithm as per RFC2104 with SHA-256
// Generalizing this to other hashes is left as an exercise to the reader...


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

#endif /* _SHA256_H_ */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

