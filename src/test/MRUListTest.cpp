/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// MRUListTest.cpp: Unit test for the Set/Get MRU List and the RecentDbList class

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/PWSprefs.h"
#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class MRUListTest : public ::testing::Test
{
protected:
  MRUListTest();
  ~MRUListTest() { prefs->PrepForUnitTests(false); }

  PWSprefs *prefs;
  std::vector<stringT> MRUFiles;
};

MRUListTest::MRUListTest()
{
  prefs = PWSprefs::GetInstance();
  prefs->PrepForUnitTests(true);

  MRUFiles.push_back(L"filename1");
  MRUFiles.push_back(L"filename2");
  MRUFiles.push_back(L"filename3");
  MRUFiles.push_back(L"filename4");
  MRUFiles.push_back(L"filename5");
}


TEST_F(MRUListTest, MRUList)
{
  unsigned int is, ig;
  std::vector<stringT> MRURet, v4, v0;

  // Test no-op behavior
  prefs->PrepForUnitTests(false);
  is = prefs->SetMRUList(MRUFiles, 10);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 0);
  EXPECT_EQ(ig, 0);
  EXPECT_EQ(MRURet.size(), 0);
  prefs->PrepForUnitTests(true);

  // Basic Set/Get
  prefs->SetPref(PWSprefs::MaxMRUItems, 10);
  is = prefs->SetMRUList(MRUFiles, 10);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 5);
  EXPECT_EQ(ig, 5);
  EXPECT_EQ(MRUFiles, MRURet);

  // Test max_MRU parameter of SetMRUList
  is = prefs->SetMRUList(MRUFiles, 6);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 5);
  EXPECT_EQ(ig, 5);
  EXPECT_EQ(MRUFiles, MRURet);

  v4 = MRUFiles;
  v4.pop_back();
  v4.pop_back();
  is = prefs->SetMRUList(MRUFiles, 3);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 3);
  EXPECT_EQ(ig, 3);
  EXPECT_EQ(v4, MRURet);

  v0.clear();
  is = prefs->SetMRUList(MRUFiles, 0);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 0);
  EXPECT_EQ(ig, 0);
  EXPECT_EQ(v0, MRURet);

  // Test MaxMRUItems limit in GetMRUList
  prefs->SetPref(PWSprefs::MaxMRUItems, 3);
  is = prefs->SetMRUList(MRUFiles, 10);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 5);
  EXPECT_EQ(ig, 3);
  EXPECT_EQ(v4, MRURet);

  prefs->SetPref(PWSprefs::MaxMRUItems, 0);
  is = prefs->SetMRUList(MRUFiles, 10);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 5);
  EXPECT_EQ(ig, 0);
  EXPECT_EQ(v0, MRURet);

  // Test removing *.ibak from the list
  prefs->SetPref(PWSprefs::MaxMRUItems, 10);
  v4 = MRUFiles;
  MRUFiles.push_back(L"Filename.bak");
  MRUFiles.push_back(L"Filename.ibak~");
  MRUFiles.push_back(L"Filename.ibak");
  MRUFiles.push_back(L"Filename.ibak~");
  is = prefs->SetMRUList(MRUFiles, 10);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 5);
  EXPECT_EQ(ig, 5);
  EXPECT_EQ(v4, MRURet);
}

#ifndef WIN32
#include "ui/wxWidgets/RecentDbList.h"

class RecentDBTest : public ::testing::Test
{
protected:
  RecentDBTest();
  ~RecentDBTest() { prefs->PrepForUnitTests(false); }

  PWSprefs *prefs;
  std::vector<stringT> MRUFiles;
  wxArrayString StrList;
};

RecentDBTest::RecentDBTest()
{
  prefs = PWSprefs::GetInstance();
  prefs->PrepForUnitTests(true);

  MRUFiles.push_back(L"filename1");
  MRUFiles.push_back(L"filename2");
  MRUFiles.push_back(L"filename3");
  MRUFiles.push_back(L"filename4");
  MRUFiles.push_back(L"filename5");

  StrList.Add(L"filename1");
  StrList.Add(L"filename2");
  StrList.Add(L"filename3");
  StrList.Add(L"filename4");
  StrList.Add(L"filename5");
}


TEST_F(RecentDBTest, RecentList)
{
  int is, ig;
  wxArrayString GotStrings;
  std::vector<stringT> MRURet;

  // Initial prep
  prefs->SetPref(PWSprefs::MaxMRUItems, 10);
  is = prefs->SetMRUList(MRUFiles, 10);
  EXPECT_EQ(is, 5);

  // Initialize RecentDbList after setting MaxMRUItems
  RecentDbList rdb;
  is = rdb.GetMaxFiles();
  EXPECT_EQ(is, 10);
  EXPECT_EQ(rdb.GetCount(), 0);

  // Load the list from PWSprefs and get the list of strings
  rdb.Load();
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 5);
  EXPECT_EQ(GotStrings.size(), 5);
  EXPECT_EQ(StrList, GotStrings);

  // Add a new name to the top of the list
  GotStrings.clear();
  StrList.Insert("NewFilename1", 0);

  rdb.AddFileToHistory("NewFilename1");
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 6);
  EXPECT_EQ(GotStrings.size(), 6);
  EXPECT_EQ(StrList, GotStrings);

  // Remove a name from the list
  GotStrings.clear();
  StrList.Remove("filename1");

  rdb.RemoveFile("filename1");
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 5);
  EXPECT_EQ(GotStrings.size(), 5);
  EXPECT_EQ(StrList, GotStrings);

  // Save the modified list to PWSprefs and check that it got there
  auto pos = MRUFiles.begin();
  auto pos2 = MRUFiles.insert(pos, L"NewFilename1");
  MRUFiles.erase(++pos2);

  rdb.Save();
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(ig, 5);
  EXPECT_EQ(MRUFiles, MRURet);
  
  // Clear the list
  GotStrings.clear();
  rdb.Clear();
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 0);
  EXPECT_EQ(GotStrings.size(), 0);

  rdb.Save();
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(ig, 0);
  EXPECT_EQ(MRURet.size(), 0);
}

TEST_F(RecentDBTest, RecentListLimit)
{
  int is;
  wxArrayString GotStrings;

  prefs->SetPref(PWSprefs::MaxMRUItems, 4);
  is = prefs->SetMRUList(MRUFiles, 10);
  EXPECT_EQ(is, 5);

  // Initialize after MaxMRUItems is set
  RecentDbList rdb;
  is = rdb.GetMaxFiles();
  EXPECT_EQ(is, 4);

  // Test limiting to 4
  StrList.Remove("filename5");
  GotStrings.clear();
  rdb.Load();
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 4);
  EXPECT_EQ(GotStrings.size(), 4);
  EXPECT_EQ(StrList, GotStrings);

  // Add to the top, remove from the bottom
  StrList.Insert("NewName1", 0);
  StrList.Remove("filename4");
  GotStrings.clear();

  rdb.AddFileToHistory("NewName1");
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 4);
  EXPECT_EQ(GotStrings.size(), 4);
  EXPECT_EQ(StrList, GotStrings);
}

TEST_F(RecentDBTest, RecentListLimit0)
{
  int is, ig;
  wxArrayString GotStrings;
  std::vector<stringT> MRURet;

  prefs->SetPref(PWSprefs::MaxMRUItems, 0);
  is = prefs->SetMRUList(MRUFiles, 10);
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(is, 5);
  EXPECT_EQ(ig, 0);

  // Initialize after MacMRUItems is set
  RecentDbList rdb;
  is = rdb.GetMaxFiles();
  EXPECT_EQ(is, 0);

  // Test limiting to 0
  GotStrings.clear();
  rdb.Load();
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 0);
  EXPECT_EQ(GotStrings.size(), 0);

  // Add should be sliently ignored
  GotStrings.clear();
  rdb.AddFileToHistory("NewName1");
  rdb.GetAll(GotStrings);
  EXPECT_EQ(rdb.GetCount(), 0);
  EXPECT_EQ(GotStrings.size(), 0);

  rdb.Save();
  ig = prefs->GetMRUList(MRURet);
  EXPECT_EQ(ig, 0);
  EXPECT_EQ(MRURet.size(), 0);
}
#endif //WIN32
