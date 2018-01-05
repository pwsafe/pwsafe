/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include "core/ItemField.h"
#include "core/BlowFish.h"
#include "gtest/gtest.h"

class NullFish : public Fish
{
public:
  NullFish() {}
  virtual ~NullFish() {}
virtual unsigned int GetBlockSize() const {return 8;}
  // Following encrypt/decrypt a single block
  // (blocksize dependent on cipher)
  virtual void Encrypt(const unsigned char *pt, unsigned char *ct)
  {memcpy(ct, pt, GetBlockSize());}
  virtual void Decrypt(const unsigned char *ct, unsigned char *pt)
  {memcpy(pt, ct, GetBlockSize());}
};

class ItemFieldTest : public ::testing::Test
{

public:
  ItemFieldTest()
    {
#if 1
      unsigned char sessionkey[64] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
      };
      unsigned char salt[20] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
      };
      m_bf = BlowFish::MakeBlowFish(sessionkey, sizeof(sessionkey),
                                    salt, sizeof(salt));
#else
      m_bf = new NullFish;
#endif 
  }
  ~ItemFieldTest() {delete m_bf;}
 protected:
  Fish *m_bf;
};

TEST_F(ItemFieldTest, testMe)
{
  unsigned char v1[16] = {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
                          0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf};
  unsigned char v2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};
  size_t lenV2 = sizeof(v2);
    
  CItemField i1(1);
  EXPECT_TRUE(i1.IsEmpty());
  EXPECT_EQ(1, i1.GetType());

  i1.Set(v1, sizeof(v1), m_bf);
  i1.Get(v2, lenV2, m_bf);
  EXPECT_EQ(sizeof(v1), lenV2);
  EXPECT_TRUE(memcmp(v1, v2, sizeof(v1)) == 0);
}
