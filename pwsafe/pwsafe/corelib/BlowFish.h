// BlowFish.h
//-----------------------------------------------------------------------------
#ifndef __BLOWFISH_H
#define __BLOWFISH_H

#define MAXKEYBYTES 56 // unused

typedef unsigned char block[8];


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

class BlowFish
{
public:
  static BlowFish *MakeBlowFish(const unsigned char *pass, int passlen,
                                const unsigned char *salt, int saltlen);
  enum {BLOCKSIZE=8};
  BlowFish(unsigned char* key, int keylen);
  ~BlowFish();
  void Encrypt(const block in, block out);
  void Decrypt(const block in, block out);
private:
  enum {bf_N = 16};
  unsigned long bf_S[4][256];
  unsigned long bf_P[bf_N + 2];
  const static unsigned long tempbf_S[4][256];
  const static unsigned long tempbf_P[bf_N + 2];
  void Blowfish_encipher(unsigned long* xl, unsigned long* xr);
  void Blowfish_decipher(unsigned long* xl, unsigned long* xr);
  void InitializeBlowfish(unsigned char key[], short keybytes);
};
#endif /* __BLOWFISH_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
