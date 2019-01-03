/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OSTest.cpp: Unit test for misc pws_os functions

#include "os/media.h"
#include "gtest/gtest.h"

TEST(OSTest, testMedia)
{
  EXPECT_EQ(_S("unknown"), pws_os::GetMediaType(_S("nosuchfile")));
  EXPECT_EQ(_S("text/plain"), pws_os::GetMediaType(_S("data/text1.txt")));
  EXPECT_EQ(_S("image/jpeg"), pws_os::GetMediaType(_S("data/image1.jpg")));
}
