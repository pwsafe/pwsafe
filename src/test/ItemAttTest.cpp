/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemAttTest.cpp: Unit test for CItemAtt class

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/ItemAtt.h"
#include "core/PWScore.h"
#include "os/file.h"
#include "os/dir.h"
#include "os/debug.h"

#include "gtest/gtest.h"

#include <vector>

using namespace std;

// A fixture for factoring common code across tests
class ItemAttTest : public ::testing::Test
{
protected:
  ItemAttTest(); // to init members
  void SetUp();

  // members used to populate and test fullItem:
  const StringX title, mediaType;
  stringT fullfileName, fileName, filePath;
};

ItemAttTest::ItemAttTest()
  : title(_T("a-title")), mediaType(_T("application/octet-stream"))
{
  fullfileName = pws_os::fullpath(L"data/image1.jpg");

  stringT sDrive, sDir, sFName, sExtn;
  pws_os::splitpath(fullfileName, sDrive, sDir, sFName, sExtn);
  fileName = sFName + sExtn;
  filePath = sDrive + sDir;
}

void ItemAttTest::SetUp()
{
}

// And now the tests...

TEST_F(ItemAttTest, EmptyItems)
{
  CItemAtt ai1, ai2;
  EXPECT_TRUE(ai1 == ai2);

  ai1.SetTitle(title);
  EXPECT_FALSE(ai1 == ai2);  

  ai2.SetTitle(title);
  EXPECT_TRUE(ai1 == ai2);
}

TEST_F(ItemAttTest, UUIDs)
{
  CItemAtt ai1, ai2;
  EXPECT_FALSE(ai1.HasUUID());

  ai1.CreateUUID();
  ai2.CreateUUID();
  EXPECT_FALSE(ai1 == ai2);

  ai2.SetUUID(ai1.GetUUID());
  EXPECT_TRUE(ai1 == ai2);
}

TEST_F(ItemAttTest, ImpExp)
{
  const stringT testImpFile(fullfileName);
  const stringT testExpFile(L"output.tmp");
  CItemAtt ai;
  int status;

  status = ai.Import(L"nosuchfile");
  EXPECT_EQ(PWScore::CANT_OPEN_FILE, status);
  EXPECT_EQ(L"", ai.GetFileName());
  EXPECT_FALSE(ai.HasContent());

  status = ai.Import(testImpFile);
  EXPECT_EQ(PWScore::SUCCESS, status);
  EXPECT_STREQ(fileName.c_str(), ai.GetFileName().c_str());
  EXPECT_STREQ(filePath.c_str(), ai.GetFilePath().c_str());
  EXPECT_TRUE(ai.HasContent());

  status = ai.Export(testExpFile);
  EXPECT_EQ(PWScore::SUCCESS, status);
  EXPECT_TRUE(pws_os::FileExists(testExpFile));

  FILE *f1 = pws_os::FOpen(testImpFile, L"rb");
  FILE *f2 = pws_os::FOpen(testExpFile, L"rb");
  EXPECT_EQ(pws_os::fileLength(f1), pws_os::fileLength(f2));

  size_t flen = static_cast<size_t>(pws_os::fileLength(f1));
  unsigned char *m1 = new unsigned char[flen];
  unsigned char *m2 = new unsigned char[flen];

  ASSERT_EQ(1, fread(m1, flen, 1, f1));
  ASSERT_EQ(1, fread(m2, flen, 1, f2));

  fclose(f1); fclose(f2);
  EXPECT_EQ(0, memcmp(m1, m2, flen));

  delete[] m1; delete[] m2;
  pws_os::DeleteAFile(testExpFile);
}

TEST_F(ItemAttTest, CopyCtor)
{
  const stringT testImpFile(fullfileName);
  CItemAtt ea1;
  CItemAtt ea2(ea1);
  EXPECT_TRUE(ea1 == ea2);

  CItemAtt a1;
  a1.SetTitle(title);
  int status = a1.Import(testImpFile);
  ASSERT_EQ(PWScore::SUCCESS, status);

  CItemAtt a2(a1);
  EXPECT_TRUE(a1 == a2);
}

TEST_F(ItemAttTest, Getters_n_Setters)
{
  CItemAtt ai;
  pws_os::CUUID uuid;
  time_t cTime = 1425836169;
  unsigned char content[122] = {0xff, 0x00, 0xb4, 0x65, 0xfc, 0x91, 0xfb, 0xbf,
                                0xe0, 0x8f, 0xea, 0x6b, 0xf9, 0x9f, 0xde, 0x1f,
                                0x62, 0xd6, 0xbf, 0xe8, 0x2f, 0xfb, 0x2c, 0xff,
                                0x00, 0xe0, 0xf3, 0xe1, 0xff, 0x00, 0xff, 0x00,
                                0x1d, 0xa5, 0x5b, 0x2d, 0x67, 0xbe, 0xaf, 0xfb,
                                0x2c, 0xff, 0x00, 0xe0, 0xf3, 0xc0, 0x1f, 0xfc,
                                0x76, 0x8a, 0x29, 0x7f, 0x68, 0x4b, 0xf9, 0x23,
                                0xf7, 0x7f, 0xc1, 0x1f, 0xd4, 0xd7, 0xf3, 0x3f,
                                0xbc, 0x5f, 0xb1, 0x6b, 0x1f, 0xf4, 0x17, 0xfd,
                                0x96, 0x7f, 0xf0, 0x79, 0xe0, 0x0f, 0xfe, 0x3b,
                                0x47, 0xd8, 0xb5, 0x8f, 0xfa, 0x0b, 0xfe, 0xcb,
                                0x3f, 0xf8, 0x3c, 0xf0, 0x07, 0xff, 0x00, 0x1d,
                                0xa2, 0x8a, 0x3f, 0xb4, 0x25, 0xfc, 0x91, 0xfb,
                                0xbf, 0xe0, 0x8b, 0xea, 0x6b, 0xf9, 0x9f, 0xde,
                                0xb0, 0xf1, 0xa7, 0xef, 0xa6, 0xdb, 0xf3, 0x3f,
                                0xff, 0xd9};
  ai.SetUUID(uuid);
  ai.SetTitle(title);
  ai.SetCTime(cTime);
  ai.SetContent(content, sizeof(content));

  time_t tVal = 0;
  unsigned char *contentVal;

  EXPECT_EQ(uuid, ai.GetUUID());
  EXPECT_EQ(title, ai.GetTitle());
  EXPECT_EQ(cTime, ai.GetCTime(tVal));
  ASSERT_EQ(sizeof(content), ai.GetContentLength());

  size_t contentSize = ai.GetContentSize();
  contentVal = new unsigned char[contentSize];

  EXPECT_FALSE(ai.GetContent(contentVal, contentSize - 1));
  EXPECT_TRUE(ai.GetContent(contentVal, contentSize));
  EXPECT_EQ(0, memcmp(content, contentVal, sizeof(content)));

  delete[] contentVal;
}
