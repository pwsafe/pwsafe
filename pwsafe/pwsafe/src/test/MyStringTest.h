/*
* Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// MyStringTest.h: Use the test class
#include "test.h"
#include "MyString.h"
#include "os/pws_tchar.h"

class CMyStringTest : public Test
{

public:
  CMyStringTest()
  {
  }

  void run()
  {
    // The tests to run:
    testConstructors();
    testCasts();
    testCharAccess();
    testAssignment();
    testConcat();
    testExtract();
    testFind();
    testTrash();
  }

  // first test
  void testConstructors()
  {
    CMyString s0;
    // test CMyString() - empty ctor
    _test(s0.GetLength() == 0);
    _test(s0.IsEmpty() ? true : false);

    // test CMyString(LPCTSTR lpsz);
    LPCTSTR t1 = _T("onetwothree");
    CMyString s1(t1);
    _test(s1 == t1);

    // test CMyString(LPCTSTR lpsz, int nLength);
    CMyString s2(t1, 6);
    _test(s2 == _T("onetwo"));


    // test CMyString(const CMyString& stringSrc);
    CMyString s3(s1);
    _test(s3 == s1);

    // test CMyString(const CString& stringSrc);
    CString t2("fourfivesix");
    CMyString s4(t2);
    _test(CString(s4) == t2);

  }

  void testCasts()
  {
    // operator CString() const;
    LPCTSTR t1 = _T("alphabravocharliedelta");
    const CMyString s1(t1);
    CString v1 = CString(s1);
    _test(v1 == t1);
    // operator CString&();
    CMyString s2(t1);
    LPCTSTR t2 = _T("echofoxgolf");
    CString &v2 = s2;
    v2 = t2;
    _test(s2 == t2);
    // operator LPCTSTR() const;
    const CMyString s3(t2);
    _test(_tcscmp(t2, LPCTSTR(s3)) == 0 ? true : false);
  }

  void testCharAccess()
  {
    int i;
    // TCHAR operator[](int nIndex) const;
    CMyString s1(_T("abcdefghijklmnopqrstuvwxyz"));
    const TCHAR *t1 = _T("abcdefghijklmnopqrstuvwxyz");
    for (i = 0; i < 26; i++)
      _test(s1[i] == t1[i]);
    // void SetAt(int nIndex, TCHAR ch);
    CMyString s2(_T("--------------------------"));
    for (i = 0; i < 26; i++) {
      s2.SetAt(i, t1[i]);
      _test(s2[i] == t1[i]);
    }
  }

  void testAssignment()
  {
    // const CMyString& operator=(const CMyString& stringSrc);
    CMyString s1(_T("one"));
    CMyString s2;
    s2 = s1;
    _test(s1 == s2);
    // const CMyString& operator=(TCHAR ch);
    s1 = TCHAR('x');
    _test(s1.GetLength() == 1);
    _test(s1[0] == TCHAR('x'));
    // const CMyString& operator=(LPCTSTR lpsz);
    const LPCTSTR t3 = _T("ABC123acb!@#");
    CMyString s3;
    s3 = t3;
    _test(_tcscmp(t3, LPCTSTR(s3)) == 0 ? true : false);
#ifndef UNICODE
    // const CMyString& operator=(const unsigned char* psz);
    const unsigned char *t4 = (const unsigned char *)"yada-yada";
    CMyString s4 = t4;
    _test(::strcmp(s4, (const char *)t4) == 0 ? true : false);
#endif
  }

  void testConcat()
  {
    // const CMyString& operator+=(const CMyString& string);
    CMyString s1(_T("one "));
    CMyString s2(_T("plus one is two"));
    s1 += s2;
    _test(s1 == _T("one plus one is two"));
    // const CMyString& operator+=(TCHAR ch);
    CMyString s3(_T("cow"));
    s3 += TCHAR('s');
    _test(s3 == _T("cows"));
    // const CMyString& operator+=(LPCTSTR lpsz);
    CMyString s4(_T("Fish"));
    LPCTSTR t4 = _T(" and chips");
    s4 += t4;
    _test(s4 == _T("Fish and chips"));
    // 
    // friend CMyString AFXAPI operator+(const CMyString& string1,
    //                                   const CMyString& string2);
    CMyString s5(_T("Black"));
    CMyString s6(_T(" and white"));
    CMyString s7 = s5 + s6;
    _test(s7 == _T("Black and white"));
    // friend CMyString AFXAPI operator+(const CMyString& string,
    //                                   TCHAR ch);
    CMyString s8(_T("dog"));
    CMyString s9 = s8 + TCHAR('s');
    _test(s9 == _T("dogs"));
    // friend CMyString AFXAPI operator+(TCHAR ch,
    //                                   const CMyString& string);
    CMyString s10(_T("ats"));
    CMyString s11 = TCHAR('c') + s10;
    _test(s11 == _T("cats"));
    // friend CMyString AFXAPI operator+(const CMyString& string,
    //                                   LPCTSTR lpsz);
    CMyString s12(_T("Yin and"));
    CMyString s13 = s12 + _T(" yang");
    _test(s13 == _T("Yin and yang"));
    // friend CMyString AFXAPI operator+(LPCTSTR lpsz,
    //                                   const CMyString& string);
    // 
    CMyString s14(_T("Butthead"));
    CMyString s15 = _T("Beavis and ") + s14;
    _test(s15 == _T("Beavis and Butthead"));
  }

  void testExtract()
  {
    // CMyString Mid(int nFirst, int nCount) const;
    const CMyString s1(_T("1234567890"));
    _test(s1.Mid(3,3) == _T("456"));
    // CString Left(int nCount) const;
    _test(s1.Left(3) == _T("123"));
    // CString Right(int nCount) const;
    _test(s1.Right(3) == _T("890"));
    // void TrimRight();
    CMyString s2("   blablaBLA   ");
    s2.TrimRight();
    _test(s2 == _T("   blablaBLA"));
    // void TrimLeft();
    s2.TrimLeft();
    _test(s2 == _T("blablaBLA"));
    // void MakeLower();
    s2.MakeLower();
    _test(s2 == _T("blablabla"));
  }

  void testFind()
  {
    const CMyString s1(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    // int Find(TCHAR ch) const;
    int x2 = s1.Find(TCHAR('E'));
    _test(x2 == 4);
    // int Find(LPCTSTR lpszSub) const;
    int x3 = s1.Find(_T("XYZ"));
    _test(x3 == 23);
  }
  // 
  // 
  // last test
  void testTrash()
  {
    LPCTSTR t1 = _T("that's all, folks!");
    CMyString s1(t1);
    s1.Trash();
    _test(s1 != t1);
  }
};


