/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
 * functionality connecting PwSafe Core users (i.e., UI/CLI) with TOTP internals.
 */

#include <string>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "core.h"
#include "core/crypto/totp.h"
#include "UTF8Conv.h"
#include "ItemData.h"
#include "TotpCore.h"

namespace PWSTotp {

  std::wstring GetTotpErrorString(TOTP_Result r)
  {
    std::wstring error_message;

    switch (r) {
    case Success:
      LoadAString(error_message, IDSC_TOTP_ERROR_SUCCESS);
      break;
    case InvalidTotpConfiguration:
      LoadAString(error_message, IDSC_TOTP_ERROR_INVALID_CONFIG);
      break;
    case TotpKeyNotFound:
      LoadAString(error_message, IDSC_TOTP_ERROR_KEY_NOT_FOUND);
      break;
    case InvalidCharactersInKey:
      LoadAString(error_message, IDSC_TOTP_ERROR_KEY_INVALID_CHARS);
      break;
    case KeyBase32DecodingFailure:
      LoadAString(error_message, IDSC_TOTP_ERROR_BASE32_DECODE_FAILURE);
      break;
    default:
      LoadAString(error_message, IDSC_TOTP_ERROR_UNKNOWN);
      break;
    }

    if (error_message.empty()) {
      std::wstringstream os;
      os << L"A TOTP error occurred but the error message cannot be retrieved (" << static_cast<int>(r) << L")";
      error_message = os.str();
    }

    return error_message;
  }

  void GetCurrentTotpIntervalInformation(uint8_t time_step_seconds, time_t start_time, time_t& time_now, double& ratio_expired)
  {
    const auto one_second_msecs = std::chrono::milliseconds::period::den;
    const auto interval_msecs = time_step_seconds * one_second_msecs;
    const auto start_time_msecs = start_time * one_second_msecs;

    auto chrono_time_now = std::chrono::system_clock::now().time_since_epoch();
    time_now = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(chrono_time_now).count());
    const auto time_now_msecs = std::chrono::duration_cast<std::chrono::milliseconds>(chrono_time_now);
    const auto totp_elapsed_msecs = time_now_msecs.count() - start_time_msecs;
    ratio_expired = (totp_elapsed_msecs % interval_msecs) / (double)(interval_msecs);
  }

  TOTP_Result GetCurrentTotpIntervalInformation(const CItemData& data, time_t& time_now, double& ratio)
  {
    uint8_t totp_algo = data.GetTotpAlgorithmAsByte();
    if (totp_algo != TOTP_CONFIG_ALGORITHM_HMAC_SHA1)
      return InvalidTotpConfiguration;
    time_t totp_start_time = data.GetTotpStartTimeAsTimeT();
    uint8_t totp_time_step_seconds = data.GetTotpTimeStepSecondsAsByte();
    GetCurrentTotpIntervalInformation(totp_time_step_seconds, totp_start_time, time_now, ratio);
    return Success;
  }

  static bool validBase32chars(const StringX &totp_key)
  {
    const StringX set = L"abcdefghijklmonpqrstuvwxyzABCDEFGHIJKLOMNOPQRSTUVWXYZ234567 -";

    return std::all_of(totp_key.begin(), totp_key.end(), [&set](auto c) {
      return set.find(c) != std::string::npos;
      });

  }

  TOTP_Result GetNextTotpAuthCode(const CItemData& data, uint32_t& totpCode, time_t* pBasisTimeNow, double* pRatioExpired)
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
    if (!validBase32chars(totp_key) || !conv.ToUTF8(totp_key, utf8, utf8Len))
      return InvalidCharactersInKey;

    RFC4648_Base32Decoder base32_key(reinterpret_cast<const char*>(utf8));
    if (!base32_key.is_decoding_successful())
      return KeyBase32DecodingFailure;

    TOTP_SHA1 totp(base32_key, totp_code_digit_length, totp_time_step_seconds, totp_start_time);
    time_t time_now;
    double ratio_expired;
    GetCurrentTotpIntervalInformation(totp_time_step_seconds, totp_start_time, time_now, ratio_expired);
    if (pBasisTimeNow)
      *pBasisTimeNow = time_now;
    if (pRatioExpired)
      *pRatioExpired = ratio_expired;
    totpCode = totp.Generate(time_now);
    return Success;
  }

  StringX TotpCodeToString(const CItemData& data, uint32_t totp_auth_code)
  {
    uint8_t totp_code_digit_length = data.GetTotpLengthAsByte();
    StringX result;
    Format(result, L"%0*d", totp_code_digit_length, totp_auth_code);
    return result;
  }

  TOTP_Result GetNextTotpAuthCodeString(const CItemData& data, StringX& totpAuthCodeStr, time_t* pBasisTimeNow, double* pRatioExpired)
  {
    totpAuthCodeStr.clear();
    uint32_t totp_auth_code;
    TOTP_Result result = GetNextTotpAuthCode(data, totp_auth_code, pBasisTimeNow, pRatioExpired);
    if (result == Success)
      totpAuthCodeStr = TotpCodeToString(data, totp_auth_code);
    return result;
  }

  TOTP_Result ValidateTotpConfiguration(const CItemData& data, time_t* pBasisTimeNow, double* pRatioExpired)
  {
    uint32_t totp_auth_code;
    TOTP_Result result = GetNextTotpAuthCode(data, totp_auth_code, pBasisTimeNow, pRatioExpired);
    totp_auth_code = 0xAAAAAAAA;
    return result;
  }
}
