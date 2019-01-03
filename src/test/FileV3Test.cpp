/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FileV3Test.cpp: Unit test for V3 file format

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWSfileV3.h"
#include "os/file.h"

#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class FileV3Test : public ::testing::Test
{
protected:
  FileV3Test(); // to init members
  CItemData smallItem, fullItem, item;
  void SetUp();
  void TearDown();

  const StringX passphrase;
  stringT fname;
  // members used to populate and test fullItem:
  const StringX title, password, user, notes, group;
  const StringX url, at, email, polname, symbols, runcmd;
  const time_t aTime, cTime, xTime, pmTime, rmTime;
  const int16 iDCA, iSDCA;
  const int32 kbs;
  time_t tVal;
  int16 iVal16;
  int32 iVal32;
};

FileV3Test::FileV3Test()
  : passphrase(_T("enchilada-sonol")), fname(_T("V3test.psafe3")),
    title(_T("a-title")), password(_T("b-password!?")),
    user(_T("C-UserR-ינור")), // non-English
    notes(_T("N is for notes\nwhich can span lines\r\nin several ways.")),
    group(_T("Groups.are.nested.by.dots")), url(_T("http://pwsafe.org/")),
    at(_T("\\u\\t\\t\\n\\p\\t\\n")), email(_T("joe@spammenot.com")),
    polname(_T("liberal")), symbols(_T("<-_+=@?>")), runcmd(_T("Run 4 your life")),
    aTime(1409901292), // time test was first added, from http://www.unixtimestamp.com/
    cTime(1409901293), xTime(1409901294), pmTime(1409901295), rmTime(1409901296),
    iDCA(3), iSDCA(8), kbs(0x12345678),
    tVal(0), iVal16(-1), iVal32(-1)
{}

void FileV3Test::SetUp()
{
  fullItem.CreateUUID();
  fullItem.SetTitle(title);
  fullItem.SetPassword(password);
  fullItem.SetUser(user);
  fullItem.SetNotes(notes);
  fullItem.SetGroup(group);
  fullItem.SetURL(url);
  fullItem.SetAutoType(at);
  fullItem.SetEmail(email);
  fullItem.SetPolicyName(polname);
  fullItem.SetSymbols(symbols);
  fullItem.SetRunCommand(runcmd);
  fullItem.SetATime(aTime);
  fullItem.SetCTime(cTime);
  fullItem.SetXTime(xTime);
  fullItem.SetPMTime(pmTime);
  fullItem.SetRMTime(rmTime);
  fullItem.SetDCA(iDCA);
  fullItem.SetShiftDCA(iSDCA);
  fullItem.SetKBShortcut(kbs);

  smallItem.CreateUUID();
  smallItem.SetTitle(_T("picollo"));
  smallItem.SetPassword(_T("tiny-passw"));
}

void FileV3Test::TearDown()
{
  ASSERT_TRUE(pws_os::DeleteAFile(fname));
  ASSERT_FALSE(pws_os::FileExists(fname));
}

// And now the tests...

TEST_F(FileV3Test, EmptyFile)
{
  PWSfileV3 fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV3 fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  // Try opening with wrong passphrase, check failure
  EXPECT_EQ(PWSfile::WRONG_PASSWORD, fr.Open(_T("x")));

  // Now open with correct one, check emptiness
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}

TEST_F(FileV3Test, HeaderTest)
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

  PWSfileV3 fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  fw.SetHeader(hdr1);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));

  hdr1 = fw.GetHeader(); // Some fields set by Open()
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV3 fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));

  hdr2 = fr.GetHeader();
  // We need the following to read past the termination block!
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
  ASSERT_EQ(hdr1, hdr2);
}

TEST_F(FileV3Test, ItemTest)
{
  PWSfileV3 fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(smallItem));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(fullItem));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV3 fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(smallItem, item);
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(fullItem, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}

TEST_F(FileV3Test, UnknownPersistencyTest)
{
  CItemData d1;
  d1.CreateUUID();
  d1.SetTitle(_T("future"));
  d1.SetPassword(_T("possible"));
  unsigned char uv[] = {55, 42, 78, 30, 16, 93};
  d1.SetUnknownField(CItemData::UNKNOWN_TESTING, sizeof(uv), uv);

  PWSfileV3 fw(fname.c_str(), PWSfile::Write, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(d1));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV3 fr(fname.c_str(), PWSfile::Read, PWSfile::V30);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(d1, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}
