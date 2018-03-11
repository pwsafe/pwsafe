/*
* Copyright (c) 2018 Rony Shapiro <ronys@pwsafe.org>.
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
  EXPECT_EQ(dst_size, 3);
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
  EXPECT_EQ(dst_size, 6);
  EXPECT_STREQ("אבג", reinterpret_cast<const char *>(dst));
  delete[] dst;
}
