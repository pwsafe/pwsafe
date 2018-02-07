/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file sha1.cpp
//-----------------------------------------------------------------------------

/* based on SHA-1 in C By Steve Reid <steve@edmweb.com> */

#include "PwsPlatform.h"
//#define SHA1HANDSOFF Copies data before messing with it.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sha1.h"

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef PWS_LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
  |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
  ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */

static void SHA1Transform(uint32 state[5],
                          const unsigned char buffer[64])
{
  uint32 a, b, c, d, e;
  typedef union {
    unsigned char c[64];
    uint32 l[16];
  } CHAR64LONG16;
  CHAR64LONG16* block;
//commented, because otherwise we change the buffer
//#ifdef SHA1HANDSOFF
  static unsigned char workspace[64];
  block = reinterpret_cast<CHAR64LONG16 *>(workspace);
  memcpy(block, buffer, 64);
//#else
//  block = reinterpret_cast<const CHAR64LONG16 *>(buffer);
//#endif
  /* Copy context->state[] to working vars */
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  /* 4 rounds of 20 operations each. Loop unrolled. */
  R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
  R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
  R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
  R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
  R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
  R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
  R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
  R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
  R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
  R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
  R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
  R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
  R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
  R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
  R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
  R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
  R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
  R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
  R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
  R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
  /* Add the working vars back into context.state[] */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  /* Wipe variables */
  a = b = c = d = e = 0;
}

SHA1::SHA1()
{
  /* SHA1 initialization constants */
  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
  state[4] = 0xC3D2E1F0;
  count[0] = count[1] = 0;  
}

/* Run your data through this. */
void SHA1::Update(const unsigned char* data, unsigned int len)
{
  unsigned int i, j;

  j = (count[0] >> 3) & 63;
  if ((count[0] += len << 3) < (len << 3)) count[1]++;
  count[1] += (len >> 29);
  if ((j + len) > 63) {
    memcpy(&buffer[j], data, (i = 64-j));
    SHA1Transform(state, buffer);
    for ( ; i + 63 < len; i += 64) {
      SHA1Transform(state, &data[i]);
    }
    j = 0;
  }
  else i = 0;
  memcpy(&buffer[j], &data[i], len - i);
}

/* Add padding and return the message digest. */
void SHA1::Final(unsigned char digest[HASHLEN])
{
  unsigned long i;
  unsigned char finalcount[8];

  for (i = 0; i < 8; i++) {
    finalcount[i] = static_cast<unsigned char>((count[(i >= 4 ? 0 : 1)]
    >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
  }
  Update(reinterpret_cast<const unsigned char *>("\200"), 1);
  while ((count[0] & 504) != 448) {
    Update(reinterpret_cast<const unsigned char *>("\0"), 1);
  }
  Update(finalcount, 8);  /* Should cause a SHA1Transform() */
  for (i = 0; i < 20; i++) {
    digest[i] = static_cast<unsigned char>
      ((state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
  }
  /* Wipe variables */
  memset(buffer, 0, 64);
  memset(state, 0, 20);
  memset(count, 0, 8);
  memset(finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
  SHA1Transform(state, buffer);
#endif
}

SHA1::~SHA1()
{
  // first thought was to clear context here, but better to do this
  // as early as possible, in Final().
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
