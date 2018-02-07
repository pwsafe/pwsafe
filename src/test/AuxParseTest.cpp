/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "gtest/gtest.h"


TEST(AuxParseTest, testGetEffectiveValuesNoBase)
{
  StringX group, title, user, pswd, lastpswd, notes, url,
    email, autotype, runcmd;

  CItemData item;
  // null test, all empty values
  bool status = PWSAuxParse::GetEffectiveValues(&item, nullptr,
                                                group, title, user, pswd,
                                                lastpswd, notes, url,
                                                email, autotype, runcmd);
  EXPECT_TRUE(status);
  EXPECT_EQ(group, item.GetGroup());
  EXPECT_EQ(title, item.GetTitle());
  EXPECT_EQ(user, item.GetUser());
  EXPECT_EQ(pswd, item.GetPassword());
  EXPECT_TRUE(lastpswd.empty());
  EXPECT_EQ(notes, item.GetNotes());
  EXPECT_EQ(url, item.GetURL());
  EXPECT_EQ(email, item.GetEmail());
  EXPECT_EQ(autotype, item.GetAutoType());
  EXPECT_EQ(runcmd, item.GetRunCommand());

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
  status = PWSAuxParse::GetEffectiveValues(&item, nullptr,
                                           group, title, user, pswd,
                                           lastpswd, notes, url,
                                           email, autotype, runcmd);
  EXPECT_TRUE(status);
  EXPECT_EQ(group, item.GetGroup());
  EXPECT_EQ(title, item.GetTitle());
  EXPECT_EQ(user, item.GetUser());
  EXPECT_EQ(pswd, item.GetPassword());
  EXPECT_TRUE(lastpswd.empty());
  EXPECT_EQ(notes, item.GetNotes());
  EXPECT_EQ(url, item.GetURL());
  EXPECT_EQ(email, item.GetEmail());
  EXPECT_EQ(autotype, item.GetAutoType());
  EXPECT_EQ(runcmd, item.GetRunCommand());
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
