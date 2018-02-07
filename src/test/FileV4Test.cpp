/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FileV4Test.cpp: Unit test for V4 file format

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWSfileV4.h"
#include "core/PWScore.h"

#include "os/file.h"

#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class FileV4Test : public ::testing::Test
{
protected:
  FileV4Test(); // to init members
  PWSfileHeader hdr;
  CItemData smallItem, fullItem, item;
  CItemAtt attItem;
  void SetUp();
  void TearDown();

  const StringX passphrase;
  stringT fname;
  // members used to populate and test fullItem:
  const StringX title, password, user, notes, group;
  const StringX url, at, email, symbols, runcmd;
  const time_t aTime, cTime, xTime, pmTime, rmTime;
  const int16 iDCA, iSDCA;
  time_t tVal;
  int16 iVal16;
  int32 iVal32;
};

FileV4Test::FileV4Test()
  : passphrase(_T("enchilada-sonol")), fname(_T("V4test.psafe4")),
    title(_T("a-title")), password(_T("b-password!?")),
    user(_T("C-UserR-ינור")), // non-English
    notes(_T("N is for notes\nwhich can span lines\r\nin several ways.")),
    group(_T("Groups.are.nested.by.dots")), url(_T("http://pwsafe.org/")),
    at(_T("\\u\\t\\t\\n\\p\\t\\n")), email(_T("joe@spammenot.com")),
    symbols(_T("<-_+=@?>")), runcmd(_T("Run 4 your life")),
    aTime(1409901292), // time test was first added, from http://www.unixtimestamp.com/
    cTime(1409901293), xTime(1409901294), pmTime(1409901295), rmTime(1409901296),
    iDCA(3), iSDCA(8),
    tVal(0), iVal16(-1), iVal32(-1)
{}

void FileV4Test::SetUp()
{
  hdr.m_prefString = _T("aPrefString");
  hdr.m_whenlastsaved = 1413129351; // overwritten in Open()
  hdr.m_lastsavedby = _T("aUser");
  hdr.m_lastsavedon = _T("aMachine");
  hdr.m_whatlastsaved = _T("PasswordSafe test framework");
  hdr.m_DB_Name = fname.c_str();
  hdr.m_DB_Description = _T("Test the header's persistency");

  fullItem.CreateUUID();
  fullItem.SetTitle(title);
  fullItem.SetPassword(password);
  fullItem.SetUser(user);
  fullItem.SetNotes(notes);
  fullItem.SetGroup(group);
  fullItem.SetURL(url);
  fullItem.SetAutoType(at);
  fullItem.SetEmail(email);
  fullItem.SetSymbols(symbols);
  fullItem.SetRunCommand(runcmd);
  fullItem.SetATime(aTime);
  fullItem.SetCTime(cTime);
  fullItem.SetXTime(xTime);
  fullItem.SetPMTime(pmTime);
  fullItem.SetRMTime(rmTime);
  fullItem.SetDCA(iDCA);
  fullItem.SetShiftDCA(iSDCA);

  smallItem.CreateUUID();
  smallItem.SetTitle(_T("picollo"));
  smallItem.SetPassword(_T("tiny-passw"));

  attItem.CreateUUID();
  attItem.SetTitle(L"I'm an attachment");
  const stringT testAttFile(L"data/image1.jpg");
  int status = attItem.Import(testAttFile);
  ASSERT_EQ(PWSfile::SUCCESS, status);
}

void FileV4Test::TearDown()
{
  ASSERT_TRUE(pws_os::DeleteAFile(fname));
  ASSERT_FALSE(pws_os::FileExists(fname));
}

// And now the tests...

TEST_F(FileV4Test, EmptyFile)
{
  PWSfileV4 fw(fname.c_str(), PWSfile::Write, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV4 fr(fname.c_str(), PWSfile::Read, PWSfile::V40);
  // Try opening with wrong passphrasse, check failure
  EXPECT_EQ(PWSfile::WRONG_PASSWORD, fr.Open(_T("x")));

  // Now open with correct one, check emptiness
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}

TEST_F(FileV4Test, HeaderTest)
{
  // header is written when file's opened for write.
  PWSfileHeader hdr1, hdr2;

  PWSfileV4 fw(fname.c_str(), PWSfile::Write, PWSfile::V40);
  fw.SetHeader(hdr);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));

  hdr1 = fw.GetHeader(); // Some fields set by Open()
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV4 fr(fname.c_str(), PWSfile::Read, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));

  hdr2 = fr.GetHeader();
  // We need the following to read past the termination block!
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
  ASSERT_EQ(hdr1, hdr2);
}

TEST_F(FileV4Test, ItemTest)
{
  PWSfileV4 fw(fname.c_str(), PWSfile::Write, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(smallItem));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(fullItem));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV4 fr(fname.c_str(), PWSfile::Read, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(smallItem, item);
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(fullItem, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}

TEST_F(FileV4Test, MulitKeysTest)
{
  const StringX pw2(_T("Mellow Yellowerer")), pw3(_T("spr1ngtime~nAplam"));

  PWSfileV4::CKeyBlocks kbs;
  ASSERT_TRUE(kbs.AddKeyBlock(passphrase, passphrase));
  ASSERT_TRUE(kbs.AddKeyBlock(passphrase, pw2));
  ASSERT_TRUE(kbs.AddKeyBlock(passphrase, pw3));

  PWSfileV4 fw(fname.c_str(), PWSfile::Write, PWSfile::V40);

  fw.SetKeyBlocks(kbs);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(pw2));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(smallItem));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(fullItem));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  PWSfileV4 fr(fname.c_str(), PWSfile::Read, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(pw3));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(smallItem, item);
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(fullItem, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(smallItem, item);
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(item));
  EXPECT_EQ(fullItem, item);
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());

  PWSfileV4::CKeyBlocks kbs2 = fr.GetKeyBlocks();
  EXPECT_TRUE(kbs.RemoveKeyBlock(pw2));
  EXPECT_FALSE(kbs.RemoveKeyBlock(pw2));
  EXPECT_TRUE(kbs.RemoveKeyBlock(pw3));
  EXPECT_FALSE(kbs.RemoveKeyBlock(pw3));
  EXPECT_FALSE(kbs.RemoveKeyBlock(passphrase));
}

TEST_F(FileV4Test, AttTest)
{
  PWSfileV4 fw(fname.c_str(), PWSfile::Write, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(attItem));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  CItemAtt readAtt;
  PWSfileV4 fr(fname.c_str(), PWSfile::Read, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(readAtt));
  EXPECT_EQ(PWSfile::END_OF_FILE, fr.ReadRecord(item));
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
  attItem.SetOffset(readAtt.GetOffset());
  EXPECT_EQ(attItem, readAtt);
}

TEST_F(FileV4Test, HdrItemAttTest)
{
  PWSfileHeader hdr1;
  PWSfileV4 fw(fname.c_str(), PWSfile::Write, PWSfile::V40);

  pws_os::CUUID att_uuid = attItem.GetUUID();
  fullItem.SetAttUUID(att_uuid);

  fw.SetHeader(hdr);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(passphrase));

  hdr1 = fw.GetHeader(); // Some fields set by Open()
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(fullItem));
  EXPECT_EQ(PWSfile::SUCCESS, fw.WriteRecord(attItem));
  ASSERT_EQ(PWSfile::SUCCESS, fw.Close());
  ASSERT_TRUE(pws_os::FileExists(fname));

  CItemData readData[2];
  CItemAtt readAtt;
  PWSfileV4 fr(fname.c_str(), PWSfile::Read, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fr.Open(passphrase));
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(readData[0]));
  EXPECT_EQ(fullItem, readData[0]);
  EXPECT_EQ(PWSfile::WRONG_RECORD, fr.ReadRecord(readData[1])); // att here!
  EXPECT_EQ(PWSfile::SUCCESS, fr.ReadRecord(readAtt));
  attItem.SetOffset(readAtt.GetOffset());
  EXPECT_EQ(attItem, readAtt);
  EXPECT_EQ(PWSfile::SUCCESS, fr.Close());
}

TEST_F(FileV4Test, CoreRWTest)
{
  PWScore core;
  const StringX passkey(L"3rdMambo");

  fullItem.SetAttUUID(attItem.GetUUID());
  EXPECT_EQ(0U, attItem.GetRefcount());

  core.SetPassKey(passkey);
  core.Execute(AddEntryCommand::Create(&core, fullItem, pws_os::CUUID::NullUUID(), &attItem));
  EXPECT_TRUE(core.HasAtt(attItem.GetUUID()));
  EXPECT_EQ(1U, core.GetAtt(attItem.GetUUID()).GetRefcount());
  EXPECT_EQ(PWSfile::SUCCESS, core.WriteFile(fname.c_str(), PWSfile::V40));

  core.ClearDBData();
  EXPECT_EQ(PWSfile::FAILURE, core.ReadFile(fname.c_str(), L"WrongPassword", true));
  EXPECT_EQ(PWSfile::SUCCESS, core.ReadFile(fname.c_str(), passkey, true));
  ASSERT_EQ(1, core.GetNumEntries());
  ASSERT_EQ(1, core.GetNumAtts());
  ASSERT_TRUE(core.Find(fullItem.GetUUID()) != core.GetEntryEndIter());

  const CItemData readFullItem = core.GetEntry(core.Find(fullItem.GetUUID()));
  EXPECT_TRUE(readFullItem.HasAttRef());
  EXPECT_EQ(attItem.GetUUID(), readFullItem.GetAttUUID());
  EXPECT_EQ(fullItem, readFullItem);
  ASSERT_TRUE(core.HasAtt(attItem.GetUUID()));
  EXPECT_EQ(1U, core.GetAtt(attItem.GetUUID()).GetRefcount());

  core.Execute(DeleteEntryCommand::Create(&core, readFullItem));
  ASSERT_EQ(0, core.GetNumEntries());
  ASSERT_EQ(0, core.GetNumAtts());

  // Get core to delete any existing commands
  core.ClearCommands();
}
