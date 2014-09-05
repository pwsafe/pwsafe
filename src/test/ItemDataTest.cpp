/*
* Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemDataTest.cpp: Unit test for CItemData class

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/ItemData.h"
#include "gtest/gtest.h"

// We need to call CItemData::SetSessionKey() exactly once.
// That's what the following is for:

class ItemDataEnv : public ::testing::Environment
{
public:
  ItemDataEnv() { }
  virtual void SetUp() { CItemData::SetSessionKey(); }
};

static ::testing::Environment *const env = ::testing::AddGlobalTestEnvironment(new ItemDataEnv);

// And now the tests...

TEST(ItemDataTest, EmptyItems)
{
  CItemData di1, di2;
  const StringX t(L"title");
  EXPECT_TRUE(di1 == di2);
  di1.SetTitle(t);
  EXPECT_FALSE(di1 == di2);  
  di2.SetTitle(t);
  EXPECT_TRUE(di1 == di2);
}

TEST(ItemDataTest, CopyCtor)
{
  CItemData di1;
  di1.SetTitle(StringX(_T("title")));
  di1.SetPassword(_T("password!"));
  CItemData di2(di1);
  EXPECT_TRUE(di1 == di2);
}

TEST(ItemDataTest, Getters_n_Setters)
{
  CItemData di;
  const StringX title(_T("a-title"));
  const StringX password(_T("b-password!?"));
  const StringX user(_T("C-UserR-ינור")); // non-English
  const StringX notes(_T("N is for notes\nwhich can span lines\r\nin several ways."));
  const StringX group(_T("Groups.are.nested.by.dots"));
  const StringX url(_T("http://pwsafe.org/"));
  const StringX at(_T("\\u\\t\\t\\n\\p\\t\\n"));
  const StringX email(_T("joe@spammenot.com"));
  const StringX polname(_T("liberal"));
  const StringX symbols(_T("<-_+=@?>"));
  const time_t aTime = 1409901292; // time test was first added, from http://www.unixtimestamp.com/
  const time_t cTime = 1409901293;
  const time_t xTime = 1409901294;
  const time_t pmTime = 1409901295;
  const time_t rmTime = 1409901296;
  time_t tVal(0);
  const int16 iDCA = 3;
  const int16 iSDCA = 8;
  const int32 kbs = 0x12345678;
  int16 iVal16(-1);
  int32 iVal32(-1);

  di.SetTitle(title);
  di.SetPassword(password);
  di.SetUser(user);
  di.SetNotes(notes);
  di.SetGroup(group);
  di.SetURL(url);
  di.SetAutoType(at);
  di.SetEmail(email);
  di.SetPolicyName(polname);
  di.SetSymbols(symbols);
  di.SetATime(aTime);
  di.SetCTime(cTime);
  di.SetXTime(xTime);
  di.SetPMTime(pmTime);
  di.SetRMTime(rmTime);
  di.SetDCA(iDCA);
  di.SetShiftDCA(iSDCA);
  di.SetKBShortcut(kbs);

  EXPECT_EQ(title, di.GetTitle());
  EXPECT_EQ(password, di.GetPassword());
  EXPECT_EQ(user, di.GetUser());
  EXPECT_EQ(notes, di.GetNotes());
  EXPECT_EQ(group, di.GetGroup());
  EXPECT_EQ(url, di.GetURL());
  EXPECT_EQ(at, di.GetAutoType());
  EXPECT_EQ(email, di.GetEmail());
  EXPECT_EQ(polname, di.GetPolicyName());
  EXPECT_EQ(symbols, di.GetSymbols());
  EXPECT_EQ(aTime, di.GetATime(tVal));
  EXPECT_EQ(cTime, di.GetCTime(tVal));
  EXPECT_EQ(xTime, di.GetXTime(tVal));
  EXPECT_EQ(pmTime, di.GetPMTime(tVal));
  EXPECT_EQ(rmTime, di.GetRMTime(tVal));
  EXPECT_EQ(iDCA, di.GetDCA(iVal16));
  EXPECT_EQ(iSDCA, di.GetShiftDCA(iVal16));
  EXPECT_EQ(kbs, di.GetKBShortcut(iVal32));
}
