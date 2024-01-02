/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// HMAC_SHA1Test.cpp: Unit test for HMAC implementation with SHA1
// Test vectors from RFC4231.
// RFC4321 does not specify SHA1 expected results so the expected
// results used in this test were obtained using a non-pwsafe
// HMAC-SHA1 implementation on both Windows/Linux. That effort
// also validated the well-known HMAC-SHA256 expected test results
// from RFC4321 (and used by pwsafe) toward ensuring correctness
// of the derivation effort.

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include <string>
#include <vector>
#include <tuple>

#include "TestCommon.h"

#include "core/crypto/hmac.h"
#include "core/crypto/sha1.h"
#include "gtest/gtest.h"

using namespace std;

// RFC2202 HMAC-SHA1 Test Vectors:
vector<
  tuple<
  int,          // RFC2202 Test Case #
  byte_vector,  // Key.
  byte_vector,  // Data.
  byte_vector   // Digest.
  >
> RFC2202_HMAC_SHA1_TestVectors = {
    {
      1,
      byte_vector(20, 0x0b),
      byte_vector_from_string("Hi There"),
      {0xb6, 0x17, 0x31, 0x86, 0x55, 0x05, 0x72, 0x64,
       0xe2, 0x8b, 0xc0, 0xb6, 0xfb, 0x37, 0x8c, 0x8e,
       0xf1, 0x46, 0xbe, 0x00}
    },
    {
      2,
      byte_vector_from_string("Jefe"),
      byte_vector_from_string("what do ya want for nothing?"),
      {0xef, 0xfc, 0xdf, 0x6a, 0xe5, 0xeb, 0x2f, 0xa2,
       0xd2, 0x74, 0x16, 0xd5, 0xf1, 0x84, 0xdf, 0x9c,
       0x25, 0x9a, 0x7c, 0x79}
    },
    {
      3,
      byte_vector(20, 0xaa),
      byte_vector(50, 0xdd),
      {0x12, 0x5d, 0x73, 0x42, 0xb9, 0xac, 0x11, 0xcd,
       0x91, 0xa3, 0x9a, 0xf4, 0x8a, 0xa1, 0x7b, 0x4f,
       0x63, 0xf1, 0x75, 0xd3}
    },
    {
      4,
      {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
       0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
       0x19},
      byte_vector(50, 0xcd),
      {0x4c, 0x90, 0x07, 0xf4, 0x02, 0x62, 0x50, 0xc6,
       0xbc, 0x84, 0x14, 0xf9, 0xbf, 0x50, 0xc8, 0x6c,
       0x2d, 0x72, 0x35, 0xda}
    },
    // Truncation Test Vector # 5 omitted because truncation is
    // performed upon a complete digest by the caller.
    {
      6,
      byte_vector(80, 0xaa),
      byte_vector_from_string("Test Using Larger Than Block-Size Key - Hash Key First"),
      {0xaa, 0x4a, 0xe5, 0xe1, 0x52, 0x72, 0xd0, 0x0e,
       0x95, 0x70, 0x56, 0x37, 0xce, 0x8a, 0x3b, 0x55,
       0xed, 0x40, 0x21, 0x12}
    },
    {
      7,
      byte_vector(80, 0xaa),
      byte_vector_from_string("Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data"),
      {0xe8, 0xe9, 0x9d, 0x0f, 0x45, 0x23, 0x7d, 0x78,
       0x6d, 0x6b, 0xba, 0xa7, 0x96, 0x5c, 0x78, 0x08,
       0xbb, 0xff, 0x1a, 0x91}
    }
};

TEST(HMAC_SHA1Test, hmac_sha1_test)
{
  unsigned char tmp[SHA1::HASHLEN];
  for (auto& tv : RFC2202_HMAC_SHA1_TestVectors) {
    auto tv_num = std::get<0>(tv);

    const uint8_t* pkey = &std::get<1>(tv)[0];
    unsigned long key_len = static_cast<unsigned long>(std::get<1>(tv).size());

    const uint8_t* pdata = &std::get<2>(tv)[0];
    unsigned long data_len = static_cast<unsigned long>(std::get<2>(tv).size());

    const uint8_t* pdigest_expected = &std::get<3>(tv)[0];
    size_t digest_len = ::get<3>(tv).size();
    EXPECT_TRUE(digest_len == SHA1::HASHLEN);

    // Direct type usage: HMAC<SHA1...>
    HMAC<SHA1, SHA1::HASHLEN, SHA1::BLOCKSIZE> md(pkey, key_len);
    md.Update(pdata, data_len);
    md.Final(tmp);
    EXPECT_TRUE(memcmp(tmp, pdigest_expected, SHA1::HASHLEN) == 0) << "HMAC<SHA1...> type: Test vector " << tv_num;

    // Indirect type usage: HMAC_SHA1
    HMAC_SHA1 md2(pkey, key_len);
    md2.Update(pdata, data_len);
    md2.Final(tmp);
    EXPECT_TRUE(memcmp(tmp, pdigest_expected, SHA1::HASHLEN) == 0) << "HMAC_SHA1 type: Test vector " << tv_num;
  }
}
