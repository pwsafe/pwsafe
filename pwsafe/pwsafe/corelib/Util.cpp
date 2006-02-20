/// \file Util.cpp
//-----------------------------------------------------------------------------

#include "sha1.h"
#include "BlowFish.h"
#include "PwsPlatform.h"

#include <stdio.h>

#include "Util.h"


// used by CBC routines...
static void
xormem(unsigned char* mem1, const unsigned char* mem2, int length)
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
  if ( length != 0 ) {
    const int numiter = 30;
    for (int x=0; x<numiter; x++) {
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
  const LPCTSTR pkey = (const LPCTSTR)a_passkey;
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
  for( unsigned int ib = 0; ib < length; ++ib ) {
    pb[ib] = randchar();
  }
  return TRUE;
}

static BOOLEAN __stdcall LoadRandomDataFunction(void * pv, ULONG cb)
{
  //  this is the default function we'll use if loading RtlGenRandom fails
  pfnGetRandomData = MyGetRandomData;

  HMODULE hLib = LoadLibrary(_T("ADVAPI32.DLL"));
  if (hLib) {
    BOOLEAN (APIENTRY *pfnGetRandomDataT)(void*, ULONG);
    pfnGetRandomDataT = (BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib,"SystemFunction036");
    if (pfnGetRandomDataT) {
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

  static const int cbRandomData = 256;
  static BYTE rgbRandomData[cbRandomData];
  static int ibRandomData = cbRandomData;

  if( ibRandomData > ( cbRandomData - sizeof( unsigned int ) ) ) {
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
_writecbc(FILE *fp, const unsigned char* buffer, int length, unsigned char type,
          Fish *Algorithm, unsigned char* cbcbuffer)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  int numWritten = 0;

  // some trickery to avoid new/delete
  unsigned char block1[16];

  unsigned char *curblock = NULL;
  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...

  // First encrypt and write the length of the buffer
  curblock = block1;
  // Fill unused bytes of length with random data, to make
  // a dictionary attack harder
  GetRandomData(curblock, BS);
  // block length overwrites 4 bytes of the above randomness.
  putInt32(curblock, length);

  // following new for format 2.0 - lengthblock bytes 4-7 were unused before.
  curblock[sizeof(length)] = type;

  if (BS == 16) {
    // In this case, we've too many (11) wasted bytes in the length block
    // So we store actual data there:
    // (11 = BlockSize - 4 (length) - 1 (type)
    const int len1 = (length > 11) ? 11 : length;
    memcpy(curblock+5, buffer, len1);
    length -= len1;
    buffer += len1;
  }

  xormem(curblock, cbcbuffer, BS); // do the CBC thing
  Algorithm->Encrypt(curblock, curblock);
  memcpy(cbcbuffer, curblock, BS); // update CBC for next round

  numWritten = fwrite(curblock, 1, BS, fp);

  if (length > 0) {
    unsigned int BlockLength = ((length+(BS-1))/BS)*BS;
    if (BlockLength == 0 && BS == 8)
      BlockLength = BS;

    // Now, encrypt and write the (rest of the) buffer
    for (unsigned int x=0; x<BlockLength; x+=BS) {
      if ((length == 0) || ((length%BS != 0) && (length-x<BS))) {
        //This is for an uneven last block
        GetRandomData(curblock, BS);
        memcpy(curblock, buffer+x, length % BS);
      } else
        memcpy(curblock, buffer+x, BS);
      xormem(curblock, cbcbuffer, BS);
      Algorithm->Encrypt(curblock, curblock);
      memcpy(cbcbuffer, curblock, BS);
      numWritten += fwrite(curblock, 1, BS, fp);
    }
  }
  trashMemory(curblock, BS);
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
 *
 * If TERMINAL_BLOCK is non-NULL, the first block read is tested against it,
 * and -1 is returned if it matches. (used in V3)
 */
int
_readcbc(FILE *fp,
         unsigned char* &buffer, unsigned int &buffer_len, unsigned char &type,
         Fish *Algorithm, unsigned char* cbcbuffer,
         const unsigned char *TERMINAL_BLOCK)
{
  const unsigned int BS = Algorithm->GetBlockSize();
  unsigned int numRead = 0;
  
  // some trickery to avoid new/delete
  unsigned char block1[16];
  unsigned char block2[16];
  unsigned char block3[16];
  unsigned char *lengthblock = NULL;

  ASSERT(BS <= sizeof(block1)); // if needed we can be more sophisticated here...
  lengthblock = block1;

  buffer_len = 0;
  numRead = fread(lengthblock, 1, BS, fp);
  if (numRead != BS) {
    return 0;
  }

  if (TERMINAL_BLOCK != NULL &&
      memcmp(lengthblock, TERMINAL_BLOCK, BS) == 0)
    return -1;

  unsigned char *lcpy = block2;
  memcpy(lcpy, lengthblock, BS);

  Algorithm->Decrypt(lengthblock, lengthblock);
  xormem(lengthblock, cbcbuffer, BS);
  memcpy(cbcbuffer, lcpy, BS);

  int length = getInt32(lengthblock);

  // new for 2.0 -- lengthblock[4..7] previously set to zero
  type = lengthblock[sizeof(int)]; // type is first byte after the length

  if (length < 0) { // sanity check
    TRACE("_readcbc: Read negative length - aborting\n");
    buffer = NULL;
    buffer_len = 0;
    trashMemory(lengthblock, BS);
    return 0;
  }

  buffer_len = length;
  buffer = new unsigned char[(length/BS)*BS +2*BS]; // round upwards
  unsigned char *b = buffer;

  if (BS == 16) {
    // length block contains up to 11 (= 16 - 4 - 1) bytes
    // of data
    const int len1 = (length > 11) ? 11 : length;
    memcpy(b, lengthblock+5, len1);
    length -= len1;
    b += len1;
  }

  unsigned int BlockLength = ((length+(BS-1))/BS)*BS;
  // Following is meant for lengths < BS,
  // but results in a block being read even
  // if length is zero. This is wasteful,
  // but fixing it would break all existing pre-3.0 databases.
  if (BlockLength == 0 && BS == 8)
    BlockLength = BS;

  trashMemory(lengthblock, BS);

  if (length > 0) {
    unsigned char *tempcbc = block3;
    numRead += fread(b, 1, BlockLength, fp);
    for (unsigned int x=0; x<BlockLength; x+=BS) {
      memcpy(tempcbc, b + x, BS);
      Algorithm->Decrypt(b + x, b + x);
      xormem(b + x, cbcbuffer, BS);
      memcpy(cbcbuffer, tempcbc, BS);
    }
  }

  if (buffer_len == 0) {
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
