/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/// \file MyString.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "SecString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


SecString::~SecString()
{
   trashstring();
}

void SecString::trashstring()
{
   trashMemory((unsigned char*)m_mystring.GetBuffer(m_mystring.GetLength()),
               m_mystring.GetLength());
}

LPTSTR SecString::GetBuffer(int nMinBufLength)
{
   return m_mystring.GetBuffer(nMinBufLength);
}

void SecString::ReleaseBuffer(int nNewLength)
{
   m_mystring.ReleaseBuffer(nNewLength);
}

int SecString::GetLength() const
{
   return m_mystring.GetLength();
}

const SecString& SecString::operator=(const SecString& stringSrc)
{
   trashstring();
   m_mystring = stringSrc.m_mystring;
   return *this;
}

const SecString& SecString::operator=(TCHAR ch)
{
   trashstring();
   m_mystring = ch;
   return *this;
}

const SecString& SecString::operator=(LPCSTR lpsz)
{
   trashstring();
   m_mystring = lpsz;
   return *this;
}

const SecString& SecString::operator=(LPCWSTR lpsz)
{
   trashstring();
   m_mystring = lpsz;
   return *this;
}

const SecString& SecString::operator=(const unsigned char* psz)
{
   trashstring();
   m_mystring = psz;
   return *this;
}

const SecString& SecString::operator+=(const SecString& string)
{
   m_mystring += string.m_mystring;
   return *this;
}

const SecString& SecString::operator+=(TCHAR ch)
{
   m_mystring += ch;
   return *this;
}

const SecString& SecString::operator+=(LPCTSTR lpsz)
{
   m_mystring += lpsz;
   return *this;
}

SecString AFXAPI operator+(const SecString& string1,const SecString& string2)
{
   SecString s;
   s = (SecString)(string1.m_mystring+string2.m_mystring);
   return s;
}

SecString AFXAPI operator+(const SecString& string, TCHAR ch)
{
   SecString s;
   s = (SecString)(string.m_mystring + ch);
   return s;
}

SecString AFXAPI operator+(TCHAR ch, const SecString& string)
{
   SecString s;
   s = (SecString)(ch + string.m_mystring);
   return s;
}

SecString AFXAPI operator+(const SecString& string, LPCTSTR lpsz)
{
   SecString s;
   s = (SecString)(string.m_mystring + lpsz);
   return s;
}

SecString AFXAPI operator+(LPCTSTR lpsz, const SecString& string)
{
   SecString s;
   s = (SecString)(lpsz + string.m_mystring);
   return s;
}

SecString SecString::Mid(int nFirst, int nCount) const
{
   return m_mystring.Mid(nFirst,nCount);
}

TCHAR SecString::operator[](int nIndex) const
{
   return m_mystring[nIndex];
}

void SecString::SetAt(int nIndex, TCHAR ch)
{
   m_mystring.SetAt(nIndex,ch);
}

SecString::operator CString() const
{
   return m_mystring;
}

SecString::operator LPCTSTR() const
{
   return (LPCTSTR)m_mystring;
}

BOOL SecString::IsEmpty() const
{
   return m_mystring.IsEmpty();
}

BOOL SecString::LoadString(UINT nID)
{
   return m_mystring.LoadString(nID);
}


int SecString::Find( TCHAR ch ) const
{
   return m_mystring.Find(ch);
}

int SecString::Find( LPCTSTR lpszSub ) const
{
   return m_mystring.Find(lpszSub);
}

//Can't properly trash the memory here, so it is better to just return a CString
CString SecString::Left( int nCount ) const
{
   return m_mystring.Left(nCount);
}

//Can't properly trash the memory here, so it is better to just return a CString
CString SecString::Right( int nCount ) const
{
   return m_mystring.Right(nCount);
}

bool operator==(const SecString& s1, const SecString& s2)
{
   return s1.m_mystring==s2.m_mystring;
}

bool operator==(const SecString& s1, LPCTSTR s2)
{
   return s1.m_mystring==s2;
}

bool operator==(LPCTSTR s1, const SecString& s2)
{
   return s1==s2.m_mystring;
}

bool operator!=(const SecString& s1, const SecString& s2)
{
   return s1.m_mystring!=s2.m_mystring;
}

bool operator!=(const SecString& s1, LPCTSTR s2)
{
   return s1.m_mystring!=s2;
}

bool operator!=(LPCTSTR s1, const SecString& s2)
{
   return s1!=s2.m_mystring;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
