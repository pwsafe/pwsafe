/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// StringXTest.cpp: Unit test for our "secure" string class

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include <sstream>

#include "core/StringX.h"
#include "core/StringXStream.h"
#include "os/typedefs.h"
#include "gtest/gtest.h"

using std::wstring;

TEST(StringXTest, testConstructors)
{
  StringX s0;
  // StringX()
  EXPECT_EQ(0, s0.length());
  EXPECT_TRUE(s0.empty());

  // StringX( const char* str )
  charT v1[] = _S("abcd");
  StringX s1(v1);
  EXPECT_EQ(4, s1.length());

  for (int i = 0; i < 4; i++)
    EXPECT_TRUE(v1[i] == s1[i]);

  // StringX( const StringX& s )
  StringX s2(s1);
  EXPECT_TRUE(s1 == s2);

  // StringX( size_type length, const char& ch )
  StringX s3(5, charT('X'));
  StringX s4(_S("XXXXX"));
  EXPECT_TRUE(s3 == s4);

  // StringX( const char* str, size_type length )
  StringX s5(v1, 2);
  EXPECT_EQ(2, s5.length());
  for (int i = 0; i < 2; i++)
    EXPECT_TRUE(v1[i] == s5[i]);

  // StringX( const string& str, size_type index, size_type length )
  StringX s6(s1, 1, 2);
  EXPECT_EQ(2, s6.length());
  EXPECT_TRUE(s6[0] == s1[1] && s6[1] == s1[2]);

  // StringX( input_iterator start, input_iterator end )
  StringX s7(s1.begin()+1, s1.end()-1);
  EXPECT_TRUE(s6 == s7);
}

TEST(StringXTest, testOperators)
{
  /* bool operator==(const StringX& c1, const StringX& c2) */
  StringX s1(_S("yada")), s2(_S("yada")), s3;
  EXPECT_TRUE(s1 == s2);
  EXPECT_TRUE(s2 != s3);

  /* bool operator!=(const StringX& c1, const StringX& c2) */
  EXPECT_TRUE(s2 != s3);
  EXPECT_FALSE((s1 != s2));

  /* bool operator<(const StringX& c1, const StringX& c2) */
  StringX s4(_S("one1")), s5(_S("one2"));
  EXPECT_TRUE(s4 < s5);

  /* bool operator>(const StringX& c1, const StringX& c2) */
  EXPECT_TRUE(s5 > s4);
    
  /* bool operator<=(const StringX& c1, const StringX& c2) */
  EXPECT_TRUE(s4 <= s5);
  EXPECT_TRUE(s4 <= s4);

  /* bool operator>=(const StringX& c1, const StringX& c2) */
  EXPECT_TRUE(s5 >= s4);
  EXPECT_TRUE(s5 >= s5);
    
  /* StringX operator+(const StringX& s1, const StringX& s2 ) */
  StringX s6(_S("one1one2"));
  EXPECT_TRUE(s6 == s4 + s5);

  /* StringX operator+(const char* s, const StringX& s2 ) */
  EXPECT_TRUE(s6 == _S("one1") + s5);

  /* StringX operator+( char c, const StringX& s2 ) */
  StringX s7 = charT('q') + s6;
  StringX s8(_S("qone1one2"));
  EXPECT_TRUE(s7 == s8);

  /* StringX operator+( const StringX& s1, const char* s ) */
  charT v2[] = _S("hell");
  StringX s9(_S("bloody")), s10(_S("bloodyhell"));
  EXPECT_TRUE(s10 == s9 + v2);

  /* StringX operator+( const StringX& s1, char c ) */
  StringX s11 = s6 + charT('q');
  StringX s12(_S("one1one2q"));
  EXPECT_TRUE(s11 == s12);

  /* ostream& operator<<( ostream& os, const StringX& s ) */
  oStringXStream os;
  os << s12;
  EXPECT_TRUE(os.str() == _S("one1one2q"));
    
  /* istream& operator>>( istream& is, StringX& s ) */
  iStringXStream is(_S("15"));
  int x;
  is >> x;
  EXPECT_EQ(15, x);
  /* StringX& operator=( const StringX& s ) */
  StringX s14(_S("mumble"));
  StringX s15;
  s15 = s14;
  EXPECT_TRUE(s14 == s15);

  /* StringX& operator=( const char* s ) */
  s15 = _S("oklahoma");
  EXPECT_TRUE(s15 == _S("oklahoma"));

  /* StringX& operator=( char ch ) */
  s15 = charT('W');
  EXPECT_TRUE(s15.length() == 1 && s15[0] == charT('W'));

  /* char& operator[]( size_type index ) */
  s14[0] = charT('j');
  EXPECT_TRUE(s14 == _S("jumble"));
}

TEST(StringXTest, testAppend)
{
  /* StringX& append( const StringX& str ); */
  StringX s1(_S("blue")), s2(_S("eyes")), s3;
  s3 = s1.append(s2);
  EXPECT_TRUE(_S("blueeyes") == s3);
    
  /* StringX& append( const char* str ); */
  s1 = _S("blue");
  s3 = s1.append(_S("nose"));
  EXPECT_TRUE(_S("bluenose") == s3);
    
  /* StringX& append( const StringX& str, size_type index, size_type len ); */
  StringX s4(_S("green"));
  s3 = s4.append(s3, 4, 4);
  EXPECT_TRUE(_S("greennose") == s3);
    
  /* StringX& append( const char* str, size_type num ); */
  s3 = s3.append(_S("redyellow"), 3);
  EXPECT_TRUE(_S("greennosered") == s3);
    
  /* StringX& append( size_type num, char ch ); */
  s3 = s3.append(3, charT('!'));
  EXPECT_TRUE(_S("greennosered!!!") == s3);
    
  /* StringX& append( input_iterator start, input_iterator end ); */
  s3 = _S("Yeehaw");
  s3 = s3.append(s3.begin()+3, s3.end());
  EXPECT_TRUE(_S("Yeehawhaw") == s3);
}

TEST(StringXTest, testAssign)
{
  StringX s1(_S("Flew"));
  s1.assign(2, charT('B'));
  EXPECT_TRUE(_S("BB") == s1);

  StringX s2;
  s2.assign(s1);
  EXPECT_TRUE(s1 == s2);
}


TEST(TrimLeft, SpaceOnLeft) {
  wstring s{L" abc"};
  EXPECT_EQ(TrimLeft(s), L"abc");
  EXPECT_EQ(s, L"abc");
}

TEST(TrimLeft, SpaceOnRight) {
  wstring s{L"abc "};
  EXPECT_EQ(TrimLeft(s), L"abc ");
  EXPECT_EQ(s,  L"abc ");
}

TEST(TrimLeft, SpaceOnBothSides) {
  wstring s{L" abc "};
  EXPECT_EQ(TrimLeft(s), L"abc ");
  EXPECT_EQ(s,  L"abc ");
}

TEST(TrimLeft, SpaceInMiddle) {
  wstring s{L"ab cd"};
 EXPECT_EQ(TrimLeft(s), L"ab cd");
 EXPECT_EQ(s, L"ab cd");
}


TEST(TrimLeft, SpaceInMiddleAndLeft) {
  wstring s{L" ab cd"};
  EXPECT_EQ(TrimLeft(s), L"ab cd");
  EXPECT_EQ(s, L"ab cd");
}


TEST(TrimLeft, SpaceInMiddleAndRight) {
  wstring s{L"ab cd "};
  EXPECT_EQ(TrimLeft(s), L"ab cd ");
  EXPECT_EQ(s, L"ab cd ");
}

TEST(TrimLeft, SpaceInLeftMiddleAndRight) {
  wstring s{L" ab cd "};
  EXPECT_EQ(TrimLeft(s), L"ab cd ");
  EXPECT_EQ(s, L"ab cd ");
}

TEST(TrimLeft, AllSpaces) {
  wstring s{L" "};
  EXPECT_EQ(TrimLeft(s), L"");
  EXPECT_EQ(s, L"");
}

TEST(TrimLeft, EmptyString) {
  wstring s{L""};
  EXPECT_EQ(TrimLeft(s), L"");
  EXPECT_EQ(s, L"");
}

/////////////// TrimRight ////////////////////

TEST(TrimRight, SpaceOnRight) {
  wstring s{L"abc "};
 EXPECT_EQ(TrimRight(s), L"abc");
 EXPECT_EQ(s, L"abc");
}

TEST(TrimRight, SpaceOnBothSides) {
  wstring s{L" abc "};
  EXPECT_EQ(TrimRight(s), L" abc");
  EXPECT_EQ(s, L" abc");
}

TEST(TrimRight, SpaceInMiddle) {
  wstring s{L"ab cd"};
  EXPECT_EQ(TrimRight(s), L"ab cd");
  EXPECT_EQ(s, L"ab cd");
}

TEST(TrimRight, SpaceInMiddleAndLeft) {
  wstring s{L" ab cd"};
  EXPECT_EQ(TrimRight(s), L" ab cd");
  EXPECT_EQ(s, L" ab cd");
}

TEST(TrimRight, SpaceInMiddleAndRight) {
  wstring s{L"ab cd "};
  EXPECT_EQ(TrimRight(s), L"ab cd");
  EXPECT_EQ(s, L"ab cd");
}

TEST(TrimRight, AllSpaces) {
  wstring s{L" "};
  EXPECT_EQ(TrimRight(s), L"");
  EXPECT_EQ(s, L"");
}

TEST(TrimRight, EmptyString) {
  wstring s{L""};
  EXPECT_EQ(TrimRight(s), L"");
  EXPECT_EQ(s, L"");
}

///////////// trim //////////////////////

TEST(trim, SpaceOnBothSides) {
  wstring s{L" abcd "};
  EXPECT_EQ(Trim(s), L"abcd");
  EXPECT_EQ(s, L"abcd");
}

TEST(trim, SpaceOnLeft) {
  wstring s{L" abc"};
  EXPECT_EQ(Trim(s), L"abc");
  EXPECT_EQ(s, L"abc");
}

TEST(trim, SpaceOnRight) {
  wstring s{L"abc "};
  EXPECT_EQ(Trim(s), L"abc");
  EXPECT_EQ(s, L"abc");
}

TEST(trim, SpaceInMiddle) {
  wstring s{L"ab cd"};
  EXPECT_EQ(Trim(s), L"ab cd");
  EXPECT_EQ(s, L"ab cd");
}

TEST(trim, AllSpaces) {
  wstring s{L" "};
  EXPECT_EQ(Trim(s), L"");
  EXPECT_EQ(s, L"");
}

TEST(trim, EmptyString) {
  wstring s{L""};
  EXPECT_EQ(Trim(s), L"");
  EXPECT_EQ(s, L"");
}
