/*
* Copyright (c) 2003-2022 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OSTest.cpp: Unit test for misc pws_os functions

#include "os/media.h"
#include "os/dir.h"
#include "gtest/gtest.h"

TEST(OSTest, testMedia)
{
  EXPECT_EQ(_S("unknown"), pws_os::GetMediaType(_S("nosuchfile")));
  EXPECT_EQ(_S("text/plain"), pws_os::GetMediaType(_S("data/text1.txt")));
  EXPECT_EQ(_S("image/jpeg"), pws_os::GetMediaType(_S("data/image1.jpg")));
}

TEST(OSTest, testPath)
{
  #ifdef WIN32
  const stringT path_sep(_S("\\"));
  const stringT in_drive(_S("C:"));
  #else
  const stringT path_sep(_S("/"));
  const stringT in_drive(_S(""));
  #endif

  const stringT in_dir = path_sep + _S("dir1") + path_sep + _S("b") + path_sep;
  const stringT in_file = _S("filename");
  const stringT in_ext = _S(".ext");
  const stringT in_path =  in_drive + in_dir + in_file + in_ext;

  stringT out_drive, out_dir, out_file, out_ext, out_path;

  EXPECT_TRUE(pws_os::splitpath(in_path, out_drive, out_dir, out_file, out_ext));

  EXPECT_EQ(in_drive, out_drive);
  EXPECT_EQ(in_dir, out_dir);
  EXPECT_EQ(in_file, out_file);
  EXPECT_EQ(in_ext, out_ext);

  out_path = pws_os::makepath(in_drive, in_dir, in_file, in_ext);
  EXPECT_EQ(in_path, out_path);
}