/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
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
    const stringT testFile2 = L"import-text-unit-test2.csv";
    const stringT testFile3 = L"import-text-unit-test3.csv";
    const stringT testFile4 = L"import-text-unit-test4.csv";
    const stringT testFile5 = L"import-text-unit-test5.csv";


  
  void SetUp();
  void TearDown();

  int numImported, numSkipped, numPWHErrors, numRenamed, numWarnings, numNoPolicy;

  void importText(const stringT& fname, const TCHAR fieldSeparator, int expectedImports);
  void testImport(); // for testFiles 1, 2, and 3
};

ImportTextTest::ImportTextTest()
  : numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numWarnings(0), numNoPolicy(0)
{
  
}

void ImportTextTest::SetUp()
{
  ASSERT_TRUE(pws_os::chdir(L"data"));
  ASSERT_TRUE(pws_os::FileExists(testFile1));
  ASSERT_TRUE(pws_os::FileExists(testFile2));
  ASSERT_TRUE(pws_os::FileExists(testFile3));
  ASSERT_TRUE(pws_os::FileExists(testFile4));
  ASSERT_TRUE(pws_os::FileExists(testFile5));
}

void ImportTextTest::TearDown()
{
  ASSERT_TRUE(pws_os::chdir(L".."));
}

void ImportTextTest::importText(const stringT& fname, const TCHAR fieldSeparator, int expectedImports)
{
  stringT errorStr;
  CReport rpt;
  Command* cmd(nullptr);

  numImported = numSkipped = numPWHErrors = numRenamed = numWarnings = numNoPolicy = 0;
  core.ReInit();

  int status = core.ImportPlaintextFile(L"", fname.c_str(), fieldSeparator,
    L'\xbb', false,
    errorStr,
    numImported, numSkipped,
    numPWHErrors, numRenamed,
    numNoPolicy,
    rpt, cmd);

  ASSERT_EQ(status, PWScore::SUCCESS);
  EXPECT_EQ(numImported, expectedImports);
  EXPECT_EQ(numSkipped, 0);
  EXPECT_EQ(numPWHErrors, 0);
  EXPECT_EQ(numRenamed, 0);
  EXPECT_EQ(numNoPolicy, 0);
  EXPECT_TRUE(errorStr.empty());
  EXPECT_NE(cmd, nullptr);

  status = core.Execute(cmd);
  EXPECT_EQ(status, 0);
  delete cmd;

  //std::cout << rpt.GetString().c_str();
}

void ImportTextTest::testImport()
{
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


TEST_F(ImportTextTest, test1)
{
  // this test is of a file that was created directly via the text export function.

  importText(testFile1, L'\t', 2);
  testImport();
}

TEST_F(ImportTextTest, test2)
{
  // same file as test2, except:
  // - Group/Title replaced with separate Group and Title columns
  // - some columns were renamed to equivalent (lowercase or accepted synonyms)

  importText(testFile2, L',', 2);
  testImport();
}

TEST_F(ImportTextTest, test3)
{
  // same file as test2, except:
  // - Group/Title replaced with separate Group and Title columns
  // - some columns were renamed to equivalent (lowercase or accepted synonyms)

  importText(testFile3, L',', 2);
  testImport();
}

TEST_F(ImportTextTest, test4)
{
  // csv file created by LastPass export

  importText(testFile4, L',', 1);

  // now test that we've read the data correctly
  auto p1 = core.Find(L"folder", L"jojo-name", L"jojo-username");
  EXPECT_NE(p1, core.GetEntryEndIter());
  auto item1 = core.GetEntry(p1);
  EXPECT_EQ(item1.GetPassword(), L"site-password");
  EXPECT_EQ(item1.GetNotes(), L"notes-field");
  EXPECT_EQ(item1.GetURL(), L"http://biteme.com");
}

TEST_F(ImportTextTest, test5)
{
  // csv file created by LastPass export, with multiline note, backslash subgroup separator.

  importText(testFile5, L',', 1);

  // now test that we've read the data correctly
  auto p1 = core.Find(L"acme-folder.subfolder1", L"acme-name", L"acme-username");
  EXPECT_NE(p1, core.GetEntryEndIter());
  auto item1 = core.GetEntry(p1);
  EXPECT_EQ(item1.GetGroup(), L"acme-folder.subfolder1");
  EXPECT_EQ(item1.GetTitle(), L"acme-name");
  EXPECT_EQ(item1.GetUser(), L"acme-username");
  EXPECT_EQ(item1.GetPassword(), L"acme-password");
  EXPECT_EQ(item1.GetNotes(),
    L"how are multiline notes exported?\r\n"
    L"only time will tell.\r\n"
    L"marked as favorite!");
  EXPECT_EQ(item1.GetURL(), L"http://acme.org");
}

class SelectedTotpFieldsTest : public ImportTextTest,
                             public ::testing::WithParamInterface<unsigned int>
{
};

TEST_P(SelectedTotpFieldsTest, export_import_roundtrip)
{
  const stringT exportFile = _T("import-text-unit-test-selected-totp.txt");
  pws_os::DeleteAFile(exportFile);

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("accounts"));
  ci.SetTitle(_T("totp"));
  ci.SetUser(_T("user"));
  ci.SetPassword(_T("password"));
  ci.SetTwoFactorKey(_T("JBSWY3DPEHPK3PXP"));
  ASSERT_TRUE(ci.SetTotpConfig(_T("1")));
  ci.SetTotpStartTime(time_t(946684800));
  ASSERT_TRUE(ci.SetTotpTimeStep(_T("60")));
  ASSERT_TRUE(ci.SetTotpLength(_T("8")));
  ci.SetURL(_T("https://example.com"));
  ci.SetAutoType(_T("\\u\\t\\p\\n"));
  ci.SetNotes(_T("Notes after the TOTP fields"));

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  exportCore.ClearCommands();

  const CItemData::FieldType optionalFields[] = {
    CItemData::TOTPCONFIG, CItemData::TOTPSTARTTIME,
    CItemData::TOTPTIMESTEP, CItemData::TOTPLENGTH,
    CItemData::URL, CItemData::AUTOTYPE
  };
  const CItemData defaults;

  const CItemData::FieldType selectedFields[] = {
    CItemData::GROUP, CItemData::TITLE, CItemData::USER,
    CItemData::PASSWORD, CItemData::TWOFACTORKEY, CItemData::NOTES
  };
  CItemData::FieldBits bsFields;
  for (const auto field : selectedFields)
    bsFields.set(field);
  for (size_t i = 0; i < NumberOf(optionalFields); ++i)
    bsFields.set(optionalFields[i], (GetParam() & (1U << i)) != 0);

  int numExported(0);
  CReport rpt;
  ASSERT_EQ(exportCore.WritePlaintextFile(StringX(exportFile.c_str()), bsFields,
                                        _T(""), 0, 0, TCHAR('\xbb'),
                                        numExported, nullptr, &rpt),
            PWScore::SUCCESS);
  EXPECT_EQ(numExported, 1);

  importText(exportFile, TCHAR('\t'), 1);
  auto iter = core.Find(ci.GetGroup(), ci.GetTitle(), ci.GetUser());
  ASSERT_NE(iter, core.GetEntryEndIter());
  const auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetPassword(), ci.GetPassword());
  EXPECT_EQ(item.GetTwoFactorKey(), ci.GetTwoFactorKey());
  for (const auto field : optionalFields) {
    EXPECT_EQ(item.GetFieldValue(field),
              bsFields.test(field) ? ci.GetFieldValue(field) : defaults.GetFieldValue(field));
  }
  EXPECT_EQ(item.GetNotes(), ci.GetNotes());

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

// Exercise every selection of the TOTP parameters, URL, and AutoType.
INSTANTIATE_TEST_SUITE_P(SelectedFields, SelectedTotpFieldsTest,
                        ::testing::Range(0U, 64U));

TEST_F(ImportTextTest, export_import_custom_fields_roundtrip)
{
  const stringT exportFile = _T("import-text-unit-test-customfields.txt");
  const StringX sxExportFile(exportFile.c_str());
  const StringX rawFields = _T("010004Name020005Value0300012")
                            _T("000000")
                            _T("010004Wifi020004Test040003abc");

  pws_os::DeleteAFile(exportFile);

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("network"));
  ci.SetTitle(_T("router"));
  ci.SetUser(_T("admin"));
  ci.SetPassword(_T("secret"));
  ci.SetFieldValue(CItemData::CUSTOMTEXT, rawFields);

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  CItemData::FieldBits bsFields;
  bsFields.set();
  int numExported(0);
  CReport rpt;
  ASSERT_EQ(exportCore.WritePlaintextFile(sxExportFile, bsFields, _T(""), 0, 0,
                                          TCHAR('\xbb'), numExported, nullptr, &rpt),
            PWScore::SUCCESS);
  EXPECT_EQ(numExported, 1);
  ASSERT_TRUE(pws_os::FileExists(exportFile));

  importText(exportFile, TCHAR('\t'), 1);

  auto iter = core.Find(_T("network"), _T("router"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetCustomFieldsRaw(), rawFields);

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

TEST_F(ImportTextTest, export_import_custom_fields_special_chars_roundtrip)
{
  const stringT exportFile = _T("import-text-unit-test-customfields-special.txt");
  const StringX sxExportFile(exportFile.c_str());

  pws_os::DeleteAFile(exportFile);

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("network"));
  ci.SetTitle(_T("router-special"));
  ci.SetUser(_T("admin"));
  ci.SetPassword(_T("secret"));

  CustomFieldList fields;
  CustomField cf;
  cf.SetName(_T("line\tname"));
  cf.SetValue(_T("slash\\ quote\" semi; row1\r\nrow2"));
  fields.push_back(cf);
  ci.SetCustomFields(fields);

  const StringX rawFields = ci.GetCustomFieldsRaw();

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  CItemData::FieldBits bsFields;
  bsFields.set();
  int numExported(0);
  CReport rpt;
  ASSERT_EQ(exportCore.WritePlaintextFile(sxExportFile, bsFields, _T(""), 0, 0,
                                          TCHAR('\xbb'), numExported, nullptr, &rpt),
            PWScore::SUCCESS);
  EXPECT_EQ(numExported, 1);
  ASSERT_TRUE(pws_os::FileExists(exportFile));

  importText(exportFile, TCHAR('\t'), 1);

  auto iter = core.Find(_T("network"), _T("router-special"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetCustomFieldsRaw(), rawFields);

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

TEST_F(ImportTextTest, export_import_custom_fields_utf8_roundtrip)
{
  const stringT exportFile = _T("import-text-unit-test-customfields-utf8.txt");
  const StringX sxExportFile(exportFile.c_str());

  pws_os::DeleteAFile(exportFile);

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("network"));
  ci.SetTitle(_T("router-utf8"));
  ci.SetUser(_T("admin"));
  ci.SetPassword(_T("secret"));

  CustomFieldList fields;
  CustomField cf;
  cf.SetName(_T("cafe 名称"));
  cf.SetValue(_T("paard café Ελληνικά Пример 日本語"));
  fields.push_back(cf);
  ci.SetCustomFields(fields);

  const StringX rawFields = ci.GetCustomFieldsRaw();

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  CItemData::FieldBits bsFields;
  bsFields.set();
  int numExported(0);
  CReport rpt;
  ASSERT_EQ(exportCore.WritePlaintextFile(sxExportFile, bsFields, _T(""), 0, 0,
                                          TCHAR('\xbb'), numExported, nullptr, &rpt),
            PWScore::SUCCESS);
  EXPECT_EQ(numExported, 1);
  ASSERT_TRUE(pws_os::FileExists(exportFile));

  importText(exportFile, TCHAR('\t'), 1);

  auto iter = core.Find(_T("network"), _T("router-utf8"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetCustomFieldsRaw(), rawFields);

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}
