/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// sha256.h
// SHA256 for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef _SHA256_H_
#define _SHA256_H_
#include "typedefs.h"
class SHA256
{
public:
  enum {HASHLEN = 32};
  SHA256();
  ~SHA256();
  void Update(const unsigned char *in, unsigned long inlen);
  void Final(unsigned char digest[HASHLEN]);
private:
  ulong64 length;
  ulong32 state[8], curlen;
  unsigned char buf[64];
};

#endif /* _SHA256_H_ */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

