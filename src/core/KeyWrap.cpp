/*
* Copyright (c) 2013-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// KeyWrap.cpp
// C++ wrapper of RFC3394 Keywrap algorithm for PasswordSafe,
// Modified to work with any block cipher, not just AES.
// Based on OpenSSL's aes_wrap.c http://www.openssl.org
//-----------------------------------------------------------------------------

#include "KeyWrap.h"

#include <cstring>

#include "Fish.h"
#include "os/typedefs.h"

static const unsigned char default_iv[] = {
  0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
};

void KeyWrap::Wrap(const unsigned char *in, unsigned char *out,
                   unsigned int inlen)
{
	unsigned char *A, B[16], *R;
	unsigned int i, j, t;
  ASSERT(m_fish != nullptr);
  ASSERT(in != nullptr); ASSERT(out != nullptr);
	ASSERT(!((inlen & 0x7) || (inlen < 8)));
	A = B;
	t = 1;
  std::memcpy(out + 8, in, inlen);
  const unsigned char *iv = default_iv;

  std::memcpy(A, iv, 8);

	for (j = 0; j < 6; j++) {
		R = out + 8;
		for (i = 0; i < inlen; i += 8, t++, R += 8) {
      memcpy(B + 8, R, 8);
      m_fish->Encrypt(B, B);
      A[7] ^= (unsigned char)(t & 0xff);
      if (t > 0xff)	 {
        A[6] ^= (unsigned char)((t >> 8) & 0xff);
        A[5] ^= (unsigned char)((t >> 16) & 0xff);
        A[4] ^= (unsigned char)((t >> 24) & 0xff);
      }
      std::memcpy(R, B + 8, 8);
    }
  }
  std::memcpy(out, A, 8);
}

bool KeyWrap::Unwrap(const unsigned char *in, unsigned char *out,
                     unsigned int inlen)
{
	unsigned char *A, B[16], *R;
	unsigned int i, j, t;
  ASSERT(m_fish != nullptr);
  ASSERT(in != nullptr); ASSERT(out != nullptr);
	inlen -= 8;
	if (inlen & 0x7)
		return false;
	if (inlen < 8)
		return false;
	A = B;
	t =  6 * (inlen >> 3);
  std::memcpy(A, in, 8);
  std::memcpy(out, in + 8, inlen);
	for (j = 0; j < 6; j++) {
    R = out + inlen - 8;
    for (i = 0; i < inlen; i += 8, t--, R -= 8) {
      A[7] ^= (unsigned char)(t & 0xff);
      if (t > 0xff)	{
        A[6] ^= (unsigned char)((t >> 8) & 0xff);
        A[5] ^= (unsigned char)((t >> 16) & 0xff);
        A[4] ^= (unsigned char)((t >> 24) & 0xff);
      }
      std::memcpy(B + 8, R, 8);
      m_fish->Decrypt(B, B);
      std::memcpy(R, B + 8, 8);
    }
  }
  const unsigned char *iv = default_iv;
	return std::memcmp(A, iv, 8) == 0;
}
