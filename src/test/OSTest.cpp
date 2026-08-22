/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OSTest.cpp: Unit test for misc pws_os functions

#include "os/media.h"
#include "os/dir.h"
#include "os/file.h"

#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#include "gtest/gtest.h"

TEST(OSTest, testMedia)
{
  EXPECT_EQ(_T("unknown"), pws_os::GetMediaType(_T("nosuchfile")));
  EXPECT_EQ(_T("text/plain"), pws_os::GetMediaType(_T("data/text1.txt")));
  EXPECT_EQ(_T("image/jpeg"), pws_os::GetMediaType(_T("data/image1.jpg")));
}

TEST(OSTest, testPath)
{
  #ifdef WIN32
  const stringT path_sep(_T("\\"));
  const stringT in_drive(_T("C:"));
  #else
  const stringT path_sep(_T("/"));
  const stringT in_drive(_T(""));
  #endif

  const stringT in_dir = path_sep + _T("dir1") + path_sep + _T("b") + path_sep;
  const stringT in_file = _T("filename");
  const stringT in_ext = _T(".ext");
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

TEST(OSTest, testNullFileOperations)
{
  EXPECT_EQ(0, pws_os::FFlush(nullptr));
  EXPECT_EQ(0, pws_os::FClose(nullptr));
  EXPECT_EQ(0, pws_os::FFlushAndClose(nullptr));
}

#ifndef WIN32
TEST(OSTest, testFlushAndCloseAfterFlushFailure)
{
  int pipeDescriptors[2];
  ASSERT_EQ(0, pipe(pipeDescriptors));

  std::FILE *fd = fdopen(pipeDescriptors[1], "w");
  ASSERT_NE(nullptr, fd);
  const int fileDescriptor = fileno(fd);
  ASSERT_NE(-1, fileDescriptor);

  ASSERT_EQ(1U, fwrite("x", 1, 1, fd));
  // The stdio flush succeeds, but the OS-level sync fails because a pipe
  // has no backing storage to synchronize.
  EXPECT_EQ(EOF, pws_os::FFlushAndClose(fd));
  EXPECT_EQ(-1, fcntl(fileDescriptor, F_GETFD));

  close(pipeDescriptors[0]);
}
#endif
