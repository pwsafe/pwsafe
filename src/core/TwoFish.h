/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// TwoFish.h
// C++ wrapper of TwoFish for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef __TWOFISH_H
#define __TWOFISH_H

#include "Fish.h"
#include "os/typedefs.h"

#ifndef TWOFISH_SMALL
struct twofish_key {
  uint32 S[4][256], K[40];
};
#else
struct twofish_key {
  uint32 K[40];
  unsigned char S[32], start;
};
#endif

class TwoFish : public Fish
{
public:
  static const unsigned int BLOCKSIZE = 16;
  TwoFish(const unsigned char* key, int keylen);
  ~TwoFish();
  void Encrypt(const unsigned char *in, unsigned char *out) const;
  void Decrypt(const unsigned char *in, unsigned char *out) const;
  unsigned int GetBlockSize() const {return BLOCKSIZE;}

private:
  twofish_key key_schedule;
};
#endif /* __TWOFISH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
