// sha256.h
// SHA256 for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef _SHA256_H_
#define _SHA256_H_
#include "typedefs.h"
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

#endif /* _SHA256_H_ */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

