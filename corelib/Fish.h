// Fish.h
#ifndef __FISH_H
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
  virtual void Encrypt(const unsigned char *pt, unsigned char *ct) = 0;
  virtual void Decrypt(const unsigned char *ct, unsigned char *pt) = 0;
};

#define __FISH_H
#endif /* __FISH_H */
