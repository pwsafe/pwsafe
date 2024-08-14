/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __SHA1_H
#define __SHA1_H
#include "os/typedefs.h"

class SHA1
{
public:
  static const unsigned int HASHLEN = 20;
  static const unsigned int BLOCKSIZE = 64;
  SHA1();
  ~SHA1();
  void Update(const unsigned char* data, uint64 len);
  void Final(unsigned char digest[HASHLEN]);

private:
  uint32 state[5];
  uint64 count;  // Use a single 64-bit value instead of two 32-bit values
  unsigned char buffer[BLOCKSIZE];
};
#endif /* __SHA1_H */
