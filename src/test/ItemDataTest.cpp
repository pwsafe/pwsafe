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
  ItemDataEnv() { ::testing::AddGlobalTestEnvironment(this); }
  virtual void SetUp() { CItemData::SetSessionKey(); }
};

static ItemDataEnv env;

// And now the tests...

TEST(ItemDataTest, EmptyItems)
{
  CItemData di1, di2;
  EXPECT_TRUE(di1 == di2);
}


