/*
* Copyright (c) 2013-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AES.h
// C++ wrapper of AES for PasswordSafe, based on LibTomCrypt by
// Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
//-----------------------------------------------------------------------------
#ifndef __AES_H
#define __AES_H

#include "Fish.h"
#include "os/typedefs.h"

struct rijndael_key {
   ulong32 eK[60], dK[60];
   int Nr;
};

class AES : public Fish
{
public:
  static const unsigned int BLOCKSIZE = 16;
  AES(const unsigned char* key, int keylen);
  ~AES();
  void Encrypt(const unsigned char *in, unsigned char *out) const;
  void Decrypt(const unsigned char *in, unsigned char *out) const;
  unsigned int GetBlockSize() const {return BLOCKSIZE;}

private:
  rijndael_key key_schedule;
};
#endif /* __AES_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
