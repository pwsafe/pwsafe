/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Fish.h
#ifndef __FISH_H
#define __FISH_H

#include "../../os/mem.h"
#include "../Util.h"

/**
* Fish is an abstract base class for BlowFish and TwoFish
* (and for any block cipher, but it's cooler to call it "Fish"
* rather than "Cipher"...)
*/

class Fish
{
public:
  Fish() {}
  virtual ~Fish() {}
  virtual unsigned int GetBlockSize() const = 0;
  // Following encrypt/decrypt a single block
  // (blocksize dependent on cipher)
  virtual void Encrypt(const unsigned char *pt, unsigned char *ct) const = 0;
  virtual void Decrypt(const unsigned char *ct, unsigned char *pt) const = 0;
};


/*
* Returns a Fish object set up for encryption or decryption.
*
* The main issue here is that the key is SHA1(passphrase|salt)
* Aside from saving duplicate code, we win here by minimizing the exposure
* of the actual key.
* The lose is that the FishT object is now dynamically allocated.
* This could be fixed by having a ctor of BlowFish that works without a key,
* which would be set by another member function, but I doubt that it's worth the bother.
*
* Note that it's the caller's responsibility to delete the FishT object allocated here
*/
template<typename FishT, typename HashT>
FishT *makeFish(const unsigned char* pass, unsigned int passlen,
  const unsigned char* salt, unsigned int saltlen)
{
  unsigned char passkey[HashT::HASHLEN];
  pws_os::mlock(passkey, sizeof(passkey));

  HashT context;
  context.Update(pass, passlen);
  context.Update(salt, saltlen);
  context.Final(passkey);

  FishT* retval = new FishT(passkey, sizeof(passkey));
  trashMemory(passkey, sizeof(passkey));
  pws_os::munlock(passkey, sizeof(passkey));
  return retval;
}

#endif /* __FISH_H */
