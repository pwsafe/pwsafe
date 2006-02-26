/// \file Util.cpp
//-----------------------------------------------------------------------------

#include "sha1.h"
#include "BlowFish.h"
#include "PwsPlatform.h"

#include <stdio.h>

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
trashMemory(void* buffer, long length)
{
  ASSERT(buffer != NULL);
  // {kjp} no point in looping around doing nothing is there?
  if ( length != 0 )
    {
      const int numiter = 30;
      for (int x=0; x<numiter; x++)
        {
          memset(buffer, 0x00, length);
          memset(buffer, 0xFF, length);
          memset(buffer, 0x00, length);
        }
    }
}

void
trashMemory( LPTSTR buffer, long length )
{
  trashMemory( (unsigned char *) buffer, length * sizeof(buffer[0])  );
}

/**
   Burn some stack memory
   @param len amount of stack to burn in bytes
*/
void burnStack(unsigned long len)
{
   unsigned char buf[32];
   trashMemory(buf, sizeof(buf));
   if (len > (unsigned long)sizeof(buf))
      burnStack(len - sizeof(buf));
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
   SHA1 keyHash;
   keyHash.Update(a_randstuff, StuffSize);
   keyHash.Update((const unsigned char*)pkey, a_passkey.GetLength());

   unsigned char tempSalt[20]; // HashSize
   keyHash.Final(tempSalt);

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
   keyHash.Update(tempbuf, StuffSize);
   keyHash.Final(a_randhash);
}



unsigned char
randchar()
{
   int	r;
   while ((r = rand()) % 257 == 256)
      ; // 257?!?
   return (unsigned char)r;
}

// See the MSDN documentation for RtlGenRandom. We will try to load it
// and if that fails, use the simple random number generator. The function
// call is indirected through a function pointer, which initially points
// to a function that tries to load RtlGenRandom

static BOOLEAN __stdcall LoadRandomDataFunction(void *, ULONG);
static BOOLEAN __stdcall MyGetRandomData( PVOID buffer, ULONG length );
static BOOLEAN (APIENTRY *pfnGetRandomData)(void*, ULONG) = LoadRandomDataFunction;

static BOOLEAN __stdcall MyGetRandomData( PVOID buffer, ULONG length )
{
  BYTE * const pb = reinterpret_cast<BYTE *>( buffer );
  for( unsigned int ib = 0; ib < length; ++ib )
  {
    pb[ib] = randchar();
  }
  return TRUE;
}

static BOOLEAN __stdcall LoadRandomDataFunction(void * pv, ULONG cb)
{
  //  this is the default function we'll use if loading RtlGenRandom fails
  pfnGetRandomData = MyGetRandomData;

  HMODULE hLib = LoadLibrary("ADVAPI32.DLL");
  if (hLib)
  {
    BOOLEAN (APIENTRY *pfnGetRandomDataT)(void*, ULONG);
    pfnGetRandomDataT = (BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib,"SystemFunction036");
    if (pfnGetRandomDataT)
    {
      pfnGetRandomData = pfnGetRandomDataT;
    }
  }
  return (*pfnGetRandomData)(pv, cb );
}
 
void GetRandomData( void * const buffer, unsigned long length )
{
  (void)(*pfnGetRandomData)(buffer, length);
}


// generate random numbers from a buffer filled in by GetRandomData()
// NOTE: not threadsafe. the static data in the function can't
// be used by multiple threads. hack it with __threadlocal,
// make it an object or something like that if you need multi-threading
 unsigned int MyRand()
{
  // we don't want to keep filling the random buffer for each number we
  // want, so fill the buffer with random data and use it up

  static const cbRandomData = 256;
  static BYTE rgbRandomData[cbRandomData];
  static ibRandomData = cbRandomData;

  if( ibRandomData > ( cbRandomData - sizeof( unsigned int ) ) )
  {
    // no data left, refill the buffer
    GetRandomData( rgbRandomData, cbRandomData );
    ibRandomData = 0;
  }

  const unsigned int u = *(reinterpret_cast<unsigned int *>(rgbRandomData+ibRandomData));
  ibRandomData += sizeof( unsigned int );
  return u;
}

/* 
 *  RangeRand(len)
 *
 *  Returns a random number in the range 0 to (len-1).
 *  For example, RangeRand(256) returns a value from 0 to 255.
 */
unsigned int
RangeRand(size_t len)
{
   unsigned int      r;
   const unsigned int ceil = UINT_MAX - (UINT_MAX % len) - 1;
   while ((r = MyRand()) > ceil)
      ;
   return(r%len);
}

int
_writecbc(FILE *fp,
          const unsigned char* buffer,
          int length, unsigned char type,
	  const unsigned char *pass, int passlen,
          const unsigned char* salt, int saltlen,
          unsigned char* cbcbuffer)
{
  const unsigned int BS = BlowFish::BLOCKSIZE;
  int numWritten = 0;

  int BlockLength = ((length+(BS-1))/BS)*BS;
  if (BlockLength == 0)
    BlockLength = BS;

  BlowFish *Algorithm = BlowFish::MakeBlowFish(pass, passlen, salt, saltlen);

  // First encrypt and write the length of the buffer
  unsigned char lengthblock[BS];
  // Fill unused bytes of length with random data, to make
  // a dictionary attack harder
  GetRandomData(lengthblock, sizeof(lengthblock));
  // block length overwrites 4 bytes of the above randomness.
  putInt32( lengthblock, length );

  // following new for format 2.0 - lengthblock bytes 4-7 were unused before.
  lengthblock[sizeof(length)] = type;

  xormem(lengthblock, cbcbuffer, BS); // do the CBC thing
  Algorithm->Encrypt(lengthblock, lengthblock);
  memcpy(cbcbuffer, lengthblock, BS); // update CBC for next round

  numWritten = fwrite(lengthblock, 1, BS, fp);

  trashMemory(lengthblock, BS);

  // Now, encrypt and write the buffer
  unsigned char curblock[BS];
  for (int x=0;x<BlockLength;x+=BS)
    {
      if ((length == 0) || ((length%BS != 0) && (length-x<BS)))
        {
          //This is for an uneven last block
          memset(curblock, 0, BS);
          memcpy(curblock, buffer+x, length % BS);
        }
      else
        memcpy(curblock, buffer+x, BS);
      xormem(curblock, cbcbuffer, BS);
      Algorithm->Encrypt(curblock, curblock);
      memcpy(cbcbuffer, curblock, BS);
      numWritten += fwrite(curblock, 1, BS, fp);
    }
  trashMemory(curblock, BS);
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
 * (unless buffer_len is zero)
 *
 * An alternative to STL strings would be to accept a buffer, and allocate a replacement
 * iff the buffer is too small. This is serious ugliness, but should be considered if
 * the new/delete performance hit is too big.
 */
int
_readcbc(FILE *fp,
         unsigned char* &buffer, unsigned int &buffer_len, unsigned char &type,
         const unsigned char *pass, int passlen,
         const unsigned char* salt, int saltlen,
         unsigned char* cbcbuffer)
{
  const unsigned int BS = BlowFish::BLOCKSIZE;
  int numRead = 0;

  unsigned char lengthblock[BS];
  unsigned char lcpy[BS];

  buffer_len = 0;
  numRead = fread(lengthblock, 1, sizeof lengthblock, fp);
  if (numRead != BS)
    return 0;
  memcpy(lcpy, lengthblock, BS);

  BlowFish *Algorithm = BlowFish::MakeBlowFish(pass, passlen, salt, saltlen);

  Algorithm->Decrypt(lengthblock, lengthblock);
  xormem(lengthblock, cbcbuffer, BS);
  memcpy(cbcbuffer, lcpy, BS);

  int length = getInt32( lengthblock );

  // new for 2.0 -- lengthblock[4..7] previously set to zero
  type = lengthblock[sizeof(int)]; // type is first byte after the length

  trashMemory(lengthblock, BS);
  trashMemory(lcpy, BS);


  if (length < 0) {
    delete Algorithm;
    buffer = NULL;
    buffer_len = 0;
    return 0;
  }

  int BlockLength = ((length+(BS-1))/BS)*BS;
  // Following is meant for lengths < BS,
  // but results in a block being read even
  // if length is zero. This is wasteful,
  // but fixing it would break all existing databases.
  if (BlockLength == 0)
    BlockLength = BS;

  buffer_len = length;
  buffer = new unsigned char[BlockLength]; // so we lie a little...


  unsigned char tempcbc[BS];
  numRead += fread(buffer, 1, BlockLength, fp);
  for (int x=0;x<BlockLength;x+=BS)
    {
      memcpy(tempcbc, buffer+x, BS);
      Algorithm->Decrypt(buffer+x, buffer+x);
      xormem(buffer+x, cbcbuffer, BS);
      memcpy(cbcbuffer, tempcbc, BS);
    }
	
  trashMemory(tempcbc, BS);
  delete Algorithm;
  if (length == 0) {
    // delete[] buffer here since caller will see zero length
    delete[] buffer;
  }
  return numRead;
}

/**
 * Returns the current length of a file.
 */
long fileLength( FILE *fp )
{
	long	pos;
	long	len;

	pos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	len	= ftell( fp );
	fseek( fp, pos, SEEK_SET );

	return len;
}

//----------------------------------------------------------------
/* Given a 6 bit binary value, get a base 64 character. */
static char b64table[64] = {'A', 'B', 'C', 'D', 'E', 'F',
                            'G', 'H', 'I', 'J', 'K', 'L',
                            'M', 'N', 'O', 'P', 'Q', 'R',
                            'S', 'T', 'U', 'V', 'W', 'X',
                            'Y', 'Z', 'a', 'b', 'c', 'd',
                            'e', 'f', 'g', 'h', 'i', 'j',
                            'k', 'l', 'm', 'n', 'o', 'p',
                            'q', 'r', 's', 't', 'u', 'v',
                            'w', 'x', 'y', 'z', '0', '1',
                            '2', '3', '4', '5', '6', '7',
                            '8', '9', '+', '/'};

/* Given a base 64 character, return the original 6 bit binary value. 
 * We treat a null in the input as end of string and = as padding 
 * signifying the end of string.  Everything else is ignored.
 */
static char b64revtb[256] = { 
  -3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*0-15*/ 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*16-31*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /*32-47*/
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, /*48-63*/
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /*64-79*/
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /*80-95*/
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /*96-111*/
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /*112-127*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*128-143*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*144-159*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*160-175*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*176-191*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*192-207*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*208-223*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*224-239*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  /*240-255*/
};

/* Accepts a binary buffer with an associated size.
   Returns a base64 encoded, null-terminated string.
 */
unsigned char *base64_encode(unsigned char *input, int len)
{
  unsigned char *output, *p;
  int mod = len % 3;
  int i = 0;
 
  p = output = new unsigned char[((len/3)+(mod?1:0))*4 + 1];
  while(i < len - mod) {
    *p++ = b64table[input[i++] >> 2];
    *p++ = b64table[((input[i-1] << 4) | (input[i++] >> 4)) & 0x3f];
    *p++ = b64table[((input[i-1] << 2) | (input[i]>>6)) & 0x3f];
    *p++ = b64table[input[i++] & 0x3f];
  }
  if(!mod) {
    *p = 0;
    return output;
  }
  *p++ = b64table[input[i++] >> 2];
  *p++ = b64table[((input[i-1] << 4) | (input[i] >> 4)) & 0x3f];
  *p++ = (mod == 1) ? '=' : b64table[(input[i] << 2) & 0x3f];
  *p++ = '=';
  *p = 0;
  return output;
}

static unsigned int raw_base64_decode(unsigned char *in,
                                      unsigned char *out, int *err)
{
  unsigned char buf[3];
  unsigned char pad = 0;
  unsigned int result = 0;
  char x;

  *err = 0;
  while(1) {
  ch1:
    switch(x = b64revtb[*in++]) {
    case -3: /* NULL TERMINATOR */
      return result;
    case -2: /* PADDING CHAR... INVALID HERE */
      *err = 1;
      return result;
    case -1:
      goto ch1;
    default:
      buf[0] = x<<2;
    }
  ch2:
    switch(x = b64revtb[*in++]) {
    case -3: /* NULL TERMINATOR... INVALID HERE */
    case -2: /* PADDING CHAR... INVALID HERE */
      *err = 1;
      return result;
    case -1:
      goto ch2;
    default:
      buf[0] |= (x>>4);
      buf[1] = x<<4;
    }
  ch3:
    switch(x = b64revtb[*in++]) {
    case -3: /* NULL TERMINATOR... INVALID HERE */
      *err = 1;
      return result;
    case -2:
      /* Make sure there's appropriate padding. */
      if(*in != '=') {
	*err = 1;
	return result;
      }
      buf[2] = 0;
      pad = 2;
      result += 1;
      goto assembled;
    case -1:
      goto ch3;
    default:
      buf[1] |= (x>>2);
      buf[2] = x<<6;
    }
  ch4:
    switch(x = b64revtb[*in++]) {
    case -3: /* NULL TERMINATOR... INVALID HERE */
      *err = 1;
      return result;
    case -2:
      pad = 1;
      result += 2;
      /* assert(buf[2] == 0) */
      goto assembled;
    case -1:
      goto ch4;
    default:
      buf[2] |= x;
    }
    result += 3;
  assembled:
    for(x=0;x<3-pad;x++) {
      *out++ = buf[x];
    }
    if(pad) {
      return result;
    }
  }
}

/* If err is non-zero on exit, then there was an incorrect padding
   error.  We allocate enough space for all circumstances, but when
   there is padding, or there are characters outside the character 
   set in the string (which we are supposed to ignore), then we 
   end up allocating too much space.  You can realloc to the 
   correct length if you wish.
 */

unsigned char *base64_decode(unsigned char *buf, unsigned int *len, int *err)
{
  unsigned char *outbuf;

  outbuf = new unsigned char[3*(strlen((const char *)buf)/4+1)];

  *len = raw_base64_decode(buf, outbuf, err);
  return outbuf;
}
