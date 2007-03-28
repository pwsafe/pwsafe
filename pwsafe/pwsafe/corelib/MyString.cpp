/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file MyString.cpp
//-----------------------------------------------------------------------------

#include "PwsPlatform.h"
#include "Util.h"

#include "MyString.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CMyString::CMyString() : m_mystring(_T(""))
{
	init();
}

CMyString::CMyString(LPCTSTR lpsz) : m_mystring(lpsz)
{
	init();
}

CMyString::CMyString(LPCTSTR lpsz, int nLength) : m_mystring(lpsz, nLength)
{
	init();
}

CMyString::CMyString(const CMyString& stringSrc) : m_mystring(stringSrc.m_mystring)
{
	init();
}

CMyString::CMyString(const CString& stringSrc) : m_mystring(stringSrc)
{
	init();
}

CMyString::~CMyString()
{
   trashstring();
#if defined(UNICODE)
	trashbuffer();
#endif
}


void
CMyString::init()
{
#if defined(UNICODE)
	char_buffer		= 0;
	char_buffer_len	= 0;
#endif
}

#if defined(UNICODE)
void CMyString::trashbuffer()
{
	if ( char_buffer != 0 )
	{
		trashMemory((unsigned char*)char_buffer, char_buffer_len * sizeof(char_buffer[0]));
		delete [] char_buffer;
		char_buffer		= 0;
		char_buffer_len	= 0;
	}
}
#endif


void
CMyString::trashstring()
{
	trashMemory( m_mystring.GetBuffer(m_mystring.GetLength()), m_mystring.GetLength() );
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

void
CMyString::Empty()
{
	trashstring();
	m_mystring.Empty();
}

BOOL
CMyString::LoadString(const UINT &nID)
{
	return m_mystring.LoadString(nID);
}

void
CMyString::Format(LPCTSTR lpszFormat, ... )
{
	va_list args;
	va_start(args, lpszFormat);
	m_mystring.FormatV(lpszFormat, args);
	va_end(args);
}

void
CMyString::Format(UINT nID, ... )
{
	va_list args;
	va_start(args, nID);
	CString csFormat(MAKEINTRESOURCE(nID));
	m_mystring.FormatV(csFormat, args);
	va_end(args);
}

const CMyString&
CMyString::operator=(const CMyString& stringSrc)
{
   trashstring();
#if defined(UNICODE)
   trashbuffer();
#endif
   m_mystring = stringSrc.m_mystring;
   return *this;
}

const CMyString&
CMyString::operator=(TCHAR ch)
{
   trashstring();
#if defined(UNICODE)
   trashbuffer();
#endif
   m_mystring = ch;
   return *this;
}

const CMyString&
CMyString::operator=(LPCTSTR lpsz)
{
   trashstring();
#if defined(UNICODE)
   trashbuffer();
#endif
   m_mystring = lpsz;
   return *this;
}

#ifndef UNICODE // do we need this at all?
const CMyString&
CMyString::operator=(const unsigned char* psz)
{
   trashstring();
#if defined(UNICODE)
   trashbuffer();
#endif
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

TCHAR
CMyString::GetAt(int nIndex)
{
   return m_mystring.GetAt(nIndex);
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
/*
#ifdef UNICODE
/*
 * Returns a c-style string i.e. a null terminated char* array.
 *
CMyString::operator LPCSTR() const
{
	int		len;
	LPSTR	buf;
	LPTSTR	uni;

	const_cast<CMyString*>(this)->trashbuffer();

	len	= this->GetLength();
	uni	= const_cast<CMyString*>(this)->GetBuffer( len );		// override const attribute
	buf	= new CHAR[ len + 1 ];

	_wcstombsz( buf, uni, len + 1 );

	(PCHAR) char_buffer		= buf;		// override const attribute
	(int) char_buffer_len	= len + 1;		// override const attribute

	return char_buffer;
}
#endif
*/

/*
 * If compiling to a unicode target this returns a null terminated wide string
 * i.e. a wchar_t array, otherwise ite returns a c-style string.
 */ 
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

//Can't properly trash the memory here, so it is better to just return a CString
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

#ifdef UNICODE
bool
operator==(const CMyString& s1, LPCSTR s2)
{
	CString	t(s2);
	return (const CString)s1 == (const CString)t;
}
#endif

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

#ifdef UNICODE
bool
operator!=(const CMyString& s1, LPCSTR s2)
{
	CString	t(s2);
	return (const CString)s1 != (const CString)t;
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

