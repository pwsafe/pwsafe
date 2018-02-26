/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// BlowFish.h
//-----------------------------------------------------------------------------
#ifndef __BLOWFISH_H
#define __BLOWFISH_H

#include "Fish.h"
#include "os/typedefs.h"

class BlowFish : public Fish
{
public:
  static BlowFish *MakeBlowFish(const unsigned char *pass, unsigned int passlen,
                                const unsigned char *salt, unsigned int saltlen);
// Simple version for protecting ItemFields in memory:
  static BlowFish *MakeBlowFish(const unsigned char *key, int keylen) {
    return new BlowFish(key, keylen);
  }

  static const unsigned int  BLOCKSIZE = 8;

  BlowFish(const unsigned char* key, int keylen);
  ~BlowFish();
  // ensure no copy c'tor or assignment:
  BlowFish(const BlowFish &) = delete;
  BlowFish &operator=(const BlowFish &) = delete;
  
  void Encrypt(const unsigned char *in, unsigned char *out) const;
  void Decrypt(const unsigned char *in, unsigned char *out) const;
  unsigned int GetBlockSize() const {return BLOCKSIZE;}

private:
  static const unsigned int bf_N = 16;
  uint32 bf_S[4][256];
  uint32 bf_P[bf_N + 2];
  static const uint32 tempbf_S[4][256];
  static const uint32 tempbf_P[bf_N + 2];
  void Blowfish_encipher(uint32* xl, uint32* xr) const;
  void Blowfish_decipher(uint32* xl, uint32* xr) const;
  void InitializeBlowfish(const unsigned char key[], short keybytes);
};
#endif /* __BLOWFISH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
