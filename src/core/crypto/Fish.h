/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Fish.h
#ifndef __FISH_H
#define __FISH_H

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

#endif /* __FISH_H */
