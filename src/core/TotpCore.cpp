/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 18-Oct-2023
*/
// TotpCore.cpp
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

namespace PWSTotp {

  std::wstring GetTotpErrorString(TOTP_Result r)
  {
    switch (r) {
    case Success: return L"The TOTP operation was successful";
    case InvalidTotpConfiguration: return L"The TOTP configuration is invalid.";
    case TotpKeyNotFound: return L"The TOTP key was not found.";
    case ConvertKeyToUtf8Failure: return L"Apparent invalid TOTP key. Cannot convert the TOTP key to UTF8.";
    case KeyBase32DecodingFailure: return L"Apparent invalid TOTP key. The TOTP key base32 decoding failed.";
    default: return L"An unknown TOTP error occurred.";
    }
  }

  TOTP_Result GetNextTotpAuthCode(const CItemData& data, uint32_t& totpCode, time_t* pBasisTimeNow)
  {
    totpCode = TOTP_INVALID_AUTH_CODE;

    uint8_t totp_algo = data.GetTotpAlgorithmAsByte();
    if (totp_algo != TOTP_CONFIG_ALGORITHM_HMAC_SHA1)
      return InvalidTotpConfiguration;

    time_t totp_start_time = data.GetTotpStartTimeAsTimeT();
    uint8_t totp_code_digit_length = data.GetTotpLengthAsByte();
    uint8_t totp_time_step_seconds = data.GetTotpTimeStepSecondsAsByte();

    auto totp_key = data.GetTwoFactorKey();
    if (totp_key.empty())
      return TotpKeyNotFound;

    CUTF8Conv conv;
    const unsigned char* utf8 = nullptr;
    size_t utf8Len = 0;
    if (!conv.ToUTF8(totp_key, utf8, utf8Len))
      return ConvertKeyToUtf8Failure;

    RFC4648_Base32Decoder base32_key(reinterpret_cast<const char*>(utf8));
    if (!base32_key.is_decoding_successful())
      return KeyBase32DecodingFailure;

    TOTP_SHA1 totp(base32_key, totp_code_digit_length, totp_time_step_seconds, totp_start_time);
    time_t time_now = time(NULL);
    if (pBasisTimeNow)
      *pBasisTimeNow = time_now;
    totpCode = totp.Generate(time_now);
    return Success;
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

  TOTP_Result GetNextTotpAuthCodeString(const CItemData& data, std::string& totpAuthCodeStr, time_t* pBasisTimeNow)
  {
    totpAuthCodeStr.clear();
    uint32_t totp_auth_code;
    TOTP_Result result = GetNextTotpAuthCode(data, totp_auth_code, pBasisTimeNow);
    if (result == Success)
      totpAuthCodeStr = TotpCodeToString(data, totp_auth_code);
    return result;
  }

}
