/*
* Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// MyStringTest.h: Use the test class
#include <sstream>

#include "test.h"
#include "XString.h"
#include "XStringStream.h"
#include "os/typedefs.h"

class XStringTest : public Test
{

public:
  XStringTest()
  {
  }

  void run()
  {
    // The tests to run:
    testConstructors();
    testOperators();
    testAppend();
    testAssign();
  }

  void testConstructors()
  {
    XString s0;
    // XString()
    _test(s0.length() == 0);
    _test(s0.empty());
    
    // XString( const char* str )
    charT v1[] = _S("abcd");
    XString s1(v1);
    _test(s1.length() == 4);
    for (int i = 0; i < 4; i++)
      _test(s1[i] == v1[i]);
    
    // XString( const XString& s )
    XString s2(s1);
    _test(s1 == s2);

    // XString( size_type length, const char& ch )
    XString s3(5, charT('X'));
    XString s4(_S("XXXXX"));
    _test(s3 == s4);
    // XString( const char* str, size_type length )
    XString s5(v1, 2);
    _test(s5.length() == 2);
    for (int i = 0; i < 2; i++)
      _test(s5[i] == v1[i]);

    // XString( const string& str, size_type index, size_type length )
    XString s6(s1, 1, 2);
    _test(s6.length() == 2);
    _test(s6[0] == s1[1] && s6[1] == s1[2]);
    
    // XString( input_iterator start, input_iterator end )
    XString s7(s1.begin()+1, s1.end()-1);
    _test(s6 == s7);
  }

  void testOperators()
  {
    /* bool operator==(const XString& c1, const XString& c2) */
    XString s1(_S("yada")), s2(_S("yada")), s3;
    _test(s1 == s2);
    _test(!(s2 == s3));

/* bool operator!=(const XString& c1, const XString& c2) */
    _test(s2 != s3);
    _test(!(s1 != s2));

/* bool operator<(const XString& c1, const XString& c2) */
    XString s4(_S("one1")), s5(_S("one2"));
    _test(s4 < s5);

/* bool operator>(const XString& c1, const XString& c2) */
    _test(s5 > s4);
    
/* bool operator<=(const XString& c1, const XString& c2) */
    _test(s4 <= s5);
    _test(s4 <= s4);
/* bool operator>=(const XString& c1, const XString& c2) */
    _test(s5 >= s4);
    _test(s5 >= s5);
    
/* XString operator+(const XString& s1, const XString& s2 ) */
    XString s6(_S("one1one2"));
    _test(s6 == s4 + s5);

/* XString operator+(const char* s, const XString& s2 ) */
    _test(s6 == _S("one1") + s5);

/* XString operator+( char c, const XString& s2 ) */
    XString s7 = charT('q') + s6;
    XString s8(_S("qone1one2"));
    _test(s7 == s8);

/* XString operator+( const XString& s1, const char* s ) */
    charT v2[] = _S("hell");
    XString s9(_S("bloody")), s10(_S("bloodyhell"));
    _test(s10 == s9 + v2);

/* XString operator+( const XString& s1, char c ) */
    XString s11 = s6 + charT('q');
    XString s12(_S("one1one2q"));
    _test(s11 == s12);

/* ostream& operator<<( ostream& os, const XString& s ) */
    oXStringStream os;
    os << s12;
    _test(os.str() == _S("one1one2q"));
    
/* istream& operator>>( istream& is, XString& s ) */
    iXStringStream is(_S("15"));
    int x;
    is >> x;
    _test(x == 15);
/* XString& operator=( const XString& s ) */
    XString s14(_S("mumble"));
    XString s15;
    s15 = s14;
    _test(s15 == s14);

/* XString& operator=( const char* s ) */
    s15 = _S("oklahoma");
    _test(s15 == _S("oklahoma"));

/* XString& operator=( char ch ) */
    s15 = charT('W');
    _test(s15.length() == 1 && s15[0] == charT('W'));

/* char& operator[]( size_type index ) */
    s14[0] = charT('j');
    _test(s14 == _S("jumble"));
  }

  void testAppend()
  {
    /* XString& append( const XString& str ); */
    XString s1(_S("blue")), s2(_S("eyes")), s3;
    s3 = s1.append(s2);
    _test(s3 == _S("blueeyes"));
    
    /* XString& append( const char* str ); */
    s1 = _S("blue");
    s3 = s1.append(_S("nose"));
    _test(s3 == _S("bluenose"));
    
    /* XString& append( const XString& str, size_type index, size_type len ); */
    XString s4(_S("green"));
    s3 = s4.append(s3, 4, 4);
    _test(s3 == _S("greennose"));
    
    /* XString& append( const char* str, size_type num ); */
    s3 = s3.append(_S("redyellow"), 3);
    _test(s3 == _S("greennosered"));
    
    /* XString& append( size_type num, char ch ); */
    s3 = s3.append(3, charT('!'));
    _test(s3 == _S("greennosered!!!"));
    
    /* XString& append( input_iterator start, input_iterator end ); */
    s3 = _S("Yeehaw");
    s3 = s3.append(s3.begin()+3, s3.end());
    _test(s3 == _S("Yeehawhaw"));
  }
  void testAssign()
  {
    XString s1(_S("Flew"));
    s1.assign(2, charT('B'));
    _test(s1 == _S("BB"));
    XString s2;
    s2.assign(s1);
    _test(s1 == s2);
  }
};


