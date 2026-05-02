/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AuxParseTest.cpp: Unit test for Run command and autotype string parsing

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include <sstream>

#include "core/PWSAuxParse.h"
#include "core/ItemData.h"
#include "core/PWScore.h"
#include "gtest/gtest.h"


TEST(AuxParseTest, testGetEffectiveValuesNoBase)
{
  StringX lastpswd, totpauthcode;

  CItemData item, effitem;
  // null test, all empty values
  PWSAuxParse::GetEffectiveValues(&item, nullptr, effitem, lastpswd, totpauthcode);
  EXPECT_EQ(effitem.GetGroup(), item.GetGroup());
  EXPECT_EQ(effitem.GetTitle(), item.GetTitle());
  EXPECT_EQ(effitem.GetUser(), item.GetUser());
  EXPECT_EQ(effitem.GetPassword(), item.GetPassword());
  EXPECT_TRUE(lastpswd.empty());
  EXPECT_EQ(effitem.GetNotes(), item.GetNotes());
  EXPECT_EQ(effitem.GetURL(), item.GetURL());
  EXPECT_EQ(effitem.GetEmail(), item.GetEmail());
  EXPECT_EQ(effitem.GetAutoType(), item.GetAutoType());
  EXPECT_EQ(effitem.GetRunCommand(), item.GetRunCommand());
  EXPECT_TRUE(totpauthcode.empty());

  EXPECT_EQ(effitem, item);

  // populate fields
  item.SetGroup(L"goldfish");
  item.SetTitle(L"tuna");
  item.SetUser(L"uncombed blenny");
  item.SetPassword(L"pirahna");
  item.SetNotes(L"northern hog sucker");
  item.SetURL(L"unicorn icefish");
  item.SetEmail(L"eel");
  item.SetAutoType(L"aden splitfin");
  item.SetRunCommand(L"red bellied dace");
  CustomField pin;
  pin.SetName(L"PIN");
  pin.SetValue(L"1234");
  item.AddCustomField(pin);

  PWSAuxParse::GetEffectiveValues(&item, nullptr, effitem, lastpswd, totpauthcode);

  EXPECT_EQ(effitem.GetGroup(), item.GetGroup());
  EXPECT_EQ(effitem.GetTitle(), item.GetTitle());
  EXPECT_EQ(effitem.GetUser(), item.GetUser());
  EXPECT_EQ(effitem.GetPassword(), item.GetPassword());
  EXPECT_TRUE(lastpswd.empty());
  EXPECT_EQ(effitem.GetNotes(), item.GetNotes());
  EXPECT_EQ(effitem.GetURL(), item.GetURL());
  EXPECT_EQ(effitem.GetEmail(), item.GetEmail());
  EXPECT_EQ(effitem.GetAutoType(), item.GetAutoType());
  EXPECT_EQ(effitem.GetRunCommand(), item.GetRunCommand());
  EXPECT_EQ(effitem.GetCustomFieldsRaw(), item.GetCustomFieldsRaw());
  EXPECT_TRUE(totpauthcode.empty());

  EXPECT_EQ(effitem, item);
}

TEST(AuxParseTest, testGetExpandedString)
{
  StringX runCmd = L"runme simple";
  StringX currentDB, autoTypeStr;
  stringT errmsg;
  StringX::size_type column;
  bool autoTypeBool(false), urlSpecial(false);
  CItemData item;


  StringX exString = PWSAuxParse::GetExpandedString(runCmd, currentDB,
                                                    &item, nullptr,
                                                    autoTypeBool,  autoTypeStr,
                                                    errmsg, column, urlSpecial);
  EXPECT_TRUE(errmsg.empty());
  EXPECT_EQ(L"runme simple", exString);

  
  runCmd = L"runme \\\\$u";
  item.SetUser(L"Joe");
  exString = PWSAuxParse::GetExpandedString(runCmd, currentDB,
                                            &item, nullptr,
                                            autoTypeBool,  autoTypeStr,
                                            errmsg, column, urlSpecial);
  EXPECT_TRUE(errmsg.empty());
  EXPECT_EQ(L"runme \\Joe", exString);
}

TEST(AuxParseTest, testGetAutoTypeStringCustomFieldExpansion)
{
  std::vector<size_t> vactionverboffsets;
  CustomFieldList customFields;

  CustomField pin;
  pin.SetName(L"PIN");
  pin.SetValue(L"12\\34");
  pin.SetSensitive(true);
  customFields.push_back(pin);

  const StringX expanded = PWSAuxParse::GetAutoTypeString(
      L"prefix \\v{PIN} suffix \\v{Missing}",
      L"", L"", L"", L"", L"", L"", L"", L"", L"",
      &customFields, vactionverboffsets);

  EXPECT_EQ(L"prefix 12\\\\34 suffix ", expanded);
}

TEST(AuxParseTest, testGetAutoTypeStringUsesItemCustomFields)
{
  PWScore core;
  CItemData item;
  item.SetAutoType(L"\\v{PIN}");

  CustomField pin;
  pin.SetName(L"PIN");
  pin.SetValue(L"1234");
  pin.SetSensitive(true);
  ASSERT_TRUE(item.AddCustomField(pin));

  std::vector<size_t> vactionverboffsets;
  const StringX expanded = PWSAuxParse::GetAutoTypeString(item, core, vactionverboffsets);

  EXPECT_EQ(L"1234", expanded);
}

TEST(AuxParseTest, testCustomFieldsEdgeCases)
{
  PWScore core;
  CItemData item;
  item.SetAutoType(L"\\v{} and/or \\v");

  CustomField pin;
  pin.SetName(L"PIN");
  pin.SetValue(L"1234");
  pin.SetSensitive(true);
  ASSERT_TRUE(item.AddCustomField(pin));

  std::vector<size_t> vactionverboffsets;
  const StringX expanded = PWSAuxParse::GetAutoTypeString(item, core, vactionverboffsets);

  EXPECT_EQ(L" and/or \\v", expanded);
}
