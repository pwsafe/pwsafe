//-----------------------------------------------------------------------------

#include "util.h"

#define MAXKEYBYTES 56

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
   BlowFish(unsigned char *key,int keylen);
   void Encrypt(block in,block out);
   void Decrypt(block in,block out);
private:
   void Blowfish_encipher(unsigned long *xl, unsigned long *xr);
   void Blowfish_decipher(unsigned long *xl, unsigned long *xr);
   void InitializeBlowfish(unsigned char key[], short keybytes);
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
