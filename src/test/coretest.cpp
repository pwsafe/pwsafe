/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#else
#include <cstring>
#endif

#include "gtest/gtest.h"

using namespace std;

#define TEST_BLOWFISH
#define TEST_TWOFISH
#define TEST_AES
#define TEST_SHA256
#define TEST_HMAC_SHA256
#define TEST_STRINGX
#define TEST_ITEMFIELD
#define TEST_KEYWRAP

#ifdef TEST_BLOWFISH
#include "BlowFishTest.h"
#endif
#ifdef TEST_TWOFISH
#include "TwoFishTest.h"
#endif
#ifdef TEST_AES
#include "AESTest.h"
#endif
#ifdef TEST_SHA256
#include "SHA256Test.h"
#endif
#ifdef TEST_HMAC_SHA256
#include "HMAC_SHA256Test.h"
#endif
#ifdef TEST_STRINGX
#include "StringXTest.h"
#endif
#ifdef TEST_ITEMFIELD
#include "ItemFieldTest.h"
#endif
#ifdef TEST_KEYWRAP
#include "KeyWrapTest.h"
#endif

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
#ifdef TEST_BLOWFISH
  CBlowFishTest t0;
  t0.setStream(&cout);
  t0.run();
  t0.report();
#endif
#ifdef TEST_MYSTRING
  CMyStringTest t1;
  t1.setStream(&cout);
  t1.run();
  t1.report();
#endif
#ifdef TEST_TWOFISH
  CTwoFishTest t2;
  t2.setStream(&cout);
  t2.run();
  t2.report();
#endif
#ifdef TEST_AES
  CAESTest t3;
  t3.setStream(&cout);
  t3.run();
  t3.report();
#endif
#ifdef TEST_SHA256
  CSHA256Test t4;
  t4.setStream(&cout);
  t4.run();
  t4.report();
#endif
#ifdef TEST_HMAC_SHA256
  CHMAC_SHA256Test t5;
  t5.setStream(&cout);
  t5.run();
  t5.report();
#endif
#ifdef TEST_STRINGX
  StringXTest t6;
  t6.setStream(&cout);
  t6.run();
  t6.report();
#endif
#ifdef TEST_ITEMFIELD
  ItemFieldTest t7;
  t7.setStream(&cout);
  t7.run();
  t7.report();
#endif
#ifdef TEST_KEYWRAP
  CKeyWrapTest t8;
  t8.setStream(&cout);
  t8.run();
  t8.report();
#endif
  return 0;
  return RUN_ALL_TESTS();
}
