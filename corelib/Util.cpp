/// \file Util.cpp
//-----------------------------------------------------------------------------

#include "sha1.h"
#include "BlowFish.h"

#include <fcntl.h>
#include <errno.h>
#include <io.h>
#include <sys\stat.h>

#include "Util.h"

// for now
#define String CString
#define SecString CMyString

// used by CBC routines...
static void
xormem(unsigned char* mem1, unsigned char* mem2, int length)
{
   for (int x=0;x<length;x++)
      mem1[x] ^= mem2[x];
}



//-----------------------------------------------------------------------------
/*
  Note: A bunch of the encryption-related routines may not be Unicode
  compliant.  Which really isn't a huge problem, since they are actually
  using MFC routines anyway
*/

//-----------------------------------------------------------------------------
//Overwrite the memory

void
trashMemory(SHA1_CTX& context)
{
   trashMemory((unsigned char*)context.state, sizeof context.state);
   trashMemory((unsigned char*)context.count, sizeof context.count);
   trashMemory((unsigned char*)context.buffer, sizeof context.buffer);
}


void
trashMemory(unsigned char* buffer,
            long length,
            int numiter) // default 30
{
   for (int x=0; x<numiter; x++)
   {
      memset(buffer, 0x00, length);
      memset(buffer, 0xFF, length);
      memset(buffer, 0x00, length);
   }
}

//Generates a passkey-based hash from stuff - used to validate the passkey
void
GenRandhash(const CMyString &a_passkey,
            const unsigned char* a_randstuff,
            unsigned char* a_randhash)
{
  const LPCSTR pkey = (const LPCSTR)a_passkey;
   /*
     I'm not quite sure what this is doing, so as I figure out each piece,
     I'll add more comments {jpr}
   */

   /*
     tempSalt <- H(a_randstuff + a_passkey)
   */
   SHA1_CTX keyHash;
   SHA1Init(&keyHash);
   SHA1Update(&keyHash, a_randstuff, StuffSize);
   SHA1Update(&keyHash,
              (const unsigned char*)pkey,
              a_passkey.GetLength());

   unsigned char tempSalt[20]; // HashSize
   SHA1Final(tempSalt, &keyHash);

   /*
     tempbuf <- a_randstuff encrypted 1000 times using tempSalt as key?
   */
	
   BlowFish Cipher(tempSalt, sizeof(tempSalt));
	
   unsigned char tempbuf[StuffSize];
   memcpy((char*)tempbuf, (char*)a_randstuff, StuffSize);

   for (int x=0; x<1000; x++)
      Cipher.Encrypt(tempbuf, tempbuf);

   /*
     hmm - seems we're not done with this context
     we throw the tempbuf into the hasher, and extract a_randhash
   */
   SHA1Update(&keyHash, tempbuf, StuffSize);
   SHA1Final(a_randhash, &keyHash);
   trashMemory(keyHash);
}



unsigned char
newrand()
{
   int	r;
   while ((r = rand()) % 257 == 256)
      ; // 257?!?
   return (unsigned char)r;
}


char
GetRandAlphaNumChar()
{
   int temp = newrand() % 3;

   if (temp == 0)
      return char(((newrand() % ('9'-'0')) + '0'));
   else if (temp == 1)
      return char(((newrand() % ('Z'-'A')) + 'A'));
   else
      return char(((newrand() % ('z'-'a')) + 'a'));
}


/*
 * Returns a BlowFish object set up for encryption or decrytion.
 *
 * The main issue here is that the BlowFish key is SHA1(passphrase|salt)
 * Aside from saving duplicate code, we win here by minimizing the exposure
 * of the actual key.
 * The lose is that the BlowFish object is now dynamically allocated.
 * This could be fixed by having a ctor of BlowFish that works without a key,
 * which would be set by another member function, but I doubt that it's worth the bother.
 *
 * Note that it's the caller's responsibility to delete the BlowFish object allocated here
 */

BlowFish *MakeBlowFish(const unsigned char *pass, int passlen,
		       const unsigned char *salt, int saltlen)
{
   unsigned char passkey[20]; // SHA1 digest is 20 bytes - why isn't there a constant for this?
   VirtualLock(passkey, sizeof(passkey));

   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context, pass, passlen);
   SHA1Update(&context, salt, saltlen);
   SHA1Final(passkey, &context);
   BlowFish *retval = new BlowFish(passkey, sizeof(passkey));
   trashMemory(passkey, sizeof(passkey));
   trashMemory(context);
   VirtualUnlock(passkey, sizeof(passkey));
   
   return retval;
}

int
_writecbc(int fp,
          const unsigned char* buffer,
          int length,
	  const unsigned char *pass, int passlen,
          const unsigned char* salt, int saltlen,
          unsigned char* cbcbuffer)
{
   int numWritten = 0;

   int BlockLength = ((length+7)/8)*8;
   if (BlockLength == 0)
      BlockLength = 8;

   BlowFish *Algorithm = MakeBlowFish(pass, passlen, salt, saltlen);

   // First encrypt and write the length of the buffer
   unsigned char lengthblock[8];
   memset(lengthblock, 0, 8);
   // XXX next line is a portability issue - what if file is read by a program
   // compiled with a different sizeof int or different endian-ness?
   memcpy(lengthblock, (unsigned char*)&length, sizeof length);
   xormem(lengthblock, cbcbuffer, 8); // do the CBC thing
   Algorithm->Encrypt(lengthblock, lengthblock);
   memcpy(cbcbuffer, lengthblock, 8); // update CBC for next round
   numWritten = _write(fp, lengthblock, 8);
   trashMemory(lengthblock, 8);

   // Now, encrypt and write the buffer
   unsigned char curblock[8];
   for (int x=0;x<BlockLength;x+=8)
   {
      if ((length == 0) || ((length%8 != 0) && (length-x<8)))
      {
         //This is for an uneven last block
         memset(curblock, 0, 8);
         memcpy(curblock, buffer+x, length % 8);
      }
      else
         memcpy(curblock, buffer+x, 8);
      xormem(curblock, cbcbuffer, 8);
      Algorithm->Encrypt(curblock, curblock);
      memcpy(cbcbuffer, curblock, 8);
      numWritten += _write(fp, curblock, 8);
   }
   trashMemory(curblock, 8);
   delete Algorithm;
   return numWritten;
}

/*
 * Reads an encrypted record into buffer.
 * The first block of the record contains the encrypted record length
 * We have the usual ugly problem of fixed buffer lengths in C/C++.
 * Don't want to use CStrings, for future portability.
 * Best solution would be STL strings, but not enough experience with them (yet)
 * So for the meantime, we're going to allocate the buffer here, to ensure that it's long
 * enough.
 * *** THE CALLER MUST delete[] IT AFTER USE *** UGH++
 *
 * An alternative to STL strings would be to accept a buffer, and allocate a replacement
 * iff the buffer is too small. This is serious ugliness, but should be considered if
 * the new/delete performance hit is too big.
 */
int
_readcbc(int fp,
         unsigned char* &buffer, unsigned int &buffer_len,
	 const unsigned char *pass, int passlen,
         const unsigned char* salt, int saltlen,
         unsigned char* cbcbuffer)
{
   int numRead = 0;

   BlowFish *Algorithm = MakeBlowFish(pass, passlen, salt, saltlen);

   unsigned char lengthblock[8];
   unsigned char lcpy[8];
   numRead = _read(fp, lengthblock, 8);
   if (numRead != 8)
      return 0;
   memcpy(lcpy, lengthblock, 8);
   Algorithm->Decrypt(lengthblock, lengthblock);
   xormem(lengthblock, cbcbuffer, 8);
   memcpy(cbcbuffer, lcpy, 8);

   // portability issue - see comment in _writecbc
   int length = *((int*)lengthblock);

   trashMemory(lengthblock, 8);
   trashMemory(lcpy, 8);


   if (length < 0) {
     delete Algorithm;
     buffer = NULL;
     buffer_len = 0;
     return 0;
   }

   int BlockLength = ((length+7)/8)*8;
   if (BlockLength == 0)
      BlockLength = 8;

   buffer_len = length;
   buffer = new unsigned char[BlockLength]; // so we lie a little...


   unsigned char tempcbc[8];
   numRead += _read(fp, buffer, BlockLength);
   for (int x=0;x<BlockLength;x+=8)
   {
      memcpy(tempcbc, buffer+x, 8);
      Algorithm->Decrypt(buffer+x, buffer+x);
      xormem(buffer+x, cbcbuffer, 8);
      memcpy(cbcbuffer, tempcbc, 8);
   }
	
   trashMemory(tempcbc, 8);
   delete Algorithm;

   return numRead;
}
