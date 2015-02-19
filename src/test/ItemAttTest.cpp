/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "gtest/gtest.h"

#include <vector>

using namespace std;

// A fixture for factoring common code across tests
class ItemAttTest : public ::testing::Test
{
protected:
  ItemAttTest(); // to init members
  CItemAtt emptyItem, fullItem;
  void SetUp();

  // members used to populate and test fullItem:
  const StringX title, mediaType, fileName;
  const time_t cTime;
  CItemAtt::key256T EK, AK;
  CItemAtt::contentHMACT ContentHMAC;
  vector<char> content;
};

ItemAttTest::ItemAttTest()
  : title(_T("a-title")), mediaType(_T("application/octet-stream")),
    fileName(_T("notafile.foo")), cTime(1409901293)
{}

void ItemAttTest::SetUp()
{
  fullItem.SetTitle(title);
}

// And now the tests...

TEST_F(ItemAttTest, EmptyItems)
{
  CItemAtt di2;
  const StringX t(L"title");
  EXPECT_TRUE(emptyItem == di2);
  emptyItem.SetTitle(t);
  EXPECT_FALSE(emptyItem == di2);  
  di2.SetTitle(t);
  EXPECT_TRUE(emptyItem == di2);
}

#if 0
TEST_F(ItemAttTest, CopyCtor)
{
  emptyItem.SetTitle(_T("title"));
  emptyItem.SetPassword(_T("password!"));
  CItemAtt di2(emptyItem);
  EXPECT_TRUE(emptyItem == di2);
}

TEST_F(ItemAttTest, Getters_n_Setters)
{
  // Setters called in SetUp()
  EXPECT_EQ(title, fullItem.GetTitle());
  EXPECT_EQ(password, fullItem.GetPassword());
  EXPECT_EQ(user, fullItem.GetUser());
  EXPECT_EQ(notes, fullItem.GetNotes());
  EXPECT_EQ(group, fullItem.GetGroup());
  EXPECT_EQ(url, fullItem.GetURL());
  EXPECT_EQ(at, fullItem.GetAutoType());
  EXPECT_EQ(email, fullItem.GetEmail());
  EXPECT_EQ(polname, fullItem.GetPolicyName());
  EXPECT_EQ(symbols, fullItem.GetSymbols());
  EXPECT_EQ(runcmd, fullItem.GetRunCommand());
  EXPECT_EQ(aTime, fullItem.GetATime(tVal));
  EXPECT_EQ(cTime, fullItem.GetCTime(tVal));
  EXPECT_EQ(xTime, fullItem.GetXTime(tVal));
  EXPECT_EQ(pmTime, fullItem.GetPMTime(tVal));
  EXPECT_EQ(rmTime, fullItem.GetRMTime(tVal));
  EXPECT_EQ(iDCA, fullItem.GetDCA(iVal16));
  EXPECT_EQ(iSDCA, fullItem.GetShiftDCA(iVal16));
  EXPECT_EQ(kbs, fullItem.GetKBShortcut(iVal32));
}

TEST_F(ItemAttTest, PlainTextSerialization)
{
  std::vector<char> v;
  emptyItem.SerializePlainText(v);
  CItemAtt di;
  EXPECT_TRUE(di.DeSerializePlainText(v));
  EXPECT_EQ(emptyItem, di);

  fullItem.SerializePlainText(v);
  EXPECT_TRUE(di.DeSerializePlainText(v));
  EXPECT_EQ(fullItem, di);
}

TEST_F(ItemAttTest, UnknownFields)
{
  unsigned char u1t = 100, u2t = 200, u3t = 100;
  unsigned char u1v[] = {10, 11, 33, 57};
  unsigned char u2v[] = {92, 77, 76, 40, 65, 66};
  unsigned char u3v[] = {1};

  ASSERT_EQ(0, emptyItem.NumberUnknownFields());
  emptyItem.SetUnknownField(u1t, sizeof(u1v), u1v);
  emptyItem.SetUnknownField(u2t, sizeof(u2v), u2v);
  emptyItem.SetUnknownField(u3t, sizeof(u3v), u3v);
  EXPECT_EQ(3, emptyItem.NumberUnknownFields());

  // Getting Unknown Fields is done by private
  // member functions, which make sense considering
  // how they're processed. Worth exposing an API
  // just for testing, TBD.
}
#endif
