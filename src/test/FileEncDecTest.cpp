/*
* Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
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
};

FileEncDecTest::FileEncDecTest()
  : passphrase(_T("strawberry_phasers")), fname(_T("data/text1.txt"))
{}

void FileEncDecTest::SetUp()
{

}

void FileEncDecTest::TearDown()
{
  // ASSERT_TRUE(pws_os::DeleteAFile(fname));
  // ASSERT_FALSE(pws_os::FileExists(fname));
}

// And now the tests...

TEST_F(FileEncDecTest, NoFile)
{
  bool res;
  stringT errmes(L"");

  res = PWSfile::Encrypt(L"nosuchfile", passphrase, errmes);
  EXPECT_FALSE(res);
  EXPECT_FALSE(errmes.empty());

  errmes = L"";
  res = PWSfile::Decrypt(L"nosuchfile", passphrase, errmes);
  EXPECT_FALSE(res);
  EXPECT_FALSE(errmes.empty());
}

#if 0
TEST_F(FileEncDecTest, HeaderTest)
{
  // header is written when file's opened for write.
  PWSfileHeader hdr1, hdr2;
  hdr1.m_prefString = _T("aPrefString");
  hdr1.m_whenlastsaved = 1413129351; // overwritten in Open()
  hdr1.m_whenpwdlastchanged = 1529684734;
  hdr1.m_lastsavedby = _T("aUser");
  hdr1.m_lastsavedon = _T("aMachine");
  hdr1.m_whatlastsaved = _T("PasswordSafe test framework");
  hdr1.m_DB_Name = fname.c_str();
  hdr1.m_DB_Description = _T("Test the header's persistency");

  PWSFileEncDec fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  fw.SetHeader(hdr1);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));

  hdr1 = fw.GetHeader(); // Some fields set by Open()
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSFileEncDec fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));

  hdr2 = fr.GetHeader();
  // We need the following to read past the termination block!
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
  ASSERT_EQ(hdr1, hdr2);
}

TEST_F(FileEncDecTest, ItemTest)
{
  PWSFileEncDec fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(smallItem));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(fullItem));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSFileEncDec fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(smallItem, item);
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(fullItem, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}

TEST_F(FileEncDecTest, UnknownPersistencyTest)
{
  CItemData d1;
  d1.CreateUUID();
  d1.SetTitle(_T("future"));
  d1.SetPassword(_T("possible"));
  unsigned char uv[] = {55, 42, 78, 30, 16, 93};
  d1.SetUnknownField(CItemData::UNKNOWN_TESTING, sizeof(uv), uv);

  PWSFileEncDec fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(d1));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSFileEncDec fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(d1, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}
#endif 0