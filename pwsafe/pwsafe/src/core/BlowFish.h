/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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

union aword
{
  uint32 word;
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

  enum {BLOCKSIZE = 8};

  BlowFish(const unsigned char* key, int keylen);
  virtual ~BlowFish();
  virtual void Encrypt(const unsigned char *in, unsigned char *out);
  virtual void Decrypt(const unsigned char *in, unsigned char *out);
  virtual unsigned int GetBlockSize() const {return BLOCKSIZE;}

private:
  enum {bf_N = 16};
  uint32 bf_S[4][256];
  uint32 bf_P[bf_N + 2];
  static const uint32 tempbf_S[4][256];
  static const uint32 tempbf_P[bf_N + 2];
  void Blowfish_encipher(uint32* xl, uint32* xr);
  void Blowfish_decipher(uint32* xl, uint32* xr);
  void InitializeBlowfish(const unsigned char key[], short keybytes);
};
#endif /* __BLOWFISH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
