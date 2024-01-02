/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 18-Oct-2023
*/
// TotpCore.h
//-----------------------------------------------------------------------------

#ifndef _TOTPCORE_H
#define _TOTPCORE_H

#include <string>
#include <stdint.h>

#include "StringX.h"

/**
 * Password Safe Core library two factor authentication related structs and
 * functionality connecting PwSafe Core users (i.e., UI/CLI) with TOTP internals.
 */

class CItemData;

namespace PWSTotp {

  enum TOTP_Result {
    Success = 0,
    InvalidTotpConfiguration = 2,
    TotpKeyNotFound = 3,
    InvalidCharactersInKey = 4,
    KeyBase32DecodingFailure = 5,
  };

  const uint32_t TOTP_INVALID_AUTH_CODE = 0;

  const uint8_t TOTP_DEFAULT_AUTH_CODE_LENGTH = 6;
  const uint8_t TOTP_DEFAULT_TIME_STEP_SECONDS = 30;

  // Mask and access information from DB TOTP Config (see formatV3.txt).
  const uint8_t TOTP_CONFIG_ALGORITHM_MASK = 0x03;
  const uint8_t TOTP_CONFIG_ALGORITHM_HMAC_SHA1 = 0x00;
  const uint8_t TOTP_CONFIG_ALGORITHM_DEFAULT = TOTP_CONFIG_ALGORITHM_HMAC_SHA1;

  std::wstring GetTotpErrorString(TOTP_Result r);
  void GetCurrentTotpIntervalInformation(uint8_t time_step_seconds, time_t start_time, time_t& time_now, double& ratio_expired);
  TOTP_Result GetCurrentTotpIntervalInformation(const CItemData& data, time_t& time_now, double& ratio);
  TOTP_Result GetNextTotpAuthCode(const CItemData& data, uint32_t& totpCode, time_t* pBasisTimeNow = nullptr, double* pRatioExpired = nullptr);
  StringX TotpCodeToString(const CItemData& data, uint32_t totp_auth_code);
  TOTP_Result GetNextTotpAuthCodeString(const CItemData& data, StringX& totpAuthCodeStr, time_t* pBasisTimeNow = nullptr, double* pRatioExpired = nullptr);
  TOTP_Result ValidateTotpConfiguration(const CItemData& data, time_t* pBasisTimeNow = nullptr, double* pRatioExpired = nullptr);
}
#endif
