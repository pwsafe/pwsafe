/// \file MyString.cpp
//-----------------------------------------------------------------------------

#include "Util.h"

#include "MyString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CMyString::CMyString() : m_mystring(_T(""))
{
}

CMyString::CMyString(LPCTSTR lpsz) : m_mystring(lpsz)
{
}

CMyString::CMyString(LPCTSTR lpsz, int nLength) : m_mystring(lpsz, nLength)
{
}

CMyString::CMyString(const CMyString& stringSrc) : m_mystring(stringSrc.m_mystring)
{
}

CMyString::CMyString(const CString& stringSrc) : m_mystring(stringSrc)
{
}

CMyString::~CMyString()
{
   trashstring();
}


void
CMyString::trashstring()
{
   trashMemory((unsigned char*)m_mystring.GetBuffer(m_mystring.GetLength()),
               m_mystring.GetLength());
}


LPTSTR
CMyString::GetBuffer(int nMinBufLength)
{
   return m_mystring.GetBuffer(nMinBufLength);
}

void
CMyString::ReleaseBuffer(int nNewLength)
{
   m_mystring.ReleaseBuffer(nNewLength);
}

int
CMyString::GetLength() const
{
   return m_mystring.GetLength();
}

const CMyString&
CMyString::operator=(const CMyString& stringSrc)
{
   trashstring();
   m_mystring = stringSrc.m_mystring;
   return *this;
}

const CMyString&
CMyString::operator=(TCHAR ch)
{
   trashstring();
   m_mystring = ch;
   return *this;
}

const CMyString&
CMyString::operator=(LPCTSTR lpsz)
{
   trashstring();
   m_mystring = lpsz;
   return *this;
}

#ifndef UNICODE // do we need this at all?
const CMyString&
CMyString::operator=(const unsigned char* psz)
{
   trashstring();
   m_mystring = psz;
   return *this;
}
#endif

const CMyString&
CMyString::operator+=(const CMyString& string)
{
   m_mystring += string.m_mystring;
   return *this;
}

const CMyString&
CMyString::operator+=(TCHAR ch)
{
   m_mystring += ch;
   return *this;
}

const CMyString&
CMyString::operator+=(LPCTSTR lpsz)
{
   m_mystring += lpsz;
   return *this;
}

CMyString AFXAPI
operator+(const CMyString& string1,const CMyString& string2)
{
   CMyString s;
   s = (CMyString)(string1.m_mystring+string2.m_mystring);
   return s;
}

CMyString AFXAPI
operator+(const CMyString& string, TCHAR ch)
{
   CMyString s;
   s = (CMyString)(string.m_mystring + ch);
   return s;
}

CMyString AFXAPI
operator+(TCHAR ch, const CMyString& string)
{
   CMyString s;
   s = (CMyString)(ch + string.m_mystring);
   return s;
}

CMyString AFXAPI
operator+(const CMyString& string, LPCTSTR lpsz)
{
   CMyString s;
   s = (CMyString)(string.m_mystring + lpsz);
   return s;
}

CMyString AFXAPI
operator+(LPCTSTR lpsz, const CMyString& string)
{
   CMyString s;
   s = (CMyString)(lpsz + string.m_mystring);
   return s;
}

TCHAR
CMyString::operator[](int nIndex) const
{
   return m_mystring[nIndex];
}

void
CMyString::SetAt(int nIndex, TCHAR ch)
{
   m_mystring.SetAt(nIndex,ch);
}

CMyString::operator CString() const
{
   return m_mystring;
}

CMyString::operator CString&()
{
   return m_mystring;
}

CMyString::operator LPCTSTR() const
{
   return (LPCTSTR)m_mystring;
}

BOOL
CMyString::IsEmpty() const
{
   return m_mystring.IsEmpty();
}

int
CMyString::FindByte(char ch) const
{
  int		nRetVal = -1;	// default to not found
  int		nIndex	= 0;;

  LPCTSTR pszString = LPCTSTR(m_mystring);

  while ( pszString[nIndex] ) {
#ifndef UNICODE
    if ( pszString[nIndex] == ch ) {
#else
    if ( LOBYTE(pszString[nIndex]) == ch ) {
#endif
      nRetVal = nIndex;
      break;
    }

    ++nIndex;
  }

  return nRetVal;
}

int
CMyString::Find(TCHAR ch) const
{
   return m_mystring.Find(ch);
}

int
CMyString::Find(LPCTSTR lpszSub) const
{
   return m_mystring.Find(lpszSub);
}

int
CMyString::Find(TCHAR ch, int nstart) const
{
   return m_mystring.Find(ch, nstart);
}

int
CMyString::Find(LPCTSTR lpszSub, int nstart) const
{
   return m_mystring.Find(lpszSub, nstart);
}

int
CMyString::FindOneOf(LPCTSTR lpszSub) const
{
   return m_mystring.FindOneOf(lpszSub);
}

int
CMyString::Replace(const TCHAR chOld, const TCHAR chNew) 
{
   return m_mystring.Replace(chOld,chNew);
}

int
CMyString::Replace(const LPCTSTR lpszOld, const LPCTSTR lpszNew) 
{
   return m_mystring.Replace(lpszOld,lpszNew);
}

int
CMyString::Remove(TCHAR ch) 
{
   return m_mystring.Remove(ch);
}

CMyString
CMyString::Left(int nCount) const
{
   CMyString s;
   s.m_mystring = m_mystring.Left(nCount);
   return s;
}

CMyString
CMyString::Right(int nCount) const
{
   CMyString s;
   s.m_mystring = m_mystring.Right(nCount);
   return s;
}

CMyString
CMyString::Mid(int nFirst) const
{
   CMyString s;
   s.m_mystring = m_mystring.Mid(nFirst);
   return s;
}

CMyString
CMyString::Mid(int nFirst, int nCount) const
{
   CMyString s;
   s.m_mystring = m_mystring.Mid(nFirst, nCount);
   return s;
}

bool
operator==(const CMyString& s1, const CMyString& s2)
{
   return (const CString)s1 == (const CString)s2;
}

bool
operator==(const CMyString& s1, LPCTSTR s2)
{
   return (const CString)s1==s2;
}

bool
operator==(LPCTSTR s1, const CMyString& s2)
{
   return s1==(const CString)s2;
}

bool
operator!=(const CMyString& s1, const CMyString& s2)
{
   return (const CString)s1 != (const CString)s2;
}

bool
operator!=(const CMyString& s1, LPCTSTR s2)
{
   return (const CString)s1 != s2;
}

bool
operator!=(LPCTSTR s1, const CMyString& s2)
{
   return s1 != (const CString)s2;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

