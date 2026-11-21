/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SearchUtilsTest.cpp: Unit tests for FindMatches() in core/SearchUtils.h
// (the search used by the wxWidgets Find toolbar and pwsafe-cli)

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/SearchUtils.h"
#include "gtest/gtest.h"

#include <cstdio>
#include <cwctype>
#include <set>
#include <string>
#include <vector>

namespace {

typedef std::vector<CItemData> ItemVec;
typedef std::vector<StringX> Titles;

// CItem::SetField() is protected. Exposing it lets the field coverage test
// store raw text in *any* field, including those CItemData::SetFieldValue()
// doesn't handle (credit card, passkey, V3 attachment fields...)
class RawFieldItem : public CItemData
{
public:
  using CItem::SetField; // not CItemData::SetField, which is private
};

StringX Upper(StringX s)
{
  for (auto &c : s)
    c = static_cast<wchar_t>(std::towupper(c));
  return s;
}

std::string FieldId(int ft)
{
  char buf[8];
  std::snprintf(buf, sizeof(buf), "0x%02x", ft);
  return buf;
}

class SearchUtilsTest : public ::testing::Test
{
protected:
  void SetUp() override;

  // Returns the titles of all matching entries, in container order
  Titles Find(const StringX &text, bool caseSensitive,
              const CItemData::FieldBits &bsFields) const;
  Titles Find(const StringX &text, bool caseSensitive = false) const
  {
    CItemData::FieldBits all;
    all.set();  // same default as the wx and CLI front-ends
    return Find(text, caseSensitive, all);
  }

  // Common to both entries' passwords
  const StringX sharedPasswordWord = L"secret";

  const StringX plainTitle = L"plain entry";
  const StringX plainPassword = L"plain " + sharedPasswordWord;
  const StringX plainNotes = L"nothing here folks, please move on";

  // An entry with a V3 attachment
  const StringX attTitle = L"entry with attachment";
  const StringX attPassword = L"attachment " + sharedPasswordWord;
  const StringX attAttTitle = L"Passport scan";    // mixed case, for case-sensitivity tests
  const StringX attAttFileName = L"my_passport.jpg";
  const StringX attAttMediaType = L"image/jpeg";

  ItemVec items;
};

void SearchUtilsTest::SetUp()
{
  CItemData plain;
  plain.CreateUUID();
  plain.SetTitle(plainTitle);
  plain.SetPassword(plainPassword);
  plain.SetNotes(plainNotes);
  items.push_back(plain);

  CItemData withAtt;
  withAtt.CreateUUID();
  withAtt.SetTitle(attTitle);
  withAtt.SetPassword(attPassword);
  withAtt.SetAttTitle(attAttTitle);
  withAtt.SetAttFileName(attAttFileName);
  withAtt.SetAttMediaType(attAttMediaType);
  const unsigned char content[] = {0xff, 0xd8, 0xff, 0xe0, 0xff, 0xd9};
  withAtt.SetAttContent(content, sizeof(content));
  items.push_back(withAtt);
}

Titles SearchUtilsTest::Find(const StringX &text, bool caseSensitive,
                             const CItemData::FieldBits &bsFields) const
{
  Titles found;
  ::FindMatches(text, caseSensitive, bsFields,
                false, stringT{}, CItemData::END, PWSMatch::MR_INVALID, false,
                items.begin(), items.end(),
                [](ItemVec::const_iterator it) -> const CItemData & { return *it; },
                [&found](ItemVec::const_iterator it, bool *keep_going) {
                  found.push_back(it->GetTitle());
                  *keep_going = true;
                });
  return found;
}

} // namespace

TEST_F(SearchUtilsTest, regularFieldsStillFound)
{
  EXPECT_EQ(Titles({plainTitle}), Find(plainNotes));
  EXPECT_EQ(Titles({plainTitle, attTitle}), Find(sharedPasswordWord));
  EXPECT_TRUE(Find(L"no such text").empty());
}

// Bug SF1638: Find does not find text on the Attachment tab of an entry
TEST_F(SearchUtilsTest, findsAttachmentTitle)
{
  ASSERT_NE(attAttTitle, Upper(attAttTitle)); // else the case tests are meaningless
  EXPECT_EQ(Titles({attTitle}), Find(attAttTitle));
  EXPECT_EQ(Titles({attTitle}), Find(Upper(attAttTitle)));        // case-insensitive
  EXPECT_EQ(Titles({attTitle}), Find(attAttTitle, true));         // case-sensitive
  EXPECT_TRUE(Find(Upper(attAttTitle), true).empty());
}

TEST_F(SearchUtilsTest, findsAttachmentFileName)
{
  EXPECT_EQ(Titles({attTitle}), Find(attAttFileName));
  EXPECT_EQ(Titles({attTitle}), Find(Upper(attAttFileName)));
}

TEST_F(SearchUtilsTest, doesNotSearchAttachmentMediaType)
{
  // We expect users to search for a file type by its suffix
  EXPECT_TRUE(Find(attAttMediaType).empty());

  // Don't find it even if the media type bit is the only one set
  CItemData::FieldBits bs;
  bs.set(CItemData::DATA_ATT_MEDIATYPE);
  EXPECT_TRUE(Find(attAttMediaType, false, bs).empty());
}

TEST_F(SearchUtilsTest, attachmentFieldsHonorFieldSelection)
{
  CItemData::FieldBits bs;
  bs.set();
  bs.reset(CItemData::DATA_ATT_TITLE);
  EXPECT_TRUE(Find(attAttTitle, false, bs).empty());
  EXPECT_EQ(Titles({attTitle}), Find(attAttFileName, false, bs));

  bs.set(CItemData::DATA_ATT_TITLE);
  bs.reset(CItemData::DATA_ATT_FILENAME);
  EXPECT_EQ(Titles({attTitle}), Find(attAttTitle, false, bs));
  EXPECT_TRUE(Find(attAttFileName, false, bs).empty());

  // Only an attachment field selected
  bs.reset();
  bs.set(CItemData::DATA_ATT_TITLE);
  EXPECT_EQ(Titles({attTitle}), Find(attAttTitle, false, bs));
  EXPECT_TRUE(Find(attPassword, false, bs).empty());
}

TEST_F(SearchUtilsTest, attachmentRemovedNotFound)
{
  items[1].ClearV3Attachment();
  EXPECT_TRUE(Find(attAttTitle).empty());
  EXPECT_TRUE(Find(attAttFileName).empty());
}

// This test guards against adding a text field to CItem::FieldType and forgetting
// to decide whether Find should search it.
//
// Every CItemData text field (per CItemData::IsTextField()) must appear in
// exactly one of `searched` or `notSearched` below, and FindMatches() must
// behave accordingly. A new text field fails this test until it's classified.
// A new non-text field also fails it, until it's added to IsTextField().
TEST_F(SearchUtilsTest, everyTextFieldIsClassified)
{
  // Text fields that FindMatches() must search
  const std::set<int> searched = {
    CItemData::GROUP, CItemData::TITLE, CItemData::USER, CItemData::NOTES,
    CItemData::PASSWORD, CItemData::URL, CItemData::AUTOTYPE,
    CItemData::PWHIST,   // previous passwords
    CItemData::RUNCMD, CItemData::EMAIL,
    CItemData::DATA_ATT_TITLE, CItemData::DATA_ATT_FILENAME,
  };

  // Text fields that FindMatches() does not search.
  // Moving a field from here to `searched` is a policy decision, not a test fix.
  const std::set<int> notSearched = {
    CItemData::POLICY,             // encoded password policy, not user text
    CItemData::DATA_ATT_MEDIATYPE, // users search by file name suffix instead
    CItemData::TWOFACTORKEY,       // not searched by Windows FindAll() either
    CItemData::CCNUM, CItemData::CCEXP, CItemData::CCVV, CItemData::CCPIN, // ditto
    CItemData::PASSKEY_RP_ID,      // ditto
    // NOTE: the following ARE searched by the Windows UI (DboxMain::FindAll),
    // so wx and CLI differ from it here:
    CItemData::SYMBOLS, CItemData::POLICYNAME, CItemData::CUSTOMTEXT,
  };

  // IDs in [START, LAST_DATA) that aren't stored fields at all
  auto isNotAField = [](int ft) {
    return ft == CItemData::GROUPTITLE ||   // pseudo-field: Group and Title are searched separately
           ft == 0x20 ||                    // unassigned
           (ft >= CItemData::LAST_USER_FIELD && ft < CItemData::BASEUUID); // unassigned
  };

  const StringX marker = L"Xq7SearchMarker";
  // A valid V3 password history holding `marker` as the one old password:
  // status(1) max(2 hex) count(2 hex), then per entry: time(8 hex) length(4 hex) password
  wchar_t len[5];
  std::swprintf(len, NumberOf(len), L"%04x", static_cast<unsigned>(marker.length()));
  const StringX pwhistWithMarker = StringX(L"10501") + L"00000000" + len + marker;

  CItemData::FieldBits all;
  all.set();
  int nTextFields = 0;

  for (int ft = CItemData::START; ft < CItemData::LAST_DATA; ++ft) {
    if (!CItemData::IsTextField(static_cast<unsigned char>(ft)) || isNotAField(ft))
      continue;
    ++nTextFields;

    const bool isSearched = searched.count(ft) != 0;
    const bool isNotSearched = notSearched.count(ft) != 0;
    ASSERT_TRUE(isSearched != isNotSearched)
      << "Text field " << FieldId(ft) << " must be in exactly one of `searched` or `notSearched`."
      << " If it's a new field, decide whether Find should search it (core/SearchUtils.h and"
      << " DboxMain::FindAll()), and classify it here. If it isn't text, add it to"
      << " CItemData::IsTextField().";

    RawFieldItem item;
    item.SetField(ft, ft == CItemData::PWHIST ? pwhistWithMarker : marker);
    items.assign(1, item);

    EXPECT_EQ(isSearched ? 1u : 0u, Find(marker, true, all).size())
      << "Field " << FieldId(ft) << " is " << (isSearched ? "" : "not ")
      << "expected to be searched by FindMatches()";
  }

  // Sanity check the loop itself
  EXPECT_EQ(searched.size() + notSearched.size(), static_cast<size_t>(nTextFields));
}
