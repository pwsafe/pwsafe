/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include <limits.h>
#include "os/rand.h"

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
  : ibRandomData(SHA256::HASHLEN)
{
  m_IsInternalPRNG = !pws_os::InitRandomDataFunction();

  SHA256 s;
  unsigned slen = 0;
  unsigned char *p;

  pws_os::GetRandomSeed(NULL, slen);
  p = new unsigned char[slen];
  pws_os::GetRandomSeed(p, slen);
  s.Update(p, slen);
  delete[] p;
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
    bool status;
    status = pws_os::GetRandomData(buffer, length);
    ASSERT(status);
  }

  // If we have an external random source, we'll
  // xor it in with ours. This helps protect against
  // poor or subverted external PRNGs.
  // Otherwise, we'll rely on our lonesome.

  unsigned char *pb = (unsigned char *)buffer;
  while (length > SHA256::HASHLEN) {
    NextRandBlock();
    for (int j = 0; j < SHA256::HASHLEN; j++)
      pb[j] = (m_IsInternalPRNG) ? R[j] : pb[j] ^ R[j];
    length -= SHA256::HASHLEN;
    pb += SHA256::HASHLEN;
  }

  ASSERT(length <= SHA256::HASHLEN);
  if (length > 0) {
    unsigned long i = 0;
    NextRandBlock();
    while (i < length) {
      pb[i] = (m_IsInternalPRNG) ? R[i] : pb[i] ^ R[i];
      i++;
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
