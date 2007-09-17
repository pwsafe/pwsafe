/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file MyString.cpp
//-----------------------------------------------------------------------------

#include "MyString.h"
#include "Util.h"

#include <cstdarg>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void
CMyString::trashstring()
{
   trashMemory((unsigned char*)m_mystring.GetBuffer(m_mystring.GetLength()),
               m_mystring.GetLength());
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
  if (this != &stringSrc) {
    trashstring();
    m_mystring = stringSrc.m_mystring;
  }
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

