/*
* Copyright (c) 2013-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 01-Oct-2023
*/
// totp.h
//-----------------------------------------------------------------------------
#ifndef __TOTP_H
#define __TOTP_H

#include "RFC4648_Base32Decoder.h"
#include "hotp.h"

template<class HMACGenerator>
class TOTP : public HOTP<HMACGenerator>
{
public:
  using Base = HOTP<HMACGenerator>;
public:
  TOTP(const unsigned char* key, unsigned long keylen, int numDigits = 6, time_t interval = 30, time_t stepCountStartTime = 0)
    :
    Base(key, keylen),
    numDigits(numDigits),
    intervalX(interval),
    startTimeT0(stepCountStartTime) {}
  TOTP(const RFC4648_Base32Decoder& base32_key, int numDigits = 6, time_t interval = 30, time_t stepCountStartTime = 0)
    :
    TOTP(base32_key.get_ptr(), static_cast<int>(base32_key.get_size()), numDigits, interval, stepCountStartTime) {}
  virtual ~TOTP() {}
  uint32_t Generate(time_t timeNow)
  {
    uint64_t step = (timeNow - startTimeT0) / intervalX;
    return Base::Generate(step, numDigits);
  }
private:
  int numDigits;
  time_t intervalX;
  time_t startTimeT0;
};

using TOTP_SHA1 = TOTP<HMAC_SHA1>;
using TOTP_SHA256 = TOTP<HMAC_SHA256>;

#endif /* __TOTP_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
