/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// MyString.h
// A drop-in replacement for CString, the main difference being that the
// data is scrubbed by trashstring() in the destructor, thus leaving an attacker
// with a little less info to grovel for in the swap file / core dump / whatever
//
// First priority for porting is to implement this without CString
//-----------------------------------------------------------------------------

#ifndef _MYSTRING_H_
#define _MYSTRING_H_
#include <afx.h>
//-----------------------------------------------------------------------------
class CMyString
{
public:
    CMyString() : m_mystring(_T("")) {}
    CMyString(LPCTSTR lpsz) : m_mystring(lpsz) {}
    CMyString(LPCTSTR lpsz, int nLength) : m_mystring(lpsz, nLength) {}
    CMyString(const CMyString& stringSrc) : m_mystring(stringSrc.m_mystring) {}
    CMyString(const CString& stringSrc) : m_mystring(stringSrc) {}

    ~CMyString() {trashstring();}

    TCHAR operator[](int nIndex) const {return m_mystring[nIndex];}

    TCHAR GetAt(int nIndex) {return m_mystring.GetAt(nIndex);}

    void SetAt(int nIndex, TCHAR ch) {m_mystring.SetAt(nIndex,ch);}

    operator CString() const {return m_mystring;}
    operator CString&() {return m_mystring;}
    operator LPCTSTR() const {return (LPCTSTR)m_mystring;}

    BOOL IsEmpty() const {return m_mystring.IsEmpty();}

    const CMyString& operator=(const CMyString& stringSrc);
    const CMyString& operator=(TCHAR ch);
    const CMyString& operator=(LPCTSTR lpsz);
#ifndef UNICODE // do we need this at all?
    const CMyString& operator=(const unsigned char* psz);
#endif
    const CMyString& operator+=(const CMyString& s) {m_mystring += s.m_mystring; return *this;}
    const CMyString& operator+=(TCHAR ch) {m_mystring += ch; return *this;}
    const CMyString& operator+=(LPCTSTR lpsz) {m_mystring += lpsz; return *this;}

    // CMyString operator+(LPCTSTR lpsz);

    friend CMyString AFXAPI operator+(const CMyString& string1,
                                      const CMyString& string2);
    friend CMyString AFXAPI operator+(const CMyString& string,
                                      TCHAR ch);
    friend CMyString AFXAPI operator+(TCHAR ch,
                                      const CMyString& string);
    friend CMyString AFXAPI operator+(const CMyString& string,
                                      LPCTSTR lpsz);
    friend CMyString AFXAPI operator+(LPCTSTR lpsz,
                                      const CMyString& string);

    LPTSTR GetBuffer(int nMinBufLength) {return m_mystring.GetBuffer(nMinBufLength);}
    LPTSTR GetBuffer() {return m_mystring.GetBuffer();}

    void ReleaseBuffer(int nNewLength = -1) {m_mystring.ReleaseBuffer(nNewLength);}

    int GetLength() const {return m_mystring.GetLength();}

    int FindByte( char ch ) const;
    int Find(TCHAR ch) const {return m_mystring.Find(ch);}
    int Find(LPCTSTR lpszSub) const {return m_mystring.Find(lpszSub);}
    int Find(TCHAR ch, int nstart) const {return m_mystring.Find(ch, nstart);}
    int Find(LPCTSTR lpszSub, int nstart) const {return m_mystring.Find(lpszSub, nstart);}
    int FindOneOf(LPCTSTR lpszSub) const {return m_mystring.FindOneOf(lpszSub);}
    int Replace(const TCHAR chOld, const TCHAR chNew) {return m_mystring.Replace(chOld,chNew);}
    int Replace(const LPCTSTR lpszOld, const LPCTSTR lpszNew)
        {return m_mystring.Replace(lpszOld,lpszNew);}
    int Remove(TCHAR ch) {return m_mystring.Remove(ch);}
    CMyString Left(int nCount) const;
    CMyString Right(int nCount) const;
    CMyString Mid(int nFirst) const;
    CMyString Mid(int nFirst, int nCount) const;
    void TrimRight() {m_mystring.TrimRight();}
    void TrimLeft() {m_mystring.TrimLeft();}
#if _MSC_VER >= 1400
    CMyString &Trim() {m_mystring.Trim(); return *this;}
#else
    CMyString &Trim()
      {m_mystring.TrimLeft(); m_mystring.TrimRight(); return *this}
#endif
    void MakeLower() {m_mystring.MakeLower();}
    int Compare(const LPCTSTR lpszOther) const {return m_mystring.Compare(lpszOther);}
    int CompareNoCase(const LPCTSTR lpszOther) const {return m_mystring.CompareNoCase(lpszOther);}
    void Empty();
    BOOL LoadString(const UINT &nID);
    void Format(LPCTSTR lpszFormat, ... );
    void Format(UINT nID, ... );
    BOOL IsOnlyWhiteSpace() const
      {CMyString t(*this); return t.Trim().IsEmpty();}
    void EmptyIfOnlyWhiteSpace()
      {if (IsOnlyWhiteSpace() == TRUE) Empty();}
    void Trash() {trashstring();}
    CMyString SpanIncluding(LPCTSTR lpszCharSet)
      {return CMyString(m_mystring.SpanIncluding(lpszCharSet));}
    CMyString SpanExcluding(LPCTSTR lpszCharSet)
      {return CMyString(m_mystring.SpanExcluding(lpszCharSet));}

private:
    CString m_mystring;
    void trashstring();
};
//-----------------------------------------------------------------------------

inline bool operator==(const CMyString& s1, const CMyString& s2)
{return (const CString)s1 == (const CString)s2;}
inline bool operator==(const CMyString& s1, LPCTSTR s2)
{return (const CString)s1==s2;}
inline bool operator==(LPCTSTR s1, const CMyString& s2)
{return s1==(const CString)s2;}
inline bool operator!=(const CMyString& s1, const CMyString& s2)
{return (const CString)s1 != (const CString)s2;}
inline bool operator!=(const CMyString& s1, LPCTSTR s2)
{return (const CString)s1 != s2;}
inline bool operator!=(LPCTSTR s1, const CMyString& s2)
{return s1 != (const CString)s2;}


//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

