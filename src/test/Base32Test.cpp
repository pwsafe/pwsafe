/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// TOTPTest.cpp: Unit test for TOTP.

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include <algorithm>
#include <vector>

#include "core/crypto/external/Chromium/base32.h"
#include "core/crypto/totp.h"
#include "gtest/gtest.h"

using namespace std;

// The odd string splits below are an effort to mitigate
// CI/CD false positives around secrets detection.
vector<tuple<const char*, const char*>> base32_test_cases{
  {"", ""},
  {"T", "KQ======"},
  {"Th", "KRUA===="},
  {"The", "KRUGK==="},
  {"The ", "KRUGKIA="},
  {"The q", "KRUGKIDR"},
  {"The qu", "KRUGKID" "ROU======"},
  {"The qui", "KRUGKID" "ROVUQ===="},
  {"The quic", "KRUGKID" "ROVUWG==="},
  {"The quick", "KRUGKID" "ROVUWG2Y="},
  {"The quick ", "KRUGKID" "ROVUWG2ZA"},
  {"The quick b", "KRUGKIDROVUWG2" "ZAMI======"},
  {"The quick br", "KRUGKIDROVUWG2" "ZAMJZA===="},
  {"The quick bro", "KRUGKIDROVUWG2Z" "AMJZG6==="},
  {"The quick brow", "KRUGKIDROVUWG2Z" "AMJZG65Y="},
  {"The quick brown", "KRUGKIDROVUWG2Z" "AMJZG653O"},
  {"The quick brown ", "KRUGKIDROVUWG2Z" "AMJZG653OEA======"},
  {"The quick brown f", "KRUGKIDROVUWG2Z" "AMJZG653OEBTA===="},
  {"The quick brown fo", "KRUGKIDROVUWG2Z" "AMJZG653OEBTG6==="},
  {"The quick brown fox", "KRUGKIDROVUWG2Z" "AMJZG653OEBTG66A="},
};

TEST(Base32Test, base32_encode_test)
{
  int i = 0;
  for (auto tc : base32_test_cases) {
    auto decoded_string = std::string(std::get<0>(tc));
    auto encoded_string = std::string(std::get<1>(tc));

    // Chromium base32.c/base32.h does not support RFC3548 base32 padding.
    // Remove padding, give/expect unpadding base32 string.
    while (!encoded_string.empty() && *encoded_string.rbegin() == '=')
      encoded_string.pop_back();

    // Expected encoded string length plus null terminator.
    string encoded_string_actual;
    encoded_string_actual.resize(encoded_string.size() + 1);

    bool result = base32_encode(
      &encoded_string_actual[0],
      static_cast<int>(encoded_string_actual.size()),
      decoded_string.c_str(),
      static_cast<int>(decoded_string.size() * 8),
      0
    );

    encoded_string_actual.resize(strlen(encoded_string_actual.c_str()));

    EXPECT_TRUE(result)
      << "Test vector " << i << ": base32_encode must be successful.";
    EXPECT_TRUE(encoded_string_actual == encoded_string)
      << "base32_encode: Test vector " << i << ": expected and actual encoded strings must match.";
    i++;
  }
}

TEST(Base32Test, base32_decode_test)
{
  int i = 0;
  for (auto tc : base32_test_cases) {
    auto decoded_string = std::string(std::get<0>(tc));
    auto encoded_string = std::string(std::get<1>(tc));

    // Chromium base32.c/base32.h does not support RFC3548 base32 padding.
    // Remove padding, give/expect unpadding base32 string.
    while (!encoded_string.empty() && *encoded_string.rbegin() == '=')
      encoded_string.pop_back();

    string decoded_string_actual;
    decoded_string_actual.resize(decoded_string.size());

    bool result = base32_decode(
      reinterpret_cast<uint8_t*>(&decoded_string_actual[0]),
      static_cast<int>(decoded_string_actual.size()) * 8,
      encoded_string.c_str(),
      0
    );

    EXPECT_TRUE(result)
      << "Test vector " << i << ": base32_decode must be successful.";
    EXPECT_TRUE(decoded_string_actual == decoded_string)
      << "base32_decode: Test vector " << i << ": expected and actual encoded strings must match.";
    i++;
  }
}
