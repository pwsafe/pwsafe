//-----------------------------------------------------------------------------
#include "blowfish.h"

#define S(x,i) (bf_S[i][x.w.byte##i])
#define bf_F(x) (((S(x,0) + S(x,1)) ^ S(x,2)) + S(x,3))
#define ROUND(a,b,n) (a.word ^= bf_F(b) ^ bf_P[n])

void BlowFish::Blowfish_encipher(unsigned long *xl, unsigned long *xr)
{
   union aword  Xl;
   union aword  Xr;

   Xl.word = *xl;
   Xr.word = *xr;

   Xl.word ^= bf_P[0];
   ROUND (Xr, Xl, 1);  ROUND (Xl, Xr, 2);
   ROUND (Xr, Xl, 3);  ROUND (Xl, Xr, 4);
   ROUND (Xr, Xl, 5);  ROUND (Xl, Xr, 6);
   ROUND (Xr, Xl, 7);  ROUND (Xl, Xr, 8);
   ROUND (Xr, Xl, 9);  ROUND (Xl, Xr, 10);
   ROUND (Xr, Xl, 11); ROUND (Xl, Xr, 12);
   ROUND (Xr, Xl, 13); ROUND (Xl, Xr, 14);
   ROUND (Xr, Xl, 15); ROUND (Xl, Xr, 16);
   Xr.word ^= bf_P[17];

   *xr = Xl.word;
   *xl = Xr.word;
}


void BlowFish::Blowfish_decipher(unsigned long *xl, unsigned long *xr)
{
   union aword  Xl;
   union aword  Xr;

   Xl.word = *xl;
   Xr.word = *xr;

   Xl.word ^= bf_P[17];
   ROUND (Xr, Xl, 16);  ROUND (Xl, Xr, 15);
   ROUND (Xr, Xl, 14);  ROUND (Xl, Xr, 13);
   ROUND (Xr, Xl, 12);  ROUND (Xl, Xr, 11);
   ROUND (Xr, Xl, 10);  ROUND (Xl, Xr, 9);
   ROUND (Xr, Xl, 8);   ROUND (Xl, Xr, 7);
   ROUND (Xr, Xl, 6);   ROUND (Xl, Xr, 5);
   ROUND (Xr, Xl, 4);   ROUND (Xl, Xr, 3);
   ROUND (Xr, Xl, 2);   ROUND (Xl, Xr, 1);
   Xr.word ^= bf_P[0];

   *xl = Xr.word;
   *xr = Xl.word;
}


void BlowFish::InitializeBlowfish(unsigned char key[], short keybytes)
{
   short          i;
   short          j;
   unsigned long  data;
   unsigned long  datal;
   unsigned long  datar;
   union aword temp;

   j = 0;
   for (i = 0; i < bf_N + 2; ++i)
   {
      temp.word = 0;
      temp.w.byte0 = key[j];
      temp.w.byte1 = key[(j+1)%keybytes];
      temp.w.byte2 = key[(j+2)%keybytes];
      temp.w.byte3 = key[(j+3)%keybytes];
      data = temp.word;
      bf_P[i] = bf_P[i] ^ data;
      j = (j + 4) % keybytes;
   }

   datal = 0x00000000;
   datar = 0x00000000;

   for (i = 0; i < bf_N + 2; i += 2)
   {
      Blowfish_encipher(&datal, &datar);

      bf_P[i] = datal;
      bf_P[i + 1] = datar;
   }

   for (i = 0; i < 4; ++i)
   {
      for (j = 0; j < 256; j += 2)
      {
         Blowfish_encipher(&datal, &datar);

         bf_S[i][j] = datal;
         bf_S[i][j + 1] = datar;
      }
   }
}


BlowFish::BlowFish(unsigned char *key,int keylen)
{
   /*
     bf_P and bf_S, for speed reasons, are static global variables.
     the temp... versions never are changed and are copied to the 
     real" ones used by the actual algorithm. These can change, as
     they are dependent on passkeys. The "real" and the temp are trashed
     in ~CPasswordSafeApp(), because after that they are surely not needed.
   */

   int x,y;
   for (x=0;x<18;x++)
      bf_P[x] = tempbf_P[x];
   for (x=0;x<4;x++)
   {
      for (y=0;y<256;y++)
      {
         bf_S[x][y] = tempbf_S[x][y];
      }
   }

   InitializeBlowfish(key,keylen);
}

void BlowFish::Encrypt(block in,block out)
{
   for (int x=0;x<8;x++)
      out[x] = in[x];
   Blowfish_encipher((unsigned long*)out,
                     (unsigned long*)(out+sizeof(unsigned long)));
}

void BlowFish::Decrypt(block in,block out)
{
   for (int x=0;x<8;x++)
      out[x] = in[x];
   Blowfish_decipher((unsigned long*)out,
                     (unsigned long*)(out+sizeof(unsigned long)));
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
