/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 18-Oct-2023
*/
// TotpCore.h
//-----------------------------------------------------------------------------

/**
 * Password Safe Core library two factor authentication related structs and
 * funtionality connecting PwSafe Core users (i.e., UI/CLI) with TOTP internals.
 */

#include <string>
#include <sstream>
#include "core/crypto/totp.h"
#include "UTF8Conv.h"
#include "ItemData.h"
#include "TotpCore.h"

uint32_t GetNextTotpAuthCode(const CItemData& data, time_t* pBasisTimeNow)
{
  uint8_t totp_algo = data.GetTotpAlgorithmAsByte();
  if (totp_algo != TOTP_CONFIG_ALGORITHM_HMAC_SHA1)
    return TOTP_INVALID_AUTH_CODE;

  time_t totp_start_time = data.GetTotpStartTimeAsTimeT();
  uint8_t totp_code_digit_length = data.GetTotpLengthAsByte();
  uint8_t totp_time_step_seconds = data.GetTotpTimeStepSecondsAsByte();

  auto totp_key = data.GetTwoFactorKey();
  if (totp_key.empty())
    return TOTP_INVALID_AUTH_CODE;
  CUTF8Conv conv;
  const unsigned char* utf8 = nullptr;
  size_t utf8Len = 0;
  if (!conv.ToUTF8(totp_key, utf8, utf8Len))
    return TOTP_INVALID_AUTH_CODE;

  RFC4648_Base32Decoder base32_key(reinterpret_cast<const char*>(utf8));
  if (!base32_key.is_decoding_successful())
    return TOTP_INVALID_AUTH_CODE;

  TOTP_SHA1 totp(base32_key, totp_code_digit_length, totp_time_step_seconds, totp_start_time);
  time_t time_now = time(NULL);
  if (pBasisTimeNow)
    *pBasisTimeNow = time_now;
  return totp.Generate(time_now);
}

std::string TotpCodeToString(const CItemData& data, uint32_t totp_auth_code)
{
  std::stringstream os;
  os << totp_auth_code;
  std::string result = os.str();
  uint8_t totp_code_digit_length = data.GetTotpLengthAsByte();
  while (result.size() < totp_code_digit_length)
    result.insert(result.begin(), '0');
  return result;
}

std::string GetNextTotpAuthCodeString(const CItemData& data, time_t* pBasisTimeNow)
{
  uint32_t totp_auth_code = GetNextTotpAuthCode(data, pBasisTimeNow);
  if (totp_auth_code == TOTP_INVALID_AUTH_CODE)
    return "";
  return TotpCodeToString(data, totp_auth_code);
}
