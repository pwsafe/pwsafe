#include <limits.h>
#include <stdlib.h>

#include "PwsPlatform.h"
#include "PWSrand.h"

PWSrand *PWSrand::self = NULL;

PWSrand *PWSrand::GetInstance()
{
  if (self == NULL) {
    self = new PWSrand;
  }
  return self;
}

void PWSrand::DeleteInstance()
{
  delete self;
  self = NULL;
}

PWSrand::PWSrand()
{
  srand((unsigned)time(NULL)); // XXX temporary...

}

PWSrand::~PWSrand()
{
}

void PWSrand::AddEntropy(unsigned char *bytes, unsigned int numBytes)
{
}

// See the MSDN documentation for RtlGenRandom. We will try to load it
// and if that fails, use the simple random number generator. The function
// call is indirected through a function pointer, which initially points
// to a function that tries to load RtlGenRandom

static BOOLEAN __stdcall LoadRandomDataFunction(void *, ULONG);
static BOOLEAN __stdcall MyGetRandomData( PVOID buffer, ULONG length );
static BOOLEAN (APIENTRY *pfnGetRandomData)(void*, ULONG) = LoadRandomDataFunction;

void PWSrand::GetRandomData( void * const buffer, unsigned long length )
{
  (void)(*pfnGetRandomData)(buffer, length);
}

// generate random numbers from a buffer filled in by GetRandomData()
// NOTE: not threadsafe. the static data in the function can't
// be used by multiple threads. hack it with __threadlocal,
// make it an object or something like that if you need multi-threading
static unsigned int MyRand()
{
  // we don't want to keep filling the random buffer for each number we
  // want, so fill the buffer with random data and use it up

  static const int cbRandomData = 256;
  static BYTE rgbRandomData[cbRandomData];
  static int ibRandomData = cbRandomData;

  if( ibRandomData > ( cbRandomData - sizeof( unsigned int ) ) ) {
    // no data left, refill the buffer
    PWSrand::GetInstance()->GetRandomData( rgbRandomData, cbRandomData );
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
unsigned int PWSrand::RangeRand(size_t len)
{
  unsigned int      r;
  const unsigned int ceil = UINT_MAX - (UINT_MAX % len) - 1;
  while ((r = MyRand()) > ceil)
    ;
  return(r%len);
}

static unsigned char
randchar()
{
  int	r;
  while ((r = rand()) % 257 == 256)
    ; // 257?!?
  return (unsigned char)r;
}

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
 
