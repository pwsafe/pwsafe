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
#include "core/PWSrand.h"
#include "core/Util.h"
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
// Support for LengthRegression_* tests below: craft a V4 attachment record
// on disk with an arbitrary (possibly malformed) CONTENT length field, so
// that we can verify CItemAtt::Read() rejects bad lengths instead of
// misbehaving on them (e.g. by mis-sizing an allocation for the content).
const StringX kAttRegPass(_T("regression-pass"));

// Exposes PWSfileV4's protected m_fd so the test can write a raw content
// block whose length doesn't match the CONTENT field it wrote earlier.
class CraftV4 : public PWSfileV4
{
public:
  CraftV4(const StringX &f, PWSfile::RWmode m, PWSfile::VERSION v) : PWSfileV4(f, m, v) {}
  FILE *fd() { return m_fd; }
};

void WriteMalformedAttachment(const StringX &file, uint32_t len32)
{
  CraftV4 fw(file, PWSfile::Write, PWSfile::V40);
  ASSERT_EQ(PWSfile::SUCCESS, fw.Open(kAttRegPass));

  unsigned char IV[TwoFish::BLOCKSIZE], EK[PWSfileV4::KLEN], AK[PWSfileV4::KLEN];
  PWSrand::GetInstance()->GetRandomData(IV, sizeof(IV));
  PWSrand::GetInstance()->GetRandomData(EK, sizeof(EK));
  PWSrand::GetInstance()->GetRandomData(AK, sizeof(AK));

  uuid_array_t uuid;
  pws_os::CUUID cu;
  cu.GetARep(uuid);

  fw.WriteField(CItemAtt::ATTUUID, uuid, sizeof(uuid_array_t));
  fw.WriteField(CItemAtt::ATTIV, IV, sizeof(IV));
  fw.WriteField(CItemAtt::ATTEK, EK, sizeof(EK));
  fw.WriteField(CItemAtt::ATTAK, AK, sizeof(AK));

  const unsigned char lb[4] = {(unsigned char)(len32 & 0xff), (unsigned char)((len32 >> 8) & 0xff),
                               (unsigned char)((len32 >> 16) & 0xff), (unsigned char)((len32 >> 24) & 0xff)};
  fw.WriteField(CItemAtt::CONTENT, lb, sizeof(lb));          // <-- malformed length

  TwoFish fish(EK, sizeof(EK));
  unsigned char payload[16];
  for (int i = 0; i < 16; ++i) payload[i] = 0x41;
  _writecbcRest(fw.fd(), payload, sizeof(payload), &fish, IV);

  fw.WriteField(static_cast<unsigned char>(0xff), _T(""));
  fw.Close();
}

// A malformed length must be rejected, not written past a zero-length allocation.
void ExpectSafeReject(const StringX &file)
{
  PWSfileV4 fr(file, PWSfile::Read, PWSfile::V40);
  int st = fr.Open(kAttRegPass);
  if (st != PWSfile::SUCCESS)
    return;      // an open failure is an acceptable safe outcome

  CItemAtt att;
  EXPECT_NE(PWSfile::SUCCESS, fr.ReadRecord(att));   // must be rejected, and must not crash
  fr.Close();
}

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
  const stringT f(_T("ItemAttLengthRegression.psafe4"));
  for (uint32_t v : bad) {
    WriteMalformedAttachment(f.c_str(), v);
    ExpectSafeReject(f.c_str());
    pws_os::DeleteAFile(f);
  }
}

TEST_F(ItemAttTest, LengthRegression_ValidPositiveLengthsStillParse)
{
  const uint32_t good[] = {1u, 15u, 16u, 17u, 100u, 4096u};
  const stringT f(_T("ItemAttLengthRegression.psafe4"));
  for (uint32_t v : good) {
    WriteMalformedAttachment(f.c_str(), v);
    PWSfileV4 fr(f.c_str(), PWSfile::Read, PWSfile::V40);
    ASSERT_EQ(PWSfile::SUCCESS, fr.Open(kAttRegPass));
    CItemAtt att;
    EXPECT_EQ(PWSfile::READ_FAIL, fr.ReadRecord(att));  // truncated content -> read failure, not a crash
    fr.Close();
    pws_os::DeleteAFile(f);
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
