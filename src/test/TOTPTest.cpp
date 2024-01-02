/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 01-Oct-2023
*/
// TOTPTest.cpp: Unit test for TOTP.

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/crypto/totp.h"
#include "gtest/gtest.h"

using namespace std;

// The strings below are split in an effort to mitigate CI/CD secret detection false positives.
vector<tuple<const char*, const char*, int, time_t, time_t, time_t, uint32_t>> totp_test_cases {
  {"Goo" "gle",    "y7p2mq33tdqbddp3" "3cbvgquclcihdq27",   6, 30, 0, 1696145833, 643493},
  {"Goo" "gle",    "y7p2mq33tdqbddp3" "3cbvgquclcihdq27",   6, 30, 0, 1696145913, 802888},
  {"Twit" "ter",   "ODAVH3CH" "B2ZBAVON",                   6, 30, 0, 1696146173,  77912},
  {"Twit" "ter",   "ODAVH3CH" "B2ZBAVON",                   6, 30, 0, 1696146302, 988442},
  {"Face" "book",  "YRUTW6JLVKRXEC7Z" "A7QMPXCGBSOO6HHT",   6, 30, 0, 1696145984, 174075},
  {"Face" "book",  "YRUTW6JLVKRXEC7Z" "A7QMPXCGBSOO6HHT",   6, 30, 0, 1696146044, 597507},
};

TEST(TOTPTest, totp_basic_test)
{
  int i = 0;
  for (auto& tc : totp_test_cases) {
    auto verifier_name = std::string(std::get<0>(tc));
    auto totp_key_base32 = std::string(std::get<1>(tc));
    auto totp_digits = std::get<2>(tc);
    auto totp_interval = std::get<3>(tc);
    auto totp_start_time = std::get<4>(tc);
    auto totp_time_now = std::get<5>(tc);
    auto totp_code_expected = std::get<6>(tc);

    RFC4648_Base32Decoder base32_key(totp_key_base32.c_str());
    EXPECT_TRUE(base32_key.is_decoding_successful())
      << "totp_test: Test vector " << i << ": The base32 key must be decoded.";
    TOTP_SHA1 totp(base32_key, totp_digits, totp_interval, totp_start_time);

    uint32_t totp_code_actual = totp.Generate(totp_time_now);
    EXPECT_TRUE(totp_code_actual == totp_code_expected)
      << "totp_test: Test vector " << i << ": The TOTP code must match.";
    i++;
  }
}

TEST(TOTPTest, key_decoder_failure)
{
  std::string totp_key_base32_invalid("y7p2mq33tdqbddp$3cbvgquclcihdq27");
  RFC4648_Base32Decoder base32_key(totp_key_base32_invalid.c_str());
  EXPECT_TRUE(!base32_key.is_decoding_successful())
    << "totp_test_negative: An invalid base32 key should not decode.";
  EXPECT_TRUE(base32_key.get_size() == 0)
    << "totp_test_negative: An invalid base32 key should yield a zero byte key.";
}

TEST(TOTPTest, key_decoder_copy_move_cleanup)
{
  const std::string valid_key1("YRUTW6JLVKRXEC7ZA7QMPXCGBSOO6HHT");
  const std::string valid_key2("y7p2mq33tdqbddp33cbvgquclcihdq27");

  RFC4648_Base32Decoder dec(valid_key1.c_str());
  EXPECT_TRUE(dec.is_decoding_successful());
  EXPECT_TRUE(dec.get_size() != 0);

  RFC4648_Base32Decoder dec2(dec);
  EXPECT_TRUE(dec.is_decoding_successful());
  EXPECT_TRUE(dec2.is_decoding_successful());
  EXPECT_TRUE(dec.get_bytes() == dec2.get_bytes());
  EXPECT_TRUE(dec.get_size() != 0);
  EXPECT_TRUE(dec2.get_size() != 0);

  const unsigned char* dec_ptr = dec.get_ptr();
  RFC4648_Base32Decoder dec3(std::move(dec));
  EXPECT_TRUE(dec.get_size() == 0);
  EXPECT_TRUE(dec.get_ptr() == nullptr);
  EXPECT_TRUE(dec_ptr != dec.get_ptr());
  EXPECT_TRUE(dec_ptr == dec3.get_ptr());
  EXPECT_TRUE(dec3.is_decoding_successful());
  EXPECT_TRUE(!dec.is_decoding_successful());
  EXPECT_TRUE(dec3.get_bytes() == dec2.get_bytes());
  EXPECT_TRUE(dec3.get_size() != 0);
  EXPECT_TRUE(dec2.get_size() == dec2.get_size());

  const unsigned char* dec3_ptr = dec3.get_ptr();
  dec3 = dec3;
  EXPECT_TRUE(dec3_ptr == dec3.get_ptr());
  dec3 = std::move(dec3);
  EXPECT_TRUE(dec3_ptr == dec3.get_ptr());

  RFC4648_Base32Decoder dec4(valid_key2.c_str());
  EXPECT_TRUE(dec4.is_decoding_successful());
  EXPECT_TRUE(dec4.get_size() != 0);
  EXPECT_TRUE(dec3.get_bytes() != dec4.get_bytes());
  dec3 = dec4;
  EXPECT_TRUE(dec3.is_decoding_successful());
  EXPECT_TRUE(dec3.get_bytes() == dec4.get_bytes());
  EXPECT_TRUE(dec3.get_bytes() != dec2.get_bytes());

  const unsigned char* dec2_ptr = dec2.get_ptr();
  dec3 = std::move(dec2);
  EXPECT_TRUE(dec2.get_size() == 0);
  EXPECT_TRUE(dec2.get_ptr() == nullptr);
  EXPECT_TRUE(!dec2.is_decoding_successful());
  EXPECT_TRUE(dec2.get_ptr() != dec2_ptr);
  EXPECT_TRUE(dec3.get_ptr() == dec2_ptr);
  EXPECT_TRUE(dec3.is_decoding_successful());
  EXPECT_TRUE(dec3.get_bytes() != dec2.get_bytes());
  EXPECT_TRUE(dec3.get_bytes() != dec4.get_bytes());
}

