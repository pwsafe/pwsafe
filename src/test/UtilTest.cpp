/*
* Copyright (c) 2018-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// UtilTest.cpp: Unit test for selected functions in Util.cpp

#include "core/Util.h"
#include "gtest/gtest.h"

TEST(UtilTest1, convert_test_ascii)
{
  StringX src(L"abc");
  unsigned char *dst = nullptr;
  size_t dst_size = 0;
  ConvertPasskey(src, dst, dst_size);
  EXPECT_EQ(3U, dst_size);
  EXPECT_STREQ("abc", reinterpret_cast<const char *>(dst));
  delete[] dst;
}

TEST(UtilTest2, convert_test_nonascii)
{
  wchar_t src_wchar[] = {0x05d0, 0x05d1, 0x05d2, 0}; // aleph bet gimel unicode
  StringX src(src_wchar);
  unsigned char *dst = nullptr;
  size_t dst_size = 0;
  ConvertPasskey(src, dst, dst_size);
  EXPECT_EQ(6U, dst_size);
  EXPECT_STREQ("אבג", reinterpret_cast<const char *>(dst));
  delete[] dst;
}

TEST(UtilTest, UnsignedIntegerDecoding)
{
  struct ReadCase {
    alignas(uint64) unsigned char bytes[8];
    uint16 expected16;
    uint32 expected32;
    uint64 expected64;
  };
  const ReadCase cases[] = {
    {{0}, 0, 0, 0},
    {{0xff, 0x7f}, 0x7fffU, 0x7fffU, 0x7fffULL},
    {{0x00, 0x80}, 0x8000U, 0x8000U, 0x8000ULL},
    {{0xff, 0xff, 0xff, 0x7f}, 0xffffU, 0x7fffffffU, 0x7fffffffULL},
    {{0x00, 0x00, 0x00, 0x80}, 0, 0x80000000U, 0x80000000ULL},
    {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f},
     0xffffU, 0xffffffffU, 0x7fffffffffffffffULL},
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80},
     0, 0, 0x8000000000000000ULL},
    {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
     0xffffU, 0xffffffffU, 0xffffffffffffffffULL},
    {{0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01},
     0xcdefU, 0x89abcdefU, 0x0123456789abcdefULL}
  };
  for (const auto &test : cases) {
    SCOPED_TRACE(::testing::Message() << "value=" << test.expected64);
    EXPECT_EQ(test.expected16, getUint16(test.bytes));
    EXPECT_EQ(test.expected32, getUint32(test.bytes));
    EXPECT_EQ(test.expected64, getUint64(test.bytes));
  }
}

TEST(UtilTest, UnsignedIntegerWidening)
{
  alignas(uint64) const unsigned char highBit[] = {0x00, 0x00, 0x00, 0x80};
  alignas(uint64) const unsigned char allBits[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };
  const size_t highLength = getUint32(highBit);
  const size_t maxLength = getUint32(allBits);
  EXPECT_EQ(size_t(0x80000000U), highLength);
  EXPECT_EQ(size_t(0xffffffffU), maxLength);
  EXPECT_EQ(0xffffULL, static_cast<uint64>(getUint16(allBits)));
  EXPECT_EQ(0xffffffffULL, static_cast<uint64>(getUint32(allBits)));

  // The signed helpers must keep their existing interpretation.
  EXPECT_EQ(-1, getInt16(allBits));
  EXPECT_EQ(-1, getInt32(allBits));
  EXPECT_EQ(-1, getInt64(allBits));
}

#if defined(PWS_BIG_ENDIAN)
TEST(UtilTest, UnalignedInteger64Decoding)
{
  // Offset an aligned buffer by one byte to exercise the bytewise reader.
  alignas(uint64) const unsigned char bytes[] = {
    0, 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
  };
  EXPECT_EQ(0x0123456789abcdefULL, getUint64(bytes + 1));
  EXPECT_EQ(int64(0x0123456789abcdefULL), getInt64(bytes + 1));
}
#endif
