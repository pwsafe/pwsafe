/*
* Copyright (c) 2013-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 01-Oct-2023
*/
// HOTP.h
//-----------------------------------------------------------------------------
#ifndef __HOTP_H
#define __HOTP_H

#include <vector>
#include <cmath>

#include "../Util.h"
#include "hmac.h"

template<class HMACGenerator>
class HOTP
{
protected:
  using KeyBuffer = std::vector<unsigned char>;
  using Digest = std::vector<unsigned char>;
public:
  HOTP(const unsigned char* key, unsigned long keylen) : keyBuf(key, key + keylen) {}
  virtual ~HOTP()
  {
    trashMemory(&keyBuf[0], keyBuf.size());
  }
  uint32_t Generate(uint64_t counter, int numDigits)
  {
    HMACGenerator hmacGenerator(&keyBuf[0], static_cast<unsigned long>(keyBuf.size()));
#ifdef PWS_LITTLE_ENDIAN
    byteswap(counter);
#endif
    hmacGenerator.Update(reinterpret_cast<unsigned char*>(&counter), sizeof(counter));
    Digest digest(HMACGenerator::HASH_LENGTH);
    hmacGenerator.Final(&digest[0]);
    int index = digest[HMACGenerator::HASH_LENGTH - 1] & 0x0F;
    uint32_t code = *reinterpret_cast<uint32_t*>(&digest[0] + index);
#ifdef PWS_LITTLE_ENDIAN
    byteswap(code);
#endif
    uint32_t digits_divisor = static_cast<uint32_t>(std::pow(10, numDigits));
    code &= 0x7FFFFFFF;
    code %= digits_divisor;
    return code;
  }
private:
  KeyBuffer keyBuf;
};

using HOTP_SHA1 = HOTP<HMAC_SHA1>;

#endif /* __HOTP_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
