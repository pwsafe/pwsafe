/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ImportTextTest.cpp: Unit test for importing csv/text

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "os/file.h"
#include "os/dir.h"
#include "core/PWScore.h"

#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class ImportTextTest : public ::testing::Test
{
protected:
  ImportTextTest(); // to init members
  PWScore core;
    const stringT testFile1 = L"import-text-unit-test1.txt";

  
  void SetUp();
  void TearDown();

  int numImported, numSkipped, numPWHErrors, numRenamed, numWarnings, numNoPolicy;
};

ImportTextTest::ImportTextTest()
  : numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numWarnings(0), numNoPolicy(0)
{
  
}

void ImportTextTest::SetUp()
{
  ASSERT_TRUE(pws_os::chdir(L"data"));
  ASSERT_TRUE(pws_os::FileExists(testFile1));
}

void ImportTextTest::TearDown()
{
  ASSERT_TRUE(pws_os::chdir(L".."));
}

TEST_F(ImportTextTest, test1)
{
  stringT errorStr;
  CReport rpt;
  Command* cmd(nullptr);

  int status = core.ImportPlaintextFile(L"", testFile1.c_str(), L'\t',
    L'\xbb', false,
    errorStr,
    numImported, numSkipped,
    numPWHErrors, numRenamed,
    numNoPolicy,
    rpt, cmd);

  ASSERT_EQ(status, PWScore::SUCCESS);
  EXPECT_EQ(numImported, 2);
  EXPECT_EQ(numSkipped, 0);
  EXPECT_EQ(numPWHErrors, 0);
  EXPECT_EQ(numRenamed, 0);
  EXPECT_EQ(numNoPolicy, 0);
  EXPECT_TRUE(errorStr.empty());
  EXPECT_NE(cmd, nullptr);

  status = core.Execute(cmd);
  EXPECT_EQ(status, 0);
  // now test that we've read the data correctly
  auto p1 = core.Find(L"a.b.c", L"d-level-title", L"d-user");
  EXPECT_NE(p1, core.GetEntryEndIter());
  auto item1 = core.GetEntry(p1);
  EXPECT_EQ(item1.GetPassword(), L"d-password");
  EXPECT_EQ(item1.GetNotes(), L"line 1 of 3\r\nline 2 of 3\r\nline 3 of 3");


  auto p2 = core.Find(L"", L"toplevel-title1", L"toplevel user");
  EXPECT_NE(p2, core.GetEntryEndIter());
  auto item2 = core.GetEntry(p2);
  EXPECT_EQ(item2.GetPassword(), L"toplevel-password");
  EXPECT_EQ(item2.GetURL(), L"toplevelurl.com");
  EXPECT_EQ(item2.GetEmail(), L"tom@email.com");
  EXPECT_EQ(item2.GetNotes(), L"simple one-line note");
}