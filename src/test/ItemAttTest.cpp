/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
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
#include "core/PWScore.h"
#include "core/PWSfile.h"
#include "core/PWSfileV4.h"
#include "core/crypto/TwoFish.h"
#include "os/file.h"
#include "os/dir.h"
#include "os/debug.h"

#include "gtest/gtest.h"

#include <vector>

using namespace std;

// A fixture for factoring common code across tests
class ItemAttTest : public ::testing::Test
{
protected:
  ItemAttTest(); // to init members
  void SetUp();

  // members used to populate and test fullItem:
  const StringX title, mediaType;
  stringT fullfileName, fileName, filePath;
};

ItemAttTest::ItemAttTest()
  : title(_T("a-title")), mediaType(_T("application/octet-stream"))
{
  fullfileName = pws_os::fullpath(L"data/image1.jpg");

  stringT sDrive, sDir, sFName, sExtn;
  pws_os::splitpath(fullfileName, sDrive, sDir, sFName, sExtn);
  fileName = sFName + sExtn;
  filePath = sDrive + sDir;
}

void ItemAttTest::SetUp()
{
    if (!pws_os::FileExists(fullfileName)) {
        wcout << L"Can't find " << fullfileName << L", no sense in continuing." << endl;
        FAIL();
    }
}

namespace {
// A minimal PWSfile stand-in that serves one canned field (followed by an
// END marker) straight from memory, with no encryption or file I/O
// involved. CItem::Read() only ever calls PWSfile::ReadField(), which
// dispatches to the virtual ReadCBC() below - everything else about
// PWSfile is irrelevant to it, so this is enough to drive
// CItemAtt::Read()/CItemData::Read() directly.
class FakeFieldSource : public PWSfile
{
public:
  FakeFieldSource(unsigned char type, const unsigned char *data, size_t len)
    : PWSfile(_T(""), PWSfile::Read, PWSfile::V40),
      m_type(type), m_data(data, data + len), m_done(false)
  {
    // PWSfile::GetOffset() is not virtual and unconditionally calls
    // ftell(m_fd) at the end of CItem::Read() - it needs a real, open
    // FILE*, even though none of our canned field data ever goes through
    // it. std::tmpfile() is anonymous and deleted in std::exit().
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // tmpfile() flagged as "unsafe"; tmpfile_s() isn't portable
#endif
    m_fd = std::tmpfile();
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  }

  int Open(const StringX &) override { return SUCCESS; }
  int WriteRecord(const CItemData &) override { return FAILURE; }
  int ReadRecord(CItemData &) override { return FAILURE; }

protected:
  size_t WriteCBC(unsigned char, const StringX &) override { return 0; }

  size_t ReadCBC(unsigned char &type, unsigned char *&data, size_t &length) override
  {
    if (m_done) {
      type = CItemAtt::END;
      data = nullptr;
      length = 0;
      return 1;
    }
    m_done = true;
    type = m_type;
    length = m_data.size();
    if (length > 0) {
      data = new unsigned char[length];
      for (size_t i = 0; i < length; i++)
        data[i] = m_data[i];
    } else {
      data = nullptr;
    }
    return 1;
  }

private:
  unsigned char m_type;
  std::vector<unsigned char> m_data;
  bool m_done;
};

// Support for LengthRegression_* below. A PWSfileV4 stand-in that serves
// ATTIV/ATTEK/CONTENT straight from memory, same idea as FakeFieldSource -
// except CItemAtt::Read()'s CONTENT case calls PWSfileV4::ReadContent(),
// which is not virtual and does its own fread()/decrypt directly against
// m_fd. So this still needs a real (anonymous, tmpfile()-backed) FILE*
// holding some actual bytes; what it doesn't need is a real V40 file,
// a real password, or PWSfileV4::Open()'s key-stretching. Since none of
// the tests using this ever supply ATTAK/CONTENTHMAC, CItemAtt::Read()
// is guaranteed to end up in its "missing prerequisites" READ_FAIL branch
// regardless of what ReadContent() makes of the length - it exists purely
// to observe that a bad length is handled safely, not accepted.
class FakeV4ContentSource : public PWSfileV4
{
public:
  explicit FakeV4ContentSource(uint32_t contentLen32)
    : PWSfileV4(_T(""), PWSfile::Read, PWSfile::V40), m_step(0)
  {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // tmpfile() flagged as "unsafe"; tmpfile_s() isn't portable
#endif
    m_fd = std::tmpfile();
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    // What's actually on "disk": 16 arbitrary bytes - plausible content
    // for a real single-block attachment, but deliberately unrelated to
    // whatever length the CONTENT field below claims.
    const unsigned char payload[16] = {0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
                                       0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41};
    std::fwrite(payload, 1, sizeof(payload), m_fd);
    std::fseek(m_fd, 0, SEEK_SET);

    m_lenBytes[0] = static_cast<unsigned char>(contentLen32 & 0xff);
    m_lenBytes[1] = static_cast<unsigned char>((contentLen32 >> 8) & 0xff);
    m_lenBytes[2] = static_cast<unsigned char>((contentLen32 >> 16) & 0xff);
    m_lenBytes[3] = static_cast<unsigned char>((contentLen32 >> 24) & 0xff);
  }

protected:
  size_t ReadCBC(unsigned char &type, unsigned char *&data, size_t &length) override
  {
    switch (m_step++) {
    case 0: // ATTIV
      type = CItemAtt::ATTIV;
      length = TwoFish::BLOCKSIZE;
      data = new unsigned char[length]();
      return 1;
    case 1: // ATTEK
      type = CItemAtt::ATTEK;
      length = PWSfileV4::KLEN;
      data = new unsigned char[length]();
      return 1;
    case 2: // CONTENT - the (possibly malformed) length under test
      type = CItemAtt::CONTENT;
      length = sizeof(m_lenBytes);
      data = new unsigned char[length];
      for (size_t i = 0; i < length; i++)
        data[i] = m_lenBytes[i];
      return 1;
    default:
      type = CItemAtt::END;
      data = nullptr;
      length = 0;
      return 1;
    }
  }

private:
  int m_step;
  unsigned char m_lenBytes[4];
};
} // namespace

// And now the tests...

TEST_F(ItemAttTest, EmptyItems)
{
  CItemAtt ai1, ai2;
  EXPECT_TRUE(ai1 == ai2);

  ai1.SetTitle(title);
  EXPECT_FALSE(ai1 == ai2);  

  ai2.SetTitle(title);
  EXPECT_TRUE(ai1 == ai2);
}

TEST_F(ItemAttTest, UUIDs)
{
  CItemAtt ai1, ai2;
  EXPECT_FALSE(ai1.HasUUID());

  ai1.CreateUUID();
  ai2.CreateUUID();
  EXPECT_FALSE(ai1 == ai2);

  ai2.SetUUID(ai1.GetUUID());
  EXPECT_TRUE(ai1 == ai2);
}

TEST_F(ItemAttTest, ImpExp)
{
  const stringT testImpFile(fullfileName);
  const stringT testExpFile(L"output.tmp");
  CItemAtt ai;
  int status;

  status = ai.Import(L"nosuchfile");
  EXPECT_EQ(PWScore::CANT_OPEN_FILE, status);
  EXPECT_EQ(L"", ai.GetFileName());
  EXPECT_FALSE(ai.HasContent());

  status = ai.Import(testImpFile);
  EXPECT_EQ(PWScore::SUCCESS, status);
  EXPECT_STREQ(fileName.c_str(), ai.GetFileName().c_str());
  EXPECT_STREQ(filePath.c_str(), ai.GetFilePath().c_str());
  EXPECT_TRUE(ai.HasContent());

  status = ai.Export(testExpFile);
  EXPECT_EQ(PWScore::SUCCESS, status);
  EXPECT_TRUE(pws_os::FileExists(testExpFile));

  FILE *f1 = pws_os::FOpen(testImpFile, L"rb");
  FILE *f2 = pws_os::FOpen(testExpFile, L"rb");
  EXPECT_EQ(pws_os::fileLength(f1), pws_os::fileLength(f2));

  size_t flen = static_cast<size_t>(pws_os::fileLength(f1));
  unsigned char *m1 = new unsigned char[flen];
  unsigned char *m2 = new unsigned char[flen];

  ASSERT_EQ(1U, fread(m1, flen, 1, f1));
  ASSERT_EQ(1U, fread(m2, flen, 1, f2));

  fclose(f1); fclose(f2);
  EXPECT_EQ(0, memcmp(m1, m2, flen));

  delete[] m1; delete[] m2;
  pws_os::DeleteAFile(testExpFile);
}

TEST_F(ItemAttTest, CopyCtor)
{
  const stringT testImpFile(fullfileName);
  CItemAtt ea1;
  CItemAtt ea2(ea1);
  EXPECT_TRUE(ea1 == ea2);

  CItemAtt a1;
  a1.SetTitle(title);
  int status = a1.Import(testImpFile);
  ASSERT_EQ(PWScore::SUCCESS, status);

  CItemAtt a2(a1);
  EXPECT_TRUE(a1 == a2);
}

TEST_F(ItemAttTest, Getters_n_Setters)
{
  CItemAtt ai;
  pws_os::CUUID uuid;
  time_t cTime = 1425836169;
  unsigned char content[122] = {0xff, 0x00, 0xb4, 0x65, 0xfc, 0x91, 0xfb, 0xbf,
                                0xe0, 0x8f, 0xea, 0x6b, 0xf9, 0x9f, 0xde, 0x1f,
                                0x62, 0xd6, 0xbf, 0xe8, 0x2f, 0xfb, 0x2c, 0xff,
                                0x00, 0xe0, 0xf3, 0xe1, 0xff, 0x00, 0xff, 0x00,
                                0x1d, 0xa5, 0x5b, 0x2d, 0x67, 0xbe, 0xaf, 0xfb,
                                0x2c, 0xff, 0x00, 0xe0, 0xf3, 0xc0, 0x1f, 0xfc,
                                0x76, 0x8a, 0x29, 0x7f, 0x68, 0x4b, 0xf9, 0x23,
                                0xf7, 0x7f, 0xc1, 0x1f, 0xd4, 0xd7, 0xf3, 0x3f,
                                0xbc, 0x5f, 0xb1, 0x6b, 0x1f, 0xf4, 0x17, 0xfd,
                                0x96, 0x7f, 0xf0, 0x79, 0xe0, 0x0f, 0xfe, 0x3b,
                                0x47, 0xd8, 0xb5, 0x8f, 0xfa, 0x0b, 0xfe, 0xcb,
                                0x3f, 0xf8, 0x3c, 0xf0, 0x07, 0xff, 0x00, 0x1d,
                                0xa2, 0x8a, 0x3f, 0xb4, 0x25, 0xfc, 0x91, 0xfb,
                                0xbf, 0xe0, 0x8b, 0xea, 0x6b, 0xf9, 0x9f, 0xde,
                                0xb0, 0xf1, 0xa7, 0xef, 0xa6, 0xdb, 0xf3, 0x3f,
                                0xff, 0xd9};
  ai.SetUUID(uuid);
  ai.SetTitle(title);
  ai.SetCTime(cTime);
  ai.SetContent(content, sizeof(content));

  time_t tVal = 0;
  unsigned char *contentVal;

  EXPECT_EQ(uuid, ai.GetUUID());
  EXPECT_EQ(title, ai.GetTitle());
  EXPECT_EQ(cTime, ai.GetCTime(tVal));
  ASSERT_EQ(sizeof(content), ai.GetContentLength());

  size_t contentSize = ai.GetContentSize();
  contentVal = new unsigned char[contentSize];

  EXPECT_FALSE(ai.GetContent(contentVal, contentSize - 1));
  EXPECT_TRUE(ai.GetContent(contentVal, contentSize));
  EXPECT_EQ(0, memcmp(content, contentVal, sizeof(content)));

  delete[] contentVal;
}

TEST_F(ItemAttTest, LengthRegression_VulnerableBoundaryValuesAreRejected)
{
  const uint32_t bad[] = {0x00000000u, 0xffffffffu, 0xfffffffeu, 0xfffffffdu, 0xfffffffcu,
                          0xfffffffbu, 0xfffffffau, 0xfffffff9u, 0xfffffff8u, 0xfffffff7u,
                          0xfffffff6u, 0xfffffff5u, 0xfffffff4u, 0xfffffff3u, 0xfffffff2u,
                          0xfffffff1u, 0xfffffff0u, 0x80000000u, 0x7fffffffu};
  for (uint32_t v : bad) {
    FakeV4ContentSource src(v);
    CItemAtt att;
    EXPECT_NE(PWSfile::SUCCESS, att.Read(&src));  // must be rejected, and must not crash
  }
}

TEST_F(ItemAttTest, LengthRegression_ValidPositiveLengthsStillParse)
{
  const uint32_t good[] = {1u, 15u, 16u, 17u, 100u, 4096u};
  for (uint32_t v : good) {
    FakeV4ContentSource src(v);
    CItemAtt att;
    EXPECT_EQ(PWSfile::READ_FAIL, att.Read(&src));  // truncated content -> read failure, not a crash
  }
}

// A 1-byte ATTUUID field must be rejected, not read past.
TEST_F(ItemAttTest, ATTUUID_ShortLengthMustNotBeAccepted)
{
  const unsigned char shortUuid[1] = {0xAA};
  FakeFieldSource src(CItemAtt::ATTUUID, shortUuid, sizeof(shortUuid));

  CItemAtt att;
  att.Read(&src);

  // A 1-byte field must never be accepted as a 16-byte UUID.
  EXPECT_FALSE(att.HasUUID());
}
