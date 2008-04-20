/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// PWSrand.h
//-----------------------------------------------------------------------------

#ifndef PWSrand_h
#define PWSrand_h

#include "PwsPlatform.h"
#include "sha256.h"

class PWSrand {
 public:
  static PWSrand *GetInstance();
  static void DeleteInstance();

  void AddEntropy(unsigned char *bytes, unsigned int numBytes);
  //  fill this buffer with random data
  void GetRandomData( void * const buffer, unsigned long length );

  unsigned int RandUInt(); // generate a random uint
  //  generate a random number between 0 and len
  unsigned int RangeRand(size_t len);
 private:
  PWSrand(); // start with some minimal entropy
  ~PWSrand();

  void NextRandBlock();
  static PWSrand *self;
  bool m_IsInternalPRNG;
  unsigned char K[SHA256::HASHLEN];
  unsigned char R[SHA256::HASHLEN];

  char rgbRandomData[SHA256::HASHLEN];
  int ibRandomData;
};
#endif // PWSrand_h
