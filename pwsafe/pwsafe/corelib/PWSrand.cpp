/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include <limits.h>
#include <stdlib.h>
#include <process.h>

#include "PwsPlatform.h"
#include "PWSrand.h"

// See the MSDN documentation for RtlGenRandom. We will try to load it
// and if that fails, use our own random number generator. The function
// call is indirected through a function pointer

static BOOLEAN (APIENTRY *pfnGetRandomData)(void*, ULONG) = NULL;

static bool __stdcall LoadRandomDataFunction();

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
  : ibRandomData(SHA256::HASHLEN)
{
  m_IsInternalPRNG = !LoadRandomDataFunction();

  SHA256 s;
  time_t t = time(NULL);
  int pid = _getpid();
  DWORD ticks = GetTickCount();

  s.Update((const unsigned char *)&t, sizeof(t));
  s.Update((const unsigned char *)&pid, sizeof(pid));
  s.Update((const unsigned char *)&ticks, sizeof(ticks));

  s.Final(K);
}

PWSrand::~PWSrand()
{
}

void PWSrand::AddEntropy(unsigned char *bytes, unsigned int numBytes)
{
  ASSERT(bytes != NULL);

  SHA256 s;

  s.Update(K, sizeof(K));
  s.Update(bytes, numBytes);
  s.Final(K);
}

void PWSrand::NextRandBlock()
{
  SHA256 s;
  s.Update(K, sizeof(K));
  s.Final(R);
  unsigned int *Kp = (unsigned int *)K;
  unsigned int *Rp = (unsigned int *)R;
  const int N = SHA256::HASHLEN/sizeof(unsigned int);
  
  Kp[0]++;

  for (int i = 0; i < N; i++)
    Kp[i] += Rp[i];
}

void PWSrand::GetRandomData( void * const buffer, unsigned long length )
{
  if (!m_IsInternalPRNG) {
    ASSERT(pfnGetRandomData != NULL);
    (void)(*pfnGetRandomData)(buffer, length);
  } else {
    unsigned char *pb = (unsigned char *)buffer;
    while (length > SHA256::HASHLEN) {
      NextRandBlock();
      for (int j = 0; j < SHA256::HASHLEN; j++)
        pb[j] = R[j];
      length -= SHA256::HASHLEN;
      pb += SHA256::HASHLEN;
    }
    ASSERT(length <= SHA256::HASHLEN);
    if (length > 0) {
      unsigned long i = 0;
      NextRandBlock();
      while (i < length) {
        pb[i] = R[i];
        i++;
      }
    }
  }
}

// generate random numbers from a buffer filled in by GetRandomData()
unsigned int PWSrand::RandUInt()
{
  // we don't want to keep filling the random buffer for each number we
  // want, so fill the buffer with random data and use it up

  if( ibRandomData > ( SHA256::HASHLEN - sizeof( unsigned int ) ) ) {
    // no data left, refill the buffer
    GetRandomData(rgbRandomData, SHA256::HASHLEN);
    ibRandomData = 0;
  }

  const unsigned int u = 
    *(reinterpret_cast<unsigned int *>(rgbRandomData + ibRandomData));
  ibRandomData += sizeof(unsigned int);
  return u;
}

/* 
 *  RangeRand(len)
 *
 *  Returns a random number in the range 0 to (len-1).
 *  For example, RangeRand(256) returns a value from 0 to 255.
 */
unsigned int PWSrand::RangeRand(unsigned int len)
{
  unsigned int      r;
  const unsigned int ceil = UINT_MAX - (UINT_MAX % len) - 1;
  while ((r = RandUInt()) > ceil)
    ;
  return(r%len);
}



static bool __stdcall LoadRandomDataFunction()
{
  HMODULE hLib = LoadLibrary(_T("ADVAPI32.DLL"));
  BOOLEAN (APIENTRY *pfnGetRandomDataT)(void*, ULONG) = NULL;
  if (hLib != NULL) {
    pfnGetRandomDataT = (BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib,"SystemFunction036");
    if (pfnGetRandomDataT) {
      pfnGetRandomData = pfnGetRandomDataT;
    }
  }
  return (hLib != NULL && pfnGetRandomDataT != NULL);
}
 
