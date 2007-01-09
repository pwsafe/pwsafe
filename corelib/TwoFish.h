/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// TwoFish.h
// C++ wrapper of TwoFish for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef _TWOFISH_H_
#define _TWOFISH_H_

#include "Fish.h"

#ifndef TWOFISH_SMALL
   struct twofish_key {
      unsigned long S[4][256], K[40];
   };
#else
   struct twofish_key {
      unsigned long K[40];
      unsigned char S[32], start;
   };
#endif

class TwoFish : public Fish
{
public:
  enum {BLOCKSIZE=16};
  TwoFish(const unsigned char* key, int keylen);
  virtual ~TwoFish();
  virtual void Encrypt(const unsigned char *in, unsigned char *out);
  virtual void Decrypt(const unsigned char *in, unsigned char *out);
  virtual unsigned int GetBlockSize() const {return BLOCKSIZE;}
private:
  twofish_key key_schedule;
};
#endif /* _TWOFISH_H_ */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
