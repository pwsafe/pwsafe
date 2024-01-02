/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SHA1Test.cpp: Unit test for SHA1 implementation
#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/crypto/sha1.h"
#include "gtest/gtest.h"

TEST(SHA1Test, sha1_test)
{
  static const struct {
    const char *msg;
    unsigned char hash[SHA1::HASHLEN];
  } tests[] = {
    { "abc",
      { 0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a,
        0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c,
        0x9c, 0xd0, 0xd8, 0x9d }
    },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
      { 0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e,
        0xba, 0xae, 0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5,
        0xe5, 0x46, 0x70, 0xf1 }
    },
  };

  size_t i;
  unsigned char tmp[SHA1::HASHLEN];
  for (i = 0; i < (sizeof(tests) / sizeof(tests[0])); i++) {
    SHA1 md;
    md.Update( (unsigned char*)tests[i].msg,
               (unsigned long)strlen(tests[i].msg));
    md.Final(tmp);
    EXPECT_TRUE(memcmp(tmp, tests[i].hash, SHA1::HASHLEN) == 0) << "test vector " << i;
  }
}
