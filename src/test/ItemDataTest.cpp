/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
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
#include "core/PWSprefs.h"
#include "core/PWHistory.h"
#include "gtest/gtest.h"

// A fixture for factoring common code across tests
class ItemDataTest : public ::testing::Test
{
protected:
  ItemDataTest(); // to init members
  CItemData emptyItem, fullItem;
  void SetUp();

  // members used to populate and test fullItem:
  const StringX title, password, user, notes, group;
  const StringX url, at, email, polname, symbols, runcmd;
  const time_t aTime, cTime, xTime, pmTime, rmTime;
  const int16 iDCA, iSDCA;
  const int32 xTimeInt, kbs;
  time_t tVal;
  int16 iVal16;
  int32 iVal32;
};

ItemDataTest::ItemDataTest()
  : title(_T("a-title")), password(_T("b-password!?")),
    user(_T("C-UserR-ינור")), // non-English
    notes(_T("N is for notes\nwhich can span lines\r\nin several ways.")),
    group(_T("Groups.are.nested.by.dots")), url(_T("http://pwsafe.org/")),
    at(_T("\\u\\t\\t\\n\\p\\t\\n")), email(_T("joe@spammenot.com")),
    polname(_T("liberal")), symbols(_T("<-_+=@?>")), runcmd(_T("Run 4 your life")),
    aTime(1409901292), // time test was first added, from http://www.unixtimestamp.com/
    cTime(1409901293), xTime(1409901294), pmTime(1409901295), rmTime(1409901296),
    iDCA(3), iSDCA(8), xTimeInt(42), kbs(0x12345678),
    tVal(0), iVal16(-1), iVal32(-1)
{}

void ItemDataTest::SetUp()
{
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
  fullItem.SetXTimeInt(xTimeInt);
  fullItem.SetKBShortcut(kbs);
}

// And now the tests...

TEST_F(ItemDataTest, EmptyItems)
{
  CItemData di2;
  const StringX t(L"title");
  EXPECT_TRUE(emptyItem == di2);

  emptyItem.SetTitle(t);
  EXPECT_FALSE(emptyItem == di2);  

  di2.SetTitle(t);
  EXPECT_TRUE(emptyItem == di2);
}

TEST_F(ItemDataTest, CopyCtor)
{
  emptyItem.SetTitle(_T("title"));
  emptyItem.SetPassword(_T("password!"));

  CItemData di2(emptyItem);
  EXPECT_TRUE(emptyItem == di2);
}

TEST_F(ItemDataTest, Assignment)
{
  CItemData d1;
  CItemData d2;
  d1.SetTitle(_T("title"));
  d1.SetPassword(_T("password!"));
  d2.SetTitle(_T("eltit"));
  d2.SetPassword(_T("!drowssap"));
  d2 = d1;
  EXPECT_TRUE(d1 == d2);  
}

TEST_F(ItemDataTest, Getters_n_Setters)
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
  EXPECT_EQ(xTimeInt, fullItem.GetXTimeInt(iVal32));
  EXPECT_EQ(kbs, fullItem.GetKBShortcut(iVal32));
  EXPECT_FALSE(fullItem.IsProtected()); // default value

  fullItem.SetProtected(true); // modify
  EXPECT_TRUE(fullItem.IsProtected());

  fullItem.SetProtected(false); // modify back to default
  EXPECT_FALSE(fullItem.IsProtected());
}

TEST_F(ItemDataTest, PlainTextSerialization)
{
  std::vector<char> v;
  emptyItem.SerializePlainText(v);
  CItemData di;
  EXPECT_TRUE(di.DeSerializePlainText(v));
  EXPECT_EQ(emptyItem, di);

  fullItem.SerializePlainText(v);
  EXPECT_TRUE(di.DeSerializePlainText(v));
  EXPECT_EQ(fullItem, di);
}

TEST_F(ItemDataTest, PasswordHistory)
{
  const StringX pw1(L"banana-0rchid");
  const StringX pw2(L"banana-1rchid");
  const StringX pw3(L"banana-2rchid");
  const StringX pw4(L"banana-5rchid");
  const StringX emptyHeader(L"00000");
  const StringX last1Header(L"10303");
  const StringX alteredHeader(L"00c02");

  PWSprefs *prefs = PWSprefs::GetInstance();
  prefs->SetPref(PWSprefs::SavePasswordHistory, true);
  prefs->SetPref(PWSprefs::NumPWHistoryDefault, 3);

  CItemData di;
  di.SetCTime();
  di.SetPassword(pw1); // first time must be Set, not Update!
  di.UpdatePassword(pw2);
  EXPECT_FALSE(di.GetPWHistory().empty());

  {
    PWHistList pwhl(di.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(1U, pwhl.size());
    EXPECT_EQ(pw1, pwhl[0].password);
    EXPECT_EQ(di.GetPWHistory(), (StringX)pwhl);
  }

  di.UpdatePassword(pw3);

  {
    PWHistList pwhl(di.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(2U, pwhl.size());
    EXPECT_EQ(pw1, pwhl[0].password);
    EXPECT_EQ(pw2, pwhl[1].password);
    EXPECT_EQ(di.GetPWHistory(), (StringX)pwhl);
  }

  di.UpdatePassword(pw4);

  {
    PWHistList pwhl(di.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(3U, pwhl.size());
    EXPECT_EQ(pw1, pwhl[0].password);
    EXPECT_EQ(pw2, pwhl[1].password);
    EXPECT_EQ(pw3, pwhl[2].password);
    EXPECT_EQ(di.GetPWHistory(), (StringX)pwhl);
  }

  di.UpdatePassword(L"Last1");

  {
    PWHistList pwhl(di.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(3U, pwhl.size());
    EXPECT_EQ(pw2, pwhl[0].password);
    EXPECT_EQ(pw3, pwhl[1].password);
    EXPECT_EQ(pw4, pwhl[2].password);
    EXPECT_EQ(di.GetPWHistory(), (StringX)pwhl);

    EXPECT_EQ(last1Header, pwhl.MakePWHistoryHeader());
    EXPECT_EQ(pw4, PWHistList::GetPreviousPassword(di.GetPWHistory()));

    // Reduce the max and make sure the oldest is removed
    pwhl.setMax(2);
    PWHistList pwh2(pwhl, PWSUtil::TMC_ASC_UNKNOWN);
    EXPECT_TRUE(pwh2.isSaving());
    EXPECT_EQ(0U, pwh2.getErr());
    EXPECT_EQ(2U, pwh2.getMax());
    EXPECT_EQ(2U, pwh2.size());
    EXPECT_EQ(pw3, pwh2[0].password);
    EXPECT_EQ(pw4, pwh2[1].password);

    pwhl.setMax(12);
    pwhl.setSaving(false);
    EXPECT_FALSE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(12U, pwhl.getMax());
    EXPECT_EQ(2U, pwhl.size());
    EXPECT_EQ(alteredHeader, pwhl.MakePWHistoryHeader());
    EXPECT_EQ(pw4, PWHistList::GetPreviousPassword(pwhl));
  }

  {
    // Test the copy constructor
    PWHistList pwh_first(di.GetPWHistory(), PWSUtil::TMC_ASC_UNKNOWN);
    PWHistList pwhl = pwh_first;
    EXPECT_TRUE(pwhl.isSaving());
    EXPECT_EQ(0U, pwhl.getErr());
    EXPECT_EQ(3U, pwhl.getMax());
    EXPECT_EQ(3U, pwhl.size());
    EXPECT_EQ(pw2, pwhl[0].password);
    EXPECT_EQ(pw3, pwhl[1].password);
    EXPECT_EQ(pw4, pwhl[2].password);
    EXPECT_EQ(di.GetPWHistory(), (StringX)pwhl);
  }

  {
    // Test the default constructor
    PWHistList pwh;
    EXPECT_EQ(emptyHeader, pwh.MakePWHistoryHeader());
  }

  EXPECT_EQ(emptyHeader, PWHistList::MakePWHistoryHeader(false, 0));
  EXPECT_EQ(emptyHeader, PWHistList::MakePWHistoryHeader(false, 0, 0));
}

TEST_F(ItemDataTest, CustomFields)
{
  const StringX raw = _T("010004Name020005Value0300012")
                      _T("000000")
                      _T("010004Wifi020004Test040003abc")
                      _T("000000")
                      _T("010006000000020006000000");
  const StringX normalized = _T("010004Name020005Value0300011")
                             _T("000000")
                             _T("010004Wifi020004Test040003abc")
                             _T("000000")
                             _T("010006000000020006000000");

  CustomFieldList custom_fields(raw);
  EXPECT_EQ(0U, custom_fields.getErr());
  EXPECT_EQ(normalized, static_cast<StringX>(custom_fields));
  ASSERT_EQ(3U, custom_fields.size());

  EXPECT_EQ(_T("Name"), custom_fields[0].GetName());
  EXPECT_EQ(_T("Value"), custom_fields[0].GetValue());
  EXPECT_TRUE(custom_fields[0].IsSensitive());

  bool found_unknown = false;
  StringX unknown_value;
  for (const auto &prop : custom_fields[1].GetProperties()) {
    if (prop.first == 0x04) {
      found_unknown = true;
      unknown_value = prop.second;
      break;
    }
  }
  EXPECT_TRUE(found_unknown);
  EXPECT_EQ(_T("abc"), unknown_value);

  EXPECT_EQ(_T("000000"), custom_fields[2].GetName());
  EXPECT_EQ(_T("000000"), custom_fields[2].GetValue());

  CItemData di;
  di.SetCustomFields(custom_fields);
  EXPECT_TRUE(di.IsCustomFieldsSet());
  EXPECT_EQ(normalized, di.GetCustomFieldsRaw());

  CustomFieldList parsed = di.GetCustomFields();
  EXPECT_EQ(0U, parsed.getErr());
  EXPECT_EQ(normalized, static_cast<StringX>(parsed));
}

TEST_F(ItemDataTest, CustomFieldsOps)
{
  CItemData di;

  CustomField cf1;
  cf1.SetName(_T("A"));
  cf1.SetValue(_T("1"));
  EXPECT_TRUE(di.AddCustomField(cf1));

  CustomField dup;
  dup.SetName(_T("A"));
  dup.SetValue(_T("2"));
  EXPECT_FALSE(di.AddCustomField(dup));

  CustomField cf2;
  cf2.SetName(_T("B"));
  cf2.SetValue(_T("2"));
  EXPECT_TRUE(di.AddCustomField(cf2));

  CustomField edit_b;
  edit_b.SetName(_T("B"));
  edit_b.SetValue(_T("3"));
  EXPECT_TRUE(di.EditCustomField(_T("B"), edit_b));

  CustomField rename_dup;
  rename_dup.SetName(_T("A"));
  rename_dup.SetValue(_T("9"));
  EXPECT_FALSE(di.EditCustomField(_T("B"), rename_dup));

  EXPECT_TRUE(di.SetCustomFieldProperty(_T("B"), CustomField::PROP_NAME, _T("C")));
  EXPECT_FALSE(di.SetCustomFieldProperty(_T("C"), CustomField::PROP_NAME, _T("A")));

  EXPECT_FALSE(di.SetCustomFieldProperty(_T("A"), CustomField::PROP_SENSITIVE, _T("10")));
  EXPECT_TRUE(di.SetCustomFieldProperty(_T("A"), CustomField::PROP_SENSITIVE, _T("1")));

  EXPECT_TRUE(di.DeleteCustomField(_T("C")));
  EXPECT_FALSE(di.DeleteCustomField(_T("C")));

  di.SetFieldValue(CItemData::CUSTOMTEXT, _T("010001A020001B03000210"));
  EXPECT_FALSE(di.AddCustomField(cf1));
}

TEST_F(ItemDataTest, UnknownFields)
{
  unsigned char u1v[] = {10, 11, 33, 57};
  unsigned char u2v[] = {92, 77, 76, 40, 65, 66};
  unsigned char u3v[] = {1};

  CItemData d1, d2;

  ASSERT_EQ(0U, d1.NumberUnknownFields());
  d1.SetUnknownField(CItemData::UNKNOWN_TESTING, sizeof(u1v), u1v);
  d1.SetUnknownField(CItemData::UNKNOWN_TESTING, sizeof(u2v), u2v);
  d1.SetUnknownField(CItemData::UNKNOWN_TESTING, sizeof(u3v), u3v);
  EXPECT_EQ(3U, d1.NumberUnknownFields());

  EXPECT_NE(d1, d2);
  d2 = d1;
  EXPECT_EQ(d1, d2);
  CItemData d3(d1);
  EXPECT_EQ(d1, d3);

  // Getting Unknown Fields is done by private
  // member functions, which make sense considering
  // how they're processed. Worth exposing an API
  // just for testing, TBD.
}
