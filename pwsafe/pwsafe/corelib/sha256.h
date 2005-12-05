// sha256.h
// SHA256 for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef _SHA256_H_
#define _SHA256_H_

#include "Util.h" // for typedefs

typedef struct {
    ulong64 length;
    ulong32 state[8], curlen;
    unsigned char buf[64];
} sha256_state;

void sha256_init(sha256_state * md);
void sha256_update(sha256_state *md, const unsigned char *in, unsigned long inlen);
void sha256_done(sha256_state * md, unsigned char *out);

#endif /* _SHA256_H_ */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

