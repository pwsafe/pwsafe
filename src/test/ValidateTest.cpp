/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ValidateTest.cpp --- Test the Validate function

#include "./gtest/gtest.h"
#include "../core/PWScore.h"
#include "../core/Report.h"
#include "../core/Validate.h"
#include "../core/Command.h"

// Test fixture class
class ValidateTest : public ::testing::Test, public PWScore {
protected:
  ValidateTest() {}

  virtual void SetUp() {
    // Set up test environment
    SetCurFile(L""); // Empty database
  }

  virtual void TearDown() {
    // Clean up test environment
  }

  CReport m_report;
  st_ValidateResults m_vr;
};

// Test empty title validation
TEST_F(ValidateTest, EmptyTitleTest) {
  // Create an entry with empty title
  CItemData item1;
  item1.CreateUUID();
  item1.SetGroup(L"TestGroup");
  item1.SetTitle(L"");  // Empty title
  item1.SetPassword(L"password123");
  item1.SetUser(L"testuser");

  AddEntryCommand *cmd1 = AddEntryCommand::Create(this, item1);
  Execute(cmd1);

  // Create an entry with non-empty title
  CItemData item2;
  item2.CreateUUID();
  item2.SetGroup(L"TestGroup");
  item2.SetTitle(L"ValidTitle");
  item2.SetPassword(L"password123");
  item2.SetUser(L"testuser");

  AddEntryCommand *cmd2 = AddEntryCommand::Create(this, item2);
  Execute(cmd2);

  // Run validation
  bool result = Validate(255, &m_report, m_vr);

  // Check results
  EXPECT_TRUE(result);  // true means data was modified
  EXPECT_EQ(m_vr.TotalIssues(), 1);  // Should have one issue
  EXPECT_EQ(m_vr.num_empty_titles, 1);  // Should have one empty title
}

// Test empty password validation
TEST_F(ValidateTest, EmptyPasswordTest) {
  // Create an entry with empty password
  CItemData item1;
  item1.CreateUUID();
  item1.SetGroup(L"TestGroup");
  item1.SetTitle(L"TestTitle");
  item1.SetPassword(L"");  // Empty password
  item1.SetUser(L"testuser");

  AddEntryCommand *cmd1 = AddEntryCommand::Create(this, item1);
  Execute(cmd1);

  // Create an entry with non-empty password
  CItemData item2;
  item2.CreateUUID();
  item2.SetGroup(L"TestGroup");
  item2.SetTitle(L"TestTitle2");
  item2.SetPassword(L"password123");
  item2.SetUser(L"testuser");

  AddEntryCommand *cmd2 = AddEntryCommand::Create(this, item2);
  Execute(cmd2);

  // Run validation
  bool result = Validate(255, &m_report, m_vr);

  // Check results
  EXPECT_TRUE(result);  // true means data was modified
  EXPECT_EQ(m_vr.TotalIssues(), 1);  // Should have one issue
  EXPECT_EQ(m_vr.num_empty_passwords, 1);  // Should have one empty password
}

// Test duplicate GTU validation
TEST_F(ValidateTest, DuplicateGTUTest) {
  // Create first entry
  CItemData item1;
  item1.CreateUUID();
  item1.SetGroup(L"TestGroup");
  item1.SetTitle(L"TestTitle");
  item1.SetPassword(L"password123");
  item1.SetUser(L"testuser");

  AddEntryCommand *cmd1 = AddEntryCommand::Create(this, item1);
  Execute(cmd1);

  // Create second entry with same GTU
  CItemData item2;
  item2.CreateUUID();
  item2.SetGroup(L"TestGroup");
  item2.SetTitle(L"TestTitle");
  item2.SetPassword(L"differentpassword");
  item2.SetUser(L"testuser");

  AddEntryCommand *cmd2 = AddEntryCommand::Create(this, item2);
  Execute(cmd2);

  // Run validation
  bool result = Validate(255, &m_report, m_vr);

  // Check results
  EXPECT_TRUE(result);  // true means data was modified
  EXPECT_EQ(m_vr.TotalIssues(), 1);  // Should have one issue
  EXPECT_EQ(m_vr.num_duplicate_GTU_fixed, 1);  // Should have one duplicate GTU
}

// Test text field size validation
TEST_F(ValidateTest, TextFieldSizeTest) {
  // Create an entry with oversized title
  CItemData item1;
  item1.CreateUUID();
  item1.SetGroup(L"TestGroup");
  // Create a string longer than 255 characters
  std::wstring longTitle(300, L'A');
  item1.SetTitle(longTitle.c_str());
  item1.SetPassword(L"password123");
  item1.SetUser(L"testuser");

  AddEntryCommand *cmd1 = AddEntryCommand::Create(this, item1);
  Execute(cmd1);

  // Run validation with max chars = 255
  bool result = Validate(255, &m_report, m_vr);

  // Check results
  EXPECT_TRUE(result);  // true means data was modified
  EXPECT_EQ(m_vr.TotalIssues(), 1);  // Should have one issue
  EXPECT_EQ(m_vr.num_excessivetxt_found, 1);  // Should have one oversized field
}

// Test attachment reference validation
TEST_F(ValidateTest, AttachmentRefTest) {
  // Create an entry with invalid attachment reference
  CItemData item1;
  item1.CreateUUID();
  item1.SetGroup(L"TestGroup");
  item1.SetTitle(L"TestTitle");
  item1.SetPassword(L"password123");
  item1.SetUser(L"testuser");
  
  // Set an attachment UUID that doesn't exist
  pws_os::CUUID bogusAttachmentUUID;
  item1.SetAttUUID(bogusAttachmentUUID);

  AddEntryCommand *cmd1 = AddEntryCommand::Create(this, item1);
  Execute(cmd1);

  // Run validation
  bool result = Validate(255, &m_report, m_vr);

  // Check results
  EXPECT_TRUE(result);  // true means data was modified
  EXPECT_EQ(m_vr.TotalIssues(), 1);  // Should have one issue
  EXPECT_EQ(m_vr.num_missing_att, 1);  // Should have one missing attachment
}


// Empty group checks don't affect the entries themselves.
// Testing them requires access to more private members of PWScore.
// For these reasons, we're not testing them, at least, for now.

// Test password history validation
TEST_F(ValidateTest, PasswordHistoryTest) {
  // Create an entry with invalid password history
  CItemData item1;
  item1.CreateUUID();
  item1.SetGroup(L"TestGroup");
  item1.SetTitle(L"TestTitle");
  item1.SetPassword(L"password123");
  item1.SetUser(L"testuser");
  
  // Set invalid password history
  // Format should be "1:2:3" where:
  // 1 = On/Off indicator (0 or 1)
  // 2 = Maximum number of entries
  // 3 = Current number of entries
  item1.SetPWHistory(L"InvalidHistory");  // Invalid format

  AddEntryCommand *cmd1 = AddEntryCommand::Create(this, item1);
  Execute(cmd1);

  // Run validation
  bool result = Validate(255, &m_report, m_vr);

  // Check results
  EXPECT_TRUE(result);  // true means data was modified
  EXPECT_EQ(m_vr.TotalIssues(), 1);  // Should have one issue
  EXPECT_EQ(m_vr.num_PWH_fixed, 1);  // Should have one password history issue
} 
