/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FileEncDecTest.cpp: Unit test for file encryption/decryption

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWSfile.h"
#include "os/file.h"
#include "os/dir.h"

#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class FileEncDecTest : public ::testing::Test
{
protected:
  FileEncDecTest(); // to init members
  void SetUp();
  void TearDown();

  const StringX passphrase;
  stringT fname;
  stringT errmes;
  static const stringT suffix;

  void TestFile(const stringT& testfile); // encrypt, decrypt and compare against original
};

const stringT FileEncDecTest::suffix = L".PSF";

FileEncDecTest::FileEncDecTest()
  : passphrase(_T("strawberry_phasers")), fname(_T("text1.txt")), errmes(L"")
{}

void FileEncDecTest::SetUp()
{
  // tests require writable data dir with text1.txt file
  ASSERT_TRUE(pws_os::chdir(L"data"));
}

void FileEncDecTest::TearDown()
{
  ASSERT_TRUE(pws_os::chdir(L".."));
}

// And now the tests...

TEST_F(FileEncDecTest, NoFile)
{

  EXPECT_FALSE(PWSfile::Encrypt(L"nosuchfile", passphrase, errmes));
  EXPECT_FALSE(errmes.empty());

  errmes = L"";
  EXPECT_FALSE(PWSfile::Decrypt(L"nosuchfile", passphrase, errmes));
  EXPECT_FALSE(errmes.empty());
}

TEST_F(FileEncDecTest, EmptyFile)
{
  const stringT emptyFile(L"anEmptyFile");
  const stringT emptyCipherFile = emptyFile + suffix;

  // create an empty file
  auto fp = pws_os::FOpen(emptyFile, L"w");
  ASSERT_TRUE(fp != nullptr);
  auto res = pws_os::FClose(fp, true);
  ASSERT_TRUE(res == 0);

  // decrypting an empty file should fail (too small)
  errmes = L"";
  EXPECT_FALSE(PWSfile::Decrypt(emptyCipherFile, emptyFile.c_str(), errmes));
  EXPECT_FALSE(errmes.empty());

  errmes = L"";
  EXPECT_TRUE(PWSfile::Encrypt(emptyFile, passphrase, errmes));
  EXPECT_TRUE(errmes.empty());
  ASSERT_TRUE(pws_os::FileExists(emptyCipherFile));

  // try decrypting with wrong passphrase
  errmes = L"";
  EXPECT_FALSE(PWSfile::Decrypt(emptyCipherFile, emptyFile.c_str(), errmes));
  EXPECT_FALSE(errmes.empty());

  // now with the correct passphrase
  errmes = L"";
  EXPECT_TRUE(PWSfile::Decrypt(emptyCipherFile, passphrase, errmes));
  EXPECT_TRUE(errmes.empty());

  // check decrypted file
  fp = pws_os::FOpen(emptyFile, L"r");
  ASSERT_TRUE(fp != nullptr);
  EXPECT_EQ(pws_os::fileLength(fp), 0);
  res = pws_os::FClose(fp, true);
  ASSERT_TRUE(res == 0);


  // cleanup
  ASSERT_TRUE(pws_os::DeleteAFile(emptyFile));
  ASSERT_TRUE(pws_os::DeleteAFile(emptyCipherFile));
}

TEST_F(FileEncDecTest, RegularFile)
{
  // test files are part of source tree
  TestFile(L"text1.txt");
  TestFile(L"image1.jpg");
}

TEST_F(FileEncDecTest, BigFile)
{
  // fake big file by changing definition of "big"
  auto oldThreshold = PWSfile::fileThresholdSize;
  PWSfile::fileThresholdSize = 100000; // smaller than image1.jpg
  TestFile(L"image1.jpg");
  PWSfile::fileThresholdSize = oldThreshold;
}

void FileEncDecTest::TestFile(const stringT& testfile)
{
  const stringT originalTestFile = testfile; 
  const stringT workTestFile = L"EncDecTest";
  const stringT workCipherFile = workTestFile + suffix;

  ASSERT_TRUE(pws_os::FileExists(originalTestFile));

  // create a copy
  const stringT curdir = pws_os::getcwd() + L"/";
  ASSERT_TRUE(pws_os::CopyAFile(curdir + originalTestFile, curdir + workTestFile));


  errmes = L"";
  EXPECT_TRUE(PWSfile::Encrypt(workTestFile, passphrase, errmes));
  EXPECT_TRUE(errmes.empty());
  ASSERT_TRUE(pws_os::FileExists(workCipherFile));

  // try decrypting with wrong passphrase
  errmes = L"";
  EXPECT_FALSE(PWSfile::Decrypt(workCipherFile, originalTestFile.c_str(), errmes));
  EXPECT_FALSE(errmes.empty());

  // now with the correct passphrase
  errmes = L"";
  EXPECT_TRUE(PWSfile::Decrypt(workCipherFile, passphrase, errmes));
  EXPECT_TRUE(errmes.empty());

  // check decrypted file
  auto fp = pws_os::FOpen(originalTestFile, L"rb");
  ASSERT_TRUE(fp != nullptr);
  auto len1 = pws_os::fileLength(fp);
  auto origBuf = new char[len1];
  ASSERT_EQ(len1, fread(origBuf, 1, len1, fp));
  auto res = pws_os::FClose(fp, true);
  ASSERT_TRUE(res == 0);

  fp = pws_os::FOpen(workTestFile, L"rb");
  ASSERT_TRUE(fp != nullptr);
  auto len2 = pws_os::fileLength(fp);
  EXPECT_EQ(len1, len2);
  auto workBuf = new char[len2];
  ASSERT_EQ(len2, fread(workBuf, 1, len2, fp));
  res = pws_os::FClose(fp, true);
  ASSERT_TRUE(res == 0);
  ASSERT_EQ(memcmp(origBuf, workBuf, len1), 0);

  //cleanup
  delete[] origBuf;
  delete[] workBuf;
  ASSERT_TRUE(pws_os::DeleteAFile(workTestFile));
  ASSERT_TRUE(pws_os::DeleteAFile(workCipherFile));
}
