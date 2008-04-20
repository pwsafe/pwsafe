/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// BlowFish.h
//-----------------------------------------------------------------------------
#ifndef __BLOWFISH_H
#define __BLOWFISH_H

#include "PwsPlatform.h"
#include "Fish.h"

#define MAXKEYBYTES 56 // unused

union aword
{
   unsigned long word;
   unsigned char byte [4];
   struct
   {
      unsigned int byte3:8;
      unsigned int byte2:8;
      unsigned int byte1:8;
      unsigned int byte0:8;
   } w;
};

class BlowFish : public Fish
{
public:
  static BlowFish *MakeBlowFish(const unsigned char *pass, int passlen,
                                const unsigned char *salt, int saltlen);
  enum {BLOCKSIZE=8};
  BlowFish(unsigned char* key, int keylen);
  virtual ~BlowFish();
  virtual void Encrypt(const unsigned char *in, unsigned char *out);
  virtual void Decrypt(const unsigned char *in, unsigned char *out);
  virtual unsigned int GetBlockSize() const {return BLOCKSIZE;}
private:
  enum {bf_N = 16};
  unsigned long bf_S[4][256];
  unsigned long bf_P[bf_N + 2];
  static const unsigned long tempbf_S[4][256];
  static const unsigned long tempbf_P[bf_N + 2];
  void Blowfish_encipher(unsigned long* xl, unsigned long* xr);
  void Blowfish_decipher(unsigned long* xl, unsigned long* xr);
  void InitializeBlowfish(unsigned char key[], short keybytes);
};
#endif /* __BLOWFISH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
