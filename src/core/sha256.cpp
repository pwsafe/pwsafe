/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// sha256.cpp
// SHA256 for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#include "sha256.h"
#include "PwsPlatform.h"
#include "Util.h"

#include <algorithm>

#define LTC_CLEAN_STACK

/* hashsize = 32, blocksize = 64 */

#ifdef LTC_SMALL_CODE
/* the K array */
static const unsigned long K[64] = {
  0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
  0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
  0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
  0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
  0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
  0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
  0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
  0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
  0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
  0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
  0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
  0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
  0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};
#endif

/* Various logical functions */
#define Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define Maj(x,y,z)      (((x | y) & z) | (x & y)) 
#define S(x, n)         RORc((x),(n))
#define R(x, n)         (((x)&0xFFFFFFFFUL)>>(n))
#define Sigma0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))

/* compress 512-bits */
#ifdef LTC_CLEAN_STACK
static void _sha256_compress(ulong32 state[8], const unsigned char *buf)
#else
static void  sha256_compress(ulong32 state[8], const unsigned char *buf)
#endif
{
  unsigned long S[8], W[64], t0, t1;
#ifdef LTC_SMALL_CODE
  unsigned long t;
#endif
  int i;

  /* copy state into S */
  for (i = 0; i < 8; i++) {
    S[i] = state[i];
  }

  /* copy the state into 512-bits into W[0..15] */
  for (i = 0; i < 16; i++) {
    LOAD32H(W[i], buf + (4*i));
  }

  /* fill W[16..63] */
  for (i = 16; i < 64; i++) {
    W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];
  }        

  /* Compress */
#ifdef LTC_SMALL_CODE   
#define RND(a,b,c,d,e,f,g,h,i)                         \
  t0 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];   \
  t1 = Sigma0(a) + Maj(a, b, c);                    \
  d += t0;                                          \
  h  = t0 + t1;

  for (i = 0; i < 64; ++i) {
    RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i);
    t = S[7]; S[7] = S[6]; S[6] = S[5]; S[5] = S[4]; 
    S[4] = S[3]; S[3] = S[2]; S[2] = S[1]; S[1] = S[0]; S[0] = t;
  }  
#else 
#define RND(a,b,c,d,e,f,g,h,i,ki)                    \
  t0 = h + Sigma1(e) + Ch(e, f, g) + ki + W[i];   \
  t1 = Sigma0(a) + Maj(a, b, c);                  \
  d += t0;                                        \
  h  = t0 + t1;

  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],0,0x428a2f98);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],1,0x71374491);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],2,0xb5c0fbcf);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],3,0xe9b5dba5);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],4,0x3956c25b);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],5,0x59f111f1);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],6,0x923f82a4);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],7,0xab1c5ed5);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],8,0xd807aa98);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],9,0x12835b01);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],10,0x243185be);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],11,0x550c7dc3);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],12,0x72be5d74);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],13,0x80deb1fe);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],14,0x9bdc06a7);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],15,0xc19bf174);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],16,0xe49b69c1);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],17,0xefbe4786);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],18,0x0fc19dc6);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],19,0x240ca1cc);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],20,0x2de92c6f);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],21,0x4a7484aa);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],22,0x5cb0a9dc);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],23,0x76f988da);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],24,0x983e5152);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],25,0xa831c66d);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],26,0xb00327c8);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],27,0xbf597fc7);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],28,0xc6e00bf3);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],29,0xd5a79147);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],30,0x06ca6351);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],31,0x14292967);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],32,0x27b70a85);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],33,0x2e1b2138);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],34,0x4d2c6dfc);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],35,0x53380d13);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],36,0x650a7354);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],37,0x766a0abb);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],38,0x81c2c92e);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],39,0x92722c85);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],40,0xa2bfe8a1);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],41,0xa81a664b);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],42,0xc24b8b70);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],43,0xc76c51a3);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],44,0xd192e819);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],45,0xd6990624);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],46,0xf40e3585);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],47,0x106aa070);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],48,0x19a4c116);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],49,0x1e376c08);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],50,0x2748774c);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],51,0x34b0bcb5);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],52,0x391c0cb3);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],53,0x4ed8aa4a);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],54,0x5b9cca4f);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],55,0x682e6ff3);
  RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],56,0x748f82ee);
  RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],57,0x78a5636f);
  RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],58,0x84c87814);
  RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],59,0x8cc70208);
  RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],60,0x90befffa);
  RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],61,0xa4506ceb);
  RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],62,0xbef9a3f7);
  RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],63,0xc67178f2);

#undef RND     

#endif     

  /* feedback */
  for (i = 0; i < 8; i++) {
    state[i] = state[i] + S[i];
  }
}

#ifdef LTC_CLEAN_STACK
static void sha256_compress(ulong32 state[8], const unsigned char *buf)
{
  _sha256_compress(state, buf);
  burnStack(sizeof(unsigned long) * 74);
}
#endif

/*
  Initialize the hash state
*/
SHA256::SHA256()
{
  curlen = 0;
  length = 0;
  state[0] = 0x6A09E667UL;
  state[1] = 0xBB67AE85UL;
  state[2] = 0x3C6EF372UL;
  state[3] = 0xA54FF53AUL;
  state[4] = 0x510E527FUL;
  state[5] = 0x9B05688CUL;
  state[6] = 0x1F83D9ABUL;
  state[7] = 0x5BE0CD19UL;
}

SHA256::~SHA256()
{
  // sanitizing done in Final()
}

/*
  Process a block of memory though the hash
  @param in     The data to hash
  @param inlen  The length of the data (octets)
*/
void SHA256::Update(const unsigned char *in, size_t inlen)
{
  const size_t block_size = 64;
  size_t n;
  ASSERT(in != nullptr || inlen == 0);
  ASSERT(curlen <= sizeof(buf));
  while (inlen > 0) {
    if (curlen == 0 && inlen >= block_size) {
      sha256_compress(state, in);
      length += block_size * 8;
      in             += block_size;
      inlen          -= block_size;
    } else {
      n = std::min(inlen, (block_size - curlen));
      memcpy(buf + curlen, in, static_cast<size_t>(n));
      curlen += n;
      in             += n;
      inlen          -= n;
      if (curlen == block_size) {
        sha256_compress(state, buf);
        length += 8*block_size;
        curlen = 0;
      }
    }
  }
}

/*
  Terminate the hash to get the digest
  @param digest The destination of the hash (32 bytes)
*/
void SHA256::Final(unsigned char digest[HASHLEN])
{
  int i;

  ASSERT(digest != nullptr);

  ASSERT(curlen < sizeof(buf));

  /* increase the length of the message */
  length += curlen * 8;

  /* append the '1' bit */
  buf[curlen++] = static_cast<unsigned char>(0x80);

  /* if the length is currently above 56 bytes we append zeros
  * then compress.  Then we can fall back to padding zeros and length
  * encoding like normal.
  */
  if (curlen > 56) {
    while (curlen < 64) {
      buf[curlen++] = 0;
    }
    sha256_compress(state, buf);
    curlen = 0;
  }

  /* pad up to 56 bytes of zeroes */
  while (curlen < 56) {
    buf[curlen++] = 0;
  }

  /* store length */
  STORE64H(length, buf+56);
  sha256_compress(state, buf);

  /* copy output */
  for (i = 0; i < 8; i++) {
    STORE32H(state[i], digest+(4*i));
  }
#ifdef LTC_CLEAN_STACK
  trashMemory(state, sizeof(state));
  trashMemory(buf, sizeof(buf));
#endif
}
