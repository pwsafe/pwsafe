/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ImportXmlTest.cpp: Unit test for importing/exporting XML

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "os/file.h"
#include "os/dir.h"
#include "core/PWScore.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

std::string readFileUtf8(const stringT &filename)
{
  const std::filesystem::path path(filename.c_str());
  std::ifstream ifs(path, std::ios::binary);
  EXPECT_TRUE(ifs.is_open());
  return std::string(std::istreambuf_iterator<char>(ifs),
                     std::istreambuf_iterator<char>());
}

void writeFileUtf8(const stringT &filename, const std::string &content)
{
  const std::filesystem::path path(filename.c_str());
  std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
  ASSERT_TRUE(ofs.is_open());
  ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
  ASSERT_TRUE(static_cast<bool>(ofs));
}

void removeSensitiveElement(const stringT &filename)
{
  std::string xml = readFileUtf8(filename);

  const std::string sensitiveTrue = "\t\t\t\t<sensitive>1</sensitive>\n";
  const std::string sensitiveFalse = "\t\t\t\t<sensitive>0</sensitive>\n";
  size_t pos = xml.find(sensitiveTrue);
  size_t len = sensitiveTrue.length();

  if (pos == std::string::npos) {
    pos = xml.find(sensitiveFalse);
    len = sensitiveFalse.length();
  }

  ASSERT_NE(pos, std::string::npos);
  xml.erase(pos, len);
  writeFileUtf8(filename, xml);
}

} // namespace

class ImportXmlTest : public ::testing::Test
{
protected:
  ImportXmlTest();

  void SetUp() override;
  void TearDown() override;

  void exportXml(PWScore &exportCore, const stringT &xmlFile, int expectedExports);
  void importXml(const stringT &xmlFile, int expectedImports);

  PWScore core;
  stringT xsdFile = L"pwsafe.xsd";
  int numValidated, numImported, numSkipped, numPWHErrors, numRenamed, numNoPolicy;
  int numRenamedPolicies, numShortcutsRemoved, numEmptyGroupsImported;
};

ImportXmlTest::ImportXmlTest()
  : numValidated(0), numImported(0), numSkipped(0), numPWHErrors(0),
    numRenamed(0), numNoPolicy(0), numRenamedPolicies(0),
    numShortcutsRemoved(0), numEmptyGroupsImported(0)
{
}

void ImportXmlTest::SetUp()
{
  ASSERT_TRUE(pws_os::chdir(L"data"));
  // Depending on how this is run, the xsd file may be in one of two places:
  if (pws_os::FileExists(L"../../../xml/" + xsdFile)) { // we're running in the source tree
    xsdFile = L"../../../xml/" + xsdFile;
  } else if (pws_os::FileExists(L"../../../" + xsdFile)) { // we're running in the build tree
    xsdFile = L"../../../" + xsdFile;
  }
  ASSERT_TRUE(pws_os::FileExists(xsdFile));
}

void ImportXmlTest::TearDown()
{
  ASSERT_TRUE(pws_os::chdir(L".."));
}

void ImportXmlTest::exportXml(PWScore &exportCore, const stringT &xmlFile, int expectedExports)
{
  pws_os::DeleteAFile(xmlFile);

  CItemData::FieldBits bsFields;
  bsFields.set();
  int numExported(0);
  CReport rpt;

  ASSERT_EQ(exportCore.WriteXMLFile(StringX(xmlFile.c_str()), bsFields, _T(""), 0, 0,
                                    TCHAR('\xbb'), _T(""), numExported, nullptr,
                                    false, &rpt),
            PWScore::SUCCESS);
  EXPECT_EQ(numExported, expectedExports);
  ASSERT_TRUE(pws_os::FileExists(xmlFile));
}

void ImportXmlTest::importXml(const stringT &xmlFile, int expectedImports)
{
  stringT xmlErrors, skippedList, pwhErrorList, renameList;
  CReport rpt;
  Command *cmd(nullptr);

  numValidated = numImported = numSkipped = numPWHErrors = numRenamed = numNoPolicy = 0;
  numRenamedPolicies = numShortcutsRemoved = numEmptyGroupsImported = 0;
  core.ReInit();

  const int status = core.ImportXMLFile(_T(""), xmlFile, xsdFile, false,
                                        xmlErrors, skippedList, pwhErrorList, renameList,
                                        numValidated, numImported, numSkipped,
                                        numPWHErrors, numRenamed, numNoPolicy,
                                        numRenamedPolicies, numShortcutsRemoved,
                                        numEmptyGroupsImported, rpt, cmd);

  ASSERT_EQ(status, PWScore::SUCCESS);
  EXPECT_EQ(numValidated, expectedImports);
  EXPECT_EQ(numImported, expectedImports);
  EXPECT_EQ(numSkipped, 0);
  EXPECT_EQ(numPWHErrors, 0);
  EXPECT_EQ(numRenamed, 0);
  EXPECT_EQ(numNoPolicy, 0);
  EXPECT_EQ(numRenamedPolicies, 0);
  EXPECT_EQ(numShortcutsRemoved, 0);
  EXPECT_EQ(numEmptyGroupsImported, 0);
  EXPECT_TRUE(xmlErrors.empty());
  EXPECT_TRUE(skippedList.empty());
  EXPECT_TRUE(pwhErrorList.empty());
  EXPECT_TRUE(renameList.empty());
  ASSERT_NE(cmd, nullptr);

  EXPECT_EQ(core.Execute(cmd), 0);
  delete cmd;
}

TEST_F(ImportXmlTest, export_import_custom_fields_roundtrip)
{
  const stringT exportFile = _T("import-xml-unit-test-customfields.xml");

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("network"));
  ci.SetTitle(_T("router"));
  ci.SetUser(_T("admin"));
  ci.SetPassword(_T("secret"));

  CustomFieldList fields;
  CustomField cf1;
  cf1.SetName(_T("Name"));
  cf1.SetValue(_T("Value"));
  cf1.SetSensitive(true);
  fields.push_back(cf1);

  CustomField cf2;
  cf2.SetName(_T("Wifi"));
  cf2.SetValue(_T("Test"));
  fields.push_back(cf2);

  ci.SetCustomFields(fields);

  const StringX rawFields = ci.GetCustomFieldsRaw();

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  exportXml(exportCore, exportFile, 1);
  importXml(exportFile, 1);

  auto iter = core.Find(_T("network"), _T("router"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetCustomFieldsRaw(), rawFields);

  const CustomFieldList importedFields = item.GetCustomFields();
  ASSERT_EQ(0U, importedFields.getErr());
  ASSERT_EQ(2U, importedFields.size());
  EXPECT_TRUE(importedFields[0].IsSensitive());
  EXPECT_FALSE(importedFields[1].IsSensitive());

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

TEST_F(ImportXmlTest, export_import_custom_fields_special_chars_roundtrip)
{
  const stringT exportFile = _T("import-xml-unit-test-customfields-special.xml");

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
  cf.SetSensitive(true);
  fields.push_back(cf);
  ci.SetCustomFields(fields);

  const StringX rawFields = ci.GetCustomFieldsRaw();

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  exportXml(exportCore, exportFile, 1);
  importXml(exportFile, 1);

  auto iter = core.Find(_T("network"), _T("router-special"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetCustomFieldsRaw(), rawFields);

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

TEST_F(ImportXmlTest, export_import_custom_fields_utf8_roundtrip)
{
  const stringT exportFile = _T("import-xml-unit-test-customfields-utf8.xml");

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

  exportXml(exportCore, exportFile, 1);
  importXml(exportFile, 1);

  auto iter = core.Find(_T("network"), _T("router-utf8"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  auto item = core.GetEntry(iter);
  EXPECT_EQ(item.GetCustomFieldsRaw(), rawFields);

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

TEST_F(ImportXmlTest, import_custom_fields_with_sensitive_element)
{
  const stringT exportFile = _T("import-xml-unit-test-customfields-sensitive.xml");

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("network"));
  ci.SetTitle(_T("router-sensitive"));
  ci.SetUser(_T("admin"));
  ci.SetPassword(_T("secret"));

  CustomFieldList fields;
  CustomField cf;
  cf.SetName(_T("ApiKey"));
  cf.SetValue(_T("abcdef"));
  cf.SetSensitive(true);
  fields.push_back(cf);
  ci.SetCustomFields(fields);

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  exportXml(exportCore, exportFile, 1);
  importXml(exportFile, 1);

  auto iter = core.Find(_T("network"), _T("router-sensitive"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  const CustomFieldList importedFields = core.GetEntry(iter).GetCustomFields();
  ASSERT_EQ(0U, importedFields.getErr());
  ASSERT_EQ(1U, importedFields.size());
  EXPECT_TRUE(importedFields[0].IsSensitive());

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}

TEST_F(ImportXmlTest, import_custom_fields_without_sensitive_element_defaults_false)
{
  const stringT exportFile = _T("import-xml-unit-test-customfields-no-sensitive.xml");

  PWScore exportCore;
  CItemData ci;
  ci.CreateUUID();
  ci.SetGroup(_T("network"));
  ci.SetTitle(_T("router-no-sensitive"));
  ci.SetUser(_T("admin"));
  ci.SetPassword(_T("secret"));

  CustomFieldList fields;
  CustomField cf;
  cf.SetName(_T("ApiKey"));
  cf.SetValue(_T("abcdef"));
  cf.SetSensitive(true);
  fields.push_back(cf);
  ci.SetCustomFields(fields);

  Command *pcmd = AddEntryCommand::Create(&exportCore, ci);
  ASSERT_NE(pcmd, nullptr);
  ASSERT_EQ(exportCore.Execute(pcmd), 0);
  delete pcmd;

  exportXml(exportCore, exportFile, 1);
  removeSensitiveElement(exportFile);
  importXml(exportFile, 1);

  auto iter = core.Find(_T("network"), _T("router-no-sensitive"), _T("admin"));
  ASSERT_NE(iter, core.GetEntryEndIter());
  const CustomFieldList importedFields = core.GetEntry(iter).GetCustomFields();
  ASSERT_EQ(0U, importedFields.getErr());
  ASSERT_EQ(1U, importedFields.size());
  EXPECT_FALSE(importedFields[0].IsSensitive());

  EXPECT_TRUE(pws_os::DeleteAFile(exportFile));
}
