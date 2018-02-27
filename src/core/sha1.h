/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
  void Update(const unsigned char* data, unsigned int len);
  void Final(unsigned char digest[HASHLEN]);

private:
  uint32 state[5];
  uint32 count[2];
  unsigned char buffer[BLOCKSIZE];
};
#endif /* __SHA1_H */
